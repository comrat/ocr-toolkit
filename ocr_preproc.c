#include "ocr_meta.h"
#include "ocr_preproc.h"

#include <stdlib.h>
#include <math.h>


/* This function convert colored pixel to the grayscale. */
int rgb2grey(int r, int g, int b)
{ return (int)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16; }

ocr_img_info *ocr_preproc_color2grey(ocr_img_info *img)
{
	/* Skip not colored image. */
	if (img->bytes_for_pix < 3)
		return NULL;

	int i = 0, j = 0;
	int stride = img->stride;
	int rowpix = img->bytes_for_pix;
	int width = img->width;
	int height = img->height;
	int curr = 0;
	//TODO: remove magic number
	int out_stride = (width % 4 == 0) ? width : width + (4 - width % 4);	// 4 is for mashine word
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	uchar *pix = img->pix;
	uchar *out_img = (uchar *)malloc(sizeof(char) * height * out_stride);
	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			curr = i * stride + j * rowpix;
			out_img[i * out_stride + j] = (uchar)rgb2grey(pix[curr], pix[curr + 1], pix[curr + 2]);
		}
	}

	result->width = width;
	result->height = height;
	result->stride = out_stride;
	result->bytes_for_pix = 1;
	result->pix = out_img;
	return result;
}


void ocr_preproc_dilate(ocr_img_info *img)
{
	/* Skip not binarized images */
	if (img->bytes_for_pix != 1)
		return;

	int i = 0;
	int j = 0;
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int curr_ind = 0;
	uchar *pix = img->pix;
	uchar *out_img = (uchar *)malloc(sizeof(char) * height * stride);

	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			curr_ind = i * stride + j;
			if (pix[curr_ind] == CR_BLACK) {
				if (i > 0)
					out_img[curr_ind - stride] = CR_BLACK;
				if (j > 0)
					out_img[curr_ind - 1] = CR_BLACK;

				out_img[curr_ind] = CR_BLACK;

				if (j < width - 1)
					out_img[curr_ind + 1] = CR_BLACK;
				if (i < height - 1)
					out_img[curr_ind + stride] = CR_BLACK;
			} else {
				out_img[curr_ind] = CR_WHITE;
			}
		}
	}
	free(img->pix);
	img->pix = out_img;
}


void ocr_preproc_errosion(ocr_img_info *img) {
	/* Skip not binarized images */
	if (img->bytes_for_pix != 1)
		return;

	int i = 0;
	int j = 0;
	int ind = 0;
	int width = img->width;
	int height = img->height;
	int stride = img->stride;
	uchar* pix = img->pix;
	uchar* out_img = (uchar *)malloc(sizeof(char) * width * height);

	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			if (i > 0 && i < height && j > 0 && j < width) {
				ind = i * width + j;

				if ((pix[ind - stride] == CR_BLACK) &&
					(pix[ind - 1] == CR_BLACK) && (pix[ind] == CR_BLACK) &&
					(pix[ind + 1] == CR_BLACK) && (pix[ind + stride] == CR_BLACK))
					out_img[ind] = CR_BLACK;
				else
					out_img[ind] = CR_WHITE;
			}
		}
	}
	img->pix = out_img;

	free(pix);
}


