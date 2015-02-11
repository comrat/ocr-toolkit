#include "ocr_meta.h"
#include "ocr_segm.h"
#include "ocr_char_templates.h"
#include "ocr_recog.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>

/************************ Прототипы *****************************/
int ocr_recog_char_hist(ocr_text_area *char_area, wchar_t *res_char);

char ocr_recog_char_corners(ocr_text_area *char_area);

int ocr_recog_char_region(ocr_text_area *char_area, wchar_t *res_char);

int ocr_recog_get_region_stat(ocr_text_area *char_area, double *regions);

proj_hist *ocr_recog_get_proj_hist(ocr_text_area *text);

void ocr_recog_normalize_hist(proj_hist *hist, int width, int height);

int ocr_recog_punct_signs(ocr_text_area *char_area, wchar_t *result);
/************************ Реализация ****************************/
/****************************************
* Подход с гистограммами.
*****************************************/
proj_hist *ocr_recog_get_proj_hist(ocr_text_area *text)
{
	double *lines = NULL, *colls = NULL;
	if (text->width <= 0 || text->height <= 0) {
		printf("Некорректные ширина или высота текстовой области.\n");
		return NULL;
	}
	/* Выделяем память для гистограмм. */
	proj_hist *hist = (proj_hist *)malloc(sizeof(proj_hist));
	if (hist == NULL) {
		printf("Не удалось выделить память для результирующей проекционной гистограммы.\n");
		return NULL;
	}
	/* Получаем гистограмму по строкам. */
	lines = ocr_segm_get_line_stat(text);
	if (lines == NULL) {
		printf("Не удалось получить проективную гистограмму по строкам.\n");
		return NULL;
	}
	/* Получаем гистограмму по столбцам. */
	colls = ocr_segm_get_coll_stat(text);
	if (colls == NULL) {
		printf("Не удалось получить проективную гистограмму по строкам.\n");
		return NULL;
	}
	hist->lines = lines;
	hist->colls = colls;
	hist->width = text->width;
	hist->height = text->height;
	return hist;
}

/* Интерполяция для шага по х между y1 и y2 = 1. */
double interpolate(double y1, double y2, double rel_x)
{
	double result = 0.0;
	if (rel_x < 0 && rel_x > 1) {
		printf("Неправильное указания относительного положения х в интервале.\n");
		return -1;
	}
	result =  y1 + (y2 - y1) * rel_x;
	result = (result >= 0) ? result : 0;
	return result;
}

