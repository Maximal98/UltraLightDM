
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "libs/PID_Tools.h"

int verbose;
extern char **environ;

int run_prog( char *prog_name, char **argv, int WaitOnProc ) {
	//Starting DE Setup
	if( verbose == 1 ) {
		printf("[ \033[0;34mINFO\033[0m ] Trying to start DE\n");
	}
	pid_t ChildPid;
	int status;
	//Starting and Error handling

	status = posix_spawn(&ChildPid, prog_name, NULL, NULL, argv, environ);
	int error = errno;
	if ( status == 0 ) {
		if( verbose == 1) {
			printf("[  \033[0;32mOK\033[0m  ] started DE successfully!\n");
		}
		if( WaitOnProc == 1) {
			wait( &status );
		}
	}
	else {
		printf("[ \033[0;31mFAIL\033[0m ] couldn't start program, returned error code %d \n", errno);
		return 1;
	}

	return 0; 
}

int main ( int argc, char **argv ) {

	#define X 32

	// Process arguments
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

	//Get the UID
	uid_t RunningUID;
	RunningUID = getuid();
	if( verbose == 1 ) {
		printf("[ \033[0;34mINFO\033[0m ] Current UID is: %d \n", RunningUID);
	}
	//checking if the program is running as Root
	if ( RunningUID == 0 ) {
		printf("[ \033[0;31mFAIL\033[0m ] Running as Root. Quitting. \n");
		return 0;
	}

	char RunningUIDChar[128];
	snprintf( RunningUIDChar, 128, "%d", RunningUID );

	//Config File reading setup
	FILE *ConfigFile;
	char textbufffer[128];
	int Count = 0;

	//Loading Configuration
	if( verbose == 1 ) {
		printf("[ \033[0;34mINFO\033[0m ] Loading config file.\n");
	}
	ConfigFile = fopen("/etc/UL-DM/config", "r");
	if( ConfigFile == NULL ) {
		int error = errno;
		printf("There was an error opening the Config file. Check if it exists and this user has read access to it.\n");
		printf("[ \033[0;31mFAIL\033[0m ]: Error code %n\n", error );
		return 1;
	}

	// New Config Parser!

	char DEProcess[128];
	char ArgvPrep[X][128];

	while( fgets( textbufffer, 128, ConfigFile ) ) {
		textbufffer[ strcspn( textbufffer, "\n" ) ] = 0;

		char *ETHAN_BAD = textbufffer;
		char *found;

		while( (found = strsep(&ETHAN_BAD,"=")) != NULL )  {

			int chaosmonkey = strcmp( found, "DEStarter" );
			switch ( chaosmonkey ) {
				case 0:
					//DEStarter

					char *Found2TheSecondComing = strsep(&ETHAN_BAD,"=");
					int ArgAssemblerCounter;

					while( ( found = strsep( &Found2TheSecondComing, "," ) ) ) {
						
						// ArgvPrep[ArgAssemblerCounter] = found;
						// NEVER DO THIS ^

						strcpy( ArgvPrep[ArgAssemblerCounter], found );
						ArgAssemblerCounter++;
					}

					while( ArgAssemblerCounter < X ) {
						strcpy( ArgvPrep[ArgAssemblerCounter], "" );
						ArgAssemblerCounter++;
					}
					

					break;
				
				case -3:
					//DEProcess


					char *found2 = strsep( &ETHAN_BAD, "=" );
					strcpy( DEProcess, found2 );

					break;
			
				default:
					// Saftey net for garbage data
					break;
			}
		}



	}

	char **new_argv = malloc(X * sizeof *new_argv);
	for ( int i = 0; i < X; i++ ) {
		new_argv[i] = ArgvPrep[i];
	}

	
	fclose( ConfigFile );

	printf("[  \033[0;32mOK\033[0m  ] Successfully loaded config file!\n");

	//Get PID of DE, if it returns -1, its not running.
	//And start if its not running.

	

	char *PID = getPidByNameUID( DEProcess, RunningUIDChar, verbose );
	if ( strcmp( PID, "-2" ) == 0 ) {
		return(1);
	}

	if (verbose == 1) {
		printf("[  \033[0;32mOK\033[0m  ] Getting PID was Successfull! Its: ");
	}
	if ( !strcmp( PID, "-1" ) ) {
		if ( verbose == 1 ) {
			printf("Not running.\n");
		}
		printf("[ \033[0;34mINFO\033[0m ] DE not Running. Starting it. \n");

		char *DEStarter = new_argv[0];

		if ( verbose == 1 ) {
			printf( "[ \033[0;34mINFO\033[0m ] Attempting to start DE %s\n", DEStarter );
		}	



		int runp_ret = run_prog( DEStarter, new_argv, 1);
		if ( runp_ret == 1 ) {
			printf("[ \033[0;31mFAIL\033[0m ] there was an error launching the DE.\n");
			return 1;
		}

	
	}
	else {
		if( verbose == 1) {
			printf( "%s\n", PID );
		}
		printf("[ \033[0;31mEXIT\033[0m ] DE is Running. Exiting. \n");
		return 0;
	}
	return 0;
}


//TODO:
//Better Commenting and Documentation						NOT DONE
//UID Checking									DONE
//Possibly extension loading OR a list of programs to run with args needs ↓	NOT DONE
//Better Config file parsing, 				  <----------------	DONE
//!!!add args to the desktop starting shit you moron!!! needs this ^		DONE

// illegal forgetti