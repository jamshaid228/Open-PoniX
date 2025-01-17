/* gtkbuilderprivate.h
 * Copyright (C) 2006-2007 Async Open Source,
 *                         Johan Dahlin <jdahlin@async.com.br>
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
 */

#ifndef __GTK_BUILDER_PRIVATE_H__
#define __GTK_BUILDER_PRIVATE_H__

#include <glib/gmarkup.h>
#include <glib/gslist.h>

#include <glib-object.h>
#include "gtkbuilder.h"

typedef struct {
  const gchar *name;
} TagInfo;

typedef struct {
  TagInfo tag;
} CommonInfo;

typedef struct {
  TagInfo tag;
  gchar *class_name;
  gchar *id;
  gchar *constructor;
  GSList *properties;
  GSList *signals;
  GObject *object;
  CommonInfo *parent;
} ObjectInfo;

typedef struct {
  TagInfo tag;
  GSList *packing_properties;
  GObject *object;
  CommonInfo *parent;
  gchar *type;
  gchar *internal_child;
  gboolean added;
} ChildInfo;

typedef struct {
  TagInfo tag;
  gchar *name;
  GString *text;
  gchar *data;
  gboolean translatable;
  gchar *context;
} PropertyInfo;

typedef struct {
  TagInfo tag;
  gchar *object_name;
  gchar *name;
  gchar *handler;
  GConnectFlags flags;
  gchar *connect_object_name;
} SignalInfo;

typedef struct {
  GMarkupParser *parser;
  gchar *tagname;
  const gchar *start;
  gpointer data;
  GObject *object;
  GObject *child;
} SubParser;

typedef struct {
  const gchar *last_element;
  GtkBuilder *builder;
  gchar *domain;
  GSList *stack;
  SubParser *subparser;
  GMarkupParseContext *ctx;
  const gchar *filename;
  GSList *finalizers;
  GSList *custom_finalizers;
} ParserData;

typedef GType (*GTypeGetFunc) (void);

void _gtk_builder_parser_parse_buffer (GtkBuilder *builder,
                                       const gchar *filename,
                                       const gchar *buffer,
                                       gsize length,
                                       GError **error);
GObject * _gtk_builder_construct (GtkBuilder *builder,
                                  ObjectInfo *info,
				  GError    **error);
void      _gtk_builder_add (GtkBuilder *builder,
                            ChildInfo *child_info);
void      _gtk_builder_add_signals (GtkBuilder *builder,
				    GSList     *signals);
void      _gtk_builder_finish (GtkBuilder *builder);
void _free_signal_info (SignalInfo *info,
                        gpointer user_data);
gboolean _gtk_builder_boolean_from_string (const gchar  *string,
					   gboolean     *value,
					   GError      **error);
gboolean  _gtk_builder_flags_from_string (GType       type,
					  const char *string,
					  gint       *value,
					  GError    **error);

#endif /* __GTK_BUILDER_PRIVATE_H__ */
