all:
	g++ -std=c++0x -pthread `pkg-config opencv --cflags` projet.cpp -o projet `pkg-config opencv --libs`
