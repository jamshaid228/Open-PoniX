/* GLib testing framework examples and tests
 *
 * Copyright (C) 2008-2011 Red Hat, Inc.
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
 * Author: David Zeuthen <davidz@redhat.com>
 */

#include <gio/gio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "gdbus-tests.h"

#include "gdbus-test-codegen-generated.h"

/* ---------------------------------------------------------------------------------------------------- */

static guint
count_annotations (GDBusAnnotationInfo **annotations)
{
  guint ret;
  ret = 0;
  while (annotations != NULL && annotations[ret] != NULL)
    ret++;
  return ret;
}

/* checks that
 *
 *  - non-internal annotations are written out correctly; and
 *  - injection via --annotation --key --value works
 */
static void
test_annotations (void)
{
  GDBusInterfaceInfo *iface;
  GDBusMethodInfo *method;
  GDBusSignalInfo *signal;
  GDBusPropertyInfo *property;

  iface = foo_bar_interface_info ();
  g_assert (iface != NULL);

  /* see Makefile.am for where these annotations are injected */
  g_assert_cmpint (count_annotations (iface->annotations), ==, 1);
  g_assert_cmpstr (g_dbus_annotation_info_lookup (iface->annotations, "Key1"), ==, "Value1");

  method = g_dbus_interface_info_lookup_method (iface, "HelloWorld");
  g_assert (method != NULL);
  g_assert_cmpint (count_annotations (method->annotations), ==, 2);
  g_assert_cmpstr (g_dbus_annotation_info_lookup (method->annotations, "ExistingAnnotation"), ==, "blah");
  g_assert_cmpstr (g_dbus_annotation_info_lookup (method->annotations, "Key3"), ==, "Value3");

  signal = g_dbus_interface_info_lookup_signal (iface, "TestSignal");
  g_assert (signal != NULL);
  g_assert_cmpint (count_annotations (signal->annotations), ==, 1);
  g_assert_cmpstr (g_dbus_annotation_info_lookup (signal->annotations, "Key4"), ==, "Value4");
  g_assert_cmpstr (g_dbus_annotation_info_lookup (signal->args[1]->annotations, "Key8"), ==, "Value8");

  property = g_dbus_interface_info_lookup_property (iface, "ay");
  g_assert (property != NULL);
  g_assert_cmpint (count_annotations (property->annotations), ==, 1);
  g_assert_cmpstr (g_dbus_annotation_info_lookup (property->annotations, "Key5"), ==, "Value5");

  method = g_dbus_interface_info_lookup_method (iface, "TestPrimitiveTypes");
  g_assert (method != NULL);
  g_assert_cmpstr (g_dbus_annotation_info_lookup (method->in_args[4]->annotations, "Key6"), ==, "Value6");
  g_assert_cmpstr (g_dbus_annotation_info_lookup (method->out_args[5]->annotations, "Key7"), ==, "Value7");
}

/* ---------------------------------------------------------------------------------------------------- */

static gboolean
on_handle_hello_world (FooBar                 *object,
                       GDBusMethodInvocation  *invocation,
                       const gchar            *greeting,
                       gpointer                user_data)
{
  gchar *response;
  response = g_strdup_printf ("Word! You said `%s'. I'm Skeleton, btw!", greeting);
  foo_bar_complete_hello_world (object, invocation, response);
  g_free (response);
  return TRUE;
}

static gboolean
on_handle_test_primitive_types (FooBar                *object,
                                GDBusMethodInvocation *invocation,
                                guchar                 val_byte,
                                gboolean               val_boolean,
                                gint16                 val_int16,
                                guint16                val_uint16,
                                gint                   val_int32,
                                guint                  val_uint32,
                                gint64                 val_int64,
                                guint64                val_uint64,
                                gdouble                val_double,
                                const gchar           *val_string,
                                const gchar           *val_objpath,
                                const gchar           *val_signature,
                                const gchar           *val_bytestring,
                                gpointer               user_data)
{
  gchar *s1;
  gchar *s2;
  gchar *s3;
  s1 = g_strdup_printf ("Word! You said `%s'. Rock'n'roll!", val_string);
  s2 = g_strdup_printf ("/modified%s", val_objpath);
  s3 = g_strdup_printf ("assgit%s", val_signature);
  foo_bar_complete_test_primitive_types (object,
                                         invocation,
                                         10 + val_byte,
                                         !val_boolean,
                                         100 + val_int16,
                                         1000 + val_uint16,
                                         10000 + val_int32,
                                         100000 + val_uint32,
                                         1000000 + val_int64,
                                         10000000 + val_uint64,
                                         val_double / G_PI,
                                         s1,
                                         s2,
                                         s3,
                                         "bytestring!\xff");
  g_free (s1);
  g_free (s2);
  g_free (s3);
  return TRUE;
}

static gboolean
on_handle_test_non_primitive_types (FooBar                *object,
                                    GDBusMethodInvocation *invocation,
                                    GVariant              *dict_s_to_s,
                                    GVariant              *dict_s_to_pairs,
                                    GVariant              *a_struct,
                                    const gchar* const    *array_of_strings,
                                    const gchar* const    *array_of_objpaths,
                                    GVariant              *array_of_signatures,
                                    const gchar* const    *array_of_bytestrings,
                                    gpointer               user_data)
{
  gchar *s;
  GString *str;
  str = g_string_new (NULL);
  s = g_variant_print (dict_s_to_s, TRUE); g_string_append (str, s); g_free (s);
  s = g_variant_print (dict_s_to_pairs, TRUE); g_string_append (str, s); g_free (s);
  s = g_variant_print (a_struct, TRUE); g_string_append (str, s); g_free (s);
  s = g_strjoinv (", ", (gchar **) array_of_strings);
  g_string_append_printf (str, "array_of_strings: [%s] ", s);
  g_free (s);
  s = g_strjoinv (", ", (gchar **) array_of_objpaths);
  g_string_append_printf (str, "array_of_objpaths: [%s] ", s);
  g_free (s);
  s = g_variant_print (array_of_signatures, TRUE);
  g_string_append_printf (str, "array_of_signatures: %s ", s);
  g_free (s);
  s = g_strjoinv (", ", (gchar **) array_of_bytestrings);
  g_string_append_printf (str, "array_of_bytestrings: [%s] ", s);
  g_free (s);
  foo_bar_complete_test_non_primitive_types (object, invocation, str->str);
  g_string_free (str, TRUE);
  return TRUE;
}

static gboolean
on_handle_request_signal_emission (FooBar                 *object,
                                   GDBusMethodInvocation  *invocation,
                                   gint                    which_one,
                                   gpointer                user_data)
{
  if (which_one == 0)
    {
      const gchar *a_strv[] = {"foo", "bar", NULL};
      const gchar *a_bytestring_array[] = {"foo\xff", "bar\xff", NULL};
      GVariant *a_variant = g_variant_new_parsed ("{'first': (42, 42), 'second': (43, 43)}");
      foo_bar_emit_test_signal (object, 43, a_strv, a_bytestring_array, a_variant); /* consumes a_variant */
      foo_bar_complete_request_signal_emission (object, invocation);
    }
  return TRUE;
}

static gboolean
on_handle_request_multi_property_mods (FooBar                 *object,
                                       GDBusMethodInvocation  *invocation,
                                       gpointer                user_data)
{
  foo_bar_set_y (object, foo_bar_get_y (object) + 1);
  foo_bar_set_i (object, foo_bar_get_i (object) + 1);
  foo_bar_set_y (object, foo_bar_get_y (object) + 1);
  foo_bar_set_i (object, foo_bar_get_i (object) + 1);
  g_dbus_interface_skeleton_flush (G_DBUS_INTERFACE_SKELETON (object));
  foo_bar_set_y (object, foo_bar_get_y (object) + 1);
  foo_bar_set_i (object, foo_bar_get_i (object) + 1);
  foo_bar_complete_request_multi_property_mods (object, invocation);
  return TRUE;
}

static gboolean
on_handle_property_cancellation (FooBar                 *object,
                                 GDBusMethodInvocation  *invocation,
                                 gpointer                user_data)
{
  guint n;
  n = foo_bar_get_n (object);
  /* This queues up a PropertiesChange event */
  foo_bar_set_n (object, n + 1);
  /* this modifies the queued up event */
  foo_bar_set_n (object, n);
  /* this flushes all PropertiesChanges event (sends the D-Bus message right
   * away, if any - there should not be any)
   */
  g_dbus_interface_skeleton_flush (G_DBUS_INTERFACE_SKELETON (object));
  /* this makes us return the reply D-Bus method */
  foo_bar_complete_property_cancellation (object, invocation);
  return TRUE;
}

/* ---------------------------------------------------------------------------------------------------- */

static gboolean
on_handle_force_method (FooBat                 *object,
                        GDBusMethodInvocation  *invocation,
                        GVariant               *force_in_i,
                        GVariant               *force_in_s,
                        GVariant               *force_in_ay,
                        GVariant               *force_in_struct,
                        gpointer                user_data)
{
  GVariant *ret_i;
  GVariant *ret_s;
  GVariant *ret_ay;
  GVariant *ret_struct;
  gint32 val;
  gchar *s;

  ret_i = g_variant_new_int32 (g_variant_get_int32 (force_in_i) + 10);
  s = g_strdup_printf ("%s_foo", g_variant_get_string (force_in_s, NULL));
  ret_s = g_variant_new_string (s);
  g_free (s);
  s = g_strdup_printf ("%s_foo\xff", g_variant_get_bytestring (force_in_ay));
  ret_ay = g_variant_new_bytestring (s);
  g_free (s);

  g_variant_get (force_in_struct, "(i)", &val);
  ret_struct = g_variant_new ("(i)", val + 10);

  g_variant_ref_sink (ret_i);
  g_variant_ref_sink (ret_s);
  g_variant_ref_sink (ret_ay);
  g_variant_ref_sink (ret_struct);

  foo_bat_emit_force_signal (object,
                             ret_i,
                             ret_s,
                             ret_ay,
                             ret_struct);

  foo_bat_complete_force_method (object,
                                 invocation,
                                 ret_i,
                                 ret_s,
                                 ret_ay,
                                 ret_struct);

  g_variant_unref (ret_i);
  g_variant_unref (ret_s);
  g_variant_unref (ret_ay);
  g_variant_unref (ret_struct);

  return TRUE;
}


