#ifndef __OCR_IMGPROC__
#define __OCR_IMGPROC__

#ifndef USE_META
	#define USE_META
	#include "ocr_meta.h"
#endif /* USE_META */

/**********************************************
* Морфологическая обработка. 
***********************************************/
void ocr_imgproc_morph_dilate(ocr_img_info *img, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_errosion(ocr_img_info *img, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_open(ocr_img_info *img, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_invert(ocr_img_info *img);

ocr_img_info *ocr_imgproc_get_se_nxn(int n);

/* Методы для текстовых областей. */
void ocr_imgproc_morph_invert_text(ocr_text_area *text);

void ocr_imgproc_morph_intersect_text(ocr_text_area *area1, ocr_text_area *area2);

void ocr_imgproc_morph_combine_text(ocr_text_area *area1, ocr_text_area *area2);

void ocr_imgproc_morph_hit_and_miss_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_think_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_dilate_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_errosion_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);
/*********************************************
* Функции обработки изображений для сегментации 
* Блумберга.
***********************************************/
void ocr_imgproc_threshold_reduction(ocr_img_info *img, int cell_size, int threshold);

void ocr_imgproc_threshold_expansion(ocr_img_info *img, int cell_size);
/**********************************************
* Фильтры.
***********************************************/
void ocr_imgproc_filter_sobel(ocr_img_info *img);

void ocr_imgproc_filter_gauss(ocr_img_info *img);

void ocr_imgproc_filter_median(ocr_img_info *img, int size);
/**********************************************
* Пороги Otsu.
**********************************************/
ocr_img_info *ocr_imgproc_adapt_otsu(ocr_img_info *img, int divisions);

ocr_img_info *ocr_imgproc_vert_otsu(ocr_img_info *img, int divisions);

ocr_img_info *ocr_imgproc_global_otsu(ocr_img_info *img);
/**********************************************
* Прочее.
***********************************************/
void ocr_imgproc_invert(ocr_img_info *img);

ocr_img_info *ocr_imgproc_color2grey(ocr_img_info *img);

#endif /*__OCR_IMGPROC__*/
