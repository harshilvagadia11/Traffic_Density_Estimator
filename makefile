all: program.cpp
	g++ program1.cpp -o program1 -pthread -std=c++11 `pkg-config --cflags --libs opencv4`
clean:
	rm program
