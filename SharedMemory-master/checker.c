#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#define MAXSIZE     27
#define BUFFER     80
void die(char *s)
{
	perror(s);
	exit(1);
}

int main(int argc, char *argv[]) {

	int     numbytes;
	char    line[80];

	int numerator = atoi(argv[1]);
	int denominator = atoi(argv[2]);
	char *fd = argv[3];

	 read((int)fd[0], line, 80);
	//printf("Pipeline process [%s]:\n", line);
	close(fd[0]);


	int shmid;
	key_t key;
	char *shm, *s;

	key = atoi(line);

	if ((shmid = shmget(key, MAXSIZE, IPC_CREAT | 0666)) < 0)
		die("shmget");

	if ((shm = shmat(shmid, NULL, 0)) == (char *)-1)
		die("shmat");

	 

	printf("Checker process [%d]: Starting.\n", (int)getpid());
	printf("Checker process [%d]: read %d bytes containing shm ID %d\n", (int)getpid(), 4, shmid);
	
	if (numerator % denominator == 0) {
		printf("Checker process [%d]: %d *IS* divisible by %d.\n", (int)getpid(), denominator, numerator);
		//Now write what the server put in the memory.
		char * c = "1";
		strcpy(shm, c);
	}
	else
	{
		printf("Checker process [%d]: %d *IS NOT* divisible by %d.\n", (int)getpid(), denominator, numerator);
		printf("Checker process [%d]: wrote result (0) to shared memory.\n", (int)getpid());
		//Now write what the server put in the memory.
		char * c = "0";
		strcpy(shm, c);
	}

	/* Detach the shared memory segment. */
	shmdt(shm);
	return 0;
}