/* GIO - GLib Input, Output and Streaming Library
 * 
 * Copyright (C) 2006-2008 Red Hat, Inc.
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
 * Author: Alexander Larsson <alexl@redhat.com>
 *         David Zeuthen <davidz@redhat.com>
 */

#if !defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __G_MOUNT_H__
#define __G_MOUNT_H__

#include <gio/giotypes.h>

G_BEGIN_DECLS

#define G_TYPE_MOUNT            (g_mount_get_type ())
#define G_MOUNT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_MOUNT, GMount))
#define G_IS_MOUNT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_MOUNT))
#define G_MOUNT_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), G_TYPE_MOUNT, GMountIface))

typedef struct _GMountIface    GMountIface;

/**
 * GMountIface:
 * @g_iface: The parent interface.
 * @changed: Changed signal that is emitted when the mount's state has changed.
 * @unmounted: The unmounted signal that is emitted when the #GMount have been unmounted. If the recipient is holding references to the object they should release them so the object can be finalized.
 * @get_root: Gets a #GFile to the root directory of the #GMount.
 * @get_name: Gets a string containing the name of the #GMount.
 * @get_icon: Gets a #GIcon for the #GMount.
 * @get_uuid: Gets the UUID for the #GMount. The reference is typically based on the file system UUID for the mount in question and should be considered an opaque string. Returns %NULL if there is no UUID available.
 * @get_volume: Gets a #GVolume the mount is located on. Returns %NULL if the #GMount is not associated with a #GVolume.
 * @get_drive: Gets a #GDrive the volume of the mount is located on. Returns %NULL if the #GMount is not associated with a #GDrive or a #GVolume. This is convenience method for getting the #GVolume and using that to get the #GDrive.
 * @can_unmount: Checks if a #GMount can be unmounted.
 * @can_eject: Checks if a #GMount can be ejected.
 * @unmount: Starts unmounting a #GMount.
 * @unmount_finish: Finishes an unmounting operation.
 * @eject: Starts ejecting a #GMount.
 * @eject_finish: Finishes an eject operation.
 * @remount: Starts remounting a #GMount.
 * @remount_finish: Finishes a remounting operation.
 * @guess_content_type: Starts guessing the type of the content of a #GMount.
 *     See g_mount_guess_content_type() for more information on content
 *     type guessing. This operation was added in 2.18.
 * @guess_content_type_finish: Finishes a contenet type guessing operation.
 * 
 * Interface for implementing operations for mounts.
 **/
struct _GMountIface
{
  GTypeInterface g_iface;

  /* signals */

  void (*changed) (GMount *mount);
  void (*unmounted) (GMount *mount);
  
  /* Virtual Table */

  GFile *            (*get_root)             (GMount         *mount);
  char *             (*get_name)             (GMount         *mount);
  GIcon *            (*get_icon)             (GMount         *mount);
  char *             (*get_uuid)             (GMount         *mount);
  GVolume *          (*get_volume)           (GMount         *mount);
  GDrive *           (*get_drive)            (GMount         *mount);
  gboolean           (*can_unmount)          (GMount         *mount);
  gboolean           (*can_eject)            (GMount         *mount);
  void               (*unmount)              (GMount         *mount,
					      GMountUnmountFlags flags,
                                              GCancellable    *cancellable,
                                              GAsyncReadyCallback callback,
                                              gpointer         user_data);
  gboolean           (*unmount_finish)       (GMount         *mount,
                                              GAsyncResult    *result,
                                              GError         **error);
  void               (*eject)                (GMount         *mount,
					      GMountUnmountFlags flags,
                                              GCancellable    *cancellable,
                                              GAsyncReadyCallback callback,
                                              gpointer         user_data);
  gboolean           (*eject_finish)         (GMount         *mount,
                                              GAsyncResult    *result,
                                              GError         **error);
  void               (*remount)              (GMount         *mount,
					      GMountMountFlags     flags,
					      GMountOperation     *mount_operation,
                                              GCancellable    *cancellable,
                                              GAsyncReadyCallback callback,
                                              gpointer         user_data);
  gboolean           (*remount_finish)       (GMount          *mount,
                                              GAsyncResult    *result,
                                              GError         **error);

  void               (*guess_content_type)        (GMount              *mount,
                                                   gboolean             force_rescan,
                                                   GCancellable        *cancellable,
				                   GAsyncReadyCallback  callback,
				                   gpointer             user_data);

  gchar **           (*guess_content_type_finish) (GMount              *mount,
                                                   GAsyncResult        *result,
                                                    GError             **error); 

  gchar **           (*guess_content_type_sync) (GMount              *mount,
                                                 gboolean             force_rescan,
                                                 GCancellable        *cancellable,
                                                 GError             **error);
};

GType g_mount_get_type (void) G_GNUC_CONST;

GFile *            g_mount_get_root             (GMount              *mount);
char *             g_mount_get_name             (GMount              *mount);
GIcon *            g_mount_get_icon             (GMount              *mount);
char *             g_mount_get_uuid             (GMount              *mount);
GVolume *          g_mount_get_volume           (GMount              *mount);
GDrive *           g_mount_get_drive            (GMount              *mount);
gboolean           g_mount_can_unmount          (GMount              *mount);
gboolean           g_mount_can_eject            (GMount              *mount);
void               g_mount_unmount              (GMount              *mount,
						 GMountUnmountFlags   flags,
                                                 GCancellable        *cancellable,
                                                 GAsyncReadyCallback  callback,
                                                 gpointer             user_data);
gboolean           g_mount_unmount_finish       (GMount              *mount,
                                                 GAsyncResult        *result,
                                                 GError             **error);
void               g_mount_eject                (GMount              *mount,
						 GMountUnmountFlags   flags,
                                                 GCancellable        *cancellable,
                                                 GAsyncReadyCallback  callback,
                                                 gpointer             user_data);
gboolean           g_mount_eject_finish         (GMount              *mount,
                                                 GAsyncResult        *result,
                                                 GError             **error);
void               g_mount_remount              (GMount              *mount,
						 GMountMountFlags     flags,
						 GMountOperation     *mount_operation,
                                                 GCancellable        *cancellable,
                                                 GAsyncReadyCallback  callback,
                                                 gpointer             user_data);
gboolean           g_mount_remount_finish       (GMount              *mount,
                                                 GAsyncResult        *result,
                                                 GError             **error);

void          g_mount_guess_content_type        (GMount              *mount,
                                                 gboolean             force_rescan,
                                                 GCancellable        *cancellable,
                                                 GAsyncReadyCallback  callback,
                                                 gpointer             user_data);
gchar **      g_mount_guess_content_type_finish (GMount              *mount,
                                                 GAsyncResult        *result,
                                                 GError             **error);

gchar **      g_mount_guess_content_type_sync (GMount              *mount,
                                               gboolean             force_rescan,
                                               GCancellable        *cancellable,
                                               GError             **error);

G_END_DECLS

#endif /* __G_MOUNT_H__ */
