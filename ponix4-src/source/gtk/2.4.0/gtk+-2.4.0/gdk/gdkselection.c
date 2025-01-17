#include <config.h>
#include "gdkproperty.h"
#include "gdkdisplay.h"
#include "gdkselection.h"

gboolean
gdk_selection_owner_set (GdkWindow *owner,
			 GdkAtom    selection,
			 guint32    time,
			 gboolean   send_event)
{
  return gdk_selection_owner_set_for_display (gdk_display_get_default (),
					      owner, selection, 
					      time, send_event);
}

GdkWindow*
gdk_selection_owner_get (GdkAtom selection)
{
  return gdk_selection_owner_get_for_display (gdk_display_get_default (), 
					      selection);
}

void
gdk_selection_send_notify (guint32  requestor,
			   GdkAtom  selection,
			   GdkAtom  target,
			   GdkAtom  property,
			   guint32  time)
{
  gdk_selection_send_notify_for_display (gdk_display_get_default (), 
					 requestor, selection, 
					 target, property, time);
}

gint
gdk_text_property_to_text_list (GdkAtom       encoding,
				gint          format, 
				const guchar *text,
				gint          length,
				gchar      ***list)
{
  return gdk_text_property_to_text_list_for_display (gdk_display_get_default (),
						     encoding, format, text, length, list);
}

/**
 * gdk_text_property_to_utf8_list:
 * @encoding: an atom representing the encoding of the text
 * @format:   the format of the property
 * @text:     the text to convert
 * @length:   the length of @text, in bytes
 * @list:     location to store the list of strings or %NULL. The
 *            list should be freed with g_strfreev().
 * 
 * Convert a text property in the giving encoding to
 * a list of UTF-8 strings. 
 * 
 * Return value: the number of strings in the resulting
 *               list.
 **/
gint 
gdk_text_property_to_utf8_list (GdkAtom        encoding,
				gint           format,
				const guchar  *text,
				gint           length,
				gchar       ***list)
{
  return gdk_text_property_to_utf8_list_for_display (gdk_display_get_default (),
						     encoding, format, text, length, list);
}

gint
gdk_string_to_compound_text (const gchar *str,
			     GdkAtom     *encoding,
			     gint        *format,
			     guchar     **ctext,
			     gint        *length)
{
  return gdk_string_to_compound_text_for_display (gdk_display_get_default (),
						  str, encoding, format, 
						  ctext, length);
}

/**
 * gdk_utf8_to_compound_text:
 * @str:      a UTF-8 string
 * @encoding: location to store resulting encoding
 * @format:   location to store format of the result
 * @ctext:    location to store the data of the result
 * @length:   location to store the length of the data
 *            stored in @ctext
 * 
 * Convert from UTF-8 to compound text. 
 * 
 * Return value: %TRUE if the conversion succeeded, otherwise
 *               false.
 **/
gboolean
gdk_utf8_to_compound_text (const gchar *str,
			   GdkAtom     *encoding,
			   gint        *format,
			   guchar     **ctext,
			   gint        *length)
{
  return gdk_utf8_to_compound_text_for_display (gdk_display_get_default (),
						str, encoding, format, 
						ctext, length);
}

