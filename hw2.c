/*NIKITOPOULOS GIORGOS, MITRO IOANNIS
This programme encrypts a directory and keeps an archive of it with the option -E.
It can decrypt it, export the files and place the files changing their stats to their old ones
with the option -D.*/

#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define MAGIC_NUM_SIZE 9
void error_check(int return_val){
	if(return_val < 0){
		printf("[-]System Call Error...\n");
		printf("[-]Exiting...\n");
		exit(0);
	}else{

	}
}
void decrypt_func(char *dir_name, char *key, char *f_name){
	int p2_pid = 0;
	int p3_pid = 0;
	int return_val = 0;
	int file_desc = 0;
	int file_desc_f[2];
	char magic_num[MAGIC_NUM_SIZE + 1] = {'\0'};
	file_desc = open(f_name, O_RDONLY);
	error_check(file_desc);
	return_val = read(file_desc, &magic_num, MAGIC_NUM_SIZE);
	error_check(return_val);
	if(strcmp(magic_num, "P2CRYPTAR") != 0){
		fprintf(stderr, "Could not find magic number. Exiting...\n");
		exit(0);	
	}
	return_val = close(file_desc);
	error_check(return_val);
	pipe(file_desc_f);
	p2_pid = fork();
	if(p2_pid == 0){
		file_desc = open(f_name, O_RDONLY);	
		lseek(file_desc, 9, SEEK_SET);
		dup2(file_desc, STDIN_FILENO);
		dup2(file_desc_f[1], STDOUT_FILENO);
		close(file_desc_f[0]);
		close(file_desc_f[1]);
		close(file_desc);
		execlp("./p2crypt", "p2crypt", key, NULL);
		exit(1);
	}else{
		p3_pid = fork();
		if(p3_pid == 0){
			dup2(file_desc_f[0], STDIN_FILENO);
			close(file_desc_f[0]);
			close(file_desc_f[1]);
			execlp("./p2unarchive", "p2unarchive", dir_name, NULL);
			exit(1);
		}

	}
	close(file_desc_f[0]);
	close(file_desc_f[1]);
	waitpid(p3_pid, NULL, 0);
	waitpid(p2_pid, NULL, 0);
}
void encrypt_func(char *dir_name, char *key, char *f_name){
	int p2_pid = 0;
	int file_desc = 0;
	int return_val = 0;
	int p3_pid = 0;
	int p4_pid = 0;
	int file_desc_f[2];//First file desc(f)
	int file_desc_s[2];//Second file desc(s)
//Writing the magic number and checking if file already exists: BEGIN
	file_desc = open(f_name, O_CREAT | O_EXCL | O_WRONLY, 0666);
	if(file_desc == -1){
		printf("[+]File already exists.\n");
		printf("[+]Exiting...\n");
		exit(0);
	}else{
		file_desc = open(f_name, O_CREAT | O_WRONLY, 0666);
		error_check(file_desc);
		printf("[+]File didn't exist before.\n");
		printf("[+]Writing magic number.\n");
		return_val = write(file_desc, "P2CRYPTAR", MAGIC_NUM_SIZE);
		error_check(return_val);
		return_val = close(file_desc);
		error_check(return_val);
	}
	
	pipe(file_desc_f);	
	pipe(file_desc_s);
	p2_pid = fork();
	if(p2_pid == 0){
		dup2(file_desc_f[1], STDOUT_FILENO);
		close(file_desc_f[1]);//Closing the file descriptors for the pipes
		close(file_desc_f[0]);//Closing the file descriptors for the pipes
		close(file_desc_s[0]);
		close(file_desc_s[1]);
		execlp("./dirlist", "dirlist", 	dir_name, NULL);
		perror("execlp");
		exit(1);
	}else{
		p3_pid = fork();
		
		if(p3_pid == 0){
			dup2(file_desc_f[0], STDIN_FILENO);
			dup2(file_desc_s[1], STDOUT_FILENO);
			close(file_desc_s[0]);
			close(file_desc_s[1]);
			close(file_desc_f[1]);//Closing the file descriptors for the pipes
			close(file_desc_f[0]);//Closing the file descriptors for the pipes
			execlp("./p2archive", "p2archive", NULL);
			perror("execlp");
			exit(1);
		}else{
			p4_pid = fork();
			if(p4_pid == 0){
				file_desc = open(f_name, O_WRONLY);
				lseek(file_desc, MAGIC_NUM_SIZE, SEEK_SET);
				dup2(file_desc_s[0], STDIN_FILENO);
				dup2(file_desc, STDOUT_FILENO);
				close(file_desc_s[0]);
				close(file_desc_s[1]);
				close(file_desc_f[0]);
				close(file_desc_f[1]);
				close(file_desc);
				execlp("./p2crypt", "p2crypt", key, NULL);
				perror("execlp");
				exit(1);
			}
		}
	}
	close(file_desc_s[0]);
	close(file_desc_s[1]);
	close(file_desc_f[0]);
	close(file_desc_f[1]);
	waitpid(p2_pid, NULL, 0);
	waitpid(p3_pid, NULL, 0);
	waitpid(p4_pid, NULL, 0);

	return;	
}

int main(int argc, char* argv[]){
	char *directory_name = NULL;
	char *key = NULL;
	char *file_name = NULL;//Archive name
	
	//Handling the arguments: BEGIN
	if(argc != 5){
		fprintf(stderr, "[-]Wrong number of arguments...\n");
		fprintf(stderr, "[-]Correct usage: %s -[E or D] [Name of directory] [Alphanumeric key] [Name of file]\n", argv[0]);
		fprintf(stderr, "[-]Exiting...\n");
		exit(0);
	}
	directory_name = argv[2];
	key = argv[3];
	file_name = argv[4];
	if(strcmp(argv[1], "-E") == 0){
		encrypt_func(directory_name, key, file_name);
	}else if(strcmp(argv[1], "-D") == 0){
		decrypt_func(directory_name, key, file_name);
	}	
	//Handling the arguments: END
	return 0;
}
