
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "PID_Tools.h"


char* getPidByName( char proc_name[128], int verbose ) {
	//Setup for Getting the PID
	if( verbose == 1 ) {
		printf("[ \033[0;33mINFO\033[0m ] Attempting to find PID of %s\n", proc_name);
	}
	FILE *CurrentProcfile;
	FILE *CurrentLoginUIDFile;
	int isPidNumber;
	struct dirent *procdir_ent;
	char *CurrentPID;
	char CurrentName[128];
	int PID_counter = 0;
	char textbuffer[128];
	//open /proc/
	//TODO: Error Handling here.
	DIR *procdir = opendir("/proc");
	if (procdir) {
		while ((procdir_ent = readdir(procdir)) != NULL) {
			CurrentPID = procdir_ent->d_name; 
			isPidNumber = isdigit(*CurrentPID);
			if (isPidNumber != 0) {
				PID_counter++;
				size_t FullPath_size = sizeof( CurrentPID ) + 14;
				char* FullPath = malloc( FullPath_size );
				sprintf( FullPath, "/proc/%s/comm", CurrentPID );

				CurrentProcfile = fopen(FullPath, "r");
				if ( NULL == CurrentProcfile ) {
					printf( "[ FAIL ] Couldn't open file in /proc/: %s \n", FullPath );
					return "-2";
				}
				
				//read Data of current file in /proc/
				fgets( CurrentName, 128, CurrentProcfile );
				CurrentName[strcspn( CurrentName, "\n" )] = 0;
				fclose( CurrentProcfile );
				
				//Check if DE name is found.
				if( strcmp(CurrentName, proc_name) == 0 ) {
					
					
					if( verbose == 1 ) {
						printf("[  \033[0;32mOK\033[0m  ] Found A Running process after Checking %d Entries in /proc/.\n", PID_counter);
					}
				

					closedir(procdir);

					// turn it into a form thats actually returnable
					char* returnchar = CurrentPID;

					// return it!
					return( returnchar );
				}
			}
			
		}
		closedir(procdir);
		return "-1" ;
	}

}

char* getPidByNameUID( char proc_name[128], char *UID, int verbose ) {
	//Setup for Getting the PID
	if( verbose == 1 ) {
		printf("[ \033[0;34mINFO\033[0m ] Attempting to find PID of %s with UID checking enabled.\n", proc_name);
	}
	FILE *CurrentProcfile;
	FILE *CurrentLoginUIDFile;
	int isPidNumber;
	struct dirent *procdir_ent;
	char *CurrentPID;
	char CurrentName[128];
	char CurrentUID[128];
	int PID_counter = 0;
	char textbuffer[128];
	//open /proc/
	//TODO: Error Handling here.
	DIR *procdir = opendir("/proc");
	if (procdir) {
		while ((procdir_ent = readdir(procdir)) != NULL) {
			CurrentPID = procdir_ent->d_name; 
			isPidNumber = isdigit(*CurrentPID);
			if (isPidNumber != 0) {
				PID_counter++;
				snprintf( textbuffer, 128, "/proc/%s/comm", CurrentPID );

				CurrentProcfile = fopen( textbuffer, "r" );
				if ( NULL == CurrentProcfile ) {
					printf( "[ \033[0;31mFAIL\033[0m ] Couldn't open file in /proc/: %s \n", textbuffer );
					return "-2";
				}
				
				//read Data of current file in /proc/
				fgets( CurrentName, 128, CurrentProcfile );
				CurrentName[strcspn( CurrentName, "\n" )] = 0;
				fclose( CurrentProcfile );
				
				//Check if DE name is found.
				if( strcmp(CurrentName, proc_name) == 0 ) {

					snprintf( textbuffer, 128, "/proc/%s/loginuid", CurrentPID );
					CurrentLoginUIDFile = fopen( textbuffer, "r" );
					if ( NULL == CurrentLoginUIDFile ) {
						printf( "[ \033[0;31mFAIL\033[0m ] Couldn't open file in /proc/: %s \n", textbuffer );
						return "-2";
					}

					fgets( CurrentUID, 128, CurrentLoginUIDFile );
					fclose( CurrentLoginUIDFile );
					
					if( strcmp( CurrentUID, UID ) == 0 ) {
						if( verbose == 1 ) {
							printf("[  \033[0;32mOK\033[0m  ] Found A Running process after Checking %d Entries in /proc/.\n", PID_counter);
						}
					

						closedir(procdir);

						// turn it into a form thats actually returnable
						char* returnchar = CurrentPID;

						// return it!
						return( returnchar );
					}
				}
			}
			
		}
		closedir(procdir);
		return "-1" ;
	}

}