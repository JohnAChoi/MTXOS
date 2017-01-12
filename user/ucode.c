// ucode.c file

char *cmd[]={"getpid", "ps", "chname", "kfork", "switch", "wait", "fork", "exec", "kmode","pipe", "read", "write", "close", "pfd", 
			 "sleep", "color", "sgets", "sputs", "exit", 0};

#define LEN 64

int show_menu()
{
	printf("*********************** Menu ************************\n");
	printf("* ps chname kfork switch  wait fork exec kmode pipe *\n");
	printf("* read write close pfd sleep color sgets sputs exit *\n"); 
	printf("*****************************************************\n");
}

int find_cmd(char *name)
{
  // return command index
	int i = 0;
	while(cmd[i] != 0)
	{
		if (strcmp(name, cmd[i]) == 0)
			return i;
		i++;
	}

	return -1;
}

int getpid()
{
   return syscall(0,0,0);
}

int ps()
{
   return syscall(1, 0, 0);
}

int chname()
{
    char s[32];
    printf("input new name : ");
    gets(s);
    return syscall(2, s, 0);
}

int kfork()
{   
  int child, pid;
  pid = getpid();
  printf("proc %d enter kernel to kfork a child\n", pid); 
  child = syscall(3, 0, 0);
  printf("proc %d kforked a child %d\n", pid, child);
}    

int kswitch()
{
    return syscall(4,0,0);
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n"); 
} 

int geti()
{
	char buf[32];

	gets(buf);

	return atoi(buf);		
	// return an input integer
}

int exit()
{
   int exitValue;
   printf("enter an exitValue: ");
   exitValue = geti();
   printf("exitvalue=%d\n", exitValue);
   printf("enter kernel to die with exitValue=%d\n", exitValue);
   _exit(exitValue);
}

int _exit(int exitValue)
{
  return syscall(6,exitValue,0);
}

int invalid(char *name)
{
    printf("Invalid command : %s\n", name);
}

int getchar ()
{
	return syscall (97, 0, 0, 0);
}

int putc (char c)
{
	syscall (98, c, 0,0, 0);
}

int putchar (char data)
{
	syscall (98, data, 0, 0);
	//syscall (98, '\n', 0);
}

int fork()
{
	return syscall (7, 0, 0, 0);
}

int exec (char *s)
{
	return syscall (8, s, 0, 0);
}

int ufork ()
{
	int child = fork();
	
	(child) ? printf ("Parent ") : printf ("Child ");
	
	printf ("%d returns from fork, child_pid = %d\n", getpid(), child);
}

int uexec ()
{
	int r;
	char filename[64];

	printf ("Enter exec filename: ");
	gets (filename);
	
	r = exec (filename);
	printf ("Exec failed\n");
}

int pipe ()
{
	int pd[2];

	syscall (9, pd, 0, 0);

	printf ("Opened FDs: [%d %d]\n", pd[0], pd[1]);
}

int read ()
{
	int fd, nb, br = 0; char buf[32];

	memset(buf, 0, 20);

	printf ("FD to read from: ");
	fd = geti();

	printf ("\nNumber of bytes to read: ");
	nb = geti();

	br = syscall (10, fd, buf, nb);

	printf ("\n%d: %s\n", br, buf);	
}

int write ()
{
	int fd, nb; char buf[32];

	printf ("FD to write to: ");
	fd = geti();
	
	printf ("\nNumber of bytes to write: ");
	nb = geti();

	printf ("\nData to write: ");
	gets (buf);

	syscall (11, fd, buf, nb);
	
	printf ("\n");
}

int close()
{
	int fd;

	printf ("Enter FD to close: ");
	fd = geti();

	syscall (12, fd, 0, 0);
	printf ("\n");
}

int pfd ()
{
	syscall (13, 0, 0, 0);
}

int kmode ()
{
	syscall (14, 0, 0, 0);
}

int ktime()
{
	return syscall (15, 0, 0, 0);
}

int sleep ()
{
	int time;
	
	printf ("Enter time to sleep for (in seconds): ");
	time = geti();

	return syscall (16, time, 0, 0);
}

int strcmp (char *first, char *second)
{
	while (*first && *second)
	{
		if (*first > *second)
			return 1;
		else if (*first < *second)
			return -1;
		first++;
		second++;
	}

	if (*first != 0)
		return 1;
	else if (*second != 0)
		return -1;

	return 0;
}

int chcolor ()
{
	char c;
	printf ("Red, Yellow, Green, Purple, Cyan? ");
	c = getchar();
	
	return syscall (17, c, 0, 0);
}

int sgets ()
{
	char buf[64];
	int port = 2;

	printf ("Which port to read the message from? (0 or 1)\n");

	while (port != 0 && port != 1)
		port = geti();
	
	syscall (18, port, buf, 0);

	printf ("Message from port: \n");
	printf ("%s\n", buf);

	return 0;
}

int sputs ()
{
	char buf[64];
	int port = 2;
	
	printf ("Which port to send the message to? (0 or 1)\n");

	while (port != 0 && port != 1)
		port = geti();

	printf ("What message to send?\n");

	gets(buf);

	syscall (19, port, buf, 0);
	
	return 0;
}
