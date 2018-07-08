#include "ocr_meta.h"
#include <stdlib.h>


ocr_img_info *ocr_img_info_malloc(int width, int height, char type)
{
	int size = 0;
	int pix_size = 0;
	int ucsize = sizeof(uchar);
	int stride = 0;
	ocr_img_info *result = NULL;

	if (width <= 0 || height <= 0) {
		return NULL;
	}

	/* Get pixel size, use 1 Byte as default (for graysclae images).*/
	switch (type) {
	case BIN:
		pix_size = GREY;	/* Binirized is 1 byte size like a grayscale one. */
	case RGB:
		pix_size = RGB;
		break;
	case ARGB:
		pix_size = ARGB;
	default :
		pix_size = GREY;
	}
	size = pix_size;
	size *= ucsize;
	stride = width * ucsize;
	if (stride % WORD_SIZE != 0)
		stride += WORD_SIZE - stride % WORD_SIZE;
	size *= stride * height;

	result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	if (result == NULL) {
		return NULL;
	}
	result->pix = (uchar *)malloc(size);
	if (result->pix == NULL) {
		free(result);

		return NULL;
	}
	result->width = width;
	result->height = height;
	result->stride = stride;
	result->bytes_for_pix = type;

	return result;
}

void ocr_img_info_free(ocr_img_info *img)
{
	free(img->pix);
	free(img);
}

void ocr_img_info_array_free(ocr_img_info **img, int elem_count)
{
	int i = 0;
	for (i = 0; i < elem_count; i++) {
		free(img[i]->pix);
		free(img[i]);
	}
	free(img);
}

ocr_text_area *ocr_text_area_malloc(int width, int height, int x, int y)
{
	int size = 0;
	int ucsize = sizeof(uchar);
	int stride = 0;
	ocr_text_area *result = NULL;

	if (width <= 0 || height <= 0) {
		return NULL;
	}

	size = ucsize;
	stride = width * ucsize;
	if (stride % WORD_SIZE != 0)
		stride += WORD_SIZE - stride % WORD_SIZE;
	size *= stride * height;

	result = (ocr_text_area *)malloc(sizeof(ocr_text_area));
	if (result == NULL) {
		return NULL;
	}
	result->pix = (uchar *)malloc(size);
	if (result->pix == NULL) {
		free(result);
		return NULL;
	}
	result->width = width;
	result->height = height;
	result->x = x;
	result->y = y;

	return result;
}

void ocr_text_area_free(ocr_text_area *area)
{
	free(area->pix);
	free(area);
}

void ocr_text_area_array_free(ocr_text_area **area, int elem_count)
{
	int i = 0;
	for (i = 0; i < elem_count; i++) {
		free(area[i]->pix);
		free(area[i]);
	}
	free(area);
}
