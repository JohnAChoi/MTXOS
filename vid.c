/* vid.c
	This file contains functions for handling the video drivers */

//int org, row, column;

/* Function Prototypes */
int scroll ();
int putc (char c);
int set_VDC (u16 reg, u16 val);

int scroll ()
{
	int i;
	u16 w, bytes, cpos, coff;

	//Remove the timer when scrolling so it appears in the correct position
	for (i = 0; i < 8; i++)
	{
		cpos = 2 * (24 * 80 + (72 + i));
		coff = (org + cpos) & vid_mask;
		put_word (0x0C00, VID_BASE, coff);
	}
	
	//Position the offset to be the first line of the next screen
	offset = org + SCR_BYTES + 2*LINE_WIDTH;

	//If it's still within the range of the VRAM, move the origin down
	if (offset <= vid_mask)
		org += 2 * LINE_WIDTH;
	//Otherwise, copy the rest of the VRAM to the start of the VRAM and move back up
	else
	{
		for (i = 0; i < 24 * 80; i++)
		{
			w = get_word (VID_BASE, org + 160 + 2 * i);
			put_word (w, VID_BASE, 2 * i);
		}

		org = 0;
	}

	//Set the offset to the last line onscreen
	offset = org + 2 * 24 * 80;

	w = 0x0C00;

	//Create a blank line to scroll to
	for (i = 0; i < 80; i++)
		put_word (w, VID_BASE, offset + 2 * i);

	//Move the origin of the VRAM to the new location
	set_VDC (VID_ORG, org >> 1);
}

int putc (char c)
{
	u16 pos, w, offset;

	//Handle newlines
	if (c == '\n')
	{
		row++;

		if (row >= 25)
		{
			row = 24;
			scroll();
		}

		pos = 2 * (row * 80 + column);
		offset = (org + pos) & vid_mask;
		set_VDC (CURSOR, offset >> 1);
		
		return 1;
	}
	
	//Move the cursor in response to a carriage return
	if (c == '\r')
	{
		column = 0;
		pos = 2 * (row * 80 + column);
		offset = (org + pos) & vid_mask;
		set_VDC (CURSOR, offset >> 1);
		return 1;
	}

	//Handle a backspace key
	if (c == '\b')
	{
		if (column > 0)
		{
			column--;
			pos = 2 * (row * 80 + column);
			offset = (org + pos) & vid_mask;
			set_VDC (CURSOR, offset >> 1);
			put_word (0x0700, VID_BASE, offset);
		}
		return 1;
	}

	//Handle a tab
	if (c == '\t')
	{
		column = column - (column % 4) + 4;
		pos = 2 * (row * 80 + column);
		offset = (org + pos) & vid_mask;
		set_VDC (CURSOR, offset >> 1);
		return 1;	
	}

	//With the special cases out of the way, write a normal character to the screen
	pos = 2 * (row * 80 + column);
	offset = (org + pos) & vid_mask;
	w = (color << 8) + c;
	put_word (w, VID_BASE, offset);
	column++;

	//Handle writing off the edges of the screen
	if (column >= 80)
	{
		column = 0;
		row++;

		if (row >= 25)
		{
			row = 24;
			scroll();
		}
	}

	//Reposition the cursor
	pos = 2 * (row * 80 + column);
	offset = (org + pos) & vid_mask;
	set_VDC (CURSOR, offset >> 1);
}

int set_VDC (u16 reg, u16 val)
{
	lock();
	out_byte (VDC_INDEX, reg);
	out_byte (VDC_DATA, (val >> 8) & 0xFF);
	out_byte (VDC_INDEX, reg + 1);
	out_byte (VDC_DATA, val & 0xFF);
	unlock();
}
