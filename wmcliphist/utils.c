#include <wmcliphist.h>

/*
 * converts text in UTF-8 to charset by actual locale
 */
gchar *
from_utf8(gchar *string) {
	gsize	bytes_read;
	gsize	bytes_written;
	GError	*error;
	gchar	*converted;
	/* gchar	*error_msg; */
	gint	error_code;

	converted = g_locale_from_utf8(string, -1, &bytes_read, &bytes_written,
			&error);
	error_code = (error == NULL) ? 0 : error->code;
        /* 
	 * fprintf(stderr, "from_utf8: %d b read, %d b written, error: %d\n",
	 * 		bytes_read, bytes_written, error_code);
         */

	return converted;
}
