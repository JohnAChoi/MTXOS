/* semaphore.c
	This file contains functions for handling semaphores */

extern int do_tswitch();

/* Global Variables */
int SR;

/* Function Prototypes */
int enterCR (u8 *lock);
int exitCR (u8 *lock);
int P (SEMAPHORE *s);
int V (SEMAPHORE *s);
int SIGNAL (SEMAPHORE *s);
int BLOCK (SEMAPHORE *s);

/* Function Definitions */
int enterCR (u8 *slock)
{
	//printf ("In enterCR\n");
	SR = lock();
	while (*slock);
}

int exitCR (u8 *slock)
{
	//printf ("In exitCR\n");
	*slock = 0;
	unlock(SR);
}

int P (SEMAPHORE *s)
{
	//enterCR (&(s->lock));
	SR = int_off();
	s->value--;

	if (s->value < 0)
		BLOCK (s);
	else
		int_on(SR);// exitCR (&(s->lock));
	//printf ("Done with P\n");
}

int V (SEMAPHORE *s)
{
	//enterCR (&(s->lock));
	SR = int_off();
	s->value++;

	if (s->value <= 0)
		SIGNAL (s);
	
	//exitCR (&(s->lock));
	int_on(SR);
}

int BLOCK (SEMAPHORE *s)
{
	running->status = BLOCKED;
	enqueue (&(s->queue), running);
	tswitch();
	int_on(SR);
}

int SIGNAL (SEMAPHORE *s)
{
	PROC *p;
	p = dequeue (&(s->queue));

	if (!p)
		return 0;
	
	p->status = READY;
	enqueue (&readyQueue, p);
}
