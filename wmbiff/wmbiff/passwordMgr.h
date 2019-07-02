#include "Client.h"

/*@mustfree@*/
char *passwordFor(const char *username, const char *servername, Pop3 *pc,
				  int bFlushCache);

/* tested by test_wmbiff; don't use this for anything. */
int permissions_ok(Pop3 *pc, const char *askpass_fname);
