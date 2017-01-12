/* pipe.c
	This file contains definitions for piping functions*/

/* Function Prototypes */
int kpipe (int pd[2]);
int close_pipe (int fd);
int read_pipe (int fd, char *buf, int n);
int write_pipe (int fd, char *buf, int n);
int pfd();
//int show_pipe(PIPE *p);

int kpipe (int pd[2])
{
	int i, j, k;
	PIPE *p = &pipe[0]; OFT *o = &oft[0];
	
	//Find the first empty pipe
	for (i = 0; i < NPIPE; i++)
	{
		if (p->status == 0)
			break;
		p++;
	}

	//Initialize the pipe values
	p->head = p->tail = p->data = 0;
	p->room = PSIZE;
	p->nreader = p->nwriter = 1;
	p->status = 1; //1 = pipe in use
	
	//Find the first empty OFT
	for (j = 0; j < NOFT; j++)
	{
		if (o->refCount == 0)
			break;
		o++;
	}

	//Initialize it for use
	o->refCount = 1;
	o->mode = READ_MODE;
	o->pipe_ptr = p;
		
	//Find the first open file descriptor
	for (k = 0; k < NFD; k++)
		if (running->fd[k] == 0)
			break;

	//Let the running proc know which end is the reader
	running->fd[k] = o;
	//pd[0] = k;
	put_word (k, running->uss, pd);

	//Find a second empty OFT
	for (; j < NOFT; j++)
	{
		if (o->refCount == 0)
			break;
		o++;
	}

	o->refCount = 1;
	o->mode = WRITE_MODE; 
	o->pipe_ptr = p;

	//Find the next open file descriptor
	for (; k < NFD; k++)
		if (running->fd[k] == 0)
			break;

	//Let the running proc know which end is the writer
	running->fd[k] = o;
	put_word (k, running->uss, pd+1);

	return 0;
}

int close_pipe (int fd)
{
	OFT *o = running->fd[fd];
	PIPE *p;
	//Check to see that the file descriptor is actually open
	if (o == 0)
		return -1;

	p = o->pipe_ptr;
	o->refCount--;

	//If it's the writer end
	if (o->mode == 2) //2 == WRITE_MODE
	{
		//Decrement the number of writers to the pipe
		p->nwriter--;
		//If it was the last writer, deallocate the OFT
		if (p->nwriter == 0) 
		{
			o->mode = 0;
			o->refCount = 0;
			o->pipe_ptr = 0;

			//If there are no readers, deallocate the pipe as well
			if (p->nreader == 0)
			{
				memset (p->buf, 0, PSIZE);
				p->head = p->tail = p->data = p->room = 0;
				p->status = 0;
			}
		}
		kwakeup(p->data);
	}
	//It must be a reader. Same as above, but for the reader end
	else 
	{
		p->nreader--;
		
		if (p->nreader == 0)
		{
			o->mode = 0;
			o->refCount = 0;
			o->pipe_ptr = 0;
		
			if (p->nwriter == 0)
			{
				memset (p->buf, 0, PSIZE);
				p->head = p->tail = p->data = p->room = 0;
				p->status = 0;
			}
		}
		kwakeup(p->room);
	}

	running->fd[fd] = 0;
	return 0;
}

int read_pipe (int fd, char *buf, int n)
{
	int r = 0;
	PIPE *p;
	OFT *o = running->fd[fd];
	
	if (n <= 0)
		return 0;
	
	if (!o)
	{
		printf ("Error: Invalid FD specified\n");
		return 0;
	}
	
	if (o->mode != 1)
	{
		printf ("Error: Cannot read from a FD in write mode\n");
		return 0;
	}

	p = o->pipe_ptr;

	while (n)
	{
		while (p->data)
		{
			//*buf = p->buf[p->head];
			put_byte (p->buf[p->head], running->uss, buf + r);
			n--; r++; 
			p->data--; p->room++; p->head++;

			if (p->head == PSIZE)
				p->head = 0;
		
			if (n == 0)
				break;
		}
	
		if (p->nwriter && n > 0)	
		{
			kwakeup(&(p->room));
			ksleep(&(p->data));
			//kwakeup(p);
			//ksleep(p);
			continue;
		}

		if (r)
		{
			kwakeup(&(p->room));
			//kwakeup(p);
			return r;
		}

		return 0;
	}
}

int write_pipe (int fd, char *buf, int n)
{
	int r = 0; 
	PIPE *p;
	OFT *o = running->fd[fd];
	
	if (n <= 0)
		return 0;
	
	if (!o)
	{
		printf ("Error: Invalid FD specified\n");
		return 0;
	}
	
	if (o->mode != 2)
	{
		printf ("Error: Cannot write to a FD in read mode\n");
		return 0;
	}

	p = o->pipe_ptr;

	while (n)
	{
		if (!p->nreader)
			kexit (5); //EXIT WITH 5 BECAUSE I DECIDED IT'D BE BROKEN PIPE ERROR

		while (p->room)
		{
			p->buf[p->tail] = get_byte (running->uss, buf + r);//*buf;
			r++; n--;
			p->data++; p->room--; p->tail++;

			if (p->tail == PSIZE)
				p->tail = 0;
	
			if (n == 0)
				break;
		}

		kwakeup(&(p->data));
		//kwakeup(p);

		if (n == 0)
		{
			//tswitch();
			return r;
		}

		ksleep (&(p->room));
		//ksleep (p);
	}
}

int pfd()
{
	int i;
	OFT* o = &oft[0];	

	printf ("fd\t type\trefCount  mode (1: READ, 2: WRITE)\n");
	for (i = 0; i < NOFT; i++)
	{
		//If the pipe is in use, print out its info
		if (running->fd[i]->refCount > 0)
		{
			printf ("%d\tPipe\t%d\t%d\n", i,(o+i)->refCount, (o+i)->mode);
		}
	}
}

/*int show_pipe(PIPE *p)
{
	u16 i;

	printf ("Pipe details: \n");
	printf ("Head   Tail   Data   Room   nreader   nwriter   status\n");
	printf ("%d\t%d\t%d\t%d\t", p->head, p->tail, p->data, p->room);
	printf ("%d\t%d\t%d\n", p->nreader, p->nwriter, p->status);

	printf ("Contents:\n");
	
	//printf ("%d: %c\n", 0, p->buf[0]);
	//printf ("%d: %c\n", 1, p->buf[1]);
	//printf ("%d: %c\n", 2, p->buf[2]);
	
	for (i = 0; i < 20;i++)
		printf ("%d: %c\n", i, p->buf[i]);

	printf ("\n");
}*/
