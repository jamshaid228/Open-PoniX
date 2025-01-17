/* gtkpathbar.h
 * Copyright (C) 2004  Red Hat, Inc.,  Jonathan Blandford <jrb@gnome.org>
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

#include <config.h>
#include <string.h>
#include "gtkpathbar.h"
#include "gtktogglebutton.h"
#include "gtkarrow.h"
#include "gtkimage.h"
#include "gtkintl.h"
#include "gtkicontheme.h"
#include "gtkiconfactory.h"
#include "gtklabel.h"
#include "gtkhbox.h"
#include "gtkmain.h"
#include "gtkmarshalers.h"

enum {
  PATH_CLICKED,
  LAST_SIGNAL
};

typedef enum {
  NORMAL_BUTTON,
  ROOT_BUTTON,
  HOME_BUTTON,
  DESKTOP_BUTTON
} ButtonType;

#define BUTTON_DATA(x) ((ButtonData *)(x))

static guint path_bar_signals [LAST_SIGNAL] = { 0 };

/* Icon size for if we can't get it from the theme */
#define FALLBACK_ICON_SIZE 20

typedef struct _ButtonData ButtonData;

struct _ButtonData
{
  GtkWidget *button;
  ButtonType type;
  char *dir_name;
  GtkFilePath *path;
  GtkWidget *image;
  GtkWidget *label;
  gboolean ignore_changes;
};

G_DEFINE_TYPE (GtkPathBar,
	       gtk_path_bar,
	       GTK_TYPE_CONTAINER);

static void gtk_path_bar_finalize                 (GObject          *object);
static void gtk_path_bar_dispose                  (GObject          *object);
static void gtk_path_bar_size_request             (GtkWidget        *widget,
						   GtkRequisition   *requisition);
static void gtk_path_bar_size_allocate            (GtkWidget        *widget,
						   GtkAllocation    *allocation);
static void gtk_path_bar_add                      (GtkContainer     *container,
						   GtkWidget        *widget);
static void gtk_path_bar_remove                   (GtkContainer     *container,
						   GtkWidget        *widget);
static void gtk_path_bar_forall                   (GtkContainer     *container,
						   gboolean          include_internals,
						   GtkCallback       callback,
						   gpointer          callback_data);
static void gtk_path_bar_scroll_up                (GtkWidget        *button,
						   GtkPathBar       *path_bar);
static void gtk_path_bar_scroll_down              (GtkWidget        *button,
						   GtkPathBar       *path_bar);
static void gtk_path_bar_style_set                (GtkWidget        *widget,
						   GtkStyle         *previous_style);
static void gtk_path_bar_screen_changed           (GtkWidget        *widget,
						   GdkScreen        *previous_screen);
static void gtk_path_bar_check_icon_theme         (GtkPathBar       *path_bar);
static void gtk_path_bar_update_button_appearance (GtkPathBar       *path_bar,
						   ButtonData       *button_data,
						   gboolean          current_dir);

static GtkWidget *
get_slider_button (GtkPathBar  *path_bar,
		   GtkArrowType arrow_type)
{
  GtkWidget *button;

  gtk_widget_push_composite_child ();

  button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (button), gtk_arrow_new (arrow_type, GTK_SHADOW_OUT));
  gtk_container_add (GTK_CONTAINER (path_bar), button);
  gtk_widget_show_all (button);

  gtk_widget_pop_composite_child ();

  return button;
}

static void
gtk_path_bar_init (GtkPathBar *path_bar)
{
  GTK_WIDGET_SET_FLAGS (path_bar, GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (path_bar), FALSE);

  path_bar->spacing = 3;
  path_bar->up_slider_button = get_slider_button (path_bar, GTK_ARROW_LEFT);
  path_bar->down_slider_button = get_slider_button (path_bar, GTK_ARROW_RIGHT);
  path_bar->icon_size = FALLBACK_ICON_SIZE;

  g_signal_connect (path_bar->up_slider_button, "clicked", G_CALLBACK (gtk_path_bar_scroll_up), path_bar);
  g_signal_connect (path_bar->down_slider_button, "clicked", G_CALLBACK (gtk_path_bar_scroll_down), path_bar);
}

