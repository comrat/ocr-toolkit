#ifndef __OCR_SEGM__
#define __OCR_SEGM__

#ifndef USE_META
	#define USE_META
	#include "ocr_meta.h"
#endif /* USE_META */

/************************************************************
* Функции для определения текстовых областей строк, слов и
* символов. Для ровного текста (не инвариантино к поворту).
*************************************************************/
ocr_text_area **ocr_segm_stat_lines_area(ocr_text_area *text_area, int *line_count);

ocr_text_area **ocr_segm_stat_words_area(ocr_text_area *line_area, int *word_count);

ocr_text_area **ocr_segm_stat_chars_area(ocr_text_area *word_area, int *char_count);

void ocr_segm_stat_chars_recovery(ocr_img_info *img, ocr_text_area **chars, int char_count);

/************************************************************
* Макросы и методы для разбиения изображения на клетки с их
* последущей классификацией.
*************************************************************/
/* Типы клеток для классификации. */
#define CELL_BGROUND 	0	// клетка фона
#define CELL_TEXT 	1	// клетка текста
#define CELL_PIC 	2	// клетка рисунка
/* Клетки - начало и конец прямоугольной области. */
#define CELL_CRNR_BE 	3	// начачла прямоугльной области, верхний левый угол
#define CELL_CRNR_END	4	// конец прямоугольной области, нижний правый угол

ocr_cells_net *ocr_segm_and_class_net(ocr_img_info *img, int divisions);

ocr_text_area **ocr_segm_get_text_areas(ocr_img_info *img, int cells_count, int *text_areas_count);

ocr_img_info *ocr_segm_bloomberg(ocr_img_info *img);

ocr_img_info *ocr_segm_bloomberg_modify(ocr_img_info *img);

/************************************************************
* Функции для метода Хаффа.
*************************************************************/
ocr_img_info *ocr_segm_get_hough_image(ocr_text_area *text_area, int delta_rho, int delta_phi, int strart_phi, int end_phi);

/************************************************************
* Прочее.
*************************************************************/

int ocr_segm_get_page_count(ocr_img_info *img);

void ocr_segm_get_area(ocr_img_info *img, coord *be, coord *end);

double *ocr_segm_get_line_stat(ocr_text_area *text_area);

double *ocr_segm_get_coll_stat(ocr_text_area *text_area);

#endif /* __OCR_SEGM__ */
