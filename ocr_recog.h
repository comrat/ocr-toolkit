#ifndef __OCR_RECOG__
#define __OCR_RECOG__

/*********** Статистика через скользящее окно. ******************/
ocr_text_area *ocr_recog_stat_lines_area(ocr_text_area *text_area, int *line_count);

ocr_text_area *ocr_recog_stat_words_area(ocr_text_area *line_area, int *word_count);

ocr_text_area *ocr_recog_stat_chars_area(ocr_text_area *word_area, int *char_count);

ocr_text_area *ocr_recog_char_prop(ocr_text_area *t_area);



//ocr_text_area *ocr_recog_hist_chars_area(ocr_text_area *word_area, int *char_count)



char ocr_recog_neural_network(ocr_text_area *input);

void ocr_recog_neural_network_learn(char c, ocr_text_area *input);

#endif /*__OCR_RECOG__*/
