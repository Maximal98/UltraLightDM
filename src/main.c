
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

#define ArgsX 32
#define ArgsY 8

int main ( int argc, char **argv ) {

	int error, verbose;

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

	//Check if config exists
	FILE *ConfigFile = fopen("/etc/uldm/config", "r");
	if( ConfigFile == NULL ) {
		error = errno;
		printf("There was an error opening the Config file. Check if it exists and this user has read access to it. fopen error %d\n", error);
		return 1;
	}
	fclose( ConfigFile );
	
	//libConfuse Setup
	
	//defaults
	static cfg_bool_t Daemonize = cfg_true;
	static char *XServerPath = "/usr/bin/Xorg";
	static char *XSessionPath = NULL;
	
	cfg_opt_t ConfuseOptions[] = {
		CFG_SIMPLE_BOOL( "Daemonize", &Daemonize ),
		CFG_SIMPLE_STR( "XServerPath" , &XServerPath ),
		CFG_STR_LIST( "XServerArgs", "", CFGF_NONE ),
		CFG_SIMPLE_STR( "XSessionPath" , &XSessionPath ),
		CFG_STR_LIST( "XSessionArgs", "", CFGF_NONE ),
		CFG_END()
	};
	
	cfg_t *config;
	config = cfg_init(ConfuseOptions, 0);
	cfg_parse( config, "/etc/uldm/config" );
	
	int XServerArgIncrementor;
	char XServerArgvPrep[ArgsY][ArgsX];
	strncpy( XServerArgvPrep[0], XServerPath, ArgsX );
	for( XServerArgIncrementor = 1; XServerArgIncrementor < cfg_size(config, "XServerArgs") && XServerArgIncrementor <= ArgsY; XServerArgIncrementor++) {
		strncpy( XServerArgvPrep[XServerArgIncrementor], cfg_getnstr(config, "targets", XServerArgIncrementor), ArgsX );
	}
	printf("!\n");
	
	int XSessionArgIncrementor;
	char XSessionArgvPrep[ArgsY][ArgsX];
	strncpy( XSessionArgvPrep[0], XSessionPath, ArgsX );
	for( XSessionArgIncrementor = 1; XSessionArgIncrementor < cfg_size(config, "XServerArgs") && XSessionArgIncrementor <= ArgsY; XSessionArgIncrementor++) {
		strncpy( XSessionArgvPrep[XServerArgIncrementor], cfg_getnstr(config, "targets", XSessionArgIncrementor), ArgsX );
	}
	printf("!\n");

	printf("[  \033[0;32mOK\033[0m  ] Successfully loaded config file!\n");

	char **XServerArgv = malloc( XServerArgIncrementor * sizeof( XServerArgv ) );
	int XServerIncrementor;
	for ( int XServerIncrementor = 0; XServerIncrementor < XServerArgIncrementor; XServerIncrementor++ ) {
		XServerArgv[XServerIncrementor] = XServerArgvPrep[XServerIncrementor];
	}
	//this wont compile, fix later
	free( XServerArgvPrep )
	
	char **XSessionArgv = malloc( XSessionArgIncrementor * sizeof( XServerArgv ) );
	int XSessionIncrementor;
	for ( int XSessionIncrementor = 0; XSessionIncrementor < XSessionArgIncrementor; XSessionIncrementor++ ) {
		XSessionArgv[XSessionIncrementor] = XSessionArgvPrep[XSessionIncrementor];
	}
	
	
	//Display & ncurses
	int LoopEscape1;
	while( LoopEscape1 == 0 ) {
		initscr();
	
		if (has_colors() == FALSE) {
			endwin();
			printf("[ \033[0;31mFAIL\033[0m ] Your terminal does not support color.\n");
			return 1;
		}
	
		struct winsize w;
		ioctl(0, TIOCGWINSZ, &w);
		// nice documentation wtf does tiocgwinsz mean idiot
	
		WINDOW *authwin = newwin(5, 25, w.ws_row/2-4, w.ws_col/2-13);
		WINDOW *errorwin = newwin(2, w.ws_row, w.ws_row, w.ws_col);
	
		box(authwin, 0, 0);
		box(errorwin, 0, 0);
		refresh();
	
		// move and print in window
		uid_t UserUID;
	
		int LoopEscape2 = 0;
		while( LoopEscape2 == 0 ) {
	
			char *hostname = malloc(18);
	
			gethostname( hostname, 18 );
			mvwprintw(authwin, 0, 1, "%s", hostname );
			free(hostname);
			wprintw(authwin, " login" );
			mvwprintw(authwin, 1, 1, "Username:");
			mvwprintw(authwin, 3, 1, "Password:");
	
			wrefresh(authwin);
		
	
		
			wmove(authwin, 1, 10);
			echo();
			
			char *Username = malloc(129);
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
			struct passwd *UIDStruct = getpwnam( Username );
			UserUID = UIDStruct->pw_uid;
			free( UIDStruct );
			
			char *PasswordHash = ShadowStruct->sp_pwdp;
			char *FinalHash;
			FinalHash = crypt( Password, PasswordHash );
			free( Password );
			int AuthTrue = 0;
			
			if( FinalHash == PasswordHash ) {
				//Password is correct!
				AuthTrue = 1;
			}
			free(ShadowStruct);
			free(PasswordHash);
			free(FinalHash);
		
			if ( AuthTrue == 1 ) {
				endwin();
				LoopEscape2 = 1;
				// success!
			}
			else {
				endwin();
				mvwprintw( authwin, 1, 10, "              " );
				mvwprintw( authwin, 3, 10, "              " );
				mvwprintw( errorwin, 1, 10, "Authentication Failure!" );
				wrefresh( authwin );
				wrefresh( errorwin );
		
			}
		}
		
		setuid( UserUID );
		seteuid( UserUID );
		
		//we'll fix it another day
		char *new_argv[] = {
			XSessionPath,
			NULL
		};
	
		if ( verbose == 1 ) {
			printf( "[ \033[0;34mINFO\033[0m ] Attempting to start %s\n", new_argv[0] );
		}
	
		int runp_ret = exec_process( XSessionArgv[0], XSessionArgv, 1);
		if ( runp_ret == -1 ) {
			printf("[ \033[0;31mFAIL\033[0m ] there was an error launching the DE.\n");
			return 1;
		}

		if( Daemonize != 1 ) {
			LoopEscape1 = 1;
		}
	
	}
	
	return 0;
}




//high priority
//segment out into seperate source files					NOT DONE
//add the arguments to libconfuse						NOT DONE

//medium priority
//fancy Login Screen								NOT DONE
//actual login system								NOT DONE

//low priority
//Better Commenting and Documentation						NOT DONE
//wayland rnd									NOT DONE

//Done
//UID Checking
//Better Config file parsing, 				  <----------------
//!!!add args to the desktop starting shit you moron!!! needs this ^
//oh god fix the shadow code
//DO NOT USE GOTO!!!
//switch to libConfuse, the Confparser is ok, but still not the best.
//what the fuck why am i using strcmp as a hashing algorithm			WONT FIX ( part of oldparser, which is going to be removed. )


// illegal forgetti