ocr_img_info *ocr_preproc_threshold_otsu(ocr_img_info *img, int divisions)
{
	/* Skip not grayscale images. */
	if (img->bytes_for_pix != 1)
		return NULL;

	int i = 0;
	int j = 0;
	int x_block = 0;
	int y_block = 0;
	int x = 0, y = 0;
	int curr_ind = 0, k = 0;
	int curr_x_size = 0, curr_y_size = 0;
	int otsu_trshld = 0;
	int block_size = 0;
	int pix_count = 1;
	int width = img->width;
	int height = img->height;
	int stride = img->stride;
	int max_dev = 14;
	int y_divisions = 0;
	int y_rest = 0, x_rest = 0;

	uchar *out_img;
	uchar *pix = img->pix;
	double sum = 0;
	double otsu_mL = 0, otsu_mR = 0;
	double otsu_sum = 0, otsu_sum_left = 0;
	double otsu_max = 0, otsu_between = 0;
	double mean = 0.0, deviate = 0.0;
	double left_weight = 0, right_weight = 0;
	double *hist = (double *)malloc(sizeof(double) * 256);
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));

	block_size = width / divisions;

	y_rest = height % block_size;	// вычисляем не влезшие пиксели по каждой оси
	x_rest = width % block_size;

	y_divisions = height / block_size;	// определяем число по оси у
	curr_x_size = curr_y_size = block_size;

	out_img = (uchar *)malloc(sizeof(char) * height * stride);

	for (;;) {
		mean = 0; deviate = 0;
		pix_count = curr_x_size * curr_y_size;

		for (k = 0; k < 256; ++k) {
			hist[k] = 0;
		}

		for (i = 0; i < curr_y_size; ++i) {
			y = y_block * block_size + i;		// get Y coordinate in current block
			for (j = 0; j < curr_x_size; ++j) {
				x = x_block * block_size + j;	// get X coordinate in current block
				curr_ind = y * stride + x;		// get current pixel index
				hist[pix[curr_ind]] += 1;
				mean += pix[curr_ind];
				deviate += pix[curr_ind] * pix[curr_ind];
			}
		}
		/* Calculate mean value and sigma. */
		mean /= pix_count;
		deviate /= pix_count;
		deviate = sqrt(deviate - mean * mean);

		sum = 0;
		otsu_sum = 0;

		/* Build frequency histogram. */
		for (k = 0; k < 256; ++k)
			sum += hist[k];
		for (k = 0; k < 256; ++k) {
			hist[k] /= (double)sum;
			otsu_sum += hist[k] * (k + 1);
		}

		otsu_sum_left = 0;
		left_weight = 0;
		right_weight = 0;
		otsu_mL = 0;
		otsu_mR = 0;
		otsu_max = 0;
		otsu_trshld = 0;

		for (k = 0; k < 256; ++k) {
			left_weight += hist[k];	// calculate left side weight

			if (left_weight == 0)
				continue;

			// keep value valid
			if (left_weight > 1)
				left_weight = 1;

			right_weight = 1 - left_weight;	// right side weight
			if (right_weight == 0)
				break;
			otsu_sum_left += (k + 1) * hist[k];
			otsu_mL = (double)otsu_sum_left / left_weight;
			otsu_mR = (double)(otsu_sum - otsu_sum_left) / right_weight;

			otsu_between = left_weight * right_weight * (otsu_mL - otsu_mR) * (otsu_mL - otsu_mR);

			if (otsu_between >= otsu_max) {
				otsu_max = otsu_between;
				otsu_trshld = k;
			}
		}

		/* Apply threshold on current block. */
		for (i = 0; i < curr_y_size; ++i) {
			y = y_block * block_size + i;
			for (j = 0; j < curr_x_size; ++j) {
				x = x_block * block_size + j;
				curr_ind = y * stride + x;
				out_img[curr_ind] = THRESHOLD(pix[curr_ind], otsu_trshld);
			}
		}

		/* Treat block with small signa as background. */
		if (deviate < max_dev) {
			if (mean < 128)
				mean -= max_dev;
			else
				mean += max_dev;
			mean = 128;
			for (i = 0; i < curr_y_size; ++i) {
				y = y_block * block_size + i;
				for (j = 0; j < curr_x_size; ++j) {
					x = x_block * block_size + j;
					curr_ind = y * stride + x;
					out_img[curr_ind] = THRESHOLD(pix[curr_ind], mean);
				}
			}
		}

		/* Go to next block. */
		curr_x_size = curr_y_size = block_size;
		++x_block;		// увеличиваем x блок
		/* Если прошли до последнего блока в строке: */
		if (x_block == divisions) {
			if (x_rest != 0) {
				curr_x_size = x_rest;
			}else{
				x_block = 0;	// переходим к след. строке
				++y_block;
			}
		}

		if (x_block > divisions) {
			x_block = 0;
			++y_block;	// переходим к след. строке
		}

		if (y_block == y_divisions) {
			if (y_rest == 0)
				break;	// завершаем программу.
			else{
				curr_y_size = y_rest;
			}
		}

		if (y_block > y_divisions)
			break;
	}

	result->width = width;
	result->height = height;
	result->stride = stride;	// шаг для бинаризованного изображения не нужен
	result->bytes_for_pix = 1;	// one byte for binarized image
	result->pix = out_img;		// ссылаемся на полученное изображение

	return result;
}

