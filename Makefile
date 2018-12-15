all: clean main.o readkey.o bigChars.o terminal.o addFunctions.o client server

main.o: source/main.c
	gcc -c -o build/main.o source/main.c
terminal.o: source/terminal.c
	gcc -c -o build/terminal.o source/terminal.c
bigChars.o: source/bigChars.c
	gcc -c -o build/bigChars.o source/bigChars.c
readkey.o: source/readkey.c
	gcc -c -o build/readkey.o source/readkey.c
addFunctions.o: source/addFunctions.c
	gcc -c -o build/addFunctions.o source/addFunctions.c
client:
	gcc -Wall -o client build/main.o build/terminal.o build/bigChars.o build/readkey.o build/addFunctions.o -pthread
server:
	gcc -Wall -o server source/finalserver.c -pthread
clean:
	rm -rf build/*.o
	rm client
	rm server