static void
gtk_path_bar_class_init (GtkPathBarClass *path_bar_class)
{
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;

  gobject_class = (GObjectClass *) path_bar_class;
  object_class = (GtkObjectClass *) path_bar_class;
  widget_class = (GtkWidgetClass *) path_bar_class;
  container_class = (GtkContainerClass *) path_bar_class;

  gobject_class->finalize = gtk_path_bar_finalize;
  gobject_class->dispose = gtk_path_bar_dispose;

  widget_class->size_request = gtk_path_bar_size_request;
  widget_class->size_allocate = gtk_path_bar_size_allocate;
  widget_class->style_set = gtk_path_bar_style_set;
  widget_class->screen_changed = gtk_path_bar_screen_changed;

  container_class->add = gtk_path_bar_add;
  container_class->forall = gtk_path_bar_forall;
  container_class->remove = gtk_path_bar_remove;
  /* FIXME: */
  /*  container_class->child_type = gtk_path_bar_child_type;*/

  path_bar_signals [PATH_CLICKED] =
    g_signal_new ("path_clicked",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GtkPathBarClass, path_clicked),
		  NULL, NULL,
		  _gtk_marshal_VOID__POINTER,
		  G_TYPE_NONE, 1,
		  G_TYPE_POINTER);
}


static void
gtk_path_bar_finalize (GObject *object)
{
  GtkPathBar *path_bar;

  path_bar = GTK_PATH_BAR (object);
  g_list_free (path_bar->button_list);
  if (path_bar->root_path)
    gtk_file_path_free (path_bar->root_path);
  if (path_bar->home_path)
    gtk_file_path_free (path_bar->home_path);
  if (path_bar->desktop_path)
    gtk_file_path_free (path_bar->desktop_path);

  if (path_bar->root_icon)
    g_object_unref (path_bar->root_icon);
  if (path_bar->home_icon)
    g_object_unref (path_bar->home_icon);
  if (path_bar->desktop_icon)
    g_object_unref (path_bar->desktop_icon);

  if (path_bar->file_system)
    g_object_unref (path_bar->file_system);

  G_OBJECT_CLASS (gtk_path_bar_parent_class)->finalize (object);
}

/* Removes the settings signal handler.  It's safe to call multiple times */
static void
remove_settings_signal (GtkPathBar *path_bar,
			GdkScreen  *screen)
{
  if (path_bar->settings_signal_id)
    {
      GtkSettings *settings;

      settings = gtk_settings_get_for_screen (screen);
      g_signal_handler_disconnect (settings,
				   path_bar->settings_signal_id);
      path_bar->settings_signal_id = 0;
    }
}

static void
gtk_path_bar_dispose (GObject *object)
{
  remove_settings_signal (GTK_PATH_BAR (object),
			  gtk_widget_get_screen (GTK_WIDGET (object)));

  G_OBJECT_CLASS (gtk_path_bar_parent_class)->dispose (object);
}

/* Size requisition:
 * 
 * Ideally, our size is determined by another widget, and we are just filling
 * available space.
 */
static void
gtk_path_bar_size_request (GtkWidget      *widget,
			   GtkRequisition *requisition)
{
  ButtonData *button_data;
  GtkPathBar *path_bar;
  GtkRequisition child_requisition;
  GList *list;

  path_bar = GTK_PATH_BAR (widget);

  requisition->width = 0;
  requisition->height = 0;

  for (list = path_bar->button_list; list; list = list->next)
    {
      button_data = BUTTON_DATA (list->data);
      gtk_widget_size_request (button_data->button, &child_requisition);
      requisition->width = MAX (child_requisition.width, requisition->width);
      requisition->height = MAX (child_requisition.height, requisition->height);
    }

  /* Add space for slider, if we have more than one path */
  /* Theoretically, the slider could be bigger than the other button.  But we're
   * not going to worry about that now.
   */
  path_bar->slider_width = requisition->height / 2 + 5;
  if (path_bar->button_list && path_bar->button_list->next != NULL)
    requisition->width += (path_bar->spacing + path_bar->slider_width) * 2;

  gtk_widget_size_request (path_bar->up_slider_button, &child_requisition);
  gtk_widget_size_request (path_bar->down_slider_button, &child_requisition);

  requisition->width += GTK_CONTAINER (widget)->border_width * 2;
  requisition->height += GTK_CONTAINER (widget)->border_width * 2;

  widget->requisition = *requisition;
}

