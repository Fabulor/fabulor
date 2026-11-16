/* Exercise private cache identity without adding a production inspection API. */
#include <string.h>
#include "../../src/fe-gtk/xtext-background.c"

static void
test_cache_lifetime (void)
{
	FabulorXTextBackground *background = fabulor_xtext_background_new ();
	FabulorXTextGeometry geometry = { 4, 4 };
	XTextColor fallback = { 1.0, 0.0, 0.0, 1.0 };
	cairo_surface_t *source = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 4, 2);
	cairo_surface_t *output = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 8, 8);
	cairo_surface_t *previous;
	cairo_t *context = cairo_create (output);
	guint32 pixel;
	gint change;
	gint tile_x = 0, tile_y = 0, dim = 0;

	fabulor_xtext_background_set_surface (background, source);
	fabulor_xtext_background_begin_frame (background);
	fabulor_xtext_background_paint (background, context, &fallback,
		&geometry, 0, 0, 4, 4, tile_x, tile_y, dim);
	g_assert_nonnull (background->cache);
	previous = cairo_surface_reference (background->cache);
	fabulor_xtext_background_end_frame (background);
	fabulor_xtext_background_begin_frame (background);
	fabulor_xtext_background_paint (background, context, &fallback,
		&geometry, 1, 1, 2, 2, tile_x, tile_y, dim);
	g_assert_true (background->cache == previous);
	cairo_surface_destroy (previous);

	/* Each cache input must independently invalidate the retained image. */
	for (change = 0; change < 9; change++)
	{
		previous = cairo_surface_reference (background->cache);
		switch (change)
		{
		case 0: geometry.width++; break;
		case 1: geometry.height++; break;
		case 2: tile_x++; break;
		case 3: tile_y++; break;
		case 4: dim = 50; break;
		case 5: fallback.red = 0.5; break;
		case 6: fallback.green = 0.5; break;
		case 7: fallback.blue = 0.5; break;
		case 8: fallback.alpha = 0.5; break;
		}
		fabulor_xtext_background_paint (background, context, &fallback,
			&geometry, 0, 0, 4, 4, tile_x, tile_y, dim);
		g_assert_nonnull (background->cache);
		g_assert_true (background->cache != previous);
		cairo_surface_destroy (previous);
	}

	/* Reapplying a source invalidates even when its pointer is unchanged. */
	previous = cairo_surface_reference (background->cache);
	fabulor_xtext_background_set_surface (background, source);
	g_assert_null (background->cache);
	fabulor_xtext_background_paint (background, context, &fallback,
		&geometry, 0, 0, 4, 4, tile_x, tile_y, dim);
	g_assert_true (background->cache != previous);
	cairo_surface_destroy (previous);

	fabulor_xtext_background_set_surface (background, NULL);
	g_assert_null (background->cache);
	g_assert_false (fabulor_xtext_background_has_surface (background));
	fallback = (XTextColor) { 0.0, 1.0, 0.0, 1.0 };
	fabulor_xtext_background_paint (background, context, &fallback,
		&geometry, 0, 0, 4, 4, 0, 0, 0);
	cairo_surface_flush (output);
	memcpy (&pixel, cairo_image_surface_get_data (output), sizeof pixel);
	g_assert_cmphex (pixel, ==, 0xff00ff00U);
	cairo_destroy (context);
	cairo_surface_destroy (output);
	cairo_surface_destroy (source);
	fabulor_xtext_background_free (background);
}

int
main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/xtext/background/cache-lifetime", test_cache_lifetime);
	return g_test_run ();
}
