/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2006-2007 Red Hat, Inc.
 * Copyright (C) 2008 Novell, Inc.
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
 * Author: Tor Lillqvist <tml@novell.com>
 */

#ifndef __G_WINHTTP_VFS_H__
#define __G_WINHTTP_VFS_H__

#include <gio/giotypes.h>

#include "gvfs.h"

#define _WIN32_WINNT 0x0500
#include <windows.h>

#include "winhttp.h"

G_BEGIN_DECLS

#define G_TYPE_WINHTTP_VFS                      (_g_winhttp_vfs_get_type ())
#define G_WINHTTP_VFS(obj)                      (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_WINHTTP_VFS, GWinHttpVfs))
#define G_WINHTTP_VFS_CLASS(klass)              (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_WINHTTP_VFS, GWinHttpVfsClass))
#define G_IS_WINHTTP_VFS(obj)                   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_WINHTTP_VFS))
#define G_IS_WINHTTP_VFS_CLASS(klass)           (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_WINHTTP_VFS))
#define G_WINHTTP_VFS_GET_CLASS(obj)            (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_WINHTTP_VFS, GWinHttpVfsClass))

typedef struct _GWinHttpVfs       GWinHttpVfs;
typedef struct _GWinHttpVfsClass  GWinHttpVfsClass;

struct _GWinHttpVfs
{
  GVfs parent;

  GVfs *wrapped_vfs;
  HINTERNET session;
};

struct _GWinHttpVfsClass
{
  GVfsClass parent_class;

  /* As there is no import library for winhttp.dll in mingw, we must
   * look up the functions we need dynamically. Store the pointers
   * here.
   */
  BOOL (WINAPI *pWinHttpCloseHandle) (HINTERNET);
  BOOL (WINAPI *pWinHttpCrackUrl) (LPCWSTR,DWORD,DWORD,LPURL_COMPONENTS);
  HINTERNET (WINAPI *pWinHttpConnect) (HINTERNET,LPCWSTR,INTERNET_PORT,DWORD);
  BOOL (WINAPI *pWinHttpCreateUrl) (LPURL_COMPONENTS,DWORD,LPWSTR,LPDWORD);
  HINTERNET (WINAPI *pWinHttpOpen) (LPCWSTR,DWORD,LPCWSTR,LPCWSTR,DWORD);
  HINTERNET (WINAPI *pWinHttpOpenRequest) (HINTERNET,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR*,DWORD);
  BOOL (WINAPI *pWinHttpQueryDataAvailable) (HINTERNET,LPDWORD);
  BOOL (WINAPI *pWinHttpQueryHeaders) (HINTERNET,DWORD,LPCWSTR,LPVOID,LPDWORD,LPDWORD);
  BOOL (WINAPI *pWinHttpReadData) (HINTERNET,LPVOID,DWORD,LPDWORD);
  BOOL (WINAPI *pWinHttpReceiveResponse) (HINTERNET,LPVOID);
  BOOL (WINAPI *pWinHttpSendRequest) (HINTERNET,LPCWSTR,DWORD,LPVOID,DWORD,DWORD,DWORD_PTR);
  BOOL (WINAPI *pWinHttpWriteData) (HINTERNET,LPCVOID,DWORD,LPDWORD);
};


GType   _g_winhttp_vfs_get_type  (void) G_GNUC_CONST;

GVfs *_g_winhttp_vfs_new (void);

char *_g_winhttp_error_message (DWORD error_code);

void _g_winhttp_set_error (GError     **error,
                           DWORD        error_code,
                           const char  *what);

gboolean _g_winhttp_response (GWinHttpVfs *vfs,
                              HINTERNET    request,
                              GError     **error,
                              const char  *what);

gboolean _g_winhttp_query_header (GWinHttpVfs *vfs,
                                  HINTERNET    request,
                                  const char  *request_description,
                                  DWORD        which_header,
                                  wchar_t    **header,
                                  GError     **error);

G_END_DECLS

#endif /* __G_WINHTTP_VFS_H__ */
