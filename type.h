/* type.h
	This file contains many definitions used throughout the rest of the code */

#define NPROC    9  
#define SSIZE 1024

/* PROC status */
#define FREE     0
#define READY    1
#define STOP     2
#define DEAD     3
#define SLEEP	 4
#define TIME	 5
#define BLOCKED  6

#define MTXSEG 0x1000
#define PSIZE	10
#define NPIPE	20
#define NOFT	20
#define NFD		20

//OFT modes
#define READ_MODE	1
#define WRITE_MODE	2

//Random error numbers
#define BROKEN_PIPE 5

//Timer Definitions
#define LATCH_COUNT 0x00
#define SQUARE_WAVE 0x36

#define TIMER_FREQ 1193182L
#define TIMER_COUNT ((unsigned) (TIMER_FREQ / 60))

#define TIMER0		0x40
#define TIMER_MODE	0x43
#define TIMER_IRQ	0

#define INT_CNTL	0x20
#define INT_MASK	0x21

//Video Definitions
#define VDC_INDEX	0x3D4
#define VDC_DATA	0x3D5
#define VID_BASE	0xB800

#define CURSOR_SIZE	0x0A
#define VID_ORG		0x0C
#define CURSOR		0x0E

#define LINE_WIDTH	80
#define SCR_LINES	25
#define SCR_BYTES	4000

#define CURSOR_SHAPE	15

#define HGREEN		0x0A
#define HCYAN		0x0B
#define HRED		0x0C
#define HPURPLE		0x0D
#define HYELLOW		0x0E

//Serial Driver definitions
#define INT_CTL		0x20
#define ENABLE		0x20

#define NULLCHAR	0
#define BEEP		7
#define BACKSPACE	8
#define ESC			27
#define SPACE		32

#define BUFLEN		64
#define LSIZE		64

#define NR_STTY		2 //Number of serial ports

//Serial ports offsets
#define DATA	0
#define DIVL	0
#define DIVH	1
#define IER		1
#define IIR		2
#define LCR		3
#define MCR		4
#define LSR		5
#define MSR		6

//Declare a few data types
typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

u16 offset;
u16 vid_mask = 0x3FFF;
int org, row, column;
int color;

typedef struct header{
	u32 ID_space;
	u32 magic_number;
	u32 tsize;
	u32 dsize;
	u32 bsize;
	u32 zero;
	u32 total_size;
	u32 symbolTable_size;
} HEADER;

typedef struct ext2_group_block{
	u32	bg_block_bitmap;
	u32	bg_inode_bitmap;
	u32 bg_inode_table;
	u16	bg_free_blocks_count;
	u16 bg_free_inodes_count;
	u16 bg_used_dirs_count;
	u16	bg_pad;
	u32	bg_reserved[3];
} GD;

typedef struct ext2_inode{
	u16	i_mode;
	u16	i_uid;
	u32 i_size;
	u32 i_atime;
	u32 i_ctime;
	u32 i_mtime;
	u32 i_dtime;
	u16 i_gid;
	u16 i_links_count;
	u32 i_blocks;
	u32 i_flags;
	u32 i_osdl;
	u32 i_block[15];
	u32 pad[7];
} INODE;

typedef struct ext2_dir_entry_2{
	u32	inode;
	u16 rec_len;
	u8	name_len;
	u8	file_type; 
	char	name[255];
} DIR;

typedef struct pipe
{
	char buf[PSIZE];
	int head;
	int tail;
	int data;
	int room;
	int nreader;
	int nwriter;
	int status;
}PIPE;

typedef struct oft 
{
	int mode;
	int refCount;

	struct pipe *pipe_ptr;
	
	//Stuff for regular files?
	//INODE *inodePtr;
	//long offset;
}OFT;

//Process struct definition
typedef struct proc{
	struct proc	*next;
	int		*ksp;
	int		uss, usp;
	int		inkmode;

	struct proc *parent;
	int		pid;              // add pid for identify the proc
	int		ppid;             // parent pid;
	int		status;           // status = FREE|READY|STOPPED|DEAD, etc
	int		event;		//event number
	int		priority;         // scheduling priority
	int		exitCode;
	char	name[32];
	int		time;
	OFT		*fd[NFD];

	int		kstack[1024];    // proc stack area
}PROC;

typedef struct semaphore
{
	int value;
	PROC *queue;
}SEMAPHORE;

typedef struct stty {

	//Input buffer
	char inbuf[BUFLEN];
	int inhead, intail; //Head is start of data, tail is end
	SEMAPHORE inchars;
	
	//Output buffer
	char outbuf[BUFLEN];
	int outhead, outtail; //Head is start of data, tail is end
	SEMAPHORE outspace;
	int tx_on;
	
	//Control variables
	char echo;
	char ison;
	char erase, kill, intr, quit, x_on, x_off, eof;
	
	//IO Port address
	int port;
}STTY;

int  procSize = sizeof(PROC);
