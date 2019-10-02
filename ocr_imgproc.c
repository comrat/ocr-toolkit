#include "ocr_meta.h"
#include "ocr_imgproc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/** 3
 * ocr_imgproc_morph_dilate - морфологическое "утолщение"
 * (см. Гонсалес. Вудс "Цифровая обработка
 * изображений", М. - 2005) \fIimg\fP входного
 * бинаризованного изображения.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_dilate(ocr_img_info *img, ocr_img_info *struct_elem, int origin);
/** 3
 * ocr_imgproc_morph_errosion - морфологическое "утоньшение"
 * (см. Гонсалес. Вудс "Цифровая обработка  изображений",
 * М. - 2005) \fIimg\fP входного бинаризованного
 * изображения.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_morph_errosion(ocr_img_info *img, ocr_img_info *struct_elem, int origin);


void ocr_imgproc_morph_open(ocr_img_info *img, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_invert(ocr_img_info *img);




void ocr_imgproc_morph_invert_text(ocr_text_area *text);

void ocr_imgproc_morph_intersect_text(ocr_text_area *area1, ocr_text_area *area2);

void ocr_imgproc_morph_combine_text(ocr_text_area *area1, ocr_text_area *area2);

void ocr_imgproc_morph_exclude_text(ocr_text_area *source, ocr_text_area *exclude);

void ocr_imgproc_morph_hit_and_miss_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_think_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_dilate_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);

void ocr_imgproc_morph_errosion_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin);

ocr_img_info *ocr_imgproc_get_se_nxn(int n);


/** 3
 * ocr_imgproc_sobel - применение филтра Собеля
 * для входного входного серого изображения \fIimg\fP.
 * Данный фильтр выделяет границы на изображении.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_filter_sobel(ocr_img_info *img);
/** 3
 * ocr_imgproc_gauss - применение филтра Гаусса
 * входного серого изображения \fIimg\fP.
 * Данный фильтр "сглажывает" изображение.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_filter_gauss(ocr_img_info *img);

/** 3
 * медианный фильтр.
 */
void ocr_imgproc_filter_median(ocr_img_info *img, int size);
/** 3
 * ocr_imgproc_adapt_otsu - адаптивное пороговое преобразование
 * по методу Otsu для каждой квадратной области.
 * Входное серое изображение \fIimg\fP по ширине разбиваеися на
 * \fIdivisions\fP. Высота квадратов равна полученной ширине.
 * Для каждой квадратной области применяется порог по методу
 * Otsu, который состоит в разбиении точек области
 * на два класса (текстовые, фоновые) и отделить их друг от
 * друга порогом, максимизирующим межклассовую дисперсию.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает указатель на структуру с информацией о
 * преобразованном изобравжении или \fINULL\fP в случае ошибки в
 * работе функции.
 */
ocr_img_info *ocr_imgproc_adapt_otsu(ocr_img_info *img, int divisions);
/** 3
 * ocr_imgproc_vert_otsu - адаптивное пороговое преобразование
 * по методу Otsu для каждого столбца изображения.
 * Входное серое изображение \fIimg\fP по ширине разбиваеися на
 * \fIdivisions\fP столбцов.
 * Для каждого столбца применяется порог по методу Otsu,
 * который состоит в разбиении точек области на два класса
 * (текстовые, фоновые) и отделить их друг от  друга порогом,
 * максимизирующим межклассовую дисперсию.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает указатель на структуру с информацией о
 * преобразованном изобравжении или \fINULL\fP в случае ошибки в
 * работе функции.
 */
ocr_img_info *ocr_imgproc_vert_otsu(ocr_img_info *img, int divisions);
/** 3
 * ocr_imgproc_global_otsu - глобальное пороговое преобразование
 * по методу Otsu.
 * Для изображения \fIimg\fp применяется порог по методу
 * Otsu, который состоит в разбиении точек области
 * на два класса (текстовые, фоновые) и отделить их друг от
 * друга порогом, максимизирующим межклассовую дисперсию.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает указатель на структуру с информацией о
 * преобразованном изобравжении или \fINULL\fP в случае ошибки в
 * работе функции.
 */
ocr_img_info *ocr_imgproc_global_otsu(ocr_img_info *img);
/** 3
 * ocr_imgproc_color2grey - преобразование в серое
 * одноканальное изображение. Указатель на структуру
 * \fIimg\fP входного цветного изображения.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает указатель на структуру с информацией о
 * преобразованном изобравжении или \fINULL\fP в случае ошибки в
 * работе функции.
 */
ocr_img_info *ocr_imgproc_color2grey(ocr_img_info *img);
/** 3
 * ocr_imgproc_invert - функция делает инверсию цветов
 * входного бинаризованного или серого изображения \fIimg\fP.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция не возвращает значений.
 */
void ocr_imgproc_invert(ocr_img_info *img);
/** 3
 * ocr_imgproc_threshold_reduction - функция сжимает изображение
 * по порогу. Каждая квадратная область пикселей изображения
 * \fIimg\fP, размерами \fIcell_size\fPx\fIcell_size\fP заменяется
 * одним пикселем черного цвета, если число черных пикселей
 * квадрата превышает пороговое значение \fIthreshold\fP.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает указатель на структуру с информацией о
 * преобразованном изобравжении или \fINULL\fP в случае ошибки в
 * работе функции.
 */
void ocr_imgproc_threshold_reduction(ocr_img_info *img, int cell_size, int threshold);
/** 3
 * ocr_imgproc_threshold_expansion - функция расширяет входное изображение
 * \fIimg\fP, заменяя каждый пиксель на квадрат пикселей размерами
 * \fIcell_size\fPx\fIcell_size\fP.
 *
 * ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ
 * Функция возвращает указатель на структуру с информацией о
 * преобразованном изобравжении или \fINULL\fP в случае ошибки в
 * работе функции.
 */
void ocr_imgproc_threshold_expansion(ocr_img_info *img, int cell_size);


/* */
ocr_text_area *ocr_img2textarea(ocr_img_info *img);
/*************************** Реализация *************************/

/* Макрос для порогового преобразования. */
#define THRESHOLD(x, trshld) (x < trshld) ? CR_WHITE : CR_BLACK;

/* Функция преобразования цветного пикселя в серый. */
int rgb2grey(int r, int g, int b)
{
	return (int)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
}

ocr_img_info *ocr_imgproc_color2grey(ocr_img_info *img)
{
	/* Если входное изображение не цветное, выходим из функции.*/
	if (img->bytes_for_pix < 3)
		return NULL;
	int i = 0, j = 0;
	int stride = img->stride;
	int rowpix = img->bytes_for_pix;
	int width = img->width;
	int height = img->height;
	int curr = 0;
	/* Строка должна быть крата размеру слова.  */
	int out_stride = (width % WORD_SIZE == 0) ? width : width + (WORD_SIZE - width % WORD_SIZE);
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	uchar *pix = img->pix;
	uchar *out_img = (uchar *)malloc(sizeof(char) * height * out_stride);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			curr = i * stride + j * rowpix;
			out_img[i * out_stride + j] = (uchar)rgb2grey(pix[curr], pix[curr + 1], pix[curr + 2]);
		}
	}

	result->width = width;
	result->height = height;
	result->stride = out_stride;
	result->bytes_for_pix = 1;
	result->pix = out_img;
	return result;
}