/* ---------------------------------------------------------------------------------------------------- */

static gboolean
my_g_authorize_method_handler (GDBusInterfaceSkeleton *interface,
                               GDBusMethodInvocation  *invocation,
                               gpointer                user_data)
{
  const gchar *method_name;
  gboolean authorized;

  authorized = FALSE;

  method_name = g_dbus_method_invocation_get_method_name (invocation);
  if (g_strcmp0 (method_name, "CheckNotAuthorized") == 0)
    {
      authorized = FALSE;
    }
  else if (g_strcmp0 (method_name, "CheckAuthorized") == 0)
    {
      authorized = TRUE;
    }
  else if (g_strcmp0 (method_name, "CheckNotAuthorizedFromObject") == 0)
    {
      authorized = TRUE;
    }
  else
    {
      g_assert_not_reached ();
    }

  if (!authorized)
    {
      g_dbus_method_invocation_return_error (invocation,
                                             G_IO_ERROR,
                                             G_IO_ERROR_PERMISSION_DENIED,
                                             "not authorized...");
    }
  return authorized;
}

static gboolean
my_object_authorize_method_handler (GDBusObjectSkeleton     *object,
                                    GDBusInterfaceSkeleton  *interface,
                                    GDBusMethodInvocation   *invocation,
                                    gpointer                 user_data)
{
  const gchar *method_name;
  gboolean authorized;

  authorized = FALSE;

  method_name = g_dbus_method_invocation_get_method_name (invocation);
  if (g_strcmp0 (method_name, "CheckNotAuthorized") == 0)
    {
      authorized = TRUE;
    }
  else if (g_strcmp0 (method_name, "CheckAuthorized") == 0)
    {
      authorized = TRUE;
    }
  else if (g_strcmp0 (method_name, "CheckNotAuthorizedFromObject") == 0)
    {
      authorized = FALSE;
    }
  else
    {
      g_assert_not_reached ();
    }

  if (!authorized)
    {
      g_dbus_method_invocation_return_error (invocation,
                                             G_IO_ERROR,
                                             G_IO_ERROR_PENDING,
                                             "not authorized (from object)...");
    }
  return authorized;
}

static gboolean
on_handle_check_not_authorized (FooAuthorize           *object,
                                GDBusMethodInvocation  *invocation,
                                gpointer                user_data)
{
  foo_authorize_complete_check_not_authorized (object, invocation);
  return TRUE;
}

static gboolean
on_handle_check_authorized (FooAuthorize           *object,
                            GDBusMethodInvocation  *invocation,
                            gpointer                user_data)
{
  foo_authorize_complete_check_authorized (object, invocation);
  return TRUE;
}

static gboolean
on_handle_check_not_authorized_from_object (FooAuthorize           *object,
                                            GDBusMethodInvocation  *invocation,
                                            gpointer                user_data)
{
  foo_authorize_complete_check_not_authorized_from_object (object, invocation);
  return TRUE;
}

/* ---------------------------------------------------------------------------------------------------- */

static gboolean
on_handle_get_self (FooMethodThreads       *object,
                    GDBusMethodInvocation  *invocation,
                    gpointer                user_data)
{
  gchar *s;
  s = g_strdup_printf ("%p", g_thread_self ());
  foo_method_threads_complete_get_self (object, invocation, s);
  g_free (s);
  return TRUE;
}

/* ---------------------------------------------------------------------------------------------------- */

static GThread *method_handler_thread = NULL;

static FooBar *exported_bar_object = NULL;
static FooBat *exported_bat_object = NULL;
static FooAuthorize *exported_authorize_object = NULL;
static GDBusObjectSkeleton *authorize_enclosing_object = NULL;
static FooMethodThreads *exported_thread_object_1 = NULL;
static FooMethodThreads *exported_thread_object_2 = NULL;

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  GError *error;

  /* Test that we can export an object using the generated
   * FooBarSkeleton subclass. Notes:
   *
   * 1. We handle methods by simply connecting to the appropriate
   * GObject signal.
   *
   * 2. Property storage is taken care of by the class; we can
   *    use g_object_get()/g_object_set() (and the generated
   *    C bindings at will)
   */
  error = NULL;
  exported_bar_object = foo_bar_skeleton_new ();
  foo_bar_set_ay (exported_bar_object, "ABCabc");
  foo_bar_set_y (exported_bar_object, 42);
  foo_bar_set_d (exported_bar_object, 43.0);
  foo_bar_set_finally_normal_name (exported_bar_object, "There aint no place like home");
  foo_bar_set_writeonly_property (exported_bar_object, "Mr. Burns");

  /* The following works because it's on the Skeleton object - it will
   * fail (at run-time) on a Proxy (see on_proxy_appeared() below)
   */
  foo_bar_set_readonly_property (exported_bar_object, "blah");
  g_assert_cmpstr (foo_bar_get_writeonly_property (exported_bar_object), ==, "Mr. Burns");

  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (exported_bar_object),
                                    connection,
                                    "/bar",
                                    &error);
  g_assert_no_error (error);
  g_signal_connect (exported_bar_object,
                    "handle-hello-world",
                    G_CALLBACK (on_handle_hello_world),
                    NULL);
  g_signal_connect (exported_bar_object,
                    "handle-test-primitive-types",
                    G_CALLBACK (on_handle_test_primitive_types),
                    NULL);
  g_signal_connect (exported_bar_object,
                    "handle-test-non-primitive-types",
                    G_CALLBACK (on_handle_test_non_primitive_types),
                    NULL);
  g_signal_connect (exported_bar_object,
                    "handle-request-signal-emission",
                    G_CALLBACK (on_handle_request_signal_emission),
                    NULL);
  g_signal_connect (exported_bar_object,
                    "handle-request-multi-property-mods",
                    G_CALLBACK (on_handle_request_multi_property_mods),
                    NULL);
  g_signal_connect (exported_bar_object,
                    "handle-property-cancellation",
                    G_CALLBACK (on_handle_property_cancellation),
                    NULL);

  exported_bat_object = foo_bat_skeleton_new ();
  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (exported_bat_object),
                                    connection,
                                    "/bat",
                                    &error);
  g_assert_no_error (error);
  g_signal_connect (exported_bat_object,
                    "handle-force-method",
                    G_CALLBACK (on_handle_force_method),
                    NULL);
  g_object_set (exported_bat_object,
                "force-i", g_variant_new_int32 (43),
                "force-s", g_variant_new_string ("prop string"),
                "force-ay", g_variant_new_bytestring ("prop bytestring\xff"),
                "force-struct", g_variant_new ("(i)", 4300),
                NULL);

  authorize_enclosing_object = g_dbus_object_skeleton_new ("/authorize");
  g_signal_connect (authorize_enclosing_object,
                    "authorize-method",
                    G_CALLBACK (my_object_authorize_method_handler),
                    NULL);
  exported_authorize_object = foo_authorize_skeleton_new ();
  g_dbus_object_skeleton_add_interface (authorize_enclosing_object,
                                        G_DBUS_INTERFACE_SKELETON (exported_authorize_object));
  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (exported_authorize_object),
                                    connection,
                                    "/authorize",
                                    &error);
  g_assert_no_error (error);
  g_signal_connect (exported_authorize_object,
                    "g-authorize-method",
                    G_CALLBACK (my_g_authorize_method_handler),
                    NULL);
  g_signal_connect (exported_authorize_object,
                    "handle-check-not-authorized",
                    G_CALLBACK (on_handle_check_not_authorized),
                    NULL);
  g_signal_connect (exported_authorize_object,
                    "handle-check-authorized",
                    G_CALLBACK (on_handle_check_authorized),
                    NULL);
  g_signal_connect (exported_authorize_object,
                    "handle-check-not-authorized-from-object",
                    G_CALLBACK (on_handle_check_not_authorized_from_object),
                    NULL);


  /* only object 1 has the G_DBUS_INTERFACE_SKELETON_FLAGS_HANDLE_METHOD_INVOCATIONS_IN_THREAD flag set */
  exported_thread_object_1 = foo_method_threads_skeleton_new ();
  g_dbus_interface_skeleton_set_flags (G_DBUS_INTERFACE_SKELETON (exported_thread_object_1),
                                       G_DBUS_INTERFACE_SKELETON_FLAGS_HANDLE_METHOD_INVOCATIONS_IN_THREAD);
  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (exported_thread_object_1),
                                    connection,
                                    "/method_threads_1",
                                    &error);
  g_assert_no_error (error);
  g_signal_connect (exported_thread_object_1,
                    "handle-get-self",
                    G_CALLBACK (on_handle_get_self),
                    NULL);

  exported_thread_object_2 = foo_method_threads_skeleton_new ();
  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (exported_thread_object_2),
                                    connection,
                                    "/method_threads_2",
                                    &error);
  g_assert_no_error (error);
  g_signal_connect (exported_thread_object_2,
                    "handle-get-self",
                    G_CALLBACK (on_handle_get_self),
                    NULL);

  method_handler_thread = g_thread_self ();
}

static gpointer check_proxies_in_thread (gpointer user_data);

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  GMainLoop *loop = user_data;

  g_thread_create (check_proxies_in_thread,
                   loop,
                   TRUE,
                   NULL);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  g_assert_not_reached ();
}

/* ---------------------------------------------------------------------------------------------------- */

typedef struct
{
  GMainLoop *thread_loop;
  gint initial_y;
  gint initial_i;
  guint num_g_properties_changed;
  gboolean received_test_signal;
  guint num_notify_u;
  guint num_notify_n;
} ClientData;

static void
on_notify_u (GObject    *object,
           GParamSpec *pspec,
           gpointer    user_data)
{
  ClientData *data = user_data;
  g_assert_cmpstr (pspec->name, ==, "u");
  data->num_notify_u += 1;
}

static void
on_notify_n (GObject    *object,
             GParamSpec *pspec,
             gpointer    user_data)
{
  ClientData *data = user_data;
  g_assert_cmpstr (pspec->name, ==, "n");
  data->num_notify_n += 1;
}

