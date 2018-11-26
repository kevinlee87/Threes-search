all:
	g++ -std=c++11 -O3 -g -Wall -fmessage-length=0 -o Three Threes.cpp
	./Three sample-input.txt output.txt
clean:
	rm 2048