void ocr_imgproc_morph_dilate(ocr_img_info *img, ocr_img_info *struct_elem, int origin)
{
	/* Если изображение не бинаризованное, прерываем. */
	if (img->bytes_for_pix != 0) {
		printf("Входное изображение для дилатации не бинаризовано.\n");
		return;
	}
	/* Проверяем исходную точку на выход за пределы границ образца. */
	if (struct_elem->width * struct_elem->height <= origin || origin < 0) {
		printf("Исходная точка образца для дилатации за границами образца.\n");
		return;
	}

	int i = 0, j = 0, curr_ind = 0;		// переменные индексов
	int x = 0, y = 0;			// координат ысмещения
	int stride = img->stride;		// запоминаем длину строки
	int width = img->width;			// ширину изображения
	int height = img->height;		// высоту изображения
	int size = height * stride;		// размер изображения
	int pix_shift = stride - width;		// оставшийся сдвиг до конца строки
	int se_width = struct_elem->width;	// ширину образца
	int se_height = struct_elem->height;	// высоту образца
	int se_size = se_height * se_width;	// размер образца
	int orig_x = 0;				// x-координата исходной точки
	int orig_y = 0;				// y-координата исходной точки
	int curr_h = 0;				// текущий индекс по высоте
	uchar *pix = img->pix;			// запоминаем указатель на изображение
	uchar *se_pix = struct_elem->pix;	// запоминаем указатель на образце
	uchar *out_img = (uchar *)malloc(sizeof(uchar) * size);

	if (out_img == NULL) {
		printf("Не удалось выделить память для выходного дилатированного изображения.\n");
		return;
	}
	/* Закрашиваем все пиксели белым цветом. */
	out_img = (uchar *)memset(out_img, CR_WHITE, size);
	if (out_img == NULL) {
		printf("Не удалось обнулить выходное дилатированное изображение.\n");
		return;
	}
	/* Определяем координаты исходной точки образца. */
	orig_x = origin % se_width;
	orig_y = origin / se_width;
	/* Просматриваем все пиксели. */
	for (i = 0; i < size; i++) {
		/* Если пиксель черный. */
		if (pix[i] == CR_BLACK) {
			curr_h = i / stride;	// определяем текущую индекс по высоте
			for (j = 0; j < se_size; j++) {
				/* Определяем координаты смещения. */
				x = orig_x - (j % se_width);
				y = orig_y - (j / se_width);
				curr_ind = i - x - y * stride;
				/* Если индекс не вышел за границы изображения и соотв.
				пиксель образца черная. */
				if (curr_ind % stride >= 0 && curr_ind % stride < width
				&& curr_h + y < height && se_pix[j] == CR_BLACK &&
				curr_ind >= 0 && curr_ind < size) {
					out_img[curr_ind] = CR_BLACK;	// закрашиывем выходной пиксель
				}
			}
		}
		/* Если дошли до конца строки. */
		if ((i % stride) == width - 1)
			i += pix_shift;	// пропускаем отступ до следущей строки
	}
	free(img->pix);
	img->pix = out_img;
}

void ocr_imgproc_morph_errosion(ocr_img_info *img, ocr_img_info *struct_elem, int origin)
{
	/* Если изображение не бинаризованное, прерываем. */
	if (img->bytes_for_pix != 0) {
		printf("Входное изображение для дилатации не бинаризовано.\n");
		return;
	}
	/* Проверяем исходную точку на выход за пределы границ образца. */
	if (struct_elem->width * struct_elem->height <= origin || origin < 0) {
		printf("Исходная точка образца для дилатации за границами образца.\n");
		return;
	}

	int i = 0, j = 0, curr_ind = 0;		// переменные индексов
	int x = 0, y = 0;			// координат ысмещения
	int stride = img->stride;		// запоминаем длину строки
	int width = img->width;			// ширину изображения
	int height = img->height;		// высоту изображения
	int size = height * stride;		// размер изображения
	int pix_shift = stride - width;		// оставшийся сдвиг до конца строки
	int se_width = struct_elem->width;	// ширину образца
	int se_height = struct_elem->height;	// высоту образца
	int se_size = se_height * se_width;	// размер образца
	int orig_x = 0;				// x-координата исходной точки
	int orig_y = 0;				// y-координата исходной точки
	int curr_h = 0;				// текущий индекс по высоте
	int curr_color = 0;			// переменная для хранения текущего цвета
	int black_count = 0;			// число черных пикселей в текущем образце
	uchar *pix = img->pix;			// запоминаем указатель на изображение
	uchar *se_pix = struct_elem->pix;	// запоминаем указатель на образце
	uchar *out_img = (uchar *)malloc(sizeof(uchar) * size);

	if (out_img == NULL) {
		printf("Не удалось выделить память для выходного дилатированного изображения.\n");
		return;
	}
	/* Закрашиваем все пиксели белым цветом. */
	out_img = (uchar *)memset(out_img, CR_WHITE, size);
	if (out_img == NULL) {
		printf("Не удалось обнулить выходное дилатированное изображение.\n");
		return;
	}
	/* Определяем координаты исходной точки образца. */
	orig_x = origin % se_width;
	orig_y = origin / se_width;

	for (i = 1; i < size; i++) {
		/* Определяем текущий индекс по высоте. */
		curr_h = i / stride;
		/* Сбрасываем счетчик и переменную для определения цвета. */
		curr_color = 0;
		black_count = 0;
		for (j = 1; j < se_size; j++) {
			if (se_pix[j] == CR_BLACK) {
				/* Определяем координаты смещения. */
				x = orig_x - (j % se_width);
				y = orig_y - (j / se_width);
				/* Определяем текущую координату во входном изобьражении. */
				curr_ind = i - x - y * stride;
				/* Если индекс не вышел за границы изображения. */
				if (curr_ind % stride >= 0 && curr_ind % stride < width
				&& curr_h + y < height) {
					black_count++;
					curr_color += pix[curr_ind];	// закрашиывем выходной пиксель
				}
			}
		}
		/* Если все пиксели совпавшие с черными пикселями образца черные. */
		if (curr_color / black_count == CR_BLACK)
			out_img[i] = CR_BLACK;	// присваиваем выходному пикселю черный цвет
		else
			out_img[i] = CR_WHITE;	// присваиваем выходному пикселю белый цвет

		/* Если дошли до конца строки. */
		if ((i % stride) == width - 1)
			i += pix_shift;	// пропускаем отступ до следущей строки
	}
	free(img->pix);
	img->pix = out_img;
}

