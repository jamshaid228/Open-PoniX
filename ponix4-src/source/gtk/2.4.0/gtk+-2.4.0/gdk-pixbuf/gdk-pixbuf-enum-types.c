
/* Generated data (by glib-mkenums) */

#include <gdk-pixbuf/gdk-pixbuf.h>

/* enumerations from "gdk-pixbuf.h" */
GType
gdk_pixbuf_alpha_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDK_PIXBUF_ALPHA_BILEVEL, "GDK_PIXBUF_ALPHA_BILEVEL", "bilevel" },
      { GDK_PIXBUF_ALPHA_FULL, "GDK_PIXBUF_ALPHA_FULL", "full" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GdkPixbufAlphaMode", values);
  }
  return etype;
}

GType
gdk_colorspace_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDK_COLORSPACE_RGB, "GDK_COLORSPACE_RGB", "rgb" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GdkColorspace", values);
  }
  return etype;
}

GType
gdk_pixbuf_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDK_PIXBUF_ERROR_CORRUPT_IMAGE, "GDK_PIXBUF_ERROR_CORRUPT_IMAGE", "corrupt-image" },
      { GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY, "GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY", "insufficient-memory" },
      { GDK_PIXBUF_ERROR_BAD_OPTION, "GDK_PIXBUF_ERROR_BAD_OPTION", "bad-option" },
      { GDK_PIXBUF_ERROR_UNKNOWN_TYPE, "GDK_PIXBUF_ERROR_UNKNOWN_TYPE", "unknown-type" },
      { GDK_PIXBUF_ERROR_UNSUPPORTED_OPERATION, "GDK_PIXBUF_ERROR_UNSUPPORTED_OPERATION", "unsupported-operation" },
      { GDK_PIXBUF_ERROR_FAILED, "GDK_PIXBUF_ERROR_FAILED", "failed" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GdkPixbufError", values);
  }
  return etype;
}

GType
gdk_interp_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GDK_INTERP_NEAREST, "GDK_INTERP_NEAREST", "nearest" },
      { GDK_INTERP_TILES, "GDK_INTERP_TILES", "tiles" },
      { GDK_INTERP_BILINEAR, "GDK_INTERP_BILINEAR", "bilinear" },
      { GDK_INTERP_HYPER, "GDK_INTERP_HYPER", "hyper" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GdkInterpType", values);
  }
  return etype;
}


/* Generated data ends here */

