/* str.c
	This function contains many commonly-found string functions and a few
	slightly related memory manipulation functions */

/* Global Declarations */
char names[32][32];
char buf[BLKSIZE];
char sbuf[BLKSIZE];

char *totoken;
int tokloc;

/* Function Prototypes */
int strcpy(char *dest, char *source);
char *strtok (char *str, char *split);
int strlen (char *str);
u8 get_byte (u16 segment, u16 offset);
u16 get_word (u16 segment, u16 offset);
int put_byte (u8 byte, u16 segment, u16 offset);
int put_word (u16 word, u16 segment, u16 offset);
int get_block (u16 blk, int which);
int memcpy (u16 dest, u16 source, u16 size);
int loaddata (u16 dseg, u16 offset, char *buffer, u16 size);
int memset (u8 *target, u8 value, u16 size);

int strcpy(char *dest, char *source)
{
	while (*source != 0) //Copy all of the regular characters
		*dest++ = *source++;

	*dest = 0; //Copy the null character

	return 1;
}

char *strtok (char *str, char *split)
{
	char *tok = 0;
	int isp = 0;

	//If the user has passed in a new string to split up, set values accordingly
	if (str)
		totoken = str;

	//Mark the token to return
	tok = totoken;

	//If the actual end of the string has been reached, return 0
	if (!*tok)
		return 0;

	//Search through the string
	while (*totoken != 0)
	{
		isp = 0;

		//Check to see if the current character is any of the split characters
		while (split[isp] != 0)
		{
			//If it is, set it to null and break out of the loop
			if (split[isp] == *totoken)
			{
				if (totoken == tok) //If the first character is one of the delimiters, move past it
				{
					totoken++;
					tok++;
					continue;
				}

				*totoken = 0;
				totoken++; //Increment this so the next run is in the correct starting position

				return tok;
			}
			isp++;
		}
		//Go to the next character
		totoken++;
	}

	//Return the token
	return tok;
}

int strlen (char *str)
{
	int len;

	while (str[len++]);
		
	return len;
}

//For these functions, pass in the segment as the entire 0xX000 value, not an int
u8 get_byte (u16 segment, u16 offset)
{
	u8 byte;
	setds(segment);
	byte = *(u8 *)offset;
	setds(MTXSEG);

	return byte;
}

u16 get_word (u16 segment, u16 offset)
{
	u16 word;
	setds(segment);
	word = *(u16 *)offset;
	setds(MTXSEG);

	return word;
}

int put_byte (u8 byte, u16 segment, u16 offset)
{
	setds(segment);
	*(u8 *)offset = byte;
	setds(MTXSEG);
}

int put_word (u16 word, u16 segment, u16 offset)
{
	setds(segment);
	*(u16 *)offset = word;
	setds(MTXSEG);
}

//Temp fix: pass in int to determine which buffer to read the data into
//0 for buf, 1 for sbuf
int get_block (u16 blk, int which)
{
	//printf ("Getting block %d\n", blk);
	if (!which)
		diskr(blk/18, ((2 * blk) % 36) / 18, (((2 * blk) % 36) % 18), buf);
	else
		diskr(blk/18, ((2 * blk) % 36) / 18, (((2 * blk) % 36) % 18), sbuf);
}

//Assumes two pointers in the same segment, namely the running proc's
int memcpy (u16 dest, u16 source, u16 size)
{
	int i;

	//If the destination is after the source, copy from the end to prevent loss of data
	if (dest > source)
	{
		for (i = size - 1; i >= 0; i--)
			put_byte(get_byte(running->uss, dest + i), running->uss, source + i);
	}
	//Otherwise, copy from the beginning to prevent loss of data
	else
	{
		for (i = 0; i < size; i++)
			put_byte(get_byte(running->uss, dest + i), running->uss, source + i);
	}
}

//Returns the amount of data loaded. CALLER MUST MAKE SURE SIZE DOES NOT GO BEYOND BUFFER
int loaddata (u16 dseg, u16 offset, char *buffer, u16 size)
{
	int i;
	for (i = 0; i < size; i++)
		put_byte (*buffer++, dseg, offset++);

	return i;
}

//This function does not check the buffer length
int memset (u8 *target, u8 value, u16 size)
{
	int i;
	for (i = 0; i < size; i++)
		target[i] = value;
	
	return 1;
}
