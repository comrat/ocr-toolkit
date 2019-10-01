CC = gcc
CFLAGS = -O2 -g
RM = rm -f
SOURCES = ocr_meta.c ocr_correct.c ocr_preproc.c ocr_recog.c ocr_segm.c
GLIBCFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -lglib-2.0
GDKCFLAGS = -pthread -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
OTHERCFLAGS = -I./. -lm -lcairo
OBJECTS := $(SOURCES:.c=*.o)

all:
	$(CC) $(CFLAGS) -c $(SOURCES)
	$(CC) $(OBJECTS) $(CFLAGS) example/scan2pdf.c $(GLIBCFLAGS) $(GDKCFLAGS) $(OTHERCFLAGS)

.PHONY: clean
clean:
	-${RM} ${OBJECTS}
