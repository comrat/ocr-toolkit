#include "ocr_meta.h"
#include "ocr_recog.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


ocr_text_area *ocr_recog_get_word_area(ocr_text_area *t_area)
{
	return NULL;
}

ocr_text_area *ocr_recog_stat_chars_area(ocr_text_area *word_area, int *char_count)
{
	if(word_area->height <= 0)
		return NULL;

	int width = word_area->width;		// ширина текстовой области
	int height = word_area->height;		// высота текстовой области
	int i = 0, j = 0, k = 0, l = 0;		// индексы
	int shift = (int)(height * 0.05);	// будем считать длину разделителя за 1/20 высоты
	//int mid_shift = shift >> 1;	// середина шага
	int char_be = 0, char_end = 0;	// индексы начала и конца символа в слове
	int char_width = 0;		// ширина символа
	int start_ind = 0;		// индекс, откуда начинаются символы
	int char_height = 0;		// высота текущего символа
	int up_border = 0;		// индекс строки с которой начинается символ
	double mu = 0.0;		// доля черных пикселей в столбце
	double mu_height = 0.0;		// доля черных пикселей в строке
	double thrshld = 0.005;		// пороовое значение доли черных пикселей
	uchar state = 0;		// переменная указывает на текущее состояние 0 - елси символ,
					// 255 - если разделитель
	uchar **pix = word_area->pix;	// 2-мерный массив пикселей текстовой области
	ocr_text_area *chars = NULL;	// рещультирующий массив текстовых областей символов
	ocr_text_area add_area;		// добавляемая текстовая область

	chars = NULL;
	(*char_count) = 0;

	/* Находим начало инжекс, с которого начинаются символы. */
	for(i = 0; i < width; i++){
		mu = 0.0;
		for(j = 0; j < height; j++){
			mu += pix[j][i];
		}
		mu /= CR_BLACK * height;
		if(mu > thrshld){
			start_ind = i;
			break;
		}
	}
	char_be = start_ind;	// запоминаем индекс
	for(i = start_ind; i < width - shift; i++){
		/* Вычисляем долю черных пикселей в столбце пикселей. */
		mu = 0.0;
		for(j = 0; j < height; j++){
			for(k = 0; k < shift; k++){
				mu += pix[j][i + k];
			}
		}
		mu /= CR_BLACK * height * shift;

		if((mu < thrshld && state == 0) || i == width - shift - 1){	// если переходим в режим разделителя после символа
			state = 255;		// меняем на режим разделителя
			char_end = i;//(i + mid_shift < width) ? i + mid_shift: width - 1;	// индекс конца символа
			char_width = char_end - char_be;	// определяем ширину символа
			/* Если ширина положительна, то создадим новый символ. */
			if(char_width > 0){
				add_area.x = char_be;
				add_area.y = word_area->y;
				add_area.width = char_width;
				char_height = height;
				up_border = 0;
				/* Определеям границы символа сверху. */
				for(k = 0; k < height; k++){
					mu_height = 0;
					for(l = char_be; l < char_end; l++){
						mu_height += pix[k][l];
					}
					mu_height /= CR_BLACK * char_width;
					if(mu_height < thrshld){
						char_height--;
						up_border++;
					}else{
						break;
					}
				}
				/**/
				for(k = height - 1; k >= 0; k--){
					mu_height = 0;
					for(l = char_be; l < char_end; l++){
						mu_height += pix[k][l];
					}
					mu_height /= CR_BLACK * char_width;
					if(mu_height < thrshld){
						char_height--;
					}else{
						break;
					}
				}
				add_area.height = char_height;
				/* Копируем область символа из области слова. */
				add_area.pix = (uchar **)malloc(sizeof(uchar *) * char_height);
				for(k = 0; k < char_height; k++){
					add_area.pix[k] = (uchar *)malloc(sizeof(uchar) * char_width);
					for(l = 0; l < char_width; l++){
						add_area.pix[k][l] = pix[k + up_border][l + char_be];
					}
				}
				/* Добавление новой текстовой в массив области. */
				(*char_count)++;
				chars = (ocr_text_area *)realloc(chars, sizeof(ocr_text_area) * (*char_count));
				chars[*char_count - 1] = add_area;
				char_be = width - 1;	// сбрасываем индекс начала символа
			}
		}else if(mu > thrshld && state == 255){
			state = 0;			// меняем на режим символа
			//char_be = i - mid_shift;	// запоминаем индекс начала символа
			char_be = i + shift;
		}
	}
	return chars;
}

