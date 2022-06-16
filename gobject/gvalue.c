#include <glib-object.h>
#include <glib/gprintf.h>

int main (int argc, char *argv[])
{
  // GValues must be initialized
  GValue a = G_VALUE_INIT;

  // The GValue starts empty
  g_assert (!G_VALUE_HOLDS_STRING (&a));

  // Put a string in it
  g_value_init (&a, G_TYPE_STRING);
  g_assert (G_VALUE_HOLDS_STRING (&a));

  g_value_set_static_string (&a, "Hello, world!");
  g_printf ("%s\n", g_value_get_string (&a));

  // Release all resources associated with this GValue
  g_value_unset (&a);

  return 0;
}
