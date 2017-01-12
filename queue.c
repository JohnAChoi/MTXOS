/* queue.c
	This file contains functions for using queues */

/* Function Prototypes */
int enqueue (PROC **queue, PROC *p);
int put_proc (PROC **list, PROC *p);
PROC *dequeue (PROC **queue);
PROC *get_proc (PROC **list);
void printQueue (PROC *queue);
void printList (char *name, PROC *list);

int enqueue (PROC **queue, PROC *p)
{
	PROC *current = *queue;
	PROC *prev = 0;

	//If the queue is empty, set the process passed in as the first element and return
	if (!*queue)
	{
		*queue = p;
		p->next = 0;
		return 1;
	}	

	//Iterate to the correct spot in the queue, based on priority
	while (current && p->priority <= current->priority)
	{
		prev = current;
		current = current->next;
	}

	if (prev)
		prev->next = p;
	else //Item is to be entered at the front of the queue, so the head of the queue must be changed
		*queue = p;

	p->next = current;

	return 1;
}

int put_proc (PROC **list, PROC *p)
{
	PROC *current = *list;

	//If the list is empty, add the new proc as the first
	if (!current)
	{
		*list = p;
		p->next = 0;
		return 1;
	}

	//Iterate to the end of the list
	while (current->next)
	{
		current = current->next;
	}

	current->next = p; //Add the new proc to the end of the list
	p->next = 0; //Set the added proc's next pointer to 0

	return 1;
}

PROC *dequeue (PROC **queue)
{
	PROC *deq = *queue;

	if (*queue) //Prevent a segfault by making sure there is something in the queue
	{
		*queue = deq->next;
		deq->next = 0;
	}

	return deq; //Will return 0 if the queue is empty
}

PROC *get_proc (PROC **list)
{
	PROC *proc = *list;
	PROC *prev = 0;

	if (!proc) //Return 0 if the list is empty
		return 0;

	while (proc) //Iterate through until the first FREE process is found
	{
		if (proc->status == 0)
			break;
		prev = proc;
		proc = proc->next;
	}

	if (prev) //Found the item somewhere in the middle of the list
		prev->next = proc->next;
	else //Removing the head of the list
		*list = proc->next;

	proc->next = 0;

	return proc;
}

void printQueue (PROC *queue)
{
	PROC *current = queue;

	while (current)
	{
		printf ("[%d, %d] -->", current->pid, current->priority);
		current = current->next;
	}

	printf ("NULL\n");
}

void printList (char *name, PROC *list)
{
	printf ("%s", name);
	printQueue(list);
}