ocr_text_area *ocr_recog_stat_words_area(ocr_text_area *line_area, int *word_count)
{
	if(line_area->height <= 0)
		return NULL;

	int width = line_area->width;		// ширина текстовой области
	int height = line_area->height;		// высота текстовой области
	int i = 0, j = 0, k = 0, l = 0;		// индексы
	int shift = (int)(height * 0.18);	// будем считать длину разделителя за четверть высоты
	int mid_shift = shift >> 1;	// середина шага
	int word_be = 0, word_end = 0;	// индексы начала и конца слова в строке
	int word_width = 0;		// ширина слова
	int start_ind = 0;		// индекс, откуда начинаются слова
	double mu = 0.0;		// доля черных пикселей в столбце
	double thrshld = 0.0005;		// пороовое значение доли черных пикселей
	uchar state = 0;		// переменная указывает на текущее состояние 0 - елси слово,
					// 255 - если разделитель.
	uchar **pix = line_area->pix;	// 2-мерный массив пикселей текстовой области
	ocr_text_area *words = NULL;	// рещультирующий массив текстовых областей слов
	ocr_text_area add_area;		// добавляемая текстовая область

	words = NULL;
	(*word_count) = 0;
	/* Находим начало инжекс, с которого начинаются символы. */
	for(i = 0; i < width; i++){
		mu = 0.0;
		for(j = 0; j < height; j++){
			mu += pix[j][i];
		}
		mu /= CR_BLACK * height;
		if(mu > thrshld){
			start_ind = i;
			break;
		}
	}
	word_be = start_ind;	// запоминаем индекс
	for(i = start_ind; i < width - shift; i++){
		/* Вычисляем долю черных пикселей в столбце пикселей. */
		mu = 0.0;
		for(j = 0; j < height; j++){
			for(k = 0; k < shift; k++){
				mu += pix[j][i + k];
			}
		}
		mu /= CR_BLACK * height * shift;
		if((mu < thrshld && state == 0) || i == width - shift - 1){	// если переходим в режим разделителя после символа
			state = 255;		// меняем на режим разделителя
			word_end = (i + mid_shift < width) ? i + mid_shift: width - 1;	// индекс конца символа
			word_width = word_end - word_be;	// определяем ширину символа
			/* Если ширина положительна, то создадим новый символ. */
			if(word_width > 0){
				add_area.x = word_be;
				add_area.y = line_area->y;
				add_area.width = word_width;
				add_area.height = height;
				/* Копируем область символа из области слова.. */
				add_area.pix = (uchar **)malloc(sizeof(uchar *) * height);
				for(k = 0; k < height; k++){
					add_area.pix[k] = (uchar *)malloc(sizeof(uchar) * word_width);
					for(l = 0; l < word_width; l++){
						add_area.pix[k][l] = pix[k][l + word_be];
					}
				}
				/* Добавление новой текстовой в массив области. */
				(*word_count)++;
				words = (ocr_text_area *)realloc(words, sizeof(ocr_text_area) * (*word_count));
				words[*word_count - 1] = add_area;
				word_be = width - 1;	// сбрасываем индекс начала символа
			}
		}else if(mu > thrshld && state == 255){
			state = 0;			// меняем на режим символа
			word_be = i - mid_shift;	// запоминаем индекс начала символа
		}
	}
	return words;
}

