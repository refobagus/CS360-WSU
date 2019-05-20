#include "header.h"
#include "header2.h"
#include "util.h"
#include "type.h"

static int (*fptr[])(char*) = {(int (*)())my_ls, my_chdir, my_pwd,  my_mkdir, my_chmod, my_creat, my_rmdir, my_link, my_unlink, my_symlink, my_touch, open_file, close_file, write_file, read_file, my_stat, my_lseek, my_cat, my_copy, pdf, my_move, quit};
static char *cmds[] = {"ls", "cd", "pwd",  "mkdir", "chmod",  "creat", "rmdir", "link", "unlink", "symlink", "touch", "open","close","write","read","stat","lseek","cat","cp", "pdf","mv","quit"};

int main(int argc, char *argv[])
{
    char line[128], cm[64], parameter[64];
    int cmd;
    char *diskname = "mydisk";
    if(argc > 1)
        diskname = argv[1];
    init();
    mount_root(diskname);
    printf("Available commands:\n");
    
    while(1)
    {
        memset(pathname, 0, 128);
        memset(parameter, 0, 64);
        memset(cm, 0, 64);
	printf("[ls pwd cd mkdir rmdir creat link symlink unlink chmod touch stat quit]\n");
    	printf("[open write read cat close]\n");
        printf("P%d running:\n", running->pid);
        printf("RYshell~$ ");
        fgets(line, 128, stdin);
        line[strlen(line)-1] = 0;
        if (line[0]==0) continue;

        sscanf(line, "%s %s %s", cm, pathname, parameter);
        //join together
        if(parameter[0] != 0)
        {
            strcat(pathname, " ");
            strcat(pathname, parameter);
        }
        for(int i = 0; i < 22; i++)
        {
            if(strcmp(cmds[i], cm) == 0)
            {
                    fptr[i](pathname);
                    continue;
            }

        }
        putchar(10);

    }
}
