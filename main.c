#include <dirent.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <spawn.h>
#include <unistd.h>

char* getPidByName(char *proc_name) {
	FILE *fp;
	char *pid = "-1";
	int isPidNumber;
	struct dirent *dir;
	char *E;
	char FullText[60] = "This is an Error!";
	char Name[1000];
	DIR *procdir = opendir("/proc");
	if (procdir) {
		while ((dir = readdir(procdir)) != NULL) {
			E = dir->d_name; 
			isPidNumber = isdigit(*E);
			if (isPidNumber != 0) {
				strcpy(FullText, "/proc/");
				strcat(FullText, E);
				strcat(FullText, "/comm");
				fp = fopen(FullText, "r");
				if ( NULL == fp ) {
					printf("Couldn't open file in /proc/ \n");
					printf(FullText);
					printf("\n");
					return "-2";
				}
				
				/* Read and display data */
				fscanf(fp, "%s", Name);
				fclose(fp);
				if(strcmp(Name, proc_name) == 0) {
					pid = dir->d_name;
				}
			}
			
		}
		closedir(procdir);
		return(pid);
	}

}

extern char **environ;

int main (void) {
	uid_t RunningUID;
	RunningUID = getuid();
	if ( RunningUID == 0 ) {
		printf("Running as Root. Quitting. \n");
		return 0;
	}
	printf("%d", RunningUID);
	char *Return;
	pid_t ChildPid;
	Return = getPidByName("xfce4-session");
	if ( strcmp(Return, "-2") == 0 ) {
		return(1);
	}
	printf("Getting PID was Successfull! Weeee! Its: ");
	printf(Return);
	printf("\n");
	if ( strcmp(Return, "-1") == 0) {
		printf("DE not Running. Starting it. \n");
		posix_spawn(&ChildPid, "startxfce4", NULL, NULL, NULL, environ);
		printf("DE Started. PID is: ");
		printf("%d", ChildPid);
	}
	else {
		printf("DE is Running. Exiting. \n");
		return 0;
	}
	return 0;
}