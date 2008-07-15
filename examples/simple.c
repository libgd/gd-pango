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

	gdPangoInit();

	default_colors.fg = gdTrueColorAlpha(0, 0, 255, 0);
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

	text = readFile("arabic.txt");

	context = gdPangoCreateContext();

	gdPangoSetDpi(context, 96, 96);
	gdPangoSetMinimumSize(context, 300, 200);

	pangocontext = gdPangoGetPangoContext(context);
	pango_context_set_base_dir(pangocontext, PANGO_DIRECTION_LTR);

	gdPangoSetDefaultColor(context, &default_colors);

	gdPangoSetMarkup(context, text, -1);
	free(text);

	w = gdPangoGetLayoutWidth(context);
	h = gdPangoGetLayoutHeight(context);
   margin_x  = 1;
   margin_y  = 10;
   im = gdImageCreateTrueColor(640, 640);
	gdPangoRenderTo(context, im, margin_x, margin_y);

	fp = fopen("a.png", "wb");
	gdImagePng(im, fp);
	fclose(fp);
	gdImageDestroy(im);

	/* Render to a new image, sized for text */
	im = gdPangoCreateSurfaceDraw(context);

	fp = fopen("b.png", "wb");
	gdImagePng(im, fp);
	fclose(fp);
	gdImageDestroy(im);

	gdPangoFreeContext(context);
	return 0;
}
