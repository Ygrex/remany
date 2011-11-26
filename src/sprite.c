#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "log.h"
#include "sprite.h"

/* {{{ make_sprite_bin(…) - create container for a sprite */
static GtkWidget *make_sprite_bin(const gchar *lbl) {
	debug(">>> make_sprite_bin()");
	GtkWidget *frame = gtk_frame_new(lbl);
	if (! frame) {
		warn("make_sprite_bin(): gtk_frame_new()");
		goto LBL_return;
	}
	GtkWidget *box = (GtkWidget *)gtk_vbox_new(FALSE, 0);
	if (! box) {
		warn("make_sprite_bin(): gtk_vbox_new()");
		gtk_widget_destroy(frame);
		goto LBL_return;
	}
	gtk_container_add((GtkContainer *)frame, box);
	LBL_return:
	debug("<<< make_sprite_bin()");
	return frame;
}
/* }}} make_sprite_bin() */

/* {{{ append_sprite_widget(…) - append a child widget to a sprite */
static void append_sprite_widget(Sprite *sprite, GtkWidget *widget) {
	debug(">>> append_sprite_widget()");
	gtk_container_add(
		(GtkContainer *)gtk_bin_get_child((GtkBin *)sprite->widget),
		widget
	);
	debug("<<< append_sprite_widget()");
}
/* }}} append_sprite_widget() */

/* {{{ append_sprite_pic(…) - append an image to a sprite */
static Sprite *append_sprite_pic(Sprite *s, GdkPixbuf *pic) {
	debug(">>> append_sprite_pic()");
	/* FIXME animated images must be supported */
	GtkWidget *w = gtk_image_new_from_pixbuf(pic);
	if (w == NULL) {
		warn("append_sprite_pic(): gtk_image_new_from_pixbuf()");
		goto LBL_return;
	}
	gtk_widget_set_vexpand(w, TRUE);
	gtk_widget_set_hexpand(w, TRUE);
	gtk_widget_set_vexpand_set(w, TRUE);
	gtk_widget_set_hexpand_set(w, TRUE);
	/* shrink large images */
	float width = (float)gdk_pixbuf_get_width(pic);
	float height = (float)gdk_pixbuf_get_height(pic);
	float ratio = (float)width / height;
	/* FIXME avoid magic numbers */
	if ((ratio > 1) && (width > 128)) {
		width = 128;
		height = ratio * width;
	} else if ((ratio <= 1) && (height > 128)) {
		height = 128;
		width = height * ratio;
	}
	gtk_widget_set_size_request(w, (gint)width, (gint)height);
	append_sprite_widget(s, w);
	LBL_return:
	debug("<<< append_sprite_pic()");
	return s;
}
/* }}} append_sprite_pic() */

/* {{{ append_sprite_uris(…) - append a list of URIs to a sprite */
static Sprite *append_sprite_uris(Sprite *s, gchar **uris) {
	debug(">>> append_sprite_uris()");
	GtkWidget *w;
	for (; uris[0] != NULL; uris += 1) {
		w = gtk_link_button_new(*uris);
		if (w == NULL) {
			warn("append_sprite_uris(): gtk_link_button_new()");
		} else {
			append_sprite_widget(s, w);
		}
	}
	debug("<<< append_sprite_uris()");
	return s;
}
/* }}} append_sprite_uris() */

/* {{{ append_sprite_text(…) - append text to a sprite */
static Sprite *append_sprite_text(Sprite *s, guchar *text) {
	debug(">>> append-sprite_text()");
	GtkWidget *w = gtk_label_new((gchar *)text);
	if (w == NULL) {
		warn("append_sprite_text(): gtk_label_new()");
		s = NULL;
	} else {
		append_sprite_widget(s, w);
	}
	debug("<<< append-sprite_text()");
	return s;
}
/* }}} append_sprite_text() */

/* {{{ free_sprite(…) - destructor for a Sprite */
void free_sprite(Sprite *s) {
	debug(">>> free_sprite()");
	if (s->free) s->free(s);
	debug("<<< free_sprite()");
}
/* }}} free_sprite() */

/* {{{ new_sprite(…) - create new floating sprite */
/*	data	- GtkSelectionData to fetch content from
	targets	- map of handled types of passed data
	time	- creation timestamp
	ud	- no idea what it is for
*/
Sprite *new_sprite(
	GtkSelectionData *data,
	int targets,
	guint time,
	gpointer ud
) {
	debug(">>> new_sprite()");
	(void)data;
	(void)targets;
	(void)time;
	(void)ud;
	Sprite *s = NULL;
	GdkPixbuf *data_pic	= NULL;
	gchar **data_uris	= NULL;
	guchar *data_text	= NULL;
	const void *d[][3]	= {
		{&data_pic,	&gtk_selection_data_get_pixbuf,	&Data_Image },
		{&data_uris,	&gtk_selection_data_get_uris,	&Data_Uri },
		{&data_text,	&gtk_selection_data_get_text,	&Data_Text },
	};
	/* {{{ msg(…) - output warning message on converting failure */
	void msg(int target) {
		char m[64] = "new_sprite(): data cannot be converted: ";
		warn( strcat(m, get_data_name(target)) );
	}
	/* }}} msg() */
	int i = 0;
	/* find any data that can be properly converted */
	for (; i < (int)(sizeof(d) / sizeof(d[0])); i += 1) {
		void **dest = (void **)(d[i][0]);
		void *(* func)(void *) = d[i][1];
		int target = *(int *)(d[i][2]);
		if ( (targets & target) == 0 ) continue;
		if ( (*dest = func(data)) == NULL ) {
			msg( target );
			targets = (targets ^ target);
		}
	}
	if (targets == 0) {
		debug("new_sprite(): no data converted");
		goto LBL_return;
	}
	if ( (s = malloc(sizeof(Sprite))) == NULL ) {
		warn("new_sprite(): malloc(Sprite)");
		goto LBL_return;
	};
	s->targets = targets;
	s->time = time;
	s->userdata = ud;
	s->free = NULL;
	/* reset the flag to signify that the sprite is not ready yet */
	targets = 0;
	if ( (s->widget = make_sprite_bin("New Sprite")) == NULL ) {
		warn("new_sprite(): make_sprite_bin()");
		goto LBL_return;
	};
	if (data_pic) {
		if (! append_sprite_pic(s, data_pic) ) {
			g_object_unref(data_pic);
		};
	}
	if (data_uris) {
		append_sprite_uris(s, data_uris);
		g_strfreev(data_uris);
	}
	if (data_text) {
		append_sprite_text(s, data_text);
		g_free(data_text);
	}
	/* all right, do not free initialized objects then */
	targets = 1;
	g_object_set_data_full(
		(GObject *)s->widget,
		"sprite",
		s,
		(GDestroyNotify)free_sprite
	);
	LBL_return:
	if (! targets) {
		if (s->widget) gtk_widget_destroy(s->widget);
		free(s);
	}
	debug("<<< new_sprite()");
	return s;
}
/* }}} new_sprite() */

