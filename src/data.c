#include "data.h"

/* supported data types */
const int Data_Image	= 1 << 0;
const int Data_Text	= 1 << 1;
const int Data_Uri	= 1 << 2;
const int Data_Rich	= 1 << 3;

const char *DataName_Unknown	= "unknown";
const char *DataName_Image	= "image";
const char *DataName_Text	= "text";
const char *DataName_Uri	= "URI";
const char *DataName_Rich	= "rich text";

const void *data_attr[][2] = {
	{ &Data_Image,	&DataName_Image	},
	{ &Data_Text,	&DataName_Text	},
	{ &Data_Uri,	&DataName_Uri	},
	{ &Data_Rich,	&DataName_Rich	},
};

/* {{{ get_data_name(…) - return string representation of the data type */
/*	data_type	- one of supported Data_* types
	returns one of DataName_* strings
*/
const char *get_data_name(int data_type) {
	int i = 0;
	for (; i < (int)(sizeof(data_attr) / sizeof(data_attr[0])); i += 1) {
		if ( data_type & *(int *)( data_attr[i][0] ) )
			return *(const char **)( data_attr[i][1] );
	}
	return DataName_Unknown;
}
/* }}} get_data_name() */

