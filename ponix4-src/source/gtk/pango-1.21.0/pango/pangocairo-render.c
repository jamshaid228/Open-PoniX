/* Pango
 * pangocairo-render.c: Rendering routines to Cairo surfaces
 *
 * Copyright (C) 2004 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <math.h>

#include "pangocairo-private.h"

typedef struct _PangoCairoRendererClass PangoCairoRendererClass;

#define PANGO_CAIRO_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_CAIRO_RENDERER, PangoCairoRendererClass))
#define PANGO_IS_CAIRO_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_CAIRO_RENDERER))
#define PANGO_CAIRO_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_CAIRO_RENDERER, PangoCairoRendererClass))

struct _PangoCairoRenderer
{
  PangoRenderer parent_instance;

  cairo_t *cr;
  gboolean do_path;
  double x_offset, y_offset;

  /* house-keeping options */
  gboolean is_cached_renderer;
  gboolean cr_had_current_point;
};

struct _PangoCairoRendererClass
{
  PangoRendererClass parent_class;
};

G_DEFINE_TYPE (PangoCairoRenderer, pango_cairo_renderer, PANGO_TYPE_RENDERER)

static void
set_color (PangoCairoRenderer *crenderer,
	   PangoRenderPart     part)
{
  PangoColor *color = pango_renderer_get_color ((PangoRenderer *) (crenderer), part);

  if (color)
    cairo_set_source_rgb (crenderer->cr,
			  color->red / 65535.,
			  color->green / 65535.,
			  color->blue / 65535.);
}

/* note: modifies crenderer->cr without doing cairo_save/restore() */
static void
_pango_cairo_renderer_draw_frame (PangoCairoRenderer *crenderer,
				  double              x,
				  double              y,
				  double              width,
				  double              height,
				  double              line_width,
				  gboolean            invalid)
{
  cairo_t *cr = crenderer->cr;

  if (crenderer->do_path)
    {
      double d2 = line_width * .5, d = line_width;

      /* we draw an outer box in one winding direction and an inner one in the
       * opposite direction.  This works for both cairo windings rules.
       *
       * what we really want is cairo_stroke_to_path(), but that's not
       * implemented in cairo yet.
       */

      /* outer */
      cairo_rectangle (cr, x-d2, y-d2, width+d, height+d);

      /* inner */
      if (invalid)
        {
	  /* delicacies of computing the joint... this is REALLY slow */

	  double alpha, tan_alpha2, cos_alpha;
	  double sx, sy;

	  alpha = atan2 (height, width);

	  tan_alpha2 = tan (alpha * .5);
	  if (tan_alpha2 < 1e-5 || (sx = d2 / tan_alpha2, 2. * sx > width - d))
	    sx = (width - d) * .5;

	  cos_alpha = cos (alpha);
	  if (cos_alpha < 1e-5 || (sy = d2 / cos_alpha, 2. * sy > height - d))
	    sy = (height - d) * .5;

	  /* top triangle */
	  cairo_new_sub_path (cr);
	  cairo_line_to (cr, x+width-sx, y+d2);
	  cairo_line_to (cr, x+sx, y+d2);
	  cairo_line_to (cr, x+.5*width, y+.5*height-sy);
	  cairo_close_path (cr);

	  /* bottom triangle */
	  cairo_new_sub_path (cr);
	  cairo_line_to (cr, x+width-sx, y+height-d2);
	  cairo_line_to (cr, x+.5*width, y+.5*height+sy);
	  cairo_line_to (cr, x+sx, y+height-d2);
	  cairo_close_path (cr);


	  alpha = G_PI_2 - alpha;
	  tan_alpha2 = tan (alpha * .5);
	  if (tan_alpha2 < 1e-5 || (sy = d2 / tan_alpha2, 2. * sy > height - d))
	    sy = (width - d) * .5;

	  cos_alpha = cos (alpha);
	  if (cos_alpha < 1e-5 || (sx = d2 / cos_alpha, 2. * sx > width - d))
	    sx = (width - d) * .5;

	  /* left triangle */
	  cairo_new_sub_path (cr);
	  cairo_line_to (cr, x+d2, y+sy);
	  cairo_line_to (cr, x+d2, y+height-sy);
	  cairo_line_to (cr, x+.5*width-sx, y+.5*height);
	  cairo_close_path (cr);

	  /* right triangle */
	  cairo_new_sub_path (cr);
	  cairo_line_to (cr, x+width-d2, y+sy);
	  cairo_line_to (cr, x+.5*width+sx, y+.5*height);
	  cairo_line_to (cr, x+width-d2, y+height-sy);
	  cairo_close_path (cr);
	}
      else
	cairo_rectangle (cr, x+width-d2, y+d2, - (width-d), height-d);
    }
  else
    {
      cairo_rectangle (cr, x, y, width, height);

      if (invalid)
        {
	  /* draw an X */

	  cairo_new_sub_path (cr);
	  cairo_move_to (cr, x, y);
	  cairo_rel_line_to (cr, width, height);

	  cairo_new_sub_path (cr);
	  cairo_move_to (cr, x + width, y);
	  cairo_rel_line_to (cr, -width, height);

	  cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	}

      cairo_set_line_width (cr, line_width);
      cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
      cairo_set_miter_limit (cr, 2.);
      cairo_stroke (cr);
    }
}

