#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <pthread.h>

int devansh_cd(char **args,char *line);
int devansh_help(char **args,char *line);
int devansh_exit(char **args,char *line);
int devansh_pwd(char ** args,char *line);
int devansh_echo(char **args,char *line);
int devansh_pinfo(char **args,char *line);
int devansh_ls(char **args,char *line);
void print_details(char dir[],struct dirent *namelist);
int devansh_clock(char **args,char *line);
int devansh_remindme(char **args,char *line);
void *print_reminders();
int devansh_launch(char **args,int priv);

// #define _POSIX_SOURCE
#define devansh_RL_BUFSIZE 1024

char hash_pname[9999][25];

struct reminder{
	int time;
	char msg[1000];
	int displayed;
}remindme_array[10000];

int remindme_i = 0;
char def_str[1000];


void *print_reminders()
{
	int ret_value = 0;
	int current_time;
	
	char start_time[50];
	while(1)
	{
		usleep(100);
		FILE *fp = fopen("/sys/class/rtc/rtc0/since_epoch","r");
		fgets(start_time,50,fp);
		current_time = atoi(start_time);
		for(int i= 0;i<remindme_i;i++)
		{
		//	printf("%d %d\n",remindme_array[i].time,current_time);
			if(remindme_array[i].time <= current_time)
			{
				if(remindme_array[i].displayed == 0)
				{
					printf("\n%s\n",remindme_array[i].msg );
					remindme_array[i].displayed = 1;
				}
			}
		}
		fclose(fp);	
	}
	
}
int devansh_remindme(char **args,char *line)
{
	struct reminder R;
	FILE *fp = fopen("/sys/class/rtc/rtc0/since_epoch","r");
	char start_time[50];
	fgets(start_time,50,fp);
	R.time = atoi(start_time) + atoi(args[1]);
	//printf("%d\n",R.time);
	int i = 3;
	strcpy(R.msg,args[2]);
	while(args[i] != NULL)
	{
		strcat(R.msg,args[i]);
		i++;
	}
	R.msg[strlen(R.msg)] = '\0';
	R.displayed = 0;
	remindme_array[remindme_i++] = R;
	fclose(fp);
	return 1;
}

int devansh_clock(char**args,char *line)
{
	int time = atoi(args[2]);
	int val;
	 strcpy(args[0],"cat");
	// strcpy(args[1],"/sys/class/rtc/rtc0/time");
	 args[2] = NULL;
	char *targs[2] = {"cat","time.txt"};
	char tempfile[255],ch;
	strcpy(tempfile,"time.txt");
	while(1)
	{
		usleep(100);
		strcpy(args[1],"/sys/class/rtc/rtc0/date");
		val = devansh_launch(args,1);
		strcpy(args[1],"/sys/class/rtc/rtc0/time");
		val = devansh_launch(args,1);
		sleep(time);
	}
	free(targs);
	return 1;
}


char *make_line(char str[])
{
	char *ans = malloc(sizeof(char) * 1000);
	char *token;
	token = strtok(str,"/");
	if(strcmp(token,"home") == 0)
	{
		strcpy(ans,"~");
		token = strtok(NULL, "/");
		token = strtok(NULL, "/");
		while(token != NULL)
		{
			strcat(ans,"/");
			strcat(ans,token);
			token = strtok(NULL, "/");
		}
	}
	strcat(ans,"$ ");
	return ans;
}


int devansh_pinfo(char **args, char *line){
	char str[1000];
	char proc[1000];
	strcpy(proc,"/proc/");
	if(args[1] != NULL)
	{
		strcat(proc,args[1]);
	}
	else
	{
		strcat(proc,"self");
	}
	strcpy(str,proc);
	strcat(str,"/status");
	char temp[1000],temp2[1000];
	char * required_prop[] = {
		"Name","State","Pid","VmSize"
	};
	FILE * stat = fopen(str,"r");
	if(stat == NULL)
	{
		printf("No such process exist\n");
		return -1;
	}
	while(fgets(temp,1000,stat) != NULL)
	{
		strcpy(temp2,temp);
		char * token = strtok(temp,":");
		//printf("%s\n\n",token);
		for(int i= 0;i< 4;i++)
		{
			if(strcmp(token,required_prop[i]) == 0)
			{
				printf("%s",temp2);
			}
		}

	}
	char Exe[1000];
	strcpy(Exe,proc);
	strcat(Exe,"/exe");
	readlink("/proc/self/exe",Exe,1000);
	printf("%s\n",Exe);
	return 1;
}




