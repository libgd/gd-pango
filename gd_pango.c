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
 * @brief Pango support for GD
 * Rendering can be done on true color images and in all upcoming new
 * formats.
 *
 * @author Pierre-Alain Joye
 * @date   2006/12/19
 * $Id$
 */
/**
 * \mainpage
 * \section introduction Introduction
 * Pango is a library for laying out and rendering of text, with an
 * emphasis on internationalization. Pango can be used anywhere that text
 * layout is needed.
 *
 * gd-pango provides an interface to render text GD buffers keeping all
 * the flexibility of the Pango engine.
 *
 * The current version supports only the in-memory rendering buffer
 * (fontconfig). The other backends will follow shortly.
 *
 * \section get Getting Started
 * \subsection getlatest Get latest files
 * During the initial development phase gd-pango is available via cvs
 * only. The module name is "gd/gd-pango"
 * Check out the download page for the anonymous CVS instruction:
 * http://www.libgd.org/Downloads
 *
 * \section development Development
 * \subsection example first contact
 * \code
 * gdImagePtr im;
 * char *text;
 * FILE *fp;
 *
 * int w,h;
 * int margin_x, margin_y;
 * gdPangoContext *context;
 * gdPangoColors default_colors;
 * PangoLayout *pangolayout;
 *
 * default_colors.fg = gdTrueColorAlpha(0, 0, 255, 0);
 * default_colors.bg = gdTrueColorAlpha(255, 255, 255, 0);
 * default_colors.alpha = 0;
 * \endcode
 * russian, arabic, hebrew or german examples are in CVS or in
 * pango distributions
 * \code
 * text = readFile("arabic.txt");
 * context = gdPangoCreateContext();
 *
 * gdPangoSetMinimumSize(context, 300, 300);
 *
 * pangolayout = gdPangoGetPangoLayout(context);
 * pango_context_set_base_dir(pangolayout, PANGO_DIRECTION_LTR);
 *
 * gdPangoSetDefaultColor(context, &default_colors);
 *
 * gdPangoSetMarkup(context, text, -1);
 *
 * w = gdPangoGetLayoutWidth(context);
 * h = gdPangoGetLayoutHeight(context);
 * margin_x  = 1;
 * margin_y  = 10;
 * im = gdImageCreateTrueColor(640, 640);
 * gdPangoRenderTo(context, im, margin_x, margin_y);
 *
 * fp = fopen("c.png", "wb");
 * gdImagePng(im, fp);
 * fclose(fp);
 * gdImageDestroy(im);
 * gdPangoFreeContext(context);
 * \endcode
 */

#include <pango/pango.h>
#include <pango/pangoft2.h>
#include <gd.h>
#include "gd_pango.h"

//! non-zero if initialized
static int GD_PANGO_IS_INITIALIZED = 0;

static void gdPangoGetItemProperties (
    PangoItem *item,
    PangoUnderline *uline,
    gboolean *strikethrough,
    gint *rise,
    PangoColor *fg_color,
    gboolean *fg_set,
    PangoColor *bg_color,
    gboolean *bg_set,
    gboolean *shape_set,
    PangoRectangle *ink_rect,
    PangoRectangle *logical_rect)
{
	GSList *tmp_list = item->analysis.extra_attrs;

	if (strikethrough) {
		*strikethrough = FALSE;
	}

	if (fg_set) {
	  *fg_set = FALSE;
	}

	if (bg_set) {
		*bg_set = FALSE;
	}

	if (shape_set) {
		*shape_set = FALSE;
	}

	if (rise) {
		*rise = 0;
	}

	while (tmp_list) {
		PangoAttribute *attr = tmp_list->data;

		switch (attr->klass->type) {
		case PANGO_ATTR_UNDERLINE:
			if (uline) {
				*uline = ((PangoAttrInt *)attr)->value;
			}
			break;

		case PANGO_ATTR_STRIKETHROUGH:
			if (strikethrough) {
				*strikethrough = ((PangoAttrInt *)attr)->value;
			}
			break;

		case PANGO_ATTR_FOREGROUND:
			if (fg_color) {
			*fg_color = ((PangoAttrColor *)attr)->color;
			}
			if (fg_set) {
			*fg_set = TRUE;
			}
			break;

		case PANGO_ATTR_BACKGROUND:
			if (bg_color) {
				*bg_color = ((PangoAttrColor *)attr)->color;
			}
			if (bg_set) {
				*bg_set = TRUE;
			}
			break;

		case PANGO_ATTR_SHAPE:
			if (shape_set) {
				*shape_set = TRUE;
			}
			if (logical_rect) {
				*logical_rect = ((PangoAttrShape *)attr)->logical_rect;
			}
			if (ink_rect) {
			*ink_rect = ((PangoAttrShape *)attr)->ink_rect;
			}
			break;

		case PANGO_ATTR_RISE:
			if (rise) {
				*rise = ((PangoAttrInt *)attr)->value;
			}
			break;

		default:
			break;
		}
		tmp_list = tmp_list->next;
	}
}

