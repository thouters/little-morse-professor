linux_morse: hsm.cpp linux.cpp professor.cpp professor.h morsetable.h
	g++ -std=c++11 -g -o linux_morse linux.cpp professor.cpp hsm.cpp 

all: linux_morse

clean:
	rm -f linux_morse

run: linux_morse
	./linux_morse
