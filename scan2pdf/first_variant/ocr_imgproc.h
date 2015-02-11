#ifndef __OCR_IMGPROC__
#define __OCR_IMGPROC__

#ifndef USE_META
	#define USE_META
	#include "ocr_meta.h"
#endif /* USE_META */

ocr_img_info *ocr_imgproc_color2grey(ocr_img_info *img);

void ocr_imgproc_dilate(ocr_img_info *img);

void ocr_imgproc_errosion(ocr_img_info *img);

void ocr_imgproc_filter_sobel(ocr_img_info *img);

void ocr_imgproc_filter_gauss(ocr_img_info *img);

void ocr_imgproc_filter_median(ocr_img_info *img, int size);

ocr_img_info *ocr_imgproc_threshold_otsu(ocr_img_info *img, int divisions);

ocr_img_info *ocr_imgproc_threshold_sauvolas(ocr_img_info *img, int divisions);

void ocr_imgproc_invert(ocr_img_info *img);
#endif /*__OCR_IMGPROC__*/
