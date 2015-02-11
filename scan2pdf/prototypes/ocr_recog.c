#include "ocr_meta.h"
#include "ocr_recog.h"

ocr_text_area *ocr_recog_get_word_area(ocr_text_area *t_area);

ocr_text_area *ocr_recog_get_char_area(ocr_text_area *t_area);

ocr_text_area *ocr_recog_get_char_prop(ocr_text_area *t_area);

char ocr_recog_neural_network(ocr_text_area *input);

void ocr_recog_neural_network_learn(char c, ocr_text_area *input);

double *ocr_recog_get_lines_stat(ocr_img_info *img);

double *ocr_recog_get_coll_stat(ocr_text_area *img);
