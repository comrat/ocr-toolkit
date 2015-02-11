#include "ocr_meta.h"
#include "ocr_segm.h"
#include "ocr_imgproc.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/************************ Прототипы *****************************/
ocr_text_area **ocr_segm_stat_lines_area(ocr_text_area *text_area, int *line_count);

ocr_text_area **ocr_segm_stat_words_area(ocr_text_area *line_area, int *word_count);

ocr_text_area **ocr_segm_stat_chars_area(ocr_text_area *word_area, int *char_count);

void ocr_segm_stat_chars_recovery(ocr_img_info *img, ocr_text_area **chars, int char_count);


ocr_cells_net *ocr_segm_and_class_net(ocr_img_info *img, int cells_count);

ocr_text_area **ocr_segm_get_text_areas(ocr_img_info *img, int cells_count, int *text_areas_count);

ocr_img_info *ocr_segm_bloomberg(ocr_img_info *img);

ocr_img_info *ocr_segm_bloomberg_modify(ocr_img_info *img);

ocr_img_info *ocr_segm_get_hough_image(ocr_text_area *text_area, int delta_rho, int delta_phi, int strart_phi, int end_phi);
/** 3 
* Функция определяет число страниц на бинаризованном
* изображении (1 или 2). Функция, статистически анализируя,
* скользящим окном по длине изображения /fIimg/fP определяет
* наличие разделителя страниц, и в случае наличия таковой,
* возвращает 2, иначе 1. 
*/
int ocr_segm_get_page_count(ocr_img_info *img);

void ocr_segm_get_area(ocr_img_info *img, coord *begin, coord *end);

double *ocr_segm_get_line_stat(ocr_text_area *text_area);

double *ocr_segm_get_coll_stat(ocr_text_area *text_area);

/************************ Реализация ****************************/

/* Функция сравнения строк для быстрой сортировки. */
/*static int compare(const void  *p1, const void *p2)
{
	return (*(double *)p1 - *(double *)p2);
}*/


