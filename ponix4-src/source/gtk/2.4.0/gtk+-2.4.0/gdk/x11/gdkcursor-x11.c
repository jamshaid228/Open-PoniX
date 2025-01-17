/* GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include <config.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif

#include "gdkprivate-x11.h"
#include "gdkcursor.h"
#include "gdkpixmap-x11.h"
#include "gdkx.h"
#include <gdk/gdkpixmap.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


/**
 * gdk_cursor_new_for_display:
 * @display: the #GdkDisplay for which the cursor will be created
 * @cursor_type: cursor to create
 * 
 * Creates a new cursor from the set of builtin cursors.
 * Some useful ones are:
 * <itemizedlist>
 * <listitem><para>
 *  <inlinegraphic format="PNG" fileref="right_ptr.png"></inlinegraphic> #GDK_RIGHT_PTR (right-facing arrow)
 * </para></listitem>
 * <listitem><para>
 *  <inlinegraphic format="PNG" fileref="crosshair.png"></inlinegraphic> #GDK_CROSSHAIR (crosshair)
 * </para></listitem>
 * <listitem><para>
 *  <inlinegraphic format="PNG" fileref="xterm.png"></inlinegraphic> #GDK_XTERM (I-beam)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="watch.png"></inlinegraphic> #GDK_WATCH (busy)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="fleur.png"></inlinegraphic> #GDK_FLEUR (for moving objects)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="hand1.png"></inlinegraphic> #GDK_HAND1 (a right-pointing hand)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="hand2.png"></inlinegraphic> #GDK_HAND2 (a left-pointing hand)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="left_side.png"></inlinegraphic> #GDK_LEFT_SIDE (resize left side)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="right_side.png"></inlinegraphic> #GDK_RIGHT_SIDE (resize right side)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="top_left_corner.png"></inlinegraphic> #GDK_TOP_LEFT_CORNER (resize northwest corner)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="top_right_corner.png"></inlinegraphic> #GDK_TOP_RIGHT_CORNER (resize northeast corner)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="bottom_left_corner.png"></inlinegraphic> #GDK_BOTTOM_LEFT_CORNER (resize southwest corner)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="bottom_right_corner.png"></inlinegraphic> #GDK_BOTTOM_RIGHT_CORNER (resize southeast corner)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="top_side.png"></inlinegraphic> #GDK_TOP_SIDE (resize top side)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="bottom_side.png"></inlinegraphic> #GDK_BOTTOM_SIDE (resize bottom side)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="sb_h_double_arrow.png"></inlinegraphic> #GDK_SB_H_DOUBLE_ARROW (move vertical splitter)
 * </para></listitem>
 * <listitem><para>
 * <inlinegraphic format="PNG" fileref="sb_v_double_arrow.png"></inlinegraphic> #GDK_SB_V_DOUBLE_ARROW (move horizontal splitter)
 * </para></listitem>
 * </itemizedlist>
 *
 * To make the cursor invisible, use gdk_cursor_new_from_pixmap() to create
 * a cursor with no pixels in it.
 * 
 * Return value: a new #GdkCursor
 *
 * Since: 2.2
 **/
GdkCursor*
gdk_cursor_new_for_display (GdkDisplay    *display,
			    GdkCursorType  cursor_type)
{
  GdkCursorPrivate *private;
  GdkCursor *cursor;
  Cursor xcursor;

  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);

  if (display->closed)
    xcursor = None;
  else
    xcursor = XCreateFontCursor (GDK_DISPLAY_XDISPLAY (display), cursor_type);
  
  private = g_new (GdkCursorPrivate, 1);
  private->display = display;
  private->xcursor = xcursor;
  cursor = (GdkCursor *) private;
  cursor->type = cursor_type;
  cursor->ref_count = 1;
  
  return cursor;
}

