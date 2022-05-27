
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <crypt.h>

#include <sys/ioctl.h>
#include <ncurses.h>

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

	int errnum;

	#define X 128

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
	if ( RunningUID != 0 ) {
		printf("[ \033[0;31mEXIT\033[0m ] Must be run as root. Quitting. \n");
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
	ConfigFile = fopen("/etc/uldm/config", "r");
	if( ConfigFile == NULL ) {
		errnum = errno;
		printf("There was an error opening the Config file. Check if it exists and this user has read access to it. fopen error %d\n", errnum);
		return 1;
	}

	// New Config Parser!

	char DEProcess[128];
	char ArgvPrep[X][128];
	int ArgvLengthReal;

	while( fgets( textbufffer, 128, ConfigFile ) ) {
		textbufffer[ strcspn( textbufffer, "\n" ) ] = 0;

		char *pointerbuffer = textbufffer;
		char *found = "placeholder";

		while( (found = strsep(&pointerbuffer,"=")) != NULL )  {

			int switchint = strcmp( found, "DEStarter" );
			switch ( switchint ) {
				case 0:
					//DEStarter

					char *Stage2found = strsep(&pointerbuffer,"=");
					int ArgAssemblerCounter = 0;

					while( ( found = strsep( &Stage2found, "," ) ) != NULL ) {
						
						// ArgvPrep[ArgAssemblerCounter] = found;
						// NEVER DO THIS ^

						strcpy( ArgvPrep[ArgAssemblerCounter], found );
						ArgAssemblerCounter++;
						

					}
					ArgvLengthReal = ArgAssemblerCounter;
					break;
				
				case -3:
					//DEProcess


					char *found2 = strsep( &pointerbuffer, "=" );
					strcpy( DEProcess, found2 );

					break;
			
				default:
					// Saftey net for garbage data
					break;
			}
		}



	}

	char **new_argv = malloc(ArgvLengthReal * sizeof *new_argv);
	for ( int i = 0; i < ArgvLengthReal; i++ ) {
		new_argv[i] = ArgvPrep[i];
	}

	
	fclose( ConfigFile );

	printf("[  \033[0;32mOK\033[0m  ] Successfully loaded config file!\n");


	//Display & ncurses

	initscr();

	if (has_colors() == FALSE) {
		endwin();
		printf("[ \033[0;31mFAIL\033[0m ] Your terminal does not support color.\n");
		return 1;
	}

	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);

	// creating a window;
	// with height = 15 and width = 10
	// also with start x axis 10 and start y axis = 20
	WINDOW *authwin = newwin(5, 25, w.ws_row/2-4, w.ws_col/2-13);
	refresh();

	box(authwin, 0, 0);

	// move and print in window
	mvwprintw(authwin, 0, 1, "placeholder login");
	mvwprintw(authwin, 1, 1, "Username:");
	mvwprintw(authwin, 3, 1, "Password:");

	wrefresh(authwin);

	char *Username = malloc(129);

	wmove(authwin, 1, 10);
	echo();
	wgetnstr(authwin, Username, 128);
	Username[ strcspn( Username, "\n" ) ] = 0;
	

	char *Password = malloc(129);
	wmove(authwin, 3, 10);
	noecho();
	wgetnstr(authwin, Password, 128);
	Password[ strcspn( Password, "\n" ) ] = 0;

	// aquire salt

	FILE *shadow = fopen("/etc/shadow", "r");
	if( shadow == NULL ) {
		endwin();
		printf("[ \033[0;31mFAIL\033[0m ] couldnt open /etc/shaddow\n");
		return 1;
	}
	char *found1;
	int BreakLoop = 0, Line = 0;
	strcpy( textbufffer, "" );
	while( fgets( textbufffer, 128, shadow ) && BreakLoop == 0 ) {
		Line++;
		textbufffer[ strcspn( textbufffer, "\n" ) ] = 0;
		char *pointerbuffer = textbufffer;

		while( (found1 = strsep(&pointerbuffer,":")) != NULL && BreakLoop == 0 ) { 
			if( strcmp( Username, found1 ) == 0 ){
				BreakLoop = 1;
			}
		}

	}
	fclose( shadow );

	endwin();
	if( found1 == NULL ) {
		//invalid username
		printf("invalid username\n");
		return 0; 
	}

	char *ShadowHash = malloc( sizeof( found1 )+1 );
	strcpy( ShadowHash, found1 );
	printf( "Shaddow line: %d\ncrypt:%s\n", Line, found1 );

	char *pointerbuffer = found1;

	// free(found1);
	int cryptline = 0;
	
	char CryptAlgorythmID[32];
	char CryptSalt[32];
	// char* CryptID = malloc( 64 );
	// char* CryptSalt = malloc( 2 );

	char *found2;
	while( (found2 = strsep(&pointerbuffer,"$")) != NULL ) {
		if( strcmp( found2, "" ) != 0 && cryptline <= 2 ) {
			cryptline++;
			switch ( cryptline ) {
			case 1:
				//Algorythm ID
				strcpy( CryptAlgorythmID, found2);
				break;
			case 2:
				//Salt
				strcpy( CryptSalt, found2);
				break;
			default:
				// Luis
				break;
			}
		}
	}


	char *CryptSaltFinal = malloc(strlen(CryptAlgorythmID)+strlen(CryptSalt)+4);
	sprintf(CryptSaltFinal,"$%s$%s$", CryptAlgorythmID, CryptSalt);


	char *Hash = crypt( Password, CryptSaltFinal );
	printf( "DECODED:%s|\nSHADOW:%s|\n", Hash, ShadowHash );

	if ( strcmp( Hash, ShadowHash ) == 0 ) {
		printf("AUTHENTICATION SUCCESS");
	}






	if ( verbose == 1 ) {
		printf( "[ \033[0;34mINFO\033[0m ] Attempting to start DE %s\n", new_argv[0] );
	}

	int runp_ret = run_prog( new_argv[0], new_argv, 1);
	if ( runp_ret == 1 ) {
		printf("[ \033[0;31mFAIL\033[0m ] there was an error launching the DE.\n");
		return 1;
	}
	return 0;
}


//TODO:
//Better Commenting and Documentation						NOT DONE
//UID Checking									DONE
//Better Config file parsing, 				  <----------------	DONE
//!!!add args to the desktop starting shit you moron!!! needs this ^		DONE
//fancy Login Screen								ALMOST FINISHED
//actuall login system								IN PROGRESS


// illegal forgetti
