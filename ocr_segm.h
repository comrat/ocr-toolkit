#ifndef __OCR_SEGM__
#define __OCR_SEGM__

#ifndef USE_META
	#define USE_META
	#include "ocr_meta.h"
#endif /* USE_META */

/** 3
 * ocr_segm_get_net - function returns net where each cell
 * is background or not.
 * \fIocr_img_info\fP - pointer to the grayscale image.
 * Input image is devided on \fIcells_count\fP in both
 * dimensions. iDispersion is callculated for each cell
 * in comparation with \fIsigma_thrs\fP threshod.
 *
 * RETURN VALUE
 * Function returns pointer to the grid where each
 * cell is background or not. If any error occured return NULL.
 */
ocr_cells_net *ocr_segm_get_net(ocr_img_info *grey, int cells_count, double sigma_thrs);


/** 3
 * ocr_con_comp - function returns connected
 * components count.
 * Image grid \fIcells\fP
 *
 * RETURN VALUE
 * Function returns connected components count,
 * if it fails returns NULL.
 */
ocr_con_comp *ocr_segm_get_component(ocr_cells_net *cells, int **labeled);


/** 3
 * ocr_segm_analyze_comp - calculate connected component \fIcomp\fP
 * params such as width or height.
 *
 * RETURN VALUE
 * This function returns no value.
 */
void ocr_segm_analyze_comp(ocr_con_comp *comp, int comp_count);


/** 3
 * ocr_segm_lines_stat - функция вычисляет долю черных пикселей
 * в каждой строке компоненты \fIcomponent\fp полученном на
 * сетке с шириной клетки \fIcell_width\fP на изображении \fIimg\fP.
 *
 * RETURN VALUE
 * Функция возвращает массив вещественных чисел, соответствующих
 * доле черных пикселей в строке компоненты \fIcomponent\fP.
 */
ocr_segm_stat_info *ocr_segm_lines_stat(ocr_img_info *img, ocr_con_comp *component, int cell_width);


/** 3
 * ocr_segm_get_net - функция классифицирует компоненты
 * связности \fIcomp\fP из \fIocr_comp_type\fP по статистическим
 * характеристикам, проецируя компоненты на серое изображение
 * \fIimg\fP.
 *
 * RETURN VALUE
 * This function returns no value.
 */
void ocr_segm_classify_comp(ocr_con_comp *comp, ocr_img_info *img, int comp_count, int cell_width);


/** 3
 * ocr_segm_get_comp_by_net - функция возвращаеи массив
 * компонент связности, полученных с серого изображения
 * \fIimg\fP методом разбиения изображения на сетку из
 * квадратных изображений, составляющих \fIpercent\fP
 * процентов от изображения. Каждая клетка проверяется,
 * является ли она фоном или нет оценивая через сигму
 * данной области сравнивая ее с \fIsigma_thrshld\fP.
 *
 * RETURN VALUE
 * Функция возвращает указатель на массив компонент
 * связности.
 */
ocr_con_comp *ocr_segm_get_comp_by_net(ocr_img_info *grey, ocr_img_info *bin, int cells_count, double sigma_thrshld);


/** 3
 * ocr_segm_get_comp_by_net - функция возвращаеи массив
 * компонент связности, полученных с серого изображения
 * \fIimg\fP методом разбиения изображения на сетку из
 * квадратных изображений, составляющих \fIpercent\fP
 * процентов от изображения. Каждая клетка проверяется,
 * является ли она фоном или нет оценивая через сигму
 * данной области сравнивая ее с \fIsigma_thrshld\fP.
 *
 * RETURN VALUE
 * Функция возвращает указатель на массив компонент
 * связности.
 */
int ocr_segm_get_page_count(ocr_img_info *img);


/** 3
 * ocr_segm_get_rls_vert - функция обрабатывает входное
 * бинаризованное изображение \fIimg\fP, "размазыванием"
 * всех черных пикселей по вертикали на \fIshift\fP
 * пикселей.
 *
 * RETURN VALUE
 * Функция возвращает указатель на структуру с информацией
 * об обработанном изображении или NULL в случае неудачи.
 */
ocr_img_info *ocr_segm_rls_vert(ocr_img_info *img, int shift);


/** 3
 * ocr_segm_get_rls_horizont - функция обрабатывает входное
 * бинаризованное изображение \fIimg\fP, "размазыванием"
 * всех черных пикселей по горизонтали на \fIshift\fP
 * пикселей.
 *
 * RETURN VALUE
 * Функция возвращает указатель на структуру с информацией
 * об обработанном изображении или NULL в случае неудачи.
 */
ocr_img_info *ocr_segm_rls_horizont(ocr_img_info *img, int shift);


/** 3
 * ocr_segm_rlsa - сегментация текста методом RLSA (Run
 * Length Smoothing Algorithm). На вход подается серое
 * изображение \fIgrey\fP и бинаризованное \fIbin\fP.
 * Области определяются по пересечению черных областей
 * "размытых" по вертикали и горизонтали бинаризованных
 * изображений на \fIvert_shift\fP \fIhorizont_shift\fP
 * соответственно. По серому изображению классифицируется
 * области по типам: текст, изображение, формула или шум.
 *
 * RETURN VALUE
 * Функция возвращает указатель на массив компонент
 * связности.
 */
ocr_con_comp *ocr_segm_rlsa(ocr_img_info *grey, ocr_img_info *bin, int vert_shift, int horizont_shift);

#endif /* __OCR_SEGM__ */