/**
 * gdk_cursor_new_from_pixmap:
 * @source: the pixmap specifying the cursor.
 * @mask: the pixmap specifying the mask, which must be the same size as 
 *    @source.
 * @fg: the foreground color, used for the bits in the source which are 1.
 *    The color does not have to be allocated first. 
 * @bg: the background color, used for the bits in the source which are 0.
 *    The color does not have to be allocated first.
 * @x: the horizontal offset of the 'hotspot' of the cursor. 
 * @y: the vertical offset of the 'hotspot' of the cursor.
 * 
 * Creates a new cursor from a given pixmap and mask. Both the pixmap and mask
 * must have a depth of 1 (i.e. each pixel has only 2 values - on or off).
 * The standard cursor size is 16 by 16 pixels. You can create a bitmap 
 * from inline data as in the below example.
 * 
 * <example><title>Creating a custom cursor</title>
 * <programlisting>
 * /<!-- -->* This data is in X bitmap format, and can be created with the 'bitmap'
 *    utility. *<!-- -->/
 * &num;define cursor1_width 16
 * &num;define cursor1_height 16
 * static unsigned char cursor1_bits[] = {
 *   0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20,
 *   0x82, 0x41, 0x41, 0x82, 0x41, 0x82, 0x82, 0x41, 0x04, 0x20, 0x08, 0x10,
 *   0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01};
 *  
 * static unsigned char cursor1mask_bits[] = {
 *   0x80, 0x01, 0xc0, 0x03, 0x60, 0x06, 0x30, 0x0c, 0x18, 0x18, 0x8c, 0x31,
 *   0xc6, 0x63, 0x63, 0xc6, 0x63, 0xc6, 0xc6, 0x63, 0x8c, 0x31, 0x18, 0x18,
 *   0x30, 0x0c, 0x60, 0x06, 0xc0, 0x03, 0x80, 0x01};
 *  
 *  
 *  GdkCursor *cursor;
 *  GdkPixmap *source, *mask;
 *  GdkColor fg = { 0, 65535, 0, 0 }; /<!-- -->* Red. *<!-- -->/
 *  GdkColor bg = { 0, 0, 0, 65535 }; /<!-- -->* Blue. *<!-- -->/
 *  
 *  
 *  source = gdk_bitmap_create_from_data (NULL, cursor1_bits,
 *                                        cursor1_width, cursor1_height);
 *  mask = gdk_bitmap_create_from_data (NULL, cursor1mask_bits,
 *                                      cursor1_width, cursor1_height);
 *  cursor = gdk_cursor_new_from_pixmap (source, mask, &amp;fg, &amp;bg, 8, 8);
 *  gdk_pixmap_unref (source);
 *  gdk_pixmap_unref (mask);
 *  
 *  
 *  gdk_window_set_cursor (widget->window, cursor);
 * </programlisting>
 * </example>
 *
 * Return value: a new #GdkCursor.
 **/
GdkCursor*
gdk_cursor_new_from_pixmap (GdkPixmap      *source,
			    GdkPixmap      *mask,
			    const GdkColor *fg,
			    const GdkColor *bg,
			    gint            x,
			    gint            y)
{
  GdkCursorPrivate *private;
  GdkCursor *cursor;
  Pixmap source_pixmap, mask_pixmap;
  Cursor xcursor;
  XColor xfg, xbg;
  GdkDisplay *display;

  g_return_val_if_fail (GDK_IS_PIXMAP (source), NULL);
  g_return_val_if_fail (GDK_IS_PIXMAP (mask), NULL);
  g_return_val_if_fail (fg != NULL, NULL);
  g_return_val_if_fail (bg != NULL, NULL);

  source_pixmap = GDK_PIXMAP_XID (source);
  mask_pixmap   = GDK_PIXMAP_XID (mask);
  display = GDK_PIXMAP_DISPLAY (source);

  xfg.pixel = fg->pixel;
  xfg.red = fg->red;
  xfg.blue = fg->blue;
  xfg.green = fg->green;
  xbg.pixel = bg->pixel;
  xbg.red = bg->red;
  xbg.blue = bg->blue;
  xbg.green = bg->green;
  
  if (display->closed)
    xcursor = None;
  else
    xcursor = XCreatePixmapCursor (GDK_DISPLAY_XDISPLAY (display),
				   source_pixmap, mask_pixmap, &xfg, &xbg, x, y);
  private = g_new (GdkCursorPrivate, 1);
  private->display = display;
  private->xcursor = xcursor;
  cursor = (GdkCursor *) private;
  cursor->type = GDK_CURSOR_IS_PIXMAP;
  cursor->ref_count = 1;
  
  return cursor;
}

void
_gdk_cursor_destroy (GdkCursor *cursor)
{
  GdkCursorPrivate *private;

  g_return_if_fail (cursor != NULL);
  g_return_if_fail (cursor->ref_count == 0);

  private = (GdkCursorPrivate *) cursor;
  if (!private->display->closed && private->xcursor)
    XFreeCursor (GDK_DISPLAY_XDISPLAY (private->display), private->xcursor);

  g_free (private);
}

/**
 * gdk_x11_cursor_get_xdisplay:
 * @cursor: a #GdkCursor.
 * 
 * Returns the display of a #GdkCursor.
 * 
 * Return value: an Xlib <type>Display*</type>.
 **/
