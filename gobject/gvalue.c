#include <glib-object.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>

int main(int argc, char *argv[])
{
    g_printf("G_TYPE_STRING: %ld\n"
             "G_TYPE_INT   : %ld\n",
             G_TYPE_STRING,
             G_TYPE_INT);

    // GValues must be initialized
    GValue a = G_VALUE_INIT;

    // The GValue starts empty
    g_assert(!G_VALUE_HOLDS_STRING (&a));

    // Put a string in it
    g_value_init(&a, G_TYPE_STRING);
    g_assert(G_VALUE_HOLDS_STRING (&a));

    g_value_set_string(&a, "Hello, world!");
    g_printf("%s\n", g_value_get_string(&a));

    // Release all resources associated with this GValue
    g_value_unset(&a);

    g_value_init(&a, G_TYPE_INT);
    g_value_set_int(&a, 42);
    g_printf("%d\n", g_value_get_int(&a));
    g_value_unset(&a);

    // Init GDK?
    //gint gdk_argc = 0;
    //gchar **gdk_argv = NULL;
    //gdk_init(&gdk_argc, &gdk_argv);

    /*
      https://github.com/ten0s/node-gtk/tree/windows-build
      See also gvalue-from-gir.c

      const color0 = new Gdk.RGBA({ red: 0.5, blue: 0.5, green: 0.5, alpha: 0.5 })
      const type = GObject.typeFromName('GdkRGBA')

      const val = new GObject.Value()
      val.init(type)
      val.setBoxed(color0)

      color0.toString() => 'rgba(128,128,128,0.5)'
      const color1 = val.getBoxed()
      color1.toString() => 'rgba(128,128,128,0.5)'
   */

    GdkRGBA color0 = { .red = 0.5, .blue = 0.5, .green = 0.5, .alpha = 0.5 };

    GType type1 = gdk_rgba_get_type();
    g_printf("GdkRGBA type1: %ld\n", type1);

    // Returns 0 without the call above.
    // Works if loaded from GIR. See also gvalue-from-gir.c
    GType type2 = g_type_from_name("GdkRGBA");
    g_printf("GdkRGBA type2: %ld\n", type2);

    GValue b = G_VALUE_INIT;
    g_value_init(&b, type1);
    g_value_set_boxed(&b, &color0);

    gchar *str0 = gdk_rgba_to_string(&color0);
    g_printf("GdkRGBA str0: %s\n", str0);

    GdkRGBA *color1 = (GdkRGBA *)g_value_get_boxed(&b);
    gchar *str1 = gdk_rgba_to_string(color1);
    g_printf("GdkRGBA str1: %s\n", str1);

    g_free(str0);
    g_free(str1);
    g_value_unset(&b);

    return 0;
}
