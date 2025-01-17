/* GtkCellRendererCombo
 * Copyright (C) 2004 Lorenzo Gil Sanchez
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

#include <config.h>
#include <string.h>

#include "gtkintl.h"
#include "gtkbin.h"
#include "gtkentry.h"
#include "gtkcelllayout.h"
#include "gtkcellrenderercombo.h"
#include "gtkcellrenderertext.h"
#include "gtkcombobox.h"
#include "gtkcomboboxentry.h"
#include "gtkprivate.h"
#include "gtkalias.h"

static void gtk_cell_renderer_combo_class_init (GtkCellRendererComboClass *klass);
static void gtk_cell_renderer_combo_init       (GtkCellRendererCombo      *self);
static void gtk_cell_renderer_combo_finalize     (GObject      *object);
static void gtk_cell_renderer_combo_get_property (GObject      *object,
						  guint         prop_id,
						  GValue       *value,
						  GParamSpec   *pspec);

static void gtk_cell_renderer_combo_set_property (GObject      *object,
						  guint         prop_id,
						  const GValue *value,
						  GParamSpec   *pspec);

static GtkCellEditable *gtk_cell_renderer_combo_start_editing (GtkCellRenderer     *cell,
							       GdkEvent            *event,
							       GtkWidget           *widget,
							       const gchar         *path,
							       GdkRectangle        *background_area,
							       GdkRectangle        *cell_area,
							       GtkCellRendererState flags);

enum {
  PROP_0,
  PROP_MODEL,
  PROP_TEXT_COLUMN,
  PROP_HAS_ENTRY
};

#define GTK_CELL_RENDERER_COMBO_PATH "gtk-cell-renderer-combo-path"

G_DEFINE_TYPE (GtkCellRendererCombo, gtk_cell_renderer_combo, GTK_TYPE_CELL_RENDERER_TEXT)

static void
gtk_cell_renderer_combo_class_init (GtkCellRendererComboClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);

  object_class->finalize = gtk_cell_renderer_combo_finalize;
  object_class->get_property = gtk_cell_renderer_combo_get_property;
  object_class->set_property = gtk_cell_renderer_combo_set_property;

  cell_class->start_editing = gtk_cell_renderer_combo_start_editing;

  /**
   * GtkCellRendererCombo:model:
   *
   * Holds a tree model containing the possible values for the combo box. 
   * Use the text_column property to specify the column holding the values.
   * 
   * Since: 2.6
   */
  g_object_class_install_property (object_class,
				   PROP_MODEL,
				   g_param_spec_object ("model",
							P_("Model"),
							P_("The model containing the possible values for the combo box"),
							GTK_TYPE_TREE_MODEL,
							GTK_PARAM_READWRITE));

  /**
   * GtkCellRendererCombo:text-column:
   *
   * Specifies the model column which holds the possible values for the 
   * combo box. 
   *
   * Note that this refers to the model specified in the model property, 
   * <emphasis>not</emphasis> the model backing the tree view to which 
   * this cell renderer is attached.
   * 
   * #GtkCellRendererCombo automatically adds a text cell renderer for 
   * this column to its combo box.
   *
   * Since: 2.6
   */
  g_object_class_install_property (object_class,
                                   PROP_TEXT_COLUMN,
                                   g_param_spec_int ("text-column",
                                                     P_("Text Column"),
                                                     P_("A column in the data source model to get the strings from"),
                                                     -1,
                                                     G_MAXINT,
                                                     -1,
                                                     GTK_PARAM_READWRITE));

  /** 
   * GtkCellRendererCombo:has-entry:
   *
   * If %TRUE, the cell renderer will include an entry and allow to enter 
   * values other than the ones in the popup list. 
   *
   * Since: 2.6
   */
  g_object_class_install_property (object_class,
                                   PROP_HAS_ENTRY,
                                   g_param_spec_boolean ("has-entry",
							 P_("Has Entry"),
							 P_("If FALSE, don't allow to enter strings other than the chosen ones"),
							 TRUE,
							 GTK_PARAM_READWRITE));

}

static void
gtk_cell_renderer_combo_init (GtkCellRendererCombo *self)
{
  self->model = NULL;
  self->text_column = -1;
  self->has_entry = TRUE;
  self->focus_out_id = 0;
}

/**
 * gtk_cell_renderer_combo_new: 
 * 
 * Creates a new #GtkCellRendererCombo. 
 * Adjust how text is drawn using object properties. 
 * Object properties can be set globally (with g_object_set()). 
 * Also, with #GtkTreeViewColumn, you can bind a property to a value 
 * in a #GtkTreeModel. For example, you can bind the "text" property 
 * on the cell renderer to a string value in the model, thus rendering 
 * a different string in each row of the #GtkTreeView.
 * 
 * Returns: the new cell renderer
 *
 * Since: 2.6
 */
GtkCellRenderer *
gtk_cell_renderer_combo_new (void)
{
  return g_object_new (GTK_TYPE_CELL_RENDERER_COMBO, NULL); 
}

static void
gtk_cell_renderer_combo_finalize (GObject *object)
{
  GtkCellRendererCombo *cell = GTK_CELL_RENDERER_COMBO (object);
  
  if (cell->model)
    {
      g_object_unref (cell->model);
      cell->model = NULL;
    }
  
  G_OBJECT_CLASS (gtk_cell_renderer_combo_parent_class)->finalize (object);
}

