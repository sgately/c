/***
 * shm.c - A shared memory demo.
 *
 * Inspired by examples in Advanced Programming in the UNIX Environment, 
 * third edition, pp. 529.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include <semaphore.h>

void print_error(const char*, int);

//Integer Lock used to protect the critical code
sem_t lock;

int main(int argc, char* argv[])
{
	//Intializing the lock
	sem_init(&lock, 0, 1);	
	
//Get a key I can share with other processes.

	int tokid = 0;
	char *filepath = "/tmp";
	
	key_t key;
	
	if ((key = ftok(filepath, tokid)) == -1)
		print_error("Can not create token", errno);

	printf("Token id: %d\n", key);

//Get an id for the shared memory space.

	long bufsz = sysconf(_SC_PAGESIZE);
	printf("Page size: %ld\n", bufsz);

	int shmid;
	if ((shmid = shmget(key, bufsz, IPC_CREAT | 0666)) == -1)
		print_error("Can not create shared memory", errno);

	printf("Shared memory id: %d\n", shmid);

//Attach: Get a pointer to the memory space. 

	char *shm = NULL;
	if ((shm = shmat(shmid, 0, 0)) == (void *)-1)
		print_error("Unable to attach to shared memory", errno);

//Write to the shared memory.
	
	char *cbuf;
	char *name;

	size_t len = 0;
	ssize_t lineSize = 0;
	
 	//Clean up the shared memory pointer and id.	
	if(argc > 1 && 0 == strcmp(argv[1], "reset")){
		//Clear the shared memory regardless if it can be closed or not
		memset(shm, '\0', sizeof(char));
		if (shmdt(shm) == -1)
			print_error("Unable to detach shared memory", errno);

		if (shmctl(shmid, IPC_RMID, 0)  == -1)
			print_error("Can not validate shared memory", errno);

		exit(0);
	}else{
		while(1){
			int shmlen = strlen(shm);
	
			//Get the users name and message
			printf("Please type your username: ");
			lineSize = getline(&name, &len, stdin);
			printf("Type your message: ");
			lineSize = getline(&cbuf, &len, stdin);
			
			//add a null to the name, and concat to "Name: Message" 
			name[strlen(name) - 1] = '\0';
			strcat(name, ": ");
			strcat(name, cbuf);

	
			//Check if the mutex is locked (locked when mutex is zero), or wait until it is unlocked
			sem_wait(&lock);

			//Calculate the length of the whole message and recalculate the shm
			int cbuflen = strlen(name);
			printf("Length of string to write: %d\n", cbuflen);
			shmlen = strlen(shm);
			printf("Shared memory bytes used: %d\n", shmlen);
			
			//Check if the buffer is full
			if (shmlen + cbuflen + 1 < bufsz) {
				//Copy the message to the shared memory and print
				memcpy(shm + shmlen, name,  cbuflen + 1);
				printf("%s\n", shm);
			} 
			else {
				//If the buffer is full, clear the memory and ask to re-input info
				printf("Buffer full\n");
				memset(shm, '\0', sizeof(char));
				printf("Re-enter Message\n");

			}
			
			//Unlock the mutex
			sem_post(&lock);	
		}
	}	
	exit(0);
}

//Helper method to print the error
void print_error(const char* str, int code)
{
	printf("%s: %s\n",str, strerror(code));
	exit(-1);

}

//  END OF FILE