static void
_pango_cairo_renderer_draw_box_glyph (PangoCairoRenderer *crenderer,
				      PangoGlyphInfo     *gi,
				      double              cx,
				      double              cy,
				      gboolean            invalid)
{
  cairo_save (crenderer->cr);

  _pango_cairo_renderer_draw_frame (crenderer,
				    cx + 1.5,
				    cy + 1.5 - PANGO_UNKNOWN_GLYPH_HEIGHT,
				    (double)gi->geometry.width / PANGO_SCALE - 3.0,
				    PANGO_UNKNOWN_GLYPH_HEIGHT - 3.0,
				    1.0,
				    invalid);

  cairo_restore (crenderer->cr);
}

static void
_pango_cairo_renderer_draw_unknown_glyph (PangoCairoRenderer *crenderer,
					  PangoFont          *font,
					  PangoGlyphInfo     *gi,
					  double              cx,
					  double              cy)
{
  char buf[7];
  double x0, y0;
  int row, col;
  int rows, cols;
  char hexbox_string[2] = {0, 0};
  PangoCairoFontHexBoxInfo *hbi;
  gunichar ch;
  gboolean invalid_input;

  cairo_save (crenderer->cr);

  ch = gi->glyph & ~PANGO_GLYPH_UNKNOWN_FLAG;
  invalid_input = G_UNLIKELY (gi->glyph == PANGO_GLYPH_INVALID_INPUT || ch > 0x10FFFF);

  hbi = _pango_cairo_font_get_hex_box_info ((PangoCairoFont *)font);
  if (!hbi || !_pango_cairo_font_install ((PangoFont *)(hbi->font), crenderer->cr))
    {
      _pango_cairo_renderer_draw_box_glyph (crenderer, gi, cx, cy, invalid_input);
      goto done;
    }

  rows = hbi->rows;
  if (G_UNLIKELY (invalid_input))
    {
      cols = 1;
    }
  else
    {
      cols = (ch > 0xffff ? 6 : 4) / rows;
      g_snprintf (buf, sizeof(buf), (ch > 0xffff) ? "%06X" : "%04X", ch);
    }

  _pango_cairo_renderer_draw_frame (crenderer,
				    cx + hbi->pad_x * 1.5,
				    cy + hbi->box_descent - hbi->box_height + hbi->pad_y * 0.5,
				    (double)gi->geometry.width / PANGO_SCALE - 3 * hbi->pad_x,
				    (hbi->box_height - hbi->pad_y),
				    hbi->line_width,
				    invalid_input);

  if (invalid_input)
    goto done;

  x0 = cx + hbi->pad_x * 3.0;
  y0 = cy + hbi->box_descent - hbi->pad_y * 2;

  for (row = 0; row < rows; row++)
    {
      double y = y0 - (rows - 1 - row) * (hbi->digit_height + hbi->pad_y);
      for (col = 0; col < cols; col++)
	{
	  double x = x0 + col * (hbi->digit_width + hbi->pad_x);

	  cairo_move_to (crenderer->cr, x, y);

	  hexbox_string[0] = buf[row * cols + col];

	  if (crenderer->do_path)
	      cairo_text_path (crenderer->cr, hexbox_string);
	  else
	      cairo_show_text (crenderer->cr, hexbox_string);
	}
    }

done:
  cairo_restore (crenderer->cr);
}