/* Нормализуем гистограмму, приводим к фиксированной ширине и высоте. */
void ocr_recog_normalize_hist(proj_hist *hist, int width, int height)
{
	int shift = 0;
	int curr = 0;
	int mod = 0;
	int i = 0;
	double *lines = NULL;
	double *colls = NULL;
	double prev_val = 0.0;

	/* Если входные ширина и высота соотвествуют шаблонным, 
	возвращаем исходную. */
	if (width == TEMP_WIDTH && height == TEMP_HEIGHT) {
		return;
	}
	
	if (width <= 1 || height <= 1) {
		printf("Введены некорректные ширина или высота нормализуемой области.\n");
		return;
	}
	/* Выделяем память для выходных данных. */
	lines = (double *)malloc(sizeof(double) * TEMP_HEIGHT);
	colls = (double *)malloc(sizeof(double) * TEMP_WIDTH);
	if (lines == NULL || colls == NULL) {
		printf("Проблемы при выделении памяти для нормированной области.\n");
		return;
	}
	lines = (double *)memset(lines, 0, TEMP_HEIGHT * sizeof(double));
	colls = (double *)memset(colls, 0, TEMP_WIDTH * sizeof(double));

	/* Если символ шире стандартного. */
	if (width <= TEMP_WIDTH) {
		/* Определяем шаг разбиения. */
		shift = TEMP_WIDTH / width + 1;
		for (i = 0; i < TEMP_WIDTH - shift; i++) {
			curr = i / shift;	/* текущий индекс входной гистограммы */
			mod = i % shift;
			if (mod == 0) {
				colls[i] = hist->colls[curr];
				prev_val = colls[i];
			} else {
			//	colls[i] = interpolate(hist->colls[curr], hist->colls[curr + 1], (double)mod / shift);
				colls[i] = prev_val;//hist->colls[curr - 1];
			}
		}
		/* Заполняем оставшийся шаг выходного изображения. */
		for (i = TEMP_WIDTH - shift; i < TEMP_WIDTH; i++) {
			mod = i % shift;
			//colls[i] = interpolate(hist->colls[width - 1], hist->colls[width], (double)mod / shift);
			colls[i] = hist->colls[width - 1];
		}
	} else {
		/* Если символ не шире стандартного. */
		/* Определяем шаг разбиения. */
		shift = width / TEMP_WIDTH;
		for (i = 0; i < TEMP_WIDTH; i++) {
			if (i == TEMP_WIDTH - 1)
				colls[i] = hist->colls[width - 1];
			else
				colls[i] = hist->colls[i * shift];
		}
	}
	/* Если символ выше стандартного. */
	if (height <= TEMP_HEIGHT) {
		/* Определяем шаг разбиения. */
		shift = TEMP_HEIGHT / height + 1;
		for (i = 0; i < TEMP_HEIGHT - shift; i++) {
			curr = i / shift;	/* текущий индекс входной гистограммы */
			mod = i % shift;
			if (mod == 0) {
				lines[i] = hist->lines[curr];
				prev_val = lines[i];
			} else {
			//	lines[i] = interpolate(hist->lines[curr], hist->lines[curr + 1], (double)mod / shift);
				
				lines[i] = prev_val;//hist->lines[curr - 1];
			}
		}
		/* Заполняем оставшийся шаг выходного изображения. */
		for (i = TEMP_HEIGHT - shift; i < TEMP_HEIGHT; i++) {
			mod = i % shift;
			lines[i] = hist->lines[height - 1];//interpolate(hist->lines[height - 1], hist->lines[height], (double)mod / shift);
		}
	} else {
		/* Если символ не выше стандартного. */
		/* Определяем шаг разбиения. */
		shift = height / TEMP_HEIGHT;
		for (i = 0; i < TEMP_HEIGHT; i++) {
			if (i == TEMP_HEIGHT - 1)
				lines[i] = hist->lines[height - 1];
			else
				lines[i] = hist->lines[i * shift];
		}
	}
	/* Ссылаемся на полученные строки и столбцы. */
	hist->width = TEMP_WIDTH;
	hist->height = TEMP_HEIGHT;
	//free(hist->lines);
	//free(hist->colls);
	hist->lines = (double *)realloc(hist->lines, sizeof(double) * TEMP_HEIGHT);
	hist->colls = (double *)realloc(hist->colls, sizeof(double) * TEMP_WIDTH);
	hist->lines = (double *)memcpy(hist->lines, lines, sizeof(double) * TEMP_HEIGHT);
	hist->colls = (double *)memcpy(hist->colls, colls, sizeof(double) * TEMP_WIDTH);
}

/* Нахождение расстояния до шаблона по методу гистограмм. */
double ocr_recog_get_dist_hist(proj_hist *hist, temp_hist *template)
{
	int i = 0;
	double result = 0.0;

	for (i = 0; i < TEMP_WIDTH; i++)
		result += (hist->colls[i] - template->colls[i]) * (hist->colls[i] - template->colls[i]);

	for (i = 0; i < TEMP_HEIGHT; i++)
		result += (hist->lines[i] - template->lines[i]) *(hist->lines[i] - template->lines[i]) ;
	return result;
}

