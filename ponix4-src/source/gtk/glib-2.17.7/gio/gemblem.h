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
     
#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_EMBLEM_H__
#define __G_EMBLEM_H__

#include <gio/gicon.h>
#include <gio/gioenums.h>


G_BEGIN_DECLS

#define G_TYPE_EMBLEM         (g_emblem_get_type ())
#define G_EMBLEM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), G_TYPE_EMBLEM, GEmblem))
#define G_EMBLEM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), G_TYPE_EMBLEM, GEmblemClass))
#define G_IS_EMBLEM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_TYPE_EMBLEM))
#define G_IS_EMBLEM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_TYPE_EMBLEM))
#define G_EMBLEM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), G_TYPE_EMBLEM, GEmblemClass))

/**
 * GEmblem:
 * 
 * An object for Emblems
 */
typedef struct _GEmblem        GEmblem;
typedef struct _GEmblemClass   GEmblemClass;

GType          g_emblem_get_type        (void) G_GNUC_CONST;
  
GEmblem       *g_emblem_new             (GIcon         *icon);
GEmblem       *g_emblem_new_with_origin (GIcon         *icon,
                                         GEmblemOrigin  origin);
GIcon         *g_emblem_get_icon        (GEmblem       *emblem);
GEmblemOrigin  g_emblem_get_origin      (GEmblem       *emblem);

G_END_DECLS

#endif /* __G_EMBLEM_H__ */