static void
gtk_path_bar_update_slider_buttons (GtkPathBar *path_bar)
{
  if (path_bar->button_list)
    {
      GtkWidget *button;

      button = BUTTON_DATA (path_bar->button_list->data)->button;
      if (gtk_widget_get_child_visible (button))
	gtk_widget_set_sensitive (path_bar->down_slider_button, FALSE);
      else
	gtk_widget_set_sensitive (path_bar->down_slider_button, TRUE);

      button = BUTTON_DATA (g_list_last (path_bar->button_list)->data)->button;
      if (gtk_widget_get_child_visible (button))
	gtk_widget_set_sensitive (path_bar->up_slider_button, FALSE);
      else
	gtk_widget_set_sensitive (path_bar->up_slider_button, TRUE);
    }
}

/* This is a tad complicated
 */
static void
gtk_path_bar_size_allocate (GtkWidget     *widget,
			    GtkAllocation *allocation)
{
  GtkWidget *child;
  GtkPathBar *path_bar = GTK_PATH_BAR (widget);
  GtkTextDirection direction;
  GtkAllocation child_allocation;
  GList *list, *first_button;
  gint width;
  gint allocation_width;
  gint border_width;
  gboolean need_sliders = FALSE;
  gint up_slider_offset = 0;
  gint down_slider_offset = 0;

  widget->allocation = *allocation;

  /* No path is set; we don't have to allocate anything. */
  if (path_bar->button_list == NULL)
    return;

  direction = gtk_widget_get_direction (widget);
  border_width = (gint) GTK_CONTAINER (path_bar)->border_width;
  allocation_width = allocation->width - 2 * border_width;

  /* First, we check to see if we need the scrollbars. */
  width = BUTTON_DATA (path_bar->button_list->data)->button->requisition.width;
  for (list = path_bar->button_list->next; list; list = list->next)
    {
      child = BUTTON_DATA (list->data)->button;

      width += child->requisition.width + path_bar->spacing;
    }

  if (width <= allocation_width)
    {
      first_button = g_list_last (path_bar->button_list);
    }
  else
    {
      gboolean reached_end = FALSE;
      gint slider_space = 2 * (path_bar->spacing + path_bar->slider_width);

      if (path_bar->first_scrolled_button)
	first_button = path_bar->first_scrolled_button;
      else
	first_button = path_bar->button_list;
      need_sliders = TRUE;
      
      /* To see how much space we have, and how many buttons we can display.
       * We start at the first button, count forward until hit the new
       * button, then count backwards.
       */
      /* Count down the path chain towards the end. */
      width = BUTTON_DATA (first_button->data)->button->requisition.width;
      list = first_button->prev;
      while (list && !reached_end)
	{
	  child = BUTTON_DATA (list->data)->button;

	  if (width + child->requisition.width +
	      path_bar->spacing + slider_space > allocation_width)
	    reached_end = TRUE;
	  else
	    width += child->requisition.width + path_bar->spacing;

	  list = list->prev;
	}

      /* Finally, we walk up, seeing how many of the previous buttons we can
       * add */
      while (first_button->next && ! reached_end)
	{
	  child = BUTTON_DATA (first_button->next->data)->button;

	  if (width + child->requisition.width + path_bar->spacing + slider_space > allocation_width)
	    {
	      reached_end = TRUE;
	    }
	  else
	    {
	      width += child->requisition.width + path_bar->spacing;
	      first_button = first_button->next;
	    }
	}
    }

  /* Now, we allocate space to the buttons */
  child_allocation.y = allocation->y + border_width;
  child_allocation.height = MAX (1, (gint) allocation->height - border_width * 2);

  if (direction == GTK_TEXT_DIR_RTL)
    {
      child_allocation.x = allocation->x + allocation->width - border_width;
      if (need_sliders)
	{
	  child_allocation.x -= (path_bar->spacing + path_bar->slider_width);
	  up_slider_offset = allocation->width - border_width - path_bar->slider_width;
	}
    }
  else
    {
      child_allocation.x = allocation->x + border_width;
      if (need_sliders)
	{
	  up_slider_offset = border_width;
	  child_allocation.x += (path_bar->spacing + path_bar->slider_width);
	}
    }

  for (list = first_button; list; list = list->prev)
    {
      child = BUTTON_DATA (list->data)->button;

      child_allocation.width = child->requisition.width;
      if (direction == GTK_TEXT_DIR_RTL)
	child_allocation.x -= child_allocation.width;

      /* Check to see if we've don't have any more space to allocate buttons */
      if (need_sliders && direction == GTK_TEXT_DIR_RTL)
	{
	  if (child_allocation.x - path_bar->spacing - path_bar->slider_width < widget->allocation.x + border_width)
	    break;
	}
      else if (need_sliders && direction == GTK_TEXT_DIR_LTR)
	{
	  if (child_allocation.x + child_allocation.width + path_bar->spacing + path_bar->slider_width >
	      widget->allocation.x + border_width + allocation_width)
	    break;
	}

      gtk_widget_set_child_visible (BUTTON_DATA (list->data)->button, TRUE);
      gtk_widget_size_allocate (child, &child_allocation);

      if (direction == GTK_TEXT_DIR_RTL)
	{
	  child_allocation.x -= path_bar->spacing;
	  down_slider_offset = child_allocation.x - widget->allocation.x - path_bar->slider_width;
	}
      else
	{
	  child_allocation.x += child_allocation.width + path_bar->spacing;
	  down_slider_offset = child_allocation.x - widget->allocation.x;
	}
    }
  /* Now we go hide all the widgets that don't fit */
  while (list)
    {
      gtk_widget_set_child_visible (BUTTON_DATA (list->data)->button, FALSE);
      list = list->prev;
    }
  for (list = first_button->next; list; list = list->next)
    {
      gtk_widget_set_child_visible (BUTTON_DATA (list->data)->button, FALSE);
    }

  if (need_sliders)
    {
      child_allocation.width = path_bar->slider_width;
      
      child_allocation.x = up_slider_offset + allocation->x;
      gtk_widget_size_allocate (path_bar->up_slider_button, &child_allocation);

      child_allocation.x = down_slider_offset + allocation->x;
      gtk_widget_size_allocate (path_bar->down_slider_button, &child_allocation);

      gtk_widget_set_child_visible (path_bar->up_slider_button, TRUE);
      gtk_widget_set_child_visible (path_bar->down_slider_button, TRUE);
      gtk_widget_show_all (path_bar->up_slider_button);
      gtk_widget_show_all (path_bar->down_slider_button);
      gtk_path_bar_update_slider_buttons (path_bar);
    }
  else
    {
      gtk_widget_set_child_visible (path_bar->up_slider_button, FALSE);
      gtk_widget_set_child_visible (path_bar->down_slider_button, FALSE);
    }
}

