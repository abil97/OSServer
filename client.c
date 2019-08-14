#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <sys/select.h>

#define BUFSIZE		4096
#define LINE_MAX 250
#define NUM_THREADS 10
#define BYTES_TO_READ 5000000

// g in the beginning stands for global
int gcsock, gcc;
char *gfilename;
char *gdirectory = "./readers_files";
double grate, gtimeout;
pthread_mutex_t lock, locklock; 
char *ghost;
char *gservice;

int total_number_of_connections = 0;
int total_number_of_timeouts = 0;

/*      
**	Poisson interarrival times. Adapted from various sources
**      L = desired arrival rate
*/
double poissonRandomInterarrivalDelay( double L )
{
    return (log((double) 1.0 - ((double) rand())/((double) RAND_MAX)))/-L;
}

int connectsock( char *host, char *service, char *protocol );

//https://stackoverflow.com/a/43218077
char **reservoir_sample(const char *filename, int count) {
    FILE *file;
    char **lines;
    char buf[LINE_MAX];
    int i, n;

    file = fopen(filename, "r");
    lines = calloc(count, sizeof(char *));
    for (n = 1; fgets(buf, LINE_MAX, file); n++) {
        if (n <= count) {
            lines[n - 1] = strdup(buf);
        } else {
            i = random() % n;
            if (i < count) {
                free(lines[i]);
                lines[i] = strdup(buf);
            }
        }
    }
    fclose(file);

    return lines;
}
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

void *reader_handler(void *threadid){

			long tid;
    		tid = (long) threadid;
			printf("Reader #%lu started execution\n", tid);

			char buf[BUFSIZE];
			char newbuf[BUFSIZE];
			int cc = gcc;
			int csock;
			int sret;
			char *filename = gfilename;
			char *host = ghost;
			char *service = gservice;
			fd_set readfds;
			struct timeval timeout;
			double time_limit = gtimeout;

			timeout.tv_sec = time_limit;
			timeout.tv_usec = 0;
	
			/*	Create the socket to the controller  */
			if(( csock = connectsock( host, service, "tcp" )) == 0){
					fprintf( stderr, "Cannot connect to server.\n" );
					exit( -1 );
			}
			
			total_number_of_connections++;
			strcpy(buf, "READ ");
			strcat(buf, filename);
	
			// Send to the server
			if(write(csock, buf, strlen(buf)) < 0){
				fprintf( stderr, "client write: %s\n", strerror(errno) );
				exit( -1 );
			}	

			FD_ZERO(&readfds);
			FD_SET(csock, &readfds);

			sret = select(8, &readfds, NULL, NULL, &timeout); 
			
			if(sret == 0){	
				printf("Timeout to reader #%lu :( \n", tid);
				total_number_of_timeouts++;
				close(csock);		
				pthread_exit(NULL);
			}
		
			char finalmessage[100];						// Array that will contain the fianl message
			if((cc = read( csock, finalmessage, 100 )) <= 0 ){
                	printf( "The server has gone.\n" );
                    close(csock);
					pthread_exit(NULL);
        	} else {
				
              	buf[cc] = '\0';
				if(finalmessage[0] == 'S' || finalmessage[0] == 's') {
						
						//printf("DATA is: \n");
						char filename_for_this_particular_file[256];
						char snum[64];
						itoa((int) tid, snum, 10);
						strcpy(filename_for_this_particular_file, "./readers_files/readerfile_");
						strcat(filename_for_this_particular_file, snum);
						strcat(filename_for_this_particular_file, ".txt");

						// Actually write to newly created file
						FILE *f1 = fopen(filename_for_this_particular_file, "w");
						if (f1 == NULL){
    						printf("Error opening file!\n");
    						exit(1);
						} else {
							long counter = 0;
							char tempmessage[1000];
							while(counter < BYTES_TO_READ){
								read(csock, tempmessage, 1000);
								fprintf(f1, "%s", tempmessage);
								counter += strlen(tempmessage);
								memset(tempmessage, 0, 1000);
							}
							printf("Server says to reader #%lu: SIZE %lu\n", tid, counter);
						}
						fflush(f1);
						fclose(f1);		
					}
			}
			printf("\nReader nubmber %lu finished\n", tid);
			printf("-----------------------------------------------------\n\n");
			close(csock);
			pthread_exit(NULL);
}


