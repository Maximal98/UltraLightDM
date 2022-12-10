/*Run a process. ProgramPath is the Path to the executable, ex /usr/bin/echo
argv is the char array to pass to the program
WaitOnProcess is a value that makes the function wait until the process finishes when set to 1.
when returning a positive value, it is the PID of the child process.
when returning a negative value, it is the the return code of the child */
int exec_process( char *ProgramPath, char **argv, int WaitOnProcess );