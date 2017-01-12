/* serial.c
	This file contains definitions for functions related to serial drivers */

/* Function Prototypes */
int bputc (int port, int c);
int bgetc (int port);

int s1handler ();
int s2handler ();
int shandler (int port);
int do_errors ();
int do_modem ();

int enable_tx (STTY *t);
int disable_tx (STTY *t);

int do_rx (STTY *t);
int sgetc (STTY *t);
int sgetline (STTY *t, char *line);

int do_tx (STTY *t);
int sputc (STTY *t);
int sputline (STTY *t);

/* Function Definitions */

int bputc (int port, int c)
{
	//Busy wait for the port's Line Status Register to say that it's ready to receive a byte of data
	while ((in_byte (port + LSR) & 0x20) == 0);
	//Put the byte out
	out_byte (port + DATA, c); 
}

int bgetc (int port)
{
	//Busy wait for the port's LSR to say that it has a character to read
	while ((in_byte (port + LSR) & 0x01) == 0);
	//Grab the character
	return (in_byte (port + DATA) & 0x7F);
}

//Handler call for asm to allow for interrupts from port 0
int s0handler()
{
	shandler(0);
}

//Handler call for asm to allow for interrupts from port 1
int s1handler()
{
	shandler(1);
}

//Serial port interrupt handler
int shandler (int port)
{
	//Declare some variables to use
	STTY *t;
	int IntID, LineStatus, ModemStatus, intType, c;

	//Set the pointer to point to the port that the interrupt is coming from
	t = &stty[port];
	
	//Get the type of interrupt and status of the line and modem
	IntID = in_byte (t->port + IIR);
	LineStatus = in_byte (t->port + LSR);
	ModemStatus = in_byte (t->port + MSR);

	//Mask out bit #7
	intType = IntID & 7;

	//Handle the interrupt accordingly
	switch (intType)
	{
		case 6: do_errors (t);	break;
		case 4: do_rx (t);		break;
		case 2: do_tx (t);		break;
		case 0: do_modem (t);	break;
	}

	out_byte (INT_CTL, ENABLE);	//Reenable 8259 controller
}

//hahahahha
int do_errors (STTY *t)
{
	printf ("What's error checking?\n");
}

//No modem
int do_modem (STTY *t)
{
	printf ("There shouldn't be a modem available\n");
}

//Enable TX interrupts
int enable_tx (STTY *t)
{
	lock();
	out_byte (t->port + IER, 0x03); //Both tx and rx on
	t->tx_on = 1;
	unlock();
}

//Disable tx interrupts
int disable_tx (STTY *t) 
{
	lock();
	out_byte (t->port + IER, 0x01); //tx off, rx on
	t->tx_on = 0;
	unlock();
}

/* Input Driver Functions */

//Port-to-OS interrupt
int do_rx (STTY *t)
{
	int c;
	//Grab the character from the port, leaving out the highest bit
	c = in_byte (t->port) & 0x7F;

	if (t->inchars.value >= BUFLEN)
	{
		//Write a beep out to the port
		out_byte(t, 7);
		return 0;
	}
	
	//Show what came from the interrupt
	printf ("rx interrupt c = " );

	if (c == '\r')
		printf ("return \n");
	else if (c == '\n')
		printf ("newline \n");
	else
		printf ("%c\n", c);

	//Echo the character back to the terminal
	bputc (t->port, c);

	//Enter the character into the buff
	t->inbuf[t->inhead++] = c;
	t->inhead %= BUFLEN;

	//Let the semaphore know that there is another character in the inbuf
	V(&(t->inchars));	
}

//Get a character from the serial port
int sgetc (STTY *t)
{
	char c;

	// Wait for input character
	P (&(t->inchars));
	
	//Don't allow interrupts
	lock();
	//Take a character from the inbuf of the port
	c = t->inbuf[t->intail++]; //Tail is the point of buffer that hasn't been read from
	t->intail %= BUFLEN;
	unlock(); //Turn interrupts back on

	//return the character from the inbuf
	return c;
}

int sgetline (STTY *t, char *line)
{
	char c = sgetc (t);
	int i = 0;	

	//Read in characters until a newline, carriage return, or a null is reached
	//or the max buffer size is reached, to be extra safe
	while(c!= '\n' && c != '\r' && c != 0 && i < BUFLEN ) 
	{
		line[i] = c;
		c = sgetc (t);
		i++;
	}

	//Set the last character in the string to null so other functions work properly
	if (i == BUFLEN)
		i--;

	line[i] = 0;
	
	//printf ("String: %s\n", line);
}

/* Output Driver Functions */

//Outputs a character from the outbuf to the serial port
int do_tx (STTY *t)
{
	char c;
	printf ("tx interrupt\n");
	
	//Check if the outbuf is empty
	if (t->outspace.value == BUFLEN) //outspace.value keeps track of how much space is in the buffer.
	{								 //max value == buffer empty
		disable_tx(t);
		return 0;
	}

	//bputc (t->port, t->outbuf[t->outtail++]); //Not sure if this is supposed to go straight out to serial port
	//Get a character from the outbuf of the port
	c = t->outbuf[t->outtail++];
	t->outtail %= BUFLEN;

	//Write the character to the port
	out_byte(t->port, c);

	V (&(t->outspace)); //The semaphore keeps track of how much SPACE is in the buffer, not how many characters are IN it
}

int sputc (STTY *t, char c)
{
	//Wait for space in the buffer
	//Remember that this semaphore here keeps track of how much SPACE is in the buffer, not how many character are IN it
	P (&(t->outspace));

	lock();
	//Enter the character into the outbuffer
	t->outbuf[t->outhead++] = c;
	t->outhead %= BUFLEN;

	//If tx interrupts are not on, reenable them
	if (!t->tx_on)
		enable_tx(t);

	unlock();
}

//Put a line out to the terminal
int sputline (STTY *t, char *line)
{
	int i = 0;
	
	//printf ("Sputline: %s\n", line);
	
	//Put characters into outbuffer of the STTY until a newline or null character has been hit
	//Or until BUFLEN characters has been entered
	while (line[i] != '\n' && line[i] != 0 && i < BUFLEN)
		sputc (t, line[i++]);
	//Put out a carriage return and newline to maintain formatting
	sputc (t, '\n');
	sputc (t, '\r');
}