void ocr_imgproc_morph_open(ocr_img_info *img, ocr_img_info *struct_elem, int origin)
{
	ocr_imgproc_morph_errosion(img, struct_elem, origin);
	ocr_imgproc_morph_dilate(img, struct_elem, origin);
}

ocr_img_info *ocr_imgproc_vert_otsu(ocr_img_info *img, int vert_divisions)
{
	/* Если входное изображение не серое (на пиксель приходится
	не 1 байт) возвращаем NULL. */
	if (img->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0, k = 0;
	int curr = 0;				// индекс текущего пикселя изображения
	int otsu_trshld = 0;			// значение порга для текущей области
	int width = img->width;			// запоминаем текущее значения ширины
	int height = img->height;		// высоты
	int stride = img->stride;		// ширину строки изображения
	int col_width = width / vert_divisions;	// ширина каждого обрабатываемого столбца
	int col_end = 0;			// переменная для хранения индекса конца
						// текущего конца
	uchar *out_img;				// результирующее изображение
	uchar *pix = img->pix;			// запоминаем указатель на входное изображение
	double sum = 0;				// переменная для хранения суммы всех значений
						// пикселей
	double otsu_mL = 0, otsu_mR = 0;	// мат. ожидание правого и левого класса
	double otsu_sum = 0, otsu_sum_left = 0;	// общая сумма и сумма слева элементов гистограммы
	double otsu_max = 0;			// максимальная межклассоаяй дисперсия
	double otsu_between = 0;		// межклассовая дисперсия
	double otsu_wL = 0, otsu_wR = 0;	// вес левого (ближе к 0) и правого (ближе к 255)
	double *hist = (double *)malloc(sizeof(double) * 256);	// гистограмма градации серого
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));	// результирующая информация об изображении

	/* Инициализируем выходное изображение. */
	out_img = (uchar *)malloc(sizeof(uchar) * height * stride);

	/* Просматриваем изображение по столбцам. */
	for (k = 0; k < vert_divisions; k++) {
		/* Определяем x-коорд. конца текущего столбца. */
		col_end = (k + 1) * col_width;
		/* Ширину последнего столбца продлим до конца строки. */
		if (k == vert_divisions - 1) {
			col_end = width;
		}
		/* Сбрасываем гистограмму. */
		for (i = 0; i < 256; i++) {
			hist[i] = 0;
		}
		/* Строим частотную гистограмму. */
		for (i = 0; i < height; i++) {
			for(j = k * col_width; j < col_end; j++){
				curr = i * stride + j;
				hist[pix[curr]]++;
			}
		}
		/* Считаем сумму всех значений. */
		sum = 0;
		otsu_sum = 0;
		for (i = 0; i < 256; i++) {
			sum += hist[i];
		}
		/* Нормализуем. */
		for (i = 0; i < 256; i++) {
			hist[i] /= sum;
			otsu_sum += hist[i] * (i + 1);
		}
		otsu_sum_left = 0;
		otsu_wL = 0;
		otsu_wR = 0;
		otsu_mL = 0;
		otsu_mR = 0;
		otsu_max = 0;
		otsu_trshld = CR_BLACK;
		/* Перебираем значения порога для поиска
		такого, чтобы максимизировать дисперсию. */
		for (i = 0; i < 256; i++) {
			otsu_wL += hist[i];	// вычисляем вес слева

			if (otsu_wL == 0)	// если вес нулевой идем дальше
				continue;

			if (otsu_wL > 1)	// выравниваем до 1, чтобы избежать
				otsu_wL = 1;	// значений выше. (TODO: костыль)

			otsu_wR = 1 - otsu_wL;	// вычисляем вес справа (1 - максимальное значение)

			if (otsu_wR == 0)
				break;

			/* Вычисляем межклассовую дисперсию. */
			otsu_sum_left += (i + 1) * hist[i];
			otsu_mL = otsu_sum_left / otsu_wL;
			otsu_mR = (otsu_sum - otsu_sum_left) / otsu_wR;
			otsu_between = otsu_wL * otsu_wR * (otsu_mL - otsu_mR) * (otsu_mL - otsu_mR);
			/* Запоминаем максимальный. */
			if (otsu_between >= otsu_max) {
				otsu_max = otsu_between;
				otsu_trshld = i - 1;
			}
		}
		/* Применяем полученный порог к текущему столбцу. */
		for (i = 0; i < height; i++) {
			for (j = k * col_width; j < col_end; j++) {
				curr = i * stride + j;
				out_img[curr] = THRESHOLD(pix[curr], otsu_trshld);
			//	printf("%d ", pix[curr]);
			}
			//printf("\n");
		}
	}

	result->width = width;
	result->height = height;
	result->stride = stride;
	result->bytes_for_pix = 0;	// '0' для бинаризованного изображения
	result->pix = out_img;		// ссылаемся на полученное изображение

	return result;
}

