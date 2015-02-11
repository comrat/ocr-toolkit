#ifndef __OCR_IMGPROC__
#define __OCR_IMGPROC__

#ifndef USE_META
	#define USE_META
	#include "ocr_meta.h"
#endif /* USE_META */

/* Тип логической операции для наложения масок. */
typedef enum
{
	XOR,	/* иключающее 'ИЛИ' */
	OR,	/* 'ИЛИ' */
	AND,	/* 'И' */
	NXOR,	/* 'НЕ' исключающее 'ИЛИ' */
	NOR,	/* 'НЕ ИЛИ' */
	NAND,	/* 'НЕ И' */
	EXCLUDE	/* исключение */
} bool_op_type;


/**********************************************
* Морфологическая обработка и 
* операции с множествами. 
***********************************************/
/** 3
 * ocr_imgproc_morph_dilate - морфологическое "утолщение"
 * (см. Гонсалес. Вудс "Цифровая обработка 
 * изображений", М. - 2005) \fIimg\fP входного 
 * бинаризованного изображения по примитиву
 * \fIstruct_elem\fP с центром в \fIorigin\fP. Входное 
 * изобаржение заменяется на обработанное, в случае ошибки
 * изображение остается не измененным.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_dilate(ocr_img_info *img, ocr_img_info *struct_elem, int origin);
/** 3
 * ocr_imgproc_morph_errosion - морфологическое "утоньшение"
 * (см. Гонсалес. Вудс "Цифровая обработка  изображений",
 * М. - 2005) \fIimg\fP входного бинаризованного  изображения
 * по примитиву \fIstruct_elem\fP с центром в \fIorigin\fP.
 * Входное изобаржение заменяется на обработанное, в случае 
 * ошибки изображение остается не измененным.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_errosion(ocr_img_info *img, ocr_img_info *struct_elem, int origin);
/** 3
 * ocr_imgproc_morph_open - морфологическое размыкание
 * (см. Гонсалес. Вудс "Цифровая обработка  изображений",
 * М. - 2005) \fIimg\fP входного бинаризованного  изображения
 * по примитиву \fIstruct_elem\fP с центром в \fIorigin\fP.
 * Входное изобаржение заменяется на обработанное, в случае 
 * ошибки изображение остается не измененным.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_open(ocr_img_info *img, ocr_img_info *struct_elem, int origin);
/** 3
 * ocr_imgproc_morph_close - морфологическое зaмыкание
 * (см. Гонсалес. Вудс "Цифровая обработка  изображений",
 * М. - 2005) \fIimg\fP входного бинаризованного  изображения
 * по примитиву \fIstruct_elem\fP с центром в \fIorigin\fP.
 * Входное изобаржение заменяется на обработанное, в случае 
 * ошибки изображение остается не измененным.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_close(ocr_img_info *img, ocr_img_info *struct_elem, int origin);
/** 3
 * ocr_imgproc_invert - инверсия бинаризованного 
 * изображения \fIimg\fP, все черные пиксели становятся
 * белыми, иначе черными.
 * Входное изобаржение заменяется на обработанное, в случае 
 * ошибки изображение остается не измененным.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_invert(ocr_img_info *img);
/**3
 * ocr_imgproc_get_se_nxn - функция возвращает структурирующий
 * элемент - квадрат со сторонами \fIn\fP, заполненный 
 * единицами.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает укзатель на струкуру \fIocr_img_info\fP
 * с информацией о созданном структурирующем элементе или
 * \fINULL\fP в случае ошибки.
 */