ocr_img_info *ocr_preproc_threshold_sauvolas(ocr_img_info *img, int divisions)
{
	/* Если входное изображение не серое (на пиксель приходится
	не 1 байт) возвращаем NULL. */
	if (img->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0, x_block = 0, y_block = 0;
	int x = 0, y = 0, curr_ind = 0;
	int curr_x_size = 0, curr_y_size = 0;
	int block_size = 0;
	int pix_count = 1;
	int width = img->width;
	int height = img->height;
	int stride = img->stride;
	int rowpix = img->bytes_for_pix;
	int trshld = 0;
	int R = 128;
	int y_divisions = 0, y_rest = 0, x_rest = 0;
	char curr_color = 0;
	uchar *out_img = (unsigned char *)malloc(sizeof(char) * height * stride);
	uchar *pix = img->pix;
	uchar **curr_window;
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	/*==================Treshold computing:==========================
		trshld = mean * (1 + k * (deviate / R - 1))
		k - подобрано эмпирически.
	=================================================================*/
	double k = 0.15, mean = 0.0, deviate = 0.0;	// varaibles for treshold calculating

	/* Get width & height for every tresholding area. */
	block_size = width / divisions;
	y_rest = height % block_size;
	x_rest = width % block_size;
	y_divisions = height / block_size;
	/* Allocate memeory for considered window of image. */
	curr_window = (unsigned char **)malloc(sizeof(unsigned char *) * block_size);
	for (i = 0; i < block_size; ++i) {
		curr_window[i] = (unsigned char *)malloc(sizeof(char) * block_size);
		for (j = 0; j < block_size; ++j) {
			curr_window[i][j] = 0;
		}
	}
	curr_x_size = curr_y_size = block_size;

	/* Initialize output image. */
	for (;;) {
		pix_count = curr_x_size * curr_y_size;
		mean = 0.0; deviate = 0.0;
		/* Calculate mean value. */
		for (i = 0; i < curr_y_size; ++i) {
			y = y_block * block_size + i;

			for (j = 0; j < curr_x_size; ++j) {
				// look up pixels in one area
				x = x_block * block_size + j;
				// get greyscale value:
				curr_ind = y * stride + x * rowpix;
				//printf("%d--\n", curr_ind);
				curr_window[i][j] = (unsigned char)rgb2grey(pix[curr_ind], pix[curr_ind + 1], pix[curr_ind + 2]);
				mean += curr_window[i][j];
				deviate += curr_window[i][j] * curr_window[i][j];
			}
		}
		mean /= pix_count;
		deviate /= pix_count;
		deviate += sqrt(deviate - mean * mean);

		trshld = (int)(mean * (1 + k * ((double)deviate / R - 1)));
		/* Use threshold for current area. */
		for (i = 0; i < curr_y_size; ++i) {
			y = y_block * block_size + i;

			for (j = 0; j < curr_x_size; ++j) {
				x = x_block * block_size + j;
				curr_ind = y * stride + x * rowpix;
				curr_color = THRESHOLD(curr_window[i][j], trshld);
				out_img[curr_ind] = curr_color;
				out_img[curr_ind + 1] = curr_color;
				out_img[curr_ind + 2] = curr_color;
			}
		}
		curr_x_size = curr_y_size = block_size;
		/* Prepare to next iterarion. */
		++x_block;		// go to next x-block
		// if we reach last area in row...
		if (x_block == divisions) {
			if (x_rest != 0) {
				curr_x_size = x_rest;
			}else{
				x_block = 0;
				++y_block;
			}
		}

		if (x_block > divisions) {
			x_block = 0;
			++y_block;	// go to next row
		}

		if (y_block == y_divisions) {
			if (y_rest == 0)
				break;
			else{
				curr_y_size = y_rest;
			}
		}

		if (y_block > y_divisions)
			break;
	}
	return result;
}

void ocr_preproc_filter_sobel(ocr_img_info *img)
{
	/* Skip not grayscale image. */
	if (img->bytes_for_pix == 1)
		return;

	int *filter_x, *filter_y;
	int i = 0, j = 0, curr = 0;
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int rowpix = img->bytes_for_pix;
	int sum_x = 0;
	int sum_y = 0;
	uchar *out_img = (uchar *)malloc(sizeof(char) * height * stride);
	uchar *pix = img->pix;

	/* Sobel x-filter
		|-1|-2|-1|
		|_0|_0|_0|
		|_1|_2|_1|
	*/
	filter_x = (int *)malloc(sizeof(int) * 9);

	/* Sobel x-filter
		|_1|_0|-1|
		|_2|_0|-2|
		|_1|_0|-1|
	*/
	filter_y = (int *)malloc(sizeof(int) * 9);

	filter_x[0] = -1;
	filter_x[1] = -2;
	filter_x[2] = -1;

	filter_x[3] = 0;
	filter_x[4] = 0;
	filter_x[5] = 0;

	filter_x[6] = 1;
	filter_x[7] = 2;
	filter_x[8] = 1;

	filter_y[0] = 1;
	filter_y[1] = 0;
	filter_y[2] = -1;

	filter_y[3] = 2;
	filter_y[4] = 0;
	filter_y[5] = -2;

	filter_y[6] = 1;
	filter_y[7] = 0;
	filter_y[8] = -1;

	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			curr = i * stride + j * rowpix;
			if (i > 0 && j > 0 && i < height - 1 && j < width - 1) {
				sum_x = 0;
				sum_y = 0;

				sum_x += pix[curr - stride - rowpix] * filter_x[0];
				sum_x += pix[curr - stride] * filter_x[1];
				sum_x += pix[curr - stride + rowpix] * filter_x[2];

				sum_x += pix[curr - rowpix] * filter_x[3];
				sum_x += pix[curr] * filter_x[4];
				sum_x += pix[curr + rowpix] * filter_x[5];

				sum_x += pix[curr + stride - rowpix] * filter_x[6];
				sum_x += pix[curr + stride] * filter_x[7];
				sum_x += pix[curr + stride + rowpix] * filter_x[8];

				sum_y += pix[curr - stride - rowpix] * filter_y[0];
				sum_y += pix[curr - stride] * filter_y[1];
				sum_y += pix[curr - stride + rowpix] * filter_y[2];

				sum_y += pix[curr - rowpix] * filter_y[3];
				sum_y += pix[curr] * filter_y[4];
				sum_y += pix[curr + rowpix] * filter_y[5];

				sum_y += pix[curr + stride - rowpix] * filter_y[6];
				sum_y += pix[curr + stride] * filter_y[7];
				sum_y += pix[curr + stride + rowpix] * filter_y[8];

				if ((sum_x * sum_x + sum_y * sum_y) <= 127 * 127) {
					out_img[curr - rowpix] = CR_WHITE;
					out_img[curr] = CR_WHITE;
					out_img[curr + rowpix] = CR_WHITE;
				}else{
					out_img[curr - rowpix] = CR_BLACK;
					out_img[curr] = CR_BLACK;
					out_img[curr + rowpix] = CR_BLACK;
				}
			}
		}
	}
	img->pix = out_img;
	free(pix);
}

