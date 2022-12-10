
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <errno.h>
#include <confuse.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <shadow.h>
#include <crypt.h>

#include "exec_process.h"

int main ( int argc, char **argv ) {

	int errnum, verbose;

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

	char ArgvPrep[X][128];
	int ArgvLengthReal;
	char X_Exec[64];
	
	//libConfuse Setup
	static cfg_bool_t Daemonize = cfg_true;
	static char *XServerPath = "/usr/bin/Xorg";
	static char *XSessionPath = NULL;
	
	cfg_opt_t ConfuseOptions[] = {
		CFG_SIMPLE_BOOL( "Daemonize", &Daemonize ),
		CFG_SIMPLE_STR( "XServerPath" , &XServerPath ),
		CFG_SIMPLE_STR( "XSessionPath" , &XSessionPath ),
		CFG_END()
	};
	cfg_t *config;
	config = cfg_init(ConfuseOptions, 0);
	

	// char **new_argv = malloc(ArgvLengthReal * sizeof *new_argv);
	// for ( int i = 0; i < ArgvLengthReal; i++ ) {
	//	new_argv[i] = ArgvPrep[i];
	// }

	
	fclose( ConfigFile );

	printf("[  \033[0;32mOK\033[0m  ] Successfully loaded config file!\n");


	//Display & ncurses

	nc_start:
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
	WINDOW *errorwin = newwin(2, w.ws_row, w.ws_row, w.ws_col);

	box(authwin, 0, 0);
	box(errorwin, 0, 0);
	refresh();

	// move and print in window
	loginscreen:

	char *hostname = malloc(18);

	gethostname( hostname, 18 );
	mvwprintw(authwin, 0, 1, hostname );
	free(hostname);
	wprintw(authwin, " login" );
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
	

	struct spwd *ShadowStruct = getspnam( Username );
	if( ShadowStruct == NULL ) {
		//Nonexistent user?
		return -2;
	}
	char *PasswordHash = ShadowStruct->sp_pwdp;
	char *FinalHash;
	FinalHash = crypt( Password, PasswordHash );
	int AuthTrue = 0;
	
	if( FinalHash == PasswordHash ) {
		//Password is correct!
		AuthTrue = 1;
	}
	free(ShadowStruct);
	free(PasswordHash);
	free(FinalHash);

	
	// YEET that MEAT
	free( Password );

	if ( AuthTrue == 1 ) {
		endwin();
		// success!
	}
	else {
		endwin();
		return 1;
		mvwprintw( authwin, 1, 10, "              " );
		mvwprintw( authwin, 3, 10, "              " );
		mvwprintw( errorwin, 1, 10, "Authentication Failure!" );
		wrefresh( authwin );
		wrefresh( errorwin );

		goto loginscreen;
	}

	struct passwd *UIDStruct = getpwnam( Username );
	uid_t UID = UIDStruct->pw_uid;
	setuid( UID );
	seteuid( UID );
	
	//we'll fix it another day
	char *new_argv[] = {
		XSessionPath,
		NULL
	};

	if ( verbose == 1 ) {
		printf( "[ \033[0;34mINFO\033[0m ] Attempting to start %s\n", new_argv[0] );
	}

	int runp_ret = exec_process( new_argv[0], new_argv, 1);
	if ( runp_ret == -1 ) {
		printf("[ \033[0;31mFAIL\033[0m ] there was an error launching the DE.\n");
		return 1;
	}
	
	if( Daemonize == 1 ) {
		goto nc_start;
	}

	return 0;
}




//high priority
//segment out into seperate source files					NOT DONE
//switch to libConfuse, the Confparser is ok, but still not the best.		NOT DONE

//medium priority
//fancy Login Screen								ALMOST FINISHED
//actual login system								IN PROGRESS

//low priority
//Better Commenting and Documentation						NOT DONE
//what the fuck why am i using strcmp as a hashing algorithm			WONT FIX ( part of oldparser, which is going to be removed. )
//wayland rnd									NOT DONE

//Done
//UID Checking
//Better Config file parsing, 				  <----------------
//!!!add args to the desktop starting shit you moron!!! needs this ^
//oh god fix the shadow code



// illegal forgetti