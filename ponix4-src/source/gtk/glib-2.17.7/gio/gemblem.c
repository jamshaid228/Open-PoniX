/* GIO - GLib Input, Output and Streaming Library
 * 
 * Copyright (C) 2008 Clemens N. Buss <cebuzz@gmail.com>
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */


#include <config.h>

#include "gemblem.h"
#include "glibintl.h"
#include "gioenums.h"
#include "gioenumtypes.h"

#include "gioalias.h"

/**
 * SECTION:gemblem
 * @short_description: An object for emblems
 * @include: gio/gio.h
 * @see_also: #GIcon, #GEmblemedIcon, #GLoadableIcon, #GThemedIcon
 *
 * #GEmblem is an implementation of #GIcon that supports
 * having an emblem, which is an icon with additional properties.
 * It can than be added to a #GEmblemedIcon.
 *
 * Currently, only metainformation about the emblem's origin is 
 * supported. More may be added in the future.
 **/

static void g_emblem_iface_init (GIconIface *iface);

struct _GEmblem
{
  GObject parent_instance;

  GIcon *icon;
  GEmblemOrigin origin;
};

struct _GEmblemClass
{
  GObjectClass parent_class;
};

enum
{
  PROP_0_GEMBLEM,
  PROP_ICON,
  PROP_ORIGIN
};

G_DEFINE_TYPE_WITH_CODE (GEmblem, g_emblem, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ICON, g_emblem_iface_init))

static void
g_emblem_get_property (GObject    *object, 
                       guint       prop_id, 
                       GValue     *value, 
                       GParamSpec *pspec)
{
  GEmblem *emblem = G_EMBLEM (object);
	
  switch (prop_id)
    {
      case PROP_ICON:
        g_value_set_object (value, emblem->icon);

      case PROP_ORIGIN:
        g_value_set_enum (value, emblem->origin);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
  }   
}

static void
g_emblem_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  GEmblem *emblem = G_EMBLEM (object);

  switch (prop_id)
    {
      case PROP_ICON:
        emblem->icon = g_value_get_object (value);
        break;

      case PROP_ORIGIN:
        emblem->origin = g_value_get_enum (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
g_emblem_finalize (GObject *object)
{
  GEmblem *emblem = G_EMBLEM (object);

  g_object_unref (emblem->icon);

  (*G_OBJECT_CLASS (g_emblem_parent_class)->finalize) (object);
}

static void
g_emblem_class_init (GEmblemClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  gobject_class->finalize = g_emblem_finalize;
  gobject_class->set_property = g_emblem_set_property;
  gobject_class->get_property = g_emblem_get_property;

  g_object_class_install_property (gobject_class, 
                                   PROP_ORIGIN,
                                   g_param_spec_enum ("origin",
                                                      P_("GEmblem's origin"),
                                                      P_("Tells which origin the emblem is derived from"),
                                                      G_TYPE_EMBLEM_ORIGIN,
                                                      G_EMBLEM_ORIGIN_UNKNOWN,
                                                      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));

  g_object_class_install_property (gobject_class,
                                   PROP_ICON,
                                   g_param_spec_object ("icon",
                                                      P_("The icon of the emblem"),
                                                      P_("The actual icon of the emblem"),
                                                      G_TYPE_OBJECT,
                                                      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK));

}

static void
g_emblem_init (GEmblem *emblem)
{
}

/**
 * g_emblem_new:
 * @icon: a GIcon containing the icon.
 * 
 * Creates a new emblem for @icon.
 * 
 * Returns: a new #GEmblem.
 *
 * Since: 2.18
 **/
GEmblem *
g_emblem_new (GIcon *icon)
{
  GEmblem* emblem;

  g_return_val_if_fail (icon != NULL, NULL);
  g_return_val_if_fail (G_IS_ICON (icon), NULL);
  g_return_val_if_fail (!G_IS_EMBLEM (icon), NULL);

  emblem = g_object_new (G_TYPE_EMBLEM, NULL);
  emblem->icon = g_object_ref (icon);
  emblem->origin = G_EMBLEM_ORIGIN_UNKNOWN;

  return emblem;
}

/**
 * g_emblem_new_with_origin:
 * @icon: a GIcon containing the icon.
 * @origin: a GEmblemOrigin enum defining the emblem's origin
 *
 * Creates a new emblem for @icon.
 * 
 * Returns: a new #GEmblem.
 *
 * Since: 2.18
 **/
GEmblem *
g_emblem_new_with_origin (GIcon         *icon,
                          GEmblemOrigin  origin)
{
  GEmblem* emblem;

  g_return_val_if_fail (icon != NULL, NULL);
  g_return_val_if_fail (G_IS_ICON (icon), NULL);
  g_return_val_if_fail (!G_IS_EMBLEM (icon), NULL);

  emblem = g_object_new (G_TYPE_EMBLEM, NULL);
  emblem->icon = g_object_ref (icon);
  emblem->origin = origin;

  return emblem;
}

/**
 * g_emblem_get_icon:
 * @emblem: a #GEmblem from which the icon should be extracted.
 * 
 * Gives back the icon from @emblem.
 * 
 * Returns: a #GIcon. The returned object belongs to the emblem
 *    and should not be modified or freed.
 *
 * Since: 2.18
 **/
GIcon *
g_emblem_get_icon (GEmblem *emblem)
{
  g_return_val_if_fail (G_IS_EMBLEM (emblem), NULL);

  return emblem->icon;
}


/**
 * g_emblem_get_origin:
 * @emblem: a #GEmblem 
 * 
 * Gets the origin of the emblem.
 * 
 * Returns: the origin of the emblem
 *
 * Since: 2.18
 **/
GEmblemOrigin
g_emblem_get_origin (GEmblem *emblem)
{
  g_return_val_if_fail (G_IS_EMBLEM (emblem), G_EMBLEM_ORIGIN_UNKNOWN);

  return emblem->origin;
}

static guint
g_emblem_hash (GIcon *icon)
{
  GEmblem *emblem = G_EMBLEM (icon);
  guint hash;

  hash  = g_icon_hash (g_emblem_get_icon (emblem));
  hash ^= emblem->origin; 

  return hash;
}

static gboolean
g_emblem_equal (GIcon *icon1,
                GIcon *icon2)
{
  GEmblem *emblem1 = G_EMBLEM (icon1);
  GEmblem *emblem2 = G_EMBLEM (icon2);
  
  return emblem1->origin == emblem2->origin &&
         g_icon_equal (emblem1->icon, emblem2->icon);
}

static void
g_emblem_iface_init (GIconIface *iface)
{
  iface->hash  = g_emblem_hash;
  iface->equal = g_emblem_equal;
}

#define __G_EMBLEM_C__
#include "gioaliasdef.c"