ocr_img_info *ocr_imgproc_global_otsu(ocr_img_info *img)
{
	/* Если входное изображение не серое (на пиксель приходится
	не 1 байт) возвращаем NULL. */
	if (img->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0;
	int curr = 0;
	int otsu_trshld = 0;			// значение порга для текущей области
	int width = img->width;			// запоминаем текущее значения ширины
	int height = img->height;		// высоты
	int stride = img->stride;		// ширину строки изображения

	uchar *out_img;				// результирующее изображение
	uchar *pix = img->pix;			// запоминаем указатель на входное изображение
	double sum = 0;				// сумма всех значений гистограммы
	double otsu_mL = 0, otsu_mR = 0;	// мат. ожидание правого и левого класса
	double otsu_sum = 0, otsu_sum_left = 0; // общая сумма и сумма слева элементов гистограммы
	double otsu_max = 0;			// максимальная межклассоаяй дисперсия
	double otsu_between = 0;		// межклассовая дисперсия
	double otsu_wL = 0, otsu_wR = 0;	// вес левого (ближе к 0) и правого (ближе к 255)
	double *hist = (double *)malloc(sizeof(double) * 256);	// гистограмма градации серого
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));	// результирующая информация об изображении

	/* Инициализируем выходное изображение. */
	out_img = (uchar *)malloc(sizeof(char) * height * stride);

	for (i = 0; i < 256; i++) {
		hist[i] = 0;
	}
	/* Строим частотную гистограмму. */
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			curr = i * stride + j;
			hist[pix[curr]]++;
		}
	}

	sum = 0;
	otsu_sum = 0;
	/* Считаем сумму всех значений. */
	for (i = 0; i < 256; i++) {
		sum += hist[i];
	}
	/* Нормализуем. */
	for (i = 0; i < 256; i++) {
		hist[i] /= sum;
		otsu_sum += hist[i] * (i + 1);
	}

	otsu_sum_left = 0;
	otsu_wL = 0;
	otsu_wR = 0;
	otsu_mL = 0;
	otsu_mR = 0;
	otsu_max = 0;
	otsu_trshld = 0;
	/* Перебираем значения порога для поиска
	такого, чтобы максимизировать дисперсию. */
	for (i = 0; i < 256; i++) {
		otsu_wL += hist[i];	// вычисляем вес слева

		if (otsu_wL == 0)	// если вес нулевой идем дальше
			continue;

		if (otsu_wL > 1)	// выравниваем до 1, чтобы избежать
			otsu_wL = 1;	// значений выше. (TODO: костыль)

		otsu_wR = 1 - otsu_wL;	// вычисляем вес справа (1 - максимальное значение)

		if (otsu_wR == 0)
			break;

		/* Вычисляем межклассовую дисперсию. */
		otsu_sum_left += (i + 1) * hist[i];
		otsu_mL = otsu_sum_left / otsu_wL;
		otsu_mR = (otsu_sum - otsu_sum_left) / otsu_wR;
		otsu_between = otsu_wL * otsu_wR * (otsu_mL - otsu_mR) * (otsu_mL - otsu_mR);

		/* Запоминаем максимальный. */
		if (otsu_between >= otsu_max) {
			otsu_max = otsu_between;
			otsu_trshld = i - 1;
		}
	}

	/* Применяем полученный порог к текущей области. */
	for (i = 0; i < height; i++) {
		for (j = 0 ; j < width; j++) {
			curr = i * stride + j;
			out_img[curr] = THRESHOLD(pix[curr], otsu_trshld);
		}
	}

	result->width = width;
	result->height = height;
	result->stride = stride;
	result->bytes_for_pix = 0;	// '0' для бинаризованного изображения
	result->pix = out_img;		// ссылаемся на полученное изображение

	return result;
}

