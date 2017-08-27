#ifndef __OCR_IMGPROC__
#define __OCR_IMGPROC__

#include "ocr_meta.h"

/* Treshold macro */
#define THRESHOLD(x, trshld) ((x) <= (trshld)) ? CR_WHITE : CR_BLACK;

/** 3
 * ocr_preproc_color2grey - convert colored
 * image \fIimg\fP to grayscale
 *
 * RETURN VALUE
 * Pointer to grayscale image.
 */
ocr_img_info *ocr_preproc_color2grey(ocr_img_info *img);


/** 3
 * ocr_preproc_dilate - morphological dilation
 * (see https://en.wikipedia.org/wiki/Dilation_(morphology))
 * Pointer to binarized image \fIimg\fP
 *
 * RETURN VALUE
 * Pointer to dilated image.
 */
void ocr_preproc_dilate(ocr_img_info *img);


/** 3
 * ocr_preproc_errosion - morphological errosion
 * (see https://en.wikipedia.org/wiki/Errosion_(morphology))
 * Pointer to binarized image \fIimg\fP
 *
 * RETURN VALUE
 * Pointer to the image after errosion.
 */
void ocr_preproc_errosion(ocr_img_info *img);


/** 3
 * ocr_preproc_sobel - applies sobel filter
 * for input grayscale image \fIimg\fP.
 * This filter dectecs borders on the image
 *
 * RETURN VALUE
 * This function returns no value.
 */
void ocr_preproc_filter_sobel(ocr_img_info *img);


/** 3
 * ocr_preproc_filter_gauss - applies gauss filter
 * for input image \fIimg\fP.
 * This filter blur the image
 *
 * RETURN VALUE
 * This function returns no value.
 */
void ocr_preproc_filter_gauss(ocr_img_info *img);



/** 3
 * ocr_preproc_filter_median - Median filter
 * for input image \fIimg\fP.
 * This filter applies mean value of block with
 * \fIsize\fP size
 *
 * RETURN VALUE
 * This function returns no value.
 */
void ocr_preproc_filter_median(ocr_img_info *img, int size);


/** 3
 * ocr_preproc_threshold_otsu - adaptive Otsu threshold
 * method for input grayscale image \fIimg\fP
 * Input grayscale image width devide on \fIdivisions\fP
 * and the image devide on cells with corresponded width
 * and height. Otsu threshold applies for each cell respectevly.
 * Otsu method sort or grayscale points by frequency and
 * separate them on two classes - black and white and threshold
 * lies in beetween.
 *
 * RETURN VALUE
 * Function returns binarized image.
 */
ocr_img_info *ocr_preproc_threshold_otsu(ocr_img_info *img, int divisions);


/** 3
 * ocr_preproc_threshold_sauvolas - adaptive Sauvola threshold
 * method for input grayscale image \fImg_\fP
 * Серое изображение по ширине разбиваеися на \fIdivisions\fP.
 * Input grayscale image width devide on \fIdivisions\fP
 * and the image devide on cells with corresponded width
 * and height. Sauvola threshold applies for each cell respectevly.
 * Sauvola get the threshold via mathematical expectation.
 *
 * RETURN VALUE
 * Function returns binarized image.
 */
ocr_img_info *ocr_preproc_threshold_sauvolas(ocr_img_info *img, int divisions);

void ocr_preproc_invert(ocr_img_info *img);

#endif /*__OCR_IMGPROC__*/
