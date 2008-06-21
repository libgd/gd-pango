/*
  +----------------------------------------------------------------------+
  | GD-Pango                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2007 Pierre-Alain Joye                            |
  +----------------------------------------------------------------------+
  | This source file is subject to the New BSD license, That is bundled  |
  | with this package in the file LICENSE.NEWBSD, and is available       |
  | through the world-wide-web at                                        |
  | http://www.opensource.org/licenses/bsd-license.php                   |
  | If you did not receive a copy of the new BSDlicense and are unable   |
  | to obtain it through the world-wide-web, please send a note to       |
  | pajoye@php.net so we can mail you a copy immediately.                |
  +----------------------------------------------------------------------+
  | Authors: Pierre-A. Joye <pierre@php.net>                             |
  +----------------------------------------------------------------------+
*/
/* $Id$ */

#include <pango/pango.h>
#include <pango/pangoft2.h>
#include <gd.h>
#include <gd_pango.h>

char *readFile(const char *filename)
{
	long file_size;
	char *text;
	FILE *file = fopen(filename, "rb");

	if(!file) {
		printf("Cannot open file <%s>\n", filename);
		exit(1);
	}

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	if (file_size == -1) {
		printf("Cannot tell file <%s>\n", filename);
		exit(2);
	}
	fseek(file, 0, SEEK_SET);
	text = (char *)malloc(file_size + 1);
	if (!text) {
		printf("Cannot allocate\n");
		exit(3);
	}
	fread(text, file_size, 1, file);
	text[file_size] = '\0';

	return text;
}

int main(int argc, char *argv[])
{
	gdImagePtr im;
	char *text;
	FILE *fp;

	int w,h;
	int margin_x, margin_y;
	gdPangoContext *context;
	gdPangoColors default_colors;
	PangoContext *pangocontext;
	PangoMatrix rotated_matrix = PANGO_MATRIX_INIT;
	PangoLayout *layout;

	gdPangoInit();

	default_colors.fg = gdTrueColorAlpha(0, 255, 255, 0);
	default_colors.bg = gdTrueColorAlpha(255, 255, 255, 0);
	default_colors.alpha = 0;
/*
	text = readFile("russian.txt");
	text = readFile("arabic.txt");
	text = readFile("hebrew.txt");
	text = readFile("german.txt");
 	text = readFile("english.txt");
	text = readFile("japanese.txt");
*/

	text = readFile("english.txt");

	context = gdPangoCreateContext();

	gdPangoSetDpi(context, 96, 96);
	gdPangoSetMinimumSize(context, 800, 200);

	gdPangoSetDefaultColor(context, &default_colors);
	gdPangoSetMarkup(context, text, -1);
	free(text);

	pangocontext = gdPangoGetPangoContext(context);
	layout = gdPangoGetPangoLayout(context);
	pango_context_set_base_dir(pangocontext, PANGO_DIRECTION_LTR);


	w = gdPangoGetLayoutWidth(context);
	h = gdPangoGetLayoutHeight(context);
   margin_x  = 0;
   margin_y  = 0;
   im = gdImageCreateTrueColor(800, 800);
	gdPangoRenderTo(context, im, margin_x, margin_y);

	fp = fopen("c.png", "wb");
	gdImagePng(im, fp);
	fclose(fp);

	gdImageDestroy(im);

	context->angle = -30;

	if (context->angle != 0.0) {
		pango_matrix_rotate (&rotated_matrix, context->angle);
		pango_context_set_matrix(pangocontext, &rotated_matrix);
		pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
		pango_layout_context_changed (layout);
		context->matrix = &rotated_matrix;
	}

	/* Render to a new image, sized for a rotated text */
	im = gdPangoCreateSurfaceDraw(context);
	fp = fopen("d.png", "wb");
	gdImagePng(im, fp);
	fclose(fp);
	gdPangoFreeContext(context);
	gdImageDestroy(im);
	return 0;
}
