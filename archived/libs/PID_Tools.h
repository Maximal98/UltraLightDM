/*Get the PID of a process by name
returns "-1" if there is no process found*/
char *getPidByName( char proc_name[128], int verbose );

/*Get the PID of a process by name
while also checking for the given UID
returns "-1" if there is no process found*/
char *getPidByNameUID( char proc_name[128], char *UID, int verbose );