static void
on_g_properties_changed (GDBusProxy          *_proxy,
                         GVariant            *changed_properties,
                         const gchar* const  *invalidated_properties,
                         gpointer             user_data)
{
  ClientData *data = user_data;
  FooBar *proxy = FOO_BAR (_proxy);

  g_assert_cmpint (g_variant_n_children (changed_properties), ==, 2);

  if (data->num_g_properties_changed == 0)
    {
      g_assert_cmpint (data->initial_y, ==, foo_bar_get_y (proxy) - 2);
      g_assert_cmpint (data->initial_i, ==, foo_bar_get_i (proxy) - 2);
    }
  else if (data->num_g_properties_changed == 1)
    {
      g_assert_cmpint (data->initial_y, ==, foo_bar_get_y (proxy) - 3);
      g_assert_cmpint (data->initial_i, ==, foo_bar_get_i (proxy) - 3);
    }
  else
    g_assert_not_reached ();

  data->num_g_properties_changed++;

  if (data->num_g_properties_changed == 2)
    g_main_loop_quit (data->thread_loop);
}

static void
on_test_signal (FooBar              *proxy,
                gint                 val_int32,
                const gchar* const  *array_of_strings,
                const gchar* const  *array_of_bytestrings,
                GVariant            *dict_s_to_pairs,
                gpointer             user_data)
{
  ClientData *data = user_data;

  g_assert_cmpint (val_int32, ==, 43);
  g_assert_cmpstr (array_of_strings[0], ==, "foo");
  g_assert_cmpstr (array_of_strings[1], ==, "bar");
  g_assert (array_of_strings[2] == NULL);
  g_assert_cmpstr (array_of_bytestrings[0], ==, "foo\xff");
  g_assert_cmpstr (array_of_bytestrings[1], ==, "bar\xff");
  g_assert (array_of_bytestrings[2] == NULL);

  data->received_test_signal = TRUE;
  g_main_loop_quit (data->thread_loop);
}

static void
on_property_cancellation_cb (FooBar        *proxy,
                             GAsyncResult  *res,
                             gpointer       user_data)
{
  ClientData *data = user_data;
  gboolean ret;
  GError *error = NULL;

  error = NULL;
  ret = foo_bar_call_property_cancellation_finish (proxy, res, &error);
  g_assert_no_error (error);
  g_assert (ret);

  g_main_loop_quit (data->thread_loop);
}

static void
check_bar_proxy (FooBar    *proxy,
                 GMainLoop *thread_loop)
{
  guchar ret_val_byte;
  gboolean ret_val_boolean;
  gint16 ret_val_int16;
  guint16 ret_val_uint16;
  gint ret_val_int32;
  guint ret_val_uint32;
  gint64 ret_val_int64;
  guint64 ret_val_uint64;
  gdouble ret_val_double;
  gchar *ret_val_string;
  gchar *ret_val_objpath;
  gchar *ret_val_signature;
  gchar *ret_val_bytestring;
  gboolean ret;
  GError *error;
  ClientData *data;
  guchar val_y;
  gboolean val_b;
  gint val_n;
  guint val_q;
  gint val_i;
  guint val_u;
  gint64 val_x;
  guint64 val_t;
  gdouble val_d;
  gchar *val_s;
  gchar *val_o;
  gchar *val_g;
  gchar *val_ay;
  gchar **val_as;
  gchar **val_ao;
  GVariant *val_ag;
  gint32 val_unset_i;
  gdouble val_unset_d;
  gchar *val_unset_s;
  gchar *val_unset_o;
  gchar *val_unset_g;
  gchar *val_unset_ay;
  gchar **val_unset_as;
  gchar **val_unset_ao;
  GVariant *val_unset_ag;
  GVariant *val_unset_struct;
  gchar *val_finally_normal_name;
  GVariant *v;
  gchar *s;

  data = g_new0 (ClientData, 1);
  data->thread_loop = thread_loop;

  v = g_dbus_proxy_get_cached_property (G_DBUS_PROXY (proxy), "y");
  g_assert (v != NULL);
  g_variant_unref (v);

  /* set empty values to non-empty */
  val_unset_i = 42;
  val_unset_d = 42.0;
  val_unset_s = "42";
  val_unset_o = "42";
  val_unset_g = "42";
  val_unset_ay = NULL;
  val_unset_as = NULL;
  val_unset_ao = NULL;
  val_unset_ag = NULL;
  val_unset_struct = NULL;
  /* check properties */
  g_object_get (proxy,
                "y", &val_y,
                "b", &val_b,
                "n", &val_n,
                "q", &val_q,
                "i", &val_i,
                "u", &val_u,
                "x", &val_x,
                "t", &val_t,
                "d", &val_d,
                "s", &val_s,
                "o", &val_o,
                "g", &val_g,
                "ay", &val_ay,
                "as", &val_as,
                "ao", &val_ao,
                "ag", &val_ag,
                "unset_i", &val_unset_i,
                "unset_d", &val_unset_d,
                "unset_s", &val_unset_s,
                "unset_o", &val_unset_o,
                "unset_g", &val_unset_g,
                "unset_ay", &val_unset_ay,
                "unset_as", &val_unset_as,
                "unset_ao", &val_unset_ao,
                "unset_ag", &val_unset_ag,
                "unset_struct", &val_unset_struct,
                "finally-normal-name", &val_finally_normal_name,
                NULL);
  g_assert_cmpint (val_y, ==, 42);
  g_assert_cmpstr (val_finally_normal_name, ==, "There aint no place like home");
  g_free (val_s);
  g_free (val_o);
  g_free (val_g);
  g_assert_cmpstr (val_ay, ==, "ABCabc");
  g_free (val_ay);
  g_strfreev (val_as);
  g_strfreev (val_ao);
  g_variant_unref (val_ag);
  g_free (val_finally_normal_name);
  /* check empty values */
  g_assert_cmpint (val_unset_i, ==, 0);
  g_assert_cmpfloat (val_unset_d, ==, 0.0);
  g_assert_cmpstr (val_unset_s, ==, "");
  g_assert_cmpstr (val_unset_o, ==, "/");
  g_assert_cmpstr (val_unset_g, ==, "");
  g_free (val_unset_s);
  g_free (val_unset_o);
  g_free (val_unset_g);
  g_assert_cmpstr (val_unset_ay, ==, "");
  g_assert (val_unset_as[0] == NULL);
  g_assert (val_unset_ao[0] == NULL);
  g_assert (g_variant_is_of_type (val_unset_ag, G_VARIANT_TYPE ("ag")));
  g_assert (g_variant_is_of_type (val_unset_struct, G_VARIANT_TYPE ("(idsogayasaoag)")));
  s = g_variant_print (val_unset_struct, TRUE);
  g_assert_cmpstr (s, ==, "(0, 0.0, '', objectpath '/', signature '', @ay [], @as [], @ao [], @ag [])");
  g_free (s);
  g_free (val_unset_ay);
  g_strfreev (val_unset_as);
  g_strfreev (val_unset_ao);
  g_variant_unref (val_unset_ag);
  g_variant_unref (val_unset_struct);

  /* Try setting a property. This causes the generated glue to invoke
   * the org.fd.DBus.Properties.Set() method asynchronously. So we
   * have to wait for properties-changed...
   */
  foo_bar_set_finally_normal_name (proxy, "foo!");
  _g_assert_property_notify (proxy, "finally-normal-name");
  g_assert_cmpstr (foo_bar_get_finally_normal_name (proxy), ==, "foo!");

  /* Try setting properties that requires memory management. This
   * is to exercise the paths that frees the references.
   */
  const gchar *array_of_strings[3] = {"one", "two", NULL};
  const gchar *array_of_objpaths[3] = {"/one", "/one/two", NULL};
  const gchar *array_of_bytestrings[3] = {"one\xff", "two\xff", NULL};

  g_object_set (proxy,
                "s", "a string",
                "o", "/a/path",
                "g", "asig",
                "ay", g_variant_new_parsed ("[byte 0x65, 0x67]"),
                "as", array_of_strings,
                "ao", array_of_objpaths,
                "ag", g_variant_new_parsed ("[@g 'ass', 'git']"),
                NULL);

  error = NULL;
  ret = foo_bar_call_test_primitive_types_sync (proxy,
                                                10,
                                                TRUE,
                                                11,
                                                12,
                                                13,
                                                14,
                                                15,
                                                16,
                                                17,
                                                "a string",
                                                "/a/path",
                                                "asig",
                                                "bytestring\xff",
                                                &ret_val_byte,
                                                &ret_val_boolean,
                                                &ret_val_int16,
                                                &ret_val_uint16,
                                                &ret_val_int32,
                                                &ret_val_uint32,
                                                &ret_val_int64,
                                                &ret_val_uint64,
                                                &ret_val_double,
                                                &ret_val_string,
                                                &ret_val_objpath,
                                                &ret_val_signature,
                                                &ret_val_bytestring,
                                                NULL, /* GCancellable */
                                                &error);
  g_assert_no_error (error);
  g_assert (ret);

  error = NULL;
  ret = foo_bar_call_test_non_primitive_types_sync (proxy,
                                                    g_variant_new_parsed ("{'one': 'red',"
                                                                          " 'two': 'blue'}"),
                                                    g_variant_new_parsed ("{'first': (42, 42), "
                                                                          "'second': (43, 43)}"),
                                                    g_variant_new_parsed ("(42, 'foo', 'bar')"),
                                                    array_of_strings,
                                                    array_of_objpaths,
                                                    g_variant_new_parsed ("[@g 'ass', 'git']"),
                                                    array_of_bytestrings,
                                                    &s,
                                                    NULL, /* GCancellable */
                                                    &error);

  g_assert_no_error (error);
  g_assert (ret);

  /* Check that org.freedesktop.DBus.Error.UnknownMethod is returned on
   * unimplemented methods.
   */
  error = NULL;
  ret = foo_bar_call_unimplemented_method_sync (proxy, NULL /* GCancellable */, &error);
  g_assert_error (error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD);
  g_error_free (error);
  error = NULL;
  g_assert (!ret);

  g_signal_connect (proxy,
                    "test-signal",
                    G_CALLBACK (on_test_signal),
                    data);
  error = NULL;
  ret = foo_bar_call_request_signal_emission_sync (proxy, 0, NULL, &error);
  g_assert_no_error (error);
  g_assert (ret);

  g_assert (!data->received_test_signal);
  g_main_loop_run (thread_loop);
  g_assert (data->received_test_signal);

  /* Try setting a property. This causes the generated glue to invoke
   * the org.fd.DBus.Properties.Set() method asynchronously. So we
   * have to wait for properties-changed...
   */
  foo_bar_set_finally_normal_name (proxy, "hey back!");
  _g_assert_property_notify (proxy, "finally-normal-name");
  g_assert_cmpstr (foo_bar_get_finally_normal_name (proxy), ==, "hey back!");

  /* Check that grouping changes in idle works.
   *
   * See on_handle_request_multi_property_mods(). The server should
   * emit exactly two PropertiesChanged signals each containing two
   * properties.
   *
   * On the first reception, y and i should both be increased by
   * two. On the the second reception, only by one. The signal handler
   * checks this.
   *
   * This also checks that _drain_notify() works.
   */
  data->initial_y = foo_bar_get_y (proxy);
  data->initial_i = foo_bar_get_i (proxy);
  g_signal_connect (proxy,
                    "g-properties-changed",
                    G_CALLBACK (on_g_properties_changed),
                    data);
  error = NULL;
  ret = foo_bar_call_request_multi_property_mods_sync (proxy, NULL, &error);
  g_assert_no_error (error);
  g_assert (ret);
  g_main_loop_run (thread_loop);
  g_assert_cmpint (data->num_g_properties_changed, ==, 2);
  g_signal_handlers_disconnect_by_func (proxy,
                                        G_CALLBACK (on_g_properties_changed),
                                        data);

  /* Check that we don't emit PropertiesChanged() if the property
   * didn't change... we actually get two notifies.. one for the
   * local set (without a value change) and one when receiving
   * the PropertiesChanged() signal generated from the remote end.
   */
  g_assert_cmpint (data->num_notify_u, ==, 0);
  g_signal_connect (proxy,
                    "notify::u",
                    G_CALLBACK (on_notify_u),
                    data);
  foo_bar_set_u (proxy, 1042);
  g_assert_cmpint (data->num_notify_u, ==, 1);
  g_assert_cmpint (foo_bar_get_u (proxy), ==, 0);
  _g_assert_property_notify (proxy, "u");
  g_assert_cmpint (foo_bar_get_u (proxy), ==, 1042);
  g_assert_cmpint (data->num_notify_u, ==, 2);

  /* Now change u again to the same value.. this will cause a
   * local notify:: notify and the usual Properties.Set() call
   *
   * (Btw, why also the Set() call if the value in the cache is
   * the same? Because someone else might have changed it
   * in the mean time and we're just waiting to receive the
   * PropertiesChanged() signal...)
   *
   * More tricky - how do we check for the *absence* of the
   * notification that u changed? Simple: we change another
   * property and wait for that PropertiesChanged() message
   * to arrive.
   */
  foo_bar_set_u (proxy, 1042);
  g_assert_cmpint (data->num_notify_u, ==, 3);

  g_assert_cmpint (data->num_notify_n, ==, 0);
  g_signal_connect (proxy,
                    "notify::n",
                    G_CALLBACK (on_notify_n),
                    data);
  foo_bar_set_n (proxy, 10042);
  g_assert_cmpint (data->num_notify_n, ==, 1);
  g_assert_cmpint (foo_bar_get_n (proxy), ==, 0);
  _g_assert_property_notify (proxy, "n");
  g_assert_cmpint (foo_bar_get_n (proxy), ==, 10042);
  g_assert_cmpint (data->num_notify_n, ==, 2);
  /* Checks that u didn't change at all */
  g_assert_cmpint (data->num_notify_u, ==, 3);

  /* Now we check that if the service does
   *
   *   guint n = foo_bar_get_n (foo);
   *   foo_bar_set_n (foo, n + 1);
   *   foo_bar_set_n (foo, n);
   *
   *  then no PropertiesChanged() signal is emitted!
   */
  error = NULL;
  foo_bar_call_property_cancellation (proxy,
                                      NULL, /* GCancellable */
                                      (GAsyncReadyCallback) on_property_cancellation_cb,
                                      data);
  g_main_loop_run (thread_loop);
  /* Checks that n didn't change at all */
  g_assert_cmpint (data->num_notify_n, ==, 2);

  /* cleanup */
  g_free (data);
}

