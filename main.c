#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

char* getPidByName(char proc_name[128], int verbose) {
	//Setup for Getting the PID
	if( verbose == 1 ) {
		printf("\nAttempting to find PID of ");
		printf(proc_name);
		printf("\n");
	}
	FILE *CurrentProcfile;
	int isPidNumber;
	struct dirent *procdir_ent;
	char *CurrentPID;
	char FullPath[60] = "This is an Error!";
	char CurrentName[128];
	int PID_counter = 0;
	int compreturn;
	//open /proc/
	//TODO: Error Handling here.
	DIR *procdir = opendir("/proc");
	if (procdir) {
		while ((procdir_ent = readdir(procdir)) != NULL) {
			CurrentPID = procdir_ent->d_name; 
			isPidNumber = isdigit(*CurrentPID);
			if (isPidNumber != 0) {
				PID_counter++;
				strcpy(FullPath, "/proc/");
				strcat(FullPath, CurrentPID);
				strcat(FullPath, "/comm");
				CurrentProcfile = fopen(FullPath, "r");
				if ( NULL == CurrentProcfile ) {
					printf( "Couldn't open file in /proc/: %s \n", FullPath );
					return "-2";
				}
				
				//read Data of current file in /proc/
				fgets( CurrentName, 128, CurrentProcfile );
				CurrentName[strcspn( CurrentName, "\n" )] = 0;
				fclose( CurrentProcfile );
				
				//Check if DE name is found.
				compreturn = strcmp(CurrentName, proc_name);
				if( compreturn == 0 ) {
					if( verbose == 1 ) {
						printf("Found Desktop after Checking %d Entries in /proc/.\n", PID_counter);
					}

					//Allocate the space for the pid for returning and all
					size_t pid_size = sizeof( procdir_ent->d_name );
					char *pid = malloc( pid_size );
					sprintf(pid, "%s", procdir_ent->d_name );


					closedir(procdir);
					return(pid);
				}
			}
			
		}
		closedir(procdir);
		return "-1" ;
	}

}

extern char **environ;

int run_DE(char *DE_name, int verbose) {
	//Starting DE Setup
	if( verbose == 1 ) {
		printf("Trying to start DE\n");
	}
	pid_t ChildPid;
	int status;
	char *args[] = {DE_name, NULL};
	//Starting and Error handling
	status = posix_spawn(&ChildPid, DE_name, NULL, NULL, args, environ);
	if ( status == 0 ) {
		if( verbose == 1) {
			printf("started DE successfully!\n");
		}
		wait(&status);
	}
	else {
		printf("couldn't start program, returned error code %d \n", errno);
	}

}

int main (int argc, char **argv) {
	// Process arguments
	int verbose = 0;
	if( argc > 2) {
		printf("Error: too many arguments.\n");
		return 1;
	}
	if( argc > 1) {
		if( strcmp(argv[1], "-h" ) == 0 || strcmp(argv[1], "--help" ) == 0 ) {
			printf("UL-DM, there is only one option and its --help or --verbose (or -h and -v repsectivly.)\n");
			return 1;
		}
		else if( strcmp(argv[1], "-v" ) == 0 || strcmp(argv[1], "--verbose" ) == 0 ) {
			printf("Running in Verbose mode.\n");
			verbose = 1;
		}
	}
	//Getting UID
	uid_t RunningUID;
	RunningUID = getuid();
	if( verbose == 1 ) {
		printf("Running UID is: %d \n", RunningUID);
	}
	//checking if the program is running as Root
	if ( RunningUID == 0 ) {
		printf("Running as Root. Quitting. \n");
		return 0;
	}
	//Setup Variables for Starting
	char *Return;
	FILE *ConfigFile;
	char DEProcess[128];
	char DEStarter[128];
	int line = 0;

	//Loading Configuration and Error handling
	if( verbose == 1 ) {
		printf("Loading config file.\n");
	}
	ConfigFile = fopen("/etc/UL-DM/config", "r");
	if( ConfigFile == NULL ) {
		printf("There was an error opening the Config file. Check if it exists and this user has read access to it.\n");
		return 1;
	}
	fgets(DEProcess, 128, ConfigFile);
	fgets(DEStarter, 128, ConfigFile);
	DEProcess[strcspn(DEProcess, "\n")] = 0;
	DEStarter[strcspn(DEStarter, "\n")] = 0;

	//Get PID of DE, if it returns -1, its not running.
	//(this was a huge pain in the ass)
	//And start if its not running.
	Return = getPidByName(DEProcess, verbose);
	if ( strcmp(Return, "-2") == 0 ) {
		return(1);
	}

	if (verbose == 1) {
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
			printf( "%s\n", Return );
		}
		printf("DE is Running. Exiting. \n");
		return 0;
	}
	return 0;
}