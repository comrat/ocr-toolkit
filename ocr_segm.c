#include "ocr_meta.h"
#include "ocr_segm.h"
#include <stdlib.h>
#include <math.h>

#include <stdio.h>


void ocr_segm_page_vert_divisor(ocr_img_info *img, int *div_be, int *div_end)
{
	if (img->bytes_for_pix != 1)
		return;

	int i = 0, j = 0, k = 0;
	int curr = 0;			// current index pixel
	int width = img->width;
	int height = img->height;
	int stride = img->stride;
	int middle_be = (width >> 1) - (width >> 3);	// middle delimeter area start index
	int middle_en = (width >> 1) + (width >> 3);	// middle delimeter area end index
	int window_width = 10;
	int be = -1;
	int end = -1;
	uchar *pix = img->pix;
	double stat = 0.0;
	double prev_stat = 2.0;

	/* Reset return values. */
	*div_end = -1;
	*div_be = -1;

	/* Apply 10pixels slide window. */
	for (i = 0; i < width - window_width; i++) {
		stat = 0;
		for (j = 0; j < height; j++) {
			curr = j * stride + i;
			for (k = 0; k < window_width; k++) {
				stat += pix[curr + k];
			}
		}
		/* Get black pixels count. */
		stat /= CR_BLACK * height * window_width;

		/* Check current index is it page delimeter. */
		if (stat > 0.6 && i > middle_be && i < middle_en) {
			be = i;
			break;
		}
	}

	if (be == -1)
		return;

	*div_be = be;

	/* Figuring out page delimeter width. */
	for (i = be; i >= middle_be; i--){
		stat = 0;
		for (j = 0; j < height; j++){
			curr = j * stride + i;
			stat += pix[curr];
		}
		stat /= CR_BLACK * height;
		if (stat < 0.1) {
			*div_be = i;
			break;
		}
	}

	/* Find delimeter area end. */
	for (i = be; i < middle_en; i++) {
		stat = 0;
		for (j = 0; j < height; j++) {
			curr = j * stride + i;
			stat += pix[curr];
		}
		stat /= CR_BLACK * height;
		if (stat < 0.1) {
			*div_end = i;
			break;
		}
	}
}

void ocr_segm_get_area(ocr_img_info *img, coord *begin, coord *end){
	int i = 0, j = 0;
	int curr = 0;
	int width = img->width;
	int height = img->height;
	int stride = img->stride;
	int space_begin = -1;
	int w_quart = width >> 2;
	int h_quart = height >> 2;
	int w_quart_r = width - w_quart;
	int h_quart_b = height - h_quart;
	uchar *pix = img->pix;
	double stat = 0.0;
	double small_mu = 0.025;
	double big_mu = 2 * small_mu;
	begin->x = 0;
	begin->y = 0;
	end->x = width - 1;
	end->y = height - 1;

	/* Find left border of the first qurter. */
	for (i = 1; i < w_quart; i++) {
		stat = 0.0;
		for (j = 0; j < height; j++) {
			curr = stride * j + i;
			stat += pix[curr];
		}
		stat /= CR_BLACK * height;
		if (stat > big_mu && space_begin != -1) {
			begin->x = i - 1;
			break;
		}
		if (stat < small_mu && space_begin == -1) {
			space_begin = i;
		}
	}
	/* Find tight border. */
	space_begin = -1;
	// TODO: rempve code duplication
	for (i = width - 2; i >= w_quart_r; i--){
		stat = 0.0;
		for (j = 0; j < height; j++){
			curr = stride * j + i;
			stat += pix[curr];
		}
		stat /= CR_BLACK * height;
		if (stat > big_mu && space_begin != -1){
			end->x = i + 1;
			break;
		}
		if (stat < small_mu && space_begin == -1){
			space_begin = i;
		}
	}
	/* Find upper bound. */
	space_begin = -1;
	for (i = 1; i < h_quart; i++) {
		stat = 0.0;
		for (j = 0; j < width; j++) {
			curr = stride * i + j;
			stat += pix[curr];
		}
		stat /= CR_BLACK * width;
		if (stat > big_mu && space_begin != -1) {
			begin->y = i - 1;
			break;
		}
		if (stat < 2 * small_mu && space_begin == -1) {
			space_begin = i;
		}
	}
	/* Find lower bound. */
	space_begin = -1;
	for (i = height - 2; i > h_quart_b; i--) {
		stat = 0.0;
		for (j = 0; j < width; j++) {
			curr = stride * i + j;
			stat += pix[curr];
		}
		stat /= CR_BLACK * width;
		if (stat > big_mu && space_begin != -1) {
			end->y = i + 1;
			break;
		}
		if (stat < 2 * small_mu && space_begin == -1) {
			space_begin = i;
		}
	}
}


