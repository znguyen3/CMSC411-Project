# CMSC 411, Fall 2021, Term project Makefile

all: simulator simulator2

simulator: instruction.h instruction.cpp simulator.cpp
	g++ simulator.cpp -o simulator
simulator2: instruction.h instruction.cpp simulator2.cpp
	g++ simulator2.cpp -o simulator2

run:
	./simulator inst.txt data_segment.txt output.txt
run2:
	./simulator2 inst.txt data_segment.txt output2.txt
	
clean:
	-rm simulator *.o core* 
