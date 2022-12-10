#include <sys/types.h>
#include <spawn.h>

#include "exec_process.h"

extern char **environ;

int exec_process( char *prog_name, char **argv, int WaitOnProcess ) {

	pid_t ChildPid;
	int status;

	status = posix_spawn(&ChildPid, prog_name, NULL, NULL, argv, environ);

	if ( status != 0 ) {
                // couldn't start, return an error.
                // *-1 to turn the error code into a negative number,
                // so it can be returned and cant be mistaken for the PID of the child,
                // since pids cant be negative.
                if( status > 0 ) {
			return status * -1;
                }
                else {
                	return status;
                }
	}

	return (int)ChildPid;

}