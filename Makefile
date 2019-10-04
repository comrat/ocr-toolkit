CC = gcc
CFLAGS = -O2 -g
RM = rm -f
SOURCES = ocr_meta.c ocr_preproc.c
EXAMPLE = scan2pdf.c
GDKCFLAGS = -pthread -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -lgdk_pixbuf-2.0 -lgobject-2.0 -lglib-2.0 -I./. -lm -lcairo

OBJECTS := $(SOURCES:.c=*.o)
BINARY := $(EXAMPLE:.c=)

scan2pdf:
	$(CC) $(CFLAGS) -c $(SOURCES)
	$(CC) $(OBJECTS) $(CFLAGS) example/${EXAMPLE} $(GDKCFLAGS) $(OTHERCFLAGS) -o ${BINARY}

.PHONY: clean
clean:
	-${RM} ${OBJECTS}
	-${RM} ${BINARY}
