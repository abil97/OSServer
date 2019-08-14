#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>


#define	QLEN		5
#define	BUFSIZE		4096
#define NUM_THREADS 10
#define BYTES_TO_READ 5000000

int gsock;
char *filename;
char *filename2; // filename for writer
char *tmp;
pthread_mutex_t lock; 
sem_t resource, rmutex;
int readcount = 0; 

int readcount_sol2 = 0, writecount_sol2 = 0;
sem_t rmutex_sol2, wmutex_sol2, readTry_sol2, resource_sol2;

//http://www.strudel.org.uk/itoa/
char* itoa(int value, char* result, int base) {
		// check that the base if valid
		if (base < 2 || base > 36) { *result = '\0'; return result; }

		char* ptr = result, *ptr1 = result, tmp_char;
		int tmp_value;

		do {
			tmp_value = value;
			value /= base;
			*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
		} while ( value );

		// Apply negative sign
		if (tmp_value < 0) *ptr++ = '-';
		*ptr-- = '\0';
		while(ptr1 < ptr) {
			tmp_char = *ptr;
			*ptr--= *ptr1;
			*ptr1++ = tmp_char;
		}
		return result;
	}

int passivesock( char *service, char *protocol, int qlen, int *rport );

void *write_clients_rp(void* threadid) {

	sem_wait(&resource);
	char		buf[BUFSIZE];
	int			cc;
	int ssock = gsock;
	long tid;
    tid = (long) threadid;
						char answergo[32];
						char newfilename[256];
						for(int i = 0; i < strlen(filename); i++ ){
							newfilename[i] = filename[i];
						}
	
						FILE *f = fopen(newfilename, "w");
						if (f == NULL){
    						printf("Error opening file!\n");
    						exit(1);
						}			

						printf("The client #%lu (writer) says: WRITE %s\n", tid, filename);
						strcat(answergo, "GO ");
						strcat(answergo, filename);
						printf("Sending to client #%lu (writer): %s\n", tid, answergo);
					
						write(ssock, answergo, 32);
						
						char upgraded_buf[256][2000];		
						// Read from socket and actually write to test.txt
						// Also print message: SIZE size data
					
						//printf("DATA is: \n");
						size_t totsize = 0;
						for(int kk = 0; kk < 200; kk++){
						 		for(int jj = 0; jj < 256; jj++){
						 			cc = read(gsock, upgraded_buf[jj], 2000); 
						 			fprintf(f, "%s", upgraded_buf[jj]);
									 totsize += strlen(upgraded_buf[jj]);
						 		} 
								memset(upgraded_buf, 0, 256);
						}
						printf("The client #%lu (writer) says: SIZE %lu data\n", tid, totsize);
						fclose(f);
						memset(upgraded_buf, 0, sizeof(upgraded_buf));
						printf("The client #%lu (writer) finished execution\n\n", tid);
						printf("-----------------------------------------------------------------------------------------------------\n\n");
						fflush(stdout);
						sem_post(&resource);
						pthread_exit(NULL);		
}
		
void *read_clients_rp(void* threadid) {

	sem_wait(&rmutex);
	readcount++;

	if(readcount == 1){
		sem_wait(&resource);
	}
	sem_post(&rmutex);

	char		buf[BUFSIZE];
	char        newbuf[BUFSIZE];
	int			cc;
	//char *filename;
	int sock = gsock;
	long tid;
  tid = (long) threadid;
	//total_number_of_connections++; 			// Increment the total numbers of connections
				// Handle first request "READ filename"
				//char *tmp;			// This holds last token, which should be filename
						char newfilename[256];
						char tempbuf[256];
						char bufread[100][2048];
						char data_to_send[1000];
						char final_message[100];

						strcpy(final_message, "SIZE size");
	
						//filename = tmp;
						for(int i = 0; i < strlen(filename); i++ ){
							newfilename[i] = filename[i];
						}
						
					
						FILE *f = fopen(newfilename, "r");
						if (f == NULL){
    						printf("Error opening file!\n");
    						exit(1);
						}	
						// Start actual reading here:
						// Reading from test.txt
						// Error is here
						// Final message is: READ size data				
						//strcpy(final_message[0], "READ");		// First element is read
						write(sock, final_message, 100);
						
						int counter = 0;

						while(counter < BYTES_TO_READ) {
							fgets(data_to_send, 1000, f);
							
							counter += strlen(data_to_send);
							write(sock, data_to_send, 1000);
							memset(data_to_send, 0, 1000);
							
						}	
						fclose(f);
						fflush(f);
	printf("The client #%lu (reader) finished execution \n", tid);
	printf("-----------------------------------------------------------------------------------------------------\n\n");
	fflush(stdout);
	sem_wait(&rmutex);
		readcount--;
		if(readcount == 0){
			sem_post(&resource);
		}
	sem_post(&rmutex);
	pthread_exit(NULL);
}

