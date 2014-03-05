all:
	g++ `pkg-config opencv --cflags --libs` -O3 projet.cpp -o projet
