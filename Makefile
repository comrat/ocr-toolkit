CC = gcc
CFLAGS = -O2 -g
RM = rm -f
SOURCES = ocr_meta.c ocr_correct.c ocr_imgproc.c ocr_recog.c ocr_segm.c
OBJECTS  := $(SOURCES:.c=*.o)

all:
	$(CC) $(CFLAGS) -c $(SOURCES)

.PHONY: clean
clean:
	-${RM} ${OBJECTS}
