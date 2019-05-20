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

//Functions 2
int my_truncate(MINODE *mip);
int open_file(char *pathname);
int close_file(char *pathname);
int my_close(int fd);
int write_file(char *pathname);
int my_write(int fd, char *buf, int nbytes);
int read_file(char *path);
int my_read(int fd, char *buf, int nbytes);
int my_lseek(char *pathname);
int file_lseek(int fd, int position);
int my_cat(char *pathname);
int my_copy(char *pathname);
int copy_file(char *source, char *target);
int my_move(char*pathname);
int move_file(char*source,char*target);
