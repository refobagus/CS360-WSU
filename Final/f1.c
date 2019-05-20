#include "header.h"
#include "util.h"

int init()
{
  proc[0].uid = 0;
  proc[0].cwd = 0;
  proc[1].uid = 1;
  proc[1].cwd = 0;

  running = &(proc[0]);
  readQueue = &(proc[1]);
  int i = 0;
  for(i = 0; i < 64; i++)
  {
    minode[i].refCount = 0;
    minode[i].ino = 0;
  }
  for(i = 0; i < 10; i++) { MountTable[i].dev = 0;}
  root = 0;
}

int mount_root(char *diskname)
{
  char buf[BLKSIZE];
  printf("mountroot()\n");
  dev = open(diskname, O_RDWR);

  if (dev < 0){
    printf("open %s failed\n", diskname);
    exit(1);
  }
  get_super(dev, buf);
  sp = (SUPER*)buf;
  if (SUPER_MAGIC != sp->s_magic)
  {
    printf("Not an EXT2 file sytem\n");
    exit(0);
  }
  //Set inodeBegin to start
  //Set root inode
  //set cwd proc to root inode
  get_inode_table(dev); 
  ninodes = sp->s_inodes_count;
  root = iget(dev, ROOT_INODE); 
  proc[0].cwd = iget(dev, ROOT_INODE); 
  proc[1].cwd = iget(dev, ROOT_INODE);
  MountTable[0].mounted_inode = root;
  MountTable[0].ninodes = ninodes;
  MountTable[0].nblocks = sp->s_blocks_count;
  MountTable[0].dev = dev;
  strncpy(MountTable[0].name, diskname, 256);
  return dev;
}

/*    START FUNCTIONS 1     */


int my_chdir(char* pathname)
{
  MINODE *mip;
  unsigned int ino;
  if (strlen(pathname) == 0 || strcmp(pathname, "/") == 0){
    ino = root->ino;
  } 
  else { 
    ino = getino(dev, pathname);
  }
  //load inode
  mip = iget(dev, ino); 
  //check if dir
  if(!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Error: End of path is not a directory\n");
    iput(dev, mip);
    return 0;
  }
  iput(dev, running->cwd); 
  running->cwd = mip;   //new minode to mip
}

int my_ls(char *path)
{

  unsigned long ino;
  MINODE *mip, *pip;
  int device = running->cwd->dev;
  char *child;

  if(path[0] == 0)// print cwd
  {
    mip = iget(device, running->cwd->ino);

    ls_dir(device, mip);

  }
  else
  {
    if(path[0] == '/')
    {
      device = root->dev;
    }
    ino = getino(device, path);
    //DNE
    if(ino == -1 || ino == 0)
    {
      return 1;
    }
    mip = iget(device, ino);

    //check dir
    if(((mip->INODE.i_mode) & 0040000)!= 0040000)
    {
      //get basename
      if(findparent(path))
        child = basename(pathname);
      else
      {
        child = (char *)malloc((strlen(pathname) + 1) * sizeof(char));
        strcpy(child, path);
      }
      //print file
      ls_file(mip, child);
      iput(mip->dev, mip);
      return 0;
    }
    //print dir
    ls_dir(device, mip);
  }

  iput(mip->dev, mip);
  return 0;
}

void ls_file(MINODE *mip, char *namebuf)
{
    INODE* pip = &mip->INODE;
    u16 mode = pip->i_mode;
    time_t val = pip->i_ctime;
    char *mtime = ctime(&val);
    mtime[strlen(mtime) - 1] = '\0';
    u16 type = mode & 0xF000;
    switch(type)
    {
        case 0x4000:
            printf("d");
            break;
        case 0x8000:
            printf("-");
            break;
        case 0xA000:
            printf("l");
            break;
        default:
            printf("-");
            break;
    }
    printf( (mode & S_IRUSR) ? "r" : "-");
    printf( (mode & S_IWUSR) ? "w" : "-");
    printf( (mode & S_IXUSR) ? "x" : "-");
    printf( (mode & S_IRGRP) ? "r" : "-");
    printf( (mode & S_IWGRP) ? "w" : "-");
    printf( (mode & S_IXGRP) ? "x" : "-");
    printf( (mode & S_IROTH) ? "r" : "-");
    printf( (mode & S_IWOTH) ? "w" : "-");
    printf( (mode & S_IXOTH) ? "x" : "-");
    printf("%4d%4d%4d  %s%8d    %s", pip->i_links_count, pip->i_gid, pip->i_uid, mtime, pip->i_size, namebuf);
    // if this is a symlink file, show the file it points to
    if((mode & 0120000) == 0120000)
    printf(" => %s\n",(char *)(mip->INODE.i_block));
    else
    printf("\n");
}

