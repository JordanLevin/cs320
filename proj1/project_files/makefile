all: main.cpp
	g++ -std=c++11 main.cpp -Wall -Wextra -pedantic -o predict

test: all
	./predict traces/short_trace1.txt test1.out

run: all
