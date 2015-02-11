/* Заголовочные файлы для распознования текста. */
#include "ocr_imgproc.h"
#include "ocr_segm.h"
#include "ocr_recog.h"
/* */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
//#include <locale.h>
/* Для загрузки изображений и ренедринга текста. */
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
/* Для флагов. */
#include <getopt.h>

/* Число режимов работы. */
#define MODE_COUNT 6

/* Режимы работы программы. */
typedef enum {
	method_hist,	/* "1" - вывод результатов по методу гистограмм */
	method_stat,	/* "2" - вывод результатов по статистическому методу */
	method_corner,	/* "3" - вывод результатов по методу углов */
	recog_hist,	/* "4" - режим распознания символа по методу гистограмм */
	recog_stat,	/* "5" - режим распознания символа по статистическому методу */
	recog_corner,	/* "6" - режим распознания символа по методу углов */
} mode;

int main(int argc, char **argv)
{
	GdkPixbuf *pbuf;
	GError *gerror;

	int i = 0, j = 0, k = 0, l = 0;
	int open_dir = -1;
	int f_count = 0;		// число файлов в директории.
	int w = 1, h = 1;		// ширина и высота
	int stride = 0;
	int rowpix = 0;
	int opt = -1;
	char c = 0;
	char *cat_name = "";
	char **file_list = (char **)malloc(sizeof(char *) * 1000);

	mode work_mode = method_hist;	/* режим работы программы, по умолчанию метод гистограмм */

	proj_hist *hist = (proj_hist *)malloc(sizeof(proj_hist));

	ocr_img_info *img = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	ocr_img_info *grey = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	ocr_img_info *bin = (ocr_img_info *)malloc(sizeof(ocr_img_info));

	ocr_text_area *text = (ocr_text_area *)malloc(sizeof(ocr_text_area));

	uchar *pix = NULL;

	GDir *cat;
	gchar *fname;
	gerror = NULL;

	//wchar_t c_rus = 0;
	//setlocale(LC_ALL, "ru_RU.UTF8");

	/**** Получение списка файлов. ****/
	/* Проверка входных данных. */
	if(argc <= 1){
		printf("Please enter directory name.\n ");
		exit(1);
	}

	/* Проверка входныз флагов. */
	while((opt = getopt(argc, argv, "f:m:")) != -1){
		switch(opt){
		case 'f':
			cat = g_dir_open(optarg, 0, gerror);
			cat_name = optarg;
			break;
		case 'm':
			work_mode = atoi(optarg);
			if (work_mode > MODE_COUNT)
				work_mode = MODE_COUNT;
			if (work_mode < 1)
				work_mode = 1;
			break;
		}
	}

	if(gerror != NULL){
		printf("Could't open catalog.\n");
		exit(1);
	}

	open_dir = (int)g_chdir(cat_name);

	if(open_dir == -1){
		printf("Couldn't open catalog./\n");
		exit(1);
	}

	/* Считываем список файлов. */
	while((fname = (char *)g_dir_read_name(cat)) != NULL){
		/* Если счетчик файлов превысил 1000, выделяем доп. память. */
		if(f_count > 1000)
			file_list = (char **)realloc(file_list, f_count);
		file_list[f_count] = malloc(sizeof(char) * strlen(fname));
		memcpy(file_list[f_count], fname, strlen(fname));
		f_count++;
	}

	/**** Обработка каждого файла. ****/
	for (k = 0; k < f_count; k++) {
		/**** Получение изображение ****/
		printf("Processing file %s...\n", file_list[k]);
		g_type_init();
		gerror = NULL;

		/* Получаем указатель на пиксели. */
		pbuf = gdk_pixbuf_new_from_file(file_list[k], &gerror);

		if(pbuf == NULL){
			printf("Couldn't open file %s\n", file_list[k]);
			continue;
		}

		if(gerror != NULL){
			printf("Failed to open file %s:%s\n", file_list[k], gerror->message);
			continue;
		}

		/* Запоминаем ширину и высоту изображения и
		изменяем размер */
		w = gdk_pixbuf_get_width(pbuf);
		h = gdk_pixbuf_get_height(pbuf);

		pix = gdk_pixbuf_get_pixels(pbuf);
		stride = gdk_pixbuf_get_rowstride(pbuf);

		/* Определяем есть ли альфа канал в изображении. */
		if(gdk_pixbuf_get_has_alpha(pbuf))
			rowpix = 4;	// для альфа канала
		else
			rowpix = 3;	// для RGB

		/* Заполняем поля информацией об изображении. */	
		img->width = w;
		img->height = h;
		img->pix = pix;
		img->stride = stride;
		img->bytes_for_pix = rowpix;

		/**** Бинаризация ****/
		grey = ocr_imgproc_color2grey(img);
		ocr_imgproc_invert(grey);

		if (grey == NULL) {
			printf("Converting to grey error.");
			continue;
		}
		bin = ocr_imgproc_global_otsu(grey);	// бинаризуем

		if(bin == NULL){
			printf("Binarization error.");
			continue;
		}

		text = ocr_img2textarea(bin);
		if (text == NULL) {
			continue;
		}
		if (work_mode == method_hist) {
			hist = ocr_recog_get_proj_hist(text);
			ocr_recog_normalize_hist(hist, hist->width, hist->height);
			if (hist == NULL) {
				continue;
			}
			printf("\t\t{");
			for (i = 0; i < hist->width - 1; i++) {
				printf("%.2f, ", hist->colls[i]);
				if ((i + 1) % 10 == 0)
					printf("\n\t\t ");
			}
			printf("%.2f},\n\t\t{", hist->colls[hist->width - 1]);

			for (i = 0; i < hist->height - 1; i++) {	
				printf("%.2f, ", hist->lines[i]);
				if ((i + 1) % 10 == 0)
					printf("\n\t\t ");
			}
			printf("%.2f},\n\n\n", hist->lines[hist->height - 1]);
			free(grey);
			free(bin);	
			free(hist);
		} else if (work_mode == method_stat) {
			
		} else if (work_mode == method_corner) {

		} else if (work_mode == recog_hist) {
			c = ocr_recog_char_hist(text);
			printf("result:%c\n", c);
		} else if (work_mode == recog_hist) {
		
		} else if (work_mode == recog_corner) {
		
		}
	}

/*	for(i = 0; i < 256; i++) {
		c_rus = L'я';
		printf("%Lc\n", c_rus);
	}*/

	return 0;
}
