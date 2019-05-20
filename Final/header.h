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

//Initialize
int init();
int mount_root(char *devName);

//Functions 1
int my_chdir(char* pathname);
int my_ls(char *path);
void ls_dir(int devicename, MINODE *mp);
void ls_file(MINODE *mip, char *namebuf);
int my_pwd(char* pathname);
int rpwd(MINODE *wd);
int my_mkdir(char *pathname);
int rmkdir(MINODE *pip, char child[256]);
int my_touch(char* name);
int my_creat(char* pathname);
int creat_file(MINODE *pip, char child[256]);
int my_chmod(char* pathname);
int my_rmdir(char* pathname);
int rm_child(MINODE *pip, char *child);
int my_link(char* pathname);
int my_unlink(char* pathname);
int rm(MINODE *mip);
int my_symlink(char *pathname);
int quit(char *pathname);
int my_stat (char *pathname);
int pdf(char *pathname);