ocr_cells_net *ocr_segm_get_net(ocr_img_info *grey, int cells_count, double sigma_threshold)
{
	if(grey->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0, curr = 0, curr_bin = 0;
	int x_be = 0, x_en = 0, y_be = 0, y_en = 0;
	int x_cell = 0, y_cell = 0;	// индексы ячейки в сетке
	int stride = grey->stride;
	int width = grey->width;
	int height = grey->height;
	int x_seg_count = cells_count;
	int segment_width = (int)((double)width / cells_count);
	int y_seg_count = height / segment_width;
	int rowpix = grey->bytes_for_pix;
	int curr_cells_count = 0;	// число клеток сетки в текущей итерации
	int div_be, div_end;		// индексы начала и конца рделителя страницы на 2 (если он есть)
	double sigma = 0;		// переменняа хранения сигмы
	double mu = 0;			// переменная дял хранения мат ожидания
	uchar is_on_divisor = 0;	// переменная равная 0, если пиксель лежит на разделители странице, иначе 255
	uchar **output = (char **)malloc(sizeof(char *) * y_seg_count);
	ocr_cells_net *result = (ocr_cells_net *)malloc(sizeof(ocr_cells_net));
	uchar *pix = grey->pix;
	coord begin, end;

	/* Инициализируем выходное изображение. */
	for(i = 0; i < y_seg_count; i++){
		output[i] = (char *)malloc(sizeof(char) * x_seg_count);
		for(j = 0; j < x_seg_count; j++){
			output[i][j] = 0;
		}
	}

	/* Находим индексы начала и конца разделителя страницы (если такой есть). */
	ocr_segm_page_vert_divisor(grey, &div_be, &div_end);
	printf("%dx%d\n", div_be, div_end);

	/* Находим рабочуюю область. */
	ocr_segm_get_area(grey, &begin, &end);

	x_cell = y_cell = 0;
	for(;;){
		/* Переход к новой строке сетки. */
		if(x_cell == x_seg_count){
			x_cell = 0;
			y_cell++;
		}

		/* Завершаем работу. */
		if(y_cell == y_seg_count)
			break;
		x_be = x_cell * segment_width;
		y_be = y_cell * segment_width;
		/* Если блок последний, просматриваем пиксели до края. */
		x_en = (x_cell == x_seg_count - 1) ? width : (x_cell + 1) * segment_width;
		y_en = (y_cell == y_seg_count - 1) ? height : (y_cell + 1) * segment_width;
		/* Вычисляем текущее число пикселей. */
		curr_cells_count = (y_en - y_be) * (x_en - x_be);
		sigma = 0.0;
		mu = 0.0;
		is_on_divisor = 0;
		for(i = x_be; i < x_en; i++){
			if(i >= div_be - 1 && i <= div_end + 1){
				is_on_divisor = 255;
				break;
			}
			for(j = y_be; j < y_en; j++){
				if(begin.x > i || end.x < i ||
				j > end.y || j < begin.y){
					sigma = 0;
					mu = 0;
					break;
				}
				curr = j * stride + i * rowpix;
				// Увеличиваем счетчик черных пикселей
				mu += pix[curr];
				sigma += pix[curr] * pix[curr];
			}
		}

		mu /= CR_BLACK * curr_cells_count;
		sigma /= CR_BLACK * curr_cells_count;
		sigma -= mu * mu;
		sigma = sqrt(sigma);

		//printf("%.2f ", sigma);
		/* Если сигма проходит по порогу, помечаем клетку как черную. */
		if(sigma >= sigma_threshold/* && is_on_divisor != 255*/)
			output[y_cell][x_cell] = CR_BLACK;

		x_cell++;
	}

	/* Сохраняем полученные настройки. */
	result->height = y_seg_count;
	result->cell_width = segment_width;
	result->width = x_seg_count;
	result->net = output;
	result->comp_count = 0;

	return result;
}

ocr_con_comp *ocr_segm_get_component(ocr_cells_net *cells, int **labeled)
{
	int i = 0, j = 0;
	int width = cells->width;
	int height = cells->height;
	int curr_label = 0;
	int curr_ind = 0;
	int curr_size = 0;
	uchar **net = cells->net;
	ocr_con_comp *component = NULL;
	coord *com_coords = NULL;
	coord *tmp_coords = NULL;
	ocr_con_comp tmp_com;
	coord tmp_coord;

	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			if(labeled[i][j] == 0){
				continue;
			}
			/* Add new component. */
			if(labeled[i][j] > curr_label){
				tmp_com.width = 0;
				tmp_com.height = 0;
				tmp_com.center.x = 0;
				tmp_com.center.y = 0;
				tmp_com.size = 1;
				/* Set coord-s of next component. */
				tmp_coord.x = j;
				tmp_coord.y = i;
				com_coords = (coord *)malloc(sizeof(coord));
				com_coords[0] = tmp_coord;
				tmp_com.coords = com_coords;
				//tmp_com.position.x = j;
				//tmp_com.position.y = i;
				/* Initialile area rectangle info. */
				tmp_com.up_left.x = 0;
				tmp_com.up_left.y = 0;
				tmp_com.bot_right.x = 0;
				tmp_com.bot_right.y = 0;
				/* Set "NOISE: type as default. */
				tmp_com.type = NOISE;

				curr_label++;
				component = (ocr_con_comp *)realloc(component, sizeof(ocr_con_comp) * curr_label);
				component[curr_label - 1] = tmp_com;
			}else{
				curr_ind = labeled[i][j] - 1;
				/* Set coord-s of next component. */
				tmp_coord.x = j;
				tmp_coord.y = i;
				/* Increase count of component segments. */
				component[curr_ind].size++;
				curr_size = component[curr_ind].size;
				/* Allocate more memory for next component segment. */
				component[curr_ind].coords = realloc(component[curr_ind].coords, sizeof(coord) * curr_size);
				/* Add next segment coord-s. */
				component[curr_ind].coords[curr_size - 1] = tmp_coord;
			}
		}
	}

	return component;
}

