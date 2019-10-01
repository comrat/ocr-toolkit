#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
#include <getopt.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>

#include "ocr_preproc.h"

static int compare(const void *p1, const void *p2)
{ return strcmp(*(char *const *)p1, *(char *const *)p2); }

int main(int argc, char **argv)
{
	GdkPixbuf *pbuf;
	GError *gerror;
	cairo_t *cr;
	cairo_surface_t *dst, *src;
	cairo_format_t format;

	int i = 0;
	int j = 0;
	int k = 0;
	int opt;
	int open_dir = -1;
	int cells_on_side = 10;		// horisontal cells count
	int f_count = 0;			// files in dir count
	int w = 1, h = 1;			// width and height
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

	while ((opt = getopt(argc, argv, "df:gost:")) != -1) {
		switch (opt) {
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

	if (argc <= 1) {
		printf("Please enter directory name.\n ");
		exit(1);
	}

	if (gerror != NULL) {
		printf("Could't open catalog.\n");
		exit(1);
	}

	open_dir = (int)g_chdir(cat_name);

	if (open_dir == -1) {
		printf("Couldn't open catalog.\n");
		exit(1);
	}

	/* Read file list from directory */
	while ((fname = (char *)g_dir_read_name(cat)) != NULL) {
		/* Reallocate memory for more than 1000 files */
		if (f_count > 1000)
			file_list = (char **)realloc(file_list, f_count);
		file_list[f_count] = malloc(sizeof(char) * strlen(fname));
		memcpy(file_list[f_count], fname, strlen(fname));
		++f_count;
	}

	/* Sort files */
	qsort(file_list, f_count, sizeof(char *), compare);

	/* Create surface for output pdf file: 'result.pdf' */
	dst = cairo_pdf_surface_create("result.pdf", w, h);
	cr = cairo_create(dst);
	if (cr == NULL) {
		printf("Couldn't create \"result.pdf\"");
		exit(1);
	}

	for (k = 0; k < f_count; ++k) {
		printf("Processing file %s:\n", file_list[k]);
		g_type_init();
		gerror = NULL;

		pbuf = gdk_pixbuf_new_from_file(file_list[k], &gerror);
		if (pbuf == NULL) {
			printf("Couldn't open file %s\n", file_list[k]);
			continue;
		}

		if (gerror != NULL) {
			printf("Failed to open file %s:%s\n", file_list[k], gerror->message);
			continue;
		}

		w = gdk_pixbuf_get_width(pbuf);
		h = gdk_pixbuf_get_height(pbuf);

		cairo_pdf_surface_set_size(dst, w, h);
		pix = gdk_pixbuf_get_pixels(pbuf);
		stride = gdk_pixbuf_get_rowstride(pbuf);

		/* Check alpha channel and use 4 bytes per pixel then */
		rowpix = gdk_pixbuf_get_has_alpha(pbuf) ? 4 : 3

		/* Fill image info structure */
		img->width = w;
		img->height = h;
		img->pix = pix;
		img->stride = stride;
		img->bytes_for_pix = rowpix;

		/**** Binarization ****/
		grey = ocr_imgproc_color2grey(img);
		ocr_imgproc_invert(grey);

		if (grey == NULL) {
			printf("Converting to grey error.");
			continue;
		}

		format = CAIRO_FORMAT_A8;
		stride = cairo_format_stride_for_width(format, w);

		/* Apply Gauss filter for corresponded flag */
		if (use_gauss)
			ocr_imgproc_filter_gauss(grey);

		/* Apply Otsu binarization */
		bin = ocr_imgproc_threshold_otsu(grey, cells_on_side);

		/* Apply delate transform if necessary */
		if (use_dilate)
			ocr_imgproc_dilate(bin);

		if (bin == NULL) {
			printf("Binarization error.");
			continue;
		}

		/* Save the page */
		src = cairo_image_surface_create_for_data(bin->pix, format, w, h, stride);
		if (src == NULL) {
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

	return 0;
}
