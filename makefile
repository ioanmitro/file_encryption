all: hw2 dirlist p2archive p2crypt p2unarchive

hw2: hw2.c
	gcc -Wall -g hw2.c -o hw2
	@echo Making hw2 
dirlist: dirlist.c
	gcc -Wall -g dirlist.c -o dirlist
	@echo Making dirlist
p2archive: p2archive.c
	gcc -Wall -g p2archive.c -o p2archive
	@echo Making p2archive	
p2crypt: p2crypt.c
	gcc -Wall -g p2crypt.c -o p2crypt
	@echo Making p2crypt
p2unarchive: p2unarchive.c
	gcc -Wall -g p2unarchive.c -o p2unarchive
	@echo Making p2unarchive
clean:
	rm hw2 dirlist p2archive p2crypt p2unarchive