static void
pango_cairo_renderer_draw_glyphs (PangoRenderer     *renderer,
				  PangoFont         *font,
				  PangoGlyphString  *glyphs,
				  int                x,
				  int                y)
{
  PangoCairoRenderer *crenderer = (PangoCairoRenderer *) (renderer);

  /* cairo_glyph_t is 24 bytes */
#define MAX_STACK 40

  int i, count;
  int x_position = 0;
  cairo_glyph_t *cairo_glyphs;
  cairo_glyph_t stack_glyphs[MAX_STACK];
  double base_x = crenderer->x_offset + (double)x / PANGO_SCALE;
  double base_y = crenderer->y_offset + (double)y / PANGO_SCALE;

  cairo_save (crenderer->cr);
  if (!crenderer->do_path)
    set_color (crenderer, PANGO_RENDER_PART_FOREGROUND);

  if (!_pango_cairo_font_install (font, crenderer->cr))
    {
      for (i = 0; i < glyphs->num_glyphs; i++)
	{
	  PangoGlyphInfo *gi = &glyphs->glyphs[i];

	  if (gi->glyph != PANGO_GLYPH_EMPTY)
	    {
	      double cx = base_x + (double)(x_position + gi->geometry.x_offset) / PANGO_SCALE;
	      double cy = gi->geometry.y_offset == 0 ?
			  base_y :
			  base_y + (double)(gi->geometry.y_offset) / PANGO_SCALE;

	      _pango_cairo_renderer_draw_unknown_glyph (crenderer, font, gi, cx, cy);
	    }
	  x_position += gi->geometry.width;
	}

      goto done;
    }

  if (glyphs->num_glyphs > MAX_STACK)
    cairo_glyphs = g_new (cairo_glyph_t, glyphs->num_glyphs);
  else
    cairo_glyphs = stack_glyphs;

  count = 0;
  for (i = 0; i < glyphs->num_glyphs; i++)
    {
      PangoGlyphInfo *gi = &glyphs->glyphs[i];

      if (gi->glyph != PANGO_GLYPH_EMPTY)
	{
	  double cx = base_x + (double)(x_position + gi->geometry.x_offset) / PANGO_SCALE;
	  double cy = gi->geometry.y_offset == 0 ?
		      base_y :
		      base_y + (double)(gi->geometry.y_offset) / PANGO_SCALE;

	  if (gi->glyph & PANGO_GLYPH_UNKNOWN_FLAG)
	    _pango_cairo_renderer_draw_unknown_glyph (crenderer, font, gi, cx, cy);
	  else
	    {
	      cairo_glyphs[count].index = gi->glyph;
	      cairo_glyphs[count].x = cx;
	      cairo_glyphs[count].y = cy;
	      count++;
	    }
	}
      x_position += gi->geometry.width;
    }

  if (crenderer->do_path)
    cairo_glyph_path (crenderer->cr, cairo_glyphs, count);
  else
    cairo_show_glyphs (crenderer->cr, cairo_glyphs, count);

  if (glyphs->num_glyphs > MAX_STACK)
    g_free (cairo_glyphs);

done:
  cairo_restore (crenderer->cr);

#undef MAX_STACK
}