static void
gtk_path_bar_style_set (GtkWidget *widget,
			GtkStyle  *previous_style)
{
  if (GTK_WIDGET_CLASS (gtk_path_bar_parent_class)->style_set)
    GTK_WIDGET_CLASS (gtk_path_bar_parent_class)->style_set (widget, previous_style);

  gtk_path_bar_check_icon_theme (GTK_PATH_BAR (widget));
}

static void
gtk_path_bar_screen_changed (GtkWidget *widget,
			     GdkScreen *previous_screen)
{
  if (GTK_WIDGET_CLASS (gtk_path_bar_parent_class)->screen_changed)
    GTK_WIDGET_CLASS (gtk_path_bar_parent_class)->screen_changed (widget, previous_screen);

  /* We might nave a new settings, so we remove the old one */
  if (previous_screen)
    remove_settings_signal (GTK_PATH_BAR (widget), previous_screen);

  gtk_path_bar_check_icon_theme (GTK_PATH_BAR (widget));
}

static void
gtk_path_bar_add (GtkContainer *container,
		  GtkWidget    *widget)
{
  gtk_widget_set_parent (widget, GTK_WIDGET (container));
}

static void
gtk_path_bar_remove (GtkContainer *container,
		     GtkWidget    *widget)
{
  GtkPathBar *path_bar;
  GList *children;

  path_bar = GTK_PATH_BAR (container);

  children = path_bar->button_list;

  while (children)
    {
      if (widget == BUTTON_DATA (children->data)->button)
	{
	  gboolean was_visible;

	  was_visible = GTK_WIDGET_VISIBLE (widget);
	  gtk_widget_unparent (widget);

	  path_bar->button_list = g_list_remove_link (path_bar->button_list, children);
	  g_list_free (children);

	  if (was_visible)
	    gtk_widget_queue_resize (GTK_WIDGET (container));
	  break;
	}
      
      children = children->next;
    }
}

