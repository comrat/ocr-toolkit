
/* Нормализуем гистограмму, приводим к фиксированной ширине и высоте. */
void ocr_recog_normalize_hist(proj_hist *hist, int width, int height)
{
	int shift = 0;
	int length = 0;
	int curr = 0;
	int next = 0;
	int mod = 0;
	int i = 0;
	double *lines = NULL;
	double *colls = NULL;
	proj_hist *result = NULL;

	/* Если входные ширина и высота соотвествуют шаблонным, 
	возвращаем исходную. */
	if (width == TEMP_WIDTH && height == TEMP_HEIGHT) {
		return;
	}
	/* Выделяем память для выходных данных. */
	lines = (double *)malloc(sizeof(double) * TEMP_HEIGHT);
	colls = (double *)malloc(sizeof(double) * TEMP_WIDTH);
	result = (proj_hist *)malloc(sizeof(proj_hist));
	if (result == NULL || lines == NULL || colls == NULL) {
		printf("Проблемы при выделении памяти для нормированной области.\n");
		return;
	}
	/* Ссылаемся на полученные строки и столбцы. */
	result->lines = lines;
	result->colls = colls;
	result->width = TEMP_WIDTH;
	result->height = TEMP_HEIGHT;

	if (width >= TEMP_WIDTH) { 
		/* Если ширина входного изображения больше стандартного... */
		shift = round((double)width / TEMP_WIDTH);
		/* Берем каждый пиксель с отступом в x_shift из входной
		гистограммы. */
		for (i = 0; i < TEMP_WIDTH; i+=shift)
			colls[i] = hist->colls[i * shift];
	} else {
		/* Если ширина входного изображения меньше стандартного... */
		shift = round((double)TEMP_WIDTH / width);
		/* Интерполируем промежуточные значения. */
		for (i = 0; i < TEMP_WIDTH - 1; i++) {
			curr = i / shift;
			//printf("M:%d D:%d\n", mod, curr);
			if (curr + shift >= TEMP_WIDTH)
				next = TEMP_WIDTH - 1;
			else
				next = curr + shift;
			/* Запоминаем длину интервала. */
			length = next - curr;
			mod = i % length;
			/* Если х-координата кратна шагу, то...*/
			if (i % shift == 0) {
				/* Берем значение из входной гистограммы. */
				colls[i] = hist->colls[curr];
			} else {
				/* Интерполируем значения. */
				colls[i] = interpolate(hist->colls[curr], hist->colls[curr + 1], (double)mod / length); 
			}
		}
		/* Копируем последний элемент гистограммы. */
		result->colls[TEMP_WIDTH - 1] = hist->colls[width - 1];
	}
	if (height > TEMP_HEIGHT) {
		/* Если ширина входного изображения больше стандартного... */
		shift = round((double)height / TEMP_HEIGHT);
		/* Берем каждый пиксель с отступом в x_shift из входной
		гистограммы. */
		for (i = 0; i < TEMP_HEIGHT; i+=shift)
			lines[i] = hist->lines[i * shift];
	} else {
		/* Если ширина входного изображения меньше стандартного... */
		shift = round((double)TEMP_HEIGHT / height);
		/* Интерполируем промежуточные значения. */
		for (i = 0; i < TEMP_HEIGHT - 1; i++) {
			curr = i / shift;

			if (curr + shift >= TEMP_HEIGHT)
				next = TEMP_HEIGHT - 1;
			else
				next = curr + shift;
			/* Запоминаем длину интервала. */
			length = next - curr;
			mod = i % length;
			/* Если х-координата кратна шагу, то...*/
			if (i % shift == 0) {
				/* Берем значение из входной гистограммы. */
				lines[i] = hist->lines[curr];
			} else {
				/* Интерполируем значения. */
				lines[i] = interpolate(hist->lines[curr], hist->lines[curr + 1], (double)mod / length); 
			}
		}
		/* Копируем последний элемент гистограммы. */
		lines[TEMP_HEIGHT - 1] = hist->lines[height - 1];
	}
	hist = (proj_hist *)memcpy(hist, result, sizeof(proj_hist));
	hist->lines = (double *)memcpy(hist->lines, lines, sizeof(double) * TEMP_HEIGHT);
	hist->colls = (double *)memcpy(hist->colls, colls, sizeof(double) * TEMP_WIDTH);

	/*hist->lines = (double *)memcpy(hist->lines, result->lines, sizeof(double) * TEMP_HEIGHT);
	hist->colls = (double *)memcpy(hist->colls, result->colls, sizeof(double) * TEMP_WIDTH);
	*/
	//free(result);
}