static void
pango_cairo_renderer_draw_rectangle (PangoRenderer     *renderer,
				     PangoRenderPart    part,
				     int                x,
				     int                y,
				     int                width,
				     int                height)
{
  PangoCairoRenderer *crenderer = (PangoCairoRenderer *) (renderer);

  if (!crenderer->do_path)
    {
      cairo_save (crenderer->cr);

      set_color (crenderer, part);
    }

  cairo_rectangle (crenderer->cr,
		   crenderer->x_offset + (double)x / PANGO_SCALE,
		   crenderer->y_offset + (double)y / PANGO_SCALE,
		   (double)width / PANGO_SCALE, (double)height / PANGO_SCALE);

  if (!crenderer->do_path)
    {
      cairo_fill (crenderer->cr);

      cairo_restore (crenderer->cr);
    }
}

/* Draws an error underline that looks like one of:
 *              H       E                H
 *     /\      /\      /\        /\      /\               -
 *   A/  \    /  \    /  \     A/  \    /  \              |
 *    \   \  /    \  /   /D     \   \  /    \             |
 *     \   \/  C   \/   /        \   \/   C  \            | height = HEIGHT_SQUARES * square
 *      \      /\  F   /          \  F   /\   \           |
 *       \    /  \    /            \    /  \   \G         |
 *        \  /    \  /              \  /    \  /          |
 *         \/      \/                \/      \/           -
 *         B                         B
 *         |---|
 *       unit_width = (HEIGHT_SQUARES - 1) * square
 *
 * The x, y, width, height passed in give the desired bounding box;
 * x/width are adjusted to make the underline a integer number of units
 * wide.
 */
#define HEIGHT_SQUARES 2.5

static void
draw_error_underline (cairo_t *cr,
		      double   x,
		      double   y,
		      double   width,
		      double   height)
{
  double square = height / HEIGHT_SQUARES;
  double unit_width = (HEIGHT_SQUARES - 1) * square;
  int width_units = (width + unit_width / 2) / unit_width;
  double y_top, y_bottom;
  int i;

  x += (width - width_units * unit_width) / 2;
  width = width_units * unit_width;

  y_top = y;
  y_bottom = y + height;

  /* Bottom of squiggle */
  cairo_move_to (cr, x - square / 2, y_top + square / 2); /* A */
  for (i = 0; i < width_units; i += 2)
    {
      double x_middle = x + (i + 1) * unit_width;
      double x_right = x + (i + 2) * unit_width;

      cairo_line_to (cr, x_middle, y_bottom); /* B */

      if (i + 1 == width_units)
	/* Nothing */;
      else if (i + 2 == width_units)
	cairo_line_to (cr, x_right + square / 2, y_top + square / 2); /* D */
      else
	cairo_line_to (cr, x_right, y_top + square); /* C */
    }

  /* Top of squiggle */
  for (i -= 2; i >= 0; i -= 2)
    {
      double x_left = x + i * unit_width;
      double x_middle = x + (i + 1) * unit_width;
      double x_right = x + (i + 2) * unit_width;

      if (i + 1 == width_units)
	cairo_line_to (cr, x_middle + square / 2, y_bottom - square / 2); /* G */
      else {
	if (i + 2 == width_units)
	  cairo_line_to (cr, x_right, y_top); /* E */
	cairo_line_to (cr, x_middle, y_bottom - square); /* F */
      }

      cairo_line_to (cr, x_left, y_top);   /* H */
    }
}

static void
pango_cairo_renderer_draw_error_underline (PangoRenderer *renderer,
					   int            x,
					   int            y,
					   int            width,
					   int            height)
{
  PangoCairoRenderer *crenderer = (PangoCairoRenderer *) (renderer);
  cairo_t *cr = crenderer->cr;

  if (!crenderer->do_path)
    {
      cairo_save (cr);

      set_color (crenderer, PANGO_RENDER_PART_UNDERLINE);

      cairo_new_path (cr);
    }

  draw_error_underline (cr,
			crenderer->x_offset + (double)x / PANGO_SCALE,
			crenderer->y_offset + (double)y / PANGO_SCALE,
			(double)width / PANGO_SCALE, (double)height / PANGO_SCALE);

  if (!crenderer->do_path)
    {
      cairo_fill (cr);

      cairo_restore (cr);
    }
}

