all :  diskRead.c diskRead.h
	gcc -g diskRead.c -o diskRead
clean : 
	rm -rf diskRead
