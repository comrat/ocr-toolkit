/* Заголовочные файлы для распознования текста. */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
/* Для загрузки изображений и ренедринга текста. */
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
/* Для флагов. */
#include <getopt.h>

#include "ocr_preproc.h"

/* Функция сравнения строк для быстрой сортировки. */
static int compare(const void *p1, const void *p2)
{
	return strcmp(*(char *const *)p1, *(char *const *)p2);
}

int main(int argc, char **argv)
{
	GdkPixbuf *pbuf;
	GError *gerror;
	/* Cairo var-s. */
	cairo_t *cr;
	cairo_surface_t *dst, *src;
	cairo_format_t format;

	int i = 0, j = 0, k = 0;
	int open_dir = -1;
	int cells_on_side = 10;		// число клеток разбиения по горизонтали изображения
	int f_count = 0;		// число файлов в директории.
	int opt;			// переменная для хранения входных флагов
	int w = 1, h = 1;		// ширина и высота
	int stride = 0, rowpix = 0;

	char *cat_name;
	char **file_list = (char **)malloc(sizeof(char *) * 1000);

	uchar *pix;

	bool use_dilate = false;
	bool use_gauss = false;
	bool use_sauvolas_trshld = false;
	bool use_sobel = false;

	ocr_img_info *img = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	ocr_img_info *grey = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	ocr_img_info *bin = (ocr_img_info *)malloc(sizeof(ocr_img_info));

	GDir *cat;
	gchar *fname;
	gerror = NULL;

	/* Проверка входныз флагов. */
	while((opt = getopt(argc, argv, "df:gost:")) != -1){
		switch(opt){
		case 'd':
			use_dilate = true;
			break;
		case 'f':
			cat = g_dir_open(optarg, 0, gerror);
			cat_name = optarg;
			break;
		case 'g':
			use_gauss = true;
			break;
		case 'o':
			use_sobel = true;
			break;
		case 's':
			use_sauvolas_trshld = true;
			break;
		case 't':
			cells_on_side = atoi(optarg);
			break;
		}
	}

	/**** Получение списка файлов. ****/
	/* Проверка входных данных. */
	if(argc <= 1){
		printf("Please enter directory name.\n ");
		exit(1);
	}

	if(gerror != NULL){
		printf("Could't open catalog.\n");
		exit(1);
	}

	open_dir = (int)g_chdir(cat_name);

	if(open_dir == -1){
		printf("Couldn't open catalog.\n");
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

	/* Сортируем файлы в директории в лексиграф. порядке.*/
	qsort(file_list, f_count, sizeof(char *), compare);

	/**** Обработка каждого файла. ****/
	/* Создаем поверхность для выходного PDF-файла. */
	dst = cairo_pdf_surface_create("result.pdf", w, h);
	cr = cairo_create(dst);

	/* Проверка успешности создания выходного файла. */
	if(cr == NULL){
		printf("Couldn't create \"result.pdf\"");
		exit(1);
	}

	for(k = 0; k < f_count; k++){
		/**** Получение изображение ****/
		printf("Processing file %s:\n", file_list[k]);
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

		cairo_pdf_surface_set_size(dst, w, h);
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

		if(grey == NULL){
			printf("Converting to grey error.");
			continue;
		}

		format = CAIRO_FORMAT_A8;
		stride = cairo_format_stride_for_width(format, w);

		/* Применяем яильтр Гаусса, если стоит соответствующий флаг. */
		if(use_gauss)
			ocr_imgproc_filter_gauss(grey);

		//TODO переделать этот фильтр
		//ocr_imgproc_filter_median(grey, 10);

		bin = ocr_imgproc_threshold_otsu(grey, cells_on_side);	// бинаризуем

		/* Применяем дилатацию при необходимости. */
		if(use_dilate)
			ocr_imgproc_dilate(bin);

		if(bin == NULL){
			printf("Binarization error.");
			continue;
		}

		/**** Сегментация ****/
		ocr_segm_get_comp_by_net(grey, bin, 125, 3);




		/**** Распознание ****/
		/**** Рендеринг текста ****/
		/* Сохранение страницы. */
		src = cairo_image_surface_create_for_data(bin->pix, format, w, h, stride);
		if(src == NULL){
			printf("Couldn't create surface for data.\n");
			continue;
		}
		cairo_set_source_surface(cr, src, 0, 0);
		cairo_paint(cr);
		cairo_show_page(cr);
		cairo_surface_destroy(src);
		g_object_unref(pbuf);
		free(grey);
		free(bin);
	}
	cairo_surface_destroy(dst);
	cairo_destroy(cr);

	/**** Сохранение картинки в файл. ****/

	return 0;
}