ocr_cells_net *ocr_segm_and_class_net(ocr_img_info *img, int divisions)
{
	if (img->bytes_for_pix != 0)
		return NULL;

	int i = 0, j = 0;
	int k = 0, curr = 0;
	int x_be = 0, x_en = 0;
	int y_be = 0, y_en = 0;
	int x_cell = 0, y_cell = 0;	// индексы ячейки в сетке
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int x_cell_count = 0;
	int cell_width = 0;
	int y_cell_count = 0;
	int cells_count = 0;
	int pix_count = 0;
	int curr_cell_width = 0;
	int up = 0, down = 0;		// переменные - индексы соседних клеток
	int right = 0, left = 0;	// -"-"-
	int up_l = 0, up_r = 0;		// -"-"-
	int down_l = 0, down_r = 0;	// -"-"-
	double curr_cross_cor = 0;	// -"-"-
	double sigma = 0.0;		// переменняа хранения сигмы
	double mu = 0.0;		// переменная дял хранения мат ожидания
	double cross_cor = 0.0;		// взаимная коррелиация
	ocr_cells_net *result = NULL;	// структура с результирующей информацией
	uchar *pix = img->pix;		// указатель на пиксели входного изображения
	uchar *output = NULL;		// указатель на пиксели выходного изображения
	coord begin;
	coord end;

	cell_width = width / divisions;			// вычисляем ширину клетки
	x_cell_count = divisions;			// вычисляем число клеток по оси х
	y_cell_count = height / cell_width;		// вычисляем число клеток по оси у
	cells_count = x_cell_count * y_cell_count;	// вычисляем общее число клеток

	/* Инициализируем выходное изображение. */
	output = (uchar *)malloc(sizeof(uchar) * y_cell_count * x_cell_count);
	if(output == NULL){
		printf("Не удалось выделить память для сегментационной сетки.\n");
		return NULL;
	}
	for(i = 0; i < cells_count; i++){
		output[i] = 0;
	}
	/* Определяем рабочую область. */
	ocr_segm_get_area(img, &begin, &end);
	x_cell = y_cell = 0;
	for(k = 0; k < cells_count; k++){
		/* Переход к новой строке сетки. */	
		y_cell = k / x_cell_count;
		x_cell = k % x_cell_count;
		/* Запоминаем индекса начала клетки. */
		x_be = x_cell * cell_width;
		y_be = y_cell * cell_width;
		/* Если блок последний, просматриваем пиксели до края. */
		x_en = (x_cell == x_cell_count - 1) ? width : (x_cell + 1) * cell_width;
		y_en = (y_cell == y_cell_count - 1) ? height : (y_cell + 1) * cell_width;
		curr_cell_width = x_en - x_be;
		/* Считаем число пикселей в клетке. */
		pix_count = (y_en - y_be) * curr_cell_width;
		cross_cor = 0;

		if (begin.x > x_be || begin.y > y_be ||
		end.x <= x_en || end.y <= y_en)
			continue;
		/* Считаем мат. ожидание дисперсию и взаимную коррелиацию каждой клетки. */
		for (i = y_be; i < y_en; i++) {
			curr_cross_cor = 0;
			for (j = x_be; j < x_en; j++) {
				curr = i * stride + j;
				mu += pix[curr];
				sigma += pix[curr] * pix[curr];
				if (i < height - 1)
					curr_cross_cor += (pix[curr] ^ pix[curr + 1]);
			}
			cross_cor += 1 - (curr_cross_cor * 0.5) / (curr_cell_width * CR_BLACK);
		}
		/**/
		mu /= CR_BLACK * pix_count;
		sigma /= CR_BLACK * pix_count;
		sigma = sqrt(sigma - mu * mu);
		cross_cor /= y_en - y_be;

		output[k] = CELL_BGROUND;
		/* Классифицируем клетки. */
//		if (cross_cor < 0.97 && mu > 0.03 && mu <= 0.85 && 
//		sigma <= 9 && sigma >= 3) {
		if (cross_cor < 0.97 && mu > 0.03 && mu <= 0.85 && 
		sigma <= 11 && sigma >= 2) {
			output[k] = CELL_TEXT;
		} else if (cross_cor >= 0.97 && mu >= 0.85 && sigma >= 9) {
			output[k] = CELL_BGROUND;//CELL_PIC;
		} else if (cross_cor > 0.97 && (mu <= 0.03 || mu >= 0.97) && sigma < 3) {
			output[k] = CELL_BGROUND;
		}
	}
	/* Накладываем маски. */
	for (k = x_cell_count; k < cells_count - x_cell_count; k++) {
		/* Запоминаем индексы соседних пикселей:
			|up_l  |up  |up_r  |
			|left  |k   |right |
			|down_l|down|down_r|
		*/
		up = k - x_cell_count;
		up_l = up - 1;
		up_r = up + 1;
		down = k + x_cell_count;
		down_l = down - 1;
		down_r = down + 1;
		left = k - 1;
		right = k + 1;
		/* Пропускаем граничные клетки. */
		if (k % x_cell_count == 0)
			continue;
		/* Размываем текст.
			|D|D|D|    |D|D|D|
			|2|X|D| -> |2|2|D|
			|2|2|D|    |2|2|D|
		*/
		if (output[left] == CELL_PIC && output[down] == CELL_PIC &&
		output[down_l] == CELL_PIC && output[k] != CELL_PIC)
			output[k] = CELL_PIC;
		/* Размываем текст.
			|2|2|D|    |2|2|D|
			|2|X|D| -> |2|2|D|
			|D|D|D|    |D|D|D|
		*/
		if (output[up_l] == CELL_PIC && output[up] == CELL_PIC &&
		output[left] == CELL_PIC && output[k] != CELL_PIC)
			output[k] = CELL_PIC;
		/*
			|D|2|2|    |D|2|2|
			|D|X|2| -> |D|2|2|
			|D|X|2|    |D|2|2|
		*/
		if (output[right] == CELL_PIC && output[down_r] == CELL_PIC &&
		output[up] == CELL_PIC && output[up_r] == CELL_PIC) {
			output[down] = CELL_PIC;
			output[k] = CELL_PIC;
		}
		/* Размываем текст.
			|D|D|D|    |D|D|D|
			|1|X|D| -> |1|1|D|
			|1|1|D|    |1|1|D|
		*/
		if (output[left] == CELL_TEXT && output[down] == CELL_TEXT &&
		output[down_l] == CELL_TEXT && output[k] != CELL_TEXT)
			output[k] = CELL_TEXT;
		/* Размываем текст.
			|1|1|D|    |1|1|D|
			|1|X|D| -> |1|1|D|
			|D|D|D|    |D|D|D|
		*/
		if (output[up_l] == CELL_TEXT && output[up] == CELL_TEXT &&
		output[left] == CELL_TEXT && output[k] != CELL_TEXT)
			output[k] = CELL_TEXT;
		/* Сглаживаем внутриние клетки текста по соседним.
			|D|D|D|     |D|1|D|    |D| |D|
			|1|X|1| или |D|X|D| -> | |1| |
			|D|D|D|     |D|1|D|    |D| |D|
		*/
		if (((output[left] == CELL_TEXT && output[right] == CELL_TEXT) ||
		(output[up] == CELL_TEXT && output[down] == CELL_TEXT)) && output[k] != CELL_TEXT)
			output[k] = CELL_TEXT;
		/* Сглаживаем внутриние клетки текста по соседним.
			|D|D|D|     |D|2|D|    |D| |D|
			|2|X|2| или |D|X|D| -> | |2| |
			|D|D|D|     |D|2|D|    |D| |D|
		*/
		if (((output[left] == CELL_PIC && output[right] == CELL_PIC) ||
		(output[up] == CELL_PIC && output[down] == CELL_PIC)) && output[k] != CELL_PIC)
			output[k] = CELL_PIC;
		/*
			|D|2|D|    |D|2|D|
			|D|1|D| -> |D|2|D|
			|D|0|D|    |D|0|D|
		*/
		if (output[up] == CELL_PIC && output[k] == CELL_TEXT) {
			output[k] = CELL_PIC;
		}
		/*
			|D|D|D|    |D|0|D|
			|2|1|0| -> |2|2|0|
			|D|D|D|    |D|D|D|
		*/
		if (output[left] == CELL_PIC && output[k] == CELL_TEXT) {
			output[k] = CELL_PIC;
		}	
	}
	/* Размываем области до прямоугольных. */
	for (k = cells_count - x_cell_count; k >= x_cell_count; k--) {
		/* Пропускаем граничные клетки. */
		if(k % x_cell_count == 0)
			continue;
		/* Запоминаем индексы соседних пикселей:
			|up_l  |up  |up_r  |
			|left  |k   |right |
			|down_l|down|down_r|
		*/
		up = k - x_cell_count;
		up_l = up - 1;
		up_r = up + 1;
		down = k + x_cell_count;
		down_l = down - 1;
		down_r = down + 1;
		left = k - 1;
		right = k + 1;

		/*
			|D|D|D|    |D|D|D|
			|D|X|2| -> |D|2|2|
			|D|2|2|    |D|2|2|
		*/
		if (output[right] == CELL_PIC && output[down] == CELL_PIC && 
		output[down_r] == CELL_PIC && output[k] != CELL_PIC)
			output[k] = CELL_PIC;
		/*
			|D|D|D|    |D|D|D|
			|D|2|2| -> |D|2|2|
			|D|X|2|    |D|2|2|
		*/
		if (output[right] == CELL_PIC && output[down_r] == CELL_PIC &&
		output[k] == CELL_PIC && output[down] != CELL_PIC) {
			output[down] = CELL_PIC;
		}
		/*
			|D|2|2|    |D|2|2|
			|D|X|2| -> |D|2|2|
			|D|X|2|    |D|2|2|
		*/
		if (output[right] == CELL_PIC && output[down_r] == CELL_PIC &&
		output[up] == CELL_PIC && output[up_r] == CELL_PIC) {
			output[down] = CELL_PIC;
			output[k] = CELL_PIC;
		}
		/*
			|D|D|D|    |D|D|D|
			|D|X|1| -> |D|1|1|
			|D|1|1|    |D|1|1|
		*/
		if (output[right] == CELL_TEXT && output[down] == CELL_TEXT && 
		output[down_r] == CELL_TEXT && output[k] != CELL_TEXT)
			output[k] = CELL_TEXT;
		/*
			|D|D|D|    |D|D|D|
			|D|1|1| -> |D|1|1|
			|D|X|1|    |D|1|1|
		*/
		if (output[right] == CELL_TEXT && output[down_r] == CELL_TEXT &&
		output[k] == CELL_TEXT && output[down] != CELL_TEXT) {
			output[down] = CELL_TEXT;
		}

		/* Сглаживаем внутриние клетки текста по соседним.
			|D|D|D|     |D|1|D|    |D| |D|
			|1|X|1| или |D|X|D| -> | |1| |
			|D|D|D|     |D|1|D|    |D| |D|
		*/
		if (((output[left] == CELL_TEXT && output[right] == CELL_TEXT) ||
		(output[up] == CELL_TEXT && output[down] == CELL_TEXT)) && output[k] != CELL_TEXT)
			output[k] = CELL_TEXT;
		/* Сглаживаем внутриние клетки текста по соседним.
			|D|D|D|     |D|2|D|    |D| |D|
			|2|X|2| или |D|X|D| -> | |2| |
			|D|D|D|     |D|2|D|    |D| |D|
		*/
		if (((output[left] == CELL_PIC && output[right] == CELL_PIC) ||
		(output[up] == CELL_PIC && output[down] == CELL_PIC)) && output[k] != CELL_PIC)
			output[k] = CELL_PIC;
		/*
			|D|0|D|    |D|0|D|
			|D|1|D| -> |D|2|D|
			|D|2|D|    |D|2|D|
		*/
		if (output[down] == CELL_PIC && output[k] == CELL_TEXT) {
			output[k] = CELL_PIC;
		}
		
		/*
			|D|D|D|    |D|D|D|
			|0|1|2| -> |0|2|2|
			|D|D|D|    |D|D|D|
		*/
		if (output[right] == CELL_PIC && output[k] == CELL_TEXT) {
			output[k] = CELL_PIC;
		}
	
		/* Сглаживаем внутриние клетки текста по соседним.
			|D|D|D|    |D|D|D|
			|0|1|0| -> |0|0|0|
			|D|D|D|    |D|D|D|
		*/
		if (output[left] == CELL_BGROUND && output[right] == CELL_BGROUND &&
		output[k] == CELL_TEXT)
			output[k] = CELL_BGROUND;

	}
	/* Проставляем грацниы областей рисунков и текста. */
	for (i = 0; i < cells_count; i++) {
		/* Пропускаем граничные клетки. */
		if(k % x_cell_count == 0)
			continue;

/*		if(i % x_cell_count == x_cell_count - 1)
			printf("\n");
*/		
		/* Запоминаем индексы соседних пикселей:
			|up_l  |up  |up_r  |
			|left  |k   |right |
			|down_l|down|down_r|
		*/
		up = i - x_cell_count;
		up_l = up - 1;
		up_r = up + 1;
		down = i + x_cell_count;
		down_l = down - 1;
		down_r = down + 1;
		left = i - 1;
		right = i + 1;

		/* Находим начало текстовой области.
			B - начало текстовой области
			D - не имеет значения
			|0|0|0|    |0|0|0|
			|0|1|1| -> |0|B|1|
			|0|D|D|    |0|D|D|
		*/
		if (output[up_l] != CELL_TEXT && output[up] != CELL_TEXT &&
		output[i] == CELL_TEXT && output[left] != CELL_TEXT &&
		output[right] == CELL_TEXT && output[down_l] != CELL_TEXT &&
		output[up_r] != CELL_TEXT && output[left] != CELL_CRNR_BE)
			output[i] = CELL_CRNR_BE;

		/* Находим конец текстовой области.
			E - конец текстовой области
			D - не имеет значения
			|D|D|0|    |D|D|0|
			|1|1|0| -> |1|E|0|
			|0|0|0|    |0|0|0|
		*/
		if (output[up_r] != CELL_TEXT && output[right] != CELL_TEXT &&
		output[i] == CELL_TEXT && output[down] != CELL_TEXT &&
		(output[left] == CELL_TEXT || output[left] == CELL_CRNR_BE) && 
		output[down_l] != CELL_TEXT && output[up_r] != CELL_TEXT && 
		output[down_r] != CELL_TEXT)
			output[i] = CELL_CRNR_END;

		/* Находим начало области рисунка.
			B - начало текстовой области
			D - не имеет значения
			|0|0|0|    |0|0|0|
			|0|1|1| -> |0|B|1|
			|0|D|D|    |0|D|D|
		*/
		if (output[up_l] != CELL_PIC && output[up] != CELL_PIC &&
		output[i] == CELL_PIC && output[left] != CELL_PIC &&
		output[right] == CELL_PIC && output[down_l] != CELL_PIC &&
		output[up_r] != CELL_PIC)
			output[i] = CELL_CRNR_BE;

		/* Находим конец области рисунка.
			E - конец текстовой области
			D - не имеет значения
			|D|D|0|    |D|D|0|
			|1|1|0| -> |1|E|0|
			|0|0|0|    |0|0|0|
		*/
		if(output[up_r] != CELL_PIC && output[right] != CELL_PIC &&
		output[i] == CELL_PIC && output[down] != CELL_PIC &&
		output[left] == CELL_PIC && output[down_l] != CELL_PIC &&
		output[up_r] != CELL_PIC && output[down_r] != CELL_PIC)
			output[i] = CELL_CRNR_END;

/*		if (output[i] == CELL_BGROUND)
			printf("_ ");
		else
			printf("%d ", output[i]);
	
*/	}

	/* Выделяем память для выходной информации о сетке. */
	result = (ocr_cells_net *)malloc(sizeof(ocr_cells_net));
	if (result == NULL){
		printf("Не удалось выделить память для результирующей структуры с информацией о сетке сегментации.\n");
		return NULL;
	}
	/* Сохраняем полученные настройки. */
	result->height = y_cell_count;
	result->cell_width = cell_width;
	result->cells_count = cells_count;
	result->width = x_cell_count;
	result->net = output;
	result->comp_count = 0;
	return result;
}