static FT_Bitmap * gdPangocreateFTBitmap(int width, int height)
{
	FT_Bitmap *bitmap;
	guchar *buf;

	bitmap = malloc(sizeof(FT_Bitmap));
	if (!bitmap) {
		return NULL;
	}

	bitmap->width = width;
	bitmap->rows = height;
	bitmap->pitch = (width + 3) & ~3;
	bitmap->num_grays = 256;
	bitmap->pixel_mode = FT_PIXEL_MODE_GRAY;
	buf = calloc (bitmap->pitch * bitmap->rows, sizeof(guchar));
	bitmap->buffer = buf;

	return bitmap;
}

static void gdPangofreeFTBitmap(FT_Bitmap *bitmap)
{
	if(bitmap) {
		free(bitmap->buffer);
		free(bitmap);
	}
}

static void gdPangoCleanFTBitmap(FT_Bitmap *bitmap)
{
	unsigned char *p = (unsigned char *)bitmap->buffer;
	int length = bitmap->pitch * bitmap->rows;

	memset(p, 0, length);
}

void gdPangoCopyFTBitmapToSurface(
	const FT_Bitmap *bitmap,
	gdImagePtr surface,
	const gdPangoColors *colors,
	gdRect *rect)
{
	int i;
	unsigned char *p_ft;
	int width = rect->w;
	int height = rect->h;
	int x = rect->x;
	int y = rect->y;
	int color_fg, alpha_blending_back;

	if(x + width > surface->sx) {
		width = surface->sx - x;
		if(width <= 0) {
			return;
		}
	}
	if(y + height > surface->sy) {
		height = surface->sy - y;
		if(height <= 0) {
			return;
		}
	}
	alpha_blending_back = surface->alphaBlendingFlag;
	gdImageAlphaBlending(surface, 1);
	p_ft = (unsigned char *)bitmap->buffer + (bitmap->pitch * y);
	color_fg = colors->fg;

	for(i = 0; i < height; i++) {
		int k;
		for(k = 0; k < width; k++) {
			int level, fg;
			if (p_ft[x + k]==0) {
				continue;
			}
			level = gdAlphaMax - (p_ft[x + k] >> 1);
			gdImageSetPixel(surface, x + k, y,  color_fg | (level<<24));
		}
		p_ft += bitmap->pitch;
		y++;
	}
	gdImageAlphaBlending(surface, alpha_blending_back);
}

static void gdPangoRenderGlyphString(
	gdPangoContext *context,
	gdImagePtr surface,
	gdPangoColors *colors,
	PangoFont *font,
	PangoGlyphString *glyphs,
	gdRect *rect,
	int baseline)
{
	pango_ft2_render(context->ft2bmp, font, glyphs,
					rect->x, rect->y + baseline);
	gdPangoCopyFTBitmapToSurface(context->ft2bmp, surface,
					colors, rect);
	gdPangoCleanFTBitmap(context->ft2bmp);
}

static void gdPangoDrawSpan(
	gdImagePtr surface,
	gdPangoColors *colors,
	int y,
	int start,
	int end)
{
	int color;
	int ix;

	if (y < 0 || y >= surface->sy) {
		return;
	}

	if (end <= 0 || start >= surface->sx) {
		return;
	}

	if (start < 0) {
		start = 0;
	}

	if (end >= surface->sx) {
		end = surface->sx;
	}
	color = colors->fg;
	for (ix = start; ix <= end; ix++) {
		gdImageSetPixel(surface, ix, y, color);
	}
}