void *writer_handler(void *threadid){
	
	pthread_mutex_lock(&lock);

	long tid;
    tid = (long) threadid;

	printf("Writer #%lu started execution\n", tid);
	char buf[BUFSIZE];
	char newbuf[BUFSIZE];
	int cc;
	int	sret;
	int csock;
	char *filename = gfilename;
	char *host = ghost;
	char *service = gservice;
	double time_limit = gtimeout;
	fd_set readfds;
	struct timeval timeout;

	timeout.tv_sec = time_limit;
	timeout.tv_usec = 0;
	
	/*	Create the socket to the controller  */
	if(( csock = connectsock( host, service, "tcp" )) == 0){
			fprintf( stderr, "Cannot connect to server.\n" );
			exit( -1 );
	}
	total_number_of_connections++;								// Add one more connection

	// Write 'WRITE FILENAME' to the server
	char first_message[32];
	strcpy(first_message, "WRITE ");
	strcat(first_message, filename);
	printf("Sending to the server: %s\n", first_message);
	write(csock, first_message, 32);

	FD_ZERO(&readfds);
	FD_SET(csock, &readfds);

	sret = select(8, &readfds, NULL, NULL, &timeout); //SRET!!!
			

			if(sret == 0){	
				printf("Timeout to writer #%lu :( \n", tid);
				total_number_of_timeouts++;
				close(csock);
				pthread_mutex_unlock(&lock);			
				pthread_exit(NULL);
			}
	 			cc = read(csock, buf, 32);
				printf("Server says to client #%lu: %s\n", tid, buf);
				
				if(buf[0] == 'G' && buf[1] == 'O'){
					memset(buf, 0, BUFSIZE);
					char **data;
					char file_to_open[32];
					char part_of_name[10];

					int rn = rand() % 10;
					itoa(rn, part_of_name, 10);
					strcat(part_of_name, ".txt");
					strcat(file_to_open, "./numbers/");
					strcat(file_to_open,part_of_name);

						data = reservoir_sample(file_to_open, 256);
						char newdata[256][2000];

						// Initialize array elements to -1
						for(int i = 0; i < 256; i++){
							strcpy(newdata[i], "-1");
						}

						int i = 0;
   						int count = 0;
   						while(data[i] != NULL){
       						int j = 0;
       						while(data[i][j] != '\0'){
            					newdata[i][j] = data[i][j];
            					j++;
            					count++;
       						}  
       						i++;
   						}
			
					for(int j = 0; j < 200; j++){	
						for(int i = 0; i < 256; i++){
							write(csock, newdata[i], 2000);
						}
					//	memset(newdata, 0, 256); ?????
					}
				}
				printf("Writer #%lu finished execution\n", tid);
				printf("-----------------------------------------------------\n\n");
				close(csock);
				pthread_mutex_unlock(&lock);			
				pthread_exit(NULL);
}


/*
**	Client
*/
int main( int argc, char *argv[] )
{
	char buf[BUFSIZE];
	char *service;		
	char *host = "localhost";
	int rc, cc;
	int csock;
	
    double rate, timeout;
    char *filename;
    char *directory = "./readers_files"; // directory where all newly created files will be stored

	srand(time(NULL));

    if(strcmp(argv[1], "rclient") == 0){
        printf("Welcome to the reader client!\n");
		printf("------------------------------------------\n\n");
        // rclient [host] port rate filename directory timeout
        // ./client rclient port rate timeout

		// ./client rclient host rate filename directory timeout port
        switch(argc) {
            case 8:
                host  = argv[2];
                service = argv[7];
                rate = atof(argv[3]);
                filename = argv[4];
                directory = argv[5];
                timeout = atof(argv[6]);
				break;
            default:
                fprintf(stderr, "Invalid arguments\n");
				printf("./client rclient host rate filename directory timeout port\n");
                exit(-1);
        }

		long t;
		pthread_t threads[NUM_THREADS];

		// SET all global variables
		gfilename = filename;
		gcsock = csock;
		ghost = host;
		gservice = service;
		gtimeout = timeout;

		for (t = 0; t < NUM_THREADS; t++) {
			
			// Reading thread
			rc = pthread_create(&threads[t], NULL, reader_handler, (void *) t);
			if(rc) {
            	printf("ERROR; return code from pthread_create() is %d\n", rc);
            	exit(-1);
        	}
			sleep(poissonRandomInterarrivalDelay(rate));
			//sleep(2);
		}

		for(int i = 0; i < NUM_THREADS; i++){
			pthread_join(threads[i], NULL);
		}
		close(csock);

		printf("\n===============================================================\n\n");
		printf("Total number of connections: %d\n", total_number_of_connections);
		printf("Total number of timeouts: %d\n", total_number_of_timeouts);

		exit(0);
	
	// Writer client starts here
    } else if(strcmp(argv[1], "wclient") == 0) {
        printf("Welcome to the writer client!\n");
		printf("------------------------------------------\n\n");
		switch(argc) {
            case 7:
                host = argv[2];
                service = argv[6];
                rate = atof(argv[3]);
                filename = argv[4];
                timeout = atof(argv[5]);
				break;
            default:
                fprintf(stderr, "Invalid arguments\n");
                exit(-1);
        }

		long t;
		pthread_t threads[NUM_THREADS];

		// SET all global variables
		gfilename = filename;
		ghost = host;
		gservice = service;
		gtimeout = timeout;

		for (t = 0; t < NUM_THREADS; t++) {
			
			// writing thread	
			rc = pthread_create(&threads[t], NULL, writer_handler, (void *) t);
			if(rc) {
            	printf("ERROR; return code from pthread_create() is %d\n", rc);
            	exit(-1);
        	}
			sleep(poissonRandomInterarrivalDelay(rate));
			//sleep(2);
			//pthread_join(threads[t], NULL);
			
		}
		for(int i = 0; i < NUM_THREADS; i++){
			pthread_join(threads[i], NULL);
		}
		//close(csock);
		printf("\n===============================================================\n\n");
		printf("Total number of connections: %d\n", total_number_of_connections);
		printf("Total number of timeouts: %d\n", total_number_of_timeouts);
		exit(0);

    } else {
        fprintf(stderr, "Please specify the type of the client\n");
        exit(-1);
    }

	
	
}


