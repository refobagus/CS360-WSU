#pragma once
/*	type.h for CS360 Project             */
// define shorter TYPES, save typing efforts
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

#define BLOCK_SIZE        1024
#define BLKSIZE           1024
#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define BBITMAP           3
#define IBITMAP           4
#define INODEBLOCK        5
#define ROOT_INODE        2

// Default dir and regulsr file modes
#define DIR_MODE          0040777
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

// Modes for Opening file
#define R 0
#define W 1
#define RW 2
#define APPEND 3

// Proc status
#define FREE              0
#define READY             1
#define RUNNING           2

// Table sizes
#define NMINODES          50
#define NMOUNT            10
#define NPROC             10
#define NFD               10
#define NOFT              50

// Open File Table
typedef struct oft{
  int   mode;
  int   refCount;
  struct minode *inodeptr;
  int   offset;
} OFT;

// PROC structure
typedef struct proc{
  int   uid;
  int   pid;
  int   gid;
  int   ppid;
  int   status;

  struct minode *cwd;
  OFT   *fd[NFD];

  struct Proc *next;
  struct Proc *parent;
  struct Proc *child;
  struct Proc *sibling;
} PROC;

// In-memory inodes structure
typedef struct minode{
  INODE    INODE;               // disk inode
  int      dev, ino;

  int      refCount;
  int      dirty;
  int      mounted;
  struct mount *mountptr;
  char     name[128];           // name string of file
} MINODE;

// Mount Table structure
typedef struct mount{
  int  ninodes;
  int  nblocks;
  int  bmap;
  int  imap;
  int  iblock;
  int  dev, busy;   
  MINODE *mounted_inode;
  char   name[256]; 
  char   mount_name[64];
} MOUNT;

//Global Variables
MINODE minode[NMINODES];
PROC *running, proc[NPROC], *readQueue;
MINODE *root;

MOUNT MountTable[NMOUNT];
int inodeBegin;
char pathname[128];
int  fd, dev;
int  nblocks, ninodes, bmap, imap;
char line[256], cmd[32];