void ls_dir(int devicename, MINODE *mp)
{
  char buf[BLKSIZE], namebuf[256], *cp;
  DIR *dp;
  int i, ino;
  MINODE *temp;
  //print from direct
  for(i = 0; i < 12; i++)
  {

    if(mp->INODE.i_block[i])
    {
      get_block(devicename, mp->INODE.i_block[i], buf);
      cp = buf;
      dp = (DIR *)buf;

      while(cp < &buf[BLKSIZE])
      {

        strncpy(namebuf, dp->name, dp->name_len);
        namebuf[dp->name_len] = 0;

        ino = dp->inode;
        temp = iget(devicename, ino);

        ls_file(temp, namebuf);

        cp+=dp->rec_len;
        dp = (DIR *)cp;
        iput(temp->dev,temp);
      }
    }
  }
}
int my_pwd(char *pathname)
{
  printf("cwd = ");
  rpwd(running->cwd);
  printf("\n");
}
int rpwd(MINODE *wd)
{
  int ino = 0;
  MINODE *next = NULL;
  char temp[256];
  //check root
  if(wd == root) 
  {
    printf("/");
    return 1;
  }
  //parent minode
  ino = search(dev, "..", &(wd->INODE));
  if(ino <= 0){return -1;}

  next = iget(dev, ino); 

  if(!next){return -1;}

  //recursive till root
  rpwd(next); 
  memset(temp, 0, 256);
  //find dir name
  searchByIno(next->dev, wd->ino, &(next->INODE), temp);
  printf("%s/", temp);
  //put back
  iput(next->dev, next); 
  return 1;
}

int my_mkdir(char *pathname)
{
  int dev1, ino, r;
  char parent[256], child[256], origPathname[512];
  MINODE *mip;
  memset(parent, 0, 256);
  memset(child, 0, 256);
  memset(origPathname, 0, 512);

  strcpy(origPathname, pathname);
  //check root or dir
  if(!strcmp(pathname,"")){
    printf("Missing operand\n"); 
    return -1;
  }
  if(pathname[0] == '/') { dev1 = root->dev; }
  else { dev1 = running->cwd->dev; }

  dname(pathname, parent);
  bname(origPathname, child);
  ino = getino(dev1, parent);
  if(ino <= 0)
  {
    printf("Invalid.\n");
    return -1;
  }
  mip = iget(dev1, ino);

  //check if dir
  if(!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Not a directory.\n");
    iput(dev1, mip);
    return -1;
  }
  //check if copy
  ino = search(dev1, child, &(mip->INODE));
  if(ino > 0)
  {
    printf("Directory exists.\n");
    iput(mip->dev, mip);
    return -1;
  }
  r = rmkdir(mip, child);
  //put back
  iput(mip->dev, mip);
  return r;
}