void ocr_preproc_filter_gauss(ocr_img_info *img)
{
	/* Skip not grayscale image. */
	if (img->bytes_for_pix == 1)
		return;

	int i, j, curr;
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int rowpix = img->bytes_for_pix;
	int sum = 0;
	int cell_count = 9;
	char tmp;
	/* Filter kernel
		|1|2|1|
		|2|4|2|
		|1|2|1|
	*/
	int *filter = (int *)malloc(sizeof(int) * cell_count);
	uchar *out_img = (uchar *)malloc(sizeof(char) * stride * height);
	uchar *pix = img->pix;

	filter[0] = 1;
	filter[1] = 2;
	filter[2] = 1;

	filter[3] = 2;
	filter[4] = 4;
	filter[5] = 2;

	filter[6] = 1;
	filter[7] = 2;
	filter[8] = 1;

	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			curr = i * stride + j * rowpix;
			sum = 0;
			cell_count = 9;

			if (i > 0 && j > 0)
				sum += filter[0] * pix[curr - stride - rowpix];
			else
				cell_count--;

			if (i > 0)
				sum += filter[1] * pix[curr - stride];
			else
				cell_count--;

			if (i > 0 && j < width - 1)
				sum += filter[2] * pix[curr - stride + rowpix];
			else
				cell_count--;

			if (j > 0)
				sum += filter[3] * pix[curr - rowpix];
			else
				cell_count--;
			sum += filter[4] * pix[curr];
			if (j < width - 1)
				sum += filter[5] * pix[curr + rowpix];
			else
				cell_count--;

			if (i < height - 1 && j > 0)
				sum += filter[6] * pix[curr + stride - rowpix];
			else
				cell_count--;

			if (i < height - 1)
				sum += filter[7] * pix[curr + stride];
			else
				cell_count--;

			if (i < height - 1 && j < width - 1)
				sum += filter[8] * pix[curr + stride + rowpix];
			else
				cell_count--;
			tmp = (uchar) ((double)sum / 16);

			out_img[curr] = (uchar)tmp;
			out_img[curr + 1] = (uchar)tmp;
			out_img[curr + 2] = (uchar)tmp;
		}
	}
	img->pix = out_img;
	free(pix);
}