/* Находим области текста и картинок по границам. */
ocr_text_area **ocr_segm_find_net_areas(ocr_cells_net *net, int *areas_count) {
	int i = 0, j = 0, x = 0, k = 0;
	int cells_count = net->cells_count;
	int width = net->width;
	int height = net->height;
	int curr_type = 0;
	int curr_count = 0;
	uchar *cell = net->net;
	coord be, end;
	ocr_text_area *curr_text_area = NULL;
	ocr_text_area **text_areas = NULL;

	for (i = 0; i < cells_count; i++) {
		if (cell[i] == CELL_CRNR_BE) {		// если встретили метку начала области
			if (i + 1 >= cells_count)	// если след. клетка последняя
				continue;
			/* Запоминаем координаты начала области. */
			be.x = i % width;
			be.y = i / width;
			if (be.x > 0)
				be.x--;
			if (be.y > 0)
				be.y--;
			j = i + 1;
			/* Берем след. клетку за тип текущей области. */
			curr_type = cell[j];
			if (curr_type == CELL_BGROUND)
				continue;
			/* Сбрасываем коориднаты конца области. */
			end.x = -1;
			end.y = -1;
			while (j < cells_count) {
				/* Определяем x-координату следующей клетки. */
				x = (j + 1) % width;
				/* Если встретили символ конца области. */
				if (cell[j] == CELL_CRNR_END) {
					/* Запоминаем координаты. */
					end.x = j % width;
					end.y = j / width;
					break;
				}
				/* Если текущая клетка конец строки или следущая клетка фоновая. */
				if (x == 0 || cell[j + 1] == CELL_BGROUND) {
					/* Переходим к след. строке. */
					j += width;
					continue;
				}
				/* Перемещаемся дальше по области, пока текущая клетка
				соотв. текущему типу. */
				if (cell[j + 1] == curr_type || cell[j + 1] == CELL_CRNR_END) {
					j++;
				} else {
					/* Если встретили другой тип переходим на след. строку. */
					j += width;
				}
			}
			if (end.x != -1) {
				/* Увеличиваем счетчик областей. */
				curr_count++;
				/* Выделяем память для новой области. */
				text_areas = (ocr_text_area **)realloc(text_areas, sizeof(ocr_text_area*) * curr_count);
				if (text_areas == NULL) {
					printf("Не удалось выделить память для текстовой области при сегментации.\n");
					return NULL;
				}
				/* Записываем информацию о новой области. */
				curr_text_area = (ocr_text_area *)malloc(sizeof(ocr_text_area));
				if (curr_text_area == NULL) {
					printf("Не удалось выделить память для указателя на текстовую структуру.\n");
					return NULL;
				}
				if (end.x < width - 1)
					end.x++;
				if (end.y < height - 1)
					end.y++;
				curr_text_area->x = be.x;
				curr_text_area->y = be.y;
				curr_text_area->width = end.x - be.x + 1;
				curr_text_area->height = end.y - be.y + 1;
				curr_text_area->pix = NULL;
				text_areas[curr_count - 1] = curr_text_area;
			}
			if (j >= cells_count)
				continue;
		}
	}
	/* Убираем области попадающие в подмножества других. */
	for (i = 0; i < curr_count; i++) {
		for (j = 0; j < curr_count; j++) {
			/* Область с тем же индексом не рассматриваем. */
			if (j == i)
				continue;	
			/* Если хотя бы одна область попала в другую,
			удаляем ее. */
			if (text_areas[i]->x + text_areas[i]->width >= text_areas[j]->x &&
			text_areas[i]->x <= text_areas[j]->x && 
			text_areas[i]->y + text_areas[i]->height >= text_areas[j]->y && 
			text_areas[i]->y <= text_areas[j]->y) {
				/* Запоминаем указатель на последний элемент массива. */
				curr_text_area = text_areas[curr_count - 1];
				/* Перемещяем следущие элементы назад на один индекс. */
				for (k = j; k < curr_count - 1; k++)
					memcpy(text_areas[k], text_areas[k + 1], sizeof(ocr_text_area));
				curr_count--;	/* уменьшаем число областей */
				text_areas = (ocr_text_area **)realloc(text_areas, sizeof(ocr_text_area *) * curr_count);
				/* Удаляем область из рассмотрения. */
				free(curr_text_area);
				/* Чтоб не пропустить следующую область индекс уменьшаем,
				т.к. сдвинули массив назад на один. */
				i--;
				break;
			}
		}
	}
	/* Запоминаем число полученных областей. */
	*areas_count = curr_count;
	return text_areas;
}

void ocr_segm_get_text_area_on_img(ocr_img_info *img, int cells_count, ocr_text_area **cell_text)
{
	int i = 0, j = 0;
	int cell_width = 0;
	int width = 0, height = 0;
	int x = 0, y = 0;
	uchar *pix = img->pix;
	//cell_half = cell_width;// >> 1;		// делим пополам ширину клетки
	cell_width = img->width / cells_count;

	/* Переводим координаты в пространство изображения. */
	(*cell_text)->x *= cell_width;
	(*cell_text)->y *= cell_width;
	(*cell_text)->width *= cell_width;
	(*cell_text)->height *= cell_width;
	/* Захватываем полклетки с краев, чтобы гарантировано получить символ целиком. */
	/*if ((*cell_text)->x > 0) {
		(*cell_text)->x -= cell_half;
		(*cell_text)->width += cell_half;
	}
	if ((*cell_text)->y > 0) {
		(*cell_text)->y -= cell_half;
		(*cell_text)->height += cell_half;
	}
	if ((*cell_text)->width <= img->width - cell_width)
		(*cell_text)->width += cell_half;
	if ((*cell_text)->height <= img->height - cell_width)
		(*cell_text)->height += cell_half;
	*/
	/* Запоминаем ширину, высоту и координаты начала области. */
	width = (*cell_text)->width;
	height = (*cell_text)->height;
	x = (*cell_text)->x;
	y = (*cell_text)->y;
	(*cell_text)->pix = NULL;
	(*cell_text)->pix = (uchar *)malloc(sizeof(uchar) * width * height);
	if ((*cell_text)->pix == NULL) {
		printf("Не удалось выделить память для пикселей текстовой области.\n");
		return;
	}
	/* Заполняем пиксели. */
	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++) {
			if (pix[(i + y) * img->stride + x + j] == CR_BLACK)
				(*cell_text)->pix[i * width + j] = CR_BLACK;
			else 
				(*cell_text)->pix[i * width + j] = CR_WHITE;
			//printf("%d ", cell_text->pix[i * width + j]);
		}
}

