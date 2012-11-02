all: pislides

pislides:	pislides.c ../openvg/libshapes.o ../openvg/oglinit.o
	gcc   -Wall -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I../openvg -o pislides pislides.c ../openvg/libshapes.o ../openvg/oglinit.o -L/opt/vc/lib -lGLESv2 -ljpeg
