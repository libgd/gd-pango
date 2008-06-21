/* 
 * NOTE: prefer "gdtest.h" to <assert.h>, so
 * fix it as soon as possible.
 */
#include <assert.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>
#include "gd.h"
#include "gd_pango.h"

#define gdTestAssert assert
#define TEST(name) static void test_ ## name(void)
#define DO_TEST(name) test_ ## name()

TEST(gdPangoIsInitialized)
{
	int r;
	r = gdPangoIsInitialized();
	gdTestAssert(r == 0);
}

TEST(gdPangoInit)
{
	int r;
	r = gdPangoInit();
	gdTestAssert(r == GD_SUCCESS);
	r = gdPangoIsInitialized();
	gdTestAssert(r > 0);
}

TEST(gdPangoCreateContext)
{
	gdPangoContext *context;
	context = gdPangoCreateContext();
	gdTestAssert(context);
	gdPangoFreeContext(context);
}

#define test_gdPangoFreeContext test_gdPangoCreateContext

TEST(gdPangoRenderTo)
{
	gdPangoContext *context;
	gdImagePtr im, imx;
	context = gdPangoCreateContext();
	/* TODO */
	gdPangoSetText(context, "a", -1);
	im = gdImageCreateTrueColor(100, 120);
	imx = gdPangoRenderTo(context, im, 50, 50);
	gdTestAssert(im == imx);
	gdImageDestroy(im);
	gdPangoFreeContext(context);
}

TEST(gdPangoCreateSurfaceDraw)
{
	gdPangoContext *context;
	gdImagePtr im;
	context = gdPangoCreateContext();
	/* TODO */
	gdPangoSetText(context, "a", -1);
	im = gdPangoCreateSurfaceDraw(context);
	gdTestAssert(im);
	gdImageDestroy(im);
	gdPangoFreeContext(context);
}

TEST(gdPangoSetMinimumSize)
{
	gdPangoContext *context;
	context = gdPangoCreateContext();
	gdPangoSetMinimumSize(context, 200, 100);
	gdTestAssert(context->min_width == 200);
	gdTestAssert(context->min_height == 100);
	gdPangoFreeContext(context);
}

TEST(gdPangoSetDefaultColor)
{
	gdPangoContext *context;
	gdPangoColors color;
	color.fg = 0xAABBCC;
	color.bg = 0xFFFFFF;
	color.alpha = 0x0;
	context = gdPangoCreateContext();
	gdPangoSetDefaultColor(context, &color);
	gdTestAssert(context->default_colors.fg == 0xAABBCC);
	gdTestAssert(context->default_colors.bg == 0xFFFFFF);
	gdTestAssert(context->default_colors.alpha == 0x0);
	gdPangoFreeContext(context);
}

TEST(gdPangoGetLayoutWidth)
{
	gdPangoContext *context;
	int w;
	context = gdPangoCreateContext();
	gdPangoSetText(context, "", -1);
	w = gdPangoGetLayoutWidth(context);
	gdTestAssert(w == 0);
	gdPangoSetText(context, "a", -1);
	w = gdPangoGetLayoutWidth(context);
	gdTestAssert(w > 0);
	gdPangoFreeContext(context);
}

TEST(gdPangoGetLayoutHeight)
{
	gdPangoContext *context;
	int h;
	context = gdPangoCreateContext();
	gdPangoSetText(context, "a", -1);
	h = gdPangoGetLayoutHeight(context);
	gdTestAssert(h > 0);
	gdPangoFreeContext(context);
}

TEST(gdPangoSetMarkup)
{
	gdPangoContext *context;
	context = gdPangoCreateContext();
	gdPangoSetMarkup(context, "<u>underlined</u>", -1);
	gdPangoSetMarkup(context, "<u>under</u>lined", 12);
	gdPangoFreeContext(context);
}

TEST(gdPangoSetText)
{
	gdPangoContext *context;
	context = gdPangoCreateContext();
	gdPangoSetText(context, "it's a nice idea.", -1);
	gdPangoSetText(context, "it's a nice idea.", 6);
	gdPangoFreeContext(context);
}