int ocr_recog_char_hist(ocr_text_area *char_area, wchar_t *res_char)
{
	int i = 0;
	int min_ind = 0;
	double min = 0;
	double tmp = 0;
	proj_hist *hist = NULL;
	/* Если какой-либо из размеров области значительно меньше (в 10 раза) щаблонной,
	то не рассматриваем. */
	if ((char_area->width < TEMP_WIDTH / 10) || (char_area->height < TEMP_HEIGHT / 10))
		return -1;

	/* Получаем гистограммы по обоим измерениям. */
	hist = ocr_recog_get_proj_hist(char_area);
	if (hist == NULL) {
		printf("не удалось получить гистограммы для распознания.\n");
		return -1;
	}
	/* Нормализуем полученные результаты. */
	ocr_recog_normalize_hist(hist, char_area->width, char_area->height);
	if (hist == NULL) {
		printf("Не удалось получить гистограммы символа.\n");
		return -1;
	}
	/* За мин. возьмем максимально возможное значение. */
	min = TEMP_HEIGHT * TEMP_WIDTH;
	/* Сравниваем с базой шаблонов и находим 
	ближайщий. */
	for (i = 0; i < HIST_TEMP_COUNT; i++) {
		tmp = ocr_recog_get_dist_hist(hist, &temp_for_hist[i]);
	//	printf("%lc", temp_for_hist[i].symbol);
		if (tmp < min) {	
			min_ind = i;
			min = tmp;
		}
	}
/*
	char str[4];		
	printf("\t\t{");
	for (i = 0; i < hist->width - 1; i++) {
		sprintf(str, "%.2f", hist->colls[i]);
		str[1] = '.';
		printf("%s, ", str);
		if ((i + 1) % 10 == 0)
			printf("\n\t\t ");
	}
	sprintf(str, "%.2f", hist->colls[hist->width - 1]);
	str[1] = '.';
	printf("%s},\n\t\t{", str);

	for (i = 0; i < hist->height - 1; i++) {	
		sprintf(str, "%.2f", hist->lines[i]);
		str[1] = '.';
		printf("%s, ", str);
		if ((i + 1) % 10 == 0)
			printf("\n\t\t ");
	}
	sprintf(str, "%.2f", hist->lines[hist->height - 1]);
	str[1] = '.';
	printf("%s},\n\n\n", str);		
*/




	
	*res_char = temp_for_hist[min_ind].symbol;
//	printf("%lc", res_char);
	return 0;
}

/************************************
* Статистический подход.
*************************************/
/* Нахождение расстояния до шаблона по методу гистограмм. */
double ocr_recog_get_dist_stat(double *stat, temp_stat *template)
{
	int i = 0;
	double result = 0.0;

	for (i = 0; i < TEMP_CELLS_COUNT; i++)
		result += sqrt((stat[i] - template->stat[i]) * (stat[i] - template->stat[i]));

	return result;
}

int ocr_recog_get_region_stat(ocr_text_area *char_area, double *regions)
{
	if (regions == NULL) {
		printf("Не удалось получить данные о региональной статистике.\n");
		return -1;
	}
	int i = 0, j = 0;		/* Индексы для текстовой области. */
	int cell_x = 0, cell_y = 0;	/* Индексы для клеток. */
	int width = char_area->width;	/* Ширина текстовой области.*/
	int height = char_area->height;	/* Высота текстовой области. */
	int cells_count = 0;		/* Общее число клеток. */
	int cell_w = 0;			/* Ширина клетки на текстовой области. */
	int cell_h = 0;			/* Высота клетки на текстовой области. */
	int rest_cell_w = 0;		/* Ширина клетки на текстовой области. */
	int rest_cell_h = 0;		/* Высота клетки на текстовой области. */
	int pix_count = 0;		/* Число пикслей в одной клетке текстовой области. */
	int rest_pix_count = 0;		/* Число пикслей в клетке в конце строки. */
	int ind = 0;			/* Индекс. */
	uchar *pix = char_area->pix;	/* Указатель на пиксели текстовой области. */	
	/* Число клеток. */
	cells_count = TEMP_X_CELLS * TEMP_Y_CELLS;
	/* Ширина и высота клетки. */
	cell_w = width / TEMP_X_CELLS;
	cell_h = height / TEMP_Y_CELLS;
	/* Ширина последней в строке клекти. */
	rest_cell_w = (width % TEMP_X_CELLS == 0) ? cell_w : cell_w + (TEMP_X_CELLS - width % cell_w);
	/* Высота последней в столбце клекти. */
	rest_cell_h = (height % TEMP_Y_CELLS == 0) ? cell_h : cell_h + (TEMP_Y_CELLS - height % cell_h);
	/* Сбрасываем все значений клеток. */
	memset(regions, 0.0, cells_count);
	for (i = 0; i < cells_count; i++) {
		regions[i] = 0.0;
	}
	/* Число пикселей в клетке. */
	pix_count = cell_w * cell_h;
	/* Считаем сумму пикселей для всех клеток. */
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			cell_y = (i / cell_h < TEMP_Y_CELLS) ? i / cell_h : TEMP_Y_CELLS - 1;
			cell_x = (j / cell_w < TEMP_X_CELLS) ? j / cell_w : TEMP_X_CELLS - 1;
			regions[cell_y * TEMP_X_CELLS + cell_x] += pix[i * width + j];
		}
	}

	/* Число пикселей в конце строки. */
	rest_pix_count = rest_cell_w * cell_h;
	/* Нормируем все клетки. */
	for (i = 0; i < TEMP_Y_CELLS - 1; i++) {
		for (j = 0; j < TEMP_X_CELLS - 1; j++) {
			regions[i * TEMP_X_CELLS + j] /= pix_count * CR_BLACK;
		}
		/* Индекс последней в строке области. */
		ind = i * TEMP_X_CELLS + TEMP_X_CELLS - 1;
		regions[ind] /= rest_pix_count * CR_BLACK;
	}
	rest_pix_count = rest_cell_h * cell_w;
	for (i = 0; i < TEMP_X_CELLS - 1; i++) {
		ind = cells_count - TEMP_X_CELLS + i;
		regions[ind] /= rest_pix_count * CR_BLACK;
	}
	/* Число пикселей в последней области. */
	rest_pix_count = rest_cell_w * rest_cell_h;
	regions[cells_count - 1] /= rest_pix_count * CR_BLACK;
	return 0;
}

