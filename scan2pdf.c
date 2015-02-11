/* Заголовочные файлы для распознования текста. */
#include "ocr_meta.h"
#include "ocr_imgproc.h"
#include "ocr_segm.h"
#include "ocr_recog.h"
/* Прочее. */
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

/* Список типов бинаризации. */
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
	int cells_on_side = 10;		/* число клеток разбиения по горизонтали изображения */
	int f_count = 0;		/* число файлов в директории */
	int opt;			/* переменная для хранения входных флагов */
	int w = 1, h = 1;		/* ширина и высота */
	int stride = 0, rowpix = 0;
	int dilate_radius = 0;
	int text_areas_count = 0;
	int lines_count = 0;
	int words_count = 0;
	int chars_count = 0;
	int op_res = -1;		/* Результат операции. */ 

	char *cat_name;
	char **file_list = (char **)malloc(sizeof(char *) * 1000);


	wchar_t r_char = 0, next_char = 0;
	wchar_t *r_word = NULL;
	uchar *pix = NULL;

	bool use_gauss = false;
	bool all_details = false;
	bool pic_segm = false;

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
	while((opt = getopt(argc, argv, "ac:gf:pt:")) != -1){
		switch(opt){
		case 'a':
			all_details = true;
			break;
		case 'c':	
			cells_on_side = atoi(optarg);
			break;
		case 'p':
			pic_segm = true;
			break;
		case 'f':
			cat = g_dir_open(optarg, 0, gerror);
			cat_name = optarg;
			break;
		case 'g':
			use_gauss = true;
			break;
		case 't':
			t_mode = atoi(optarg);
			break;
		}
	}

	/**** Получение списка файлов. ****/
	/* Проверка входных данных. */
	if(argc <= 1){
		printf("Укажите имя обрабатываемой директории.\n");
		exit(1);
	}

	if(gerror != NULL){
		printf("Не удалось открыть каталог.\n");
		exit(1);
	}

	open_dir = (int)g_chdir(cat_name);

	if (open_dir == -1) {
		printf("Не удалось открыть каталог.\n");
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
		printf("Не удалось создать \"result.pdf\"");
		exit(1);
	}

	for (k = 0; k < f_count; k++) {
		/**** Получение изображение ****/
		printf("Обработка файла %s...\n", file_list[k]);
		g_type_init();
		gerror = NULL;

		/* Получаем указатель на пиксели. */
		pbuf = gdk_pixbuf_new_from_file(file_list[k], &gerror);

		if (pbuf == NULL) {
			printf("Не удалось получить пиксели из файла %s\n", file_list[k]);
			continue;
		}

		if (gerror != NULL) {
			printf("Не удалось открыть файл %s:%s\n", file_list[k], gerror->message);
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
		grey = ocr_img_info_malloc(w, h, GREY);
		if (bin == NULL) {
			printf("Не удалось выделить память для изображения.\n");
			continue;
		}
		op_res = ocr_imgproc_color2grey(img, grey);
		if (op_res != 0) {
			printf("Ошибка при переводе в серое.\n");
			continue;
		}
		ocr_imgproc_invert(grey);
		
		/* Применяем фильтр Гаусса, если стоит соответствующий флаг. */
		if (use_gauss)
			ocr_imgproc_filter_gauss(grey);

		/* Выделяем память для бинаризованного изображения. */
		bin = ocr_img_info_malloc(w, h, BIN);
		if (bin == NULL) {
			printf("Не удалось выделить память для изображения.\n");
			continue;
		}

		/* Выбераем бинаризацию. */
		switch (t_mode) {
		case otsu_global:
			op_res = ocr_imgproc_global_otsu(grey, bin);
			break;
		case otsu_adapt:		
			op_res = ocr_imgproc_vert_otsu(grey, bin, cells_on_side);
			break;
		default:
			op_res = ocr_imgproc_adapt_otsu(grey, bin, cells_on_side);
			break;
		}

		if (op_res != 0) {
			printf("Ошибки при бинаризации.\n");
			continue;
		}

		/* Применяем дилатацию при необходимости. */
		if (pic_segm > 0) {
			mask = ocr_img_info_malloc(w, h, BIN);
			op_res = ocr_segm_bloomberg(bin, mask);
			if (op_res == 0) 
				ocr_img_proc_apply_mask(bin, mask, EXCLUDE);
			else
				printf("Не удалось выполнить сегментацию блумберга.\n");
			free(mask);
		}
		/**** Сегментация ****/
		/* Получаем текстовые области. */
		text_areas = ocr_segm_get_text_areas(bin, 100, &text_areas_count);
		if (text_areas == NULL) {
			printf("Ошибка при выделении текстовых областей.\n");
			continue;
		}
		
		if (all_details)
			printf("Число текстовых областей: %d.\n", text_areas_count);		

		uchar *t_pix = NULL;
		int t_stride = 0;
		int y = 0;
		int u = 0;
		int m = 0;
	
		/* Получаем строки текстовых областей. */
		for (i = 0; i < text_areas_count; i++) {
			text_lines = ocr_segm_stat_lines_area(text_areas[i], &lines_count);
			if (text_lines == NULL) {
				printf("Ошибка при выделении строк из текста.");
			}
	

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
			if (src == NULL) {
				printf("Couldn't create surface for data.\n");
				continue;
			}
			cairo_set_source_surface(cr, src, 0, 0);
			cairo_paint(cr);
			cairo_show_page(cr);
			cairo_surface_destroy(src);
			free(t_pix);

			if (all_details)
				printf("\tЧисло строк: %d.\n", lines_count);
			/* Получаем слова из строк. */
			for (j = 0; j < lines_count; j++) {
				text_words = ocr_segm_stat_words_area(text_lines[j], &words_count);
				if (all_details)
					printf("\t\tЧисло слов в строке %d: %d.\n", j, words_count);
				
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
				if (src == NULL) {
					printf("Couldn't create surface for data.\n");
					continue;
				}
				cairo_set_source_surface(cr, src, 0, 0);
				cairo_paint(cr);
				cairo_show_page(cr);
				cairo_surface_destroy(src);
				free(t_pix);
			
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
						t_pix[y * t_stride + u] = text_lines[j]->pix[y * w + u];
					}
				}
				cairo_pdf_surface_set_size(dst, w, h);
				format = CAIRO_FORMAT_A8;
				//stride = cairo_format_stride_for_width(format, w);
				src = cairo_image_surface_create_for_data(t_pix, format, w, h, t_stride);
				if (src == NULL) {
					printf("Couldn't create surface for data.\n");
					continue;
				}
				cairo_set_source_surface(cr, src, 0, 0);
				cairo_paint(cr);
				cairo_show_page(cr);
				cairo_surface_destroy(src);
				free(t_pix);



				/**** Распознание ****/
				for (l = 0; l < words_count; l++) {
					text_chars = ocr_segm_stat_chars_area(text_words[l], &chars_count);
					ocr_segm_stat_chars_recovery(bin, text_chars, chars_count);
					t_pix = NULL;
					w = text_words[l]->width;
					h = text_words[l]->height;
					if (w <= 0) {
						printf("WW22:%d %d\n", w, l);
						continue;
					}
					if (h <= 0) {
						printf("WW22:%d\n", h);
						continue;
					}
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
					if (src == NULL) {
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
					r_char = 0;
					next_char = 0;	
					for (m = 0; m < chars_count; m++) {
						/* Распознаем символ. */
						//op_res = ocr_recog_char_hist(text_chars[m], &next_char);
						op_res = ocr_recog_char_region(text_chars[m], &next_char);
						if (op_res != 0)
							continue;
						/* Проверка на букву Ы. */
						if (r_char == L'ь' && next_char == L'I') {
							next_char = L'ы';
							printf("\b");
						}
						printf("%Lc", next_char);
						r_char = next_char;
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
						if (t_pix == NULL) {
							continue;
						}
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
										
					printf(" ");
				}
				printf("\n");
			}
			
			if (all_details)
				printf("\t==========================================\n");
		}
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
	return 0;
}
