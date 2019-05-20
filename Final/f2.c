#include "header2.h"
#include "util.h"
#include "header.h"

/* Start Functions 2  */
int my_truncate(MINODE *mip) //currently broken
{
  int buf[256];
  int buf2[256];
  int bnumber, i, j;

  if(mip == NULL)
  {
    printf("Error: No file.\n");
    return -1;
  }
  //deallocate for direct
  for(i = 0; i < 12; i++)
  {
    if(mip->INODE.i_block[i] != 0)
    {
      bdalloc(mip->dev, mip->INODE.i_block[i]);
    }
  }
  //Deallocate Indirect blocks
  if(mip->INODE.i_block[12] != 0) 
  {
    get_block(dev, mip->INODE.i_block[12], (char*)buf);
    for(i = 0; i < 256; i++)
    {
      if(buf[i] != 0) {bdalloc(mip->dev, buf[i]);}
    }
    bdalloc(mip->dev, mip->INODE.i_block[12]);
    if(mip->INODE.i_block[13] != 0) 
    {
      memset(buf, 0, 256);
      get_block(mip->dev, mip->INODE.i_block[13], (char*)buf);
      for(i = 0; i < 256; i++)
      {
        if(buf[i])
        {
          get_block(mip->dev, buf[i], (char*)buf2);
          for(j = 0; j < 256; j++)
          {
            if(buf2[j] != 0) {bdalloc(mip->dev, buf2[j]);}
          }
          bdalloc(mip->dev, buf[i]);
        }
      }
      bdalloc(mip->dev, mip->INODE.i_block[13]);
    }
  }
  mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
  mip->INODE.i_size = 0;
  mip->dirty = 1;
  return 1;
}
int open_file(char *pathname)
{
  char pathfile[256], mode[256];
  int flags, ino, i, dev;
  MINODE *mip;
  
  OFT *oftp;
  if(split_paths(pathname, pathfile, mode) <= 0) { return -1; }
  else if(strcmp("R", mode) == 0){flags = R;}
  else if(strcmp("W", mode) == 0){flags = W;}
  else if(strcmp("RW", mode) == 0){flags = RW;}
  else if(strcmp("APPEND", mode) == 0){flags = APPEND;}
  else{return -1;}
  char parent[256], child[256], origPathname[512];
  memset(parent, 0, 256);
  memset(child, 0, 256);
  memset(origPathname, 0, 512);

  strcpy(origPathname, pathfile);
  //check root or dir
  if(!strcmp(pathfile,"")){
    printf("Missing operand\n"); 
    return -1;
  }
  if(pathfile[0] == '/') { dev = root->dev; }
  else { dev = running->cwd->dev; }

  dname(pathfile, parent);
  bname(origPathname, child);
  ino = getino(dev, parent);
  if(ino <= 0)
  {
    printf("Invalid.\n");
    return -1;
  }
  mip = iget(dev, ino);
  ino = search(dev, child, &(mip->INODE));

  if(ino <= 0){
    my_creat(child);//create if DNE
    ino = getino(dev,child);
    if( 0 >= ino)
    {
    printf("Open file Err\n");
    return -1;
    }
  }

  mip = iget(dev, ino);
  if(!S_ISREG(mip->INODE.i_mode))
  {
    printf("Not a regular file.\n");
    iput(mip->dev, mip);
    return -1;
  }

    for(i = 0; i < 10; i++)
    {
      if(running->fd[i] != NULL)
      {
        if(running->fd[i]->inodeptr == mip)
        {
          if(running->fd[i]->mode != 0 || flags != 0)
          {
            printf("File in fd.\n");
            iput(mip->dev, mip);
            return -1;
          }
        }
      }
    }

    //allocate opft
    oftp = (OFT *)malloc(sizeof(OFT));
    oftp->mode = flags;
    oftp->refCount = 1;
    oftp->inodeptr = mip;
    //set offset 
    switch(flags)
    {
      case 0: oftp->offset = 0;
              printf("File opened for read\n");
              my_touch(pathfile);
              break;
      case 1: my_truncate(oftp->inodeptr);
              printf("File open for write\n");
              oftp->offset = 0;
              my_touch(pathfile);
              break;
      case 2: oftp->offset = 0;
              printf("File open for rw\n");
              my_touch(pathfile);
              break;
      case 3: oftp->offset = mip->INODE.i_size;
              printf("File open for append\n");
              my_touch(pathfile);
              break;
      default: printf("Invalid\n");
               iput(mip->dev, mip);
               free(oftp);
               return -1;
               //break;
    }

  //check fd
  i = 0;
  while(running->fd[i] != NULL && i < 10){ i++; }
  if(i == 10) //fd full
  {
    iput(mip->dev, mip);
    free(oftp);
    return -1;
  }
  //else
  running->fd[i] = oftp;
  if(flags != 0) {mip->dirty = 1;}
  printf("i = %d", i);
  return i;
}