static void
gtk_cell_renderer_combo_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
  GtkCellRendererCombo *cell;
 
  g_return_if_fail (GTK_IS_CELL_RENDERER_COMBO (object));

  cell = GTK_CELL_RENDERER_COMBO (object);

  switch (prop_id)
    {
    case PROP_MODEL:
      g_value_set_object (value, cell->model);
      break; 
    case PROP_TEXT_COLUMN:
      g_value_set_int (value, cell->text_column);
      break;
    case PROP_HAS_ENTRY:
      g_value_set_boolean (value, cell->has_entry);
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtk_cell_renderer_combo_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
  GtkCellRendererCombo *cell;
  
  g_return_if_fail (GTK_IS_CELL_RENDERER_COMBO (object));

  cell = GTK_CELL_RENDERER_COMBO (object);

  switch (prop_id)
    {
    case PROP_MODEL:
      {
	GObject *object;

	object = g_value_get_object (value);
	g_return_if_fail (GTK_IS_TREE_MODEL (object));
	g_object_ref (object);

	if (cell->model)
	  {
	    g_object_unref (cell->model);
	    cell->model = NULL;
	  }
	cell->model = GTK_TREE_MODEL (object);
	break;
      }
    case PROP_TEXT_COLUMN:
      cell->text_column = g_value_get_int (value);
      break;
    case PROP_HAS_ENTRY:
      cell->has_entry = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtk_cell_renderer_combo_editing_done (GtkCellEditable *combo,
				      gpointer         data)
{
  const gchar *path;
  gchar *new_text = NULL;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkCellRendererCombo *cell;
  GtkEntry *entry;
  gboolean canceled;

  cell = GTK_CELL_RENDERER_COMBO (data);

  if (cell->focus_out_id > 0)
    {
      g_signal_handler_disconnect (combo, cell->focus_out_id);
      cell->focus_out_id = 0;
    }
  
  canceled = _gtk_combo_box_editing_canceled (GTK_COMBO_BOX (combo));
  gtk_cell_renderer_stop_editing (GTK_CELL_RENDERER (data), canceled);
  if (canceled)
    return;

  if (GTK_IS_COMBO_BOX_ENTRY (combo))
    {
      entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (combo)));
      new_text = g_strdup (gtk_entry_get_text (entry));
    }
  else 
    {
      model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
      if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
	gtk_tree_model_get (model, &iter, cell->text_column, &new_text, -1);
    }

  path = g_object_get_data (G_OBJECT (combo), GTK_CELL_RENDERER_COMBO_PATH);
  g_signal_emit_by_name (cell, "edited", path, new_text);

  g_free (new_text);
}

static gboolean
gtk_cell_renderer_combo_focus_out_event (GtkWidget *widget,
					 GdkEvent  *event,
					 gpointer   data)
{
  
  gtk_cell_renderer_combo_editing_done (GTK_CELL_EDITABLE (widget), data);

  return FALSE;
}

typedef struct 
{
  GtkCellRendererCombo *cell;
  gboolean found;
  GtkTreeIter iter;
} SearchData;

static gboolean 
find_text (GtkTreeModel *model, 
	   GtkTreePath  *path, 
	   GtkTreeIter  *iter, 
	   gpointer      data)
{
  SearchData *search_data = (SearchData *)data;
  gchar *text;
  
  gtk_tree_model_get (model, iter, search_data->cell->text_column, &text, -1);
  if (text && GTK_CELL_RENDERER_TEXT (search_data->cell)->text &&
      strcmp (text, GTK_CELL_RENDERER_TEXT (search_data->cell)->text) == 0)
    {
      search_data->iter = *iter;
      search_data->found = TRUE;
    }

  g_free (text);
  
  return search_data->found;
}

static GtkCellEditable *
gtk_cell_renderer_combo_start_editing (GtkCellRenderer     *cell,
				       GdkEvent            *event,
				       GtkWidget           *widget,
				       const gchar         *path,
				       GdkRectangle        *background_area,
				       GdkRectangle        *cell_area,
				       GtkCellRendererState flags)
{
  GtkCellRendererCombo *cell_combo;
  GtkCellRendererText *cell_text;
  GtkWidget *combo;
  SearchData search_data;

  cell_text = GTK_CELL_RENDERER_TEXT (cell);
  if (cell_text->editable == FALSE)
    return NULL;

  cell_combo = GTK_CELL_RENDERER_COMBO (cell);
  if (cell_combo->model == NULL || cell_combo->text_column < 0)
    return NULL;

  if (cell_combo->has_entry) 
    {
      combo = gtk_combo_box_entry_new_with_model (cell_combo->model, cell_combo->text_column);

      if (cell_text->text)
	gtk_entry_set_text (GTK_ENTRY (GTK_BIN (combo)->child), 
			    cell_text->text);
    }
  else
    {
      cell = gtk_cell_renderer_text_new ();
      combo = gtk_combo_box_new_with_model (cell_combo->model);
      gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), 
				      cell, "text", cell_combo->text_column, 
				      NULL);

      /* determine the current value */
      search_data.cell = cell_combo;
      search_data.found = FALSE;
      gtk_tree_model_foreach (cell_combo->model, find_text, &search_data);
      if (search_data.found)
	gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo),
				       &(search_data.iter));
    }

  g_object_set (combo, "has-frame", FALSE, NULL);
  g_object_set_data_full (G_OBJECT (combo),
			  I_(GTK_CELL_RENDERER_COMBO_PATH),
			  g_strdup (path), g_free);

  gtk_widget_show (combo);

  g_signal_connect (GTK_CELL_EDITABLE (combo), "editing_done",
		    G_CALLBACK (gtk_cell_renderer_combo_editing_done),
		    cell_combo);
  cell_combo->focus_out_id = 
    g_signal_connect (combo, "focus_out_event",
		      G_CALLBACK (gtk_cell_renderer_combo_focus_out_event),
		      cell_combo);

  return GTK_CELL_EDITABLE (combo);
}

#define __GTK_CELL_RENDERER_COMBO_C__
#include "gtkaliasdef.c"