ocr_img_info *ocr_imgproc_adapt_otsu(ocr_img_info *img, int divisions)
{
	/* Если входное изображение не серое (на пиксель приходится
	не 1 байт) возвращаем NULL. */
	if (img->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0;
	int x_block = 0, y_block = 0;		// переменные для областей
	int x = 0, y = 0;
	int curr_ind = 0, k = 0;
	int curr_x_size = 0, curr_y_size = 0;
	int otsu_trshld = 0;			// значение порга для текущей области
	int n_size = 0;				// размер каждой области в пикселях
	int pix_count = 1;			// число пикселей каждой области
	int width = img->width;			// запоминаем текущее значения ширины
	int height = img->height;		// высоты
	int stride = img->stride;		// ширину строки изображения
	int max_sigma = 14;			// пороговое значение сигмы, значение меньше
						// max_sigma считаются фоновомы
	int y_divisions = 0;			// число разбиений по оси y
	int y_rest = 0, x_rest = 0;
	uchar *out_img = NULL;			// результирующее изображение
	uchar *pix = img->pix;			// запоминаем указатель на входное изображение
	double sum = 0;
	double otsu_mL = 0, otsu_mR = 0;
	double otsu_sum = 0, otsu_sum_left = 0;
	double otsu_max = 0, otsu_between = 0;
	double mu = 0.0, sigma = 0.0;		// мат. ожидание и сигма
	double otsu_wL = 0, otsu_wR = 0;	// вес левого (ближе к 0) и правого (ближе к 255)
	double *hist = NULL;			// гистограмма градации серого
	ocr_img_info *result = NULL;		// результирующая информация об изображении

	n_size = width / divisions;		// получаем размер каждой области
	y_rest = height % n_size;		// вычисляем не влезшие пиксели по каждой оси
	x_rest = width % n_size;
	y_divisions = height / n_size;		// определяем число по оси у
	curr_x_size = curr_y_size = n_size;

	/* Инициализируем выходное изображение. */
	out_img = (uchar *)malloc(sizeof(uchar) * height * stride);
	if (out_img == NULL) {
		printf("Не удалось выделить память для бинаризованного изображения.\n");
		return NULL;
	}
	/* Инициализируем гистограмму градаций серого.  */
	hist = (double *)malloc(sizeof(double) * 256);
	if (hist == NULL) {
		printf("Не удалось выделить память для гистограммы градаций серого.\n");
		return NULL;
	}
	for (;;) {
		mu = 0; sigma = 0;
		pix_count = curr_x_size * curr_y_size;	// вычисляем число пикселей в текущем блоке
		/* Очищаем гистограмму. */
		for (k = 0; k < 256; k++) {
			hist[k] = 0;
		}
		for (i = 0; i < curr_y_size; i++) {
			y = y_block * n_size + i;		// вычисляем координату y пикселя в текущем блоке
			for (j = 0; j < curr_x_size; j++) {
				x = x_block * n_size + j;	// вычисляем координату x пикселя в текущем блоке
				curr_ind = y * stride + x;	// вычисляем индекс текущего пикселя
				hist[pix[curr_ind]]++;
				mu += pix[curr_ind];
				sigma += pix[curr_ind] * pix[curr_ind];
			}
		}
		/* Заранее посчитаем мат. ожидание и сигму. */
		mu /= pix_count;
		sigma /= pix_count;
		sigma = sqrt(sigma - mu * mu);

		sum = 0;
		otsu_sum = 0;
		/* Строим частотную гистограмму. */
		for (k = 0; k < 256; k++) {
			sum += hist[k];
		}
		/* Нормализуем. */
		for (k = 0; k < 256; k++) {
			hist[k] /= sum;
			otsu_sum += hist[k] * (k + 1);
		}

		otsu_sum_left = 0;
		otsu_wL = 0;
		otsu_wR = 0;
		otsu_mL = 0;
		otsu_mR = 0;
		otsu_max = 0;
		otsu_trshld = 0;

		for (k = 0; k < 256; k++) {
			otsu_wL += hist[k];	// вычисляем вес слева

			if (otsu_wL == 0)
				continue;

			if (otsu_wL > 1)	// немного магии
				otsu_wL = 1;

			otsu_wR = 1 - otsu_wL;	// вычисляем вес справа (1 - максимальное значение)

			if (otsu_wR == 0)
				break;

			/* Вычисляем межклассовую дисперсию. */
			otsu_sum_left += (k + 1) * hist[k];
			otsu_mL = otsu_sum_left / otsu_wL;
			otsu_mR = (otsu_sum - otsu_sum_left) / otsu_wR;
			otsu_between = otsu_wL * otsu_wR * (otsu_mL - otsu_mR) * (otsu_mL - otsu_mR);
			/* Запоминаем максимальную. */
			if (otsu_between >= otsu_max) {
				otsu_max = otsu_between;
				otsu_trshld = k - 1;
			}
		}
		/* Области с низкой сигмой будем считать фоном. */
		if (sigma < max_sigma) {
			mu = 128;	// 256 / 2 = 128
			for (i = 0; i < curr_y_size; i++) {
				y = y_block * n_size + i;
				for (j = 0; j < curr_x_size; j++) {
					x = x_block * n_size + j;
					curr_ind = y * stride + x;
					out_img[curr_ind] = THRESHOLD(pix[curr_ind], mu);
				}
			}
		}else{
			/* Применяем полученный порог к текущей области. */
			for (i = 0; i < curr_y_size; i++) {
				y = y_block * n_size + i;
				for (j = 0; j < curr_x_size; j++) {
					x = x_block * n_size + j;
					curr_ind = y * stride + x;
					out_img[curr_ind] = THRESHOLD(pix[curr_ind], otsu_trshld);
				}
			}
		}

		/* Переходим к следущему блоку. */
		curr_x_size = curr_y_size = n_size;
		x_block++;		// увеличиваем x блок
		/* Если прошли до последнего блока в строке: */
		if (x_block == divisions) {
			if (x_rest != 0) {
				curr_x_size = x_rest;
			}else{
				x_block = 0;	// переходим к след. строке
				y_block++;
			}
		}

		if (x_block > divisions) {
			x_block = 0;
			y_block++;	// переходим к след. строке
		}

		if (y_block == y_divisions) {
			if (y_rest == 0)
				break;	// прошли все блоки
			else
				curr_y_size = y_rest;
		}

		if (y_block > y_divisions)
			break;	// прошли все блоки
	}
	/* Выделяем память для результирующей структуры. */
	result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	if (result == NULL) {
		printf("Не удалось выделить память для результирующей структуры с информацией о бинаризованном изображении.\n");
		return NULL;
	}
	/* Запоминаем полученную информацию.*/
	result->width = width;
	result->height = height;
	result->stride = stride;
	result->bytes_for_pix = 0;	// '0' для бинаризованного изображения
	result->pix = out_img;		// ссылаемся на полученное изображение

	return result;
}

void ocr_imgproc_filter_sobel(ocr_img_info *img)
{
	/* Если не серое изображение. */
	if (img->bytes_for_pix == 1)
		return;

	int *filter_x = NULL;		// массив для коэффициентов фильтра по x
	int *filter_y = NULL;		// массив для коэффициентов фильтра по y
	int i = 0, j = 0, curr = 0;
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int rowpix = img->bytes_for_pix;
	int sum_x = 0;
	int sum_y = 0;
	uchar *out_img = NULL;
	uchar *pix = img->pix;
	/* Создадим матрицу размерности 3х3
		|0|1|2|
		|3|4|5|
		|6|7|8|
	*/
	filter_x = (int *)malloc(sizeof(int) * 9);
	/* Фильтр Собеля по x.
		|-1|-2|-1|
		|_0|_0|_0|
		|_1|_2|_1|
	*/

	filter_x[0] = -1;
	filter_x[1] = -2;
	filter_x[2] = -1;

	filter_x[3] = 0;
	filter_x[4] = 0;
	filter_x[5] = 0;

	filter_x[6] = 1;
	filter_x[7] = 2;
	filter_x[8] = 1;

	filter_y = (int *)malloc(sizeof(int) * 9);
	/* Фильтр Собеля по y.
		|_1|_0|-1|
		|_2|_0|-2|
		|_1|_0|-1|
	*/
	filter_y[0] = 1;
	filter_y[1] = 0;
	filter_y[2] = -1;

	filter_y[3] = 2;
	filter_y[4] = 0;
	filter_y[5] = -2;

	filter_y[6] = 1;
	filter_y[7] = 0;
	filter_y[8] = -1;

	/* Инициализируем выходное изображения. */
	out_img = (uchar *)malloc(sizeof(uchar) * height * stride);
	if(out_img == NULL){
		printf("Не удалось инициализировать память для выходного изображения.\n");
		return;
	}
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			curr = i * stride + j * rowpix;
			if (i > 0 && j > 0 && i < height - 1 && j < width - 1) {
				sum_x = 0;
				sum_y = 0;

				sum_x += pix[curr - stride - rowpix] * filter_x[0];
				sum_x += pix[curr - stride] * filter_x[1];
				sum_x += pix[curr - stride + rowpix] * filter_x[2];

				sum_x += pix[curr - rowpix] * filter_x[3];
				sum_x += pix[curr] * filter_x[4];
				sum_x += pix[curr + rowpix] * filter_x[5];

				sum_x += pix[curr + stride - rowpix] * filter_x[6];
				sum_x += pix[curr + stride] * filter_x[7];
				sum_x += pix[curr + stride + rowpix] * filter_x[8];

				sum_y += pix[curr - stride - rowpix] * filter_y[0];
				sum_y += pix[curr - stride] * filter_y[1];
				sum_y += pix[curr - stride + rowpix] * filter_y[2];

				sum_y += pix[curr - rowpix] * filter_y[3];
				sum_y += pix[curr] * filter_y[4];
				sum_y += pix[curr + rowpix] * filter_y[5];

				sum_y += pix[curr + stride - rowpix] * filter_y[6];
				sum_y += pix[curr + stride] * filter_y[7];
				sum_y += pix[curr + stride + rowpix] * filter_y[8];

				if ((sum_x * sum_x + sum_y * sum_y) <= 127 * 127) {
					out_img[curr - rowpix] = CR_WHITE;
					out_img[curr] = CR_WHITE;
					out_img[curr + rowpix] = CR_WHITE;
				}else{
					out_img[curr - rowpix] = CR_BLACK;
					out_img[curr] = CR_BLACK;
					out_img[curr + rowpix] = CR_BLACK;
				}
			}
		}
	}
	free(img->pix);
	img->pix = out_img;
}

