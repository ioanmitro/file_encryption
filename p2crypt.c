#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

#define PACKET_SIZE 512

void error_check(int return_val){
	if(return_val == -1){
		fprintf(stderr, "[-] System call error. Exiting. . .\n");
		exit(0);
	}
}

int main(int argc, char *argv[]){
	char *my_secret_key = NULL;
	int key_len = 0;
	char *write_buffer = NULL;
	char read_buffer[PACKET_SIZE];
	int i = 0;
	int j = 0;
	int limit = 0;
	int return_val = 0;
	
	if(argc != 2){
		fprintf(stderr, "[-]Wrong number of arguments.\n");
		fprintf(stderr, "[-]Correct usage: ./%s [key]\n", argv[0]);
		fprintf(stderr, "Exiting. . .\n");
	} 
	my_secret_key = argv[1];//Reading the first argument(key)
	key_len = strlen(my_secret_key);//Size of key
	j = 0;
	while(1){
		limit = read(STDIN_FILENO, &read_buffer, PACKET_SIZE);
		if(limit == 0){
			break;
		}
		error_check(limit);
		write_buffer = malloc(sizeof(char) * limit);
		for(i = 0; i < limit; i++){
			write_buffer[i] = read_buffer[i] ^ my_secret_key[j];
			j++;
			if(j == key_len){
				j = 0;
			}
		}
		return_val = write(STDOUT_FILENO, write_buffer, sizeof(char) * limit);
		if(return_val == -1){
			fprintf(stderr, "Something went wrong with p2crypt\n");
			continue;
		}
		free(write_buffer);
	}

	return 0;
}