/* Рекурсиная "заливка" одной компоненты. */
void ocr_segm_marking_fill_rec(ocr_cells_net *net, int **labeled, int x, int y, int label)
{
	int width = net->width;
	int height = net->height;

	/* Если пиксель фоновыйы или уже помеченный, пропускаем. */
	if(net->net[y][x] == CR_WHITE || labeled[y][x] != 0){
		return;
	}

	labeled[y][x] = label;

	if(y > 0)
		ocr_segm_marking_fill_rec(net, labeled, x, y - 1, label);

	if(y < height - 1)
		ocr_segm_marking_fill_rec(net, labeled, x, y + 1, label);

	if(x < width - 1)
		ocr_segm_marking_fill_rec(net, labeled, x + 1, y, label);

	if(x > 0)
		ocr_segm_marking_fill_rec(net, labeled, x - 1, y, label);
}

/* Помечаем компоненты рекурсивно. */
int **ocr_segm_mark_components(ocr_cells_net *net)
{
	uchar **cells = net->net;
	int i = 0, j = 0;
	int width = net->width;
	int height = net->height;
	int con_count = 0;
	int **labeled = (int **)malloc(sizeof(int *) * height);

	/* Инициализируем поле с метками. */
	for(i = 0; i < height; i++){
		labeled[i] = (int *)malloc(sizeof(int) * width);
		for(j = 0; j < width; j++){
			labeled[i][j] = 0;
		}
	}

	/* Рекурсивно заполняем сетку метками. */
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			if((cells[i][j] == CR_BLACK) && (labeled[i][j] == 0)){
				con_count++;
				ocr_segm_marking_fill_rec(net, labeled, j, i, con_count);
			}
		}
	}

	net->comp_count = con_count;

	return labeled;
}