static void
gtk_path_bar_forall (GtkContainer *container,
		     gboolean      include_internals,
		     GtkCallback   callback,
		     gpointer      callback_data)
{
  GtkPathBar *path_bar;
  GList *children;

  g_return_if_fail (callback != NULL);
  path_bar = GTK_PATH_BAR (container);

  children = path_bar->button_list;
  while (children)
    {
      GtkWidget *child;
      child = BUTTON_DATA (children->data)->button;
      children = children->next;

      (* callback) (child, callback_data);
    }

  (* callback) (path_bar->up_slider_button, callback_data);
  (* callback) (path_bar->down_slider_button, callback_data);
}

static void
gtk_path_bar_scroll_down (GtkWidget *button, GtkPathBar *path_bar)
{
  GList *list;
  GList *down_button = NULL;
  GList *up_button = NULL;
  gint space_available;
  gint space_needed;
  gint border_width;
  GtkTextDirection direction;
  
  gtk_widget_queue_resize (GTK_WIDGET (path_bar));

  border_width = GTK_CONTAINER (path_bar)->border_width;
  direction = gtk_widget_get_direction (GTK_WIDGET (path_bar));
  
  /* We find the button at the 'down' end that we have to make
   * visible */
  for (list = path_bar->button_list; list; list = list->next)
    {
      if (list->next && gtk_widget_get_child_visible (BUTTON_DATA (list->next->data)->button))
	{
	  down_button = list;
	  break;
	}
    }
  
  /* Find the last visible button on the 'up' end
   */
  for (list = g_list_last (path_bar->button_list); list; list = list->prev)
    {
      if (gtk_widget_get_child_visible (BUTTON_DATA (list->data)->button))
	{
	  up_button = list;
	  break;
	}
    }

  space_needed = BUTTON_DATA (down_button->data)->button->allocation.width + path_bar->spacing;
  if (direction == GTK_TEXT_DIR_RTL)
    space_available = path_bar->down_slider_button->allocation.x - GTK_WIDGET (path_bar)->allocation.x;
  else
    space_available = (GTK_WIDGET (path_bar)->allocation.x + GTK_WIDGET (path_bar)->allocation.width - border_width) -
      (path_bar->down_slider_button->allocation.x + path_bar->down_slider_button->allocation.width);

  /* We have space_available extra space that's not being used.  We
   * need space_needed space to make the button fit.  So we walk down
   * from the end, removing buttons until we get all the space we
   * need. */
  while (space_available < space_needed)
    {
      space_available += BUTTON_DATA (up_button->data)->button->allocation.width + path_bar->spacing;
      up_button = up_button->prev;
      path_bar->first_scrolled_button = up_button;
    }
}

static void
gtk_path_bar_scroll_up (GtkWidget *button, GtkPathBar *path_bar)
{
  GList *list;

  gtk_widget_queue_resize (GTK_WIDGET (path_bar));

  for (list = g_list_last (path_bar->button_list); list; list = list->prev)
    {
      if (list->prev && gtk_widget_get_child_visible (BUTTON_DATA (list->prev->data)->button))
	{
	  path_bar->first_scrolled_button = list;
	  return;
	}
    }
}

/* Changes the icons wherever it is needed */
static void
reload_icons (GtkPathBar *path_bar)
{
  GList *list;

  if (path_bar->root_icon)
    {
      g_object_unref (path_bar->root_icon);
      path_bar->root_icon = NULL;
    }
  if (path_bar->home_icon)
    {
      g_object_unref (path_bar->home_icon);
      path_bar->home_icon = NULL;
    }
  if (path_bar->desktop_icon)
    {
      g_object_unref (path_bar->desktop_icon);
      path_bar->desktop_icon = NULL;
    }

  for (list = path_bar->button_list; list; list = list->next)
    {
      ButtonData *button_data;
      gboolean current_dir;

      button_data = BUTTON_DATA (list->data);
      if (button_data->type != NORMAL_BUTTON)
	{
	  current_dir = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_data->button));
	  gtk_path_bar_update_button_appearance (path_bar, button_data, current_dir);
	}
    }
  
}

