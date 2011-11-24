/* functions to work with sprites */

typedef struct {
	/* attributes */
	GtkWidget *widget;
	int targets;
	guint time;
	gpointer userdata;
	/* methods */
	GDestroyNotify free;
} Sprite;

Sprite *new_sprite(
	GtkSelectionData *data,
	int targets,
	guint time,
	gpointer ud
);

