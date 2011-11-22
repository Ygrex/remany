#include "main.h"

/* {{{ dnd_motion_canvas(…) - handle "drag-motion" for the canvas */
static gboolean dnd_motion_canvas(
	GtkWidget *canvas,
	GdkDragContext* ctx,
	gint x,
	gint y,
	guint time
) {
	debug(">>> dnd_motion_canvas()");
	(void)canvas;
	(void)ctx;
	(void)x;
	(void)y;
	(void)time;
	debug("<<< dnd_motion_canvas()");
	return TRUE;
}
/* }}} dnd_motion_canvas() */

/* {{{ ctx_targets_types(…) - return map of targets types of the context */
static gint ctx_targets_types(GdkDragContext *ctx) {
	debug(">>> ctx_targets_types()");
	GList *list = gdk_drag_context_list_targets(ctx);
	gint len = g_list_length(list);
	int ret = 0;
	list = g_list_first(list);
	if (!list) {
		warn("ctx_targets_types(): g_list_first()");
		goto LBL_ctx_targets_types;
	}
	GdkAtom *targets = malloc(sizeof(GdkAtom) * len);
	if (!targets) {
		warn("ctx_targets_types(): malloc()");
		goto LBL_ctx_targets_types;
	}
	GdkAtom *t = targets;
	do {
		*(t++) = GDK_POINTER_TO_ATOM(list->data);
	} while ( (list = g_list_next(list)) );
	ret =
		weight(Data_Image,
			gtk_targets_include_image(targets, len, FALSE) ) |
		weight(Data_Text,
			gtk_targets_include_text(targets, len) ) |
		weight(Data_Uri,
			gtk_targets_include_uri(targets, len) ) |
		/* FIXME rich text must be supported */
		0;
	free(targets);
	LBL_ctx_targets_types:
	debug("<<< ctx_targets_types()");
	return ret;
}
/* }}} ctx_targets_types() */

/* {{{ create_sprite(…) - create a sprite on a canvas */
/*	targets - map of transferred types to display */
static void create_sprite(
	GtkWidget *canvas,
	GdkDragContext *ctx,
	gint x,
	gint y,
	GtkSelectionData *data,
	int targets,
	guint time,
	gpointer ud
) {
	debug(">>> create_sprite()");
	(void)canvas;
	(void)ctx;
	(void)x;
	(void)y;
	(void)data;
	(void)targets;
	(void)time;
	(void)ud;
	if (! (targets & Data_Image)) {
		warn("Only images are accepted for now.");
		goto LBL_create_sprite;
	}
	GdkPixbuf *pic = gtk_selection_data_get_pixbuf(data);
	if (pic == NULL) {
		warn("Image data has not been converted.");
		goto LBL_create_sprite;
	}
	/* FIXME animated images should be supported as well */
	GtkWidget *widget = gtk_image_new_from_pixbuf(pic);
	if (widget == NULL) {
		warn("GtkImage widget creation failed.");
		goto LBL_create_sprite;
	}
	/* FIXME naive assumption the canvas is of GtkFixed */
	gtk_fixed_put((GtkFixed *)canvas, widget, x, y);
	gtk_widget_show(widget);
	LBL_create_sprite:
	debug("<<< create_sprite()");
}
/* }}} create_sprite() */

/* {{{ dnd_receive_canvas(…) - treat incoming D'n'D */
static void dnd_receive_canvas(
	GtkWidget *canvas,
	GdkDragContext *ctx,
	gint x,
	gint y,
	GtkSelectionData *data,
	guint info,
	guint time,
	gpointer ud
) {
	debug(">>> dnd_receive_canvas()");
	info = (guint)ctx_targets_types(ctx);
	if (! info) {
		debug("dnd_receive_canvas(): D'n'D transferred nothing");
	} else {
		create_sprite(canvas, ctx, x, y, data, info, time, ud);
	}
	debug("<<< dnd_receive_canvas()");
}
/* }}} dnd_receive_canvas() */

/* {{{ dnd_canvas(…) - handle D'n'D upon the canvas */
static gboolean dnd_canvas(
	GtkWidget *canvas,
	GdkDragContext *ctx,
	gint x,
	gint y,
	guint time,
	gpointer ud
) {
	(void)canvas;
	(void)ctx;
	(void)x;
	(void)y;
	(void)time;
	(void)ud;
	debug(">>> dnd_canvas()");
	gtk_drag_finish(ctx, TRUE, TRUE, time);
	debug("<<< dnd_canvas()");
	return TRUE;
}
/* }}} dnd_canvas() */

/* {{{ init_canvas_dnd(…) - attach D'n'D handlers to the canvas */
static GtkWidget *init_canvas_dnd(GtkWidget *canvas) {
	debug(">>> init_canvas_dnd()");
	gtk_drag_dest_set(
		canvas,
		GTK_DEST_DEFAULT_ALL,
		NULL,
		0,
		GDK_ACTION_COPY
	);
	gtk_drag_dest_add_text_targets(canvas);
	gtk_drag_dest_add_image_targets(canvas);
	gtk_drag_dest_add_uri_targets(canvas);
	g_signal_connect(
		canvas,
		"drag-drop",
		G_CALLBACK(dnd_canvas),
		NULL
	);
	g_signal_connect(
		canvas,
		"drag-data-received",
		G_CALLBACK(dnd_receive_canvas),
		NULL
	);
	g_signal_connect(
		canvas,
		"drag-motion",
		G_CALLBACK(dnd_motion_canvas),
		NULL
	);
	debug("<<< init_canvas_dnd()");
	return canvas;
}
/* }}} init_canvas_dnd() */

/* {{{ init_canvas_signals(…) - init signal handlers for the canvas */
static GtkWidget *init_canvas_signals(GtkWidget *canvas) {
	debug(">>> init_canvas_signals()");
	g_assert( init_canvas_dnd(canvas) );
	/*
	g_assert( gtk_widget_get_window(canvas) );
	gdk_window_set_events(
		gtk_widget_get_window(canvas),
		GDK_BUTTON_PRESS_MASK
	);
	g_signal_connect(
		canvas,
		"button-press-event",
		G_CALLBACK(button_press_canvas),
		NULL
	);
	*/
	debug("<<< init_canvas_signals()");
	return canvas;
}
/* }}} init_canvas_signals() */

/* {{{ init_canvas() - create container for anything */
static GtkWidget *init_canvas() {
	GtkWidget *canvas;
	GtkWidget *scrwin;
	debug(">>> init_canvas()");
	/* construct necessary widgets */
	canvas = gtk_fixed_new();
	g_assert(canvas != NULL);
	scrwin = gtk_scrolled_window_new(NULL, NULL);
	g_assert(scrwin != NULL);
	/* set initial size for the canvas */
	gtk_widget_set_size_request(canvas, 128, 128);
	/* size policy for the canvas */
	gtk_widget_set_vexpand((void *)canvas, TRUE);
	gtk_widget_set_hexpand((void *)canvas, TRUE);
	gtk_widget_set_vexpand_set((void *)canvas, TRUE);
	gtk_widget_set_hexpand_set((void *)canvas, TRUE);
	/* link the widgets */
	gtk_scrolled_window_add_with_viewport((void *) scrwin, canvas);
	/* append signal handlers */
	init_canvas_signals(canvas);
	debug("<<< init_canvas()");
	return scrwin;
}
/* }}} init_canvas() */

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
	gtk_container_add((void *)win, init_canvas());
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

