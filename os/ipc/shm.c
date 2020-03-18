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

int main(int argc, char* argv[])
{

	
	
//Get a key I can share with other processes.

	int tokid = 0;
	char *filepath = "/tmp";
	
	key_t key;
	sem_t mutex;
	
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
		if (shmdt(shm) == -1)
			print_error("Unable to detach shared memory", errno);

		if (shmctl(shmid, IPC_RMID, 0)  == -1)
			print_error("Can not validate shared memory", errno);

		exit(0);
	}else{
		while(1){
			int shmlen = strlen(shm);
			printf("Shared memory bytes used: %d\n", shmlen);
	
			printf("Please type your username: ");
			lineSize = getline(&name, &len, stdin);
			//scanf("%ms", &name);
			//printf("name: %s\n", &name);
			printf("Type your message: ");
			lineSize = getline(&cbuf, &len, stdin);
			//scanf("%ms", &cbuf);
			//printf("cbuf: %s\n", &cbuf);
			
			
			name[strlen(name) - 1] = '\0';
			//cbuf[strlen(cbuf) - 1] = '\0';
			//printf("%s\n", name);
			//printf("%s\n", cbuf);
			strcat(name, ": ");
			strcat(name, cbuf);
	
			int cbuflen = strlen(name);
			printf("Length of string to write: %d\n", cbuflen);
			
			sem_trywait(&mutex);
			if (shmlen + cbuflen + 1 < bufsz) {
				//printf("Before write (%lu): %s\n", strlen(shm), shm);
				memcpy(shm + shmlen, name,  cbuflen + 1);
				//printf("After write (%lu): %s\n", strlen(shm), shm);
				printf("%s\n", shm);
			} 
			else {
				printf("Buffer full\n");
			}
	
			//memset(shm, '\0', sizeof(char));
			//free(cbuf);
			//free(name);
			sem_post(&mutex);	
		}
	}	
	exit(0);
}

void print_error(const char* str, int code)
{
	printf("%s: %s\n",str, strerror(code));
	exit(-1);

}

//  END OF FILE
