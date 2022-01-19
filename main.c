#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>

char* getPidByName(char *proc_name) {
//	printf("\nAttempting to find PID of ");
//	printf(proc_name);
//	printf("\n");
	FILE *fp;
	char *pid = "-1";
	int isPidNumber;
	struct dirent *dir;
	char *E;
	char FullText[60] = "This is an Error!";
	char Name[100];
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
//	printf("%d", RunningUID);
//	printf("\n");
	char *Return;
	FILE *ConfigFile;
	// char tempstring[256];
	char DEProcess[128];
	char DEStarter[128];
	int line = 0;

	ConfigFile = fopen("/etc/UL-DM/config", "r");
	if( ConfigFile == NULL ) {
		printf("There was an error opening the Config file. Check if it exists and this user has read access to it.\n");
		return 1;
	}
	fgets(DEProcess, 128, ConfigFile);
	fgets(DEStarter, 128, ConfigFile);
	DEProcess[strcspn(DEProcess, "\n")] = 0;
	DEStarter[strcspn(DEStarter, "\n")] = 0;

	Return = getPidByName(DEProcess);
	if ( strcmp(Return, "-2") == 0 ) {
		return(1);
	}
	char *argv[] = {DEStarter, NULL};
	pid_t ChildPid;
	int status;
//	printf("\nGetting PID was Successfull! Its: ");
//	printf(Return);
//	printf("\n");
	if ( strcmp(Return, "-1") == 0) {
		printf("DE not Running. Starting it. \n");
		status = posix_spawn(&ChildPid, DEStarter, NULL, NULL, argv, environ);
		waitpid(ChildPid, &status, 0);
		printf("DE Started. PID is: ");
		printf("%d", ChildPid);
	}
	else {
		printf("DE is Running. Exiting. \n");
		return 0;
	}
	return 0;
}
