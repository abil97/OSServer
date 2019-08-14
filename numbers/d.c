#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

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


int main(int argc, char *argv[]) {
   // reader();
 

    char u[37];
    char num[1];
    char uuuu[129];
    strcpy(u, "./numbers/");
    for(int i = 0; i < 10; i++){
        itoa(i, num, 10);
        strcpy(uuuu, u);
        strcat(uuuu, num);
        strcat(uuuu, ".txt");
        FILE *f = fopen(uuuu, "w");
        for(int j = 0; j < 10000000; j++){
            fprintf(f, "%c", num[0]);
        }
        fclose(f);
    }
    
}   