/* ---------------------------------------------------------------------------------------------------- */

static void
on_force_signal (FooBat    *proxy,
                 GVariant  *force_i,
                 GVariant  *force_s,
                 GVariant  *force_ay,
                 GVariant  *force_struct,
                 gpointer   user_data)
{
  gboolean *signal_received = user_data;
  gint val;

  g_assert (!(*signal_received));

  g_assert_cmpint (g_variant_get_int32 (force_i), ==, 42 + 10);
  g_assert_cmpstr (g_variant_get_string (force_s, NULL), ==, "a string_foo");
  g_assert_cmpstr (g_variant_get_bytestring (force_ay), ==, "a bytestring\xff_foo\xff");
  g_variant_get (force_struct, "(i)", &val);
  g_assert_cmpint (val, ==, 4200 + 10);

  *signal_received = TRUE;
}

static void
check_bat_proxy (FooBat *proxy,
                 GMainLoop *thread_loop)
{
  GError *error;
  GVariant *ret_i;
  GVariant *ret_s;
  GVariant *ret_ay;
  GVariant *ret_struct;
  gint val;
  gboolean force_signal_received;

  /* --------------------------------------------------- */
  /* Check type-mapping where we force use of a GVariant */
  /* --------------------------------------------------- */

  /* check properties */
  g_object_get (proxy,
                "force-i", &ret_i,
                "force-s", &ret_s,
                "force-ay", &ret_ay,
                "force-struct", &ret_struct,
                NULL);
  g_assert_cmpint (g_variant_get_int32 (ret_i), ==, 43);
  g_assert_cmpstr (g_variant_get_string (ret_s, NULL), ==, "prop string");
  g_assert_cmpstr (g_variant_get_bytestring (ret_ay), ==, "prop bytestring\xff");
  g_variant_get (ret_struct, "(i)", &val);
  g_assert_cmpint (val, ==, 4300);
  g_variant_unref (ret_i);
  g_variant_unref (ret_s);
  g_variant_unref (ret_ay);
  g_variant_unref (ret_struct);

  /* check method and signal */
  force_signal_received = FALSE;
  g_signal_connect (proxy,
                    "force-signal",
                    G_CALLBACK (on_force_signal),
                    &force_signal_received);

  error = NULL;
  foo_bat_call_force_method_sync (proxy,
                                  g_variant_new_int32 (42),
                                  g_variant_new_string ("a string"),
                                  g_variant_new_bytestring ("a bytestring\xff"),
                                  g_variant_new ("(i)", 4200),
                                  &ret_i,
                                  &ret_s,
                                  &ret_ay,
                                  &ret_struct,
                                  NULL, /* GCancellable* */
                                  &error);
  g_assert_no_error (error);
  g_assert_cmpint (g_variant_get_int32 (ret_i), ==, 42 + 10);
  g_assert_cmpstr (g_variant_get_string (ret_s, NULL), ==, "a string_foo");
  g_assert_cmpstr (g_variant_get_bytestring (ret_ay), ==, "a bytestring\xff_foo\xff");
  g_variant_get (ret_struct, "(i)", &val);
  g_assert_cmpint (val, ==, 4200 + 10);
  g_variant_unref (ret_i);
  g_variant_unref (ret_s);
  g_variant_unref (ret_ay);
  g_variant_unref (ret_struct);
  _g_assert_signal_received (proxy, "force-signal");
  g_assert (force_signal_received);
}

/* ---------------------------------------------------------------------------------------------------- */

static void
check_authorize_proxy (FooAuthorize *proxy,
                       GMainLoop *thread_loop)
{
  GError *error;
  gboolean ret;

  /* Check that g-authorize-method works as intended */

  error = NULL;
  ret = foo_authorize_call_check_not_authorized_sync (proxy, NULL, &error);
  g_assert_error (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED);
  g_error_free (error);
  g_assert (!ret);

  error = NULL;
  ret = foo_authorize_call_check_authorized_sync (proxy, NULL, &error);
  g_assert_no_error (error);
  g_assert (ret);

  error = NULL;
  ret = foo_authorize_call_check_not_authorized_from_object_sync (proxy, NULL, &error);
  g_assert_error (error, G_IO_ERROR, G_IO_ERROR_PENDING);
  g_error_free (error);
  g_assert (!ret);
}

/* ---------------------------------------------------------------------------------------------------- */

static GThread *
get_self_via_proxy (FooMethodThreads *proxy_1)
{
  GError *error;
  gchar *self_str;
  gboolean ret;
  gpointer self;

  error = NULL;
  ret = foo_method_threads_call_get_self_sync (proxy_1, &self_str, NULL, &error);
  g_assert_no_error (error);
  g_assert (ret);

  g_assert_cmpint (sscanf (self_str, "%p", &self), ==, 1);

  g_free (self_str);

  return self;
}

static void
check_thread_proxies (FooMethodThreads *proxy_1,
                      FooMethodThreads *proxy_2,
                      GMainLoop *thread_loop)
{
  /* proxy_1 is indeed using threads so should never get the handler thread */
  g_assert (get_self_via_proxy (proxy_1) != method_handler_thread);

  /* proxy_2 is not using threads so should get the handler thread */
  g_assert (get_self_via_proxy (proxy_2) == method_handler_thread);
}

/* ---------------------------------------------------------------------------------------------------- */

