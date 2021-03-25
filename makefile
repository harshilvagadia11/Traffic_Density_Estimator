all: program.cpp
	g++ program.cpp -o program -pthread -std=c++17 `pkg-config --cflags --libs opencv4`
clean:
	rm program
