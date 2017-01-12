/* filesys.c
	This file contains functions needed to load data from the disk to RAM to use*/

/* Function Prototypes */
int tokenize (char *pathname);
int search_dir_block (int dir_block, int dir_name);
int getino (char* pathname);
int load (char *filename, u16 segment);

//Take the path from the user and split it into each directory and the file
//Does NOT check if the user is starting from root
//Also does not mess with the original string passed in
//Returns the number of times the pathname was tokenized, which is the number of directories
int tokenize (char *pathname)
{
	char copy[128];
	char *tok;
	int i = 0;

	strcpy (copy, pathname);

	tok = strtok(copy, "/");
	strcpy (names[i], tok);

	while (tok = strtok (0, "/"))
		strcpy(names[++i], tok);

	names[++i][0] = 0;

	return i;
}

//Look through the data blocks of a directory inode to search for 
//a given filename.
//Takes the index of the directory block to look through, and the 
//index of the directory name in the paths array
int search_dir_block (int dir_block, int dir_name)
{
	DIR *dp = (DIR *)sbuf;
	char *cp = sbuf;
	char n;

	//Grab the directory block from the file system
	get_block ( (u16) dir_block, 1); //sbuf);
	
	//Search through the directory block (buffer)
	while (cp < sbuf + BLKSIZE)
	{
		//First, take the directory name recorded in the directory entry
		// and create a null-terminated copy of it
		n = dp->name[dp->name_len];
		dp->name[dp->name_len] = 0;

		//Check to see if the entry name matches the directory name we're looking for
		if (strcmp (dp->name, names[dir_name]) == 0)
		{
			//If it does, free the created directory name and return the inode number
			dp->name[dp->name_len] = n;
			return dp->inode;
		}

		//Restore the value to the dir entry
		dp->name[dp->name_len] = n;

		//Otherwise, continue looking through the directory block
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}	

	//Return an error if the code gets here because it never found the name it was looking for.
	return -1;
}

//Grabs an Inumber from a pathname
int getino (char* pathname)
{
	INODE *cip;
	GD *gp;
	int i, j, cur_block, next_block;
	int inodeBlock;
	int num_names = tokenize (pathname); //Tokenize the pathname
	
	get_block ( (u16) 2, 0);//buf);
	gp = (GD *)buf;
	inodeBlock = gp->bg_inode_table; //Grab the inode table location

	get_block ( (u16) inodeBlock, 0);//buf);
	cip = (INODE *)buf + 1; //Point cip at the root inode
	
	//Search for each directory name in order
	for (i = 0; i < num_names; i++)
	{
		//Look through each directory entry in the directory
		for (j = 0; j < 12; j++)
		{
			//Grab the block number of the directory entry
			cur_block = cip->i_block[j];

			//If it is zero, there are no more entries to look through so stop iterating
			if (cur_block == 0)
			{
				j = 12;
				continue;
			}

			//Search through the directory block for the current path name
			next_block = search_dir_block ( cur_block, i);

			//If the next directory was found, grab its inode and start searching through it
			if (next_block > -1)
			{
				cur_block = next_block;
				
				get_block ( (u16) ((cur_block - 1) / 8) + inodeBlock, 0); //buf);
				cip = (INODE *) buf + ((cur_block - 1) % 8);
				j = 12;
			}
		}
		
		//If a file was found and it should be a path, return an error and let the user know
		if (i < num_names - 1 && (cip->i_mode & 0x4000) == 0) //Check if directory or not
		{
			printf ("File found where directory expected.\n");
			return -1;
		}

		//If the directory was not found, return an error
		if (next_block < 0)
		{
			printf ("Invalid path\n");
			return -1;
		}
	}

	//The specified file was found: return its inode number.
	return cur_block;
}

//Load the specified file to a given segment
int load (char *filename, u16 segment)
{
	//Grab the inode number of the file
	u16 ino = getino (filename);
	u16 inodeBlock, indirect;
	u16 i = 0, offset = 0, size, total_size, b_size;
	u32 *blockptr = (u32) buf;

	INODE *ip;
	GD *gp;
	struct header *hp;

	//If inode < 3, then something went wrong
	if (ino < 3)
	{
		printf ("Error: File not found %s\n", filename);
		return -1;
	}

	//Clear out the buffers to prevent funny stuff from happening
	memset (buf, 0, BLKSIZE);
	memset (sbuf, 10, BLKSIZE);

	//Grab the group descriptor block
	get_block( (u16)2, 0);//buf);
	gp = (GD *)buf;

	//Grab the inode table location
	inodeBlock = gp->bg_inode_table;

	//Grab the inode block with the file and point ip to the correct inode.
	get_block ((u16) ((ino - 1) / 8) + inodeBlock, 0); //buf);
	ip = (INODE *)buf + ((ino - 1) % 8);

	//Grab the indirect block number because the inode will be lost soon
	indirect = ip->i_block[12];

	//Grab the first data block
	get_block ( (u16)ip->i_block[0], 1); //sbuf);

	//Point a header struct pointer at the beginning of it
	hp = (struct header *)sbuf;

	//Calculate the total size needed and the size of the bss
	total_size = hp->tsize + hp->dsize;
	b_size = hp->bsize;

	//Slightly different case for first block: Don't load in first 32 bytes of the buffer this time
	if (total_size <= 1024 - 32)
	{
		offset += loaddata (segment, offset, sbuf + 32, total_size);
		total_size = 0;
	}
	else
	{
		offset += loaddata (segment, offset, sbuf + 32, 1024 - 32);
		total_size -= (1024 - 32);
	}
	
	//Already loaded the first block, so move on to the other blocks	
	i++;
	
	//Load data until all of the code and data segments have been loaded	
	while (total_size > 0)
	{
		//Calculate size of data to load
		if (total_size > 1024)
			size = 1024;
		else
			size = total_size;

		//Grab the block of data to work
		if (i < 12)
			get_block ( (u16)ip->i_block[i], 1); //sbuf
		//Indirect blocks
		else if (i >= 12 && i < 268)
		{
			//Grab the indirect block
			get_block ( indirect, 0); //buf

			//Grab the data block
			get_block ( (u16)*(blockptr + (i - 12)), 1); //sbuf
		}

		//Load the data into the segment at the offset
		offset += loaddata (segment, offset, sbuf, size);

		total_size -= size;
		i++;
	}

	//Clear out buf to use to blank out the bss section
	memset (buf, 0, 1024);

	while (b_size > 0)
	{
		if (b_size > 1024)
			size = 1024;
		else
			size = b_size;

		offset += loaddata (segment, offset, buf, size);
		b_size -= size; 
	}

	return 1;		
}