/*********************************************************************************/
/*
* Функция возвращает массив долей черных пикселей в каждой строке или
* NULL в случае ошибки.
*/
double *ocr_segm_get_line_stat(ocr_text_area *text_area)
{
	int i = 0, j = 0;
	double *result = NULL;
	if (text_area->width == 0) {
		printf("Обнаружена нулевая ширина области.\n");
		return NULL;
	}
	result = (double *)malloc(sizeof(double) * text_area->height);
	if (result == NULL) {
		printf("Не удалось выделить память для массива долей черных пикселей.\n");
		return NULL;
	}
	/* Подсчитываем долю черных пикселй в каждой строке.  */
	for (i = 0; i < text_area->height; i++) {
		result[i] = 0.0;
		for (j = 0; j < text_area->width; j++) {
			//printf("p:%dx%d\n", i, j);
			//printf("pPP:%d\n", text_area->pix[i * text_area->width + j]);
			result[i] += text_area->pix[i * text_area->width + j];
		}
		result[i] /= CR_BLACK * text_area->width;
	}
	return result;
}

/*********************************************************************************/
double *ocr_segm_get_coll_stat(ocr_text_area *text_area)
{
	int i = 0, j = 0;
	double *result = NULL;

	if (text_area->height == 0) {
		printf("Обнаружена нулевая высота области.\n");
		return NULL;
	}

	result = (double *)malloc(sizeof(double) * text_area->width);

	if (result == NULL) {
		printf("Не удалось выделить память для массива долей черных пикселей.\n");
		return NULL;
	}
	/* Подсчитываем долю черных пикселй в каждой столбце.  */
	for (i = 0; i < text_area->width; i++) {
		result[i] = 0.0;
		for (j = 0; j < text_area->height; j++) {
			result[i] += text_area->pix[j * text_area->width + i];
		}
		result[i] /= CR_BLACK * text_area->height;
	}
	return result;
}

ocr_text_area **ocr_segm_stat_lines_area(ocr_text_area *text_area, int *line_count)
{
	if(text_area->width <= 0)
		return NULL;
	int width = text_area->width;	// ширина текстовой области
	int height = text_area->height;	// высота текстовой области
	int i = 0, j = 0, k = 0;	// индексы
	int curr = 0;
	int shift = 1;
	int line_be = 0, line_end = 0;	// индексы начала и конца слова в строке
	int line_height = 0;		// высота строки
	int start_ind = 0;		// индекс, откуда начинаются строки
	int h_curr = 0, h_next = 0;
	int h_new = 0;
	int size = 0;
	int line_size = width * sizeof(uchar);
	int curr_count = 0;
	double mu = 0.0;		// доля черных пикселей в одной строке
	double thrshld = 0.0;		// пороговое значение доли черных пикселей
	double *line_stat = NULL;	// массив долей черных пикселей в каждой строке
	uchar state = 0;		// переменная указывает на текущее состояние 0 - елси строка,
					// 255 - если отступ
	uchar *pix = text_area->pix;	// 2-мерный массив пикселей текстовой области
	ocr_text_area **lines = NULL;	// результирующий массив текстовых областей строк
	ocr_text_area *add_area;	// добавляемая текстовая область

	//(*line_count) = 0;
	/******************** Находим порог для разбиения. *******************/
	line_stat = ocr_segm_get_line_stat(text_area);
	if (line_stat == NULL) {
		printf("Не удалось получить информацию о доле черных пикселей в строках.\n");
		return NULL;
	}
	/*for (i = 0; i < text_area->height; i++) {
		mu += line_stat[i];
		sigma += line_stat[i] * line_stat[i];
	}
	mu /= text_area->height;
	sigma /= text_area->height;
	sigma = sqrt(mu - sigma);*/
	thrshld = 0.05;
	/**********************************************************************/
	/* Находим начальный индекс, с которого начинаются строки. */
	for (i = 0; i < height; i++) {
		if (line_stat[i] > thrshld) {
			start_ind = i;
			break;
		}
	}
	line_be = start_ind;	// запоминаем индекс
	for (i = start_ind; i < height - shift; i++) {
		/* Вычисляем долю черных пикселей в столбце пикселей. */
		mu = 0.0;
		for (k = 0; k < shift; k++) {
			mu += line_stat[i + k];
		}
		mu /= shift;
		if ((mu < thrshld && state == 0) || (i == height - shift - 1)) {	// если переходим в режим разделителя после символа
			state = 255;		// меняем на режим разделителя
			line_end = (i + shift < height) ? i + shift : height - 1;	// индекс конца символа
			line_height = line_end - line_be;	// определяем высоту строки
			/* Если ширина положительна, то создадим новую строку. */
			if (line_height > 0) {
				add_area = (ocr_text_area *)malloc(sizeof(ocr_text_area));
				add_area->y = line_be + text_area->y;
				add_area->x = text_area->x;
				//printf("Line: %dx%d\n", add_area->x, add_area->y);
				add_area->height = line_height;
				add_area->width = width;
				size = line_height * width;
				/* Копируем область строки из текстовой области. */
				add_area->pix = (uchar *)malloc(sizeof(uchar) * size);
				//memset(add_area->pix, CR_WHITE, sizeof(uchar) * size);
				if (add_area->pix == NULL) {
					printf("Не удалось выделить память для текстовой строки.\n");
					return NULL;
				}
				add_area->pix = (uchar *)memcpy(add_area->pix, &(pix[line_be * line_size]), line_height * line_size);
				
				if (add_area->pix == NULL) {
					printf("не удалось скопировать.\n");
					continue;
				}
				/* Добавление новую текстовую область в массив области. */
				curr_count++;
				lines = (ocr_text_area **)realloc(lines, sizeof(ocr_text_area *) * curr_count);
				if (lines == NULL) {
					printf("Не удалось выделить память для новой текстовой строки.\n");
					return NULL;
				}
				/* Добавляем новый элемент в массив. */
				lines[curr_count - 1] = add_area;
				line_be = height - 1;	// сбрасываем индекс начала символа
			}
		} else if (mu > thrshld && state == 255) {
			state = 0;		// меняем на режим символа
			line_be = i - shift;	// запоминаем индекс начала символа
		}
	}

	/* Проссмтриваем все строки на наличие ошибочно распознанных
	"межстрочных" строк. */
	for (i = 0; i < curr_count - 1; i++) {
		h_curr = lines[i]->height;
		h_next = lines[i + 1]->height;
		/* Если следущая строка больше чем в 3 раза, то текущая строка
		ошибочная, следовательно объединяем ее со следущей строчкой. */
		if ((double)h_next / h_curr >= 3) {
			/* Задаем новую высоту - промежуток текстовой области от
			начала текущей строки до конца следующей. */
			h_new = lines[i + 1]->y - lines[i]->y + h_next;
			/* Перевыделим память для пикселей. */
			lines[i + 1]->pix = (uchar *)realloc(lines[i + 1]->pix, line_size * h_new);
			/* Зададим новой области соотв. характеристики. */
			lines[i + 1]->y = lines[i]->y;
			lines[i + 1]->x = lines[i]->x;
			lines[i + 1]->height = h_new;
			lines[i + 1]->width = width;
			/* Индекс пикселя, откада пойдет текстовая область. */
			curr = line_size * (lines[i]->y - text_area->y);
			/* Копируем нужную область. */
			lines[i + 1]->pix = (uchar *)memcpy(lines[i + 1]->pix, &(pix[curr]), h_new * line_size);
			/* Сдвинаем элементы массива на один вверх. */
			for (j = i; j < curr_count - 1; j++) {
				memcpy(lines[j], lines[j + 1], sizeof(ocr_text_area));
			}
			/* Удаляем последний элемент. */
			free(lines[curr_count - 1]);
			/* Уменьшаем размерность. */
			curr_count--;
		}
	}

	free(line_stat);
	*line_count = curr_count;
	return lines;
}