static void
change_icon_theme (GtkPathBar *path_bar)
{
  GtkSettings *settings;
  gint width, height;

  settings = gtk_settings_get_for_screen (gtk_widget_get_screen (GTK_WIDGET (path_bar)));

  if (gtk_icon_size_lookup_for_settings (settings, GTK_ICON_SIZE_BUTTON, &width, &height))
    path_bar->icon_size = MAX (width, height);
  else
    path_bar->icon_size = FALLBACK_ICON_SIZE;

  reload_icons (path_bar);
}
/* Callback used when a GtkSettings value changes */
static void
settings_notify_cb (GObject    *object,
		    GParamSpec *pspec,
		    GtkPathBar *path_bar)
{
  const char *name;

  name = g_param_spec_get_name (pspec);

  if (! strcmp (name, "gtk-icon-theme-name") ||
      ! strcmp (name, "gtk-icon-sizes"))
    change_icon_theme (path_bar);
}

static void
gtk_path_bar_check_icon_theme (GtkPathBar *path_bar)
{
  GtkSettings *settings;

  if (path_bar->settings_signal_id)
    return;

  settings = gtk_settings_get_for_screen (gtk_widget_get_screen (GTK_WIDGET (path_bar)));
  path_bar->settings_signal_id = g_signal_connect (settings, "notify", G_CALLBACK (settings_notify_cb), path_bar);

  change_icon_theme (path_bar);
}

/* Public functions and their helpers */
static void
gtk_path_bar_clear_buttons (GtkPathBar *path_bar)
{
  while (path_bar->button_list != NULL)
    {
      gtk_container_remove (GTK_CONTAINER (path_bar), BUTTON_DATA (path_bar->button_list->data)->button);
    }
  path_bar->first_scrolled_button = NULL;
}

static void
button_clicked_cb (GtkWidget *button,
		   gpointer   data)
{
  ButtonData *button_data;
  GtkWidget *path_bar;

  button_data = BUTTON_DATA (data);
  if (button_data->ignore_changes)
    return;

  path_bar = button->parent;
  g_assert (GTK_IS_PATH_BAR (path_bar));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

  g_signal_emit (path_bar, path_bar_signals [PATH_CLICKED], 0, button_data->path);
}

static GdkPixbuf *
get_button_image (GtkPathBar *path_bar,
		  ButtonType  button_type)
{
  GtkFileSystemVolume *volume;

  switch (button_type)
    {
    case ROOT_BUTTON:

      if (path_bar->root_icon != NULL)
	return path_bar->root_icon;
      
      volume = gtk_file_system_get_volume_for_path (path_bar->file_system, path_bar->root_path);
      if (volume == NULL)
	return NULL;

      path_bar->root_icon = gtk_file_system_volume_render_icon (path_bar->file_system,
								volume,
								GTK_WIDGET (path_bar),
								path_bar->icon_size,
								NULL);
      gtk_file_system_volume_free (path_bar->file_system, volume);

      return path_bar->root_icon;
    case HOME_BUTTON:
      if (path_bar->home_icon != NULL)
	return path_bar->home_icon;

      path_bar->home_icon = gtk_file_system_render_icon (path_bar->file_system,
							 path_bar->home_path,
							 GTK_WIDGET (path_bar),
							 path_bar->icon_size,
							 NULL);
      return path_bar->home_icon;
    case DESKTOP_BUTTON:
      if (path_bar->desktop_icon != NULL)
	return path_bar->desktop_icon;

      path_bar->desktop_icon = gtk_file_system_render_icon (path_bar->file_system,
							    path_bar->desktop_path,
							    GTK_WIDGET (path_bar),
							    path_bar->icon_size,
							    NULL);
      return path_bar->desktop_icon;
    default:
      return NULL;
    }
  
  return NULL;
}

static void
button_data_free (ButtonData *button_data)
{
  gtk_file_path_free (button_data->path);
  g_free (button_data->dir_name);
  g_free (button_data);
}

static void
gtk_path_bar_update_button_appearance (GtkPathBar *path_bar,
				       ButtonData *button_data,
				       gboolean    current_dir)
{
  const gchar *dir_name;

  if (button_data->type == HOME_BUTTON)
    dir_name = _("Home");
  else if (button_data->type == DESKTOP_BUTTON)
    dir_name = _("Desktop");
  else
    dir_name = button_data->dir_name;

  if (button_data->label != NULL)
    {
      if (current_dir)
	{
	  char *markup;

	  markup = g_markup_printf_escaped ("<b>%s</b>", dir_name);
	  gtk_label_set_markup (GTK_LABEL (button_data->label), markup);
	  g_free (markup);
	}
      else
	{
	  gtk_label_set_text (GTK_LABEL (button_data->label), dir_name);
	}
    }

  if (button_data->image != NULL)
    {
      GdkPixbuf *pixbuf;
      pixbuf = get_button_image (path_bar, button_data->type);
      gtk_image_set_from_pixbuf (GTK_IMAGE (button_data->image), pixbuf);
    }

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_data->button)) != current_dir)
    {
      button_data->ignore_changes = TRUE;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_data->button), current_dir);
      button_data->ignore_changes = FALSE;
    }
}

