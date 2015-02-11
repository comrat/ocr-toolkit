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
#include <locale.h>
/* Для загрузки изображений и ренедринга текста. */
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
/* Для флагов. */
#include <getopt.h>

typedef enum {
	otsu_global,
	otsu_adapt,
	otsu_vert
} thrs_mode;

/* Функция сравнения строк для быстрой сортировки. */
static int compare(const void *p1, const void *p2)
{
	return strcmp(*(char *const *)p1, *(char *const *)p2);
}

int main(int argc, char **argv)
{
	GdkPixbuf *pbuf;
	GError *gerror;

	cairo_t *cr;
	cairo_surface_t *dst, *src;
	cairo_format_t format;

	int i = 0, j = 0;
	int k = 0, l = 0, m = 0;
	int open_dir = -1;
	int cells_on_side = 10;		// число клеток разбиения по горизонтали изображения
	int f_count = 0;		// число файлов в директории.
	int opt;			// переменная для хранения входных флагов
	int w = 1, h = 1;		// ширина и высота
	int stride = 0, rowpix = 0;
	int dilate_radius = 0;
	int text_areas_count = 0;
	int lines_count = 0;
	int words_count = 0;
	int chars_count = 0;

	char *cat_name;
	char **file_list = (char **)malloc(sizeof(char *) * 1000);


	wchar_t r_char = 0;
	uchar *pix = NULL;

//	bool use_dilate = false;
	bool use_gauss = false;
	bool use_sauvolas_trshld = false;
	bool use_sobel = false;
	bool all_details = false;

	ocr_img_info* se = NULL;
	ocr_img_info *img = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	ocr_img_info *grey = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	ocr_img_info *bin = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	ocr_img_info *mask = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	ocr_text_area **text_areas = NULL;
	ocr_text_area **text_lines = NULL;
	ocr_text_area **text_words = NULL;
	ocr_text_area **text_chars = NULL;

	ocr_img_info *struct_elem = (ocr_img_info *)malloc(sizeof(ocr_img_info));

	thrs_mode t_mode;

	GDir *cat;
	gchar *fname;
	gerror = NULL;

	/* Включаем для вывода русских символов. */
	setlocale(LC_ALL, "ru_RU.UTF8");

	/* Проверка входныз флагов. */
	while((opt = getopt(argc, argv, "ac:d:f:gost:")) != -1){
		switch(opt){
		case 'a':
			all_details = true;
			break;
		case 'c':	
			cells_on_side = atoi(optarg);
			break;
		case 'd':
			dilate_radius = atoi(optarg);
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
			t_mode = atoi(optarg);
			break;
		}
	}

	/**** Получение списка файлов. ****/
	/* Проверка входных данных. */
	if(argc <= 1){
		printf("Please enter directory name.\n");
		exit(1);
	}

	if(gerror != NULL){
		printf("Could't open catalog.\n");
		exit(1);
	}

	open_dir = (int)g_chdir(cat_name);

	if (open_dir == -1) {
		printf("Couldn't open catalog./\n");
		exit(1);
	}

	/* Считываем список файлов. */
	while ((fname = (char *)g_dir_read_name(cat)) != NULL) {
		/* Если счетчик файлов превысил 1000, выделяем доп. память. */
		if(f_count > 1000)
			file_list = (char **)realloc(file_list, f_count);
		file_list[f_count] = malloc(sizeof(char) * strlen(fname));
		memcpy(file_list[f_count], fname, strlen(fname));
		f_count++;
	}

	/* Сортируем файлы в директории в лексиграф. порядке.*/
	qsort(file_list, f_count, sizeof(uchar *), compare);

	/**** Обработка каждого файла. ****/
	/* Создаем поверхность для выходного PDF-файла. */
	dst = cairo_pdf_surface_create("result.pdf", w, h);
	cr = cairo_create(dst);

	/* Проверка успешности создания выходного файла. */
	if (cr == NULL) {
		printf("Couldn't create \"result.pdf\"");
		exit(1);
	}

	for (k = 0; k < f_count; k++) {
		/**** Получение изображение ****/
		printf("Processing file %s...\n", file_list[k]);
		g_type_init();
		gerror = NULL;

		/* Получаем указатель на пиксели. */
		pbuf = gdk_pixbuf_new_from_file(file_list[k], &gerror);

		if (pbuf == NULL) {
			printf("Couldn't open file %s\n", file_list[k]);
			continue;
		}

		if (gerror != NULL) {
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

		/* Применяем фильтр Гаусса, если стоит соответствующий флаг. */
		if (use_gauss)
			ocr_imgproc_filter_gauss(grey);

		//TODO переделать этот фильтр
		//ocr_imgproc_filter_median(grey, 10);

//		switch (t_mode) {
//		case otsu_global:
//			bin = ocr_imgproc_adapt_otsu(grey, cells_on_side);
//			break;
//		case otsu_adapt:
			bin = ocr_imgproc_vert_otsu(grey, cells_on_side);
//			break;
//		default:
//			bin = ocr_imgproc_global_otsu(grey);
//			break;
//		}
		if (bin == NULL) {
			printf("Binarization error.");
			continue;
		}
		/* Применяем дилатацию при необходимости. */
		if (dilate_radius > 0) {
			//mask = ocr_segm_bloomberg(bin);
			mask = ocr_segm_bloomberg_modify(bin);
			ocr_img_proc_apply_mask(bin, mask, ANDN);

			//se = ocr_imgproc_get_se_nxn(3);
			//ocr_imgproc_morph_open(bin, se, 4);
			//ocr_img_proc_apply_mask(bin, mask, AND);
		}
		/**** Сегментация ****/
		/* Получаем текстовые области. */
		//text_areas = ocr_segm_get_text_areas(bin, 100, &text_areas_count);
		text_areas = ocr_segm_get_text_areas(bin, 100, &text_areas_count);
		if (text_areas == NULL) {
			printf("Ошибка при выделении текстовых областей.\n");
			continue;
		}
/*		ocr_img_info *h_img = NULL;
		for (i = 0; i < text_areas_count; i++) {
			h_img = ocr_segm_get_hough_image(text_areas[i], 1, 1, 85, 95);
			if (h_img == NULL)
				continue;
			cairo_pdf_surface_set_size(dst, h_img->width, h_img->height);
			format = CAIRO_FORMAT_A8;
			stride = cairo_format_stride_for_width(format, h_img->width);
			src = cairo_image_surface_create_for_data(h_img->pix, format, h_img->width, h_img->height, stride);
			if(src == NULL){
				printf("Couldn't create surface for data.\n");
				continue;
			}
			cairo_set_source_surface(cr, src, 0, 0);
			cairo_paint(cr);
			cairo_show_page(cr);
			cairo_surface_destroy(src);
			
			printf("%dx%d, %d\n", h_img->width, h_img->height, h_img->stride);
			free(h_img);
		}
*/
		if (all_details)
			printf("Число текстовых областей: %d.\n", text_areas_count);		

		uchar *t_pix = NULL;
		int t_stride = 0;
		int y = 0;
		int u = 0;
		int m = 0;

		printf("T:%d\n", text_areas_count);
		/* Получаем строки текстовых областей. */
		for(i = 0; i < text_areas_count; i++) {
			text_lines = ocr_segm_stat_lines_area(text_areas[i], &lines_count);
			if (text_lines == NULL) {
				printf("Ошибка при выделении строк из текста.");
			}
	
/*
			t_pix = NULL;
			w = text_areas[i]->width;
			h = text_areas[i]->height;
			if (w <= 0)
				printf("WW:%d\n", w);
			if (h <= 0)
				printf("WW:%d\n", h);
			if (w % WORD_SIZE == 0) 
				t_stride = w;
			else	
				t_stride = w + WORD_SIZE - (w % WORD_SIZE);
			t_pix = (uchar *)realloc(t_pix, sizeof(uchar) * t_stride * h);
		//	printf("%dx%d\n", text_lines[j]->y, text_lines[j]->x);
			for (y = 0; y < h; y++) {
				for (u = 0; u < w; u++) {
					t_pix[y * t_stride + u] = text_areas[i]->pix[y * w + u];
				//	t_pix[y * t_stride + u] = bin->pix[(text_areas[i]->y + y) * bin->stride + text_areas[i]->x + u];
				}
			}
			cairo_pdf_surface_set_size(dst, w, h);
			format = CAIRO_FORMAT_A8;
			//stride = cairo_format_stride_for_width(format, w);
			src = cairo_image_surface_create_for_data(t_pix, format, w, h, t_stride);
			if(src == NULL){
				printf("Couldn't create surface for data.\n");
				continue;
			}
			cairo_set_source_surface(cr, src, 0, 0);
			cairo_paint(cr);
			cairo_show_page(cr);
			cairo_surface_destroy(src);
			free(t_pix);

*/

	
			if (all_details)
				printf("\tЧисло строк: %d.\n", lines_count);
			/* Получаем слова из строк. */
			for (j = 0; j < lines_count; j++) {
				text_words = ocr_segm_stat_words_area(text_lines[j], &words_count);
				if (all_details)
					printf("\t\tЧисло слов в строке %d: %d.\n", j, words_count);
			/*	
				t_pix = NULL;
				w = text_lines[j]->width;
				h = text_lines[j]->height;
				if (w <= 0)
					printf("WW:%d\n", w);
				if (h <= 0)
					printf("WW:%d\n", h);
				if (w % WORD_SIZE == 0) 
					t_stride = w;
				else	
					t_stride = w + WORD_SIZE - (w % WORD_SIZE);
				t_pix = (uchar *)realloc(t_pix, sizeof(uchar) * t_stride * h);
			//	printf("%dx%d\n", text_lines[j]->y, text_lines[j]->x);
				for (y = 0; y < h; y++) {
					for (u = 0; u < w; u++) {
						t_pix[y * t_stride + u] = bin->pix[(text_lines[j]->y + y) * bin->stride + text_lines[j]->x + u];
					}
				}
				cairo_pdf_surface_set_size(dst, w, h);
				format = CAIRO_FORMAT_A8;
				//stride = cairo_format_stride_for_width(format, w);
				src = cairo_image_surface_create_for_data(t_pix, format, w, h, t_stride);
				if(src == NULL){
					printf("Couldn't create surface for data.\n");
					continue;
				}
				cairo_set_source_surface(cr, src, 0, 0);
				cairo_paint(cr);
				cairo_show_page(cr);
				cairo_surface_destroy(src);
				free(t_pix);
			*/
/*				t_pix = NULL;
				w = text_lines[j]->width;
				h = text_lines[j]->height;
				if (w <= 0)
					printf("WW:%d\n", w);
				if (h <= 0)
					printf("WW:%d\n", h);
				if (w % WORD_SIZE == 0) 
					t_stride = w;
				else	
					t_stride = w + WORD_SIZE - (w % WORD_SIZE);
				t_pix = (uchar *)realloc(t_pix, sizeof(uchar) * t_stride * h);
			//	printf("%dx%d\n", text_lines[j]->y, text_lines[j]->x);
				for (y = 0; y < h; y++) {
					for (u = 0; u < w; u++) {
						t_pix[y * t_stride + u] = text_lines[j]->pix[y * w + u];
					}
				}
				cairo_pdf_surface_set_size(dst, w, h);
				format = CAIRO_FORMAT_A8;
				//stride = cairo_format_stride_for_width(format, w);
				src = cairo_image_surface_create_for_data(t_pix, format, w, h, t_stride);
				if(src == NULL){
					printf("Couldn't create surface for data.\n");
					continue;
				}
				cairo_set_source_surface(cr, src, 0, 0);
				cairo_paint(cr);
				cairo_show_page(cr);
				cairo_surface_destroy(src);
				free(t_pix);
*/



				for (l = 0; l < words_count; l++) {
					text_chars = ocr_segm_stat_chars_area(text_words[l], &chars_count);
					ocr_segm_stat_chars_recovery(bin, text_chars, chars_count);
					t_pix = NULL;
					w = text_words[l]->width;
					h = text_words[l]->height;
					if (w <= 0)
						printf("WW:%d\n", w);
					if (h <= 0)
						printf("WW:%d\n", h);
					if (w % WORD_SIZE == 0) 
						t_stride = w;
					else	
						t_stride = w + WORD_SIZE - (w % WORD_SIZE);
					t_pix = (uchar *)realloc(t_pix, sizeof(uchar) * t_stride * h);
					for (y = 0; y < h; y++) {
						for (u = 0; u < w; u++) {
							t_pix[y * t_stride + u] = text_words[l]->pix[y * w + u];
					//		t_pix[y * t_stride + u] = bin->pix[(text_words[l]->y + y) * bin->stride + text_words[l]->x + u];
						}
					}
					cairo_pdf_surface_set_size(dst, w, h);
					format = CAIRO_FORMAT_A8;
					//stride = cairo_format_stride_for_width(format, w);
					src = cairo_image_surface_create_for_data(t_pix, format, w, h, t_stride);
					if(src == NULL){
						printf("Couldn't create surface for data.\n");
						continue;
					}
					cairo_set_source_surface(cr, src, 0, 0);
					cairo_paint(cr);
					cairo_show_page(cr);
					cairo_surface_destroy(src);
					free(t_pix);			
			

					if (all_details)
						printf("\t\t\tСимволов в слове %d: %d.\n", l, chars_count);
					for (m = 0; m < chars_count; m++) {
				//		printf("C %dx%d\n", text_chars[m]->width, text_chars[m]->height);
						/* Распознаем символ. */
						//r_char = ocr_recog_char_hist(text_chars[m]);
						//printf("%lc", r_char);
						t_pix = NULL;
						w = text_chars[m]->width;
						h = text_chars[m]->height;
						if (w <= 0)
							printf("WW:%d\n", w);
						if (h <= 0)
							printf("WW:%d\n", h);
						if (w % WORD_SIZE == 0) 
							t_stride = w;
						else	
							t_stride = w + WORD_SIZE - (w % WORD_SIZE);
						t_pix = (uchar *)realloc(t_pix, sizeof(uchar) * t_stride * h);
						for (y = 0; y < h; y++) {
							for (u = 0; u < w; u++) {
								t_pix[y * t_stride + u] = text_chars[m]->pix[y * w + u];
						//		t_pix[y * t_stride + u] = bin->pix[(text_chars[m]->y + y) * bin->stride + text_chars[m]->x + u];
							}
						}
						cairo_pdf_surface_set_size(dst, w, h);
						format = CAIRO_FORMAT_A8;
						//stride = cairo_format_stride_for_width(format, w);
						src = cairo_image_surface_create_for_data(t_pix, format, w, h, t_stride);
						if(src == NULL){
							printf("Couldn't create surface for data.\n");
							continue;
						}
						cairo_set_source_surface(cr, src, 0, 0);
						cairo_paint(cr);
						cairo_show_page(cr);
						cairo_surface_destroy(src);
						free(t_pix);
				}
				}
			}
			
			if (all_details)
				printf("\t==========================================\n");
		}
		/**** Распознание ****/
		/**** Рендеринг текста ****/
		/* Сохранение страницы. */
		w = bin->width;
		h = bin->height;
		cairo_pdf_surface_set_size(dst, w, h);
		format = CAIRO_FORMAT_A8;
		stride = cairo_format_stride_for_width(format, w);
		src = cairo_image_surface_create_for_data(bin->pix, format, w, h, stride);
		if(src == NULL){
			printf("Не удалось создать поверхность для данных.\n");
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
