/* GLib testing framework examples and tests
 * Copyright (C) 2008 Red Hat, Inc.
 * Authors: Tomas Bzatek <tbzatek@redhat.com>
 *
 * This work is provided "as is"; redistribution and modification
 * in whole or in part, in any medium, physical or electronic is
 * permitted without restriction.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In no event shall the authors or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 */

#include <glib/glib.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 	0xFFF
#define MAX_BYTES	0x10000	

static void
test_seek_to_start (GInputStream *stream)
{
  GError *error = NULL;
  gboolean res = g_seekable_seek (G_SEEKABLE (stream), 0, G_SEEK_SET, NULL, &error);
  g_assert_cmpint (res, ==, TRUE);
  g_assert (error == NULL);
}

static void
test_read_lines (GDataStreamNewlineType newline_type)
{
  GInputStream *stream;
  GInputStream *base_stream;
  GError *error = NULL;
  char *data;
  int line;
  const char* lines[MAX_LINES];
  const char* endl[4] = {"\n", "\r", "\r\n", "\n"};
  
  /*  prepare data */
  int i;
  for (i = 0; i < MAX_LINES; i++)
    lines[i] = "some_text";
	
  base_stream = g_memory_input_stream_new ();
  g_assert (base_stream != NULL);
  stream = G_INPUT_STREAM (g_data_input_stream_new (base_stream));
  g_assert(stream != NULL);
	
  /*  Byte order testing */
  g_data_input_stream_set_byte_order (G_DATA_INPUT_STREAM (stream), G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN);
  g_assert_cmpint (g_data_input_stream_get_byte_order (G_DATA_INPUT_STREAM (stream)), ==, G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN);
  g_data_input_stream_set_byte_order (G_DATA_INPUT_STREAM (stream), G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN);
  g_assert_cmpint (g_data_input_stream_get_byte_order (G_DATA_INPUT_STREAM (stream)), ==, G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN);
  
  /*  Line ends testing */
  g_data_input_stream_set_newline_type (G_DATA_INPUT_STREAM (stream), newline_type);
  g_assert_cmpint (g_data_input_stream_get_newline_type (G_DATA_INPUT_STREAM (stream)), ==, newline_type);
	

  /*  Add sample data */
  for (i = 0; i < MAX_LINES; i++) 
    g_memory_input_stream_add_data (G_MEMORY_INPUT_STREAM (base_stream),
				    g_strconcat (lines[i], endl[newline_type], NULL), -1, NULL);

  /*  Seek to the start */
  test_seek_to_start (base_stream);
	
  /*  Test read line */
  error = NULL;
  data = (char*)1;
  line = 0;
  while (data)
    {
      gsize length = -1;
      data = g_data_input_stream_read_line (G_DATA_INPUT_STREAM (stream), &length, NULL, &error);
      if (data)
	{
	  g_assert_cmpstr (data, ==, lines[line]);
	  g_assert (error == NULL);
	  line++;
	}
    }
  g_assert_cmpint (line, ==, MAX_LINES);
  
  
  g_object_unref (base_stream);
  g_object_unref (stream);
}

static void
test_read_lines_LF (void)
{
  test_read_lines (G_DATA_STREAM_NEWLINE_TYPE_LF);
}

static void
test_read_lines_CR (void)
{
  test_read_lines (G_DATA_STREAM_NEWLINE_TYPE_CR);
}

static void
test_read_lines_CR_LF (void)
{
  test_read_lines (G_DATA_STREAM_NEWLINE_TYPE_CR_LF);
}


static void
test_read_until (void)
{
  GInputStream *stream;
  GInputStream *base_stream;
  GError *error = NULL;
  char *data;
  int line;
  int i;
  
#define REPEATS			10   /* number of rounds */
#define DATA_STRING		" part1 # part2 $ part3 % part4 ^"
#define DATA_PART_LEN		7    /* number of characters between separators */
#define DATA_SEP		"#$%^"
  const int DATA_PARTS_NUM = strlen (DATA_SEP) * REPEATS;
  
  base_stream = g_memory_input_stream_new ();
  stream = G_INPUT_STREAM (g_data_input_stream_new (base_stream));
  
  for (i = 0; i < REPEATS; i++)
    g_memory_input_stream_add_data (G_MEMORY_INPUT_STREAM (base_stream), DATA_STRING, -1, NULL);
  
  /*  Test stop characters */
  error = NULL;
  data = (char*)1;
  line = 0;
  while (data)
    {
      gsize length = -1;
      data = g_data_input_stream_read_until (G_DATA_INPUT_STREAM (stream), DATA_SEP, &length, NULL, &error);
      if (data)
	{
	  g_assert_cmpint (strlen (data), ==, DATA_PART_LEN);
	  g_assert (error == NULL);
	  line++;
	}
    }
  g_assert (error == NULL);
  g_assert_cmpint (line, ==, DATA_PARTS_NUM);
	
	
  g_object_unref (base_stream);
  g_object_unref (stream);
}

enum TestDataType {
  TEST_DATA_BYTE = 0,
  TEST_DATA_INT16,
  TEST_DATA_UINT16,
  TEST_DATA_INT32,
  TEST_DATA_UINT32,
  TEST_DATA_INT64,
  TEST_DATA_UINT64
};