void ocr_imgproc_filter_gauss(ocr_img_info *img)
{
	/* Если не серое изображение. */
/*	if (img->bytes_for_pix != 1)
		return;

	int i = 0, j = 0, curr = 0;
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int rowpix = img->bytes_for_pix;
	int sum = 0;
	int cell_count = 9;
	char tmp = 0;
	uchar *out_img = NULL;
	uchar *pix = img->pix;
	int *filter = (int *)malloc(sizeof(int) * cell_count);
*/	/* Фильтр Гаусса./
		|1|2|1|
		|2|4|2|
		|1|2|1|
	*/
/*	filter[0] = 1;
	filter[1] = 2;
	filter[2] = 1;

	filter[3] = 2;
	filter[4] = 4;
	filter[5] = 2;

	filter[6] = 1;
	filter[7] = 2;
	filter[8] = 1;

*/	/* Выделяем память для выходного изображения. */
	/*out_img = (uchar *)malloc(sizeof(uchar) * stride * height);
	if (out_img == NULL) {
		printf("Не удалось выделить память для выходного изображения.\n");
		return;
	}
	int radius = 1;
	int size = 2 * radius + 1;
	double *kernel = (double *)malloc(sizeof(double) * size);
	double coeff = 1.0 / (2.0 * radius * radius);
	int r = -radius;
	double x = 0.0;

	for (i = 0; i < size; i++) {
		x = r;
		x *= x;
		kernel[i] = coeff * exp(-x * coeff);
		r++;
	}
*/	/*for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			curr = i * stride + j * rowpix;
			sum = 0;
			cell_count = 9;

			if(i > 0 && j > 0)
				sum += filter[0] * pix[curr - stride - rowpix];
			else
				cell_count--;

			if(i > 0)
				sum += filter[1] * pix[curr - stride];
			else
				cell_count--;

			if(i > 0 && j < width - 1)
				sum += filter[2] * pix[curr - stride + rowpix];
			else
				cell_count--;

			if(j > 0)
				sum += filter[3] * pix[curr - rowpix];
			else
				cell_count--;
			sum += filter[4] * pix[curr];
			if(j < width - 1)
				sum += filter[5] * pix[curr + rowpix];
			else
				cell_count--;

			if(i < height - 1 && j > 0)
				sum += filter[6] * pix[curr + stride - rowpix];
			else
				cell_count--;

			if(i < height - 1)
				sum += filter[7] * pix[curr + stride];
			else
				cell_count--;

			if(i < height - 1 && j < width - 1)
				sum += filter[8] * pix[curr + stride + rowpix];
			else
				cell_count--;
			tmp = (uchar) ((double)sum / 16);

			out_img[curr] = (uchar)tmp;
			out_img[curr + 1] = (uchar)tmp;
			out_img[curr + 2] = (uchar)tmp;
		}
	}*/
	//free(img->pix);
	//img->pix = out_img;
}

/* Функция сравнения чисел для быстрой сортировки. */
static int compare(const void  *p1, const void *p2)
{
	return (*(int *)p1 - *(int *)p2);
}

void ocr_imgproc_filter_median(ocr_img_info *img, int size)
{
	/* Если не серое изображение, прерываем. */
	if (img->bytes_for_pix != 1)
		return;

	int i = 0, j = 0, curr = 0;
	int k = 0, l = 0;
	int width = img->width - size;
	int height = img->height - size;
	int stride = img->stride;
	int win_size = (size * 2 + 1) * (size * 2 + 1);	// размер окна = текущий пиксель + отступ в size с каждой стороны.

	int side = 2 * size + 1;	// ширина окна
	uchar *pix = img->pix;
	uchar *out_img = NULL;
	uchar *window = NULL;

	/* Выделяем память для выходного изображения. */
	out_img = (uchar *)malloc(sizeof(uchar) * stride * height);
	if(out_img == NULL){
		printf("Не удалось выделить память для выходного изображения.\n");
		return;
	}
	/* Выделяем память для окна фильтра. */
	window = (uchar *)malloc(sizeof(uchar) * win_size);
	if(window == NULL){
		printf("Не удалось выделить память для фильтра.\n");
		return;
	}
	for (i = size; i < height; i++) {
		for (j = size; j < width; j++) {
			curr = i * stride + j;
			for (k = -size; k <= size; k++) {
				for (l = -size; l <= size; l++) {
					window[k * side + l] = pix[curr + k * stride + l];
				}
			}
			/* Сортируем элементы в текущем окне. */
			qsort(window, win_size, sizeof(uchar), compare);
			/* Присваиваем средний элемент текущему. */
			out_img[curr] = window[win_size / 2];
		}
	}
	free(img->pix);
	img->pix = out_img;
}

void ocr_imgproc_invert(ocr_img_info *img)
{
	/* Если изображение цветное, то
	прерываем. */
	if (img->bytes_for_pix > 1)
		return;

	int i = 0, j = 0;
	int width = img->width;
	int height = img->height;
	int stride = img->stride;

	for (i = 0; i < height; i++){
		for (j = 0; j < width; j++){
			img->pix[i * stride + j] = 255 - img->pix[i * stride + j];
		}
	}
}

void ocr_imgproc_threshold_reduction(ocr_img_info *img, int cell_size, int threshold)
{
	if (img->bytes_for_pix != 0) {
		printf("Входное изображение не бинаризовано.\n");
		return;
	}
	int i = 0, j = 0, k = 0;
	int stride = img->stride;
	int height = img->height;
	int width = img->width;
	int new_width = 0;
	int new_height = 0;
	int new_stride = 0;
	int curr_value = 0;
	int curr_ind = 0;
	int cells_count = 0;
	uchar *pix = img->pix;
	uchar *out_img = NULL;
	new_width = width / cell_size;
	new_height = height / cell_size;
	/* Определяеям размер битмап строки в памяти. */
	if (new_width % WORD_SIZE == 0)
		new_stride = new_width;	// если ширина кратна машинному слову
	else
		/* Округляем до следущего машинного слова. */
		new_stride = new_width + (WORD_SIZE - new_width % WORD_SIZE);

	out_img = (uchar *)malloc(sizeof(uchar) * new_stride * new_height);
	if (out_img == NULL) {
		printf("Не удалось выделить память для сжатого по порогу изображения.\n");
		return;
	}

	cells_count = cell_size * cell_size;
	for (i = 0; i < new_height; i++) {
		for (j = 0; j < new_width; j++) {
			for (k = 0; k < cells_count; k++) {
				curr_ind = (i * cell_size + k / cell_size) * stride + j * cell_size + k % cell_size;
				curr_value += pix[curr_ind];
			}
			curr_value /= CR_BLACK;
			out_img[i * new_stride + j] = THRESHOLD(curr_value, threshold);
		}
	}

	img->width = new_width;
	img->height = new_height;
	img->stride = new_stride;
	img->bytes_for_pix = 0;		// '0' - значения для бинаризованных
	free(img->pix);
	img->pix = out_img;
}

