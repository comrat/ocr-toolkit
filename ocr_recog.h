#ifndef __OCR_RECOG__
#define __OCR_RECOG__

#ifndef USE_META
	#define USE_META
	#include "ocr_meta.h"
#endif /* USE_META */

#include <wchar.h>

/** 3
 * ocr_recog_get_char_hist - функция возвращает распознанный
 * символ с текстовой области символа \fIchar_area\fP,
 * распознанный методом гистограмм.
 * 
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает указатель на структуру с информацией
 * о распознанном символе.
 */
ocr_recog_char *ocr_recog_get_char_hist(ocr_text_area *char_area);
/** 3
 * ocr_recog_get_char_hist - функция возвращает распознанный
 * символ с текстовой области символа \fIchar_area\fP,
 * распознанный регионным методом.
 * 
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает указатель на структуру с информацией
 * о распознанном символе.
 */
ocr_recog_char *ocr_recog_get_char_region(ocr_text_area *char_area);
/** 3
 * ocr_recog_get_proj_hist - функция возвращает проективную 
 * для текстовой области \fItext\fP гистограмму, информацию
 * с гистограммами по вертикали и горизонтали.
 * 
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает указатель на структуру \fIproj_hist\fP
 * с информацией о проективной гистограмме.
 */
proj_hist *ocr_recog_get_proj_hist(ocr_text_area *text);
/** 3
 * ocr_recog_normalize_hist - функция нормализует проективную 
 * гистограмму \fIhist\fP с шириной \fIwidth\fP и высотой
 * \fIheight\fP.
 * 
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_recog_normalize_hist(proj_hist *hist, int width, int height);



int ocr_recog_get_region_stat(ocr_text_area *char_area, double *regions);

int ocr_recog_char_region(ocr_text_area *char_area, wchar_t *res_char);

int ocr_recog_char_hist(ocr_text_area *char_area, wchar_t *res_char);

#endif /*__OCR_RECOG__*/
