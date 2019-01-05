#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/ipc.h>
#define MAXSIZE     27
#define BUFFER     80

void die(char *outputMemory)
{
	printf("Can't find the  key %s\n", outputMemory);
}

int main(int argc, char *argv[]) {
	if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL || argv[5] == NULL) {
		printf("Missing args: \n");
		return -1;
	}

	key_t key[4];
	srand((unsigned)time(NULL));
	int	fileDescriptor[4][2];
	for (unsigned int i = 0; i < 4; i++) {
		key[i] = rand() % 5000;
		if (pipe(fileDescriptor[i])) {
			printf("Pipe #%d failed to load!\n", i);
			exit(0);
		}
		else {
			printf("Key: %d, FileDescriptor: %d \n", key[i], fileDescriptor[i]);
		}
	}

	int processId[4];
	int sharedMemoryId;
	char * sharedMemory, *outputMemory;

	for (unsigned int i = 0; i < 4; i++) {

		if ((sharedMemoryId = shmget(key[i], MAXSIZE, IPC_CREAT | 0666)) < 0)
			die("shmget");

		if ((sharedMemory = shmat(sharedMemoryId, NULL, 0)) == (char *)-1)
			die("shmat");

		processId[i] = fork();
		if (processId[i] == 0) {        /* parent */

			char buffer[BUFFER];
			snprintf(buffer, 10, "%d", key[i]);
			char *write_msg = buffer;
			write(fileDescriptor[i][1], write_msg, BUFFER);
			close(fileDescriptor[i][1]);

			//printf("The Shared Memory ID  %d\n", sharedMemoryId);
			printf("Coordinator: forked process with ID %d.\n", (int)getpid());
			printf("Coordinator: wrote sharedMemory %d to pipe (%d bytes)\n", sharedMemoryId, 4);
			execlp("./checker", "./checker", argv[1], argv[2 + i], fileDescriptor[i], (char *)NULL);
			exit(0);
		}


		outputMemory = sharedMemory;
		printf("Coordinator: result %s read from shared memory: %s is divisible by %s \n", outputMemory, argv[2 + i], argv[1], (int)getpid());
		// Detach shareed memory
		shmdt(sharedMemory);
		/* Deallocate the shared memory segment.*/
		shmctl(sharedMemoryId, IPC_RMID, 0);
		
	}

	int returnStatus;

	for (unsigned int i = 0; i < 4; i++) {
		printf("Coordinator: waiting for process [%d]....\n", processId[i]);
		waitpid(processId[i], &returnStatus, 0);
		if (WIFEXITED(returnStatus)) {
			printf("Child exited with RC=%d\n", WEXITSTATUS(returnStatus));
		}
	}

	if (returnStatus == 0)  // terminated without error.  
	{
		printf("Coordinator:  exiting.\n");
	}

	return 0;
}