
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <libgen.h>
#include <fcntl.h>

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

char serstr[INET_ADDRSTRLEN];

char *paths[64];

int  mysock, client_sock;              // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables


int put(void)
{
  // server receives from client
  int i = 0, b = 0, fd;
  char *rec;
  char *token, hold[64], name[64], bytes[16];
  rec = (char*)malloc(sizeof(char) * MAX);
  n = read(client_sock, rec, MAX);
  if(!strcmp(rec, ""))
  {
    printf("put failed\n");
    return 0;
  }
  strcpy(hold, rec);
  token = strtok(hold, " ");
  strcpy(name, token);
  token = strtok(NULL, " ");
  strcpy(bytes, token);
  b = atoi(bytes);
  fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  free(rec);
  rec = (char*)malloc(sizeof(char) * b);
  n = read(client_sock, rec, b);
  write(fd, rec, b * sizeof(char));
  free(rec);
}

int get(void)
{
  // server puts to client
  struct stat buf;
  char send[64], check[64];
  char *dname, *bname, *rec;
  int n, fd;
  dname = dirname(paths[1]);
  bname = basename(paths[1]);
  if(dname == NULL)
    dname = ".";
  if(stat(dname, &buf))
  {
    printf("get failed\n");
    return 0;
  }
  if(S_ISDIR(buf.st_mode))
  {
    DIR *dir = opendir(dname);
    struct dirent *file = readdir(dir);
    if(dname[strlen(dname) - 1] == '/')
      dname[strlen(dname) - 1] = '\0';
    while(file)
    {
      if(!strcmp(file->d_name, bname))
      {
        int size = buf.st_size;
        snprintf(check, sizeof(check), "%s/%s", dname, file->d_name);
        stat(check, &buf);
        snprintf(send, sizeof(send), "%s %d", file->d_name, buf.st_size);
        n = write(client_sock, send, MAX);
        snprintf(send, sizeof(send), "%s/%s", dname, bname);
        fd = open(send, O_RDONLY);
        rec = (char*)malloc(sizeof(char) * buf.st_size);
        read(fd, rec, buf.st_size);
        n = write(client_sock, rec, buf.st_size);
        free(rec);
        printf("File get\n");
        return 1;
      }
      file = readdir(dir);
    }
    n = write(client_sock, "", MAX);
    printf("get failed\n");
    return 0;
  }
}

int rm1(void)
{
  if(paths[1])
  {
    int i = 1;
    while(paths[i])
    {
      if(!unlink(paths[i]))
      {
        printf("rm %s successful\n", paths[i]);
        n = write(client_sock, "rm successful", MAX);
      }
      else
      {
        printf("rm %s unsuccessful\n", paths[i]);
        n = write(client_sock, "rm unsuccessful", MAX);
      }
      i++;
    }
  }
  else
  {
    printf("rm unsuccessful\n");
    n = write(client_sock, "rm unsuccessful", MAX);
  } 
}

int pwd1(void)
{
  char buf[MAX];
  getcwd(buf,MAX);
  printf("%s\n",buf);
  n = write(client_sock, buf, MAX);
}

int rmdir1(void)
{
  if(paths[1])
  {
    int i = 1;
    while(paths[i])
    {
      if(!rmdir(paths[i]))
      {
        printf("rmdir %s successful\n", paths[i]);
        n = write(client_sock, "rmdir successful", MAX);
      }
      else
      {
        printf("rmdir %s unsuccessful\n", paths[i]);
        n = write(client_sock, "rmdir unsuccessful", MAX);
      }
      i++;
    }
  }
  else
  {
    printf("rmdir unsuccessful\n");
    n = write(client_sock, "rmdir unsuccessful", MAX);
  }
}

int mdir1(void)
{
  if(paths[1])
  {
    int i = 1;
    while(paths[i])
    {
      if(!mkdir(paths[i], 0755))
      {
        printf("mkdir %s successful\n", paths[i]);
        n = write(client_sock, "mkdir successful", MAX);
      }
      else
      {
        printf("mkdir %s unsuccessful\n", paths[i]);
        n = write(client_sock, "mkdir unsuccessful", MAX);
      }
      i++;
    }
  }
  else
  {
    printf("mkdir unsuccessful\n");
    n = write(client_sock, "mkdir unsuccessful", MAX);
  }
}

int chdir1(void)
{
  if(paths[1])
  {
    if(!chdir(paths[1]))
      {
        printf("chdir %s successful\n", paths[1]);
        n = write(client_sock, "chdir successful", MAX);
      }
    else 
    {  
      printf("chdir %s unsuccessful\n", paths[1]);
      n = write(client_sock, "chdir unsuccessful", MAX);
    }
  }
  else
  {
    chdir("/");
    printf("chdir / successful");
    n = write(client_sock, "chdir successful", MAX);
  }  
}

void ls_file(struct dirent *file, char *pathname)
{
  char hold[64];
  char ret[128], perm[11] = {'\0'};
  struct stat buf;
  snprintf(hold, sizeof(hold), "%s/%s", pathname, file->d_name);
  printf("ls %s\n", hold);
  if(!stat(hold, &buf))
  {
    time_t val = buf.st_mtime;
    char *mtime = ctime(&val);
    mtime[strlen(mtime) - 1] = '\0';
    strcat(perm, (S_ISDIR(buf.st_mode)) ? "d" : "-");
    strcat(perm, (buf.st_mode & S_IRUSR) ? "r" : "-");
    strcat(perm, (buf.st_mode & S_IWUSR) ? "w" : "-");
    strcat(perm, (buf.st_mode & S_IXUSR) ? "x" : "-");
    strcat(perm, (buf.st_mode & S_IRGRP) ? "r" : "-");
    strcat(perm, (buf.st_mode & S_IWGRP) ? "w" : "-");
    strcat(perm, (buf.st_mode & S_IXGRP) ? "x" : "-");
    strcat(perm, (buf.st_mode & S_IROTH) ? "r" : "-");
    strcat(perm, (buf.st_mode & S_IWOTH) ? "w" : "-");
    strcat(perm, (buf.st_mode & S_IXOTH) ? "x" : "-");
    snprintf(ret, sizeof(ret), "%s %d %d %d %s %s", perm, buf.st_nlink, buf.st_uid, buf.st_gid, mtime, file->d_name);
    n = write(client_sock, ret, MAX);
  }
}

