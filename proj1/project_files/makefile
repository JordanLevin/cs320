all: main.cpp
	g++ -std=c++11 main.cpp -Wall -Wextra -pedantic -g -O4 -o predictors

test: all
	./predictors traces/short_trace1.txt test1.out

run: all