int rmkdir(MINODE *pip, char child[256])
{
  int inumber, bnumber, idealLen, needLen, newRec, i, j;
  MINODE *mip;
  char *cp;
  DIR *dpPrev;
  char buf[BLKSIZE];
  char buf2[BLKSIZE];
  int blk[256];
  //allocate
  inumber = ialloc(pip->dev);
  bnumber = balloc(pip->dev);

  //get inode to mem
  mip = iget(pip->dev, inumber);

  //makedir update
  mip->INODE.i_mode = 0x41ED; //directory mode
  mip->INODE.i_uid = running->uid;
  mip->INODE.i_gid = running->gid;
  mip->INODE.i_size = BLKSIZE;
  mip->INODE.i_links_count = 2;
  mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
  mip->INODE.i_blocks = 2;
  mip->dirty = 1;
  for(i = 0; i <15; i++) { mip->INODE.i_block[i] = 0; }
  mip->INODE.i_block[0] = bnumber;
  //put to mem
  iput(mip->dev, mip);

  //. ..
  dp = (DIR*)buf;
  dp->inode = inumber;
  strncpy(dp->name, ".", 1);
  dp->name_len = 1;
  dp->rec_len = 12;

  cp = buf + 12;
  dp = (DIR*)cp;
  dp->inode = pip->ino;
  dp->name_len = 2;
  strncpy(dp->name, "..", 2);
  dp->rec_len = BLKSIZE - 12;

  //put block to mem
  put_block(pip->dev, bnumber, buf);
  memset(buf, 0, BLKSIZE);
  needLen = 4*((8+strlen(child)+3)/4);
  //find available block
  bnumber = findLastBlock(pip);
  //get block buffer
  get_block(pip->dev, bnumber, buf);

  cp = buf;
  dp = (DIR*)cp;
  //traverse to the last item in the block
  while((dp->rec_len + cp) < buf+BLKSIZE)
  {
    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
  idealLen = 4*((8+dp->name_len+3)/4);

  //check if enough room
  if(dp->rec_len - idealLen >= needLen)
  {
    //update
    newRec = dp->rec_len - idealLen;
    dp->rec_len = idealLen;
    cp += dp->rec_len;
    dp = (DIR*)cp;
    dp->inode = inumber;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = newRec;
  }
  else //if no room
  {
    //allocate new block and add data
    bnumber = balloc(pip->dev);
    dp = (DIR*)buf;
    dp->inode = inumber;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = BLKSIZE;
    addLastBlock(pip, bnumber);
  }
  //put block to disk
  put_block(pip->dev, bnumber, buf);
  pip->dirty = 1;
  pip->INODE.i_links_count++;
  memset(buf, 0, BLKSIZE);
  //check parent
  searchByIno(pip->dev, pip->ino, &running->cwd->INODE, buf);
  //update time using touch
  my_touch(buf);
  return 1;
}

int my_touch (char* name)
{
  char buf[1024];
  int ino;
  MINODE *mip;

  ino = getino(dev, name);
  if(ino <= 0)//create file if input unavailable
  {
    my_creat(name);
    return 1;
  }
  mip = iget(dev, ino);
  //update time
  mip->INODE.i_atime = mip->INODE.i_mtime = mip->INODE.i_ctime =(unsigned)time(NULL);
  mip->dirty = 1;
  iput(mip->dev, mip);
  return 1;
}

int my_creat(char* pathname)
{
  int dev1, ino, r;
  char parent[256];
  char child[256];
  MINODE *mip;
  memset(parent, 0, 256);
  memset(child, 0, 256);
  if(!strcmp(pathname,"")){
    printf("Missing operand\n"); 
    return -1;
  }
  //start at root
  if(pathname[0] == '/') { dev1 = root->dev;}
  //else cwd
  else { dev1 = running->cwd->dev; } 

  dname(pathname, parent);
  bname(pathname, child);

  ino = getino(dev1, parent);
  if(ino <= 0)
  {
    printf("Invalid.\n");
    return -1;
  }
  mip = iget(dev1, ino);
  if(!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Not a directory.\n");
    iput(dev1, mip);
    return -1;
  }
  ino = search(dev1, child, &(mip->INODE));
  if(ino > 0)
  {
    printf("File exists.\n");
    iput(mip->dev, mip);
    return -1;
  }
  r = creat_file(mip, child);
  iput(mip->dev, mip);
  return r;
}

int creat_file(MINODE *pip, char child[256])
{
  int inumber, bnumber, idealLen, needLen, newRec, i, j;
  MINODE *mip;
  char *cp;
  DIR *dpPrev;
  char buf[BLKSIZE];
  char buf2[BLKSIZE];
  int blk[256];

  //allocate ino
  inumber = ialloc(pip->dev);
  mip = iget(pip->dev, inumber);

  //Write contents
  mip->INODE.i_mode = 0x81A4;
  mip->INODE.i_uid = running->uid;
  mip->INODE.i_gid = running->gid;
  mip->INODE.i_size = 0;
  mip->INODE.i_links_count = 1;
  mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
  mip->INODE.i_blocks = 0;
  mip->dirty = 1;
  for(i = 0; i <15; i++)
  {
    mip->INODE.i_block[i] = 0;
  }
  iput(mip->dev, mip);
  //name
  memset(buf, 0, BLKSIZE);
  needLen = 4*((8+strlen(child)+3)/4);
  bnumber = findLastBlock(pip);

  //check room
  get_block(pip->dev, bnumber, buf);
  dp = (DIR*)buf;
  cp = buf;
  while((dp->rec_len + cp) < buf+BLKSIZE)
  {
    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
  idealLen = 4*((8+dp->name_len+3)/4);
  if(dp->rec_len - idealLen >= needLen)
  {
    newRec = dp->rec_len - idealLen;
    dp->rec_len = idealLen;
    cp += dp->rec_len;
    dp = (DIR*)cp;
    dp->inode = inumber;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = newRec;
  }
  else //else allocate block
  {
    bnumber = balloc(pip->dev);
    dp = (DIR*)buf;
    dp->inode = inumber;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = BLKSIZE;
    addLastBlock(pip, bnumber);
  }
  //putback
  put_block(pip->dev, bnumber, buf);
  pip->dirty = 1;
  memset(buf, 0, BLKSIZE);
  searchByIno(pip->dev, pip->ino, &running->cwd->INODE, buf);
  my_touch(buf);
  return 1;
}

int my_chmod(char* pathname)
{
  char buf[1024];
  char nMode[256];
  char path[256];
  int ino, newMode, i;
  MINODE* mip;

  if(!strcmp(pathname,"")){
    printf("Missing operand\n"); 
    return -1;
  }
  //split space
  if(split_paths(pathname, nMode, path) <= 0) { return -1; }
  //convert newmode
  newMode = strtoul(nMode, NULL, 8);
  ino = getino(dev, path);
  if(ino <= 0)//if file is not there, quit
  {
    return -1;
  }

  //load the inode into memory
  mip = iget(dev, ino);
  //change bitwise
  i = ~0x1FF;
  mip->INODE.i_mode &= i;
  mip->INODE.i_mode |= newMode;
  mip->dirty = 1;
  iput(dev, mip);
  return 1;
}

int my_rmdir(char *pathname)
{
  int ino, i;
  char parent[256], child[256], origPathname[512];
  MINODE *pip = NULL;
  MINODE *mip = NULL;
  strcpy(origPathname, pathname);
  //checkdirname
  if(!pathname || !pathname[0])
  {
    printf("Error: No directory given\n");
    return -1;
  }
  else
  {
    ino = getino(dev, pathname);
  }
  if(0 >= ino)
  {
    printf("Invalid.\n");
    return -1;
  }
  //load mem
  mip = iget(dev, ino);
  //check if it is a dir
  if(!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Not a directory.\n");
    iput(mip->dev, mip);
    return -1;
  }
  //check if in dir
  if(mip->refCount > 1)
  {
    printf("In Directory.\n");
    return -1;
  }
  //check if not empty
  if(mip->INODE.i_links_count > 2)
  {
    printf("Directory not empty.(dir in dir)\n");
    iput(mip->dev, mip);
    return -1;
  }
  //Check data blocks if file exist
  if(is_empty(mip) != 0)
  {
    printf("Directory not empty.(file in dir)\n");
    iput(mip->dev, mip);
    return -1;
  }
  for(i = 0; i < 12; i++)
  {
    if(mip->INODE.i_block[i] != 0)
    {
      bdalloc(mip->dev, mip->INODE.i_block[i]);
    }
  }
  idalloc(mip->dev, mip->ino);

  dname(origPathname, parent);
  bname(origPathname, child);
  //update
  ino = getino(mip->dev, parent);
  pip = iget(mip->dev, ino);
  iput(mip->dev, mip);
  rm_child(pip, child);
  pip->INODE.i_links_count--;
  my_touch(parent);
  pip->dirty = 1;
  iput(pip->dev, pip);
  return 1;
}

int rm_child(MINODE *pip, char *child)
{
  int i, size, found = 0;
  char *cp, *cp2;
  DIR *dp, *dp2, *dpPrev;
  char buf[BLKSIZE], buf2[BLKSIZE], temp[256];

  memset(buf2, 0, BLKSIZE);
  //check direct
  for(i = 0; i < 12; i++)
  {
    if(pip->INODE.i_block[i] == 0) { return 0; }
    //load to mem
    get_block(pip->dev, pip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    dp2 = (DIR *)buf;
    dpPrev = (DIR *)buf;
    cp = buf;
    cp2 = buf;

    while(cp < buf+BLKSIZE && !found)
    {
      memset(temp, 0, 256);
      strncpy(temp, dp->name, dp->name_len);
      if(strcmp(child, temp) == 0)
      {
        //if only child
        if(cp == buf && dp->rec_len == BLKSIZE)
        {
          //deallocate
          bdalloc(pip->dev, pip->INODE.i_block[i]);
          pip->INODE.i_block[i] = 0;
          pip->INODE.i_blocks--;
          found = 1;
        }
        //else delete child
        else
        {
          while((dp2->rec_len + cp2) < buf+BLKSIZE)
          {
            dpPrev = dp2;
            cp2 += dp2->rec_len;
            dp2 = (DIR*)cp2;
          }
          if(dp2 == dp)
          {
            dpPrev->rec_len += dp->rec_len;
            found = 1;
          }
          else
          {
            size = ((buf + BLKSIZE) - (cp + dp->rec_len));
            dp2->rec_len += dp->rec_len;
            //copy
            memmove(cp, (cp + dp->rec_len), size);
            dpPrev = (DIR*)cp;
            memset(temp, 0, 256);
            strncpy(temp, dpPrev->name, dpPrev->name_len);
            found = 1;
          }
        }
      }
      cp += dp->rec_len;
      dp = (DIR*)cp;
    }
    if(found)
    {
      //putback to disk
      put_block(pip->dev, pip->INODE.i_block[i], buf);
      return 1;
    }
  }
  return -1;
}

int my_link(char* pathname)
{
  char oldFile[256], newFile[256], parent[256], child[256], buf[BLKSIZE];
  int ino, ino2, bnumber, needLen, idealLen, newRec;
  MINODE *mip, *mip2;
  char *cp;
  DIR *dp;
  if(split_paths(pathname, oldFile, newFile)<=0) { return -1; }
  //check file exist
  ino = getino(dev, oldFile);

  if(ino <= 0)
  {
    printf("File DNE.\n");
    return -1;
  }
  mip = iget(dev, ino);
  //check isreg
  if(!S_ISREG(mip->INODE.i_mode))
  {
    printf("Not regular\n");
    iput(mip->dev, mip);
    return -1;
  }
  dname(newFile, parent);
  bname(newFile, child);
  //check second file
  ino2 = getino(mip->dev, parent);
  if(ino2<=0)
  {
    printf("File DNE");
    iput(mip->dev, mip);
    return -1;
  }
  mip2 = iget(mip->dev, ino2);
  //check dir parent
  if(!S_ISDIR(mip2->INODE.i_mode))
  {
    printf("Not a directory\n");
    iput(mip->dev, mip);
    iput(mip2->dev, mip2);
    return -1;
  }
  //check if filename used
  ino2 = search(mip2->dev, child, &(mip2->INODE));
  if(ino2 > 0)
  {
    printf("File exists\n");
    iput(mip->dev, mip);
    iput(mip2->dev, mip2);
    return -1;
  }

  memset(buf, 0, BLKSIZE);
  needLen = 4*((8+strlen(child)+3)/4);
  bnumber = findLastBlock(mip2);
  //check if enough room
  get_block(mip2->dev, bnumber, buf);
  dp = (DIR*)buf;
  cp = buf;
  while((dp->rec_len + cp) < buf+BLKSIZE)
  {
    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
  idealLen = 4*((8+dp->name_len+3)/4);
  if(dp->rec_len - idealLen >= needLen)
  {
    newRec = dp->rec_len - idealLen;
    dp->rec_len = idealLen;
    cp += dp->rec_len;
    dp = (DIR*)cp;
    dp->inode = ino;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = newRec;
  }
  else //allocate
  {
    bnumber = balloc(mip2->dev);
    dp = (DIR*)buf;
    dp->inode = ino;
    dp->name_len = strlen(child);
    strncpy(dp->name, child, dp->name_len);
    dp->rec_len = BLKSIZE;
    addLastBlock(mip2, bnumber);
  }

  put_block(mip2->dev, bnumber, buf);
  mip->dirty = 1;
  mip->INODE.i_links_count++;
  memset(buf, 0, BLKSIZE);
  searchByIno(mip2->dev, mip2->ino, &running->cwd->INODE, buf);
  iput(mip->dev, mip);
  iput(mip2->dev, mip2);
  return 1;
}

int my_unlink(char *pathname)
{
  char oldFile[256], parent[256], child[256], buf[BLKSIZE], oldPath[512];
  int ino, ino2, bnumber, needLen, idealLen, newRec;
  MINODE *mip, *mip2;
  char *cp;
  DIR *dp;
  strcpy(oldPath, pathname);
  //check file
  ino = getino(dev, pathname);
  if(ino <= 0)
  {
    printf("File DNE\n");
    return -1;
  }
  //check reg or lnk
  mip = iget(dev, ino);
  if(!S_ISREG(mip->INODE.i_mode) && !S_ISLNK(mip->INODE.i_mode))
  {
    printf("File not LNK or REG\n");
    iput(mip->dev, mip);
    return -1;
  }
  mip->INODE.i_links_count--;
  if(mip->INODE.i_links_count <= 0)
  {
    rm(mip);
  }
  dname(oldPath, parent);
  bname(oldPath, child);
  ino2 = getino(mip->dev, parent);

  //check if there is child
  if(ino2 == -1 || ino2 == 0) {return -1;}
  mip2 = iget(mip->dev, ino2);
  iput(mip->dev, mip);
  rm_child(mip2, child);

  iput(mip->dev, mip2);
}

int rm(MINODE *mip)
{
  int i, j;
  int buf[256], buf2[256];

  if(!S_ISLNK(mip->INODE.i_mode))
  {
    for(i = 0; i < 12; i++)
    {
      if(mip->INODE.i_block[i] != 0)
      {
        //deallocate block
        bdalloc(mip->dev, mip->INODE.i_block[i]);
      }
    }
    if(mip->INODE.i_block[12] != 0)
    {
      memset(buf, 0, 256);
      get_block(mip->dev, mip->INODE.i_block[12], (char*)buf);
      for(i = 0; i < 256; i++)
      {
        if(buf[i] != 0) {bdalloc(mip->dev, buf[i]);}
      }
      bdalloc(mip->dev, mip->INODE.i_block[12]);
    }
    if(mip->INODE.i_block[13] != 0)
    {
      memset(buf, 0, 256);
      get_block(mip->dev, mip->INODE.i_block[13], (char*)buf);
      for(i = 0; i < 256; i++)
      {
        if(buf[i] != 0)
        {
          memset(buf2, 0, 256);
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
  //finally ino
  idalloc(mip->dev, mip->ino);
  return 1;
}

int my_symlink(char *pathname)
{
  char oldname[256], newname[256];
  int ino;
  MINODE *mip;

  if(split_paths(pathname, oldname, newname) <= 0) {return -1;}
  if((ino = getino(dev, oldname))<= 0)
  {
    printf("File DNE\n");
    return -1;
  }
  //create file
  my_creat(newname);
  if(0 >= (ino = getino(dev, newname))){return -1;}
  mip = iget(dev, ino);
  mip->INODE.i_mode = 0120000; //symlink
  mip->dirty = 1;

  strcpy((char*)mip->INODE.i_block, oldname);
  iput(mip->dev, mip);
}

int quit(char* pathname)
{
  int i = 0;
  char str[256];
  for(i = 0; i < 64; i++)
  {
    if(minode[i].refCount > 0)
    {
      if(minode[i].dirty != 0)
      {
        minode[i].refCount = 1;
        iput(dev, &minode[i]);
      }
    }
  }
  exit(0);
}

int my_stat (char *pathname)
{
  char buf[1024];
  int ino;
  MINODE *mip;
  if(!strcmp(pathname,""))
  {
    printf("Missing operand");
    return -1;
  }
  ino = getino(dev, pathname);

  mip = iget(dev, ino);
  //print everything
  time_t val = mip->INODE.i_ctime;
  char *mtime = ctime(&val);
 printf("*********  stat  *********\n");
 printf("dev=%d ino=%d mod=%x\n",mip->dev,ino,mip->INODE.i_mode);
 printf("uid=%d gid=%d nlink=%d\n",mip->INODE.i_uid,mip->INODE.i_gid,mip->INODE.i_links_count);
 printf("size=%d time=%s",mip->INODE.i_size,mtime);
  return 1;
}