char *ls_dir(char *pathname)
{
  struct stat buf;
  char hold[64];
  char ret[2048];
  if(pathname == NULL)
    pathname = ".";
  if(stat(pathname, &buf))
  {
    strcpy(hold, "ls failed\n");
    return hold;
  }
  if(S_ISDIR(buf.st_mode))
  {
    DIR *dir = opendir(pathname);
    struct dirent *file = readdir(dir);
    if(pathname[strlen(pathname) - 1] == '/')
      pathname[strlen(pathname) - 1] = '\0';
    while(file)
    {
      ls_file(file, pathname);
      file = readdir(dir);
    }
  }
}


// Server initialization code:

int server_init(char *name)
{
   printf("==================== server init ======================\n");   
   // get DOT name and IP address of this host

   printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   if (hp == 0){
      printf("unknown host\n");
      exit(1);
   }
   inet_ntop(AF_INET, hp->h_addr, serstr, INET_ADDRSTRLEN);
  printf("    hostname=%s  IP=%s\n",
               hp->h_name,  serstr);

   //  create a TCP socket by socket() syscall
   printf("2 : create a socket\n");
   mysock = socket(AF_INET, SOCK_STREAM, 0);
   if (mysock < 0){
      printf("socket call failed\n");
      exit(2);
   }

   printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address  
   server_addr.sin_port = 0;   // let kernel assign port

   printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }

   printf("5 : find out Kernel assigned PORT# and show it\n");
   // find out socket port number (assigned by kernel)
   length = sizeof(name_addr);
   r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
   if (r < 0){
      printf("get socketname error\n");
      exit(4);
   }

   // show port number
   serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
   printf("    Port=%d\n", serverPort);

   // listen at port with a max. queue of 5 (waiting clients) 
   printf("5 : server is listening ....\n");
   listen(mysock, 5);
   printf("===================== init done =======================\n");
}


void parse(char *input)
{
  int len, i = 0;
  char *token, *hold;
  token = strtok(input, " ");
  paths[i] = token;
  i++;
  while(token = strtok(NULL, " "))
  {
    len = strlen(token);
    hold = (char*)malloc(len * sizeof(char));
    strcpy(hold, token);
    paths[i] = hold;
    i++;
  }
  return paths;
}


main(int argc, char *argv[])
{
   char *hostname;
   char line[MAX];
   int len = 0, check = 1;
   int i = 0;

   if (argc < 2)
      hostname = "localhost";
   else
      hostname = argv[1];
 
   server_init(hostname); 
   // Try to accept a client request
   while(1){
     printf("server: accepting new connection ....\n"); 

     // Try to accept a client connection as descriptor newsock
     length = sizeof(client_addr);
     client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length);
     if (client_sock < 0){
        printf("server: accept error\n");
        exit(1);
     }
     printf("server: accepted a client connection from\n");
     printf("-----------------------------------------------\n");
     inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, serstr, INET_ADDRSTRLEN);
    printf("        IP=%s  port=%d\n", serstr, ntohs(client_addr.sin_port));
     printf("-----------------------------------------------\n");

     // Processing loop: newsock <----> client
     while(1){
       n = read(client_sock, line, MAX);
       if (n==0)
        {
           printf("server: client died, server loops\n");
           close(client_sock);
           break;
        }
      
      // show the line string
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);

      parse(line);

      if(!strcmp(paths[0], "ls"))
      {
        ls_dir(paths[1]);
        n = write(client_sock, "", MAX);
      }
      else if(!strcmp(paths[0], "mkdir"))
      {
        mdir1();
        n = write(client_sock, "", MAX);
      }
      else if(!strcmp(paths[0], "cd"))
      {
        chdir1();
        n = write(client_sock, "", MAX);
      }
      else if(!strcmp(paths[0], "rmdir"))
      {
        rmdir1();
        n = write(client_sock, "", MAX);
      }
      else if(!strcmp(paths[0], "pwd"))
      {
        pwd1();
        n = write(client_sock, "", MAX);
      }
      else if(!strcmp(paths[0], "rm"))
      {
        rm1();
        n = write(client_sock, "", MAX);
      }
      else if(!strcmp(paths[0], "lls"))
        n = write(client_sock, "lls", MAX);
      else if(!strcmp(paths[0], "lmkdir"))
        n = write(client_sock, "lmkdir", MAX);
      else if(!strcmp(paths[0], "lcd"))
        n = write(client_sock, "lcd", MAX);
      else if(!strcmp(paths[0], "lrmdir"))
        n = write(client_sock, "lrmdir", MAX);
      else if(!strcmp(paths[0], "lpwd"))
        n = write(client_sock, "lpwd", MAX);
      else if(!strcmp(paths[0], "lrm"))
        n = write(client_sock, "lrm", MAX);
      else if(!strcmp(paths[0], "get"))
      {
        n = write(client_sock, "get", MAX);
        get();
      }
      else if(!strcmp(paths[0], "put"))
      {
        n = write(client_sock, "put", MAX);
        put();
      }
      printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
      printf("server: ready for next request\n");
      i = 0;
      while(paths[i])
      {
        paths[i] = NULL;
        i++;
      }
    }
 }
}

