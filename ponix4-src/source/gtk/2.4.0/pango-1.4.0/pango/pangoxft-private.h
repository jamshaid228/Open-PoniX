/* Pango
 * pangox-private.h:
 *
 * Copyright (C) 2000 Red Hat Software
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

#ifndef __PANGOXFT_PRIVATE_H__
#define __PANGOXFT_PRIVATE_H__

#include <pangoxft.h>
#include <pango-ot.h>

G_BEGIN_DECLS

typedef struct _PangoXftFont    PangoXftFont;
typedef struct _PangoXftFontMap PangoXftFontMap;

struct _PangoXftFont
{
  PangoFcFont parent_instance;

  XftFont *xft_font;		    /* created on demand */
  PangoFont *mini_font;		    /* font used to display missing glyphs */

  guint16 mini_width;		    /* metrics for missing glyph drawing */
  guint16 mini_height;
  guint16 mini_pad; 
};

PangoXftFont *_pango_xft_font_new          (PangoXftFontMap  *xftfontmap,
					    FcPattern        *pattern);
void          _pango_xft_font_map_get_info (PangoFontMap     *fontmap,
					    Display         **display,
					    int              *screen);

G_END_DECLS

#endif /* __PANGOXFT_PRIVATE_H__ */
