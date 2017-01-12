/* io.c 
	This file contains functions for handling basic I/O functionality*/

#define BASE 10
#define BLKSIZE 1024

char *table = "0123456789ABCDEF";

extern PROC *running;

/* Function Prototypes */
int rpu (u16 x);
int printu (u16 x);
int printd (int x);
int rpl (u32 x);
int printl (u32 x);
int rpX (u32 x);
int printX (u32 x);
int rpo (u16 x);
int printo (u16 x);
int rpx (u16 x);
int printx (u16 x);
int prints (char *sp);
int printf (char *fmt, ...);
int gets (char s[]);

int rpu (u16 x)
{
	char c;
	if (x) 
	{
		c = table [x % BASE];
		rpu ( x / BASE );
		putc (c);
	}
}

int printu (u16 x)
{
	if ( x == 0 )
		putc('0');
	else 
		rpu (x);
	putc (' ');
}

int printd (int x)
{

	if ( x < 0)
	{
		x *= -1;
		putc ('-');
	}

	printu (x);
}

int rpl (u32 x)
{
	char c;
	if (x) 
	{
		c = table [x % BASE];
		rpl ( x / BASE );
		putc (c);
	}
}

int printl (u32 x)
{
	if ( x == 0 )
		putc('0');
	else 
		rpl (x);
	putc (' ');
}

int rpX (u32 x)
{
	char c;
	if (x) 
	{
		c = table [x % 16];
		rpX ( x / 16 );
		putc (c);
	}
}

int printX (u32 x)
{
	if ( x == 0 )
		putc ('0');
	else
		rpX (x);
	putc (' ');
}

int rpo (u16 x)
{
	char c;
	if (x) 
	{
		c = table [x % 8];
		rpo ( x / 8 );
		putc (c);
	}
}

int printo (u16 x)
{
	if ( x == 0 )
		putc('0');
	else
		rpo (x);
	putc (' ');
}

int rpx (u16 x)
{
	char c;
	if (x) 
	{
		c = table [x % 16];
		rpx ( x / 16 );
		putc (c);
	}
}

int printx (u16 x)
{
	if ( x == 0 )
		putc ('0');
	else
		rpx (x);
	putc (' ');
}

int prints (char *sp)
{
	char *p = sp;

	while (*p != 0)
	{
		putc (*p);
		p++;
	}
}

int printf (char *fmt, ...)
{
	int *ip, i;
	char *cp;

	ip = &fmt;
	ip++;
	
	cp = fmt;

	while (*cp != 0)
	{
		if (*cp == '\n')
		{
				putc ('\r');
				putc ('\n');
				cp++;
				continue;
		}

		if (*cp == '%')
		{
			switch (*(cp + 1))
			{
				case 'c':
					putc (*ip);
				break;
				case 's':
					prints ((char*)*ip);
				break;
				case 'u':
					printu (*ip);
				break;
				case 'd':
					printd (*ip);
				break;
				case 'o':
					printo (*ip);
				break;
				case 'x':
					printx (*ip);
				break;
				case 'l':
					printl (*(u32 *)ip++);
				break;
				case 'X':
					printX (*(u32 *)ip++);
				break;
			}
			ip++;
			cp += 2;
			continue;
		}
		putc (*cp);
		cp++;
	}
	return 1;
}

int gets (char s[])
{
	char next = 0; 
	int i = 0;

	while ((next = getc()) && next != '\r')
	{
		putc (next);
		s[i] = next;
		i++;
	}

	putc ('\r');
	putc ('\n');

	s[i] = 0;

	return 1;
}