static void
pango_cairo_renderer_draw_shape (PangoRenderer  *renderer,
				 PangoAttrShape *attr,
				 int             x,
				 int             y)
{
  PangoCairoRenderer *crenderer = (PangoCairoRenderer *) (renderer);
  cairo_t *cr = crenderer->cr;
  PangoLayout *layout;
  PangoCairoShapeRendererFunc shape_renderer;
  gpointer                    shape_renderer_data;
  double base_x, base_y;

  layout = pango_renderer_get_layout (renderer);

  if (!layout)
  	return;

  shape_renderer = pango_cairo_context_get_shape_renderer (pango_layout_get_context (layout),
							   &shape_renderer_data);

  if (!shape_renderer)
    return;

  base_x = crenderer->x_offset + (double)x / PANGO_SCALE;
  base_y = crenderer->y_offset + (double)y / PANGO_SCALE;

  cairo_save (cr);
  if (!crenderer->do_path)
    set_color (crenderer, PANGO_RENDER_PART_FOREGROUND);

  cairo_move_to (cr, base_x, base_y);

  shape_renderer (cr, attr, crenderer->do_path, shape_renderer_data);

  cairo_restore (cr);
}

static void
pango_cairo_renderer_init (PangoCairoRenderer *renderer)
{
}

static void
pango_cairo_renderer_class_init (PangoCairoRendererClass *klass)
{
  PangoRendererClass *renderer_class = PANGO_RENDERER_CLASS (klass);

  renderer_class->draw_glyphs = pango_cairo_renderer_draw_glyphs;
  renderer_class->draw_rectangle = pango_cairo_renderer_draw_rectangle;
  renderer_class->draw_error_underline = pango_cairo_renderer_draw_error_underline;
  renderer_class->draw_shape = pango_cairo_renderer_draw_shape;
}

static PangoCairoRenderer *cached_renderer = NULL;
G_LOCK_DEFINE_STATIC (cached_renderer);

static PangoCairoRenderer *
acquire_renderer (void)
{
  PangoCairoRenderer *renderer;

  if (G_LIKELY (G_TRYLOCK (cached_renderer)))
    {
      if (G_UNLIKELY (!cached_renderer))
        {
	  cached_renderer = g_object_new (PANGO_TYPE_CAIRO_RENDERER, NULL);
	  cached_renderer->is_cached_renderer = TRUE;
	}

      renderer = cached_renderer;
    }
  else
    {
      renderer = g_object_new (PANGO_TYPE_CAIRO_RENDERER, NULL);
    }

  return renderer;
}

static void
release_renderer (PangoCairoRenderer *renderer)
{
  if (G_LIKELY (renderer->is_cached_renderer))
    {
      renderer->cr = NULL;
      renderer->do_path = FALSE;
      renderer->x_offset = 0.;
      renderer->y_offset = 0.;

      G_UNLOCK (cached_renderer);
    }
  else
    g_object_unref (renderer);
}

static void
save_current_point (PangoCairoRenderer *renderer)
{
  renderer->cr_had_current_point = cairo_has_current_point (renderer->cr);
  cairo_get_current_point (renderer->cr, &renderer->x_offset, &renderer->y_offset);
}

static void
restore_current_point (PangoCairoRenderer *renderer)
{
  if (renderer->cr_had_current_point)
    /* XXX should do cairo_set_current_point() when we have that function */
    cairo_move_to (renderer->cr, renderer->x_offset, renderer->y_offset);
  else
    cairo_new_sub_path (renderer->cr);
}


/* convenience wrappers using the default renderer */