static void gdPangoRenderLine(
	gdPangoContext *context,
	gdImagePtr surface,
	PangoLayoutLine *line,
	gint x,
	gint y,
	gint height,
	gint baseline)
{
	GSList *tmp_list = line->runs;
	PangoColor fg_color, bg_color;
	PangoRectangle logical_rect;
	PangoRectangle ink_rect;
	int x_off = 0;

	while (tmp_list) {
		gdPangoColors colors = context->default_colors;

		PangoUnderline uline = PANGO_UNDERLINE_NONE;
		gboolean strike, fg_set, bg_set, shape_set;
		gint rise, risen_y;
		PangoLayoutRun *run = tmp_list->data;
		gdRect d_rect;

		tmp_list = tmp_list->next;

		gdPangoGetItemProperties(run->item,
		&uline, &strike, &rise,
		&fg_color, &fg_set, &bg_color, &bg_set,
		&shape_set, &ink_rect, &logical_rect);

		risen_y = y + baseline - PANGO_PIXELS (rise);

		if(fg_set) {
			colors.fg = gdPangoColorToRGBA7888(fg_color);

			if(gdTrueColorGetAlpha(colors.bg) == 127) {
				colors.bg = gdPangoColorToRGBA7888(fg_color);
			}
		}

		if (bg_set) {
			colors.bg = gdPangoColorToRGBA7888(bg_color);
		}

		if(!shape_set) {
			if (uline == PANGO_UNDERLINE_NONE) {
				pango_glyph_string_extents (run->glyphs, run->item->analysis.font,
							 NULL, &logical_rect);
			} else {
				pango_glyph_string_extents (run->glyphs, run->item->analysis.font,
							 &ink_rect, &logical_rect);
			}

			d_rect.w = (int)PANGO_PIXELS(logical_rect.width);
			d_rect.h = (int)height;
			d_rect.x = (int)(x + PANGO_PIXELS (x_off));
			d_rect.y = (int)(risen_y - baseline);

			if((!context->ft2bmp) || d_rect.w + d_rect.x > context->ft2bmp->width
				|| d_rect.h + d_rect.y > context->ft2bmp->rows) {
				gdPangofreeFTBitmap(context->ft2bmp);
				context->ft2bmp = gdPangocreateFTBitmap(d_rect.w + d_rect.x, d_rect.h + d_rect.y);
			}

			gdPangoRenderGlyphString(context, surface, &colors,
				run->item->analysis.font, run->glyphs, &d_rect, baseline);
		}

		switch (uline) {
			case PANGO_UNDERLINE_NONE:
				break;

			case PANGO_UNDERLINE_DOUBLE:
				gdPangoDrawSpan(surface, &colors,
				risen_y + 4,
				x + PANGO_PIXELS (x_off + ink_rect.x),
				x + PANGO_PIXELS (x_off + ink_rect.x + ink_rect.width));
				/* Just do it twice */

			case PANGO_UNDERLINE_SINGLE:
				gdPangoDrawSpan(surface, &colors,
				risen_y + 2,
				x + PANGO_PIXELS (x_off + ink_rect.x),
				x + PANGO_PIXELS (x_off + ink_rect.x + ink_rect.width));
				break;

			case PANGO_UNDERLINE_ERROR:
			{
				int point_x;
				int counter = 0;
				int end_x = x + PANGO_PIXELS (x_off + ink_rect.x + ink_rect.width);

				for (point_x = x + PANGO_PIXELS (x_off + ink_rect.x) - 1;
				 point_x <= end_x;
				 point_x += 2) {
					if (counter) {
						gdPangoDrawSpan(surface, &colors, risen_y + 2, point_x,
									MIN (point_x + 1, end_x));
					} else {
						gdPangoDrawSpan(surface, &colors, risen_y + 3, point_x,
									MIN (point_x + 1, end_x));
					}
					counter = (counter + 1) % 2;
				}
			}
				break;

			case PANGO_UNDERLINE_LOW:
				gdPangoDrawSpan(surface, &colors,
				risen_y + PANGO_PIXELS (ink_rect.y + ink_rect.height),
				x + PANGO_PIXELS (x_off + ink_rect.x),
				x + PANGO_PIXELS (x_off + ink_rect.x + ink_rect.width));
				break;

			default:
				break;
		}

		if (strike) {
			gdPangoDrawSpan(surface, &colors,
			risen_y + PANGO_PIXELS (logical_rect.y + logical_rect.height / 2),
			x + PANGO_PIXELS (x_off + logical_rect.x),
			x + PANGO_PIXELS (x_off + logical_rect.x + logical_rect.width));
		}
		x_off += logical_rect.width;
	}
}
/* Public API */

/**
 * Initialize the Glib and Pango API.
 * This function must be called before using any other functions
 * in this library besides gdPangoIsInitialized.
 *
 * @return always GD_SUCCESS.
*/
int gdPangoInit() {
	g_type_init();
	GD_PANGO_IS_INITIALIZED = -1;
	return GD_SUCCESS;
}

/**
 * Return the intialization status
 * Tells whether gdPango has been already initialized or not.
 *
 * @return non-zero if it was not initialized, positive value if init.
 */
int gdPangoIsInitialized()
{
	return GD_PANGO_IS_INITIALIZED;
}

