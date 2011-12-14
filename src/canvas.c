#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "log.h"
#include "sprite.h"

#include "canvas.h"

/* {{{ position_canvas_popup(…) - GtkMenuPositionFunc for a popup menu */
static void position_canvas_popup(
	GtkMenu *menu,
	gint *x,
	gint *y,
	gboolean *push_in,
	gpointer ud
) {
	GValue *try = NULL;
	debug(">>> position_canvas_popup()");
	(void)ud;
	try = g_object_get_data(G_OBJECT(menu), "menu_x");
	if (try) {
		*x = g_value_get_int(try);
	} else {
		*x = 0;
	}
	try = g_object_get_data(G_OBJECT(menu), "menu_y");
	if (try) {
		*y = g_value_get_int(try);
	} else {
		*y = 0;
	}
	*push_in = FALSE;
	debug("<<< position_canvas_popup()");
}
/* }}} position_canvas_popup() */

/* {{{ g_value_destroy(…) - destroy the instance of GValue */
static void g_value_destroy(GValue *v) {
	debug(">>> g_value_destroy()");
	g_value_unset(v);
	free(v);
	debug("<<< g_value_destroy()");
}
/* }}} g_value_destroy() */

/* {{{ show_canvas_popup() - display popup menu for the canvas */
static void show_canvas_popup(
	GtkWidget *canvas,
	gint x,
	gint y,
	guint button,
	guint32 time
) {
	GtkWidget *menu;
	GtkWidget *menu_item;
	GValue *attr_x = NULL;
	GValue *attr_y = NULL;
	debug(">>> show_canvas_popup()");
	menu = gtk_menu_new();
	if (! menu) {
		warn("show_canvas_popup(): gtk_menu_new()");
		goto LBL_return;
	}
	menu_item = gtk_menu_item_new_with_label("Paste");
	if (! menu_item) {
		warn("show_canvas_popup(): gtk_menu_item_new_with_label()");
		goto LBL_return;
	}
	gtk_menu_shell_append((GtkMenuShell *)menu, menu_item);
	attr_x = malloc(sizeof(GValue));
	attr_y = malloc(sizeof(GValue));
	if (! (attr_x && attr_y)) {
		warn("show_canvas_popup(): malloc()");
		goto LBL_return;
	}
	memset(attr_x, 0, sizeof(GValue));
	memset(attr_y, 0, sizeof(GValue));
	g_value_init(attr_x, G_TYPE_INT);
	g_value_init(attr_y, G_TYPE_INT);
	g_value_set_int(attr_x, x);
	g_value_set_int(attr_y, y);
	g_object_set_data_full(G_OBJECT(menu), "menu_x", attr_x, (GDestroyNotify)g_value_destroy);
	g_object_set_data_full(G_OBJECT(menu), "menu_y", attr_y, (GDestroyNotify)g_value_destroy);
	g_signal_connect(
		menu,
		"selection-done",
		G_CALLBACK(gtk_widget_destroy),
		NULL
	);
	gtk_menu_attach_to_widget((GtkMenu *)menu, canvas, NULL);
	gtk_menu_popup(
		(GtkMenu *)menu,
		NULL,
		NULL,
		position_canvas_popup,
		NULL,
		button,
		time
	);
	gtk_widget_show_all(menu);
	LBL_return:
	if (! (attr_x && attr_y) ) {
		warn("show_canvas_popup(): free()");
		free(attr_x);
		free(attr_y);
		if (menu) gtk_widget_destroy(menu);
	}
	debug("<<< show_canvas_popup()");
}
/* }}} show_canvas_popup() */

/* {{{ button_press_canvas_popup(…) - show popup menu if button pressed */
static gboolean button_press_canvas_popup(
	GtkWidget *w,
	GdkEventButton *ev,
	gpointer ud
) {
	gboolean ret = FALSE;
	debug(">>> button_press_canvas_popup()");
	(void)ud;
	if (ev->button != 3) goto LBL_return;
	if (ev->type != GDK_BUTTON_PRESS) goto LBL_return;
	show_canvas_popup(
		w,
		ev->x,
		ev->y,
		ev->button,
		ev->time
	);
	ret = TRUE;
	LBL_return:
	debug("<<< button_press_canvas_popup()");
	return ret;
}
/* }}} button_press_canvas_popup() */

/* {{{ popup_press_canvas_popup(…) - show popup menu if key pushed */
static gboolean popup_press_canvas_popup(GtkWidget *w, gpointer ud) {
	debug(">>> popup_press_canvas_popup()");
	(void)ud;
	gint x = 0;
	gint y = 0;
	gdk_window_get_origin(gtk_widget_get_window(w), &x, &y);
	show_canvas_popup(w, x, y, 0, gtk_get_current_event_time());
	debug("<<< popup_press_canvas_popup()");
	return TRUE;
}
/* }}} popup_press_canvas_popup() */

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

/* {{{ init_canvas_popup(…) - attach popup menu to the canvas */
static GtkWidget *init_canvas_popup(GtkWidget *canvas) {
	debug(">>> init_canvas_popup()");
	gtk_widget_set_can_focus(canvas, TRUE);
	g_signal_connect(
		G_OBJECT(canvas),
		"button-press-event",
		G_CALLBACK(button_press_canvas_popup),
		NULL
	);
	g_signal_connect(
		G_OBJECT(canvas),
		"popup-menu",
		G_CALLBACK(popup_press_canvas_popup),
		NULL
	);
	gtk_widget_add_events(canvas, GDK_BUTTON_PRESS_MASK);
	debug("<<< init_canvas_popup()");
	return canvas;
}
/* }}} init_canvas_popup() */

/* {{{ init_canvas_signals(…) - init signal handlers for the canvas */
static GtkWidget *init_canvas_signals(GtkWidget *canvas) {
	debug(">>> init_canvas_signals()");
	if(! init_canvas_dnd(canvas) ) {
		warn("init_canvas_signals(): init_canvas_dnd()");
	};
	if (! init_canvas_popup(canvas) ) {
		warn("init_canvas_signals(): init_canvas_popup()");
	}
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
	gtk_widget_set_size_request(canvas, 640, 480);
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

