RELEASE = "OCR_PREPROC-0.1"
VOLUME = "Функции Библиотеки"
SOURCES = ocr_preproc.c


all: build-doc

.src-to-man:
	@echo "Extracting manpages from sources..."
	for file in $(SOURCES) ; do \
	  src2man -r $(RELEASE) -v $(VOLUME) $${file}; \
	done
	for file in `ls -1 *.3`; do \
	  { \
	    sed -e 's/\.SH NAME/\.SH ИМЯ/' \
	      -e 's/\.SH SYNOPSIS/\.SH СИНТАКСИС/' \
	      -e 's/\.SH DESCRIPTION/\.SH ОПИСАНИЕ/' \
	      -e 's/\.SH FILE/\.SH ФАЙЛЫ/' $${file}; \
	  } > $${file}.ru ;\
	done
	touch .src-to-man

.man-to-book: .src-to-man
	@echo "Building a book from manpages..."
	bookman -P -c cover.ms *.3.ru > book.ps
	ps2pdf book.ps book.pdf
	touch .man-to-book
	
build-doc: .man-to-book
	@echo "Done!"

clean:
	-rm .src-to-man .man-to-book
	-rm *.3 *.3.ru
	-rm book.ps book.pdf
