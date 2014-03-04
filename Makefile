all:
	g++ `pkg-config opencv --cflags --libs` projet.cpp -o projet