void ocr_segm_analyze_comp(ocr_con_comp *comp, int comp_count)
{
	int i = 0, j = 0;
	int max_x = 0, min_x = 0, max_y = 0, min_y = 0;

	for(i = 0; i < comp_count; i++){
		max_x = min_x = comp[i].coords[0].x;
		max_y = min_y = comp[i].coords[0].y;
		/* Находим мин. и макс. */
		for(j = 1; j < comp[i].size; j++){
			if(comp[i].coords[j].x > max_x)
				max_x = comp[i].coords[j].x;
			if(comp[i].coords[j].x < min_x)
				min_x = comp[i].coords[j].x;
			if(comp[i].coords[j].y > max_y)
				max_y = comp[i].coords[j].y;
			if(comp[i].coords[j].y < min_y)
				min_y = comp[i].coords[j].y;
		}
		/* вычисляем ширину и высоту компонент. */
		comp[i].width = max_x - min_x + 1;
		comp[i].height = max_y - min_y + 1;
		/* Запоминаем координаты диагонали прямоугольной обасти.*/
		comp[i].up_left.x = min_x;
		comp[i].up_left.y = min_y;
		comp[i].bot_right.x = max_x;
		comp[i].bot_right.y = max_y;
	}
}

ocr_segm_stat_info *ocr_segm_lines_stat(ocr_img_info *img, ocr_con_comp *component, int cell_width)
{
	/* Если изображение не бинаризованное, возвращаем NULL. */
	if(img->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0;
	int curr = 0;
	int y1 = component->up_left.y;
	int x1 = component->up_left.x;
	int x2 = component->bot_right.x;
	int y2 = component->bot_right.y;
	/* Переводим координаты сетки на изображение. */
	int x_be = x1 * cell_width;
	int x_en = (x2 == (img->width / cell_width - 1)) ? img->width : (x2 + 1) * cell_width - 1;
	int y_be = y1 * cell_width;
	int y_en = (y2 == (img->height / cell_width - 1)) ? img->height : (y2 + 1) * cell_width - 1;
	/* Запоминаем ширину и высоту компоненты на изображении. */
	int comp_width = x_en - x_be;
	int comp_height = y_en - y_be;
	double portion_of_black = 0.0;
	double *portion = (double *)malloc(sizeof(double) * comp_height);
	ocr_segm_stat_info *result = (ocr_segm_stat_info *)malloc(sizeof(ocr_segm_stat_info));

	for(i = y_be; i < y_en; i++){
		portion_of_black = 0.0;
		for(j = x_be; j < x_en; j++){
			curr = i * img->stride + j;
			portion_of_black += img->pix[curr];
		}
		portion_of_black /= /*CR_BLACK */ comp_width;
		//printf("%.2f |", portion_of_black);
		portion[i - y_be] = portion_of_black;
	}
//printf("\n");

	result->portion = portion;
	result->count = comp_height;

	return result;
}

ocr_segm_stat_info *ocr_segm_colls_stat(ocr_img_info *img, ocr_con_comp *component, int cell_width)
{
	if(img->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0;
	int curr = 0;
	int y1 = component->up_left.y;
	int x1 = component->up_left.x;
	int x2 = component->bot_right.x;
	int y2 = component->bot_right.y;
	/* Переводим координаты сетки на изображение. */
	int x_be = x1 * cell_width;
	int x_en = (x2 == (img->width / cell_width - 1)) ? img->width : (x2 + 1) * cell_width - 1;
	int y_be = y1 * cell_width;
	int y_en = (y2 == (img->height / cell_width - 1)) ? img->height : (y2 + 1) * cell_width - 1;
	/* Запоминаем ширину и высоту компоненты на изображении. */
	int comp_height = y_en - y_be;
	int comp_width = x_en - x_be;
	double portion_of_black = 0.0;
	double *portion = (double *)malloc(sizeof(double) * comp_width);
	ocr_segm_stat_info *result = (ocr_segm_stat_info *)malloc(sizeof(ocr_segm_stat_info));

	for(i = x_be; i < x_en; i++){
		portion_of_black = 0.0;
		for(j = y_be; j < y_en; j++){
			curr = j * img->stride + i;
			portion_of_black += img->pix[curr];
		}
		portion_of_black /= /*CR_BLACK */ comp_height;
		portion[i - x_be] = portion_of_black;
	}

	result->portion = portion;
	result->count = comp_width;

	return result;
}

double ocr_segm_get_comp_sigma(ocr_con_comp * component, ocr_img_info *img, int cell_width)
{
	if(img->bytes_for_pix != 1)
		return -1;

	int i = 0, j = 0;
	int curr = 0;
	int y1 = component->up_left.y;
	int x1 = component->up_left.x;
	int x2 = component->bot_right.x;
	int y2 = component->bot_right.y;
	/* Переводим координаты сетки на изображение. */
	int x_be = x1 * cell_width;
	int x_en = (x2 == (img->width / cell_width - 1)) ? img->width : (x2 + 1) * cell_width - 1;
	int y_be = y1 * cell_width;
	int y_en = (y2 == (img->height / cell_width - 1)) ? img->height : (y2 + 1) * cell_width - 1;
	/* Запоминаем ширину и высоту компоненты на изображении. */
	int comp_height = y_en - y_be;
	int comp_width = x_en - x_be;

	double mu = 0.0;
	double sigma = 0.0;

	for(i = x_be; i < x_en; i++){
		for(j = y_be; j < y_en; j++){
			curr = j * img->stride + i;
			mu += img->pix[curr];
			sigma += img->pix[curr] * img->pix[curr];
		}
	}
	mu /= CR_BLACK * comp_height * comp_width;
	sigma /= CR_BLACK * comp_height * comp_width;
	sigma = sqrt(sigma - mu * mu);

	return sigma;
}

void ocr_segm_classify_comp(ocr_con_comp *comp, ocr_img_info *img, ocr_cells_net *net, int comp_count, int cell_width)
{
	/* По умолчанию считаем тип компоненты шумом.  */
	comp->type = NOISE;
	/* Предварительная проверка компоненты.
	Если она занимает меньше 7 процентов всей площади,
	считаем ее шумовой. */
	//if(((double)comp->size / (comp->width * comp->height)) < 0.07)
	//	return;

	//if((double) (comp->width * comp->height) / (net->width * net->height) > 0.7)
	//	return;

	int i = 0, j = 0;	// переменные - счетчики
	int above_value = 0;	// число подряд идущих высот гистограммы выше текстового порога
	int below_value = 0;	// число подряд идущих высот гистограммы ниже текстового порога
	int intervals_count = 0;// яисло всех интервалов, как возрастания, так и уюывания
	int text_thrshld = 0.1;	// текстовый порог, отделяющих области строк от межстрочных
	int period_count = 0;	// число периодов
	double above_mu = 0;
	double colls_mu = 0.0;
	double colls_sigma = 0.0;
	//double lines_mu = 0.0;
	//double lines_sigma = 0.0;
	double tmp = 0;
	double sigma = ocr_segm_get_comp_sigma(comp, img, cell_width);
	ocr_hist_interval *intervals = NULL;

	ocr_segm_stat_info *lines_stat;	// переменная для хранения гистограммы черных пикселей по горизонтали
	ocr_segm_stat_info *colls_stat;	// переменная для хранения гистограммы черных пикселей по вертикали

	if(((double)comp->size / (comp->width * comp->height)) > 0.85){
		if((((double)comp->width / comp->height < 4) ||
		((double)comp->height / comp->width > 0.25)) &&
		(comp->size > 14)){
			printf("(%d)%dx%d=%.2f | %.2f\n", comp->size, comp->width, comp->height, (double)comp->width / comp->height, (double)comp->height / comp->width);
			comp->type = PICTURE;
			return;
		}
	}

	/* Построим гистограммы частот черных пикселей. */
	lines_stat = ocr_segm_lines_stat(img, comp, cell_width);
	colls_stat = ocr_segm_colls_stat(img, comp, cell_width);
	/* Анализируем статистически по столбцам. */
	for(i = 0; i < colls_stat->count; i++){
		colls_mu += colls_stat->portion[i];
		colls_sigma += colls_stat->portion[i] * colls_stat->portion[i];
	}
	/* Вычисляем мат. ожидание. */
	colls_mu /= colls_stat->count;
	/* Вычисляем сигму. */
	colls_sigma /= colls_stat->count;
	colls_sigma = sqrt(colls_sigma - colls_mu * colls_mu);
	printf("%.2f Mu%.2f\n", colls_sigma, colls_mu);
}

int ocr_segm_get_page_count(ocr_img_info *img)
{
	int be, end;
	ocr_segm_page_vert_divisor(img, &be, &end);
	return (be == -1 || end == -1) ? 1 : 2;
}

ocr_con_comp *ocr_segm_get_comp_by_net(ocr_img_info *grey, ocr_img_info *bin, int cells_count, double sigma_thrshld)
{
	/* Получаем сетку. */
	ocr_cells_net *net = ocr_segm_get_net(bin, cells_count, sigma_thrshld);

	if(net == NULL){
		return NULL;
	}

	/* Получение компонент по сетке. */
	int i = 0, j = 0;
	int comp_count = 0;	// переменная для хранения числа компонент связности.
	int cell_width = (int)((double)(grey->width) / cells_count);

	/* Строим 2-мерную сетку для меток. */
	int **labeled = ocr_segm_mark_components(net);
	/* Запоминаем число компонент. */
	comp_count = net->comp_count;
	/* Отмечаем метками каждую компоненту связности. */
	ocr_con_comp *components = ocr_segm_get_component(net, labeled);
	/* Сетка с метками больше не нужна. */

	for(i = 0; i < net->height; i++){
		for(j = 0; j < net->width; j++){
			if(labeled[i][j] != 0)
				printf("%d|", labeled[i][j] % 10);
			else
				printf("_|");
		}
		printf("\n");
	}

	/* Определяем основные характеристики (высота, ширина и т.п.) компонент. */
	ocr_segm_analyze_comp(components, comp_count);

	int curr = 0;
	int k = 0;
	int l = 0;

	for(i = 0; i < grey->height; i++){
		for(j = 0; j < grey->width; j++){
			curr = i * grey->stride + j;
			k = i / cell_width;
			l = j / cell_width;
			if(k >= net->height - 1 || l >= net->width - 1)
				continue;
			if(labeled[k][l] == 0);
				continue;
		}
	}

	/* Сетка больше не нужна. */
	free(net->net);
	free(labeled);
	free(net);
	/* Определяем тип компонент: текст, фон, рисунок, формула или шум. */
	return components;
}

ocr_img_info *ocr_segm_rls_vert(ocr_img_info *img, int shift)
{
	if(img->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0;
	int k = 0, curr = 0;
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int tmp = 0;
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	uchar *pix = img->pix;
	uchar *out_img = (uchar *)malloc(sizeof(char) * stride * height);

	for(i = 0; i < width; i++){
		for(j = 0; j < height; j++){
			curr = j * stride + i;
			if(pix[curr] == CR_BLACK){
				/* Определяем, до какого пикселя "размывать". */
				tmp = (j > shift) ? curr - shift * stride: curr - j * stride;
				/*  Обращаем все предыдущие белые пиксели в черные. */
				for(k = curr; k >= tmp; k -= stride){
					out_img[k] = CR_BLACK;
				}
			}else
				out_img[curr] = CR_WHITE;
		}
	}

	result->width = width;
	result->height = height;
	result->stride = stride;
	result->bytes_for_pix = 1;
	result->pix = out_img;

	return result;
}

ocr_img_info *ocr_segm_rls_horizont(ocr_img_info *img, int shift)
{
	if(img->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0;
	int k = 0, curr = 0;
	int stride = img->stride;
	int width = img->width;
	int height = img->height;
	int tmp = 0;
	ocr_img_info *result = (ocr_img_info *)malloc(sizeof(ocr_img_info));
	uchar *pix = img->pix;
	uchar *out_img = (uchar *)malloc(sizeof(char) * stride * height);

	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			curr = i * stride + j;
			if(pix[curr] == CR_BLACK){
				/* Определяем, до какого пикселя "размывать". */
				tmp = (i > shift) ? curr - shift: curr - i;
				/*  Обращаем все предыдущие белые пиксели в черные. */
				for(k = curr; k >= tmp; k--){
					out_img[k] = CR_BLACK;
				}
			}else
				out_img[curr] = CR_WHITE;//pix[curr];
		}
	}

	result->width = width;
	result->height = height;
	result->stride = stride;
	result->bytes_for_pix = 1;
	result->pix = out_img;

	return result;
}

ocr_con_comp *ocr_segm_rlsa(ocr_img_info *grey, ocr_img_info *bin, int vert_shift, int horizont_shift)
{
	/* Если входные изображения не соответствуют условиям,
	возвращаем null. */
	if(grey->bytes_for_pix != 1 || bin->bytes_for_pix != 1)
		return NULL;

	/* "Размазываем" по вертикали и горизонтали. */
	ocr_img_info *vert = ocr_segm_rls_vert(bin, vert_shift);
	ocr_img_info *horizont = ocr_segm_rls_horizont(bin, horizont_shift);

	int i = 0, j = 0;
	int width = bin->width;
	int height = bin->height;
	int stride = bin->stride;
	int curr = 0;
	uchar *pix = (uchar *)malloc(sizeof(uchar) * stride * height);

	/* Получание пересечение множеств vert & horizont. */
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			curr = i * stride + j;
			if((vert->pix[curr] == CR_BLACK) && (horizont->pix[curr] == CR_BLACK))
				pix[curr] = CR_BLACK;
			else
				pix[curr] = CR_WHITE;
		}
	}

	free(bin->pix);
	bin->pix = pix;

	/* Уборка мусора. */
	free(vert);
	free(horizont);


	return NULL;
}