ocr_text_area **ocr_segm_stat_words_area(ocr_text_area *line_area, int *word_count)
{
	if (line_area->height <= 0) {
		printf("Высота строки равна 0.\n");
		return NULL;
	}

	int width = line_area->width;	// ширина текстовой области
	int height = line_area->height;	// высота текстовой области
	int i = 0, k = 0;		// индексы
	int shift = (height >> 2) * 1.5;// будем считать длину разделителя за четверть высоты
	int mid_shift = 0;//shift >> 1;	// середина шага
	int word_be = 0, word_end = 0;	// индексы начала и конца слова в строке
	int word_width = 0;		// ширина слова
	int start_ind = 0;		// индекс, откуда начинаются слова
	int line_size = sizeof(uchar) * width;
	double mu = 0.0;		// доля черных пикселей в столбце
	double thrshld = 0.0005;	// пороовое значение доли черных пикселей
	double *colls_stat = NULL;	// доля черных пикселей в каждом столбце
	uchar state = 0;		// переменная указывает на текущее состояние 0 - елси слово,
					// 255 - если разделитель.
	uchar *pix = line_area->pix;	// 2-мерный массив пикселей текстовой области
	ocr_text_area **words = NULL;	// рещультирующий массив текстовых областей слов
	ocr_text_area *add_area;	// добавляемая текстовая область

	words = NULL;
	(*word_count) = 0;
	//add_area = (ocr_text_area *)malloc(sizeof(ocr_text_area));

	/* Вычисляем долю черных пикселей в каждом столбце. */
	colls_stat = ocr_segm_get_coll_stat(line_area);
	if (colls_stat == NULL) {
		printf("Ошибка при определении доли черных пикселей в каждом столбце.\n");
		return NULL;
	}

	/* Находим начало инжекс, с которого начинаются символы. */
	for (i = 0; i < width; i++) {
		if (colls_stat[i] > thrshld) {
			start_ind = i;
			break;
		}
	}
	word_be = start_ind;	// запоминаем индекс
	for (i = start_ind; i < width - shift; i++) {
		/* Вычисляем долю черных пикселей в скользящем окне столбцов пикселей. */
		mu = 0.0;
		for (k = 0; k < shift; k++) {
			mu += colls_stat[i + k];
		}
		mu /= shift;
		if ((mu < thrshld && state == 0) || i == width - shift - 1) {	// если переходим в режим разделителя после символа
			state = 255;		// меняем на режим разделителя
			word_end = (i + mid_shift < width) ? i + mid_shift : width - 1;	// индекс конца символа
			word_width = word_end - word_be;	// определяем ширину символа
			/* Если ширина положительна, то создадим новый символ. */
			if (word_width > 0) {
				add_area = (ocr_text_area *)malloc(sizeof(ocr_text_area));
				add_area->x = word_be + line_area->x;
				add_area->y = line_area->y;
				add_area->width = word_width;
				add_area->height = height;
				/* Копируем область символа из области слова. */
			//	size = height * word_width;
				add_area->pix = (uchar *)malloc(sizeof(uchar) * word_width * height);
				//memcpy(add_area->pix, pix, size);
				for (k = 0; k < height; k++) {
					memcpy(&(add_area->pix[k * word_width]), &(pix[k * line_size + word_be]), word_width);
					/*for(l = 0; l < word_width; l++){
						curr = k * word_width + l;
						if (pix[k * width + l + word_be] == CR_BLACK)
							add_area->pix[k * word_width + l] = CR_BLACK;
						else
							add_area->pix[k * word_width + l] = CR_WHITE;
					}*/
				}
				/* Добавление новой текстовой в массив области. */
				(*word_count)++;
				words = (ocr_text_area **)realloc(words, sizeof(ocr_text_area *) * (*word_count));
				words[*word_count - 1] = add_area;
				word_be = width - 1;	// сбрасываем индекс начала символа
			}
		} else if (mu > thrshld && state == 255) {
			state = 0;			// меняем на режим символа
			word_be = i - mid_shift;	// запоминаем индекс начала символа
		}
	}
	free(colls_stat);
	return words;
}

ocr_text_area **ocr_segm_stat_chars_area(ocr_text_area *word_area, int *char_count)
{
	if (word_area->height <= 0)
		return NULL;
	int width = word_area->width;		/* Для удобства запоминаем ширину, */
	int height = word_area->height;		/* высоту текстовой области слова. */
	int i = 0, k = 0, l = 0;		/* Индексы. */
	int shift = (int)(height * 0.075);	/* Будем считать длину разделителя за 1/20 высоты. */
	int char_b = 0, char_e = 0;	/* Индексы начала и конца символа в слове. */
	int char_w = 0;			/* Ширина символа. */
	int char_h = 0;			/* Высота текущего символа. */
	int start_ind = 0;		/* Индекс, откуда начинаются символы. */
	int up_border = 0;		/* Индекс строки, с которой начинается символ. */
	int curr = 0;
	int curr_count = 0;
	double mu = 0.0;		/* Доля черных пикселей в столбце. */
	double mu_height = 0.0;		/* Доля черных пикселей в строке. */
	double thrshld = 0.005;		/* Пороовое значение доли черных пикселей. */
	double *colls_stat = NULL;
	uchar state = 0;		/* Переменная текущего состояния: 0 - елси символ, */
					/* 255 - если разделитель. */
	uchar *pix = word_area->pix;	/* Массив пикселей текстовой области. */
	ocr_text_area **chars = NULL;	/* Рещультирующий массив текстовых областей символов. */
	ocr_text_area *add_area = NULL;	/* Добавляемая текстовая область. */

	chars = NULL;

	colls_stat = ocr_segm_get_coll_stat(word_area);
	add_area = NULL;

	if (colls_stat == NULL) {
		printf("Ошибка при определении доли черных пикселей в каждом столбце.\n");
		return NULL;
	}

	/* Находим начало инжекс, с которого начинаются символы. */
	for (i = 0; i < width; i++) {
		if (colls_stat[i] > thrshld) {
			start_ind = i;
			break;
		}
	}
	char_b = start_ind;	// запоминаем индекс
	for (i = start_ind; i < width - shift; i++) {
		/* Вычисляем долю черных пикселей в столбце пикселей. */
		mu = 0.0;
		for (k = 0; k < shift; k++) {
			mu += colls_stat[i + k];
		}
		mu /= shift;
		if ((mu < thrshld && state == 0) || (i == width - shift - 1)) {	/* если переходим в режим разделителя после символа */
			state = 255;			/* меняем на режим разделителя */
			char_e = i + 1;			/* индекс конца символа */
			char_w = char_e - char_b;	/* определяем ширину символа */
			/* Если ширина положительна, то создадим новый символ. */
			if (char_w > 0) {
				add_area = (ocr_text_area *)malloc(sizeof(ocr_text_area));
				if (add_area == NULL) {
					printf("Не.\n");
					break;
				} 
				add_area->x = char_b + word_area->x;
				add_area->y = word_area->y;
				add_area->width = char_w;
				/* По умолчанию зададим высоту совпадающую с высотой слова. */
				char_h = height;
				/* Верхнюю границу начнем с 0. */
				up_border = 0;
				/* Определеям границы символа . */
				for (k = 0; k < height; k++) {
					mu_height = 0;
					for (l = char_b; l < char_e; l++) {
						mu_height += pix[k * width + l];
					}
					mu_height /= CR_BLACK * char_w;
					if (mu_height < thrshld * 4) {
						char_h--;	/* уменьшаем высоту символа */
						add_area->y++;
						up_border++;	/* повышаем верхнюю границу символа */
					} else {
						break;
					}
				}
				/* Обрезеам строки, если доля черных
				пикселей меньше порога. */
				for (k = height - 1; k >= 0; k--) {
					mu_height = 0;
					for (l = char_b; l < char_e; l++) {
						mu_height += pix[k * width + l];
					}
					mu_height /= CR_BLACK * char_w;
					if (mu_height < thrshld * 4) {
						char_h--;
					//`	add_area->y++;
					} else {
						break;
					}
				}
				add_area->height = char_h;
				/* Копируем область символа из области слова. */
				add_area->pix = (uchar *)malloc(sizeof(uchar) * char_h * char_w);
				//memset(add_area->pix, 0, char_h * char_w);
				for (k = 0; k < char_h; k++) {
					//curr = k * width + char_b;
					//memcpy(&(add_area->pix[k * char_w]), &(pix[curr]), char_w);
					
					for (l = 0; l < char_w; l++) {
						curr = (k + up_border) * width + char_b + l;
						//add_area->pix[k * char_w + l] = pix[curr];
						if (pix[curr] == CR_BLACK)
							add_area->pix[k * char_w + l] = CR_BLACK;
						else
							add_area->pix[k * char_w + l] = CR_WHITE;
					}	
					
				}
				/* Добавление новой текстовой в массив области. */
		//		printf("11CWL%dx%d\n", char_w, height);
				curr_count++;
				chars = (ocr_text_area **)realloc(chars, sizeof(ocr_text_area *) * curr_count);
				chars[curr_count - 1] = add_area;
				//char_b = width - 1;	// сбрасываем индекс начала символа
			}
			char_b = width - 1;	// сбрасываем индекс начала символа
		} else if (mu > thrshld && state == 255) {
			state = 0;			// меняем на режим символа
			char_b = i + shift;
		}
	}
	(*char_count) = curr_count;
	free(colls_stat);
	return chars;
}