/**
 * Create a context which contains Pango objects.
 *
 * @return A pointer to the context as a gdPangoContext*.
 */
gdPangoContext* gdPangoCreateContext()
{
	gdPangoContext *context = g_malloc(sizeof(gdPangoContext));
	G_CONST_RETURN char *charset;

	context->font_map = pango_ft2_font_map_new ();
	pango_ft2_font_map_set_resolution (PANGO_FT2_FONT_MAP (context->font_map), GD_PANGO_DEFAULT_DPI, GD_PANGO_DEFAULT_DPI);
	context->context = pango_ft2_font_map_create_context (PANGO_FT2_FONT_MAP (context->font_map));

	g_get_charset(&charset);
	pango_context_set_language(context->context, pango_language_from_string(charset));
	pango_context_set_base_dir(context->context, PANGO_DIRECTION_LTR);
	pango_context_set_base_gravity(context->context, PANGO_GRAVITY_SOUTH);

	context->font_desc = pango_font_description_from_string(
		GD_PANGO_MAKE_FONT_NAME(GD_PANGO_DEFAULT_FONT_FAMILY, GD_PANGO_DEFAULT_FONT_SIZE));

	context->layout = pango_layout_new(context->context);

	/*
	   TODO: use other color formats to keep flexibility (once we support
	   more of them)
	 */
	context->default_colors.fg = 0xFFFFFF;
	context->default_colors.bg = 0x0;
	context->default_colors.alpha = 0x0;

	context->ft2bmp = NULL;

	context->min_height = 0;
	context->min_width = 0;

	return context;
}

/**
 * Free a context.
 *
 * @param *context [i/o] Context to be free
 */
void gdPangoFreeContext(gdPangoContext *context)
{
	gdPangofreeFTBitmap(context->ft2bmp);
	g_object_unref (context->layout);
	pango_font_description_free(context->font_desc);
	g_object_unref(context->context);
	g_object_unref(context->font_map);
	g_free(context);
}

/*!
    Create a surface and draw text on it.
    The size of surface is same as lauout size.

    @param *context [in] Context
    @return A newly created surface
*/
gdImagePtr gdPangoCreateSurfaceDraw(gdPangoContext *context)
{
	PangoRectangle logical_rect;
	gdImagePtr surface;
	int width, height;

	pango_layout_get_extents (context->layout, NULL, &logical_rect);
	width = PANGO_PIXELS (logical_rect.width);
	height = PANGO_PIXELS (logical_rect.height);
	if(width < context->min_width) {
		width = context->min_width;
	}
	if(height < context->min_height) {
		height = context->min_height;
	}

	surface = gdImageCreateTrueColor(width, height);
	gdPangoRenderTo(context, surface, 0, 0);
	return surface;
}

/**
 * Render the text to the given image
 *
 * Render the text to the given image. The (x,y) coordinate set the top
 * left corner of the text area.
 *
 * @param *context [in] Context
 * @param *surface [i/o] Surface to draw on it
 * @param x [in] X of left-top of drawing area
 * @param y [in] Y of left-top of drawing area
 */
void gdPangoRenderTo(gdPangoContext *context, gdImagePtr surface, int x, int y)
{
	PangoLayoutIter *iter;
	PangoRectangle logical_rect;
	int width, height;

	if(! surface) {
		printf("surface is NULL");
		return;
	}

	iter = pango_layout_get_iter (context->layout);

	pango_layout_get_extents (context->layout, NULL, &logical_rect);
	width = PANGO_PIXELS (logical_rect.width);
	height = PANGO_PIXELS (logical_rect.height);

	int ab = surface->alphaBlendingFlag;
	gdImageAlphaBlending(surface, 0);
	gdImageFilledRectangle(surface, 0,0, surface->sx, surface->sy, 0x0);
	gdImageAlphaBlending(surface, ab);
	gdImageColorTransparent(surface, 0xFF000011);

	if((! context->ft2bmp) || context->ft2bmp->width < width
		|| context->ft2bmp->rows < height) {
		gdPangofreeFTBitmap(context->ft2bmp);
		context->ft2bmp = gdPangocreateFTBitmap(width, height);
	}

	do {
		PangoLayoutLine *line;
		int baseline;

		line = pango_layout_iter_get_line (iter);

		pango_layout_iter_get_line_extents (iter, NULL, &logical_rect);
		baseline = pango_layout_iter_get_baseline (iter);

		gdPangoRenderLine(context, surface, line,
			x + PANGO_PIXELS (logical_rect.x),
			y + PANGO_PIXELS (logical_rect.y),
			PANGO_PIXELS (logical_rect.height),
			PANGO_PIXELS (baseline - logical_rect.y));
	} while (pango_layout_iter_next_line (iter));

	pango_layout_iter_free (iter);
}