static void
_pango_cairo_do_glyph_string (cairo_t          *cr,
			      PangoFont        *font,
			      PangoGlyphString *glyphs,
			      gboolean          do_path)
{
  PangoCairoRenderer *crenderer = acquire_renderer ();
  PangoRenderer *renderer = (PangoRenderer *) crenderer;

  crenderer->cr = cr;
  crenderer->do_path = do_path;
  save_current_point (crenderer);

  if (!do_path)
    {
      /* unset all part colors, since when drawing just a glyph string,
       * prepare_run() isn't called.
       */

      pango_renderer_activate (renderer);

      pango_renderer_set_color (renderer, PANGO_RENDER_PART_FOREGROUND, NULL);
      pango_renderer_set_color (renderer, PANGO_RENDER_PART_BACKGROUND, NULL);
      pango_renderer_set_color (renderer, PANGO_RENDER_PART_UNDERLINE, NULL);
      pango_renderer_set_color (renderer, PANGO_RENDER_PART_STRIKETHROUGH, NULL);
    }

  pango_renderer_draw_glyphs (renderer, font, glyphs, 0, 0);

  if (!do_path)
    {
      pango_renderer_deactivate (renderer);
    }

  restore_current_point (crenderer);

  release_renderer (crenderer);
}

static void
_pango_cairo_do_layout_line (cairo_t          *cr,
			     PangoLayoutLine  *line,
			     gboolean          do_path)
{
  PangoCairoRenderer *crenderer = acquire_renderer ();
  PangoRenderer *renderer = (PangoRenderer *) crenderer;

  crenderer->cr = cr;
  crenderer->do_path = do_path;
  save_current_point (crenderer);

  pango_renderer_draw_layout_line (renderer, line, 0, 0);

  restore_current_point (crenderer);

  release_renderer (crenderer);
}

static void
_pango_cairo_do_layout (cairo_t     *cr,
			PangoLayout *layout,
			gboolean     do_path)
{
  PangoCairoRenderer *crenderer = acquire_renderer ();
  PangoRenderer *renderer = (PangoRenderer *) crenderer;

  crenderer->cr = cr;
  crenderer->do_path = do_path;
  save_current_point (crenderer);

  pango_renderer_draw_layout (renderer, layout, 0, 0);

  restore_current_point (crenderer);

  release_renderer (crenderer);
}

static void
_pango_cairo_do_error_underline (cairo_t *cr,
				 double   x,
				 double   y,
				 double   width,
				 double   height,
				 gboolean do_path)
{
  /* We don't use a renderer here, for a simple reason:
   * the only renderer we can get is the default renderer, that
   * is all implemented here, so we shortcircuit and make our
   * life way easier.
   */

  if (!do_path)
    cairo_new_path (cr);

  draw_error_underline (cr, x, y, width, height);

  if (!do_path)
    cairo_fill (cr);
}


/* public wrapper of above to show or append path */


/**
 * pango_cairo_show_glyph_string:
 * @cr: a Cairo context
 * @font: a #PangoFont from a #PangoCairoFontMap
 * @glyphs: a #PangoGlyphString
 *
 * Draws the glyphs in @glyphs in the specified cairo context.
 * The origin of the glyphs (the left edge of the baseline) will
 * be drawn at the current point of the cairo context.
 *
 * Since: 1.10
 **/
void
pango_cairo_show_glyph_string (cairo_t          *cr,
			       PangoFont        *font,
			       PangoGlyphString *glyphs)
{
  g_return_if_fail (cr != NULL);
  g_return_if_fail (glyphs != NULL);

  _pango_cairo_do_glyph_string (cr, font, glyphs, FALSE);
}

/**
 * pango_cairo_show_layout_line:
 * @cr: a Cairo context
 * @line: a #PangoLayoutLine
 *
 * Draws a #PangoLayoutLine in the specified cairo context.
 * The origin of the glyphs (the left edge of the line) will
 * be drawn at the current point of the cairo context.
 *
 * Since: 1.10
 **/
void
pango_cairo_show_layout_line (cairo_t          *cr,
			      PangoLayoutLine  *line)
{
  g_return_if_fail (cr != NULL);
  g_return_if_fail (line != NULL);

  _pango_cairo_do_layout_line (cr, line, FALSE);
}

/**
 * pango_cairo_show_layout:
 * @cr: a Cairo context
 * @layout: a Pango layout
 *
 * Draws a #PangoLayoutLine in the specified cairo context.
 * The top-left corner of the #PangoLayout will be drawn
 * at the current point of the cairo context.
 *
 * Since: 1.10
 **/
