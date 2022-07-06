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

    // Init GDK
    gint gdk_argc = 0;
    gchar **gdk_argv = NULL;
    gdk_init(&gdk_argc, &gdk_argv);

    GdkRGBA rgba = { .red = 0.5, .blue = 0.5, .green = 0.5, .alpha = 0.5 };

    GValue b = G_VALUE_INIT;

    GType type1 = gdk_rgba_get_type();
    g_printf("GdkRGBA type1: %ld\n", type1);

    // Returns 0 without the call above :(
    GType type2 = g_type_from_name("GdkRGBA");
    g_printf("GdkRGBA type2: %ld\n", type2);

    return 0;
}
