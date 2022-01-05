#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <utime.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

#define PACKET_SIZE 512
struct header_struct{
        int path_length;
        struct timespec last_access;
        struct timespec last_mod;
        mode_t mode;
        off_t offset;
};
void error_check(int return_val){
	if(return_val == -1){
		fprintf(stderr, "System call error1. Exiting. . .\n");
	}
}

int main(int argc, char *argv[]){
	char *catalogue_name = NULL;
	DIR *directory_pointer = NULL;
	struct header_struct *header = NULL;
	long int file_size = 0;
	int name_size = 0;
	char *name = NULL;
	int file_desc = 0;
	int times_it_cycles = 0;
	int remaining = 0;
	char *buffer_to_move = NULL;
	int packet_size = 0;
	int i = 0;
	int return_val = 0;
	struct timespec time[2];
	char *extracted_file = NULL;
	char *short_name = NULL;
	int k = 0;

	if(argc != 2){
		fprintf(stderr, "Wrong number of arguments.\n");
		fprintf(stderr, "Exiting. . .\n");
		exit(0);
	}
	
	catalogue_name = argv[1];
	directory_pointer = opendir(catalogue_name);
	
	if(directory_pointer != NULL){
		fprintf(stderr, "This directory already exists.\n");
		fprintf(stderr, "Exiting. . .\n");
		exit(0);
	}
	mkdir(catalogue_name, 0777);
	while(1){
		header = malloc(sizeof(struct header_struct)); 
		if(header == NULL){
			fprintf(stderr, "Malloc went south\n");
			fprintf(stderr, "Exiting...\n");
			exit(0);
		}
		return_val = read(STDIN_FILENO, header, sizeof(struct header_struct));
		if(return_val == 0 || return_val == 1){
			exit(0);
		}

		if(return_val > 0 && return_val < sizeof(struct header_struct)){
			for(k = return_val; k < sizeof(struct header_struct) - 1; ){
				k += read(STDIN_FILENO, (header + k), sizeof(struct header_struct) - k);
			}
		}	
		//Getting the data of the file: BEGIN
		time[0] = header->last_access;
		time[1] = header->last_mod;
		file_size = header->offset;
		name_size = header->path_length;		
		//Getting the data of the file: END
		//Getting the name of the file:BEGIN
		name = malloc(sizeof(char) * name_size + 1);
		name[name_size] = '\0';
		return_val = read(STDIN_FILENO, name, (sizeof(char) * name_size));
		if(return_val > 0 && return_val < sizeof(struct header_struct)){
			for(k = return_val; k < name_size - 1; ){
				k += read(STDIN_FILENO, (name + k), name_size - k);
			}
		}
		//Getting the name of the file: END 
		short_name = strrchr(name, '/');
		short_name++;
		extracted_file = malloc((strlen(catalogue_name) * sizeof(char)) + (strlen(name) * sizeof(char)) + 1);
		strcpy(extracted_file, catalogue_name);
		strcat(extracted_file, "/");
		strcat(extracted_file, short_name);
		file_desc = open(extracted_file, O_WRONLY | O_CREAT, 0666);
		times_it_cycles = file_size / PACKET_SIZE + 1;
		remaining = file_size % PACKET_SIZE;
		i = 0;
		do{
			if(i == times_it_cycles - 1 || times_it_cycles == 0){
				packet_size = remaining;
			}else{
				packet_size = PACKET_SIZE;
			}
			buffer_to_move = malloc(sizeof(char) * packet_size);
			return_val = read(STDIN_FILENO, buffer_to_move, (sizeof(char) * packet_size));
			if(return_val > 0 && return_val < packet_size - 1){
				for(k = return_val; k< packet_size - 1; ){
					k += read(STDIN_FILENO, &buffer_to_move[k], packet_size - k );
				}
			}
			write(file_desc, buffer_to_move, (sizeof(char) * packet_size));
			free(buffer_to_move);
			buffer_to_move = NULL;
			i++;
		}while(i < times_it_cycles);
		return_val = futimens(file_desc, time);
		error_check(return_val);
		return_val = close(file_desc);
		error_check(return_val);
		return_val = chmod(extracted_file, header->mode);
		error_check(return_val);
		free(header);
		free(name);
		free(extracted_file);
	}			
	return 0;
}
