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
 * ocr_segm_lines_stat - function calculate part of black pixels
 * in each line of \fIcomponent\fp which was builded on net with
 * cell width \fIcell_width\fP pixels on image \fIimg\fP.
 *
 * RETURN VALUE
 * Array of float values with black pixel parts in each line.
 */
ocr_segm_stat_info *ocr_segm_lines_stat(ocr_img_info *img, ocr_con_comp *component, int cell_width);


/** 3
 * ocr_segm_get_net - this function classify the connected
 * component \fIcomp\fP according statistics properties.
 *
 * RETURN VALUE
 * This function returns no value.
 */
void ocr_segm_classify_comp(ocr_con_comp *comp, ocr_img_info *img, int comp_count, int cell_width);


/** 3
 * ocr_segm_get_comp_by_net - function returns
 * connected components from grayscale image
 * \fIimg\fP is devided on cells count
 * \fIcells_count\fP in each dimension
 * and calculate sigma for each one and compare it with
 * threshod \fIsigma_thrshld\fP.
 *
 * RETURN VALUE
 * Array of connected components.
 */
ocr_con_comp *ocr_segm_get_comp_by_net(ocr_img_info *grey, ocr_img_info *bin, int cells_count, double sigma_thrshld);


/** 3
 * ocr_segm_get_page_count - function returns connected
 * component count in image \fIimg\fP.
 *
 * RETURN VALUE
 * Connected component count.
 */
int ocr_segm_get_page_count(ocr_img_info *img);


/** 3
 * ocr_segm_rls_vert - function replace black pixels
 * in \fIimg\fP image with \fIshift\fP length vertical
 * line.
 *
 * RETURN VALUE
 * Processed image info or NULL if it fails.
 */
ocr_img_info *ocr_segm_rls_vert(ocr_img_info *img, int shift);


/** 3
 * ocr_segm_rls_horizont - function replace black pixels
 * in \fIimg\fP image with \fIshift\fP length horizontal
 * line.
 *
 * RETURN VALUE
 * Processed image info or NULL if it fails.
 */
ocr_img_info *ocr_segm_rls_horizont(ocr_img_info *img, int shift);


/** 3
 * ocr_segm_rlsa - RLSA (Run Length Smoothing Algorithm).
 * based method for text segmentation. Input arguments are
 * grayscale and binaried images \fIgrey\fP and \fIbin\fP.
 * respectively. Result areas laid on vertical and
 * horizontal blured lines intersection.
 *
 * RETURN VALUE
 * Array of result connected components or NULL if it fails.
 */
ocr_con_comp *ocr_segm_rlsa(ocr_img_info *grey, ocr_img_info *bin, int vert_shift, int horizont_shift);

#endif /* __OCR_SEGM__ */
