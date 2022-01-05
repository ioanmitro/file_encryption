#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAGIC_NUM_SIZE 9
#define PACKET_SIZE 512
#define PATH_NAME_SIZE 256

struct header_struct{
	int path_length;
	struct timespec last_access;
	struct timespec last_mod;
	mode_t mode;
	off_t offset;
};

typedef struct header_struct header_T;

void error_check(int call_int){//Checks for system call errors
        if(call_int == -1){
                printf("[-]A system call error occured.\n");
                printf("[-]Exiting...\n");
                exit(0);//Exiting if a system call error happened
        }
        return;
}


char *transfer_buffer(char *name, int pos, int size){//Gets data from a file at pos and as big as size. Returns pointer pointing to data
        char *buffer = NULL;
        int file_desc = 0;
        int return_val = 0;//Just used to check the return of the system calls in order to search for errors

        buffer = malloc(sizeof(char) * size);
        if(buffer == NULL){
                printf("A malloc failed...Exiting...\n");
                exit(0);
        }
        file_desc = open(name, O_RDONLY);
        error_check(file_desc);
        return_val = lseek(file_desc, pos, SEEK_SET);
        error_check(return_val);
        return_val = read(file_desc, buffer, size);
        error_check(return_val);
        return_val = close(file_desc);
        error_check(return_val);

        return(buffer); 
}
 
void write_file(char *path, int file_size){
	int packet_size;//Size of packet to be transfered
	int remaining;//Size of the remaining packet to be transfered
	int times_it_cycles;//Times the packet transferring loop will cycle
	char *string_to_transfer = NULL;//Buffer of data to be transferred
	int i = 0;//Counter
	int return_val = 0;//Error checking return value

	remaining = file_size % PACKET_SIZE;
	times_it_cycles = file_size / PACKET_SIZE + 1; 	
	do{//Moves the packets and appends them at the end of the file to export        
		packet_size = 0;
		if(i == times_it_cycles - 1 || times_it_cycles == 0){
			packet_size = remaining;
		}else
		{
			packet_size = PACKET_SIZE;
		}
		string_to_transfer = transfer_buffer(path, (i * PACKET_SIZE), packet_size);
		return_val = write(STDOUT_FILENO, string_to_transfer, packet_size);//Appends the packet
		error_check(return_val);
		free(string_to_transfer);
		string_to_transfer = NULL;//New string to transfer every time
		i++;
	}while(i < times_it_cycles);
}

int write_header(char path[PATH_NAME_SIZE]){
	struct stat stat_return;
	header_T header;
	char *pathname = NULL;
	int return_val = 0;
		
	return_val = stat(path, &stat_return);
	error_check(return_val);
	//Read the return of the stat functions and save results
	header.path_length = strlen(path);
	
	pathname = malloc(header.path_length * sizeof(char));
	if(pathname == NULL){
		fprintf(stderr, "Memory allocation failed.\n");
		exit(0);
	}
	//Construcint the header: BEGIN
	strcpy(pathname, path);

	header.last_access = stat_return.st_atim;
	header.last_mod = stat_return.st_mtim;
	header.mode = stat_return.st_mode;
	header.offset = stat_return.st_size;
	//Constructing the header: END
	//Writing the complete header data: BEGIN
	return_val = write(STDOUT_FILENO, &header, sizeof(struct header_struct));//Writting the header
	error_check(return_val);
	write(STDOUT_FILENO, pathname, header.path_length);//Writing the name
	error_check(return_val);
	//Writing the complete header data: END
	
	free(pathname);
	
	return(header.offset);
}

int main(int argc, char *argv[]){
	int return_val = 0;//For error checking
	char path[PATH_NAME_SIZE] = {'\0'};//Pathname taken by dirlist
	long int file_len = 0;
	int i = 0;
	
	while(1){
		return_val = read(STDIN_FILENO, path, PATH_NAME_SIZE); 
		if(return_val == 0 || return_val == -1){
			break;
		}
		if(return_val > 0 && return_val <  PATH_NAME_SIZE - 1){//Fixing the pathname if less bytes received
			i = return_val;
			for(; i < PATH_NAME_SIZE; ){
				i += read(STDIN_FILENO, &path[i], PATH_NAME_SIZE - i);
			}	
		}

		file_len = write_header(path);
		write_file(path, file_len);
	}

	return 0;
}