int close_file(char *pathname)
{
  if(pathname == NULL)// no fd
  {
    printf("No file desciptor\n");
    return -1;
  }
  int fd = atoi(pathname);
  return my_close(fd);
}

int my_close(int fd)
{
  MINODE *mip;
  OFT *oftp;
  printf("fd = %d", fd);
  if(fd < 0 || fd > 9)
  {
    printf("Fd invalid\n");
    return -1;
  }
  if(running->fd[fd] == NULL)
  {
    printf("File not open\n");
    return -1;
  }

  //close
  oftp = running->fd[fd];
  running->fd[fd] = 0;
  oftp->refCount--;
  if(oftp->refCount > 0) {return -1;}
  mip = oftp->inodeptr;
  iput(mip->dev, mip);
  free(oftp);
  printf("\nFile closed.\n");
  return 1;
}

int write_file(char *pathname)
{
  int fd;
  char writeMe[BLKSIZE];
  fd = atoi(pathname);
  if(fd < 0 || fd > 9)
  {
    printf("No FD\n");
    return -1;
  }
  if(running->fd[fd] == NULL)
  {
    printf("No FDa\n");
    return -1;
  }

  //check mode
  if(running->fd[fd]->mode == 0)
  {
    printf("File in read mode\n");
    return -1;
  }

  printf("Write: ");
  fgets(writeMe, BLKSIZE, stdin);
  writeMe[strlen(writeMe) -1] = 0;
  printf("writeme = %s",writeMe);
  if(writeMe[0] == 0)
  {
    return 0;
  }
  return my_write(fd, writeMe, strlen(writeMe));
}

int my_write(int fd, char *buf, int nbytes)
{
  MINODE *mip; 
  OFT *oftp;
  int count, lblk, start, blk, dblk, remain;
  int ibuf[256], dbuf[256];
  char writeBuf[BLKSIZE], *cp, *cq = buf;
  count = 0;
  oftp = running->fd[fd];
  mip = oftp->inodeptr;
  while(nbytes)
  {
    lblk = oftp->offset / BLKSIZE;
    start = oftp->offset % BLKSIZE;
    
    //convert logic to phys
    //direct blocks
    if(lblk < 12 ) 
    {
      if(mip->INODE.i_block[lblk]==0){
        mip->INODE.i_block[lblk] = balloc(mip->dev);
      }
      blk = mip->INODE.i_block[lblk];
    }
    //indirect blocks
    else if(lblk >= 12 && lblk < 256 + 12) 
    {
      if(mip->INODE.i_block[12]==0){
        mip->INODE.i_block[12] = balloc(mip->dev);
        memset(ibuf,0,256);
      }
      get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
      blk = ibuf[lblk - 12];
      if (blk==0){
        mip->INODE.i_block[lblk] = balloc(mip->dev);
        ibuf[lblk - 12] =mip->INODE.i_block[lblk];
      }
    }
    //double indirect blocks
    else 
    {
      memset(ibuf, 0, 256);
      get_block(mip->dev, mip->INODE.i_block[13], (char  *)dbuf);
      lblk -= (12 + 256);
      dblk = dbuf[lblk / 256];
      get_block(mip->dev, dblk, (char *)dbuf);
      blk = dbuf[lblk % 256];
    }



    memset(writeBuf,0,BLKSIZE);
    //read to buf
    get_block(mip->dev, blk, writeBuf);
    cp = writeBuf + start;
    remain = BLKSIZE - start;
    //copy
    // printf("cq = %s\n",cq);
    // while (remain > 0){               // write as much as remain allows  
    //        *cp++ = *cq++;              // cq points at buf[ ]
    //        nbytes--; remain--;         // dec counts
    //        oftp->offset++;             // advance offset
    //        if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
    //            mip->INODE.i_size++;    // inc file size (if offset > fileSize)
    //        if (nbytes <= 0) break;     // if already nbytes, break
    //  }
    if(remain < nbytes)
    {
      strncpy(cp, cq, remain);
      count += remain;
      nbytes -= remain;
      running->fd[fd]->offset += remain;
      //check offset
      if(running->fd[fd]->offset > mip->INODE.i_size)
      {
        mip->INODE.i_size += remain;
      }
      remain -= remain;
    }
    else
    {
      strncpy(cp, cq, nbytes);
      count += nbytes;
      remain -= nbytes;
      running->fd[fd]->offset += nbytes;
      if(running->fd[fd]->offset > mip->INODE.i_size)
      {
        mip->INODE.i_size += nbytes;
      }
      nbytes -= nbytes;
    }
    put_block(mip->dev, blk, writeBuf);
    mip->dirty = 1;
    printf("Wrote %d chars into file.\n", count);
  }
}

