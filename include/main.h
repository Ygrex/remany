#include "log.h"
#include <gtk/gtk.h>

/* supported data types */
const int Data_Image	= 1 << 0;
const int Data_Text	= 1 << 1;
const int Data_Uri	= 1 << 2;
const int Data_Rich	= 1 << 3;

#define weight(a, exp)	( ((exp) == TRUE) ? (a) : 0 )

