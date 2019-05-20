#pragma once
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "type.h"

int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int search(int dev, char *str, INODE *ip);
int searchByIno(int dev, int ino, INODE *ip, char *temp);
int getino(int dev, char *path);
MINODE *iget(int dev, unsigned int ino);
int iput(int dev, MINODE *mip);
int get_super(int dev, char *buf);
void get_inode_table(int dev);
int get_gd(int dev, char *buf);
int tst_bit(char* buf, int i);
int set_bit(char* buf, int i);
int clr_bit(char* buf, int i);
int decFreeInodes(int dev);
int incFreeInodes(int dev);
int decFreeBlocks(int dev);
int incFreeBlocks(int dev);
int dname(char *pathname, char buf[256]);
int bname(char *pathname, char buf[256]);
int ialloc(int dev);
int balloc(int dev);
int idalloc(int dev, int ino);
int bdalloc(int dev, int ino);
int is_empty(MINODE *mip);
int split_paths(char* original, char* path1, char* path2);
int findLastBlock(MINODE *pip);
int addLastBlock(MINODE *pip, int bnumber);
int findparent(char *pathn);