TEST(gdPangoSetDpi)
{
	gdPangoContext *context;
	context = gdPangoCreateContext();
	gdPangoSetDpi(context, 72.0, 72.0);
	gdPangoFreeContext(context);
}

TEST(gdPangoSetBaseDirection)
{
	gdPangoContext *context;
	context = gdPangoCreateContext();
	gdPangoSetBaseDirection(context, PANGO_DIRECTION_RTL);
	gdTestAssert(pango_context_get_base_dir(context->context) == PANGO_DIRECTION_RTL);
	gdPangoFreeContext(context);
}

TEST(gdPangoSetPangoFontDescriptionFromFile)
{
	gdPangoContext *context;
	char *paths[] = {
		"/usr/local/lib/X11/fonts/bitstream-vera/Vera.ttf",
		"/usr/share/fonts/truetype/ttf-bitstream-vera/Vera.ttf",
		NULL,
	};
	int i;
	char *r;
	context = gdPangoCreateContext();
	for (i=0; paths[i]; i++) {
		r = gdPangoSetPangoFontDescriptionFromFile(context, paths[i], 12);
		if (r == NULL) {
			gdTestAssert(context->font_desc);
		}
	}
	r = gdPangoSetPangoFontDescriptionFromFile(context, "you have no file of such a name", 10);
	gdTestAssert(strcmp(r, "font not found") == 0);
	gdPangoFreeContext(context);
}

TEST(gdPangoGetPangoFontMap)
{
	gdPangoContext *context;
	PangoFontMap *font_map;
	context = gdPangoCreateContext();
	font_map = gdPangoGetPangoFontMap(context);
	gdTestAssert(font_map == context->font_map);
	gdPangoFreeContext(context);
}

TEST(gdPangoGetPangoFontDescription)
{
	gdPangoContext *context;
	PangoFontDescription *font_desc;
	context = gdPangoCreateContext();
	font_desc = gdPangoGetPangoFontDescription(context);
	gdTestAssert(font_desc == context->font_desc);
	gdPangoFreeContext(context);
}

TEST(gdPangoGetPangoContext)
{
	gdPangoContext *context;
	PangoContext *pango_context;
	context = gdPangoCreateContext();
	pango_context = gdPangoGetPangoContext(context);
	gdTestAssert(pango_context == context->context);
	gdPangoFreeContext(context);
}

TEST(gdPangoGetPangoLayout)
{
	gdPangoContext *context;
	PangoLayout *layout;
	context = gdPangoCreateContext();
	layout = gdPangoGetPangoLayout(context);
	gdTestAssert(layout == context->layout);
	gdPangoFreeContext(context);
}

TEST(gdImageStringPangoFT)
{
	gdPangoContext *context;
	context = gdPangoCreateContext();
	/* TODO */
	gdPangoFreeContext(context);
}

int main(int argc, char *argv[])
{
	DO_TEST(gdPangoIsInitialized);
	DO_TEST(gdPangoInit);
	if (!gdPangoIsInitialized()) gdPangoInit();
	DO_TEST(gdPangoCreateContext);
	DO_TEST(gdPangoFreeContext);
	DO_TEST(gdPangoRenderTo);
	DO_TEST(gdPangoCreateSurfaceDraw);
	DO_TEST(gdPangoSetMinimumSize);
	DO_TEST(gdPangoSetDefaultColor);
	DO_TEST(gdPangoGetLayoutWidth);
	DO_TEST(gdPangoGetLayoutHeight);
	DO_TEST(gdPangoSetMarkup);
	DO_TEST(gdPangoSetText);
	DO_TEST(gdPangoSetDpi);
	DO_TEST(gdPangoSetBaseDirection);
	DO_TEST(gdPangoSetPangoFontDescriptionFromFile);
	DO_TEST(gdPangoGetPangoFontMap);
	DO_TEST(gdPangoGetPangoFontDescription);
	DO_TEST(gdPangoGetPangoContext);
	DO_TEST(gdPangoGetPangoLayout);
	DO_TEST(gdImageStringPangoFT);
	return 0;
}
