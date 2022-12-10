udlm: main exec_process
	gcc -o uldm objects/main.o objects/exec_process.o -lncurses -lcrypt -lconfuse

main:
	gcc ../src/main.c -c -o objects/main.o

exec_process:
	gcc ../src/exec_process.c -c -o objects/exec_process.o