void ocr_segm_stat_chars_recovery(ocr_img_info *img, ocr_text_area **chars, int char_count)
{
	int i = 0, j = 0;
	int k = 0;
	int stride = img->stride;
	int height = img->height;
	int char_w = 0;
	int char_h = 0;
	int c_stride = 0;
	int char_b = 0;
	int char_e = 0;
	int new_y = 0;
	int y = 0;
	int half_h = 0;
	int uc_size =  sizeof(uchar);
	double thrshld = 0.005;
	double mu = 0.0;
	uchar *pix = img->pix;
	uchar *c_pix = NULL;
	uchar *tmp = NULL;

	for (i = 0; i < char_count; i++) {
		char_w = chars[i]->width;
		char_h = chars[i]->height;
		half_h = char_h / 2;
		c_stride = char_w * uc_size;
		c_pix = chars[i]->pix;
		char_b = chars[i]->x;
		char_e = char_b + char_w;
		y = chars[i]->y;
		new_y = (y - half_h < 0) ? 0 : y - half_h;
		/* Просматриваем строки сверху. */
		for (j = y - 1; j >= new_y; j--) {
			mu = 0.0;
			for (k = char_b; k < char_e; k++) {
				mu += pix[j * stride + k];
			}
			mu /= CR_BLACK * char_w;
			/* Если значение больше порогового. */
			if (mu >= thrshld) {
				/* Копируем область символа во временный буфер. */
				tmp = (uchar *)malloc(c_stride * char_h);
				if (tmp == NULL) {
					printf("Не удалось выделить память для временного буффера при расширении текстовой области символа.\n");
					break;
				}
				memcpy(tmp, c_pix, c_stride * char_h);
				char_h++;
				/* Рассширяем текстовую область. */
				chars[i]->pix = (uchar *)realloc(chars[i]->pix, c_stride * char_h);
				if (chars[i] == NULL) {
					printf("Не удалось перевыделить память для расширенного символа.\n");
					break;
				}
				c_pix = chars[i]->pix;
				/* Копируем добавляемую строку с изображения. */
				memcpy(c_pix, &(pix[stride * j + char_b]), c_stride);
				/* Копируем прежнюю область. */
				memcpy(&(c_pix[c_stride]), tmp, c_stride * (char_h - 1));
				/* Смещаем y-индекс на 1 вверх. */
				chars[i]->y--;
				/* Запоминаем новую высоту. */
				chars[i]->height = char_h;
				free(tmp);
			}
		}
		/* Просматрим строки ниже символа. */
		y = chars[i]->y + char_h;
		new_y = (y + half_h >= height) ? height : y + half_h;
		for (j = y; j < new_y; j++) {
			mu = 0.0;
			/* Находим долю черных пикселей. */
			for (k = char_b; k < char_e; k++) {
				mu += pix[j * stride + k];
			}
			mu /= CR_BLACK * char_w;
			/* Если значение больше порогового. */
			if (mu >= thrshld) {
				/* Копируем область символа во временный буфер. */
				tmp = (uchar *)malloc(c_stride * char_h);
				if (tmp == NULL) {
					printf("Не удалось выделить память для временного буффера при расширении текстовой области символа.\n");
					break;
				}
				memcpy(tmp, c_pix, c_stride * char_h);
				char_h++;
				/* Рассширяем текстовую область. */
				chars[i]->pix = (uchar *)realloc(chars[i]->pix, c_stride * char_h);
				if (chars[i] == NULL) {
					printf("Не удалось перевыделить память для расширенного символа.\n");
					break;
				}
				c_pix = chars[i]->pix;
				/* Копируем прежнюю область. */
				memcpy(c_pix, tmp, c_stride * (char_h - 1));
				/* Копируем добавляемую строку с изображения. */
				memcpy(&(c_pix[c_stride * (char_h - 1)]), &(pix[stride * j + char_b]), c_stride);
				/* Запоминаем новую высоту. */
				chars[i]->height = char_h;
				free(tmp);
			}
		}
	}
}

/*******************************************************************/
ocr_text_area **ocr_segm_get_text_areas(ocr_img_info *img, int cells_count, int *text_areas_count)
{
	ocr_cells_net *net = NULL;
	ocr_text_area **text_areas = NULL;
	int i = 0;
	int areas_count = 0;

	/* Разбиваем на клетки, классифифцируем их. */
	if ((net = ocr_segm_and_class_net(img, cells_count)) == NULL) {
		printf("Не удалось получить сетку сегментации.\n");
		return NULL;
	}
	/* Получаем границы текстовых областей. */
	if ((text_areas = ocr_segm_find_net_areas(net, &areas_count)) == NULL) {
		printf("Не удалось найти ни одной текстовой области.\n");
		return NULL;
	}
	/* Переводим координаты сетки в координаты изображения и заполняем
	пикселями входного изображения. */
	for (i = 0; i < areas_count; i++) {
		ocr_segm_get_text_area_on_img(img, cells_count, &text_areas[i]);
	}
	*text_areas_count = areas_count;
	return text_areas;
}

/* Функция находит индексы начала и конца разделителя страниц. */
void ocr_segm_page_vert_divisor(ocr_img_info *img, int *div_be, int *div_end)
{
	if (img->bytes_for_pix != 1)
		return;

	int i = 0, j = 0, k = 0;
	int curr = 0;			// индекс рассматриваемого пикселя
	int width = img->width;
	int height = img->height;
	int stride = img->stride;
	int middle_be = (width >> 1) - (width >> 3);	// индекс начала диапазона середины
	int middle_en = (width >> 1) + (width >> 3);	// индекс конца диапазона середины
	int window_width = 10;
	int be = -1;
	uchar *pix = img->pix;
	double stat = 0.0;

	/* Ставим возвращаемые значения по умолчанию. */
	*div_end = -1;
	*div_be = -1;
	/* Пробегаем скользящим окном толщиной в 10 пикселей. */
	for(i = 0; i < width - window_width; i++){
		stat = 0;
		for(j = 0; j < height; j++){
			curr = j * stride + i;
			for(k = 0; k < window_width; k++){
				stat += pix[curr + k];
			}
		}
		/* Оцениваем отношение черных пикселей ко всем. */
		stat /= CR_BLACK * height * window_width;
		/* Находим индекс столбца, откуда начинается разделитель
		страницы (если он есть). */
		if(stat > 0.6 && i > middle_be && i < middle_en){
			/* Делитель изображения находится примерно посередине. */
			be = i;
			break;
		}
	}
	if(be == -1)
		return;

	*div_be = be;
	/* Определяем диапазон разделителя страницы. */
	/* Находим начало диапазона. */
	for(i = be; i >= middle_be; i--){
		stat = 0;
		for(j = 0; j < height; j++){
			curr = j * stride + i;
			stat += pix[curr];
		}
		/* Оцениваем отношение черных пикселей ко всем. */
		stat /= CR_BLACK * height;
		if(stat < 0.1){
			*div_be = i;
			break;
		}
	}

	/* Находим конец диапазона. */
	for(i = be; i < middle_en; i++){
		stat = 0;
		for(j = 0; j < height; j++){
			curr = j * stride + i;
			stat += pix[curr];
		}
		/* Оцениваем отношение черных пикселей ко всем. */
		stat /= CR_BLACK * height;
		if(stat < 0.1){
			*div_end = i;
			break;
		}
	}
}

int ocr_segm_get_page_count(ocr_img_info *img)
{
	int be, end;
	ocr_segm_page_vert_divisor(img, &be, &end);
	return (be == -1 || end == -1) ? 1 : 2;
}

