#include "ocr_meta.h"
#include "ocr_segm.h"
#include "ocr_char_templates.h"
#include "ocr_recog.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <locale.h>

/************************ Прототипы *****************************/

char ocr_recog_char_hist(ocr_text_area *char_area);

char ocr_recog_char_corners(ocr_text_area *char_area);

char ocr_recog_char_stat(ocr_text_area *char_area);


proj_hist *ocr_recog_get_proj_hist(ocr_text_area *text);

void ocr_recog_normalize_hist(proj_hist *hist, int width, int height);

/****************************************************************/
/************************ Реализация ****************************/

/*** Подход с гистограммами. ***/
/**/
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

/* интерполяция для шага по х между y1 и y2 = 1. */
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

	if (width <= TEMP_WIDTH) {
		/* Определяем шаг разбиения. */
		shift = TEMP_WIDTH / width + 1;
		for (i = 0; i < TEMP_WIDTH - shift; i++) {
			curr = i / shift;	/* текущий индекс входной гистограммы */
			mod = i % shift;
			if (mod == 0) {
				colls[i] = hist->colls[curr];
			} else {
				colls[i] = interpolate(hist->colls[curr], hist->colls[curr + 1], (double)mod / shift);
			}
		}
		/* Заполняем оставшийся шаг выходного изображения. */
		for (i = TEMP_WIDTH - shift; i < TEMP_WIDTH; i++) {
			mod = i % shift;
			colls[i] = interpolate(hist->colls[width - 1], hist->colls[width], (double)mod / shift);
		}
	} else {
		/* Определяем шаг разбиения. */
		shift = width / TEMP_WIDTH;
		for (i = 0; i < TEMP_WIDTH; i++) {
			if (i == TEMP_WIDTH - 1)
				colls[i] = hist->colls[width - 1];
			else
				colls[i] = hist->colls[i * shift];
		}
	}

	if (height <= TEMP_HEIGHT) {
		/* Определяем шаг разбиения. */
		shift = TEMP_HEIGHT / height + 1;
		for (i = 0; i < TEMP_HEIGHT - shift; i++) {
			curr = i / shift;	/* текущий индекс входной гистограммы */
			mod = i % shift;
			if (mod == 0) {
				lines[i] = hist->lines[curr];
			} else {
				lines[i] = interpolate(hist->lines[curr], hist->lines[curr + 1], (double)mod / shift);
			}
		}
		/* Заполняем оставшийся шаг выходного изображения. */
		for (i = TEMP_HEIGHT - shift; i < TEMP_HEIGHT; i++) {
			mod = i % shift;
			lines[i] = interpolate(hist->lines[height - 1], hist->lines[height], (double)mod / shift);
		}
	} else {
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

/* Нёахождение расстояния до шаблона по методу гистограмм. */
double ocr_recog_get_dist_hist(proj_hist *hist, temp_hist *template)
{
	int i = 0;
	double result = 0.0;
//	setlocale(LC_ALL, "ru_RU.UTF8");

	for (i = 0; i < TEMP_WIDTH; i++)
		result += (hist->colls[i] - template->colls[i]) * (hist->colls[i] - template->colls[i]);

	for (i = 0; i < TEMP_HEIGHT; i++)
		result += (hist->lines[i] - template->lines[i]) *(hist->lines[i] - template->lines[i]) ;
	return result;
}

char ocr_recog_char_hist(ocr_text_area *char_area)
{
	int i = 0;
	int min_ind = 0;
	double min = 0;
	double tmp = 0;
	wchar_t res_char = 0;
	proj_hist *hist = NULL;
	/* Если какой-либо из размеров области значительно меньше (в 4 раза) щаблонной,
	то не рассматриваем. */
	if ((char_area->width < TEMP_WIDTH >> 2) || (char_area->height < TEMP_HEIGHT >> 2))
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
		//printf("%lc", temp_for_hist[i].symbol);
		if (tmp < min) {
			
			min_ind = i;
			min = tmp;
		}
	}
	res_char = temp_for_hist[min_ind].symbol;
//	printf("%lc", res_char);
	//printf("%Lc", L'я');
	//printf("lc");
	return res_char;
}

/***********************************/
/*** Подход с углами. ***/
/************************/
/*** Статистический подход. ***/
/******************************/
