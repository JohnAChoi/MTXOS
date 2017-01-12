/* t.c
	This file contains main and a few related functions */

#include "type.h"	 //Header file with declarations
#include "io.c"		 //I/O functions based on getc()/putc() 
#include "queue.c"	 //Queue functions
#include "str.c"	 //String functions
#include "filesys.c" //Includes the functions needed for loading an executable from the hard drive
#include "kernel.c"	 //Kernel functions
#include "int.c"	 //Interrupt handler
#include "pipe.c"	 //Pipe Functions
#include "vid.c"	 //Video drivers	
#include "timer.c"	 //Timer
#include "semaphore.c" //Semaphore functions
#include "serial.c"  //Serial Port Drivers

//Global function declarations
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

PIPE pipe[NPIPE];
OFT oft[NOFT];
STTY stty[NR_STTY];

int nproc;
int int80h();
int tinth();
int s0inth();
int s1inth();

char *pname[] = {"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };

char *p = "\n\rSerial Port Initialized\n\r\007";

/* Function Prototypes */
int body();
int init();
int vid_init();
int enable_irq (u16 irq_nr);
int timer_init();
int serial_init();

int scheduler();
int set_vector(u16 vector, u16 handler);
main();
int do_kfork( );
int do_tswitch();
int do_kexit();
int do_ksleep();
int do_kwait();
int do_kwakeup();
int do_Umode();

int body()
{
	char c; 
	
	//color = running->pid + 5;
	running->inkmode = 1;
	printf("\nproc %d starts from body()\n", running->pid);

	while(1){
		//color = running->pid + 5;
		printf("\n-------------------------------------------------------\n");
		printList("freelist  ", freeList);
		printList("readyQueue", readyQueue);
		printf("-------------------------------------------------------\n");
		printf("proc %d running: parent=%d\n",running->pid,running->ppid);
		printf("enter a char [s|f|q  z|a|w  u] : ");

		c = getc(); 
		printf("%c\n\n", c);
		switch(c){
			case 'f' : do_kfork();		break;
			case 's' : do_tswitch();	break;
			case 'q' : do_kexit();		break;
			case 'z' : do_ksleep();		break;
			case 'a' : do_kwakeup();	break;
			case 'w' : do_kwait();		break;
			case 'u' : do_Umode();		break;
		}
	}
}

int init()
{
	PROC *p; int i, j; PIPE *pi; OFT *o;
	printf("init ....\n");
	nproc = 1; //Assuming this function completes running, 1 proc will be running

	seconds = minutes = hours = 0;

	//Initialize pipes to be empty
	for (i = 0; i < NPIPE; i++)
	{
		pi = &pipe[i];
		pi->head = 0;
		pi->tail = 0;
		pi->room = 0;
		pi->data = 0;
		pi->nreader = 0;
		pi->nwriter = 0;
		pi->status = 0;
	}
	
	//Initialize OFT to be empty
	for (i = 0; i < NOFT; i++)
	{
		o = &oft[i];
		o->mode = 0;
		o->refCount = 0;
		o->pipe_ptr = 0;
	}

	//Initialize all procs
	for (i=0; i<NPROC; i++){
		p = &proc[i];	
		p->uss = 0;
		p->usp = 0;
		p->pid = i;
		p->ppid = 0;
		p->status = FREE;
		p->event = 0;
		p->priority = 0;     
		p->exitCode = 0;
		p->inkmode = 1;
		strcpy(proc[i].name, pname[i]);
		p->next = &proc[i+1];

		for (j = 0; j < NFD; j++)
			p->fd[j] = 0;
	}
	//Set the next of the last proc to null instead of some random place
	proc[NPROC-1].next = 0;

	//Place all procs into the free list
	freeList = &proc[0];

	//The readyQueue is empty at this point
	readyQueue = 0;

	/**** create P0 as running ******/
	p = get_proc(&freeList);     // allocate a PROC from freeList
	p->ppid = 0;                 // P0’s parent is itself
	p->status = READY;

	//Important values already in kstack of P0, so don't zero it out
	p->kstack[SSIZE-1] = (int*)body;  // resume point=address of body()
	p->ksp = &p->kstack[SSIZE-9];    // proc saved sp
	enqueue(&readyQueue, p);
	running = p;                 // P0 is now running
} 

int vid_init()
{
	int i, w;
	org = row = column = 0;
	color = HGREEN;

	set_VDC (CURSOR_SIZE, CURSOR_SHAPE);
	set_VDC (VID_ORG, 0);
	set_VDC (CURSOR, 0);

	w = 0x0700;

	for (i = 0; i < 25*80; i++)
		put_word (w, VID_BASE, 2*i);
}

int enable_irq(u16 irq_nr)
{
  lock();
    out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));

}