static ButtonType
find_button_type (GtkPathBar  *path_bar,
		  GtkFilePath *path)
{
  if (! gtk_file_path_compare (path, path_bar->root_path))
    return ROOT_BUTTON;
  if (! gtk_file_path_compare (path, path_bar->home_path))
    return HOME_BUTTON;
  if (! gtk_file_path_compare (path, path_bar->desktop_path))
    return DESKTOP_BUTTON;

 return NORMAL_BUTTON;
}

static ButtonData *
make_directory_button (GtkPathBar  *path_bar,
		       const char  *dir_name,
		       GtkFilePath *path,
		       gboolean     current_dir)
{
  GtkWidget *child = NULL;
  ButtonData *button_data;

  /* Is it a special button? */
  button_data = g_new0 (ButtonData, 1);

  button_data->type = find_button_type (path_bar, path);
  button_data->button = gtk_toggle_button_new ();

  switch (button_data->type)
    {
    case ROOT_BUTTON:
      button_data->image = gtk_image_new ();
      child = button_data->image;
      button_data->label = NULL;
      break;
    case HOME_BUTTON:
    case DESKTOP_BUTTON:
      button_data->image = gtk_image_new ();
      button_data->label = gtk_label_new (NULL);
      child = gtk_hbox_new (FALSE, 2);
      gtk_box_pack_start (GTK_BOX (child), button_data->image, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (child), button_data->label, FALSE, FALSE, 0);
      break;
    case NORMAL_BUTTON:
    default:
      button_data->label = gtk_label_new (NULL);
      child = button_data->label;
      button_data->image = NULL;
    }

  button_data->dir_name = g_strdup (dir_name);
  button_data->path = gtk_file_path_new_dup (gtk_file_path_get_string (path));
			  
  gtk_container_add (GTK_CONTAINER (button_data->button), child);
  gtk_widget_show_all (button_data->button);

  gtk_path_bar_update_button_appearance (path_bar, button_data, current_dir);

  g_signal_connect (button_data->button, "clicked",
		    G_CALLBACK (button_clicked_cb),
		    button_data);
  g_object_weak_ref (G_OBJECT (button_data->button),
		     (GWeakNotify) button_data_free, button_data);

  return button_data;
}

static gboolean
gtk_path_bar_check_parent_path (GtkPathBar         *path_bar,
				const GtkFilePath  *file_path,
				GtkFileSystem      *file_system)
{
  GList *list;
  GList *current_path = NULL;

  for (list = path_bar->button_list; list; list = list->next)
    {
      ButtonData *button_data;

      button_data = list->data;
      if (! gtk_file_path_compare (file_path, button_data->path))
	{
	  current_path = list;
	  break;
	}
    }

  if (current_path)
    {
      for (list = path_bar->button_list; list; list = list->next)
	{
	  gtk_path_bar_update_button_appearance (path_bar,
						 BUTTON_DATA (list->data),
						 (list == current_path) ? TRUE : FALSE);
	}
      return TRUE;
    }
  return FALSE;
}