Display *
gdk_x11_cursor_get_xdisplay (GdkCursor *cursor)
{
  g_return_val_if_fail (cursor != NULL, NULL);

  return GDK_DISPLAY_XDISPLAY(((GdkCursorPrivate *)cursor)->display);
}

/**
 * gdk_x11_cursor_get_xcursor:
 * @cursor: a #GdkCursor.
 * 
 * Returns the X cursor belonging to a #GdkCursor.
 * 
 * Return value: an Xlib <type>Cursor</type>.
 **/
Cursor
gdk_x11_cursor_get_xcursor (GdkCursor *cursor)
{
  g_return_val_if_fail (cursor != NULL, None);

  return ((GdkCursorPrivate *)cursor)->xcursor;
}

/** 
 * gdk_cursor_get_display:
 * @cursor: a #GdkCursor.
 *
 * Returns the display on which the #GdkCursor is defined.
 *
 * Returns: the #GdkDisplay associated to @cursor
 *
 * Since: 2.2
 */

GdkDisplay *
gdk_cursor_get_display (GdkCursor *cursor)
{
  g_return_val_if_fail (cursor != NULL, NULL);

  return ((GdkCursorPrivate *)cursor)->display;
}


#ifdef HAVE_XCURSOR

static XcursorImage*
create_cursor_image (GdkPixbuf *pixbuf, 
		     gint       x, 
		     gint       y)
{
  guint width, height, rowstride, n_channels;
  guchar *pixels, *src;
  XcursorImage *xcimage;
  XcursorPixel *dest;
  guchar a;
  gint i, j;

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);

  xcimage = XcursorImageCreate (width, height);
      
  xcimage->xhot = x;
  xcimage->yhot = y;

  dest = xcimage->pixels;

  for (j = 0; j < height; j++) 
    {
      src = pixels + j * rowstride;
      for (i = 0; i < width; i++) 
	{
	  if (n_channels == 3) 
	    a = 0xff;
	  else
	    a = src[3];
	  
	  *dest =  (a << 24) | (src[0] << 16) | (src[1] << 8) | src[2];
	  src += n_channels;
	  dest++;
	}
    }

  return xcimage;
}


/**
 * gdk_cursor_new_from_pixbuf:
 * @display: the #GdkDisplay for which the cursor will be created
 * @pixbuf: the #GdkPixbuf containing the cursor image
 * @x: the horizontal offset of the 'hotspot' of the cursor. 
 * @y: the vertical offset of the 'hotspot' of the cursor.
 *
 * Creates a new cursor from a pixbuf. 
 *
 * Not all GDK backends support RGBA cursors. If they are not 
 * supported, a monochrome approximation will be displayed. 
 * The functions gdk_display_supports_cursor_alpha() and 
 * gdk_display_supports_cursor_color() can be used to determine
 * whether RGBA cursors are supported; 
 * gdk_display_get_default_cursor_size() and 
 * gdk_display_get_maximal_cursor_size() give information about 
 * cursor sizes.
 *
 * On the X backend, support for RGBA cursors requires a
 * sufficently new version of the X Render extension. 
 *
 * Returns: a new #GdkCursor.
 * 
 * Since: 2.4
 */
GdkCursor *
gdk_cursor_new_from_pixbuf (GdkDisplay *display, 
			    GdkPixbuf  *pixbuf,
			    gint        x,
			    gint        y)
{
  XcursorImage *xcimage;
  Cursor xcursor;
  GdkCursorPrivate *private;
  GdkCursor *cursor;

  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);
  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);
  g_return_val_if_fail (0 <= x && x < gdk_pixbuf_get_width (pixbuf), NULL);
  g_return_val_if_fail (0 <= y && y < gdk_pixbuf_get_height (pixbuf), NULL);

  if (display->closed)
    xcursor = None;
  else 
    {
      xcimage = create_cursor_image (pixbuf, x, y);
      xcursor = XcursorImageLoadCursor (GDK_DISPLAY_XDISPLAY (display), xcimage);
      XcursorImageDestroy (xcimage);
    }

  private = g_new (GdkCursorPrivate, 1);
  private->display = display;
  private->xcursor = xcursor;
  cursor = (GdkCursor *) private;
  cursor->type = GDK_CURSOR_IS_PIXMAP;
  cursor->ref_count = 1;
  
  return cursor;
}

/**
 * gdk_display_supports_cursor_alpha:
 * @display: a #GdkDisplay
 *
 * Returns %TRUE if cursors can use an 8bit alpha channel 
 * on @display. Otherwise, cursors are restricted to bilevel 
 * alpha (i.e. a mask).
 *
 * Returns: whether cursors can have alpha channels.
 *
 * Since: 2.4
 */