#define TEST_DATA_RETYPE_BUFF(a, v)	\
	 (a == TEST_DATA_BYTE	? *(guchar*)v : \
	 (a == TEST_DATA_INT16	? *(gint16*)v :	 \
	 (a == TEST_DATA_UINT16	? *(guint16*)v : \
	 (a == TEST_DATA_INT32	? *(gint32*)v :	 \
	 (a == TEST_DATA_UINT32	? *(guint32*)v : \
	 (a == TEST_DATA_INT64	? *(gint64*)v :	 \
	 *(guint64*)v )))))) 


static void
test_data_array (GInputStream *stream, GInputStream *base_stream,
		 gpointer buffer, int len,
		 enum TestDataType data_type, GDataStreamByteOrder byte_order)
{
  GError *error = NULL;
  int pos = 0;
  int data_size = 1;
  gint64 data;
  GDataStreamByteOrder native;
  gboolean swap;
  
  /*  Seek to start */
  test_seek_to_start (base_stream);

  /*  Set correct data size */
  switch (data_type)
    {
    case TEST_DATA_BYTE:
      data_size = 1;
      break;
    case TEST_DATA_INT16:
    case TEST_DATA_UINT16:
      data_size = 2;
      break;
    case TEST_DATA_INT32:
    case TEST_DATA_UINT32:
      data_size = 4;
      break;
    case TEST_DATA_INT64:
    case TEST_DATA_UINT64:
      data_size = 8;
      break; 
    }

  /*  Set flag to swap bytes if needed */
  native = (G_BYTE_ORDER == G_BIG_ENDIAN) ? G_DATA_STREAM_BYTE_ORDER_BIG_ENDIAN : G_DATA_STREAM_BYTE_ORDER_LITTLE_ENDIAN;
  swap = (byte_order != G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN) && (byte_order != native);

  data = 1;
  while (data != 0)
    {
      switch (data_type)
	{
	case TEST_DATA_BYTE:
	  data = g_data_input_stream_read_byte (G_DATA_INPUT_STREAM (stream), NULL, &error);
	  break;
	case TEST_DATA_INT16:
	  data = g_data_input_stream_read_int16 (G_DATA_INPUT_STREAM (stream), NULL, &error);
	  if (swap)
	    data = (gint16)GUINT16_SWAP_LE_BE((gint16)data);
	  break;
	case TEST_DATA_UINT16:
	  data = g_data_input_stream_read_uint16 (G_DATA_INPUT_STREAM (stream), NULL, &error);
	  if (swap)
	    data = (guint16)GUINT16_SWAP_LE_BE((guint16)data);
	  break;
	case TEST_DATA_INT32:
	  data = g_data_input_stream_read_int32 (G_DATA_INPUT_STREAM (stream), NULL, &error);
	  if (swap)
	    data = (gint32)GUINT32_SWAP_LE_BE((gint32)data);
	  break;
	case TEST_DATA_UINT32:
	  data = g_data_input_stream_read_uint32 (G_DATA_INPUT_STREAM (stream), NULL, &error);
	  if (swap)
	    data = (guint32)GUINT32_SWAP_LE_BE((guint32)data);
	  break;
	case TEST_DATA_INT64:
	  data = g_data_input_stream_read_int64 (G_DATA_INPUT_STREAM (stream), NULL, &error);
	  if (swap)
	    data = (gint64)GUINT64_SWAP_LE_BE((gint64)data);
	  break;
	case TEST_DATA_UINT64:
	  data = g_data_input_stream_read_uint64 (G_DATA_INPUT_STREAM (stream), NULL, &error);
	  if (swap)
	    data = (guint64)GUINT64_SWAP_LE_BE((guint64)data);
	  break;
	}
      if ((data) && (! error))  
	g_assert_cmpint (data, ==, TEST_DATA_RETYPE_BUFF(data_type, ((guchar*)buffer + pos)));
      
      pos += data_size;
    }
  if (pos < len + 1)
    g_assert (error == NULL);
  g_assert_cmpint (pos - data_size, ==, len);
}

static void
test_read_int (void)
{
  GInputStream *stream;
  GInputStream *base_stream;
  GRand *rand;
  int i;
  gpointer buffer;
  
  rand = g_rand_new ();
  buffer = g_malloc0 (MAX_BYTES);
  
  /*  Fill in some random data */
  for (i = 0; i < MAX_BYTES; i++)
    {
      guchar x = 0;
      while (! x)
	x = (guchar)g_rand_int (rand);
      *(guchar*)((guchar*)buffer + sizeof(guchar) * i) = x; 
    }

  base_stream = g_memory_input_stream_new ();
  stream = G_INPUT_STREAM (g_data_input_stream_new (base_stream));
  g_memory_input_stream_add_data (G_MEMORY_INPUT_STREAM (base_stream), buffer, MAX_BYTES, NULL);
  
  
  for (i = 0; i < 3; i++)
    {
      int j;
      g_data_input_stream_set_byte_order (G_DATA_INPUT_STREAM (stream), i);
      
      for (j = 0; j <= TEST_DATA_UINT64; j++)
	test_data_array (stream, base_stream, buffer, MAX_BYTES, j, i);
    }

  g_object_unref (base_stream);
  g_object_unref (stream);
  g_rand_free (rand);
  g_free (buffer);
}


int
main (int   argc,
      char *argv[])
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/data-input-stream/read-lines-LF", test_read_lines_LF);
  g_test_add_func ("/data-input-stream/read-lines-CR", test_read_lines_CR);
  g_test_add_func ("/data-input-stream/read-lines-CR-LF", test_read_lines_CR_LF);
  g_test_add_func ("/data-input-stream/read-until", test_read_until);
  g_test_add_func ("/data-input-stream/read-int", test_read_int);

  return g_test_run();
}