static gpointer
check_proxies_in_thread (gpointer user_data)
{
  GMainLoop *loop = user_data;
  GMainContext *thread_context;
  GMainLoop *thread_loop;
  GError *error;
  FooBar *bar_proxy;
  FooBat *bat_proxy;
  FooAuthorize *authorize_proxy;
  FooMethodThreads *thread_proxy_1;
  FooMethodThreads *thread_proxy_2;

  thread_context = g_main_context_new ();
  thread_loop = g_main_loop_new (thread_context, FALSE);
  g_main_context_push_thread_default (thread_context);

  /* Check the object */
  error = NULL;
  bar_proxy = foo_bar_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                              G_DBUS_PROXY_FLAGS_NONE,
                                              "org.gtk.GDBus.BindingsTool.Test",
                                              "/bar",
                                              NULL, /* GCancellable* */
                                              &error);
  check_bar_proxy (bar_proxy, thread_loop);
  g_assert_no_error (error);
  g_object_unref (bar_proxy);

  error = NULL;
  bat_proxy = foo_bat_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                              G_DBUS_PROXY_FLAGS_NONE,
                                              "org.gtk.GDBus.BindingsTool.Test",
                                              "/bat",
                                              NULL, /* GCancellable* */
                                              &error);
  check_bat_proxy (bat_proxy, thread_loop);
  g_assert_no_error (error);
  g_object_unref (bat_proxy);

  error = NULL;
  authorize_proxy = foo_authorize_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                          G_DBUS_PROXY_FLAGS_NONE,
                                                          "org.gtk.GDBus.BindingsTool.Test",
                                                          "/authorize",
                                                          NULL, /* GCancellable* */
                                                          &error);
  check_authorize_proxy (authorize_proxy, thread_loop);
  g_assert_no_error (error);
  g_object_unref (authorize_proxy);

  error = NULL;
  thread_proxy_1 = foo_method_threads_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                              G_DBUS_PROXY_FLAGS_NONE,
                                                              "org.gtk.GDBus.BindingsTool.Test",
                                                              "/method_threads_1",
                                                              NULL, /* GCancellable* */
                                                              &error);
  g_assert_no_error (error);
  thread_proxy_2 = foo_method_threads_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                              G_DBUS_PROXY_FLAGS_NONE,
                                                              "org.gtk.GDBus.BindingsTool.Test",
                                                              "/method_threads_2",
                                                              NULL, /* GCancellable* */
                                                              &error);
  g_assert_no_error (error);
  check_thread_proxies (thread_proxy_1, thread_proxy_2, thread_loop);
  g_object_unref (thread_proxy_1);
  g_object_unref (thread_proxy_2);

  g_main_loop_unref (thread_loop);
  g_main_context_unref (thread_context);

  /* this breaks out of the loop in main() (below) */
  g_main_loop_quit (loop);
  return NULL;
}

/* ---------------------------------------------------------------------------------------------------- */

typedef struct
{
  gchar *xml;
  GMainLoop *loop;
} IntrospectData;

static void
introspect_cb (GDBusConnection   *connection,
               GAsyncResult      *res,
               gpointer           user_data)
{
  IntrospectData *data = user_data;
  GVariant *result;
  GError *error;

  error = NULL;
  result = g_dbus_connection_call_finish (connection,
                                          res,
                                          &error);
  g_assert_no_error (error);
  g_assert (result != NULL);
  g_variant_get (result, "(s)", &data->xml);
  g_variant_unref (result);

  g_main_loop_quit (data->loop);
}

static GDBusNodeInfo *
introspect (GDBusConnection  *connection,
            const gchar      *name,
            const gchar      *object_path,
            GMainLoop        *loop)
{
  GError *error;
  GDBusNodeInfo *node_info;
  IntrospectData *data;

  data = g_new0 (IntrospectData, 1);
  data->xml = NULL;
  data->loop = loop;

  /* do this async to avoid deadlocks */
  g_dbus_connection_call (connection,
                          name,
                          object_path,
                          "org.freedesktop.DBus.Introspectable",
                          "Introspect",
                          NULL, /* params */
                          G_VARIANT_TYPE ("(s)"),
                          G_DBUS_CALL_FLAGS_NONE,
                          -1,
                          NULL,
                          (GAsyncReadyCallback) introspect_cb,
                          data);
  g_main_loop_run (loop);
  g_assert (data->xml != NULL);

  error = NULL;
  node_info = g_dbus_node_info_new_for_xml (data->xml, &error);
  g_assert_no_error (error);
  g_assert (node_info != NULL);
  g_free (data->xml);
  g_free (data);

  return node_info;
}

static guint
count_interfaces (GDBusNodeInfo *info)
{
  guint n;
  for (n = 0; info->interfaces != NULL && info->interfaces[n] != NULL; n++)
    ;
  return n;
}

static guint
count_nodes (GDBusNodeInfo *info)
{
  guint n;
  for (n = 0; info->nodes != NULL && info->nodes[n] != NULL; n++)
    ;
  return n;
}

static guint
has_interface (GDBusNodeInfo *info,
               const gchar   *name)
{
  guint n;
  for (n = 0; info->interfaces != NULL && info->interfaces[n] != NULL; n++)
    {
      if (g_strcmp0 (info->interfaces[n]->name, name) == 0)
        return TRUE;
    }
  return FALSE;
}

/* ---------------------------------------------------------------------------------------------------- */

typedef struct {
  GMainLoop *loop;
  GVariant *result;
} OMGetManagedObjectsData;

static void
om_get_all_cb (GDBusConnection *connection,
               GAsyncResult    *res,
               gpointer         user_data)
{
  OMGetManagedObjectsData *data = user_data;
  GError *error;

  error = NULL;
  data->result = g_dbus_connection_call_finish (connection,
                                                res,
                                                &error);
  g_assert_no_error (error);
  g_assert (data->result != NULL);
  g_main_loop_quit (data->loop);
}

static void
om_check_get_all (GDBusConnection *c,
                  GMainLoop       *loop,
                  const gchar     *str)
{
  OMGetManagedObjectsData data;
  gchar *s;

  data.loop = loop;
  data.result = NULL;

  /* do this async to avoid deadlocks */
  g_dbus_connection_call (c,
                          g_dbus_connection_get_unique_name (c),
                          "/managed",
                          "org.freedesktop.DBus.ObjectManager",
                          "GetManagedObjects",
                          NULL, /* params */
                          G_VARIANT_TYPE ("(a{oa{sa{sv}}})"),
                          G_DBUS_CALL_FLAGS_NONE,
                          -1,
                          NULL,
                          (GAsyncReadyCallback) om_get_all_cb,
                          &data);
  g_main_loop_run (loop);
  g_assert (data.result != NULL);
  s = g_variant_print (data.result, TRUE);
  g_assert_cmpstr (s, ==, str);
  g_free (s);
  g_variant_unref (data.result);
}

typedef struct
{
  GMainLoop *loop;
  guint state;

  guint num_object_proxy_added_signals;
  guint num_object_proxy_removed_signals;
  guint num_interface_added_signals;
  guint num_interface_removed_signals;
} OMData;

static gint
my_pstrcmp (const gchar **a, const gchar **b)
{
  return g_strcmp0 (*a, *b);
}

static void
om_check_interfaces_added (const gchar *signal_name,
                           GVariant *parameters,
                           const gchar *object_path,
                           const gchar *first_interface_name,
                           ...)
{
  const gchar *path;
  GVariant *array;
  guint n;
  GPtrArray *interfaces;
  GPtrArray *interfaces_in_message;
  va_list var_args;
  const gchar *str;

  interfaces = g_ptr_array_new ();
  g_ptr_array_add (interfaces, (gpointer) first_interface_name);
  va_start (var_args, first_interface_name);
  do
    {
      str = va_arg (var_args, const gchar *);
      if (str == NULL)
        break;
      g_ptr_array_add (interfaces, (gpointer) str);
    }
  while (TRUE);
  va_end (var_args);

  g_variant_get (parameters, "(&o*)", &path, &array);
  g_assert_cmpstr (signal_name, ==, "InterfacesAdded");
  g_assert_cmpstr (path, ==, object_path);
  g_assert_cmpint (g_variant_n_children (array), ==, interfaces->len);
  interfaces_in_message = g_ptr_array_new ();
  for (n = 0; n < interfaces->len; n++)
    {
      const gchar *iface_name;
      g_variant_get_child (array, n, "{&sa{sv}}", &iface_name, NULL);
      g_ptr_array_add (interfaces_in_message, (gpointer) iface_name);
    }
  g_assert_cmpint (interfaces_in_message->len, ==, interfaces->len);
  g_ptr_array_sort (interfaces, (GCompareFunc) my_pstrcmp);
  g_ptr_array_sort (interfaces_in_message, (GCompareFunc) my_pstrcmp);
  for (n = 0; n < interfaces->len; n++)
    g_assert_cmpstr (interfaces->pdata[n], ==, interfaces_in_message->pdata[n]);
  g_ptr_array_unref (interfaces_in_message);
  g_ptr_array_unref (interfaces);
  g_variant_unref (array);
}

static void
om_check_interfaces_removed (const gchar *signal_name,
                             GVariant *parameters,
                             const gchar *object_path,
                             const gchar *first_interface_name,
                             ...)
{
  const gchar *path;
  GVariant *array;
  guint n;
  GPtrArray *interfaces;
  GPtrArray *interfaces_in_message;
  va_list var_args;
  const gchar *str;

  interfaces = g_ptr_array_new ();
  g_ptr_array_add (interfaces, (gpointer) first_interface_name);
  va_start (var_args, first_interface_name);
  do
    {
      str = va_arg (var_args, const gchar *);
      if (str == NULL)
        break;
      g_ptr_array_add (interfaces, (gpointer) str);
    }
  while (TRUE);
  va_end (var_args);

  g_variant_get (parameters, "(&o*)", &path, &array);
  g_assert_cmpstr (signal_name, ==, "InterfacesRemoved");
  g_assert_cmpstr (path, ==, object_path);
  g_assert_cmpint (g_variant_n_children (array), ==, interfaces->len);
  interfaces_in_message = g_ptr_array_new ();
  for (n = 0; n < interfaces->len; n++)
    {
      const gchar *iface_name;
      g_variant_get_child (array, n, "&s", &iface_name, NULL);
      g_ptr_array_add (interfaces_in_message, (gpointer) iface_name);
    }
  g_assert_cmpint (interfaces_in_message->len, ==, interfaces->len);
  g_ptr_array_sort (interfaces, (GCompareFunc) my_pstrcmp);
  g_ptr_array_sort (interfaces_in_message, (GCompareFunc) my_pstrcmp);
  for (n = 0; n < interfaces->len; n++)
    g_assert_cmpstr (interfaces->pdata[n], ==, interfaces_in_message->pdata[n]);
  g_ptr_array_unref (interfaces_in_message);
  g_ptr_array_unref (interfaces);
  g_variant_unref (array);
}

