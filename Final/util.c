#include "util.h"

//return block in buf
int get_block(int dev, int blk, char *buf)
{
  if (-1 == lseek(dev, (long)(blk*BLKSIZE), 0))
  {
    printf("%s\n", strerror(errno));
    assert(0);
  }
  read(dev, buf, BLKSIZE);
}

int put_block(int dev, int blk, char *buf)
{
  if (-1 == lseek(dev, (long)(blk*BLKSIZE), 0)){ assert(0);}
  write(dev, buf, BLKSIZE);
  return 1;
}
//token pathname into str
char** tokenPath(char* pathname)
{
  int i = 0;
  char** name;
  char* tmp;
  name = (char**)malloc(sizeof(char*)*256);
  name[0] = strtok(pathname, "/");
  i = 1;
  while ((name[i] = strtok(NULL, "/")) != NULL) { i++;}
  name[i] = 0;
  i = 0;
  while(name[i])
  {
    tmp = (char*)malloc(sizeof(char)*strlen(name[i]));
    strcpy(tmp, name[i]);
    name[i] = tmp;
    i++;
  }
  return name;
}

//search direct block, return ino
int search(int dev, char *str, INODE *ip)
{
  int i;
  char *cp;
  DIR *dp;
  char buf[BLKSIZE], temp[256];

  for(i = 0; i < 12; i++)
  {
    //block empty
    if(ip->i_block[i] == 0){break;} 
    get_block(dev, ip->i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    while(cp < buf+BLKSIZE)
    {
      memset(temp, 0, 256);
      strncpy(temp, dp->name, dp->name_len);
      if(strcmp(str, temp) == 0){ return dp->inode;}
      cp += dp->rec_len;
      dp = (DIR*)cp;
    }
  }
  return 0;
}

//searched file or dir from ino
int searchByIno(int dev, int ino, INODE *ip, char* temp)
{
  int i;
  char *cp;
  DIR *dp;
  char buf[BLKSIZE];

  for(i = 0; i < 12; i++)
  {
    if(ip->i_block[i] == 0){ break;}
    get_block(dev, ip->i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    while(cp < buf+BLKSIZE)
    {
      if(ino == dp->inode)
      {
        strncpy(temp, dp->name, dp->name_len);
        return 1;
      }
      cp += dp->rec_len;
      dp = (DIR*)cp;
    }
  }
  return 0;
}

//Get ino number
int getino(int dev, char *path)
{
  int ino = 0, i = 0;
  char **tokens;
  MINODE *mip = NULL;
  if(path && path[0])
  {
    tokens = tokenPath(path);
  }
  //no path means cwd
  else
  {
    ino = running->cwd->ino;
    return ino;
  }
  if(path[0]=='/')
  {
    ip = &(root->INODE);
    ino = root->ino;
  }
  //start at cwd
  else 
  {
    ip = &(running->cwd->INODE);
  }
  //search from token
  while(tokens[i])
  {
    ino = search(dev, tokens[i], ip);
    if(0 >= ino) 
    {
      if(mip){ iput(mip->dev, mip);}//invalid
      return -1;
    }
    if(mip) { iput(mip->dev, mip);}
    i++;
    if(tokens[i])
    {
      mip = iget(dev, ino);
      ip = &(mip->INODE);
    }
  }
  i = 0;
  while(tokens[i])
  {
    //free token for next
    free(tokens[i]);
    i++;
  }
  if(mip) { iput(mip->dev, mip);}
  return ino;
}


//get minode pointer
MINODE *iget(int dev, unsigned int ino)
{
  int i = 0, blk, offset;
  char buf[BLKSIZE];
  MINODE *mip = NULL;
  for(i = 0; i < 64; i++)
  {
    //if inode in array, mip point to minode in array
    if(minode[i].refCount && minode[i].ino == ino && minode[i].dev == dev)
    {
      mip = &minode[i];
      minode[i].refCount++;
      return mip;
    }
  }

  // inode DNE, put inode disk to a free MINODE
  i = 0;
  while(minode[i].refCount > 0 && i < 64) { i++;}
  if(i == 64)
  {
    return 0;
  }
  blk = (ino-1)/8 + inodeBegin;
  offset = (ino-1)%8;
  get_block(dev, blk, buf);
  ip = (INODE *)buf + offset;
  //copy from disk to array
  memcpy(&(minode[i].INODE), ip, sizeof(INODE)); 
  minode[i].dev = dev;
  minode[i].ino = ino;
  minode[i].refCount = 1;
  minode[i].dirty = 0;
  minode[i].mounted = 0;
  minode[i].mountptr = NULL;
  return &minode[i];
}

//minode to disk
int iput(int dev, MINODE *mip)
{
  char buf[BLKSIZE];
  int blk, offset;
  INODE *tip;

  mip->refCount--;
  //check used
  if(mip->refCount > 0) {return 1;} 
  //check dirty
  if(mip->dirty == 0) {return 1;} 

  //write INODE to disk
  blk = (mip->ino-1)/8 + inodeBegin;
  offset = (mip->ino-1)%8;

  get_block(dev, blk, buf); //load to mem

  tip = (INODE*)buf + offset;
  memcpy(tip, &(mip->INODE), sizeof(INODE)); //minode to temp
  put_block(mip->dev, blk, buf);
  mip->refCount = 0;
  return 1;
}

//read block 1 to buf
int get_super(int dev, char *buf)
{
  get_block(dev, SUPERBLOCK, buf);
}

//inodeBegin to start
void get_inode_table(int dev)
{
  char buf[BLKSIZE];
  get_gd(dev, buf);
  gp = (GD*)buf;
  inodeBegin = gp->bg_inode_table;
  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
}

//gd block 2
int get_gd(int dev, char *buf)
{
  get_block(dev, GDBLOCK, buf);
}
//check if bit 0 or 1
int tst_bit(char* buf, int i)
{
  int byt, offset;
  byt = i/8;
  offset = i%8;
  return (((*(buf+byt))>>offset)&1);
}

int set_bit(char* buf, int i)
{
  int byt, offset;
  char temp;
  char *tempBuf;
  byt = i/8;
  offset = i%8;
  tempBuf = (buf+byt);
  temp = *tempBuf;
  temp |= (1<<offset);
  *tempBuf = temp;
  return 1;
}

int clr_bit(char* buf, int i)
{
  int byt, offset;
  char temp;
  char *tempBuf;
  byt = i/8;
  offset = i%8;
  tempBuf = (buf+byt);
  temp = *tempBuf;
  temp &= (~(1<<offset));
  *tempBuf = temp;
  return 1;
}

int decFreeInodes(int dev)
{
  //decrements the number of free inodes by get super and 2nd block and decrement free inode 
  char buf[BLKSIZE];
  get_super(dev, buf);
  sp = (SUPER*)buf;
  sp->s_free_inodes_count -= 1; 
  put_block(dev, SUPERBLOCK, buf);
  get_gd(dev, buf);
  gp = (GD*)buf;
  gp->bg_free_inodes_count -=1;
  put_block(dev, GDBLOCK, buf);
  return 1;
}

int incFreeInodes(int dev)
{
  //increment the number of free inodes by doing same thing as dec but +
  char buf[BLKSIZE];
  get_super(dev, buf);
  sp = (SUPER*)buf;
  sp->s_free_inodes_count += 1;
  put_block(dev, SUPERBLOCK, buf);
  get_gd(dev, buf);
  gp = (GD*)buf;
  gp->bg_free_inodes_count +=1;
  put_block(dev, GDBLOCK, buf);
  return 1;

}
//same as ino but block
int decFreeBlocks(int dev)
{
  char buf[BLKSIZE];
  get_super(dev, buf);
  sp = (SUPER*)buf;
  sp->s_free_blocks_count -= 1;
  put_block(dev, SUPERBLOCK, buf);
  get_gd(dev, buf);
  gp = (GD*)buf;
  gp->bg_free_blocks_count -=1;
  put_block(dev, GDBLOCK, buf);
  return 1;
}

int incFreeBlocks(int dev)
{
  char buf[BLKSIZE];
  get_super(dev, buf);
  sp = (SUPER*)buf;
  sp->s_free_blocks_count += 1;
  put_block(dev, SUPERBLOCK, buf);
  get_gd(dev, buf);
  gp = (GD*)buf;
  gp->bg_free_blocks_count +=1;
  put_block(dev, GDBLOCK, buf);
  return 1;

}

int dname(char *pathname, char buf[256])
{
  int i = 0;
  memset(buf, 0, 256);
  strcpy(buf, pathname);
  while(buf[i]) { i++; }//start
  while(i >= 0)
  {
    if(buf[i] == '/')   //split
    {
      buf[i+1] = 0; 
      return 1;
    }
    i--;
  }
  buf[0] = 0;
  return 1;
}

int bname(char *pathname, char *buf)
{
  int i = 0, j = 0;
  if(!pathname[0]) {return -1;}
  i = strlen(pathname);
  while(i >= 0 && pathname[i] != '/') { i--; }//start
  if(pathname[i] == '/')
  {
    i++;
    while(pathname[i])
    {
      buf[j++] = pathname[i++];//copy to buf
    }
    buf[j] = 0;
    return 1;
  }
  //if already basename
  else { strncpy(buf, pathname, 256);}//if already basename
  return 1;
}

//allocate inode
int ialloc(int dev)
{
  int i;
  char buf[BLKSIZE];
  get_block(dev, imap, buf); 
  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){//check used
      set_bit(buf, i);
      put_block(dev, imap, buf);
      decFreeInodes(dev);//update
      return (i+1);             
    }
  }
  return 0;
}


