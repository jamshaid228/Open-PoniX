/* Pango
 * pangoxft-fontmap.c: Xft font handling
 *
 * Copyright (C) 2000-2003 Red Hat Software
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

#include <stdlib.h>
#include <string.h>

#include "pangofc-fontmap.h"
#include "pangoxft.h"
#include "pangoxft-private.h"

/* For XExtSetCloseDisplay */
#include <X11/Xlibint.h>

typedef struct _PangoXftFamily       PangoXftFamily;
typedef struct _PangoXftFontMapClass PangoXftFontMapClass;

#define PANGO_TYPE_XFT_FONT_MAP              (pango_xft_font_map_get_type ())
#define PANGO_XFT_FONT_MAP(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), PANGO_TYPE_XFT_FONT_MAP, PangoXftFontMap))
#define PANGO_XFT_IS_FONT_MAP(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), PANGO_TYPE_XFT_FONT_MAP))

struct _PangoXftFontMap
{
  PangoFcFontMap parent_instance;

  Display *display;
  int screen;
  
  /* Function to call on prepared patterns to do final
   * config tweaking.
   */
  PangoXftSubstituteFunc substitute_func;
  gpointer substitute_data;
  GDestroyNotify substitute_destroy;
};

struct _PangoXftFontMapClass
{
  PangoFcFontMapClass parent_class;
};

static void          pango_xft_font_map_class_init         (PangoXftFontMapClass *class);
static void          pango_xft_font_map_default_substitute (PangoFcFontMap       *fcfontmap,
							    FcPattern            *pattern);
static PangoFcFont * pango_xft_font_map_new_font           (PangoFcFontMap       *fcfontmap,
							    FcPattern            *pattern);
static void          pango_xft_font_map_finalize           (GObject              *object);

static GObjectClass *parent_class;

static GSList *fontmaps = NULL;

GType
pango_xft_font_map_get_type (void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
      {
        sizeof (PangoXftFontMapClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) pango_xft_font_map_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (PangoXftFontMap),
        0,              /* n_preallocs */
        (GInstanceInitFunc) NULL,
      };
      
      object_type = g_type_register_static (PANGO_TYPE_FC_FONT_MAP,
                                            "PangoXftFontMap",
                                            &object_info, 0);
    }
  
  return object_type;
}

static void
pango_xft_font_map_class_init (PangoXftFontMapClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  PangoFcFontMapClass *fcfontmap_class = PANGO_FC_FONT_MAP_CLASS (class);

  parent_class = g_type_class_peek_parent (class);

  gobject_class->finalize  = pango_xft_font_map_finalize;
  fcfontmap_class->default_substitute = pango_xft_font_map_default_substitute;
  fcfontmap_class->new_font = pango_xft_font_map_new_font;
}

static void
pango_xft_font_map_finalize (GObject *object)
{
  PangoXftFontMap *xftfontmap = PANGO_XFT_FONT_MAP (object);
  
  fontmaps = g_slist_remove (fontmaps, object);

  if (xftfontmap->substitute_destroy)
    xftfontmap->substitute_destroy (xftfontmap->substitute_data);

  parent_class->finalize (object);
}


static PangoFontMap *
pango_xft_find_font_map (Display *display,
			 int      screen)
{
  GSList *tmp_list;
  
  tmp_list = fontmaps;
  while (tmp_list)
    {
      PangoXftFontMap *xftfontmap = tmp_list->data;

      if (xftfontmap->display == display &&
	  xftfontmap->screen == screen) 
	return PANGO_FONT_MAP (xftfontmap);

      tmp_list = tmp_list->next;
    }

  return NULL;
}

/*
 * Hackery to set up notification when a Display is closed
 */
static GSList *registered_displays;

static int
close_display_cb (Display   *display,
		  XExtCodes *extcodes)
{
  GSList *tmp_list;
  
  tmp_list = fontmaps;
  while (tmp_list)
    {
      PangoXftFontMap *xftfontmap = tmp_list->data;
      tmp_list = tmp_list->next;

      if (xftfontmap->display == display)
	pango_xft_shutdown_display (display, xftfontmap->screen);
    }

  registered_displays = g_slist_remove (registered_displays, display);

  return 0;
}

static void
register_display (Display *display)
{
  XExtCodes *extcodes;
  GSList *tmp_list;

  for (tmp_list = registered_displays; tmp_list; tmp_list = tmp_list->next)
    {
      if (tmp_list->data == display)
	return;
    }

  registered_displays = g_slist_prepend (registered_displays, display);
    
  extcodes = XAddExtension (display);
  XESetCloseDisplay (display, extcodes->extension, close_display_cb);
}

/**
 * pango_xft_get_font_map:
 * @display: an X display
 * @screen: the screen number of a screen within @display
 * 
 * Returns the #PangoXftFontmap for the given display and screen.
 * The fontmap is owned by Pango and will be valid until
 * the display is closed.
 * 
 * Return value: a #PangoFontMap object, owned by Pango.
 *
 * Since: 1.2
 **/
