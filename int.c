/* int.c
	This file contains functions for system calls */

#define PA 13
#define AX 8

extern PROC* running;
extern STTY stty[NR_STTY];

/* Function prototypes */
int kcinth();
int kgetpid();
int kps();
int kchname(char *name);
int kkfork();
int ktswitch();
int kkwait(int *status);
int kkexit(int value);
int kkgetchar();
int kkputchar(int b);
int proc_time();
int ktimesleep (int b);
int kchcolor (int b);
int usgets (int port, char *y);
int uputs (int port, char *y);

/****************** syscall handler in C ***************************/
int kcinth()
{
	u16 segment, offset;
	int a,b,c,d, r;
	segment = running->uss;
	offset = running->usp;

	a = get_word(segment, offset + 2 * PA);
	b = get_word(segment, offset + 2 * (PA + 1));
	c = get_word(segment, offset + 2 * (PA + 2));
	d = get_word(segment, offset + 2 * (PA + 3));

   switch(a){
		case 0 : r = kgetpid();			break;
		case 1 : r = kps();				break;
		case 2 : r = kchname(b);		break;
		case 3 : r = kkfork();			break;
		case 4 : r = ktswitch();		break;
		case 5 : r = kkwait(b);			break;
		case 6 : r = kkexit(b);			break;
		case 7 : r = umfork();			break;
		case 8 : r = kexec(b);			break;
		case 9 : r = kpipe(b);			break;
		case 10: r = read_pipe(b,c,d);	break;
		case 11: r = write_pipe(b,c,d);	break;
		case 12: r = close_pipe(b);		break;
		case 13: r = pfd();				break;
		case 14: body();				break;
		case 15: r = proc_time();		break;
		case 16: r = ktimesleep(b);		break;
		case 17: r = kchcolor(b);		break;
		case 18: r = usgets (b, c);		break;
		case 19: r = uputs (b, c);		break;

		case 97: r = kkgetchar();		break;
		case 98: r = kkputchar(b);		break;

		case 99: kkexit(b);				break;
		default: printf("invalid syscall # : %d\n", a); 
   }
	put_word (r, segment, offset + 2 * AX);
}

int kgetpid()
{
	return running->pid;
}

int kps()
{
	int i;
	PROC *p;
	printf ("PID\tName\tStatus\tPriority\n");

	for (i = 0; i < NPROC; i++)
	{
		p = &(proc[i]);
		printf ("%d\t%s\t\t", p->pid, p->name);
		printf ("%d\t\t%d\n", p->status, p->priority);
	}
	return 1;
}

int kchname(char *name)
{
	char buf[32];
	char *cp = buf;
	int count = 0;
	
	//Get the characters one at a time
	while (count < 32)
	{
		*cp = get_byte (running->uss, name);

		if (*cp == 0)
			break;

		cp++;
		name++;
		count++;
	}

	buf[31] = 0;

	printf("Named changed from %s ", running->name);
	strcpy(running->name, buf);
	printf("to %s\n", running->name);

	return 1;
}

int kkfork()
{	
	PROC *child = kfork("/bin/u1");

	if (!child)
		return -1;

	put_word (child->pid, running->uss, running->usp + 16); //Parent's return value is child's pid
	put_word (0, child->uss, child->usp + 16); //Child's return value is 0

	return child->pid;
}

int ktswitch()
{
	running->time = 2; //Reset the timer	

    return tswitch();
}

int kkwait(int *status)
{	
	return kwait (status);
}

int kkexit(int value)
{
	kexit(value);
	return 0;
}

int kkgetchar()
{
	return getc();
}

int kkputchar(int b)
{
	putc(b);
}

int proc_time()
{
	return running->time;
}

int ktimesleep (int b)
{
	running->time = b;
	running->status = TIME;
	tswitch();	
}

int kchcolor (int b)
{
	b &= 0x7F;

	switch (b)
	{
		case 'r': color = HRED;		break;
		case 'y': color = HYELLOW;	break;
		case 'g': color = HGREEN;	break;
		case 'c': color = HCYAN;	break;
		case 'p': color = HPURPLE;	break;
	}
}

int usgets (int port, char *y)
{
	char buf[BUFLEN];
	//char *cp = buf;
	int count = 0;
	
	sgetline (&(stty[port]), buf);

	//Get the characters one at a time
	while (count < BUFLEN && buf[count] != '\n' && buf[count] != 0)
	{
		put_byte (buf[count], running->uss, y);

		y++;
		count++;
	}

	if (y == BUFLEN)
		y--;

	//Put a null character at the end of the string
	put_byte (0, running->uss, y);

	return 1;
}

int uputs (int port, char *y)
{
	char buf[BUFLEN];
	int count = 0;
	
	//Get the characters one at a time
	while (count < BUFLEN)
	{
		buf[count] = get_byte (running->uss, y);
	
		if (buf[count] == 0 || buf[count] == '\n')
			break;

		y++;
		count++;
	}

	if (count == BUFLEN)
		count--;
	
	buf[count] = 0;

	sputline (&stty[port], buf);

	return 1;
}
