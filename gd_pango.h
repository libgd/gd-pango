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
/**
 * @file
 * @brief Header file of gd-pango
 *
 * @author Pierre-Alain Joye
 * @date   2006/12/19
 * $Id$
 */

#ifndef GD_PANGO_H
#define GD_PANGO_H

#ifdef __cplusplus
extern "C" {
#endif

#define GD_SUCCESS 1
#define GD_FAILURE -1

#define GD_PANGO_DEFAULT_FONT_FAMILY Vera
#define GD_PANGO_DEFAULT_FONT_SIZE 10

/* TODO: Use GD DPI instead */
#define GD_PANGO_DEFAULT_DPI 96
#define _MAKE_FONT_NAME(family, size) #family " " #size
#define GD_PANGO_MAKE_FONT_NAME(family, size) _MAKE_FONT_NAME(family, size)

#define gdPangoColorToRGBA7888(pc) \
	gdTrueColorAlpha(pc.red >> 8, \
		pc.green >> 8, \
		pc.blue >> 8, \
		0)

/**
 * Defines a colors set. The foreground and background can be
 * defined using a integer value returned by gdTrueColor(r,g,b).
 * For example, you can use when you create a gdPangoContext.
 */
typedef struct gdPangoColors { /* colors definition */
	unsigned int fg;    /*!< Foreground color */
	unsigned int bg;    /*!< Background color */
	unsigned int alpha; /*!< General alpha component */
} gdPangoColors;

/**
 * Defines a gd Pango context. Use gdPangoCreateContext to create a GD
 * Context object. Different functions are provided to access its
 * values, do not access it directly.
 */
typedef struct gdPangoContext { /* GD Pango Context */
	PangoContext *context;
	PangoFontMap *font_map;
	PangoFontDescription *font_desc;
	PangoMatrix *matrix;
	PangoLayout *layout;
	FT_Bitmap *ft2bmp;
	gdPangoColors default_colors;
	int min_width;
	int min_height;
	double angle;
} gdPangoContext;

extern int gdPangoInit(void);
extern int gdPangoIsInitialized(void);
extern gdPangoContext* gdPangoCreateContext(void);
extern void gdPangoFreeContext(gdPangoContext *context);

extern gdImagePtr gdPangoCreateSurfaceDraw(
	gdPangoContext *context);

extern gdImagePtr gdPangoRenderTo(
	gdPangoContext *context,
	gdImagePtr surface,
	int x, int y);

extern void gdPangoSetDpi(
	gdPangoContext *context,
	double dpi_x, double dpi_y);

extern void gdPangoSetMinimumSize(
	gdPangoContext *context,
	int width, int height);

extern void gdPangoSetDefaultColor(
	gdPangoContext *context,
	const gdPangoColors *colors);

extern int gdPangoGetLayoutWidth(
	gdPangoContext *context);

extern int gdPangoGetLayoutHeight(
	gdPangoContext *context);

extern void gdPangoSetMarkup(
	gdPangoContext *context,
	const char *markup,
	const int length);

extern void gdPangoSetText(
	gdPangoContext *context,
	const char *markup,
	int length);

extern  void gdPangoSetBaseDirection(
	gdPangoContext *context, PangoDirection pango_dir);

extern char *gdPangoSetPangoFontDescriptionFromFile(
	gdPangoContext *context, const char *fontlist, double ptsize);

#ifdef __FT2_BUILD_UNIX_H__

extern void gdPangoCopyFTBitmapToSurface(
	const FT_Bitmap *bitmap,
	gdImagePtr surface,
	const gdPangoColors *colors,
	gdRect *rect);

extern char *gdImageStringPangoFT(
	gdImagePtr im,
	int *brect,
	int fg,
	char *fontlist,
	double ptsize,
	double angle,
	int x,
	int y,
	char *string);
#endif	/* __FT2_BUILD_UNIX_H__ */

#ifdef __PANGO_H__

extern PangoFontMap* gdPangoGetPangoFontMap(gdPangoContext *context);
extern PangoFontDescription* gdPangoGetPangoFontDescription(gdPangoContext *context);
extern PangoLayout* gdPangoGetPangoLayout(gdPangoContext *context);
extern PangoContext* gdPangoGetPangoContext(gdPangoContext *context);
#endif /* __PANGO_H__ */

#ifdef __cplusplus
}
#endif

#endif	/* GD_PANGO_H */
