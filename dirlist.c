#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>

#define PATH_NAME_SIZE 256

void error_check(int value){
	if(value < 0){
		fprintf(stderr, "System call error. Exiting... \n");
		exit(0);
	}
}

int main(int argc, char *argv[]){
	DIR *directory_stream = NULL;//DIR struct of the directory to archive
	struct dirent *catalogue = NULL;//Catalogue of the archive
	struct stat stat_of_object;//Stat of the object used to separate files
	char pathname[256] = {'\0'};//The pathname passed as a packet of 256 bytes
	char *directory_name = NULL;//Name of directory
	int i = 0; //counter
	int return_val = 0;
	
	directory_name = argv[1];//Takes the first argument passed
	directory_stream = opendir(directory_name);//Opens directory
	catalogue = readdir(directory_stream);
	while(catalogue != NULL){
		//Creating the full pathname: BEGIN
		strcpy(pathname, directory_name);
		if(directory_name[strlen(directory_name)] != '/' && directory_name[strlen(directory_name) - 1] != '/'){
			strcat(pathname, "/");
		}
		strcat(pathname, catalogue->d_name);
		//Creating the full pathname: END

		return_val = stat(pathname, &stat_of_object);//Taking the stat of the object
		error_check(return_val);
		if((stat_of_object.st_mode & S_IFMT) == S_IFREG && catalogue->d_name[0] !=  '.'){//IF file is not hidden and is regular
			write(STDOUT_FILENO, pathname, PATH_NAME_SIZE);//Writing the packet
		}
		catalogue = readdir(directory_stream);
		for(i = 0; i < PATH_NAME_SIZE; i++){//Cleaning the pathname
			pathname[i] = '\0';
		}
	}
	return 0;
}