ocr_text_area *ocr_recog_stat_lines_area(ocr_text_area *text_area, int *line_count)
{
	if(text_area->width <= 0)
		return NULL;

	int width = text_area->width;		// ширина текстовой области
	int height = text_area->height;		// высота текстовой области
	int i = 0, j = 0, k = 0, l = 0;		// индексы
	int shift = 1;//(int)(height * 0.25);
	int mid_shift = shift >> 1;	// середина шага
	int line_be = 0, line_end = 0;	// индексы начала и конца слова в строке
	int line_height = 0;		// высота строки
	int start_ind = 0;		// индекс, откуда начинаются строки
	int h_curr = 0, h_next = 0;
	int h_new = 0;
	int line_size = sizeof(uchar) * width;
	double mu = 0.0;		// доля черных пикселей в рдной строке
	double thrshld = 0.0005;	// пороговое значение доли черных пикселей
	uchar state = 0;		// переменная указывает на текущее состояние 0 - елси строка,
					// 255 - если отступ.
	uchar **pix = text_area->pix;	// 2-мерный массив пикселей текстовой области
	uchar **tmp, **del;
	ocr_text_area *lines = NULL;	// рещультирующий массив текстовых областей слов
	ocr_text_area add_area;		// добавляемая текстовая область

	(*line_count) = 0;
	/* Находим начальный индекс, с которого начинаются строки. */
	for(i = 0; i < height; i++){
		mu = 0.0;
		for(j = 0; j < width; j++){
			mu += pix[i][j];
		}
		mu /= CR_BLACK * width;
		if(mu > thrshld){
			start_ind = i;
			break;
		}
	}
	line_be = start_ind;	// запоминаем индекс
	for(i = start_ind; i < height - shift; i++){
		/* Вычисляем долю черных пикселей в столбце пикселей. */
		mu = 0.0;
		for(j = 0; j < width; j++){
			for(k = 0; k < shift; k++){
				mu += pix[i + k][j];
			}
		}
		mu /= CR_BLACK * width * shift;
		if((mu < thrshld && state == 0) || i == height - shift - 1){	// если переходим в режим разделителя после символа
			state = 255;		// меняем на режим разделителя
			line_end = (i + mid_shift < height) ? i + mid_shift : height - 1;	// индекс конца символа
			line_height = line_end - line_be;	// определяем высоту строки
			/* Если ширина положительна, то создадим новую строку. */
			if(line_height > 0){
				add_area.y = line_be;
				add_area.x = text_area->x;
				add_area.height = line_height;
				add_area.width = width;
				/* Копируем область строки из текстовой области. */
				add_area.pix = (uchar **)malloc(sizeof(uchar *) * height);
				for(k = 0; k < line_height; k++){
					add_area.pix[k] = (uchar *)malloc(sizeof(uchar) * width);
					for(l = 0; l < width; l++){
						add_area.pix[k][l] = pix[k + line_be][l];
					}
				}
				/* Добавление новую текстовую область в массив области. */
				(*line_count)++;
				lines = (ocr_text_area *)realloc(lines, sizeof(ocr_text_area) * (*line_count));
				/* Добавляем новый элемент в массив. */
				lines[*line_count - 1] = add_area;
				line_be = height - 1;	// сбрасываем индекс начала символа
			}
		}else if(mu > thrshld && state == 255){
			state = 0;			// меняем на режим символа
			line_be = i - mid_shift;	// запоминаем индекс начала символа
		}
	}

	/* Проссмтриваем все строки на наличие ошибочно распознанных
	"межстрочных" строк. */
	for(i = 0; i < (*line_count) - 1; i++){
		h_curr = lines[i].height;
		h_next = lines[i + 1].height;
		if((double)h_next / h_curr >= 3){
			h_new = h_curr + h_next;
			/* Объединяем 2 строки в одну. */
			lines[i + 1].y = lines[i].y;
			lines[i + 1].height = h_new;
			/* Выжедяем новый 2-мерный массив для пикселей области. */
			tmp = (uchar **)malloc(sizeof(uchar *) * h_new);
			for(j = 0; j < h_new; j++)
				tmp[j] = (uchar *)malloc(line_size);
			/* Копируем строки из текущей области. */
			for(j = 0; j < h_curr; j++){
				tmp[j] = (uchar *)memcpy(tmp[j], lines[i].pix[j], line_size);
			}
			/* Копируем строки из следущей области. */
			for(j = h_curr; j < h_new; j++){
				tmp[j] = (uchar *)memcpy(tmp[j], lines[i + 1].pix[j - h_curr], line_size);
			}
			/* Удаляем уже 2-мерные массивы из скопированнх областей. */
			del = lines[i + 1].pix;
			lines[i + 1].pix = tmp;
			free(del);
			free(lines[i].pix);
			for(j = i; j < *line_count - 1; j++){
				memcpy(&lines[j], &lines[j + 1], sizeof(ocr_text_area));
			}
			(*line_count)--;
			lines = (ocr_text_area *)realloc(lines, sizeof(ocr_text_area) * (*line_count));
		}
	}
	return lines;
}
/*****************************************************************************/