void
pango_cairo_show_layout (cairo_t     *cr,
			 PangoLayout *layout)
{
  g_return_if_fail (cr != NULL);
  g_return_if_fail (PANGO_IS_LAYOUT (layout));

  _pango_cairo_do_layout (cr, layout, FALSE);
}

/**
 * pango_cairo_show_error_underline:
 * @cr: a Cairo context
 * @x: The X coordinate of one corner of the rectangle
 * @y: The Y coordinate of one corner of the rectangle
 * @width: Non-negative width of the rectangle
 * @height: Non-negative height of the rectangle
 *
 * Draw a squiggly line in the specified cairo context that approximately
 * covers the given rectangle in the style of an underline used to indicate a
 * spelling error.  (The width of the underline is rounded to an integer
 * number of up/down segments and the resulting rectangle is centered in the
 * original rectangle)
 *
 * Since: 1.14
 **/
void
pango_cairo_show_error_underline (cairo_t *cr,
				  double  x,
				  double  y,
				  double  width,
				  double  height)
{
  g_return_if_fail (cr != NULL);
  g_return_if_fail ((width >= 0) && (height >= 0));

  _pango_cairo_do_error_underline (cr, x, y, width, height, FALSE);
}

/**
 * pango_cairo_glyph_string_path
 * @cr: a Cairo context
 * @font: a #PangoFont from a #PangoCairoFontMap
 * @glyphs: a #PangoGlyphString
 *
 * Adds the glyphs in @glyphs to the current path in the specified
 * cairo context. The origin of the glyphs (the left edge of the baseline)
 * will be at the current point of the cairo context.
 *
 * Since: 1.10
 **/
void
pango_cairo_glyph_string_path (cairo_t          *cr,
			       PangoFont        *font,
			       PangoGlyphString *glyphs)
{
  g_return_if_fail (cr != NULL);
  g_return_if_fail (glyphs != NULL);

  _pango_cairo_do_glyph_string (cr, font, glyphs, TRUE);
}

/**
 * pango_cairo_layout_line_path:
 * @cr: a Cairo context
 * @line: a #PangoLayoutLine
 *
 * Adds the text in #PangoLayoutLine to the current path in the
 * specified cairo context.  The origin of the glyphs (the left edge
 * of the line) will be at the current point of the cairo context.
 *
 * Since: 1.10
 **/
void
pango_cairo_layout_line_path (cairo_t          *cr,
			      PangoLayoutLine  *line)
{
  g_return_if_fail (cr != NULL);
  g_return_if_fail (line != NULL);

  _pango_cairo_do_layout_line (cr, line, TRUE);
}

/**
 * pango_cairo_layout_path:
 * @cr: a Cairo context
 * @layout: a Pango layout
 *
 * Adds the text in a #PangoLayout to the current path in the
 * specified cairo context.  The top-left corner of the #PangoLayout
 * will be at the current point of the cairo context.
 *
 * Since: 1.10
 **/
void
pango_cairo_layout_path (cairo_t     *cr,
			 PangoLayout *layout)
{
  g_return_if_fail (cr != NULL);
  g_return_if_fail (PANGO_IS_LAYOUT (layout));

  _pango_cairo_do_layout (cr, layout, TRUE);
}

/**
 * pango_cairo_error_underline_path:
 * @cr: a Cairo context
 * @x: The X coordinate of one corner of the rectangle
 * @y: The Y coordinate of one corner of the rectangle
 * @width: Non-negative width of the rectangle
 * @height: Non-negative height of the rectangle
 *
 * Add a squiggly line to the current path in the specified cairo context that
 * approximately covers the given rectangle in the style of an underline used
 * to indicate a spelling error.  (The width of the underline is rounded to an
 * integer number of up/down segments and the resulting rectangle is centered
 * in the original rectangle)
 *
 * Since: 1.14
 **/
void
pango_cairo_error_underline_path (cairo_t *cr,
				  double   x,
				  double   y,
				  double   width,
				  double   height)
{
  g_return_if_fail (cr != NULL);
  g_return_if_fail ((width >= 0) && (height >= 0));

  _pango_cairo_do_error_underline (cr, x, y, width, height, TRUE);
}
