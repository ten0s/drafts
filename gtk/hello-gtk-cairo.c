#include <gtk/gtk.h>
#include <cairo.h>

gboolean on_draw(GtkWidget *widget,
                 GdkEventExpose *event,
                 gpointer data)
{
    // "convert" the G*t*kWidget to G*d*kWindow (no, it's not a GtkWindow!)
    GdkWindow* window = gtk_widget_get_window(widget);  

    cairo_region_t * cairoRegion = cairo_region_create();

    GdkDrawingContext * drawingContext;
    drawingContext = gdk_window_begin_draw_frame (window,cairoRegion);
    {
        // say: "I want to start drawing"
        cairo_t * cr = gdk_drawing_context_get_cairo_context(drawingContext);
        { // do your drawing
            cairo_move_to(cr, 10, 20);
            cairo_set_font_size(cr, 15);
            cairo_show_text(cr, "Hello from Cairo!");
        }

        // say: "I'm finished drawing
        gdk_window_end_draw_frame(window, drawingContext);
    }

    // cleanup
    cairo_region_destroy(cairoRegion);

    return FALSE;
}

int main (int argc, char * argv[]) {
    gtk_init(&argc, &argv);

    GtkWindow * window;
    { // window setup
        window = (GtkWindow*) gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_default_size(window, 150, 100);
        gtk_window_set_position    (window, GTK_WIN_POS_CENTER);
        gtk_window_set_title       (window, "Drawing");

        g_signal_connect(window, "destroy", gtk_main_quit, NULL);
    }  

    // create the are we can draw in
    GtkDrawingArea* drawingArea;
    {
        drawingArea = (GtkDrawingArea*) gtk_drawing_area_new();
        gtk_container_add(GTK_CONTAINER(window), (GtkWidget*)drawingArea);

        g_signal_connect((GtkWidget*)drawingArea, "draw", G_CALLBACK(on_draw), NULL);
    }  

    gtk_widget_show_all((GtkWidget*)window);
    gtk_main();

    return 0;
}
