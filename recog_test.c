/* Заголовочные файлы для распознования текста. */
#include "ocr_imgproc.h"
#include "ocr_segm.h"
#include "ocr_recog.h"
#include "ocr_meta.h"
/* */
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

int main(int argc, char **argv)
{
	GdkPixbuf *pbuf;
	GError *gerror;
	/* Cairo var-s. */
	cairo_t *cr;
	cairo_surface_t *dst, *src;
	cairo_format_t format;

	int i = 0, j = 0, k = 0;
	int opt;			// переменная для хранения входных флагов
	int w = 1, h = 1;		// ширина и высота
	int stride = 0, rowpix = 0;

	uchar *pix;

	ocr_text_area area;
	ocr_img_info *img = (ocr_img_info *)malloc(sizeof(ocr_img_info));

	gerror = NULL;

	/**** Обработка каждого файла. ****/
	/* Создаем поверхность для выходного PDF-файла. */
	/* Проверка успешности создания выходного файла. */
	if(cr == NULL){
		printf("Couldn't create \"result.pdf\"");
		exit(1);
	}

	g_type_init();
	gerror = NULL;

	/* Получаем указатель на пиксели. */
	pbuf = gdk_pixbuf_new_from_file("one_word.png", &gerror);

	w = gdk_pixbuf_get_width(pbuf);
	h = gdk_pixbuf_get_height(pbuf);
	
	pix = gdk_pixbuf_get_pixels(pbuf);
	stride = gdk_pixbuf_get_rowstride(pbuf);

	/* Определяем есть ли альфа канал в изображении. */
	if(gdk_pixbuf_get_has_alpha(pbuf))
		rowpix = 4;	// для альфа канала
	else
		rowpix = 3;	// для RGB

	img->width = w;
	img->height = h;
	img->stride = stride;
	img->bytes_for_pix = rowpix;
	img->pix = pix;

	int bin_stride = (w % 4 == 0) ? w : w + (4 - (w % 4));
	uchar *bin = (uchar *)malloc(sizeof(uchar) * bin_stride * h);
	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			bin[i * bin_stride + j] = pix[i * stride + j * rowpix];
		}
	}

	//free(img->pix);
	img->pix = bin;
	img->stride = bin_stride;
	img->bytes_for_pix = 1;
	/* Обращаем цвета. */
	ocr_imgproc_invert(img);

	area.x = 0;
	area.y = 0;
	area.width = w;
	area.height = h;
	area.pix = (uchar **)malloc(sizeof(uchar *) * h);
	for(i = 0; i < h; i++){
		area.pix[i] = (uchar *)malloc(sizeof(uchar) * w);
		for(j = 0; j < w; j++){
			area.pix[i][j] = bin[i * bin_stride + j];//pix[i * stride + j * rowpix];
		}
	}

	int char_count = 0;
	ocr_text_area *areas = ocr_recog_hist_chars_area(&area, &char_count);
	areas = ocr_recog_stat_chars_area(&area, &char_count);
//	ocr_text_area *areas = ocr_recog_hist_chars_area(&area, &char_count);


	//printf("CharL:%d\n", char_count);

/*	for(i = 0; i < char_count; i++){
		printf("=======================\nWW:%d HH:%d\n", areas[i].width, areas[i].height);
		ocr_recog_get_zone_char(&areas[i]);
	}
*/
	dst = cairo_pdf_surface_create("result.pdf", 100, 100);
	cr = cairo_create(dst);


	ocr_text_area tmp;
	free(bin);
	int t = 0;
	for(t = 0; t < char_count; t++){
		format = CAIRO_FORMAT_A8;
		tmp.width = areas[t].width;
		tmp.height = areas[t].height;
		tmp.pix = areas[t].pix;
		/* Запоминаем ширину и высоту изображения и
		изменяем размер */

		cairo_pdf_surface_set_size(dst, tmp.width, tmp.height);
		stride = cairo_format_stride_for_width(format, tmp.width);
		bin = malloc(sizeof(uchar) * stride * tmp.height);
		for(i = 0; i < tmp.height; i++){
			for(j = 0; j < tmp.width; j++){
				bin[i * stride + j] = tmp.pix[i][j];
			}
		}

		/* Сохранение страницы. */
		src = cairo_image_surface_create_for_data(bin, format, tmp.width, tmp.height, stride);
		cairo_set_source_surface(cr, src, 0, 0);
		cairo_paint(cr);
		cairo_show_page(cr);
		cairo_surface_destroy(src);
	}





	//free(areas);
	g_object_unref(pbuf);

	cairo_surface_destroy(dst);
	cairo_destroy(cr);
	free(img);
	return 0;
}
