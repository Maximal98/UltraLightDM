udlm: objects/main.o objects/PID_Tools.o
	gcc -o uldm objects/main.o objects/PID_Tools.o

objects/main.o: ../src/main.c
	gcc -c ../src/main.c -o objects/main.o

objects/PID_Tools.o: ../src/libs/PID_Tools.c
	gcc -c ../src/libs/PID_Tools.c -o objects/PID_Tools.o