gboolean
_gtk_path_bar_set_path (GtkPathBar         *path_bar,
			const GtkFilePath  *file_path,
			GError            **error)
{
  GtkFilePath *path;
  gboolean first_directory = TRUE;
  gboolean result;
  GList *new_buttons = NULL;

  g_return_val_if_fail (GTK_IS_PATH_BAR (path_bar), FALSE);
  g_return_val_if_fail (file_path != NULL, FALSE);

  result = TRUE;

  /* Check whether the new path is already present in the pathbar as buttons.
   * This could be a parent directory or a previous selected subdirectory.
   */
  if (gtk_path_bar_check_parent_path (path_bar, file_path, path_bar->file_system))
    return TRUE;

  path = gtk_file_path_copy (file_path);

  gtk_widget_push_composite_child ();

  while (path != NULL)
    {
      GtkFilePath *parent_path = NULL;
      ButtonData *button_data;
      const gchar *display_name;
      GtkFileFolder *file_folder;
      GtkFileInfo *file_info;
      gboolean valid;

      valid = gtk_file_system_get_parent (path_bar->file_system,
					  path,
					  &parent_path,
					  error);
      if (!valid)
	{
	  result = FALSE;
	  gtk_file_path_free (path);
	  break;
	}

      file_folder = gtk_file_system_get_folder (path_bar->file_system,
						parent_path ? parent_path : path,
						GTK_FILE_INFO_DISPLAY_NAME,
						NULL);
      if (!file_folder)
	{
	  result = FALSE;
	  gtk_file_path_free (parent_path);
	  gtk_file_path_free (path);
	  break;
	}

      file_info = gtk_file_folder_get_info (file_folder, parent_path ? path : NULL, error);
      g_object_unref (file_folder);

      if (!file_info)
	{
	  result = FALSE;
	  gtk_file_path_free (parent_path);
	  gtk_file_path_free (path);
	  break;
	}

      display_name = gtk_file_info_get_display_name (file_info);

      button_data = make_directory_button (path_bar, display_name, path, first_directory);
      gtk_file_info_free (file_info);
      gtk_file_path_free (path);

      new_buttons = g_list_prepend (new_buttons, button_data);

      if (button_data->type != NORMAL_BUTTON)
	{
	  gtk_file_path_free (parent_path);
	  break;
	}

      path = parent_path;
      first_directory = FALSE;
    }

  if (result)
    {
      GList *l;

      gtk_path_bar_clear_buttons (path_bar);
      path_bar->button_list = g_list_reverse (new_buttons);

      for (l = path_bar->button_list; l; l = l->next)
	{
	  GtkWidget *button = BUTTON_DATA (l->data)->button;
	  gtk_container_add (GTK_CONTAINER (path_bar), button);
	}
    }
  else
    {
      GList *l;

      for (l = new_buttons; l; l = l->next)
	{
	  GtkWidget *button = BUTTON_DATA (l->data)->button;
	  gtk_widget_destroy (button);
	  gtk_widget_unref (button);
	}

      g_list_free (new_buttons);
    }

  gtk_widget_pop_composite_child ();

  return result;
}


/* FIXME: This should be a construct-only property */
void
_gtk_path_bar_set_file_system (GtkPathBar    *path_bar,
			       GtkFileSystem *file_system)
{
  const char *home;
  char *desktop;

  g_return_if_fail (GTK_IS_PATH_BAR (path_bar));

  g_assert (path_bar->file_system == NULL);

  path_bar->file_system = g_object_ref (file_system);

  home = g_get_home_dir ();
  desktop = g_build_filename (home, "Desktop", NULL);
  path_bar->home_path = gtk_file_system_filename_to_path (path_bar->file_system, home);
  path_bar->desktop_path = gtk_file_system_filename_to_path (path_bar->file_system, desktop);
  path_bar->root_path = gtk_file_system_filename_to_path (path_bar->file_system, "/");
  g_free (desktop);
}

/**
 * _gtk_path_bar_up:
 * @path_bar: a #GtkPathBar
 * 
 * If the selected button in the pathbar is not the furthest button "up" (in the
 * root direction), act as if the user clicked on the next button up.
 **/
void
_gtk_path_bar_up (GtkPathBar *path_bar)
{
  GList *l;

  for (l = path_bar->button_list; l; l = l->next)
    {
      GtkWidget *button = BUTTON_DATA (l->data)->button;
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
	{
	  if (l->next)
	    {
	      GtkWidget *next_button = BUTTON_DATA (l->next->data)->button;
	      button_clicked_cb (next_button, l->next->data);
	    }
	  break;
	}
    }
}

/**
 * _gtk_path_bar_down:
 * @path_bar: a #GtkPathBar
 * 
 * If the selected button in the pathbar is not the furthest button "down" (in the
 * leaf direction), act as if the user clicked on the next button down.
 **/
void
_gtk_path_bar_down (GtkPathBar *path_bar)
{
  GList *l;

  for (l = path_bar->button_list; l; l = l->next)
    {
      GtkWidget *button = BUTTON_DATA (l->data)->button;
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
	{
	  if (l->prev)
	    {
	      GtkWidget *prev_button = BUTTON_DATA (l->prev->data)->button;
	      button_clicked_cb (prev_button, l->prev->data);
	    }
	  break;
	}
    }
}