static void
om_on_signal (GDBusConnection *connection,
              const gchar     *sender_name,
              const gchar     *object_path,
              const gchar     *interface_name,
              const gchar     *signal_name,
              GVariant        *parameters,
              gpointer         user_data)
{
  OMData *om_data = user_data;

  //g_debug ("foo: %s", g_variant_print (parameters, TRUE));

  switch (om_data->state)
    {
    default:
    case 0:
      g_print ("failing and om_data->state=%d on signal %s, params=%s\n",
               om_data->state,
               signal_name,
               g_variant_print (parameters, TRUE));
      g_assert_not_reached ();
      break;

    case 1:
      om_check_interfaces_added (signal_name, parameters, "/managed/first",
                                 "org.project.Bar", NULL);
      om_data->state = 2;
      g_main_loop_quit (om_data->loop);
      break;

    case 3:
      om_check_interfaces_removed (signal_name, parameters, "/managed/first",
                                   "org.project.Bar", NULL);
      om_data->state = 5;
      /* keep running the loop */
      break;

    case 5:
      om_check_interfaces_added (signal_name, parameters, "/managed/first",
                                 "org.project.Bar", NULL);
      om_data->state = 6;
      g_main_loop_quit (om_data->loop);
      break;

    case 7:
      om_check_interfaces_removed (signal_name, parameters, "/managed/first",
                                   "org.project.Bar", NULL);
      om_data->state = 9;
      /* keep running the loop */
      break;

    case 9:
      om_check_interfaces_added (signal_name, parameters, "/managed/first",
                                 "org.project.Bar", NULL);
      om_data->state = 10;
      g_main_loop_quit (om_data->loop);
      break;

    case 11:
      om_check_interfaces_added (signal_name, parameters, "/managed/first",
                                 "org.project.Bat", NULL);
      om_data->state = 12;
      g_main_loop_quit (om_data->loop);
      break;

    case 13:
      om_check_interfaces_removed (signal_name, parameters, "/managed/first",
                                   "org.project.Bar", NULL);
      om_data->state = 14;
      g_main_loop_quit (om_data->loop);
      break;

    case 15:
      om_check_interfaces_removed (signal_name, parameters, "/managed/first",
                                   "org.project.Bat", NULL);
      om_data->state = 16;
      g_main_loop_quit (om_data->loop);
      break;

    case 17:
      om_check_interfaces_added (signal_name, parameters, "/managed/first",
                                 "com.acme.Coyote", NULL);
      om_data->state = 18;
      g_main_loop_quit (om_data->loop);
      break;

    case 101:
      om_check_interfaces_added (signal_name, parameters, "/managed/second",
                                 "org.project.Bat", "org.project.Bar", NULL);
      om_data->state = 102;
      g_main_loop_quit (om_data->loop);
      break;

    case 103:
      om_check_interfaces_removed (signal_name, parameters, "/managed/second",
                                   "org.project.Bat", "org.project.Bar", NULL);
      om_data->state = 104;
      g_main_loop_quit (om_data->loop);
      break;
    }
}

static GAsyncResult *om_res = NULL;

static void
om_pm_start_cb (FooObjectManagerClient *manager,
                GAsyncResult      *res,
                gpointer           user_data)
{
  GMainLoop *loop = user_data;
  om_res = g_object_ref (res);
  g_main_loop_quit (loop);
}

static void
on_interface_added (GDBusObject    *object,
                    GDBusInterface *interface,
                    gpointer        user_data)
{
  OMData *om_data = user_data;
  om_data->num_interface_added_signals += 1;
}

static void
on_interface_removed (GDBusObject    *object,
                      GDBusInterface *interface,
                      gpointer        user_data)
{
  OMData *om_data = user_data;
  om_data->num_interface_removed_signals += 1;
}

static void
on_object_proxy_added (GDBusObjectManagerClient  *manager,
                       GDBusObjectProxy   *object_proxy,
                       gpointer            user_data)
{
  OMData *om_data = user_data;
  om_data->num_object_proxy_added_signals += 1;
  g_signal_connect (object_proxy,
                    "interface-added",
                    G_CALLBACK (on_interface_added),
                    om_data);
  g_signal_connect (object_proxy,
                    "interface-removed",
                    G_CALLBACK (on_interface_removed),
                    om_data);
}

static void
on_object_proxy_removed (GDBusObjectManagerClient  *manager,
                         GDBusObjectProxy   *object_proxy,
                         gpointer            user_data)
{
  OMData *om_data = user_data;
  om_data->num_object_proxy_removed_signals += 1;
  g_assert_cmpint (g_signal_handlers_disconnect_by_func (object_proxy,
                                                         G_CALLBACK (on_interface_added),
                                                         om_data), ==, 1);
  g_assert_cmpint (g_signal_handlers_disconnect_by_func (object_proxy,
                                                         G_CALLBACK (on_interface_removed),
                                                         om_data), ==, 1);
}

static void
om_check_property_and_signal_emission (GMainLoop  *loop,
                                       FooBar     *skeleton,
                                       FooBar     *proxy)
{
  /* First PropertiesChanged */
  g_assert_cmpint (foo_bar_get_i (skeleton), ==, 0);
  g_assert_cmpint (foo_bar_get_i (proxy), ==, 0);
  foo_bar_set_i (skeleton, 1);
  _g_assert_property_notify (proxy, "i");
  g_assert_cmpint (foo_bar_get_i (skeleton), ==, 1);
  g_assert_cmpint (foo_bar_get_i (proxy), ==, 1);

  /* Then just a regular signal */
  foo_bar_emit_another_signal (skeleton, "word");
  _g_assert_signal_received (proxy, "another-signal");
}

