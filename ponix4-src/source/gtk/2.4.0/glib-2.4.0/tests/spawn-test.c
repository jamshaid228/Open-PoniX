/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
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

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

#undef G_DISABLE_ASSERT
#undef G_LOG_DOMAIN

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef G_OS_WIN32
#include <fcntl.h>
#include <io.h>
#endif


static void
run_tests (void)
{
  GError *err;
  gchar *output = NULL;
#ifdef G_OS_WIN32
  gchar *erroutput = NULL;
  int pipedown[2], pipeup[2];
  gchar **argv = 0;
#endif
  
  printf ("The following errors are supposed to occur:\n");

  err = NULL;
  if (!g_spawn_command_line_sync ("nonexistent_application foo 'bar baz' blah blah",
                                  NULL, NULL, NULL,
                                  &err))
    {
      fprintf (stderr, "Error (normal, supposed to happen): %s\n", err->message);
      g_error_free (err);
    }

  err = NULL;
  if (!g_spawn_command_line_async ("nonexistent_application foo bar baz \"blah blah\"",
                                   &err))
    {
      fprintf (stderr, "Error (normal, supposed to happen): %s\n", err->message);
      g_error_free (err);
    }

  printf ("Errors after this are not supposed to happen:\n");
  
  err = NULL;
#ifdef G_OS_UNIX
  if (!g_spawn_command_line_sync ("/bin/sh -c 'echo hello'",
                                  &output, NULL, NULL,
                                  &err))
    {
      fprintf (stderr, "Error: %s\n", err->message);
      g_error_free (err);
      exit (1);
    }
  else
    {
      g_assert (output != NULL);
      
      if (strcmp (output, "hello\n") != 0)
        {
          printf ("output was '%s', should have been 'hello'\n",
                  output);

          exit (1);
        }

      g_free (output);
    }
#else
#ifdef G_OS_WIN32
  printf ("Running ipconfig synchronously, collecting its output\n");

  if (!g_spawn_command_line_sync ("ipconfig /all",
                                  &output, &erroutput, NULL,
                                  &err))
    {
      fprintf (stderr, "Error: %s\n", err->message);
      g_error_free (err);
      exit (1);
    }
  else
    {
      g_assert (output != NULL);
      g_assert (erroutput != NULL);
      
      if (strstr (output, "IP Configuration") == 0)
        {
          printf ("output was '%s', should have contained 'IP Configuration'\n",
                  output);

          exit (1);
        }
      if (erroutput[0] != '\0')
	{
	  printf ("error output was '%s', should have been empty\n",
		  erroutput);
	  exit (1);
	}

      g_free (output);
      output = NULL;
      g_free (erroutput);
      erroutput = NULL;
    }

  printf ("Running spawn-test-win32-gui in various ways. Click on the OK buttons.\n");

  printf ("First asynchronously (without wait).\n");

  if (!g_spawn_command_line_async ("'.\\spawn-test-win32-gui.exe' 1", &err))
    {
      fprintf (stderr, "Error: %s\n", err->message);
      g_error_free (err);
      exit (1);
    }

  printf ("Now synchronously, collecting its output.\n");
  if (!g_spawn_command_line_sync ("'.\\spawn-test-win32-gui.exe' 2",
				  &output, &erroutput, NULL,
				  &err))
    {
      fprintf (stderr, "Error: %s\n", err->message);
      g_error_free (err);
      exit (1);
    }
  else
    {
      g_assert (output != NULL);
      g_assert (erroutput != NULL);
      
      if (strcmp (output, "This is stdout\r\n") != 0)
        {
          printf ("output was '%s', should have been 'This is stdout'\n",
                  g_strescape (output, NULL));

          exit (1);
        }
      if (strcmp (erroutput, "This is stderr\r\n") != 0)
	{
	  printf ("error output was '%s', should have been 'This is stderr'\n",
		  g_strescape (erroutput, NULL));
	  exit (1);
	}

      g_free (output);
      g_free (erroutput);
    }

  printf ("Now with G_SPAWN_FILE_AND_ARGV_ZERO.\n");

  if (!g_shell_parse_argv ("'.\\spawn-test-win32-gui.exe' this-should-be-argv-zero nop", NULL, &argv, &err))
    {
      fprintf (stderr, "Error parsing command line? %s\n", err->message);
      g_error_free (err);
      exit (1);
    }

  if (!g_spawn_async (NULL, argv, NULL,
		      G_SPAWN_FILE_AND_ARGV_ZERO,
		      NULL, NULL, NULL,
		      &err))
    {
      fprintf (stderr, "Error: %s\n", err->message);
      g_error_free (err);
      exit (1);
    }

  printf ("Now talking to it through pipes.\n");

  if (pipe (pipedown) < 0 ||
      pipe (pipeup) < 0)
    {
      fprintf (stderr, "Could not create pipes\n");
      exit (1);
    }

  if (!g_shell_parse_argv (g_strdup_printf ("'.\\spawn-test-win32-gui.exe' pipes %d %d",
					    pipedown[0], pipeup[1]),
                           NULL, &argv,
                           &err))
    {
      fprintf (stderr, "Error parsing command line? %s\n", err->message);
      g_error_free (err);
      exit (1);
    }
  
  if (!g_spawn_async (NULL, argv, NULL,
		      G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
		      G_SPAWN_DO_NOT_REAP_CHILD,
		      NULL, NULL, NULL,
		      &err))
    {
      fprintf (stderr, "Error: %s\n", err->message);
      g_error_free (err);
      exit (1);
    }
  else
    {
      int k, n;
      char buf[100];

      if ((k = read (pipeup[0], &n, sizeof (n))) != sizeof (n))
	{
	  if (k == -1)
	    fprintf (stderr, "Read error: %s\n", g_strerror (errno));
	  else
	    fprintf (stderr, "Wanted to read %d bytes, got %d\n",
		     sizeof (n), k);
	  exit (1);
	}

      if ((k = read (pipeup[0], buf, n)) != n)
	{
	  if (k == -1)
	    fprintf (stderr, "Read error: %s\n", g_strerror (errno));
	  else
	    fprintf (stderr, "Wanted to read %d bytes, got %d\n",
		     n, k);
	  exit (1);
	}

      n = strlen ("Bye then");
      if (write (pipedown[1], &n, sizeof (n)) == -1 ||
	  write (pipedown[1], "Bye then", n) == -1)
	{
	  fprintf (stderr, "Write error: %s\n", g_strerror (errno));
	  exit (1);
	}

      if ((k = read (pipeup[0], &n, sizeof (n))) != sizeof (n))
	{
	  if (k == -1)
	    fprintf (stderr, "Read error: %s\n", g_strerror (errno));
	  else
	    fprintf (stderr, "Wanted to read %d bytes, got %d\n",
		     sizeof (n), k);
	  exit (1);
	}

      if ((k = read (pipeup[0], buf, n)) != n)
	{
	  if (k == -1)
	    fprintf (stderr, "Read error: %s\n", g_strerror (errno));
	  else
	    fprintf (stderr, "Wanted to read %d bytes, got %d\n",
		     n, k);
	  exit (1);
	}
    }
#endif
#endif
}

int
main (int   argc,
      char *argv[])
{
  run_tests ();
  
  return 0;
}
