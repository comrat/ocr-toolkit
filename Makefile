CC = gcc
CFLAGS = -fPIC -Wall -Wextra -O2 -g
LDFLAGS = -shared
RM = rm -f
TARGET_LIB = libocrtoolkit.0.9.9.so
SRCS = ocr_meta.c ocr_correct.c ocr_imgproc.c ocr_recog.c ocr_segm.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
	all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
		$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
		$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)
