/* kernel.c
	This file contains the definitions for kernel functions */

extern PROC proc[NPROC], *running, *freeList, *readyQueue;
extern int nproc;
extern goUmode();
extern PIPE pipe[NPIPE];
extern OFT oft[NOFT];

/* Function Prototypes */
PROC *kfork(char *filename); 
int ksleep (int event);
int kwakeup (int event);
int kexit (int exitVal);
int kwait (int *status);
int copyImage (u16 pseg, u16 cseg, u16 size);
int umfork ();
int kexec (char *file);

PROC *kfork(char *filename) // create a child process, begin from body()
{
	int i;
	u16 segment;
	PROC *p = get_proc(&freeList);

	if (!p){
		printf("no more PROC, kfork() failed\n");
		return 0;           
	}

	p->status = READY;
	p->priority = 1;         // priority = 1 for all proc except P0
	p->ppid = running->pid;          // parent = running
	p->parent = running;
	p->time = 2;			//Give each proc an arbitrary length of time to run

	//Unrolled the loop because the loop wouldn't stop iterating...
	p->kstack[SSIZE-1] = 0;      // all 0's  
	p->kstack[SSIZE-2] = 0;      // all 0's  
	p->kstack[SSIZE-3] = 0;      // all 0's  
	p->kstack[SSIZE-4] = 0;      // all 0's  
	p->kstack[SSIZE-5] = 0;      // all 0's  
	p->kstack[SSIZE-6] = 0;      // all 0's  
	p->kstack[SSIZE-7] = 0;      // all 0's  
	p->kstack[SSIZE-8] = 0;      // all 0's  
	p->kstack[SSIZE-9] = 0;      // all 0's  
	
	p->kstack[SSIZE-1] = (int*)goUmode;  // resume point=address of body()
	p->ksp = &p->kstack[SSIZE-9];    // proc saved sp
	enqueue(&readyQueue, p); // enter p into readyQueue by priority
	nproc++;

	segment = (p->pid + 1) * 0x1000; //Set which segment of memory the proc uses in Umode

	if (filename)
	{
		load (filename, segment);
		
		p->uss = segment;
		p->usp = segment - 24;

		for (i = 0; i < 12; i++)
			put_word ((u16) 0, (u16) segment, p->usp + 2 * i);

		put_word ((u16) segment, (u16) segment, p->usp);	//Set uDS to segment
		put_word ((u16) segment, (u16) segment, p->usp + 2);	//Set uES to segment
		put_word ((u16) segment, (u16) segment, p->usp + 20 );		//Set uCS to segment
		put_word ((u16) 0x0200, (u16) segment, p->usp + 22); //Set I-bit to 1 in the flag to allow for interrupts 			
	}

	return p;                        // return child PROC pointer
}

//Set the running process to sleep on an event
int ksleep (int event)
{
	running->event = event;
	running->status = SLEEP;
	tswitch();
}

//Wake up all processes sleeping on a specific event.
int kwakeup (int event)
{
	int i;
	PROC* p;

	for (i = 1; i < NPROC; i++)
	{
		p = &proc[i];
		if (p->status == SLEEP && p->event == event)
		{
			p->event = 0;
			p->status = READY;
			enqueue (&readyQueue, p);
		}
	}
}

int kexit (int exitVal)
{
	int i, wakeup = 0;
	PROC *p;

	printf ("exitVal: %d\n", exitVal);

	//Make sure P1 never dies
	if (running->pid == 1)
	{
		printf ("\nDon't kill P1 please.\n");
		return 0;		
	}

	//Ship off any of the running proc's children to P1
	for (i = 0; i < NPROC; i++)
	{
		p = &proc[i];
		if (p->status != FREE && p->ppid == running->pid)
		{
			p->ppid = 1;
			p->parent = &proc[1];
			wakeup++;
		}		
	}

	//Set running as dead and why it died
	running->exitCode = exitVal;
	running->status = DEAD;
	running->priority = 0;

	//Wake up the proc's parent
	kwakeup(running->parent);

	//If P1 got new kids, wake him up.
	if (wakeup)
		kwakeup(&proc[1]);

	tswitch();
}

int kwait (int *status)
{
	PROC *p;
	int i;
	int hasChild = 0;
	while (1)
	{
		for (i = 1; i < NPROC; i++)
		{
			p = &proc[i];
			if (p->status != FREE && p->ppid == running->pid)
			{
				hasChild = 1;
				if (p->status == DEAD)
				{	
					*status = p->exitCode;
					p->status = FREE;
					put_proc(&freeList, p);
					nproc--;

					return(p->pid);
				}
			}
		}
		if (!hasChild)
			return -1;

		ksleep(running);
	}
}

int copyImage (u16 pseg, u16 cseg, u16 size)
{
	u16 i;
	for (i = 0; i < size; i++)
		put_word (get_word(pseg, 2 * i), cseg, i * 2);
}

int umfork ()
{
	u16 segment;
	int i;
	//Fork a child without a file
	PROC *p = kfork(0);

	if (p == 0)
		return -1;

	//Copy the parent's image to the child's
	segment = (p->pid + 1) * 0x1000;
	copyImage (running->uss, segment, 32 * 1024);

	//Make sure the child goes back to its own bed
	p->uss = segment;
	p->usp = running->usp;

	//Set some register values
	put_word (segment, segment, p->usp);
	put_word (segment, segment, p->usp + 2);
	put_word (0, segment, p->usp + 2 * 8);
	put_word (segment, segment, p->usp + 2 * 10);

	//Copy the file descriptors
	for (i = 0; i < NFD; i++)
	{
		//Only the ones in use, though
		if (running->fd[i])
		{
			//Increment refCounts and such after copying it
			p->fd[i] = running->fd[i];
			p->fd[i]->refCount++;
			
			if (p->fd[i]->mode == 1) //1 == READ_MODE
				p->fd[i]->pipe_ptr->nreader++;
			if (p->fd[i]->mode == 2) //2 == WRITE_MODE
				p->fd[i]->pipe_ptr->nwriter++;
		}
	}

	return p->pid;
}

//Load and execute a given file
int kexec (char *file)
{
	int i, length = 0;
	char filename[64], *cp = filename;
	u16 segment = running->uss;

	while ( (*cp++ = get_byte (running->uss, file++)) && length++ < 64);

	if (!load(filename, segment))
		return -1;

	for (i = 1; i <= 12; i++)
		put_word (0, segment, -2 * i);

	running->usp = -24;

	put_word (segment, segment, -2 * 12);
	put_word (segment, segment, -2 * 11);
	put_word (segment, segment, -2 * 2);
	put_word (0x0200, segment, -2 * 1);	
}


