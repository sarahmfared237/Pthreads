build: 
	gcc -pthread 7374-matrix.c -o 7374-matrix
	gcc -pthread 7374-sort.c -o 7374-sort

clean:
	rm -f 7374-matrix
	rm -f 7374-sort