/************************************************************************
см. "Page segmentation and classification using fast feature extraction and
connectivity analysis." J. Sauvola, M. Pietikainen.
*************************************************************************/

ocr_cells_net *ocr_segm_and_classification(ocr_img_info *grey, int cells_count, double sigma_threshold)
{
	if(grey->bytes_for_pix != 1)
		return NULL;

	int i = 0, j = 0, curr = 0, curr_bin = 0;
	int x_be = 0, x_en = 0, y_be = 0, y_en = 0;
	int x_cell = 0, y_cell = 0;	// индексы ячейки в сетке
	int stride = grey->stride;
	int width = grey->width;
	int height = grey->height;
	int x_seg_count = cells_count;
	int segment_width = (int)((double)width / cells_count);
	int y_seg_count = height / segment_width;
	int rowpix = grey->bytes_for_pix;
	int curr_cells_count = 0;	// число клеток сетки в текущей итерации
	int div_be, div_end;		// индексы начала и конца рделителя страницы на 2 (если он есть)
	double sigma = 0;		// переменняа хранения сигмы
	double mu = 0;			// переменная дял хранения мат ожидания
	uchar is_on_divisor = 0;	// переменная равная 0, если пиксель лежит на разделители странице, иначе 255
	uchar **output = (char **)malloc(sizeof(char *) * y_seg_count);
	ocr_cells_net *result = (ocr_cells_net *)malloc(sizeof(ocr_cells_net));
	uchar *pix = grey->pix;
	coord begin, end;

	/* Инициализируем выходное изображение. */
	for(i = 0; i < y_seg_count; i++){
		output[i] = (char *)malloc(sizeof(char) * x_seg_count);
		for(j = 0; j < x_seg_count; j++){
			output[i][j] = 0;
		}
	}

	/* Находим индексы начала и конца разделителя страницы (если такой есть). */
	ocr_segm_page_vert_divisor(grey, &div_be, &div_end);
	printf("%dx%d\n", div_be, div_end);

	/* Находим рабочуюю область. */
	ocr_segm_get_area(grey, &begin, &end);

	x_cell = y_cell = 0;
	for(;;){
		/* Переход к новой строке сетки. */
		if(x_cell == x_seg_count){
			x_cell = 0;
			y_cell++;
		}

		/* Завершаем работу. */
		if(y_cell == y_seg_count)
			break;
		x_be = x_cell * segment_width;
		y_be = y_cell * segment_width;
		/* Если блок последний, просматриваем пиксели до края. */
		x_en = (x_cell == x_seg_count - 1) ? width : (x_cell + 1) * segment_width;
		y_en = (y_cell == y_seg_count - 1) ? height : (y_cell + 1) * segment_width;
		/* Вычисляем текущее число пикселей. */
		curr_cells_count = (y_en - y_be) * (x_en - x_be);
		sigma = 0.0;
		mu = 0.0;
		is_on_divisor = 0;
		for(i = x_be; i < x_en; i++){
			if(i >= div_be - 1 && i <= div_end + 1){
				is_on_divisor = 255;
				break;
			}
			for(j = y_be; j < y_en; j++){
				if(begin.x > i || end.x < i ||
				j > end.y || j < begin.y){
					sigma = 0;
					mu = 0;
					break;
				}
				curr = j * stride + i * rowpix;
				// Увеличиваем счетчик черных пикселей
				mu += pix[curr];
				sigma += pix[curr] * pix[curr];
			}
		}

		mu /= CR_BLACK * curr_cells_count;
		sigma /= CR_BLACK * curr_cells_count;
		sigma -= mu * mu;
		sigma = sqrt(sigma);

		//printf("%.2f ", sigma);
		/* Если сигма проходит по порогу, помечаем клетку как черную. */
		if(sigma >= sigma_threshold/* && is_on_divisor != 255*/)
			output[y_cell][x_cell] = CR_BLACK;

		x_cell++;
	}

	/* Сохраняем полученные настройки. */
	result->height = y_seg_count;
	result->cell_width = segment_width;
	result->width = x_seg_count;
	result->net = output;
	result->comp_count = 0;

	return result;
}
