#include <stdio.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define SUPERBLOCK_MAGIC_NUMBER 0xEF53
#define BLKSIZE 1024
#define SUPERBLK 1
#define GDBLK 2
#define FILE_NAME_MAX_SIZE 256

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc GD;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

void get_block(int fd, long blk, char buf[])
{
    lseek(fd, blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int super(char buf[], int fd)
{
  // read SUPER block
  get_block(fd, 1, buf);  
  sp = (SUPER *)buf;

  printf("s_inodes_count = %d\n", sp->s_inodes_count);
  printf("s_blocks_count = %d\n", sp->s_blocks_count);

  printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
  printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);
  printf("s_first_data_blcok = %d\n", sp->s_first_data_block);

  printf("s_log_block_size = %d\n", sp->s_log_block_size);
  printf("s_blocks_per_group = %d\n", sp->s_blocks_per_group);
  printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);

  printf("s_mnt_count = %d\n", sp->s_mnt_count);
  printf("s_max_mnt_count = %d\n", sp->s_max_mnt_count);

  printf("s_magic = %x\n", sp->s_magic);
}

int gd(int fd)
{
    char buf[BLKSIZE];
    lseek(fd, (long)GDBLK * BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
    GD *gp = (GD*)buf;

    printf("bg_block_bitmap: %u\n", gp->bg_block_bitmap);
    printf("bg_inode_bitmap: %u\n", gp->bg_inode_bitmap);
    printf("bg_inode_table: %u\n", gp->bg_inode_table);
    printf("bg_free_blocks_count: %u\n", gp->bg_free_blocks_count);
    printf("bg_free_inodes_count: %u\n", gp->bg_free_inodes_count);
    printf("bg_used_dirs_count: %u\n", gp->bg_used_dirs_count);
}

void print_direct_blocks(char dbuf[], const INODE *ip, int numblk)
{
    int line = 0;
    for(int i = 0; i < 12; i++)
    {
        if(ip->i_block[i] == 0)
            return;
        printf("%d ", ip->i_block[i]);
        if(++line == 10)
        {
            printf("\n");
            line = 0;
        }
	numblk--;
    }
	numblk++;
    printf("\nRemaining blocks: %d\n", numblk);
    printf("\n");
}

void print_indirect_blocks(char dbuf[], int numblk)
{
    int line = 0;
    for(char* cur = dbuf; cur < dbuf + BLKSIZE; cur += 4)
    {
        uint32_t val = *((uint32_t*)cur);
        if(val == 0)
        {
            printf("\n");
	    printf("\nRemaining blocks: 0\n");
            return;
        }
        printf("%d ", val);
        if(++line == 10)
        {
            printf("\n");
            line = 0;
        }
	numblk--;
    }
    numblk-=11;
    printf("\nRemaining blocks: %d\n", numblk);
    printf("\n");
}

void print_double_indirect_blocks(int fd, char dbuf[], int numblk)
{
    for(char* cur = dbuf; cur < dbuf + BLKSIZE; cur += 4)
    {
        uint32_t val = *((uint32_t*)cur);
        if(val == 0)
        {
            printf("\n");
            return;
        }
        char dbuf2[BLKSIZE];
        get_block(fd, val, dbuf2);
        print_indirect_blocks(dbuf2, numblk);
	numblk--;
    }
    printf("\n");
}

void print_blocks(int fd, const INODE *ip)
{
    for(int i = 0; i < 14; i++)
    {
        if(ip->i_block[i] == 0)
            break;
        printf("i_block[%d] = %d\n", i, ip->i_block[i]);
    }
    
    int numblk = ip->i_size / BLKSIZE;
    char dbuf[BLKSIZE];

    printf("================ Direct Blocks ===================\n");
    print_direct_blocks(dbuf, ip, numblk);
    if(ip->i_block[12] != 0)
    {
        get_block(fd, ip->i_block[12], dbuf);
        printf("===============  Indirect blocks   ===============\n");
        print_indirect_blocks(dbuf, numblk);
    }

    if(ip->i_block[13] != 0)
    {
        get_block(fd, ip->i_block[13], dbuf);
        printf("===========  Double Indirect blocks   ============\n");
        print_double_indirect_blocks(fd, dbuf, numblk);
    }
}

int search(int dev, const INODE *ip, const char * name)
{
     printf("i_number\trec_len\t\tname_len\tname\n");
    char dbuf[BLKSIZE], temp[256];
    DIR* dp;
    char* cp;
    for(int i = 0; i < 12; i++)
    {
        if(ip->i_block[i] == 0)
            break;
        get_block(dev, ip->i_block[i], dbuf);
        cp = dbuf;
        dp = (DIR*)dbuf;


        while(cp < dbuf + BLKSIZE)
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = '\0';
            printf("%4d\t\t%4d\t\t%4d\t\t%s\n",dp->inode, dp->rec_len, dp->name_len, temp);
            if(strcmp(name, temp) == 0)
                return dp->inode;
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    
    return 0;
}

int main(int argc, char* argv[])
{
    //check arg
    if(argc != 3)
    {
        printf("Usage: ./a.out diskimage pathname\n");
        return 0;
    }

    //get device
    int dev = open(argv[1], O_RDONLY);
    if(dev < 0)
    {
        printf("Failed to open device %s\n", argv[1]);
        return 1;
    }

    //get super
    printf("************  super block info: *************\n");
    char buf[BLKSIZE];
    get_block(dev, SUPERBLK, buf);
    if(((SUPER*)buf)->s_magic != SUPERBLOCK_MAGIC_NUMBER)
    {
        printf("ERROR: Incorrect filesystem type.\n");
        return 1;
    }
    super(buf,dev);
    //get gd0
    printf("************  group 0 info ******************\n");
    gd(dev);

    //get inode
    printf("***********  root inode info ***************\n");
    get_block(dev, GDBLK, buf);
    int iblk = ((GD*)buf)->bg_inode_table;
    get_block(dev, iblk, buf);
    ip = (INODE *)buf + 1;         // ip points at 2nd INODE
    printf("mode=0x%4x\n", ip->i_mode);
    printf("size=%d\n", ip->i_size);
    printf("link=%d\n", ip->i_links_count);
    printf("i_block[0]=%d\n", ip->i_block[0]);

    //print all root dir
    get_block(dev, 2, buf);
    GD* gp = (GD*)buf;
    get_block(dev, gp->bg_inode_table, buf);
    ip = (INODE*) buf + 1;
        unsigned int block0 = ip->i_block[0];
        get_block(dev, block0, buf);
        char* cp = buf;
        DIR* dp = (DIR*)buf;
	char temp[256];

    printf("********* root dir entries ***********\n");
     printf("i_number\trec_len\t\tname_len\tname\n");
    	while(cp < buf + BLKSIZE)
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = '\0';
            printf("%4d\t\t%4d\t\t%4d\t\t%s\n",
                dp->inode, dp->rec_len, dp->name_len, temp);
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }

    //reset root dir
    printf("Press Enter to Continue\n");
    getchar();
    get_block(dev, GDBLK, buf);
    iblk = ((GD*)buf)->bg_inode_table;
    get_block(dev, iblk, buf);
    INODE* ip = (INODE*)buf + 1;
    printf("mode=%x ", ip->i_mode);
    printf("uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);



    //search for inode
    char current_dir[FILE_NAME_MAX_SIZE] = "/";
    char * path = argv[2];
    if(path[0] == '/')
        path++;
    char * tok = strtok(path, "/");
    int num;
    do
    {
        printf("===========================================\n");
        printf("Searching %s in %s \n", tok, current_dir);
        num = search(dev, ip, tok);
        if(num == 0)
        {
            printf("NOT FOUND\n");
            return 0;
        }

        strcpy(current_dir, tok);
        
        int blk = (num - 1) / 8 + iblk;
        int offset = (num - 1) % 8;
        get_block(dev, blk, buf);
        ip = (INODE*)buf + offset;
    	printf("Found %s: ino = %d\n", current_dir, num);
    }while(tok = strtok(NULL, "/"));

    printf("****************  DISK BLOCKS  *******************\n");
    print_blocks(dev, ip);
    return 0;
}