//allocates block same as ino
int balloc(int dev)
{
  int i;
  char buf[BLKSIZE]; 

  get_block(dev, bmap, buf);

  for (i=0; i < BLKSIZE; i++){
    if (tst_bit(buf, i)==0){
      set_bit(buf, i);
      put_block(dev, bmap, buf);
      decFreeBlocks(dev);
      memset(buf, 0, BLKSIZE);//buf to 0
      put_block(dev, i+1, buf);
      return (i+1);
    }
  }
  return 0;
}


//deallocate ino
int idalloc(int dev, int ino)
{
  int i;
  char buf[BLKSIZE];

  get_block(dev, imap, buf);//get i to buf
  clr_bit(buf, ino-1);//clr
  put_block(dev, imap, buf);
  incFreeInodes(dev);
}

int bdalloc(int dev, int ino)
{
  int i;
  char buf[BLKSIZE];

  get_block(dev, bmap, buf);  
  clr_bit(buf, ino-1);
  put_block(dev, bmap, buf);
  incFreeBlocks(dev);
}


//check empty dir
int is_empty(MINODE *mip)
{
  int i;
  char *cp;
  DIR *dp;
  char buf[BLKSIZE], temp[256];
  for(i = 0; i < 12; i++)
  {
    if(ip->i_block[i] == 0) { return 0; }

    get_block(dev, ip->i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    while(cp < buf+BLKSIZE)
    {
      memset(temp, 0, 256);
      strncpy(temp, dp->name, dp->name_len);
      if(strncmp(".", temp, 1) != 0 && strncmp("..", temp, 2) != 0) { return 1;}
      cp += dp->rec_len;
      dp = (DIR*)cp;
    }
  }
}

int split_paths(char *original, char *path1, char *path2)
{
  //split space
  char *temp;
  temp = strtok(original, " ");
  strcpy(path1, temp);
  temp = strtok(NULL, " ");
  if(temp == NULL)
  {
    return -1;
  }
  strcpy(path2, temp);
  return 1;
}

//find last minode
int findLastBlock(MINODE *pip)
{
  int buf[256];
  int buf2[256];
  int bnumber, i, j;

  if(pip->INODE.i_block[0] == 0) {return 0;}
  //direct
  for(i = 0; i < 12; i++)
  {
    if(pip->INODE.i_block[i] == 0)
    {
      return (pip->INODE.i_block[i-1]);
    }
  }
  if(pip->INODE.i_block[12] == 0) {return pip->INODE.i_block[i-1];}
  //check indirect
  get_block(dev, pip->INODE.i_block[12], (char*)buf);
  for(i = 0; i < 256; i++)
  {
    //look for the free blocks
    if(buf[i] == 0) {return buf[i-1];}
  }
  //check double indirect
  if(pip->INODE.i_block[13] == 0) {return buf[i-1];}
  memset(buf, 0, 256);
  get_block(pip->dev, pip->INODE.i_block[13], (char*)buf);
  for(i = 0; i < 256; i++)
  {
    if(buf[i] == 0) {return buf2[j-1];}
    if(buf[i])
    {
      get_block(pip->dev, buf[i], (char*)buf2);
      for(j = 0; j < 256; j++)
      {
        if(buf2[j] == 0) {return buf2[j-1];}
      }
    }
  }
}

//addlast of minode
int addLastBlock(MINODE *pip, int bnumber)
{
  int buf[256];
  int buf2[256];
  int i, j, newBlk, newBlk2;
  for(i = 0; i < 12; i++) //direct
  {
    if(pip->INODE.i_block[i] == 0) {pip->INODE.i_block[i] = bnumber; return 1;}
  }
  //indirect
  if(pip->INODE.i_block[12] == 0)
  {
    newBlk = balloc(pip->dev);
    pip->INODE.i_block[12] = newBlk;
    memset(buf, 0, 256);
    get_block(pip->dev, newBlk, (char*)buf);
    buf[0] = bnumber;
    put_block(pip->dev, newBlk, (char*)buf);
    return 1;
  }
  memset(buf, 0, 256);
  get_block(pip->dev, pip->INODE.i_block[12], (char*)buf);
  for(i = 0; i < 256; i++)
  {
    if(buf[i] == 0) {buf[i] = bnumber; return 1;}
  }
  //double indirect
  if(pip->INODE.i_block[13] == 0) 
  {
    newBlk = balloc(pip->dev);
    pip->INODE.i_block[13] = newBlk;
    memset(buf, 0, 256);
    get_block(pip->dev, newBlk, (char*)buf);
    newBlk2 = balloc(pip->dev);
    buf[0] = newBlk2;
    put_block(pip->dev, newBlk, (char*)buf);
    memset(buf2, 0, 256);
    get_block(pip->dev, newBlk2, (char*)buf2);
    buf2[0] = bnumber;
    put_block(pip->dev, newBlk2, (char*)buf2);
    return 1;
  }
  memset(buf, 0, 256);
  get_block(pip->dev, pip->INODE.i_block[13], (char*)buf);
  for(i = 0; i < 256; i++)
  {
    if(buf[i] == 0)
    {
      newBlk2 = balloc(pip->dev);
      buf[i] = newBlk2;
      put_block(pip->dev, pip->INODE.i_block[13], (char*)buf);
      memset(buf2, 0, 256);
      get_block(pip->dev, newBlk2, (char*)buf2);
      buf2[0] = bnumber;
      put_block(pip->dev, newBlk2, (char*)buf2);
      return 1;
    }
    memset(buf2, 0, 256);
    get_block(pip->dev, buf[i], (char*)buf2);
    for(j = 0; j < 256; j++)
    {
      if(buf2[j] == 0) {buf2[j] = bnumber; return 1;}
    }
  }
  printf("Failed block to node\n");
  return -1;
}

int findparent(char *pathn)
{
  int i = 0;
  while(i < strlen(pathn))
  {
    if(pathn[i] == '/')
      return 1;
    i++;
  }
  return 0;
}