int read_file(char *pathname)
{
  char secondPath[256], path[256];
  split_paths(pathname, path, secondPath);
  //convert bytes to int
  int nbytes = atoi(secondPath), actual = 0;
  int fd = 0;
  OFT *oftp;
  INODE *pip;
  MINODE *pmip;
  int i;
  char buf[nbytes + 1];
  MINODE *mip;
  INODE* ip;

  strcpy(buf, "");
  //check fd
  if (!strcmp(pathname, ""))
  {
    printf("No FD\n");
    return 0;
  }
  //convert fd to int
  fd = atoi(pathname);
  if (!strcmp(secondPath, ""))
  {
    printf("No byte\n");
    return 0;
  }

  //return byte read
  actual = my_read(fd, buf, nbytes);

  if (actual == -1)
  {
    strcpy(secondPath, "");
    return 0;
  }

  buf[actual] = '\0';
  printf("actual = %d buf = %s\n", actual, buf);
  return actual;
}


int my_read(int fd, char *buf, int nbytes)
{
  MINODE *mip; 
  OFT *oftp;
  int count = 0;
  int lbk, blk, startByte, remain, ino;
  int avil;
  int *ip;

  int indirect_blk;
  int indirect_off;

  int buf2[BLKSIZE];

  char readbuf[1024];
  char temp[1024];

  oftp = running->fd[fd];
  //printf("flags = %d\n", oftp->mode);
  if(!oftp) {
    printf("file for write\n");
    return -1;
  }

  mip = oftp->inodeptr;
  //calculate byte to read
  avil = mip->INODE.i_size - oftp->offset;
  char *cq = buf;

  while(nbytes && avil)
  {
    lbk = oftp->offset / BLKSIZE;
    startByte = oftp->offset % BLKSIZE;

    //direct block
    if(lbk < 12)
    {
      blk = mip->INODE.i_block[lbk];
      //printf("direct\n");
    }
    //indirect blocks
    else if(lbk >= 12 && lbk < 256 + 12)
    {
      get_block(mip->dev, mip->INODE.i_block[12], readbuf);

      ip = (int *)readbuf + lbk - 12;
      blk = *ip;
      //printf("indirect\n");

    }
    //double indirect
    else
    {
      get_block(mip->dev, mip->INODE.i_block[13], readbuf);

      indirect_blk = (lbk - 256 - 12) / 256;
      indirect_off = (lbk - 256 - 12) % 256;
      printf("blk = %d, ofset = %d\n", indirect_blk, indirect_off);
      getchar();

      ip = (int *)readbuf + indirect_blk;
      getchar();
      get_block(mip->dev, *ip, readbuf);
      getchar();
      ip = (int *)readbuf + indirect_off;
      blk = *ip;
      getchar();
      //printf("double indirect\n");
    }

    //getblock to buf
    get_block(mip->dev, blk, readbuf);
    //printf("readbuf = %d and %s\nstartbyte = %d\n", readbuf, readbuf, startByte);
    char *cp = readbuf + startByte;

    remain = BLKSIZE - startByte;
    int temp =remain ^ ((avil ^ remain) & -(avil < remain));
    int temp2 =nbytes ^ ((temp ^ nbytes) & -(temp < nbytes));
    //temp2 = 1;
    //printf("minimum bytes = %d\n", temp2);
    //check available and remaining
    while(remain > 0)
    {
      // printf("avail = %d, remain = %d\n", avil, remain);
      // printf("temp2 = %d\n", temp2);
      strncpy(cq, cp, temp2);
      //*cq++ = *cp++;
      oftp->offset += temp2;
      count += temp2;
      avil -= temp2;
      nbytes -= temp2;
      remain -= temp2;
      // printf("avail = %d, remain = %d\n", avil, remain);
      if(nbytes <= 0 || avil <= 0)
        break;
    }
  }
  //printf("myread: read %d char from file descriptor %d\n", count, fd);
  return count;
}

