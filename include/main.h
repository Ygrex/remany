#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#define warn(msg)	{ fputs("WW: ", stderr) ; fputs(msg, stderr) ; fputs("\n", stderr) ; fflush(stderr) ; }
#define debug(msg)	{ fputs("DD: ", stderr) ; fputs(msg, stderr) ; fputs("\n", stderr) ; fflush(stderr) ; }

/* supported data types */
const int Data_Image	= 1 << 0;
const int Data_Text	= 1 << 1;
const int Data_Uri	= 1 << 2;
const int Data_Rich	= 1 << 3;

#define weight(a, exp)	( (exp == TRUE) ? (a) : 0 )

