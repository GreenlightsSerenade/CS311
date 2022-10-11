CC=g++

default: runfile

runfile.o: main.cpp 
	g++ -c main.cpp -o runfile.o 
 
runfile: runfile.o
	$(CC) runfile.o  -o runfile

clean:
	rm -f *.o
	rm -f runfile