int my_lseek(char * pathname)
{
  char secondPath[256], path[256];
  split_paths(pathname, path, secondPath);
  //convert bytes to int
  int nbytes = atoi(secondPath);
  int fd = 0;

  //check fd
  if (!strcmp(pathname, ""))
  {
    printf("No FD\n");
    return 0;
  }
  //convert fd to int
  fd = atoi(pathname);
  if (!strcmp(secondPath, ""))
  {
    printf("No byte\n");
    return 0;
  }
  return file_lseek(fd, nbytes);
}

int file_lseek(int fd, int position)
{
  OFT *oftp;
  oftp = running->fd[fd];
  int max = oftp->inodeptr->INODE.i_size - 1;
  int min = 0;

  if(position > max || position < min)
  {
    printf("Out of bounds.\n");
    return -1;
  }
  int originalPosition = oftp->offset;
  oftp->offset = position;
  return originalPosition;
}


int my_cat(char *pathname)
{
  char mybuf[BLKSIZE], dummy = 0;
  int n;
  char* temppath;
  strcpy(temppath, pathname);
  strcat(temppath," R");
  int fd = open_file(temppath);
  while( n = my_read(fd,mybuf,BLKSIZE))
  {
    mybuf[n]= 0;
    printf("%s",mybuf);
  }
  my_close(fd);
}


int my_copy(char *pathname)
{
  char destination[256], source[256];
  split_paths(pathname, source, destination);
  return copy_file(source, destination);
  
}
int copy_file(char *source, char *target)
{
  strcat(source, " R");
  strcat(target, " W");
  int fd = open_file(source);
  int gd = open_file(target);
  char buf[BLKSIZE];
  int n;

  while(n = my_read(fd, buf, BLKSIZE))
  {
    my_write(gd, buf, n);
  }

  my_close(fd);
  my_close(gd);
}

int pdf(char *pathname)
{
  int i = 0;
  char mode[256];
  printf(" fd     mode    offset     INODE \n");
  printf("----    ----    ------    -------\n");
  while(running->fd[i] != NULL && i < 10)
  {
    if(running->fd[i]->mode == 0)
    {
      strcpy(mode, "R ");
    }
    else if(running->fd[i]->mode == 1)
    {
      strcpy(mode, "W ");
    }
    else if(running->fd[i]->mode == 2)
    {
      strcpy(mode, "RW");
    }
    else if(running->fd[i]->mode == 3)
    {
      strcpy(mode, "A ");
    }
    printf("   %d      %s      %d       [%d,%d]\n", i, mode, running->fd[i]->offset,
     running->fd[i]->inodeptr->dev, running->fd[i]->inodeptr->ino);

    i++;
  }
  printf("--------------------------------------\n");
}

int my_move(char*pathname)
{
  char destination[256], source[256];
  split_paths(pathname, source, destination);
  return move_file(source, destination);
}

int move_file(char*source,char*target)
{
  char parent[256], child[256], parentT[256];
  memset(parent, 0, 256);
  memset(child, 0, 256);
  memset(parentT, 0, 256);
  dname(source, parent);
  dname(target, parentT);
  if(strcmp(parent,parentT)){
    strcpy(child, source);
    strcat(source, " ");
    strcat(source, target);
    printf("im here ese dad = %s source = %s child = %s\n",parent,source,child);
    my_copy(source);
    my_unlink(child);
  }
  else{
    strcpy(child, source);
    strcat(source, " ");
    strcat(source, target);

    printf("im here dad = %s source = %s child = %s\n",parent,source,child);
    my_link(source);
    my_unlink(child);
  }
}