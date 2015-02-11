CC = gcc
CFLAGS =  -Wall -O0 -g `pkg-config --cflags gtk+-x11-2.0`
CFLAGS += -I/usr/include/gdk-pixbuf-1.0
CFLAGS += -g
CFLAGS += ocr_imgproc.o
CFLAGS += ocr_segm.o
CFLAGS += ocr_recog.o
#TARGET = scan2pdf
#OBJECTS = scan2pdf.o
LDFLAGS = `pkg-config --libs gtk+-x11-2.0`
LDFLAGS += -lm

all: scan2pdf

scan2pdf: scan2pdf.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *~*.o scan2pdf