void *write_clients_wp(void* threadid) {
	sem_wait(&wmutex_sol2);
	writecount_sol2++;
	if(writecount_sol2 == 1){
		sem_wait(&readTry_sol2);
	}
	sem_post(&wmutex_sol2);

	char		buf[BUFSIZE];
	int			cc;
	int ssock = gsock;
	long tid;
    tid = (long) threadid;
						char answergo[32];
						char newfilename[256];
						for(int i = 0; i < strlen(filename); i++ ){
							newfilename[i] = filename[i];
						}
	
						FILE *f = fopen(newfilename, "w");
						if (f == NULL){
    						printf("Error opening file!\n");
    						exit(1);
						}			

						printf("The client #%lu (writer) says: WRITE %s\n", tid, filename);
						strcat(answergo, "GO ");
						strcat(answergo, filename);
						printf("Sending to client #%lu (writer): %s\n", tid, answergo);
					
						write(ssock, answergo, 32);
						
						char upgraded_buf[256][2000];
				
						// Read from socket and actually write to test.txt
						// Also print message: SIZE size data
					
						//printf("DATA is: \n");
						size_t totsize = 0;
						for(int kk = 0; kk < 200; kk++){
						 		for(int jj = 0; jj < 256; jj++){
						 			cc = read(gsock, upgraded_buf[jj], 2000); 
						 			fprintf(f, "%s", upgraded_buf[jj]);
									 totsize += strlen(upgraded_buf[jj]);
						 		} 
								memset(upgraded_buf, 0, 256);
						}
						printf("The client #%lu (writer) says: SIZE %lu data\n", tid, totsize);
						fclose(f);
						memset(upgraded_buf, 0, sizeof(upgraded_buf));
						printf("The client #%lu (writer) finished execution\n\n", tid);
						printf("-----------------------------------------------------------------------------------------------------\n\n");
						fflush(stdout);
						sem_wait(&resource_sol2);
						sem_post(&resource_sol2);

						sem_wait(&wmutex_sol2);
						writecount_sol2--;
						if(writecount_sol2 == 0){
							sem_post(&readTry_sol2);
						}
						sem_post(&wmutex_sol2);
						pthread_exit(NULL);		
}

void *read_clients_wp(void* threadid) {

	sem_wait(&readTry_sol2);
	sem_wait(&rmutex_sol2);
	readcount_sol2++;

	if(readcount_sol2 == 1){
		sem_wait(&resource_sol2);
	}
	sem_post(&rmutex_sol2);
	sem_post(&readTry_sol2);

	char		buf[BUFSIZE];
	char        newbuf[BUFSIZE];
	int			cc;
	//char *filename;
	int sock = gsock;
	long tid;
  tid = (long) threadid;
	//total_number_of_connections++; 			// Increment the total numbers of connections
				// Handle first request "READ filename"
				//char *tmp;			// This holds last token, which should be filename
						char newfilename[256];
						char tempbuf[256];
						char bufread[100][2048];
						char data_to_send[1000];
						char final_message[100];

						strcpy(final_message, "SIZE size");
	
						//filename = tmp;
						for(int i = 0; i < strlen(filename); i++ ){
							newfilename[i] = filename[i];
						}
						
					
						FILE *f = fopen(newfilename, "r");
						if (f == NULL){
    						printf("Error opening file!\n");
    						exit(1);
						}	
						// Start actual reading here:
						// Reading from test.txt
						// Final message is: READ size data				
						//strcpy(final_message[0], "READ");		// First element is read
						write(sock, final_message, 100);
						
						int counter = 0;

						while(counter < BYTES_TO_READ) {
							fgets(data_to_send, 1000, f);
							
							counter += strlen(data_to_send);
							write(sock, data_to_send, 1000);
							memset(data_to_send, 0, 1000);
							
						}	
						fclose(f);
						fflush(f);
	printf("The client #%lu (reader) finished execution \n", tid);
	fflush(stdout);
	sem_wait(&rmutex_sol2);
		readcount_sol2--;
		if(readcount_sol2 == 0){
			sem_post(&resource_sol2);
		}
	sem_post(&rmutex_sol2);
	pthread_exit(NULL);
}


