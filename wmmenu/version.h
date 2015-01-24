#ifndef version_h_
#define version_h_

#ifdef WITH_GDKPIXBUF
#define ADDVER " (with gdk-pixbuf support)"
#else
#define ADDVER " (XPM only)"
#endif

#define VERSION ("1.3" ADDVER)

#endif /* version_h_ */
