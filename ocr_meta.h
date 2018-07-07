#ifndef __OCR_META__
#define __OCR_META__

#define uchar unsigned char
#define CR_BLACK 255
#define CR_WHITE 0
#define WORD_SIZE 4

typedef enum
{
	BIN,
	GREY,
	RGB,
	ARGB
} pix_type;


/* Graphics component type. */
typedef enum
{
	PICTURE,
	NOISE,
	TEXT,
	FORMULA
} comp_type;


/* Image data struct. */
typedef struct
{
	uchar *pix;
	int width;
	int height;
	int stride;			// bytes count per line
	int bytes_for_pix;	// bytes count per pixel
} ocr_img_info;


/* Pixel coordinates struct. */
typedef struct
{
	int x;	// horiontal coordinate
	int y;	// vertical coordinate
} coord;


/* Image info grid each cell can contain info or be background. */
typedef struct
{
	uchar **net;	// grid
	int width;
	int height;
	int cell_width;
	int comp_count;
} ocr_cells_net;


/* Compound component struct. */
typedef struct
{
	coord *coords;	// cordinates array
	coord up_left;	// Up left point of diagonal of component coordinate
	coord bot_right;// bottom right point of diagonal of component coordinate
	coord center;	// center of rectangle area
	comp_type type;
	int size;		// component points length
	int width;
	int height;
} ocr_con_comp;


/* Text containing image area info struct. */
typedef struct
{
	uchar **pix;
	int x;
	int y;
	int width;
	int height;
} ocr_text_area;


/* Black&white pixels proportion info struct. */
typedef struct
{
	double *portion;	// part of black pixel in line
	int count;			// line length in pixels
} ocr_segm_stat_info;


/* Thresholded value part of histogram info struct. */
typedef struct
{
	int count;		// length of one of thresholded value interval
	uchar position;	// 0 if value is below threshod 255 otherwise
} ocr_hist_interval;


ocr_img_info *ocr_img_info_malloc(int width, int height, char type);

void ocr_img_info_free(ocr_img_info *img);

ocr_text_area *ocr_text_area_malloc(int width, int height, int x, int y);

void ocr_text_area_array_free(ocr_text_area **area, int elem_count);

#endif	/* __OCR_META__ */