void ocr_imgproc_threshold_expansion(ocr_img_info *img, int cell_size)
{
	if (img->bytes_for_pix != 0) {
		printf("Входное изображение не бинаризовано.\n");
		return;
	}
	int i = 0, j = 0, k = 0;
	int stride = img->stride;
	int height = img->height;
	int width = img->width;
	int new_width = 0;
	int new_height = 0;
	int new_stride = 0;
	int out_ind = 0;
	int curr_ind = 0;
	int cells_count = 0;
	uchar *pix = img->pix;
	uchar *out_img = NULL;

	new_width = width * cell_size;
	new_height = height * cell_size;
	/* Определяеям размер битмап строки в памяти. */
	if (new_width % WORD_SIZE == 0)
		new_stride = new_width;	// если ширина кратна машинному слову
	else
		/* Округляем до следущего машинного слова. */
		new_stride = new_width + (WORD_SIZE - new_width % WORD_SIZE);
	out_img = (uchar *)malloc(sizeof(uchar) * new_stride * new_height);
	if (out_img == NULL) {
		printf("Не удалось выделить память для сжатого по порогу изображения.\n");
		return;
	}

	cells_count = cell_size * cell_size;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			curr_ind = i * stride + j;
			for (k = 0; k < cells_count; k++) {
				out_ind = (i * cell_size + k / cell_size) * new_stride + j * cell_size + k % cell_size;
				if (pix[curr_ind] == CR_BLACK)
					out_img[out_ind] = CR_BLACK;
				else
					out_img[out_ind] = CR_WHITE;
			}
		}
	}

	img->width = new_width;
	img->height = new_height;
	img->stride = new_stride;
	img->bytes_for_pix = 0;		// '0' - значения для бинаризованных
	free(img->pix);
	img->pix = out_img;
}



/******************************************************************************/

void ocr_imgproc_morph_dilate_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin)
{
	/* Проверяем исходную точку на выход за пределы границ образца. */
	if (struct_elem->width * struct_elem->height <= origin || origin < 0) {
		printf("Исходная точка образца для дилатации за границами образца.\n");
		return;
	}

	int i = 0, j = 0, curr_ind = 0;		// переменные индексов
	int x = 0, y = 0;			// координат ысмещения
	int width = text->width;		// ширину изображения
	int height = text->height;		// высоту изображения
	int size = height * width;		// размер изображения
	int se_width = struct_elem->width;	// ширину образца
	int se_height = struct_elem->height;	// высоту образца
	int se_size = se_height * se_width;	// размер образца
	int orig_x = 0;				// x-координата исходной точки
	int orig_y = 0;				// y-координата исходной точки
	int curr_h = 0;				// текущий индекс по высоте
	uchar *pix = text->pix;			// запоминаем указатель на изображение
	uchar *se_pix = struct_elem->pix;	// запоминаем указатель на образце
	uchar *out_text = (uchar *)malloc(sizeof(uchar) * size);

	if (out_text == NULL) {
		printf("Не удалось выделить память для выходного дилатированного изображения.\n");
		return;
	}
	/* Закрашиваем все пиксели белым цветом. */
	out_text = (uchar *)memset(out_text, CR_WHITE, size);
	if (out_text == NULL) {
		printf("Не удалось обнулить выходное дилатированное изображение.\n");
		return;
	}
	/* Определяем координаты исходной точки образца. */
	orig_x = origin % se_width;
	orig_y = origin / se_width;
	/* Просматриваем все пиксели. */
	for (i = 0; i < size; i++) {
		/* Если пиксель черный. */
		if (pix[i] == CR_BLACK) {
			curr_h = i / width;	// определяем текущую индекс по высоте
			for (j = 0; j < se_size; j++) {
				/* Определяем координаты смещения. */
				x = orig_x - (j % se_width);
				y = orig_y - (j / se_width);
				curr_ind = i - x - y * width;
				/* Если индекс не вышел за границы изображения и соотв.
				пиксель образца черная. */
				if (((i - x) % width >= 0) && ((i - x) % width < width) &&
				(curr_h + y < height) && (se_pix[j] == CR_BLACK) &&
				(curr_ind >= 0) && (curr_ind < size)) {
					out_text[curr_ind] = CR_BLACK;	// закрашиывем выходной пиксель
				}
			}
		}
	}
	free(text->pix);
	text->pix = out_text;
}

void ocr_imgproc_morph_errosion_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin)
{
	/* Проверяем исходную точку на выход за пределы границ образца. */
	if (struct_elem->width * struct_elem->height <= origin || origin < 0) {
		printf("Исходная точка образца для эррозии` за границами образца.\n");
		return;
	}

	int i = 0, j = 0, curr_ind = 0;		// переменные индексов
	int x = 0, y = 0;			// координат ысмещения
	int width = text->width;		// ширину изображения
	int height = text->height;		// высоту изображения
	int size = height * width;		// размер изображения
	int se_width = struct_elem->width;	// ширину образца
	int se_height = struct_elem->height;	// высоту образца
	int se_size = se_height * se_width;	// размер образца
	int orig_x = 0;				// x-координата исходной точки
	int orig_y = 0;				// y-координата исходной точки
	int curr_h = 0;				// текущий индекс по высоте
	int curr_color = 0;			// переменная для хранения текущего цвета
	int black_count = 0;			// число черных пикселей в текущем образце
	uchar *pix = text->pix;			// запоминаем указатель на изображение
	uchar *se_pix = struct_elem->pix;	// запоминаем указатель на образце
	uchar *out_text = (uchar *)malloc(sizeof(uchar) * size);

	if (out_text == NULL) {
		printf("Не удалось выделить память для выходного дилатированного изображения.\n");
		return;
	}
	/* Закрашиваем все пиксели белым цветом. */
	out_text = (uchar *)memset(out_text, CR_WHITE, size);
	if (out_text == NULL) {
		printf("Не удалось обнулить выходное дилатированное изображение.\n");
		return;
	}
	/* Определяем координаты исходной точки образца. */
	orig_x = origin % se_width;
	orig_y = origin / se_width;

	for (i = 1; i < size; i++) {
		/* Определяем текущий индекс по высоте. */
		curr_h = i / width;
		/* Сбрасываем счетчик и переменную для определения цвета. */
		curr_color = 0;
		black_count = 0;
		for (j = 1; j < se_size; j++) {
			if (se_pix[j] == CR_BLACK) {
				/* Определяем координаты смещения. */
				x = orig_x - (j % se_width);
				y = orig_y - (j / se_width);
				/* Определяем текущую координату во входном изобьражении. */
				curr_ind = i - x - y * width;
				/* Если индекс не вышел за границы изображения. */
				if (((i - x) % width >= 0) && ((i - x) % width < width) &&
				(curr_h + y < height) && (se_pix[j] == CR_BLACK) &&
				(curr_ind >= 0) && (curr_ind < size)) {
					out_text[curr_ind] = CR_BLACK;	// закрашиывем выходной пиксель
					black_count++;
					curr_color += pix[curr_ind];	// закрашиывем выходной пиксель
				}
			}
		}
		/* Если все пиксели совпавшие с черными пикселями образца черные. */
		if (curr_color / black_count == CR_BLACK)
			out_text[i] = CR_BLACK;	// присваиваем выходному пикселю черный цвет
		else
			out_text[i] = CR_WHITE;	// присваиваем выходному пикселю белый цвет
	}
	free(text->pix);
	text->pix = out_text;
}