int timer_init()
{
  /* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */
  tick = 0; 
  out_byte(TIMER_MODE, SQUARE_WAVE);	// set timer to run continuously
  out_byte(TIMER0, TIMER_COUNT);	// timer count low byte
  out_byte(TIMER0, TIMER_COUNT >> 8);	// timer count high byte 
  enable_irq(TIMER_IRQ); 
}

int serial_init()
{
	int i;
	STTY *t;
	char *q;
	
	//Initialize both ports
	for (i = 0; i < NR_STTY; i++)
	{
		q = p;

		t = &stty[i];

		//Set the location of the port in RAM	
		if (i == 0)
			t->port = 0x3F8;
		else
			t->port = 0x2F8;

		//Initialize Sempaphore data
		t->inhead = t->intail = 0;
		t->inchars.value = 0;
		t->inchars.queue = 0;

		t->outhead = t->outtail = t->tx_on = 0;
		t->outspace.value = BUFLEN; 
		t->outspace.queue = 0;

		//Initialize other data in the stty struct
		t->ison = t->echo = 1;
		t->erase = '\b';
		t->kill = '@';
		t->intr = (char) 0177; //Delete
		t->quit = (char) 034; //ctrl + C
		t->x_on = (char) 021; //ctrl + Q
		t->x_off = (char) 023; //ctrl + S
		t->eof = (char) 004; //ctrl + D

		lock();
		out_byte (t->port+MCR, 0x09); //IRQ4 on, DTR on
		out_byte (t->port+IER, 0x00); //Interrupts Enabled Register = 0
		
		out_byte (t->port + LCR, 0x80); //Line Control Register Bit #7 active
		out_byte (t->port + DIVH, 0x00); //Generate baud rate
		out_byte (t->port + DIVL, 12);   //Baud Rate divisor = 12, baud rate = 9600

		out_byte (t->port + LCR, 0x03); //Set Line Control Register = 3 for 9600, 8bits/char, no parity

		out_byte (t->port + MCR, 0x0B); //Modem Control Register = IRQ4, DTS, RTS all on 1011
		out_byte (t->port + IER, 0x01); //Rx interrupt on, Tx off for now
		
		unlock ();

		enable_irq(4 - i); //Enable IRQ4 and IRQ3
	
		//bputc() message out to tell the port it's ready to go 
		while (*q)
		{
			bputc (t->port, *q);
			q++;
		}
	}
}

int scheduler()
{
	if (running->status == READY)     // if running is still READY
		enqueue(&readyQueue, running); // enter it into readyQueue
	running = dequeue(&readyQueue);   // new running
}

int set_vector(u16 vector, u16 handler)
{
     // put_word(word, segment, offset)
     put_word(handler, 0, vector<<2);
     put_word(0x1000,  0,(vector<<2) + 2);
}
            
main()
{
	vid_init();
    printf("MTX starts in main()\n");
    init();      // initialize and create P0 as running

    set_vector(80, int80h);

	//Serial port initialization
	set_vector (12, s0inth);
	set_vector (11, s1inth);
	serial_init();

	//Timer initialization
	lock();
	set_vector(8, tinth);
	timer_init();

    kfork("/bin/u1");     // P0 kfork() P1
    /*kfork("/bin/u1");     // P0 kfork() P1
    kfork("/bin/u1");     // P0 kfork() P1
    kfork("/bin/u1");     // P0 kfork() P1*/

    while(1){
      printf("P0 running\n");
      while(!readyQueue);
      printf("P0 switch process\n");
      tswitch();         // P0 switch to run P1
   }
}

/*************** kernel command functions ****************/
int do_kfork( )
{
	PROC *p = kfork("/bin/u1");
	
	if (p == 0){ printf("kfork failed\n"); return -1; }
	
	printf("PROC %d kfork a child %d\n", running->pid, p->pid);
	return p->pid;
}

int do_tswitch(){  
	tswitch(); 
}

int do_kexit() 
{
	char num[64];
	int val = 0;

	printf ("Enter an exit code: ");
	gets(num);
	val = atoi(num);

	kexit(val);
}

int do_ksleep()
{
	char num[64];
	int val;

	printf ("Enter a sleep value: ");
	gets(num);
	val = atoi(num);

	ksleep (val);
}

int do_kwait()
{
	int pid, val;

	pid = kwait (&val);
	
	printf ("kwait PID: %d\tStatus: %d\n", pid, val);
}

int do_kwakeup()
{
	char num[64];
	int val;

	printf ("Enter a wakeup value: ");
	gets(num);
	val = atoi(num);
	
	kwakeup (val);
}

int do_Umode()
{
	running->inkmode = 0;
	goUmode();
}