static void
check_object_manager (void)
{
  FooObjectSkeleton *o;
  FooObjectSkeleton *o2;
  GDBusInterfaceSkeleton *i;
  GDBusConnection *c;
  GDBusObjectManagerServer *manager;
  GDBusNodeInfo *info;
  GError *error;
  GMainLoop *loop;
  OMData *om_data;
  guint om_signal_id;
  GDBusObjectManager *pm;
  GList *object_proxies;
  GList *proxies;
  GDBusObject *op;
  GDBusProxy *p;
  FooBar *bar_skeleton;

  loop = g_main_loop_new (NULL, FALSE);

  om_data = g_new0 (OMData, 1);
  om_data->loop = loop;
  om_data->state = 0;

  error = NULL;
  c = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error (error);
  g_assert (c != NULL);

  om_signal_id = g_dbus_connection_signal_subscribe (c,
                                                     NULL, /* sender */
                                                     "org.freedesktop.DBus.ObjectManager",
                                                     NULL, /* member */
                                                     NULL, /* object_path */
                                                     NULL, /* arg0 */
                                                     G_DBUS_SIGNAL_FLAGS_NONE,
                                                     om_on_signal,
                                                     om_data,
                                                     NULL); /* user_data_free_func */

  /* Our GDBusObjectManagerClient tests are simple - we basically just count the
   * number of times the various signals have been emitted (we don't check
   * that the right objects/interfaces are passed though - that's checked
   * in the lower-level tests in om_on_signal()...)
   *
   * Note that these tests rely on the D-Bus signal handlers used by
   * GDBusObjectManagerClient firing before om_on_signal().
   */
  error = NULL;
  pm = foo_object_manager_client_new_sync (c,
                                           G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
                                           g_dbus_connection_get_unique_name (c),
                                           "/managed",
                                           NULL, /* GCancellable */
                                           &error);
  g_assert_error (error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD);
  g_error_free (error);
  g_assert (pm == NULL);

  manager = g_dbus_object_manager_server_new ("/managed");
  g_dbus_object_manager_server_set_connection (manager, c);

  /* Check that the manager object is visible */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed", loop);
  g_assert_cmpint (count_interfaces (info), ==, 4); /* ObjectManager + Properties,Introspectable,Peer */
  g_assert (has_interface (info, "org.freedesktop.DBus.ObjectManager"));
  g_assert_cmpint (count_nodes (info), ==, 0);
  g_dbus_node_info_unref (info);

  /* Check GetManagedObjects() - should be empty since we have no objects */
  om_check_get_all (c, loop,
                    "(@a{oa{sa{sv}}} {},)");

  /* Now try to create the the proxy manager again - this time it should work */
  error = NULL;
  foo_object_manager_client_new (c,
                                 G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
                                 g_dbus_connection_get_unique_name (c),
                                 "/managed",
                                 NULL, /* GCancellable */
                                 (GAsyncReadyCallback) om_pm_start_cb,
                                 loop);
  g_main_loop_run (loop);
  error = NULL;
  pm = foo_object_manager_client_new_finish (om_res, &error);
  g_object_unref (om_res);
  g_assert_no_error (error);
  g_assert (pm != NULL);
  g_signal_connect (pm,
                    "object-added",
                    G_CALLBACK (on_object_proxy_added),
                    om_data);
  g_signal_connect (pm,
                    "object-removed",
                    G_CALLBACK (on_object_proxy_removed),
                    om_data);
  /* ... check there are no object proxies yet */
  object_proxies = g_dbus_object_manager_get_objects (pm);
  g_assert (object_proxies == NULL);

  /* First, export an object with a single interface (also check that
   * g_dbus_interface_get_object() works and that the object isn't reffed)
   */
  o = foo_object_skeleton_new ("/managed/first");
  i = G_DBUS_INTERFACE_SKELETON (foo_bar_skeleton_new ());
  g_assert (g_dbus_interface_get_object (G_DBUS_INTERFACE (i)) == NULL);
  g_assert_cmpint (G_OBJECT (o)->ref_count, ==, 1);
  foo_object_skeleton_set_bar (o, FOO_BAR (i));
  g_assert_cmpint (G_OBJECT (o)->ref_count, ==, 1);
  g_assert (g_dbus_interface_get_object (G_DBUS_INTERFACE (i)) == G_DBUS_OBJECT (o));
  g_assert_cmpint (G_OBJECT (o)->ref_count, ==, 1);
  g_assert_cmpint (G_OBJECT (o)->ref_count, ==, 1);
  foo_object_skeleton_set_bar (o, NULL);
  g_assert (g_dbus_interface_get_object (G_DBUS_INTERFACE (i)) == NULL);
  g_assert_cmpint (G_OBJECT (o)->ref_count, ==, 1);
  foo_object_skeleton_set_bar (o, FOO_BAR (i));
  g_assert (g_dbus_interface_get_object (G_DBUS_INTERFACE (i)) == G_DBUS_OBJECT (o));
  g_assert_cmpint (G_OBJECT (o)->ref_count, ==, 1);
  g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (o));

  /* ... check we get the InterfacesAdded signal */
  om_data->state = 1;
  g_main_loop_run (om_data->loop);
  g_assert_cmpint (om_data->state, ==, 2);
  g_assert_cmpint (om_data->num_object_proxy_added_signals, ==, 1);
  g_assert_cmpint (om_data->num_object_proxy_removed_signals, ==, 0);
  g_assert_cmpint (om_data->num_interface_added_signals, ==, 0);
  g_assert_cmpint (om_data->num_interface_removed_signals, ==, 0);
  /* ... check there's one non-standard interfaces */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed/first", loop);
  g_assert_cmpint (count_interfaces (info), ==, 4); /* Bar + Properties,Introspectable,Peer */
  g_assert (has_interface (info, "org.project.Bar"));
  g_dbus_node_info_unref (info);

  /* Now, check adding the same interface replaces the existing one */
  foo_object_skeleton_set_bar (o, FOO_BAR (i));
  /* ... check we get the InterfacesRemoved */
  om_data->state = 3;
  g_main_loop_run (om_data->loop);
  /* ... and then check we get the InterfacesAdded */
  g_assert_cmpint (om_data->state, ==, 6);
  g_assert_cmpint (om_data->num_object_proxy_added_signals, ==, 2);
  g_assert_cmpint (om_data->num_object_proxy_removed_signals, ==, 1);
  g_assert_cmpint (om_data->num_interface_added_signals, ==, 0);
  g_assert_cmpint (om_data->num_interface_removed_signals, ==, 0);
  /* ... check introspection data */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed/first", loop);
  g_assert_cmpint (count_interfaces (info), ==, 4); /* Bar + Properties,Introspectable,Peer */
  g_assert (has_interface (info, "org.project.Bar"));
  g_dbus_node_info_unref (info);
  g_object_unref (i);

  /* check adding an interface of same type (but not same object) replaces the existing one */
  i = G_DBUS_INTERFACE_SKELETON (foo_bar_skeleton_new ());
  foo_object_skeleton_set_bar (o, FOO_BAR (i));
  /* ... check we get the InterfacesRemoved and then InterfacesAdded */
  om_data->state = 7;
  g_main_loop_run (om_data->loop);
  g_assert_cmpint (om_data->state, ==, 10);
  g_assert_cmpint (om_data->num_object_proxy_added_signals, ==, 3);
  g_assert_cmpint (om_data->num_object_proxy_removed_signals, ==, 2);
  g_assert_cmpint (om_data->num_interface_added_signals, ==, 0);
  g_assert_cmpint (om_data->num_interface_removed_signals, ==, 0);
  /* ... check introspection data */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed/first", loop);
  g_assert_cmpint (count_interfaces (info), ==, 4); /* Bar + Properties,Introspectable,Peer */
  g_assert (has_interface (info, "org.project.Bar"));
  g_dbus_node_info_unref (info);
  g_object_unref (i);

  /* check adding an interface of another type doesn't replace the existing one */
  i = G_DBUS_INTERFACE_SKELETON (foo_bat_skeleton_new ());
  foo_object_skeleton_set_bat (o, FOO_BAT (i));
  g_object_unref (i);
  /* ... check we get the InterfacesAdded */
  om_data->state = 11;
  g_main_loop_run (om_data->loop);
  g_assert_cmpint (om_data->state, ==, 12);
  g_assert_cmpint (om_data->num_object_proxy_added_signals, ==, 3);
  g_assert_cmpint (om_data->num_object_proxy_removed_signals, ==, 2);
  g_assert_cmpint (om_data->num_interface_added_signals, ==, 1);
  g_assert_cmpint (om_data->num_interface_removed_signals, ==, 0);
  /* ... check introspection data */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed/first", loop);
  g_assert_cmpint (count_interfaces (info), ==, 5); /* Bar,Bat + Properties,Introspectable,Peer */
  g_assert (has_interface (info, "org.project.Bar"));
  g_assert (has_interface (info, "org.project.Bat"));
  g_dbus_node_info_unref (info);

  /* check we can remove an interface */
  foo_object_skeleton_set_bar (o, NULL);
  /* ... check we get the InterfacesRemoved */
  om_data->state = 13;
  g_main_loop_run (om_data->loop);
  g_assert_cmpint (om_data->state, ==, 14);
  g_assert_cmpint (om_data->num_object_proxy_added_signals, ==, 3);
  g_assert_cmpint (om_data->num_object_proxy_removed_signals, ==, 2);
  g_assert_cmpint (om_data->num_interface_added_signals, ==, 1);
  g_assert_cmpint (om_data->num_interface_removed_signals, ==, 1);
  /* ... check introspection data */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed/first", loop);
  g_assert_cmpint (count_interfaces (info), ==, 4); /* Bat + Properties,Introspectable,Peer */
  g_assert (has_interface (info, "org.project.Bat"));
  g_dbus_node_info_unref (info);
  /* also and that the call only has effect if the interface actually exists
   *
   * (Note: if a signal was emitted we'd assert in the signal handler
   * because we're in state 14)
   */
  foo_object_skeleton_set_bar (o, NULL);
  /* ... check introspection data */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed/first", loop);
  g_assert_cmpint (count_interfaces (info), ==, 4); /* Bat + Properties,Introspectable,Peer */
  g_assert (has_interface (info, "org.project.Bat"));
  g_dbus_node_info_unref (info);

  /* remove the last interface */
  foo_object_skeleton_set_bat (o, NULL);
  /* ... check we get the InterfacesRemoved */
  om_data->state = 15;
  g_main_loop_run (om_data->loop);
  g_assert_cmpint (om_data->state, ==, 16);
  g_assert_cmpint (om_data->num_object_proxy_added_signals, ==, 3);
  g_assert_cmpint (om_data->num_object_proxy_removed_signals, ==, 3);
  g_assert_cmpint (om_data->num_interface_added_signals, ==, 1);
  g_assert_cmpint (om_data->num_interface_removed_signals, ==, 1);
  /* ... check introspection data */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed/first", loop);
  g_assert_cmpint (count_interfaces (info), ==, 0); /* nothing */
  g_dbus_node_info_unref (info);

  /* and add an interface again */
  i = G_DBUS_INTERFACE_SKELETON (foo_com_acme_coyote_skeleton_new ());
  foo_object_skeleton_set_com_acme_coyote (o, FOO_COM_ACME_COYOTE (i));
  g_object_unref (i);
  /* ... check we get the InterfacesAdded */
  om_data->state = 17;
  g_main_loop_run (om_data->loop);
  g_assert_cmpint (om_data->state, ==, 18);
  g_assert_cmpint (om_data->num_object_proxy_added_signals, ==, 4);
  g_assert_cmpint (om_data->num_object_proxy_removed_signals, ==, 3);
  g_assert_cmpint (om_data->num_interface_added_signals, ==, 1);
  g_assert_cmpint (om_data->num_interface_removed_signals, ==, 1);
  /* ... check introspection data */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed/first", loop);
  g_assert_cmpint (count_interfaces (info), ==, 4); /* com.acme.Coyote + Properties,Introspectable,Peer */
  g_assert (has_interface (info, "com.acme.Coyote"));
  g_dbus_node_info_unref (info);

  /* Check GetManagedObjects() - should be just the Coyote */
  om_check_get_all (c, loop,
                    "({objectpath '/managed/first': {'com.acme.Coyote': {'Mood': <''>}}},)");

  /* -------------------------------------------------- */

  /* create a new object with two interfaces */
  o2 = foo_object_skeleton_new ("/managed/second");
  i = G_DBUS_INTERFACE_SKELETON (foo_bar_skeleton_new ());
  bar_skeleton = FOO_BAR (i); /* save for later test */
  foo_object_skeleton_set_bar (o2, FOO_BAR (i));
  g_object_unref (i);
  i = G_DBUS_INTERFACE_SKELETON (foo_bat_skeleton_new ());
  foo_object_skeleton_set_bat (o2, FOO_BAT (i));
  g_object_unref (i);
  /* ... add it */
  g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (o2));
  /* ... check we get the InterfacesAdded with _two_ interfaces */
  om_data->state = 101;
  g_main_loop_run (om_data->loop);
  g_assert_cmpint (om_data->state, ==, 102);
  g_assert_cmpint (om_data->num_object_proxy_added_signals, ==, 5);
  g_assert_cmpint (om_data->num_object_proxy_removed_signals, ==, 3);
  g_assert_cmpint (om_data->num_interface_added_signals, ==, 1);
  g_assert_cmpint (om_data->num_interface_removed_signals, ==, 1);

  /* -------------------------------------------------- */

  /* Now that we have a couple of objects with interfaces, check
   * that ObjectManager.GetManagedObjects() works
   */
  om_check_get_all (c, loop,
                    "({objectpath '/managed/first': {'com.acme.Coyote': {'Mood': <''>}}, '/managed/second': {'org.project.Bar': {'y': <byte 0x00>, 'b': <false>, 'n': <int16 0>, 'q': <uint16 0>, 'i': <0>, 'u': <uint32 0>, 'x': <int64 0>, 't': <uint64 0>, 'd': <0.0>, 's': <''>, 'o': <objectpath '/'>, 'g': <signature ''>, 'ay': <b''>, 'as': <@as []>, 'aay': <@aay []>, 'ao': <@ao []>, 'ag': <@ag []>, 'FinallyNormalName': <''>, 'ReadonlyProperty': <''>, 'unset_i': <0>, 'unset_d': <0.0>, 'unset_s': <''>, 'unset_o': <objectpath '/'>, 'unset_g': <signature ''>, 'unset_ay': <b''>, 'unset_as': <@as []>, 'unset_ao': <@ao []>, 'unset_ag': <@ag []>, 'unset_struct': <(0, 0.0, '', objectpath '/', signature '', @ay [], @as [], @ao [], @ag [])>}, 'org.project.Bat': {'force_i': <0>, 'force_s': <''>, 'force_ay': <@ay []>, 'force_struct': <(0,)>}}},)");

  /* Set connection to NULL, causing everything to be unexported.. verify this.. and
   * then set the connection back.. and then check things still work
   */
  g_dbus_object_manager_server_set_connection (manager, NULL);
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed", loop);
  g_assert_cmpint (count_interfaces (info), ==, 0); /* nothing */
  g_dbus_node_info_unref (info);
  g_dbus_object_manager_server_set_connection (manager, c);
  om_check_get_all (c, loop,
                    "({objectpath '/managed/first': {'com.acme.Coyote': {'Mood': <''>}}, '/managed/second': {'org.project.Bar': {'y': <byte 0x00>, 'b': <false>, 'n': <int16 0>, 'q': <uint16 0>, 'i': <0>, 'u': <uint32 0>, 'x': <int64 0>, 't': <uint64 0>, 'd': <0.0>, 's': <''>, 'o': <objectpath '/'>, 'g': <signature ''>, 'ay': <b''>, 'as': <@as []>, 'aay': <@aay []>, 'ao': <@ao []>, 'ag': <@ag []>, 'FinallyNormalName': <''>, 'ReadonlyProperty': <''>, 'unset_i': <0>, 'unset_d': <0.0>, 'unset_s': <''>, 'unset_o': <objectpath '/'>, 'unset_g': <signature ''>, 'unset_ay': <b''>, 'unset_as': <@as []>, 'unset_ao': <@ao []>, 'unset_ag': <@ag []>, 'unset_struct': <(0, 0.0, '', objectpath '/', signature '', @ay [], @as [], @ao [], @ag [])>}, 'org.project.Bat': {'force_i': <0>, 'force_s': <''>, 'force_ay': <@ay []>, 'force_struct': <(0,)>}}},)");

  /* Also check that the ObjectManagerClient returns these objects - and
   * that they are of the right GType cf. what was requested via
   * the generated ::get-proxy-type signal handler
   */
  object_proxies = g_dbus_object_manager_get_objects (pm);
  g_assert (g_list_length (object_proxies) == 2);
  g_list_foreach (object_proxies, (GFunc) g_object_unref, NULL);
  g_list_free (object_proxies);
  op = g_dbus_object_manager_get_object (pm, "/managed/first");
  g_assert (op != NULL);
  g_assert (FOO_IS_OBJECT_PROXY (op));
  g_assert_cmpstr (g_dbus_object_get_object_path (op), ==, "/managed/first");
  proxies = g_dbus_object_get_interfaces (op);
  g_assert (g_list_length (proxies) == 1);
  g_list_foreach (proxies, (GFunc) g_object_unref, NULL);
  g_list_free (proxies);
  p = G_DBUS_PROXY (foo_object_get_com_acme_coyote (FOO_OBJECT (op)));
  g_assert (p != NULL);
  g_assert_cmpint (G_TYPE_FROM_INSTANCE (p), ==, FOO_TYPE_COM_ACME_COYOTE_PROXY);
  g_assert (g_type_is_a (G_TYPE_FROM_INSTANCE (p), FOO_TYPE_COM_ACME_COYOTE));
  g_object_unref (p);
  p = (GDBusProxy *) g_dbus_object_get_interface (op, "org.project.NonExisting");
  g_assert (p == NULL);
  g_object_unref (op);
  /* -- */
  op = g_dbus_object_manager_get_object (pm, "/managed/second");
  g_assert (op != NULL);
  g_assert (FOO_IS_OBJECT_PROXY (op));
  g_assert_cmpstr (g_dbus_object_get_object_path (op), ==, "/managed/second");
  proxies = g_dbus_object_get_interfaces (op);
  g_assert (g_list_length (proxies) == 2);
  g_list_foreach (proxies, (GFunc) g_object_unref, NULL);
  g_list_free (proxies);
  p = G_DBUS_PROXY (foo_object_get_bat (FOO_OBJECT (op)));
  g_assert (p != NULL);
  g_assert_cmpint (G_TYPE_FROM_INSTANCE (p), ==, FOO_TYPE_BAT_PROXY);
  g_assert (g_type_is_a (G_TYPE_FROM_INSTANCE (p), FOO_TYPE_BAT));
  g_object_unref (p);
  p = G_DBUS_PROXY (foo_object_get_bar (FOO_OBJECT (op)));
  g_assert (p != NULL);
  g_assert_cmpint (G_TYPE_FROM_INSTANCE (p), ==, FOO_TYPE_BAR_PROXY);
  g_assert (g_type_is_a (G_TYPE_FROM_INSTANCE (p), FOO_TYPE_BAR));
  /* ... now that we have a Bar instance around, also check that we get signals
   *     and property changes...
   */
  om_check_property_and_signal_emission (loop, bar_skeleton, FOO_BAR (p));
  g_object_unref (p);
  p = (GDBusProxy *) g_dbus_object_get_interface (op, "org.project.NonExisting");
  g_assert (p == NULL);
  g_object_unref (op);

  /* -------------------------------------------------- */

  /* Now remove the second object added above */
  g_dbus_object_manager_server_unexport (manager, "/managed/second");
  /* ... check we get InterfacesRemoved with both interfaces */
  om_data->state = 103;
  g_main_loop_run (om_data->loop);
  g_assert_cmpint (om_data->state, ==, 104);
  g_assert_cmpint (om_data->num_object_proxy_added_signals, ==, 5);
  g_assert_cmpint (om_data->num_object_proxy_removed_signals, ==, 4);
  g_assert_cmpint (om_data->num_interface_added_signals, ==, 1);
  g_assert_cmpint (om_data->num_interface_removed_signals, ==, 1);
  /* ... check introspection data (there should be nothing) */
  info = introspect (c, g_dbus_connection_get_unique_name (c), "/managed/second", loop);
  g_assert_cmpint (count_nodes (info), ==, 0);
  g_assert_cmpint (count_interfaces (info), ==, 0);
  g_dbus_node_info_unref (info);

  /* Check GetManagedObjects() again */
  om_check_get_all (c, loop,
                    "({objectpath '/managed/first': {'com.acme.Coyote': {'Mood': <''>}}},)");

  //g_main_loop_run (loop); /* TODO: tmp */

  g_main_loop_unref (loop);

  g_dbus_connection_signal_unsubscribe (c, om_signal_id);
  g_object_unref (o2);
  g_object_unref (o);
  g_object_unref (manager);
  g_assert_cmpint (g_signal_handlers_disconnect_by_func (pm,
                                                         G_CALLBACK (on_object_proxy_added),
                                                         om_data), ==, 1);
  g_assert_cmpint (g_signal_handlers_disconnect_by_func (pm,
                                                         G_CALLBACK (on_object_proxy_removed),
                                                         om_data), ==, 1);
  g_object_unref (pm);
  g_object_unref (c);

  g_free (om_data);
}

