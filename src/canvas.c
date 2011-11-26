#include <gtk/gtk.h>
#include <stdlib.h>

#include "data.h"
#include "log.h"
#include "sprite.h"

#include "canvas.h"

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
		goto LBL_return;
	}
	/* should fail with len = 0 */
	GdkAtom *targets = malloc(sizeof(GdkAtom) * len);
	if (!targets) {
		warn("ctx_targets_types(): malloc()");
		goto LBL_return;
	}
	GdkAtom *t = targets;
	do {
		*(t++) = GDK_POINTER_TO_ATOM(list->data);
		g_free( debug( (char *) gdk_atom_name(t[-1]) ) );
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
	LBL_return:
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
	Sprite *sprite = new_sprite(data, targets, time, ud);
	if (sprite == NULL) {
		warn("create_sprite(): new_sprite()");
		goto LBL_return;
	}
	/* FIXME naive assumption the canvas is of GtkFixed */
	gtk_fixed_put((GtkFixed *)canvas, sprite->widget, x, y);
	gtk_widget_show_all(sprite->widget);
	LBL_return:
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
	gtk_drag_finish(ctx, TRUE, FALSE, time);
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
	if(! init_canvas_dnd(canvas) ) {
		warn("init_canvas_signals(): init_canvas_dnd()");
		goto LBL_return;
	};
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
	LBL_return:
	debug("<<< init_canvas_signals()");
	return canvas;
}
/* }}} init_canvas_signals() */

/* {{{ new_canvas() - create container for anything */
GtkWidget *new_canvas() {
	GtkWidget *canvas;
	GtkWidget *scrwin;
	debug(">>> new_canvas()");
	/* construct necessary widgets */
	canvas = gtk_fixed_new();
	if (! canvas) {
		warn("new_canvas(): gtk_fixed_new()");
		goto LBL_return;
	};
	scrwin = gtk_scrolled_window_new(NULL, NULL);
	if (! scrwin) {
		warn("new_canvas(): gtk_scrolled_window_new()");
		g_object_unref(canvas);
		goto LBL_return;
	};
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
	LBL_return:
	debug("<<< new_canvas()");
	return scrwin;
}
/* }}} new_canvas() */