int devansh_cd(char **args,char *line)
{
  if (args[1] == NULL || strcmp(args[1],"~") == 0) {
  	chdir("/home/devansh");
  } else {
    if (chdir(args[1]) != 0) {
      perror("devansh");
    }
  }
  return 1;
}


int devansh_echo(char **args,char *line)
{
	int i = 1;
	while(args[i] != NULL)
	{
		char *ch;
		int j = 0;
		for(j = 0; args[i][j] != '\0';j++)
		{
			if(args[i][j] != '"')
				printf("%c",args[i][j]);
		}
		printf(" ");
		i++;
	}
	printf("\n");
	return 1;
}



int devansh_help(char **args,char *line)
{
  int i;
  printf("Stephen Brennan's devansh\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  // for (i = 0; i < devansh_num_builtins(); i++) {
  //   printf("  %s\n", builtin_str[i]);
  // }

  printf("Use the man command for information on other programs.\n");
  return 1;
}
int devansh_pwd(char **args,char *line)
{
	char cwd[1024];
	getcwd(cwd,sizeof(cwd));
	 printf("%s\n", cwd);
}

int devansh_exit(char **args,char *line)
{
  return 0;
}


int devansh_ls(char **args,char *line)
{
	int hidden = 0;
	int list = 0;
	int i = 1;
	int dir_index = 0;
	int j,n;
	while(args[i] != NULL)
	{
		if(args[i][0] == '-')
		{
			j = 1;
			while(args[i][j] != '\0')
			{
				if(args[i][j] == 'l')
					list = 1;
				if(args[i][j] == 'a')
					hidden = 1;
				j++;
			}
		}
		else
		{
			dir_index = i;
		}
		i++;
	}
	char dir[1000];
	if(dir_index == 0)
		strcpy(dir,".");
	else
		strcpy(dir,args[dir_index]);
	struct dirent **namelist;
	n = scandir(dir, &namelist, NULL, alphasort);
	 if(n < 0)
        {
            perror("scandir");
            return -1;
        }
        else
        {
        	if(list == 0)
        	{
	            while (n--)
	            {
	                if(namelist[n]->d_name[0] == '.')
	                {
	                	if(hidden == 1)
	                	{
	                    	printf("%s\n",namelist[n]->d_name);
	                	}
	                }
	                else
	                	printf("%s\n",namelist[n]->d_name);
	            }
	        }
	        else
	        {
	        	 while (n--)
	            {
	                if(namelist[n]->d_name[0] == '.')
	                {
	                	if(hidden == 1)
	                	{
	                    	print_details(dir,namelist[n]);
	                	}
	                }
	                else
	                	print_details(dir,namelist[n]);
	            }
	        }
        }   
        return 1;
}

void print_details(char dir[],struct dirent *namelist)
{
    struct stat mystat[4];
	char temp[10000];
	unsigned char mod[13];
	struct tm *starttime[2];
	time_t now;
	int year;
	sprintf(temp,"%s/%s",dir,namelist->d_name);
	stat(temp,&mystat[0]);
	printf( (S_ISDIR(mystat[0].st_mode)) ? "d" : "-");
	printf( (mystat[0].st_mode & S_IRUSR) ? "r" : "-");
	printf( (mystat[0].st_mode & S_IWUSR) ? "w" : "-");
	printf( (mystat[0].st_mode & S_IXUSR) ? "x" : "-");
	printf( (mystat[0].st_mode & S_IRGRP) ? "r" : "-");
	printf( (mystat[0].st_mode & S_IWGRP) ? "w" : "-");
	printf( (mystat[0].st_mode & S_IXGRP) ? "x" : "-");
	printf( (mystat[0].st_mode & S_IROTH) ? "r" : "-");
	printf( (mystat[0].st_mode & S_IWOTH) ? "w" : "-");
	printf( (mystat[0].st_mode & S_IXOTH) ? "x" : "-");
	printf(" \t%d",(int)mystat[0].st_nlink);
	printf(" \t%s", getpwuid(mystat[0].st_uid)->pw_name);
	printf(" \t%s",getgrgid(mystat[0].st_gid)->gr_name);
	printf(" \t%lld",(long long)mystat[0].st_size);
	time(&now);
	year = localtime(&now)->tm_year;
	starttime[1] = localtime(&mystat[0].st_ctime);
	if(starttime[1]->tm_year == year)
		strftime(mod,13,"%b %e %R",starttime[1]);
	else
		strftime(mod,13,"%b %e %Y",starttime[1]);
	printf(" \t%s",mod );
	printf(" \t%ld",mystat[0].st_mtime);
	printf(" \t%s\n",namelist->d_name);
}

int devansh_launch(char **args,int priv)
{
  pid_t pid;
  int status;
  int n = 0;
  while(args[n] != NULL)
  	n++;

  pid = fork();
  if (pid == 0) {
  	// Child process
    if (execvp(args[0], args) == -1 && priv == 0) {
      perror("devansh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0 && priv == 0) {
    // Error forking
    perror("devansh");
   }
  else {
    // Parent process
   strcpy(hash_pname[pid],args[0]);
    if(strcmp(args[n-1],"&") != 0)
	    do {
	      waitpid(pid, &status, WUNTRACED);
	    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int devansh_execute(char **args,char *line)
{
  int i;

  if (args[0] == NULL) {
        return 1;
  }
  if(strcmp(args[0],"pinfo") == 0)
  	return devansh_pinfo(args,line);
  else if(strcmp(args[0],"cd") == 0)
  	return devansh_cd(args,line);
  else if(strcmp(args[0],"help") == 0)
  	return devansh_help(args,line);
  else if(strcmp(args[0],"exit") == 0)
  	return devansh_exit(args,line);
  else if(strcmp(args[0],"pwd") == 0)
  	return devansh_pwd(args,line);
  else if(strcmp(args[0],"echo") == 0)
  	return devansh_echo(args,line);
  else if(strcmp(args[0],"ls") == 0)
  	return devansh_ls(args,line);
  else if(strcmp(args[0],"clock") == 0)
  	return devansh_clock(args,line); 
  else if(strcmp(args[0],"remindme") == 0)
  	return devansh_remindme(args,line);
  return devansh_launch(args,0);
}


char *devansh_read_line(void)
{
  int bufsize = devansh_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "devansh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += devansh_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "devansh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define devansh_TOK_BUFSIZE 64
#define devansh_TOK_DELIM " \t\r\n\a"

char **devansh_split_line(char *line)
{
  int bufsize = devansh_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "devansh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, devansh_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += devansh_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
    	free(tokens_backup);
        fprintf(stderr, "devansh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, devansh_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void *devansh_loop()
{
//  print_reminders();
  char *line;
  char **args;
  int status;
  char str[1000],fline[1000];
  strcpy(fline,"devansh@devansh:");
  do {
  	 getcwd(str,1000);
  	 printf("devansh@devansh:");
    printf("%s",make_line(str));
    line = devansh_read_line();
    args = devansh_split_line(line);
    status = devansh_execute(args,line);

    free(line);
    free(args);
  } while (status);
}





int main(int argc, char **argv)
{
  	pthread_t pth1,pth2;
	pthread_create(&pth1,NULL,print_reminders,NULL);
	devansh_loop();
	pid_t pid;
	int status;
	while((pid = waitpid(-1,&status,WNOHANG)) > 0 )
	{
		if(WIFEXITED(status))
		printf("%s with pid %d exited normally\n",hash_pname[pid],pid);
	}
  return EXIT_SUCCESS;
}