ocr_img_info *ocr_imgproc_get_se_nxn(int n);
/******************************************** 
* Морфологические операции и операции над 
* множетсвами для текстовых областей. 
********************************************/
/** 3
 * ocr_imgproc_invert_text - инверсия текстовой 
 * области \fItext\fP, все черные пиксели становятся
 * белыми, иначе черными.
 * Входная текстовая область заменяется на обработанное, 
 * в случае ошибки область остается не измененной.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_invert_text(ocr_text_area *text);
/** 3
 * ocr_imgproc_intersect_text - пересечение текстовой 
 * области \fIarea1\fP с областью \fIarea2\fP.
 * Входная текстовая область заменяется на обработанное, 
 * в случае ошибки область остается не измененной.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_intersect_text(ocr_text_area *area1, ocr_text_area *area2);
/** 3
 * ocr_imgproc_combine_text - объединение текстовых
 * областей \fIarea1\fP и области \fIarea2\fP.
 * Входная текстовая область заменяется на обработанное, 
 * в случае ошибки область остается не измененной.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_combine_text(ocr_text_area *area1, ocr_text_area *area2);
/** 3
 * ocr_imgproc_morph_errosion - морфологическое операция
 * "Успех/Неудача" (см. Гонсалес. Вудс "Цифровая обработка 
 * изображений", М. - 2005) \fIimg\fP входной текстовой области
 * по примитиву \fIstruct_elem\fP с центром в \fIorigin\fP.
 * Входная область заменяется на обработанное, в случае 
 * ошибки область остается не измененной.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_hit_and_miss_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);
/** 3
 * ocr_imgproc_morph_errosion - морфологическое "утоньшение"
 * (см. Гонсалес. Вудс "Цифровая обработка  изображений",
 * М. - 2005) \fIimg\fP входной текстовой области
 * по примитиву \fIstruct_elem\fP с центром в \fIorigin\fP.
 * Входная область заменяется на обработанное, в случае 
 * ошибки область остается не измененной.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_think_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);
/** 3
 * ocr_imgproc_morph_dilate_text - морфологическое "утолщение"
 * (см. Гонсалес. Вудс "Цифровая обработка  изображений",
 * М. - 2005) \fIimg\fP входной текстовой области
 * по примитиву \fIstruct_elem\fP с центром в \fIorigin\fP.
 * Входная область заменяется на обработанное, в случае 
 * ошибки область остается не измененной.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_dilate_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);
/** 3
 * ocr_imgproc_morph_errosion_text - морфологическое "утоньшение"
 * (см. Гонсалес. Вудс "Цифровая обработка  изображений",
 * М. - 2005) \fIimg\fP входной текстовой области.
 * по примитиву \fIstruct_elem\fP с центром в \fIorigin\fP.
 * Входная текстовая область заменяется на обработанное, в 
 * случае ошибки область остается не измененной.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_errosion_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);
/** 3
 * ocr_imgproc_exclude_text - исключение текстовой
 * области \fIexclude\fP из области \fIsource\fP.
 * Входная текстовая область заменяется на обработанное, 
 * в случае ошибки область остается не измененной.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_exclude_text(ocr_text_area *source, ocr_text_area *exclude);
/*********************************************
* Функции обработки изображений для сегментации 
* Блумберга.
***********************************************/
/** 3
 * ocr_imgproc_threshold_reduction - понижающий порог
 * изображения \fIimg\fP, где каждая квадратная область
 * со стороной \fIcell_size\fP заменяется одним черным
 * пекселем, если сумма пикселей квадрата меньше порога
 * \fIthreshold\fP, иначе белым пикселем.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_threshold_reduction(ocr_img_info *img, int cell_size, int threshold);
/** 3
 * ocr_imgproc_threshold_expansion - расширение
 * изображения \fIimg\fP, где каждый пиксель 
 * заменяется квадратной областью со стороной 
 * \fIcell_size\fP.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_threshold_expansion(ocr_img_info *img, int cell_size);
/**********************************************
* Фильтры.
***********************************************/
/** 3
 * ocr_imgproc_filter_sobel - фильтр Собеля для
 * изображения \fIimg\fP. Выделяет границы.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_filter_sobel(ocr_img_info *img);
/** 3
 * ocr_imgproc_filter_gauss - фильтр Гаусса для
 * изображения \fIimg\fP. Размывает изображение.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_filter_gauss(ocr_img_info *img);
/** 3
 * ocr_imgproc_filter_median - медианный фильтр для
 * изображения \fIimg\fP. Сглаживает точечные шумы.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_filter_median(ocr_img_info *img, int size);
/**********************************************
* Пороги Otsu.
**********************************************/
/** 3
 * ocr_imgproc_global_otsu - глобальное пороговое преобразование
 * по методу Otsu.
 * Для изображения \fIimg\fp применяется порог по методу
 * Otsu, который состоит в разбиении точек области
 * на два класса (текстовые, фоновые) и отделить их друг от 
 * друга порогом, максимизирующим межклассовую дисперсию.
 * Бинаризованное изображение записывается в \fIbin\fP.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает 0, если была завершена корректно, иначе
 * любое отличное от нуля значение.
 */
int ocr_imgproc_global_otsu(ocr_img_info *img, ocr_img_info *bin);
/** 3
 * ocr_imgproc_adapt_otsu - адаптивное пороговое преобразование
 * по методу Otsu для каждой квадратной области.
 * Входное серое изображение \fIimg\fP по ширине разбиваеися на 
 * \fIdivisions\fP. Высота квадратов равна полученной ширине.
 * Для каждой квадратной области применяется порог по методу
 * Otsu, который состоит в разбиении точек области
 * на два класса (текстовые, фоновые) и отделить их друг от 
 * друга порогом, максимизирующим межклассовую дисперсию.
 * Бинаризованное изображение записывается в \fIbin\fP.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает 0, если была завершена корректно, иначе
 * любое отличное от нуля значение.
 */
int ocr_imgproc_adapt_otsu(ocr_img_info *img, ocr_img_info *bin, int divisions);
/** 3
 * ocr_imgproc_vert_otsu - адаптивное пороговое преобразование
 * по методу Otsu для каждого столбца изображения.
 * Входное серое изображение \fIimg\fP по ширине разбиваеися на 
 * \fIdivisions\fP столбцов.
 * Для каждого столбца применяется порог по методу Otsu, 
 * который состоит в разбиении точек области на два класса 
 * (текстовые, фоновые) и отделить их друг от  друга порогом, 
 * максимизирующим межклассовую дисперсию.
 * Бинаризованное изображение записывается в \fIbin\fP.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает 0, если была завершена корректно, иначе
 * любое отличное от нуля значение.
 */
int ocr_imgproc_vert_otsu(ocr_img_info *img, ocr_img_info *bin, int divisions);

/**********************************************
* Прочее.
***********************************************/
/**3
 * ocr_imgproc_color2grey - функция преобразует
 * входное цветное изображение \fIimg\fP в серое
 * изображение \fIgrey\fP.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает 0, если была завершена корректно, иначе
 * любое отличное от нуля значение.
 */
int ocr_imgproc_color2grey(ocr_img_info *img, ocr_img_info *grey);
/**3
 * ocr_img2textarea - функция конвертирует
 * входное изображение \fIimg\fP в текстовую
 * область \fItext_area\fP.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает 0, если была завершена корректно, иначе
 * любое отличное от нуля значение.
 */
int ocr_img2textarea(ocr_img_info *img, ocr_text_area *txt_area);

#endif /*__OCR_IMGPROC__*/
