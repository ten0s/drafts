#include <gtk/gtk.h>

void hello() {
  g_print("Hello World\n");
}

void destroy() {
  gtk_main_quit();
}

int main (int argc, char *argv[]) {
  GtkWidget *window;
  GtkWidget *button;

  gtk_init (&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);

  button = gtk_button_new_with_label("Hello World");
  g_signal_connect(button, "clicked", G_CALLBACK(hello), NULL);

  gtk_container_add(GTK_CONTAINER(window), button);

  gtk_widget_show_all((GtkWidget*)window);
  gtk_main();

  return 0;
}