gboolean 
gdk_display_supports_cursor_alpha (GdkDisplay *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), FALSE);

  return XcursorSupportsARGB (GDK_DISPLAY_XDISPLAY (display));
}

/**
 * gdk_display_supports_cursor_color:
 * @display: a #GdkDisplay
 *
 * Returns %TRUE if multicolored cursors are supported
 * on @display. Otherwise, cursors have only a forground
 * and a background color.
 *
 * Returns: whether cursors can have multiple colors.
 *
 * Since: 2.4
 */
gboolean 
gdk_display_supports_cursor_color (GdkDisplay *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), FALSE);

  return XcursorSupportsARGB (GDK_DISPLAY_XDISPLAY (display));
}

/**
 * gdk_display_get_default_cursor_size:
 * @display: a #GdkDisplay
 *
 * Returns the default size to use for cursors on @display.
 *
 * Returns: the default cursor size.
 *
 * Since: 2.4
 */
guint     
gdk_display_get_default_cursor_size (GdkDisplay *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), FALSE);

  return XcursorGetDefaultSize (GDK_DISPLAY_XDISPLAY (display));
}

#else

GdkCursor *
gdk_cursor_new_from_pixbuf (GdkDisplay *display, 
			    GdkPixbuf  *pixbuf,
			    gint        x,
			    gint        y)
{
  GdkCursor *cursor;
  GdkPixmap *pixmap, *mask;
  guint width, height, n_channels, rowstride, i, j;
  guint8 *data, *mask_data, *pixels;
  GdkColor fg = { 0, 0, 0, 0 };
  GdkColor bg = { 0, 0xffff, 0xffff, 0xffff };
  GdkScreen *screen;

  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);
  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  g_return_val_if_fail (0 <= x && x < width, NULL);
  g_return_val_if_fail (0 <= y && y < height, NULL);

  n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);

  data = g_new0 (guint8, (width + 7) / 8 * height);
  mask_data = g_new0 (guint8, (width + 7) / 8 * height);

  for (j = 0; j < height; j++)
    {
      guint8 *src = pixels + j * rowstride;
      guint8 *d = data + (width + 7) / 8 * j;
      guint8 *md = mask_data + (width + 7) / 8 * j;
	
      for (i = 0; i < width; i++)
	{
	  if (src[1] < 0x80)
	    *d |= 1 << (i % 8);
	  
	  if (n_channels == 3 || src[3] >= 0x80)
	    *md |= 1 << (i % 8);
	  
	  src += n_channels;
	  if (i % 8 == 7)
	    {
	      d++; 
	      md++;
	    }
	}
    }
      
  screen = gdk_display_get_default_screen (display);
  pixmap = gdk_bitmap_create_from_data (gdk_screen_get_root_window (screen), 
					data, width, height);
 
  mask = gdk_bitmap_create_from_data (gdk_screen_get_root_window (screen),
				      mask_data, width, height);
   
  cursor = gdk_cursor_new_from_pixmap (pixmap, mask, &fg, &bg, x, y);
   
  g_object_unref (pixmap);
  g_object_unref (mask);

  g_free (data);
  g_free (mask_data);
  
  return cursor;
}

gboolean 
gdk_display_supports_cursor_alpha (GdkDisplay    *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), FALSE);

  return FALSE;
}

gboolean 
gdk_display_supports_cursor_color (GdkDisplay    *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), FALSE);

  return FALSE;
}

guint     
gdk_display_get_default_cursor_size (GdkDisplay    *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), 0);
  
  /* no idea, really */
  return 20; 
}

#endif


/**
 * gdk_display_get_maximal_cursor_size:
 * @display: a #GdkDisplay
 * @width: the return location for the maximal cursor width
 * @height: the return location for the maximal cursor height
 *
 * Gets the maximal size to use for cursors on @display.
 *
 * Since: 2.4
 */
void     
gdk_display_get_maximal_cursor_size (GdkDisplay *display,
				     guint       *width,
				     guint       *height)
{
  GdkScreen *screen;
  GdkWindow *window;

  g_return_if_fail (GDK_IS_DISPLAY (display));
  
  screen = gdk_display_get_default_screen (display);
  window = gdk_screen_get_root_window (screen);
  XQueryBestCursor (GDK_DISPLAY_XDISPLAY (display), 
		    GDK_WINDOW_XWINDOW (window), 
		    128, 128, width, height);
}