/* ---------------------------------------------------------------------------------------------------- */

static void
test_object_manager (void)
{
  GMainLoop *loop;
  guint id;

  loop = g_main_loop_new (NULL, FALSE);

  id = g_bus_own_name (G_BUS_TYPE_SESSION,
                       "org.gtk.GDBus.BindingsTool.Test",
                       G_BUS_NAME_OWNER_FLAGS_NONE,
                       on_bus_acquired,
                       on_name_acquired,
                       on_name_lost,
                       loop,
                       NULL);

  g_main_loop_run (loop);

  check_object_manager ();

  /* uncomment to keep the service around (to e.g. introspect it) */
  /* g_main_loop_run (loop); */

  g_bus_unown_name (id);
  g_main_loop_unref (loop);
}

/* ---------------------------------------------------------------------------------------------------- */
/* This checks that forcing names via org.gtk.GDBus.Name works (see test-codegen.xml) */

extern gpointer name_forcing_1;
extern gpointer name_forcing_2;
extern gpointer name_forcing_3;
extern gpointer name_forcing_4;
extern gpointer name_forcing_5;
extern gpointer name_forcing_6;
extern gpointer name_forcing_7;
gpointer name_forcing_1 = foo_rocket123_get_type;
gpointer name_forcing_2 = foo_rocket123_call_ignite_xyz;
gpointer name_forcing_3 = foo_rocket123_emit_exploded_xyz;
gpointer name_forcing_4 = foo_rocket123_get_speed_xyz;
gpointer name_forcing_5 = foo_test_ugly_case_interface_call_get_iscsi_servers;
gpointer name_forcing_6 = foo_test_ugly_case_interface_emit_servers_updated_now;
gpointer name_forcing_7 = foo_test_ugly_case_interface_get_ugly_name;

/* ---------------------------------------------------------------------------------------------------- */

/* See https://bugzilla.gnome.org/show_bug.cgi?id=647577#c5 for details */

#define CHECK_FIELD(name, v1, v2) g_assert_cmpint (G_STRUCT_OFFSET (FooChangingInterface##v1##Iface, name), ==, G_STRUCT_OFFSET (FooChangingInterface##v2##Iface, name));

static void
test_interface_stability (void)
{
  CHECK_FIELD(handle_foo_method, V1, V2);
  CHECK_FIELD(handle_bar_method, V1, V2);
  CHECK_FIELD(handle_baz_method, V1, V2);
  CHECK_FIELD(foo_signal, V1, V2);
  CHECK_FIELD(bar_signal, V1, V2);
  CHECK_FIELD(baz_signal, V1, V2);
  CHECK_FIELD(handle_new_method_in2, V2, V10);
  CHECK_FIELD(new_signal_in2, V2, V10);
}

#undef CHECK_FIELD

/* ---------------------------------------------------------------------------------------------------- */

int
main (int   argc,
      char *argv[])
{
  gint ret;

  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  /* all the tests use a session bus with a well-known address that we can bring up and down
   * using session_bus_up() and session_bus_down().
   */
  g_unsetenv ("DISPLAY");
  g_setenv ("DBUS_SESSION_BUS_ADDRESS", session_bus_get_temporary_address (), TRUE);

  session_bus_up ();

  /* TODO: wait a bit for the bus to come up.. ideally session_bus_up() won't return
   * until one can connect to the bus but that's not how things work right now
   */
  usleep (500 * 1000);

  g_test_add_func ("/gdbus/codegen/annotations", test_annotations);
  g_test_add_func ("/gdbus/codegen/interface_stability", test_interface_stability);
  g_test_add_func ("/gdbus/codegen/object-manager", test_object_manager);

  ret = g_test_run();

  /* tear down bus */
  session_bus_down ();

  return ret;
}