int main( int argc, char *argv[] ){
	char		buf[BUFSIZE];
	char		*service;
	struct sockaddr_in	fsin;
	int			alen;
	int			msock;
	int			ssock;
	int			rport = 0;
	int			rc, rc1,  cc;
	char *solution_type;
	//pthread_t thread_id;

	sem_init(&resource, 0, 1);
  sem_init(&rmutex, 0, 1);

	sem_init(&resource_sol2, 0, 1);
  sem_init(&rmutex_sol2, 0, 1);
	sem_init(&wmutex_sol2, 0, 1);
  sem_init(&readTry_sol2, 0, 1);

	// ./echoserver  rp [port]
	// ./echoserver  wp [port]
	srand(time(NULL));
	switch (argc){
		case 2:
			// No port? let the OS choose a port and tell the user
			rport = 1;
			solution_type = argv[1];
			break;
		case	3:
			// User provides a port? then use it
			service = argv[2];
			solution_type = argv[1];
			break;
		default:
			fprintf( stderr, "usage: server [port]\n" );
			exit(-1);
	}
	msock = passivesock( service, "tcp", QLEN, &rport );
	if (rport){
		//	Tell the user the selected port
		printf( "server: port %d\n", rport );	
		fflush( stdout );
	}
	if(strcmp(solution_type, "rp") == 0)
			printf("You've choosen reader prefernce solution\n\n");
	else if(strcmp(solution_type, "wp") == 0)
			printf("You've choosen writer prefernce solution\n\n");


	pthread_t wthreads[NUM_THREADS]; 
	pthread_t rthreads[NUM_THREADS]; 
	long t;
	long read_count = 0;
	long write_count = 0;

	for (t = 0; t < 2 * NUM_THREADS; t++){

		if(read_count == NUM_THREADS && write_count == NUM_THREADS){
				printf("\n\nAll clients have finished execution\n\n");
				exit(0);
		}

		alen = sizeof(fsin);
		gsock = accept( msock, (struct sockaddr *)&fsin, &alen ); // This should be in loop also for multiple terminal windows .
		if (gsock < 0){
			fprintf( stderr, "accept: %s\n", strerror(errno) );
			exit(-1);
		}

			ssock = gsock;	

			if((cc = read( ssock, buf, BUFSIZE )) <= 0 )
			{
				printf("The client has gone.\n");
				break;
			}
			else {

				//buf[cc] = '\0';
						// READ case
						if((buf[0] == 'R' || buf[0] == 'r') && read_count < NUM_THREADS){
						
							char *tmp;			// This holds last token, which should be filename
							char *tmpstr[256];	// array of arguments
							int j = 0;
							// read filename
							char newbuf[BUFSIZE];
							strcpy(newbuf, buf);
							char *token = strtok(newbuf, " ");
							while (token) {
								tmp = token;				// This will contain the filename, after loop ends
								tmpstr[j] = token;
   	 							token = strtok(NULL, " ");
								j++;
							}	
							if(strcmp(tmpstr[0], "read") == 0 || strcmp(tmpstr[0], "READ") == 0){
								filename = tmp;
								printf("The client #%lu (reader) says: %s\n",t,  buf);
								memset(buf, 0, BUFSIZE);
								if(strcmp(solution_type, "rp") == 0) 
										rc = pthread_create(rthreads + t, NULL, read_clients_rp, (void *) t);
								else if(strcmp(solution_type, "wp") == 0)
										rc = pthread_create(rthreads + t, NULL, read_clients_wp, (void *) t);
								read_count++;
							}

							// WRITE case
						} else if((buf[0] == 'W' || buf[0] == 'w') && write_count < NUM_THREADS)  {

									char *tmp;			// This holds last token, which should be filename
									char *tmpstr[256];	// array of arguments
									int j = 0;
									char newbuf[BUFSIZE];
									strcpy(newbuf, buf);
									
									char *token = strtok(newbuf, " ");
									while (token) {
										tmp = token;
										tmpstr[j] = token;
   	 									token = strtok(NULL, " ");
										j++;
									}	
									if(strcmp(tmpstr[0], "write") == 0 || strcmp(tmpstr[0], "WRITE") == 0){
												filename = tmp;
												printf("The client #%lu (writer) says: %s\n",t,  buf);
												memset(buf, 0, BUFSIZE);
												if(strcmp(solution_type, "rp") == 0)
														rc1 = pthread_create(rthreads + t, NULL, write_clients_rp, (void *) t);
												else if(strcmp(solution_type, "wp") == 0)
														rc1 = pthread_create(rthreads + t, NULL, write_clients_wp, (void *) t);
												write_count++;
												//printf("In main: thread %ld was created\n", t);
											//	sleep(2);
									}
									
						}
			}
	}
	for(int i = 0; i < NUM_THREADS; i++){
			pthread_join(wthreads[i], NULL);
			pthread_join(rthreads[i], NULL);
		} 
}


