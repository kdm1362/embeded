main:	main_official.o opencv_new.o serial.o
	g++ -o main_official.cpp opencv_new.cpp serial.c

opencv_new.o:	opencv_new.cpp
	g++ -c opencv_new.cpp

serial.o:	serial.c
	g++ -o serial.c -lwiringPi

main_official.o:
	g++ -c main_offifial.cpp
