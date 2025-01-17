/* GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat Software
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Owen Taylor <otaylor@redhat.com>
 *
 */

#include <config.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>

#include "gtk/gtkintl.h"
#include "gtk/gtkimcontextsimple.h"
#include "gtk/gtkimmodule.h"

GType type_cedilla = 0;

static void cedilla_class_init (GtkIMContextSimpleClass *class);
static void cedilla_init (GtkIMContextSimple *im_context);

static void
cedilla_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (GtkIMContextSimpleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) cedilla_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (GtkIMContextSimple),
    0,
    (GInstanceInitFunc) cedilla_init,
  };

  type_cedilla = 
    g_type_module_register_type (module,
				 GTK_TYPE_IM_CONTEXT_SIMPLE,
				 "GtkIMContextCedillaTranslit",
				 &object_info, 0);
}

/* The difference between this and the default input method is the handling
 * of C+acute - this method produces C WITH CEDILLA rather than C WITH ACUTE.
 * For languages that use CCedilla and not acute, this is the preferred mapping,
 * and is particularly important for pt_BR, where the us-intl keyboard is
 * used extensively.
 */
static guint16 cedilla_compose_seqs[] = {
  GDK_dead_acute,	GDK_C,	0,	0,	0,	0x00C7,	/* LATIN_CAPITAL_LETTER_C_WITH_CEDILLA */
  GDK_dead_acute,	GDK_c,	0,	0,	0,	0x00E7,	/* LATIN_SMALL_LETTER_C_WITH_CEDILLA */
  GDK_Multi_key,	GDK_apostrophe,	GDK_C,  0,      0,      0x00C7, /* LATIN_CAPITAL_LETTER_C_WITH_CEDILLA */
  GDK_Multi_key,	GDK_apostrophe,	GDK_c,  0,      0,      0x00E7, /* LATIN_SMALL_LETTER_C_WITH_CEDILLA */
  GDK_Multi_key,	GDK_C,  GDK_apostrophe,	0,      0,      0x00C7, /* LATIN_CAPITAL_LETTER_C_WITH_CEDILLA */
  GDK_Multi_key,	GDK_c,  GDK_apostrophe,	0,      0,      0x00E7, /* LATIN_SMALL_LETTER_C_WITH_CEDILLA */
};

static void
cedilla_class_init (GtkIMContextSimpleClass *class)
{
}

static void
cedilla_init (GtkIMContextSimple *im_context)
{
  gtk_im_context_simple_add_table (im_context,
				   cedilla_compose_seqs,
				   4,
				   G_N_ELEMENTS (cedilla_compose_seqs) / (4 + 2));
}

static const GtkIMContextInfo cedilla_info = { 
  "cedilla",		           /* ID */
  N_("Cedilla"),                   /* Human readable name */
  "gtk+",			   /* Translation domain */
   GTK_LOCALEDIR,		   /* Dir for bindtextdomain (not strictly needed for "gtk+") */
  "az:ca:co:fr:gv:oc:pt:sq:tr:wa"  /* Languages for which this module is the default */
};

static const GtkIMContextInfo *info_list[] = {
  &cedilla_info
};

void
im_module_init (GTypeModule *module)
{
  cedilla_register_type (module);
}

void 
im_module_exit (void)
{
}

void 
im_module_list (const GtkIMContextInfo ***contexts,
		int                      *n_contexts)
{
  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS (info_list);
}

GtkIMContext *
im_module_create (const gchar *context_id)
{
  if (strcmp (context_id, "cedilla") == 0)
    return GTK_IM_CONTEXT (g_object_new (type_cedilla, NULL));
  else
    return NULL;
}