PangoFontMap *
pango_xft_get_font_map (Display *display,
			int      screen)
{
  PangoFontMap *fontmap;
  PangoXftFontMap *xftfontmap;

  g_return_val_if_fail (display != NULL, NULL);
  
  fontmap = pango_xft_find_font_map (display, screen);
  if (fontmap)
    return fontmap;
  
  /* Make sure that the type system is initialized */
  g_type_init ();

  xftfontmap = (PangoXftFontMap *)g_object_new (PANGO_TYPE_XFT_FONT_MAP, NULL);
  
  xftfontmap->display = display;
  xftfontmap->screen = screen;

  register_display (display);

  fontmaps = g_slist_prepend (fontmaps, xftfontmap);

  return PANGO_FONT_MAP (xftfontmap);
}

/**
 * pango_xft_shutdown_display:
 * @display: an X display
 * @screen: the screen number of a screen within @display
 * 
 * Release any resources that have been cached for the
 * combination of @display and @screen.
 *
 * Since: 1.2
 **/
void
pango_xft_shutdown_display (Display *display,
			    int      screen)
{
  PangoFontMap *fontmap;
  
  fontmap = pango_xft_find_font_map (display, screen);
  if (fontmap)
    {
      PangoXftFontMap *xftfontmap = PANGO_XFT_FONT_MAP (fontmap);

      fontmaps = g_slist_remove (fontmaps, fontmap);
      pango_fc_font_map_shutdown (PANGO_FC_FONT_MAP (fontmap));
      
      xftfontmap->display = NULL;
      g_object_unref (fontmap);
    }
}  

/**
 * pango_xft_set_default_substitute:
 * @display: an X Display
 * @screen: the screen number of a screen within @display
 * @func: function to call to to do final config tweaking
 *        on #FcPattern objects.
 * @data: data to pass to @func
 * @notify: function to call when @data is no longer used.
 * 
 * Sets a function that will be called to do final configuration
 * substitution on a #FcPattern before it is used to load
 * the font. This function can be used to do things like set
 * hinting and antialiasing options.
 *
 * Since: 1.2
 **/
void
pango_xft_set_default_substitute (Display                *display,
				  int                     screen,
				  PangoXftSubstituteFunc  func,
				  gpointer                data,
				  GDestroyNotify          notify)
{
  PangoXftFontMap *xftfontmap = (PangoXftFontMap *)pango_xft_get_font_map (display, screen);
  
  if (xftfontmap->substitute_destroy)
    xftfontmap->substitute_destroy (xftfontmap->substitute_data);

  xftfontmap->substitute_func = func;
  xftfontmap->substitute_data = data;
  xftfontmap->substitute_destroy = notify;
  
  pango_fc_font_map_cache_clear (PANGO_FC_FONT_MAP (xftfontmap));
}

/**
 * pango_xft_substitute_changed:
 * @display: an X Display
 * @screen: the screen number of a screen within @display
 * 
 * Call this function any time the results of the
 * default substitution function set with
 * pango_xft_set_default_substitute() change.
 * That is, if your subsitution function will return different
 * results for the same input pattern, you must call this function.
 *
 * Since: 1.2
 **/
void
pango_xft_substitute_changed (Display *display,
			      int      screen)
{
  PangoXftFontMap *xftfontmap = (PangoXftFontMap *)pango_xft_get_font_map (display, screen);
  
  pango_fc_font_map_cache_clear (PANGO_FC_FONT_MAP (xftfontmap));
}

void
_pango_xft_font_map_get_info (PangoFontMap *fontmap,
			      Display     **display,
			      int          *screen)
{
  PangoXftFontMap *xftfontmap = (PangoXftFontMap *)fontmap;

  if (display)
    *display = xftfontmap->display;
  if (screen)
    *screen = xftfontmap->screen;
}

/**
 * pango_xft_get_context:
 * @display: an X display.
 * @screen: an X screen.
 *
 * Retrieves a #PangoContext appropriate for rendering with
 * Xft fonts on the given screen of the given display. 
 *
 * Return value: the new #PangoContext. 
 **/
PangoContext *
pango_xft_get_context (Display *display,
		       int      screen)
{
  PangoFontMap *fontmap;

  g_return_val_if_fail (display != NULL, NULL);

  fontmap = pango_xft_get_font_map (display, screen);
  return pango_fc_font_map_create_context (PANGO_FC_FONT_MAP (fontmap));
}

static void
pango_xft_font_map_default_substitute (PangoFcFontMap *fcfontmap,
				       FcPattern      *pattern)
{
  PangoXftFontMap *xftfontmap = PANGO_XFT_FONT_MAP (fcfontmap);
	
  FcConfigSubstitute (NULL, pattern, FcMatchPattern);
  if (xftfontmap->substitute_func)
    xftfontmap->substitute_func (pattern, xftfontmap->substitute_data);
  XftDefaultSubstitute (xftfontmap->display, xftfontmap->screen, pattern);
}

static PangoFcFont *
pango_xft_font_map_new_font (PangoFcFontMap  *fcfontmap,
			     FcPattern       *pattern)
{
  return (PangoFcFont *)_pango_xft_font_new (PANGO_XFT_FONT_MAP (fcfontmap), pattern);
}
