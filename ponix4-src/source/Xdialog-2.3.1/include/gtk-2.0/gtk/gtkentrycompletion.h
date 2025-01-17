/* gtkentrycompletion.h
 * Copyright (C) 2003  Kristian Rietveld  <kris@gtk.org>
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

#ifndef __GTK_ENTRY_COMPLETION_H__
#define __GTK_ENTRY_COMPLETION_H__

#include <glib-object.h>

#include <gtk/gtktreemodel.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtktreeviewcolumn.h>
#include <gtk/gtktreemodelfilter.h>

G_BEGIN_DECLS

#define GTK_TYPE_ENTRY_COMPLETION            (gtk_entry_completion_get_type ())
#define GTK_ENTRY_COMPLETION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_ENTRY_COMPLETION, GtkEntryCompletion))
#define GTK_ENTRY_COMPLETION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_ENTRY_COMPLETION, GtkEntryCompletionClass))
#define GTK_IS_ENTRY_COMPLETION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_ENTRY_COMPLETION))
#define GTK_IS_ENTRY_COMPLETION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ENTRY_COMPLETION))
#define GTK_ENTRY_COMPLETION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_ENTRY_COMPLETION, GtkEntryCompletionClass))

typedef struct _GtkEntryCompletion            GtkEntryCompletion;
typedef struct _GtkEntryCompletionClass       GtkEntryCompletionClass;
typedef struct _GtkEntryCompletionPrivate     GtkEntryCompletionPrivate;

typedef gboolean (* GtkEntryCompletionMatchFunc) (GtkEntryCompletion *completion,
                                                  const gchar        *key,
                                                  GtkTreeIter        *iter,
                                                  gpointer            user_data);


struct _GtkEntryCompletion
{
  GObject parent_instance;

  /*< private >*/
  GtkEntryCompletionPrivate *priv;
};

struct _GtkEntryCompletionClass
{
  GObjectClass parent_class;

  gboolean (* match_selected)   (GtkEntryCompletion *completion,
                                 GtkTreeModel       *model,
                                 GtkTreeIter        *iter);
  void     (* action_activated) (GtkEntryCompletion *completion,
                                 gint                index);

  /* Padding for future expansion */
  void (*_gtk_reserved0) (void);
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
};

/* core */
GType               gtk_entry_completion_get_type               (void);
GtkEntryCompletion *gtk_entry_completion_new                    (void);

GtkWidget          *gtk_entry_completion_get_entry              (GtkEntryCompletion          *completion);

void                gtk_entry_completion_set_model              (GtkEntryCompletion          *completion,
                                                                 GtkTreeModel                *model);
GtkTreeModel       *gtk_entry_completion_get_model              (GtkEntryCompletion          *completion);

void                gtk_entry_completion_set_match_func         (GtkEntryCompletion          *completion,
                                                                 GtkEntryCompletionMatchFunc  func,
                                                                 gpointer                     func_data,
                                                                 GDestroyNotify               func_notify);
void                gtk_entry_completion_set_minimum_key_length (GtkEntryCompletion          *completion,
                                                                 gint                         length);
gint                gtk_entry_completion_get_minimum_key_length (GtkEntryCompletion          *completion);
void                gtk_entry_completion_complete               (GtkEntryCompletion          *completion);

void                gtk_entry_completion_insert_action_text     (GtkEntryCompletion          *completion,
                                                                 gint                         index,
                                                                 const gchar                 *text);
void                gtk_entry_completion_insert_action_markup   (GtkEntryCompletion          *completion,
                                                                 gint                         index,
                                                                 const gchar                 *markup);
void                gtk_entry_completion_delete_action          (GtkEntryCompletion          *completion,
                                                                 gint                         index);

/* convenience */
void                gtk_entry_completion_set_text_column        (GtkEntryCompletion          *completion,
                                                                 gint                         column);

G_END_DECLS

#endif /* __GTK_ENTRY_COMPLETION_H__ */