/* Функция сравнения чисел для быстрой сортировки. */
static int compare(const void  *p1, const void *p2)
{ return (*(int *)p1 - *(int *)p2); }


void ocr_preproc_filter_median(ocr_img_info *img, int size)
{
	if (img->bytes_for_pix != 1)
		return;

	int i = 0, j = 0, curr = 0;
	int k = 0, l = 0;
	int width = img->width - size;
	int height = img->height - size;
	int stride = img->stride;
	int wiblock_size = (size * 2 + 1) * (size * 2 + 1);	// ширина окна = текущий пиксель + отступ в size с каждой стороны.

	int side = 2 * size + 1;
	uchar *pix = img->pix;
	uchar *out_img = (uchar *)malloc(sizeof(uchar) * stride * height);
	uchar *window = (uchar *)malloc(sizeof(uchar) * wiblock_size);
	for (i = size; i < height; ++i) {
		for (j = size; j < width; ++j) {
			curr = i * stride + j;
			for (k = -size; k <= size; ++k) {
				for (l = -size; l <= size; ++l) {
					window[k * side + l] = pix[curr + k * stride + l];
				}
			}
			/* Сортируем элементы в текущем окне. */
			qsort(window, wiblock_size, sizeof(uchar), compare);
			/* Присваиваем средний элемент текущему. */
			out_img[curr] = window[wiblock_size / 2];
		}
	}
	img->pix = out_img;
	/* Освобождаем старый указатель. */
	free(pix);
}

void ocr_preproc_invert(ocr_img_info *img)
{
	if (img->bytes_for_pix != 1)
		return;

	int i = 0, j = 0;
	int width = img->width;
	int height = img->height;
	int stride = img->stride;

	for (i = 0; i < height; ++i) {
		for (j = 0; j < width; ++j) {
			img->pix[i * stride + j] = 255 - img->pix[i * stride + j];
		}
	}
}