int ocr_recog_punct_signs(ocr_text_area *char_area, wchar_t *res_char)
{
	if (char_area->width <= 0 || char_area->height <= 0)
		return -1;
	int i = 0;
	int width = char_area->width;		/* Ширина симола. */
	int height = char_area->height;		/* Высота симола. */
	int cells_count = width * height;	/* Число пикселей. */
	double rel = 0.0;			/* Отнощение сторон. */
	double mu = 0.0;			/* Доля черных пикселей. */
	uchar *pix = char_area->pix;		/* Указатель на пиксели. */
	/* Считаем долю черных пикселей. */
	for (i = 0; i < cells_count; i++) {
		mu += pix[i];
	}
	mu /= cells_count * CR_BLACK;
	*res_char = 0;
	/* Вычисляем отнощение сторон. */
	rel = (double)height / width;
	if (mu > 0.85 && rel >= 0.85 && rel <= 1.15) 
		*res_char = L'.';
	else if (mu > 0.8 && rel >= 7 && rel < 0.85)
		*res_char = L',';
	else if (mu > 0.8 && rel <= 0.3)
		*res_char = L'-';
	/* Если символ не был распознан - выведем ошибку. */
	if (*res_char == 0)
		return -1;
	else
		return 0;
}

int ocr_recog_char_region(ocr_text_area *char_area, wchar_t *res_char)
{
	int i = 0;
	int min_ind = 0;
	int op_res = 0;
	int cells_count = TEMP_X_CELLS * TEMP_Y_CELLS;
	double *regions = (double *)malloc(sizeof(double) * cells_count);
	double min = TEMP_CELLS_COUNT;
	double tmp = 0;

	if (regions == NULL) {
		printf("Не удалось выделить память для региональной статистики.\n");
		return -1;
	}

	/* Если ширина или высота символа меньше числа разбиений. */
	if (char_area->width < TEMP_X_CELLS || char_area->height < TEMP_Y_CELLS) {
		
		op_res = ocr_recog_punct_signs(char_area, res_char);
		if (op_res == -1)
			return -1;
		else
			return 0;
	}
	
	/* Получаем статистику по регионам. */
	op_res = ocr_recog_get_region_stat(char_area, regions);
	if (op_res != 0)
		return -1;
	/* Сравниваем с шаблонами. */
	for (i = 0; i < STAT_TEMP_COUNT; i++) {
		tmp = ocr_recog_get_dist_stat(regions, &temp_for_stat[i]);
		if (tmp < min) {
			min_ind = i;
			min = tmp;
		}
	}
/*
	char str[4];
	printf("\t}, {\n");
	printf("\t\t {");
	for (i = 0; i < cells_count - 1; i++) {
		sprintf(str, "%.2f", regions[i]);
		str[1] = '.';
		printf("%s, ", str);
		if ((i + 1) % 10 == 0)
			printf("\n\t\t ");
	}
			
	sprintf(str, "%.2f", regions[cells_count - 1]);
	str[1] = '.';
	printf("%s},\n", str);
	printf("\t\t L''\n");
	*/
	*res_char = temp_for_stat[min_ind].symbol;
	free(regions);
	return 0;
}