void ocr_segm_get_area(ocr_img_info *img, coord *begin, coord *end)
{
	int i = 0, j = 0, k = 0;
	int curr = 0;
	int width = img->width;
	int height = img->height;
	int stride = img->stride;
	int space_begin = -1;
	int win_size = 5;		// ширина скользящего окна
	uchar *pix = img->pix;
	double stat = 0.0;
	double small_mu = 0.25;
	begin->x = 0;
	begin->y = 0;
	end->x = width - 1;
	end->y = height - 1;

	/* Ищем левую границу области в первой четверти. */
	for(i = 1; i < width - win_size; i++){
		stat = 0.0;
		for(j = 0; j < height; j++){
			curr = stride * j + i;
			/* Складываем значения попавшие в скользящее окно. */
			for(k = 0; k < win_size; k++)
				stat += pix[curr + k];
		}
		stat /= CR_BLACK * height * win_size;
		/* Находим начало "бeлой" рамки. */
		if(stat < small_mu && space_begin == -1){
			begin->x = i;
			break;
		}
	}
	/* Ищем правую границу. */
	for(i = width - 1; i >= win_size; i--){
		stat = 0.0;
		for(j = 0; j < height; j++){
			curr = stride * j + i;
			for(k = 0; k < win_size; k++)
				stat += pix[curr - k];
		}
		stat /= CR_BLACK * height * win_size;
		/* Попадаем в информативную область. */
		if(stat < small_mu){
			end->x = i;
			break;
		}
	}
	/* Ищем верхнюю границу. */
	for(i = 1; i < height - win_size; i++){
		stat = 0.0;
		for(j = 0; j < width; j++){
			curr = stride * i + j;
			for(k = 0; k < win_size; k++)
				stat += pix[curr + k];
		}
		stat /= CR_BLACK * width * win_size;
		/* Попадаем в информативную область. */
		if(stat < small_mu){
			begin->y = i;
			break;
		}
	}
	/* Ищем нижнюю границу. */
	for(i = height - 1; i > win_size; i--){
		stat = 0.0;
		for(j = 0; j < width; j++){
			curr = stride * i + j;
			stat += pix[curr];
		}
		stat /= CR_BLACK * width * win_size;
		/* Попадаем в информативную область. */
		if(stat < small_mu){
			end->y = i + 1;
			break;
		}
	}
}






ocr_img_info *ocr_segm_rls_vert(ocr_img_info *img, int shift)
{
	if(img->bytes_for_pix != 0)
		return NULL;
	
	int i = 0, j = 0;
	int k = 0, curr = 0;
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int tmp = 0;
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	uchar *pix = img->pix;
	uchar *out_img = (uchar *)malloc(sizeof(char) * stride * height);

	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			curr = j * stride + i;
			/* Определяем, до какого пикселя "размывать". */
			tmp = (j > shift) ? curr - shift * stride : curr - j * stride;
			if (pix[curr] == CR_BLACK && pix[tmp] == CR_BLACK) {
				/*  Обращаем все предыдущие белые пиксели в черные. */
				for (k = curr; k >= tmp; k -= stride) {
					out_img[k] = CR_BLACK;
				}
			} else
				out_img[curr] = CR_WHITE;
		}
	}

	result->width = width;
	result->height = height;
	result->stride = stride;
	result->bytes_for_pix = 0;
	result->pix = out_img;

	return result;
}

ocr_img_info *ocr_segm_rls_horizont(ocr_img_info *img, int shift)
{
	if(img->bytes_for_pix != 0)
		return NULL;
	
	int i = 0, j = 0;
	int k = 0, curr = 0;
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int tmp = 0;
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	uchar *pix = img->pix;
	uchar *out_img = (uchar *)malloc(sizeof(char) * stride * height);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			curr = i * stride + j;
			tmp = (j > shift) ? curr - shift : curr - j;
			if (pix[curr] == CR_BLACK && pix[tmp] == CR_BLACK) {
				/* Определяем, до какого пикселя "размывать". */
				/*  Обращаем все предыдущие белые пиксели в черные. */
				for (k = curr; k >= tmp; k--) {
					out_img[k] = CR_BLACK;
				}
			} else
				out_img[curr] = CR_WHITE;
		}
	}

	result->width = width;
	result->height = height;
	result->stride = stride;
	result->bytes_for_pix = 0;
	result->pix = out_img;

	return result;
}


/*void ocr_segm_to_rect(ocr_img_info *img)
{
	int i = 0, j = 0;
	int curr = 0;
	int width = img->width;
	int height = img->height;
	int stride = img->stride;
	uchar *pix = img->pix;

	for (i = 0; i < height - 1; i++) {
		for (j = 0; j < width - 1; j++) {
			curr = i * stride + j;
			if (pix[curr - stride] == CR_BLACK &&
			pix[curr - 1] == CR_BLACK) {
				pix[curr] = CR_BLACK;
			}
		}
	}
}*/


ocr_img_info *ocr_segm_bloomberg(ocr_img_info *img)
{
	int origin = 0;		// индекс начала координат образца
	int size = 0, i = 0;
	ocr_img_info *struct_elem = NULL;
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	if (result == NULL) {
		printf("Не удалось выделить память для структуры информации об изображении в алгоритме Блумберга.\n");
		return NULL;
	}
	/* Копируем информацию об изображении для дальнейшей обработки. */
	result->width = img->width;
	result->height = img->height;
	result->stride = img->stride;
	result->bytes_for_pix = img->bytes_for_pix;
	size = result->stride * result->height;
	/* Копируем пиксели из исходного изображения. */
	result->pix = (uchar *)malloc(sizeof(uchar) * size);
	if (result->pix == NULL) {
		printf("Не удалось выделить память для пикселей выходного изображения в алгоритме Блумберга.\n");
		return NULL;
	}
	for (i = 0; i < size; i++)
		result->pix[i] = img->pix[i];
	/* Сжимаем изображение по порогу '1'.*/
	ocr_imgproc_threshold_reduction(result, 2, 1);
	/* Сжимаем изображение по порогу '1'.*/
	ocr_imgproc_threshold_reduction(result, 2, 1);
	/* Сжимаем изображение по порогу '3'.*/
	ocr_imgproc_threshold_reduction(result, 2, 3);
	/* Сжимаем изображение по порогу '4'.*/
	ocr_imgproc_threshold_reduction(result, 2, 4);
	/* Генерируем образец для морфологической обработки 5х5. */
	struct_elem = ocr_imgproc_get_se_nxn(5);
	origin = 12;
	/* Применяем морфологическое размыкание. */
	ocr_imgproc_morph_open(result, struct_elem, origin);
	free(struct_elem);
	/* Расширяем обратно изображение дважды. */
	ocr_imgproc_threshold_expansion(result, 2);
	ocr_imgproc_threshold_expansion(result, 2);
	/* Генерируем образец для морфологической обработки 3х3. */
	struct_elem = ocr_imgproc_get_se_nxn(3);
	origin = 4;
	/* Применяем дилатацию. */
	ocr_imgproc_morph_dilate(result, struct_elem, origin);
	free(struct_elem);
	/* Расширяем обратно изображение. */
	ocr_imgproc_threshold_expansion(result, 2);
	ocr_imgproc_threshold_expansion(result, 2);
	return result;
}

ocr_img_info *ocr_segm_bloomberg_modify(ocr_img_info *img)
{	
	int origin = 0;		// индекс начала координат образца
	int size = 0, i = 0;
	ocr_img_info *struct_elem = NULL;
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	if (result == NULL) {
		printf("Не удалось выделить память для структуры информации об изображении в алгоритме Блумберга.\n");
		return NULL;
	}
	/* Копируем информацию об изображении для дальнейшей обработки. */
	result->width = img->width;
	result->height = img->height;
	result->stride = img->stride;
	result->bytes_for_pix = img->bytes_for_pix;
	size = result->stride * result->height;
	/* Копируем пиксели из исходного изображения. */
	result->pix = (uchar *)malloc(sizeof(uchar) * size);
	if (result->pix == NULL) {
		printf("Не удалось выделить память для пикселей выходного изображения в алгоритме Блумберга.\n");
		return NULL;
	}
	for (i = 0; i < size; i++)
		result->pix[i] = img->pix[i];
	/* Сжимаем изображение по порогу '1'.*/
	ocr_imgproc_threshold_reduction(result, 2, 1);
	/* Сжимаем изображение по порогу '1'.*/
	ocr_imgproc_threshold_reduction(result, 2, 1);
	/* Сжимаем изображение по порогу '3'.*/
	ocr_imgproc_threshold_reduction(result, 2, 3);
	/* Сжимаем изображение по порогу '4'.*/
	ocr_imgproc_threshold_reduction(result, 2, 4);
	/* Применяем морф. размывкание "крестом"
	для стирания остатков от символов. */
	struct_elem = ocr_imgproc_get_se_nxn(3);
	struct_elem->pix[0] = CR_WHITE;
	struct_elem->pix[2] = CR_WHITE;
	struct_elem->pix[6] = CR_WHITE;
	struct_elem->pix[8] = CR_WHITE;
	origin = 4;
	ocr_imgproc_morph_open(result, struct_elem, origin);
	free(struct_elem);
	struct_elem = ocr_imgproc_get_se_nxn(3);
	/* Применяем дилатацию "квдратом" для 
	восстановления части потеряной информации. */
	origin = 4;
	ocr_imgproc_morph_dilate(result, struct_elem, origin);
	free(struct_elem);

	//result = ocr_segm_rls_horizont(result, 10);
	//ocr_segm_to_rect(result);

	/* Расширяем обратно изображение дважды. */
	ocr_imgproc_threshold_expansion(result, 2);
	ocr_imgproc_threshold_expansion(result, 2);
	/* Расширяем обратно изображение. */
	ocr_imgproc_threshold_expansion(result, 2);
	ocr_imgproc_threshold_expansion(result, 2);
	return result;
}