ocr_text_area *ocr_recog_hist_chars_area(ocr_text_area *word_area, int *char_count)
{
	int i = 0, j = 0;
	int width = word_area->width;
	int height = word_area->height;
	uchar **pix = word_area->pix;
	double *mu = (double *)malloc(sizeof(double) * width);
	double thrshld = 0.005;
	ocr_text_area *chars = NULL;

	(*char_count) = 0;
	for(i = 0; i < width; i++){
		mu[i] = 0.0;
		for(j = 0; j < height; j++){
			mu[i] += pix[j][i];
		}
		mu[i] /= CR_BLACK * height;
	}

	for(i = 0; i < width - 1; i++){
		if(mu[i + 1] - mu[i] > 0.2)
			printf("Be:%d\n", i);
		if(mu[i + 1] - mu[i] < -0.2)
			printf("End:%d\n", i);
	}

	return word_area;
}

char ocr_recog_get_simillar_char(double *zone)
{
	char result_char = 0;
	return result_char;
}

char ocr_recog_get_zone_char(ocr_text_area *char_area)
{
	char result = '.';
	int zone_size = 5;
	int i = 0, j = 0;
	int block_x = 0, block_y = 0;
	int width = char_area->width;
	int height = char_area->height;
	int block_width = 0;
	int block_height = 0;
	int zone_count = 0;
	double part = 0.0;
	uchar **pix = char_area->pix;
	/* Инициализируем 2-мерный массив зон. */
	double **zone = (double **)malloc(sizeof(double *) * zone_size);

	/* Определяем ширину блока. */
	if(width % zone_size == 0)
		block_width = (int)((double)width / zone_size);
	else
		block_width = (int)((double)(width + zone_size - width % zone_size) / zone_size);
	/* Определяем высоту блока. */
	if(height % zone_size == 0)
		block_height = (int)((double)height / zone_size);
	else
		block_height = (int)((double)(height + zone_size - height % zone_size) / zone_size);
	/* Определяем число пикселей вблоке. */
	zone_count = block_width * block_height;

	for(i = 0; i < zone_size; i++){
		zone[i] = (double *)malloc(sizeof(double) * zone_size);
		for(j = 0; j < zone_size; j++){
			zone[i][j] = 0.0;
		}
	}
	/* Вычисляем число черных пикселей в области. */
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			zone[i / block_height][j / block_width] += pix[i][j];
		}
	}
	/* Нормируем. */
	for(i = 0; i < zone_size; i++){
		for(j = 0; j < zone_size; j++){
			zone[i][j] /= zone_count * CR_BLACK;
			printf("Z:%.2f\n", zone[i][j]);
		}
	}
//printf("cHras %d\n", chars_zone_stat.zone_stat[0][0]);
}
