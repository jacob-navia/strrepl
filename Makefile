all: strrepl-c strrepl-cpp

strrepl-c: strrepl.c
	gcc -o strrepl-c -O2 strrepl.c
strrepl-cpp: strrepl.cpp
	g++ -o strrepl-cpp -O2 strrepl.cpp

