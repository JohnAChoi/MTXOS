/* timer.c
	This file contains the definition for thandler*/

/* Global Variables */
u16 tick;
u16 seconds, minutes, hours;

/* Function Prototypes */
int thandler();

int thandler()
{
	int offset, pos, w, i;
	PROC *p;
	tick++; 
	tick %= 60;
	
	if (tick == 0)
	{
		seconds++;
		running->time--;

		//Check seconds to update minutes
		if (seconds == 60)
		{
			minutes++;
			seconds = 0;
		}	

		//Check minutes to update hours
		if (minutes == 60)
		{
			hours++;
			minutes = 0;
		}	

		for (i = 0; i < NPROC; i++)
		{
			p = &proc[i];

			if (p->status == TIME)
			{
				p->time--;
				if (p->time == 0)
				{
					p->status = READY;
					enqueue (&readyQueue, p);
				}
			}
		}
	}
 
	if ((tick % 12) == 0){
		//Update the ones digit of the seconds
		pos = 2 * (24 * 80 + 79);
		offset = (org + pos) & vid_mask;
		w = (15 << 8) + (seconds % 10) + 48;
		put_word (w, VID_BASE, offset);

		//Update tens digit of the seconds
		pos -= 2;
		offset = (org + pos) & vid_mask;
		w = (15 << 8) + (seconds /10) + 48;
		put_word (w, VID_BASE, offset);

		//Put in a colon
		pos -= 2;
		offset = (org + pos) & vid_mask;
		w = (15 << 8) + ':';
		put_word (w, VID_BASE, offset);
		
		//Update ones digit of minutes
		pos -= 2;
		offset = (org + pos) & vid_mask;
		w = (15 << 8) + (minutes % 10) + 48;
		put_word (w, VID_BASE, offset);

		//Update tens digit of minutes
		pos -= 2;
		offset = (org + pos) & vid_mask;
		w = (15 << 8) + (minutes / 10) + 48;
		put_word (w, VID_BASE, offset);

		//Place a colon
		pos -= 2;
		offset = (org + pos) & vid_mask;
		w = (15 << 8) + ':';
		put_word (w, VID_BASE, offset);

		//Update ones digit of hours
		pos -= 2;
		offset = (org + pos) & vid_mask;
		w = (15 << 8) + (hours % 10) + 48;
		put_word (w, VID_BASE, offset);

		//Update tens digit of hours
		pos -= 2;
		offset = (org + pos) & vid_mask;
		w = (15 << 8) + (hours / 10) + 48;
		put_word (w, VID_BASE, offset);
	}
  out_byte(0x20, 0x20);                // tell 8259 PIC EOI
}

