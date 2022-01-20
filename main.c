#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

char* getPidByName(char *proc_name, int verbose) {
	if( verbose == 1 ) {
		printf("\nAttempting to find PID of ");
		printf(proc_name);
		printf("\n");
	}
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
			if( verbose == 1 ) {
				printf("Attempting to open /proc/\n");
			}
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

int run_DE(char *DE_name, int verbose) {
	if( verbose == 1 ) {
		printf("Trying to start DE\n");
	}
	pid_t ChildPid;
	int status;
	char *args[] = {DE_name, NULL};
	status = posix_spawn(&ChildPid, DE_name, NULL, NULL, args, environ);
	if ( status == 0 ) {
		printf("started DE successfully!\n");
		wait(&status);
		printf("%d", errno);
	}
	else {
		printf("couldn't start program, returned error code ");
		printf("%d", errno);
		printf("\n");
	}

}

int main (int argc, char **argv) {
	int verbose = 0;
	if( argc > 2) {
		printf("Error: too many arguments.\n");
		return 1;
	}
	if( strcmp(argv[1], "-h" ) == 0 || strcmp(argv[1], "--help" ) == 0 ) {
		printf("UL-DM, there is only one option and its --help or --verbose (or -h and -v repsectivly.)\n");
		return 1;
	}
	else if( strcmp(argv[1], "-v" ) == 0 || strcmp(argv[1], "--verbose" ) == 0 ) {
		printf("Running in Verbose mode.\n");
		verbose = 1;
	}
	
	uid_t RunningUID;
	RunningUID = getuid();
	if ( RunningUID == 0 ) {
		printf("Running as Root. Quitting. \n");
		return 0;
	}
	printf("%d", RunningUID);
	printf("\n");
	char *Return;
	FILE *ConfigFile;
	char DEProcess[128];
	char DEStarter[128];
	int line = 0;

	if( verbose == 1 ) {
		printf("Loading config file.\n");
	}
	ConfigFile = fopen("/etc/UL-DM/config", "r");
	if( ConfigFile == NULL ) {
		printf("There was an error opening the Config file. Check if it exists and this user has read access to it.\n");
		return 1;
	}
	if( fgets(DEProcess, 128, ConfigFile) != 0) {
		printf("Couldn't read config! failed with error: ");
		printf("%d", errno);
		printf("!\n");
	}
	if( fgets(DEStarter, 128, ConfigFile) != 0) {
		printf("Couldn't read config! failed with error: ");
		printf("%d", errno);
		printf("!\n");
	}
	DEProcess[strcspn(DEProcess, "\n")] = 0;
	DEStarter[strcspn(DEStarter, "\n")] = 0;

	Return = getPidByName(DEProcess, verbose);
	if ( strcmp(Return, "-2") == 0 ) {
		return(1);
	}

	if (verbose = 1) {
		printf("\nGetting PID was Successfull! Its: ");
	}
	if ( strcmp(Return, "-1") == 0) {
		if ( verbose == 1 ) {
			printf("Not running.\n");
		}
		printf("DE not Running. Starting it. \n");
		run_DE(DEStarter, verbose);
	}
	else {
		if( verbose == 1) {
			printf(Return);
			printf("\n");
		}
		printf("DE is Running. Exiting. \n");
		return 0;
	}
	return 0;
}