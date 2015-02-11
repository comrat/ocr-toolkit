#ifndef __OCR_RECOG__
#define __OCR_RECOG__

ocr_recog_char *ocr_recog_get_char_hist(ocr_text_area *char_area);

ocr_recog_char *ocr_recog_get_char_corner(ocr_text_area *char_area);

ocr_recog_char *ocr_recog_get_char_stat(ocr_text_area *char_area);

proj_hist *ocr_recog_get_proj_hist(ocr_text_area *text);

void ocr_recog_normalize_hist(proj_hist *hist, int width, int height);
#endif /*__OCR_RECOG__*/