/**
 * Specify minimum size of drawing rect.
 *
 * @param *context [i/o] Context
 * @param width [in] Width. -1 means no wrapping mode.
 * @param height [in] Height. zero/minus value means non-specified.
 */
void gdPangoSetMinimumSize(gdPangoContext *context, int width, int height)
{
	int pango_width;
	if(width > 0) {
		pango_width = width * PANGO_SCALE;
	} else {
		pango_width = -1;
	}

	pango_layout_set_width(context->layout, pango_width);

	context->min_width = width;
	context->min_height = height;
}

/**
 * Specify default color.
 *
 * Define the default foreground, background and alpha component
 *
 * @param *context	gdPangoContext context
 * @param *colors		a gdPangoColors ptr, defines fg, bgd or alpha default
 */
void gdPangoSetDefaultColor(gdPangoContext *context,
	const gdPangoColors  *colors)
{
	context->default_colors = *colors;
}

/**
 * Get layout width.
 *
 * @param *context gdPangoContext context
 * @return Width
 */
int gdPangoGetLayoutWidth(gdPangoContext *context)
{
	PangoRectangle logical_rect;

	pango_layout_get_extents (context->layout, NULL, &logical_rect);
	return PANGO_PIXELS(logical_rect.width);
}

/**
 * Get layout height.
 *
 * @param *context [in] Context
 * @return Height
 */
int gdPangoGetLayoutHeight(gdPangoContext *context)
{
	PangoRectangle logical_rect;

	pango_layout_get_extents (context->layout, NULL, &logical_rect);
	return PANGO_PIXELS (logical_rect.height);
}

/**
 * Set markup text to context.
 * Text must be utf-8.
 * Markup format is same as pango.
 *
 * @param *context 	gdPangoContex ptr
 * @param *markup		const char Markup text
 * @param length	 	Text length. -1 means NULL-terminated text.
 */
void gdPangoSetMarkup(gdPangoContext *context, const char *markup,
	const int length)
{
	pango_layout_set_markup(context->layout, markup, length);
	pango_layout_set_auto_dir(context->layout, TRUE);
	pango_layout_set_font_description(context->layout, context->font_desc);
}

/**
 * Set plain text to context.
 * Text must be utf-8.
 *
 * @param *context [i/o] Context
 * @param *text [in] Plain text
 * @param length [in] Text length. -1 means NULL-terminated text.
 */
void gdPangoSetText(gdPangoContext *context, const char *text,
	int length)
{
	pango_layout_set_attributes(context->layout, NULL);
	pango_layout_set_text(context->layout, text, length);
	pango_layout_set_auto_dir(context->layout, TRUE);
	pango_layout_set_alignment(context->layout, PANGO_ALIGN_LEFT);
	pango_layout_set_font_description(context->layout, context->font_desc);
}

/**
 * Set DPI to context.
 *
 * @param *context [i/o] Context
 * @param dpi_x [in] X dpi
 * @param dpi_y [in] Y dpi
 */
void gdPangoSetDpi(gdPangoContext *context,
	double dpi_x, double dpi_y)
{
   pango_ft2_font_map_set_resolution(PANGO_FT2_FONT_MAP(context->font_map),
		dpi_x, dpi_y);
}

/**
 * Set base direction to context.
 *
 * @param *context	Context
 * @param direction	Direction
 */
void gdPangoSetBaseDirection(gdPangoContext *context,
	PangoDirection direction)
{
	pango_context_set_base_dir (context->context, direction);
}

PangoFontMap* gdPangoGetPangoFontMap(gdPangoContext *context)
{
	return context->font_map;
}

PangoFontDescription* gdPangoGetPangoFontDescription(gdPangoContext *context)
{
    return context->font_desc;
}

/**
 * Returns the internal pango_context pointer
 *
 * This pointer can then be used directly with Pango. There is no need for
 * GD to duplicate the pango API.
 *
 * @param *context     PangoContext * Context
 */
PangoContext* gdPangoGetPangoContext(gdPangoContext *context)
{
	return context->context;
}

/**
 * Returns the internal pango_layout pointer
 *
 * This pointer can then be used directly with Pango. There is no need for
 * GD to duplicate the pango API.
 *
 * @param *context     gdPangoContext Context
 */
PangoLayout* gdPangoGetPangoLayout(gdPangoContext *context)
{
	return context->layout;
}