uchar get_line_stat(ocr_text_area *text_area, int rho, int phi)
{
	int x = 0, y = 0;
	int width = text_area->width; 
	int height = text_area->height; 
	int rad = (double)(phi * M_PI) / 180;	/* Переводим в радианы. */
	int pos = 0;				/* Положение прямой в особых случаях.*/
	int points_count = 0;			/* Число точек в линии. */
	int black_count = 0;			/* Число черных точек в линии. */
	double sin_phi = sin(rad);	/* Запоминаем синус угла */
	double cos_phi = cos(rad);	/* и косинус для удобства. */
	double est = 0.05;		/* Оценка при подстановки точки в уравнение. */
	uchar result = 0;		/* Результат. */
	uchar *pix = text_area->pix;	/* Запоминаем указатель на пиксели для краткости. */
	/* Если прямая горизонтальная. */
	if (cos_phi == 0) {
		/* Находим положение прямой. */
		pos = (int)((double)rho / sin_phi);
		for (x = 0; x < width; x++) {
			points_count++;
			/* Если пиксель черный. */
			if (pix[pos * width + x] == CR_BLACK)
				black_count++;
		}
		/* Считаем долю. */
		result = (uchar)((double)black_count / points_count);
		return result;
	}

	/* Если прямая вертикальная. */
	if (sin_phi == 0) {
		/* Находим положение прямой. */
		pos = (int)((double)rho / cos_phi);
		for (y = 0; y < height; y++) {
			points_count++;
			/* Если пиксель черный. */
			if (pix[y * width + pos] == CR_BLACK)
				black_count++;
		}	
		result = (uchar)((double)black_count / points_count);
		/* Считаем долю. */
		return result;
	}
	/* Обходим все точки текстовой области для поиска точек,
	попавшив на прямую. */
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			/* Если точка лежит на прямой. */
			if (abs(x * cos_phi + y * sin_phi - rho) < est) {
				points_count++;
				/* Если пиксель черный. */
				if (pix[y * width + x] == CR_BLACK)
					black_count++;
			}
		}
	}
	/* Выводим результат. */
	if (points_count == 0) {
		result = 0;
	} else {
		result = (uchar)((double)black_count / points_count);
	}
	return result;
}

ocr_img_info *ocr_segm_get_hough_image(ocr_text_area *text_area, int delta_rho, int delta_phi, int start_phi, int end_phi)
{
	/* Будем работать в параметрическом пространстве pho и phi, пространство
	длин и углов соотв-но, возвращаем 2-мерное изображение, в котором каждый
	пиксель - доля черных пикселей в соотв-ей прямой во входном пространстве,
	нормированная от 0 до 255. */
	int i = 0, j = 0;
	int r = 0, p = 0;
	int width = text_area->width;	/* Запоминаем для удобства ширину,*/
	int height = text_area->height;	/* высоту. */
	int start_rho = 0;		/* Начальное значение ро зададим равным нулю. */
	int end_rho = 0;		/* Последнее значение ро. */
	int rho_width = 0;		/* Высота параметрического пространства. */
	int phi_width = 0;		/* Ширина параметрического прос-ва. */
	int rho_stride = 0;		/* Размер строки изображения Хафа. */
	int phi = 0;			/* и углов. */
	int curr = 0;			/* Индекс текщего элемента. */
	double eps = 0.05;		/* Оценка точности при проверке попадания на линию. */
	double *sin_phi;
	double *cos_phi;
	int *tmp = NULL;
	int *rho_table = NULL;
	uchar *pix = text_area->pix;

	/* Проверяем корректность входных данных. */
	if (delta_rho <= 0 || delta_phi <= 0 || start_phi < 0 || start_phi > 360 ||
	end_phi < 0 || end_phi > 360) {
		printf("Ошибка входных данных для получения изображения Хафа.\n");
		return NULL;
	}
	/* Выходное изображение Хафа. */
	ocr_img_info *hough_img = NULL;
	/* Зададим конечное расстояние. */
	end_rho = height;//(int)sqrt(width * width + height * height);
	/* Определяем размерность параметрического прос-ва. */
	rho_width = (end_rho - start_rho) / delta_rho;
	if (rho_width <= 0) {
		printf("Длина не рассматривается.\n");
		return NULL;
	}
	//phi_width = (end_phi - start_phi) / delta_phi;
	/* Определяем размер строки. */
	if (rho_width % WORD_SIZE == 0)
		rho_stride = rho_width;
	else
		rho_stride = rho_width + WORD_SIZE - rho_width % WORD_SIZE;
	/* Инициализируем выходное изображение. */
	hough_img = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	if (hough_img == NULL) {
		printf("Не удалось выделить память для изображения Хафа.\n");
		return NULL;
	}

	tmp = (int *)malloc(sizeof(int) * rho_stride * phi_width);
	if (tmp == NULL) {
		printf("Не удалось выделить память для пикселей изображения Хафа.\n");
		return NULL;
	}
	tmp = (int *)memset(tmp, 0, sizeof(int) * rho_stride * phi_width);
	/* Выделяем память для таблиц косинусов и синусов. */
	sin_phi = (double *)malloc(sizeof(double) * phi_width);
	cos_phi = (double *)malloc(sizeof(double) * phi_width);
	if (sin_phi == NULL || cos_phi == NULL) {
		printf("Не удалось выделить память дял таблицы синусов/косинусов.\n");
		return NULL;
	}
	/* Заполняем таблицу синусови косинусов. */
	for (i = 0; i < phi_width; i++) {
		phi = p * delta_phi + start_phi;
		phi = (double)((phi) * M_PI) / 180;
		sin_phi[i] = sin(phi);
		cos_phi[i] = cos(phi);
	}
	/* Создаем и заполняем таблицу расстояний ро. */
	rho_table = (int *)malloc(sizeof(int) * rho_width);
	if (rho_table == NULL) {
		printf("Не удалось выделить память дял таблицы синусов/косинусов.\n");
		return NULL;
	}
	for (i = 0; i < rho_width; i++) {
		rho_table[i] = i * delta_rho;
	}

	/* Заполняем параметрическое пространство. */
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			/* Если входной пиксель белый -  не рассматриваем. */
			if (pix[i] == CR_WHITE)
				continue;
			for (r = 0; r < end_rho; r++) {			/* Перебираем длины. */
				for (p = 0; p < phi_width; p++) {	/* Перебираем углы. */
					/* Переводим в радивны. */
					if (abs(j * cos_phi[p] + i * sin_phi[p] - rho_table[r]) < eps)
						tmp[p * rho_stride + r]++;
				}
			}
		}
	}
	/* Инициализируем массив пикселей. */
	hough_img->pix = (uchar *)malloc(sizeof(uchar) * rho_stride * phi_width);
	if (hough_img->pix == NULL) {
		printf("Не удалось выделить память для пикселей изображения Хафа.\n");
		return NULL;
	}
	
	for (p = 0; p < phi_width; p++) {	/* Перебираем углы. */
		for (r = 0; r < end_rho; r++) {	/* Перебираем длины. */
			curr = p * rho_stride + r;
			/* Нормируем по максимальному значению байта. */
			hough_img->pix[curr] = (uchar)((double)tmp[curr] / end_rho * 255);
		}
	}
	/* заполняем информацию об изображении. */
	hough_img->width = phi_width;
	hough_img->height = rho_width;
	hough_img->stride = rho_stride;
	hough_img->bytes_for_pix = 1;
	free(tmp);
	free(sin_phi);
	free(cos_phi);
	return hough_img;
}
