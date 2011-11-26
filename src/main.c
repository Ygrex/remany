#include <gtk/gtk.h>
#include <stdlib.h>

#include "data.h"
#include "log.h"
#include "sprite.h"

#include "canvas.h"

/* {{{ init_window_pref(...) - set initial properties for the man window */
static GtkWidget *init_window_pref(GtkWidget *win) {
	gtk_window_set_title((void *)win, "Remany");
	return win;
}
/* }}} init_window_pref() */

/* {{{ init_window() - create parent window */
static GtkWidget *init_window() {
	debug(">>> init_window()");
	GtkWidget *win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_assert(win != NULL);
	g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	GtkWidget *canvas = new_canvas();
	g_assert(canvas);
	gtk_container_add((void *)win, canvas);
	debug("<<< init_window()");
	return init_window_pref(win);
}
/* }}} init_window() */

/* {{{ main(…) - entry point */
int main(int argc, char **argv) {
	debug(">>> main()");
	gtk_init (&argc, &argv);
	gtk_widget_show_all(init_window());
	gtk_main ();
	debug("<<< main()");
	return 0;
}
/* }}} main() */