void ocr_imgproc_morph_invert_text(ocr_text_area *text)
{
	int i = 0;
	int size = text->width * text->height;

	for (i = 0; i < size; i++) {
		if (text->pix[i] == CR_WHITE)
			text->pix[i] = CR_BLACK;
		else
			text->pix[i] = CR_WHITE;
	}
}

void ocr_imgproc_morph_invert(ocr_img_info *img)
{
	int i = 0;
	int size = img->stride * img->height;

	for (i = 0; i < size; i++) {
		if (img->pix[i] == CR_WHITE)
			img->pix[i] = CR_BLACK;
		else
			img->pix[i] = CR_WHITE;
	}
}

void ocr_imgproc_morph_exclude_text(ocr_text_area *source, ocr_text_area *exclude)
{
	int i = 0;
	int size = source->width * source->height;

	if (source->width != exclude->width ||
	source->height != exclude->height)
		return;

	for (i = 0; i < size; i++) {
		if (source->pix[i] == CR_BLACK && exclude->pix[i] == CR_WHITE)
			source->pix[i] = CR_BLACK;
		else
			source->pix[i] = CR_WHITE;
	}
}

void ocr_imgproc_morph_intersect_text(ocr_text_area *area1, ocr_text_area *area2)
{
	if (area1->width != area2->width || area1->height != area2->height) {
		printf("1-я область должна быть равной 2-й.\n");
		return;
	}

	int i = 0;
	int size = area1->width * area1->height;

	for (i = 0; i < size; ++i) {
		area1->pix[i] = *area2->pix[i] | *area1->pix[i];
	}
}

void ocr_imgproc_morph_combine_text(ocr_text_area *area1, ocr_text_area *area2)
{
	if (area1->width != area2->width || area1->height != area2->height) {
		printf("1-я область должна быть равной 2-й.\n");
		return;
	}

	int i = 0;
	int size = area1->width * area1->height;

	for (i = 0; i < size; ++i) {
		area1->pix[i] = *area1->pix[i] & *area2->pix[i];
	}
}

void ocr_imgproc_morph_hit_or_miss_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin)
{
	int size = struct_elem->stride * struct_elem->height;
	ocr_img_info se_in;
	ocr_text_area text_in;
	/* Создаем копию образца. */
	se_in.pix = (uchar *)malloc(sizeof(uchar) * size);
	se_in.width = struct_elem->width;
	se_in.height = struct_elem->height;
	se_in.stride = struct_elem->stride;
	se_in.bytes_for_pix = struct_elem->bytes_for_pix;
	/* Создаем копию входной текстовой области. */
	size = text_in.width * text_in.height;
	text_in.pix = (uchar *)malloc(sizeof(uchar) * size);
	text_in.width = text->width;
	text_in.height = text->height;
	text_in.x = text->x;
	text_in.y = text->y;
	/* Инвертируем входной образец. */
	ocr_imgproc_morph_invert(&se_in);
	/* Применяем ерозию к входному изображению. */
	ocr_imgproc_morph_errosion_text(text, struct_elem, origin);
	/* Применяем дилатацию к входному изображении. */
	ocr_imgproc_morph_dilate_text(&text_in, &se_in, origin);
	/* Получаем пересечение областей. */
	ocr_imgproc_morph_intersect_text(text, &text_in);
}

ocr_img_info *ocr_imgproc_get_se_nxn(int n)
{
	if (n <= 0) {
		printf("Некорректная размерность.\n");
		return NULL;
	}
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	if (result == NULL) {
		printf("Не удалось выделить память для образца 3х3.\n");
		return NULL;
	}
	result->width = n;
	result->height = n;
	result->stride = n;
	result->bytes_for_pix = 0;
	result->pix = (uchar *)malloc(sizeof(uchar) * n * n);
	if (result->pix == NULL) {
		printf("Не удалось выделить память для пикселей образца 3х3.\n");
		return NULL;
	}
	result->pix = (uchar *)memset(result->pix, CR_BLACK, n * n);
	return result;
}

void ocr_imgproc_morph_think_text(ocr_text_area *text, ocr_img_info *struct_elem, int origin)
{
	ocr_text_area input_text;
	/* Делаем копию входной текстовой области. */
	input_text.width = text->width;
	input_text.height = text->height;
	input_text.x = text->x;
	input_text.y = text->y;
	input_text.pix = (uchar *)malloc(sizeof(uchar) * text->width * text->height);
	if (input_text.pix == NULL) {
		printf("Не удалось выделить память для пикселей входного изображения.\n");
		return;
	}
	input_text.pix = (uchar *)memcpy(input_text.pix, text->pix, text->width * text->height);
	if (input_text.pix == NULL) {
		printf("Не удалось скопировать пиксели входного изображения.\n");
	}
	/* Преобразование "успех/неудача". */
	ocr_imgproc_morph_hit_or_miss_text(&input_text, struct_elem, origin);
	/* Исключение из входной области преобразованную. */
	ocr_imgproc_morph_exclude_text(text, &input_text);
}

ocr_text_area *ocr_img2textarea(ocr_img_info *img)
{
	int i = 0;
	int size = 0;
	int width = img->width;
	int height = img->height;
	ocr_text_area *text = (ocr_text_area *)malloc(sizeof(ocr_text_area));
	if (text == NULL) {
		printf("Не удалось выделить память для выходной текстовой области.\n");
		return NULL;
	}
	/* Копируем ширину и высоту. */
	text->width = width;
	text->height = height;
	text->pix = (uchar *)malloc(sizeof(uchar) * width * height);
	if (text->pix == NULL) {
		printf("Не удалось выделить память для пикселей выходной текстовой области.\n");
		return NULL;
	}
	/* Копируем пиксели в массив, без сдвигов. */
	size = width * height;
	for (i = 0; i < size; i++) {
		text->pix[i] = img->pix[i / width * img->stride + i % width];
	}
	return text;
}
