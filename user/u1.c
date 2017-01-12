#include "ucode.c"
//int color;

main()
{ 
  char name[64]; 
	int pid, cmd;
	int i = 0;

  while(1){
	i = 0;
    pid = getpid();
    //color = pid + 10;
       
    printf("\n----------------------------------------------\n");
    printf("I am proc %d in U mode: running segment=%x\n",getpid(), getcs());
	/*while (i < 100000)
	{
		//printf ("Stalling for time: %d\n", i);
		i++;

		if (!ktime())
		{
			kswitch();
			break;
		}
	}*/
    show_menu();
    printf("Command ? ");
    gets(name); 
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    switch(cmd){
			case 0 : getpid();			break;
			case 1 : ps();				break;
			case 2 : chname();			break;
			case 3 : kfork();			break;
			case 4 : kswitch();			break;
			case 5 : wait();			break;
			case 6 : ufork();			break;
			case 7 : uexec();			break;
			case 8 : kmode();			break;
			case 9 : pipe();			break;
			case 10: read();			break;
			case 11: write();			break;
			case 12: close();			break;
			case 13: pfd();				break;
			case 14: sleep();			break;
			case 15: chcolor();			break;
			case 16: sgets();			break;
			case 17: sputs();			break;
			case 18: exit();			break;

           default: invalid(name); break;
    }
  }
}
