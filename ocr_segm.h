#ifndef __OCR_SEGM__
#define __OCR_SEGM__

#ifndef USE_META
	#define USE_META
	#include "ocr_meta.h"
#endif /* USE_META */

/* Метод сетки. */
/*ocr_cells_net *ocr_segm_get_net(ocr_img_info *img, double percent, double sigma_thrs);

ocr_con_comp *ocr_segm_get_component(ocr_cells_net *cells, int **labeled);

void ocr_segm_analyze_comp(ocr_con_comp *comp, int comp_count);

void ocr_segm_classify_comp(ocr_con_comp *comp, ocr_img_info *img, int comp_count, int cell_width);
*/
ocr_con_comp *ocr_segm_get_comp_by_net(ocr_img_info *grey, ocr_img_info *bin, int cells_count, double sigma_thrshld);

int ocr_segm_get_page_count(ocr_img_info *img);

/* Методы RLSA. */
ocr_img_info *ocr_segm_rls_vert(ocr_img_info *img, int shift);

ocr_img_info *ocr_segm_rls_horizont(ocr_img_info *img, int shift);

ocr_con_comp *ocr_segm_rlsa(ocr_img_info *grey, ocr_img_info *bin, int vert_shift, int horizont_shift);

#endif /* __OCR_SEGM__ */
