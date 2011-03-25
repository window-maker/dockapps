/* tlsComm.h - interface for the thin layer that looks
   sort of like fgets and fprintf, but might read or write
   to a socket or a TLS association 

   Neil Spring (nspring@cs.washington.edu)

   Comments in @'s are for lclint's benefit:
   http://lclint.cs.virginia.edu/
*/

/* used to drill through per-mailbox debug keys */
#include "Client.h"

/* opaque reference to the state associated with a 
   connection: may be just a file handle, or may include
   encryption state */
struct connection_state;

/* take a socket descriptor and negotiate a TLS connection
   over it */
/*@only@*/
struct connection_state *initialize_gnutls(int sd, /*@only@ */ char *name,
										   Pop3 pc, const char *hostname);

/* take a socket descriptor and bundle it into a connection
   state structure for later communication */
/*@only@*/
struct connection_state *initialize_unencrypted(int sd,	/*@only@ */
												char *name, Pop3 pc);

/* store a binding when connect() times out. these should be 
   skipped when trying to check mail so that other mailboxes
   are checked responsively.  I believe linux defaults to 
   around 90 seconds for a failed connect() attempt */
/* TODO: engineer an eventual retry scheme */
/*@only@*/
struct connection_state *initialize_blacklist( /*@only@ */ char *name);
int tlscomm_is_blacklisted(const struct connection_state *scs);

/* just like fprintf, only takes a connection state structure */
void tlscomm_printf(struct connection_state *scs, const char *format, ...);

/* modeled after fgets; may not work exactly the same */
int tlscomm_gets( /*@out@ */ char *buf,
				 int buflen, struct connection_state *scs);

/* gobbles lines until it finds one starting with {prefix},
   which is returned in buf */
int tlscomm_expect(struct connection_state *scs, const char *prefix,
				   /*@out@ */ char *buf,
				   int buflen);

/* terminates the TLS association or just closes the socket,
   and frees the connection state */
void tlscomm_close( /*@only@ */ struct connection_state *scs);

/* internal function exported for testing */
int getline_from_buffer(char *readbuffer, char *linebuffer,
						int linebuflen);
#ifndef UNUSED
#ifdef HAVE___ATTRIBUTE__
#define UNUSED(x) /*@unused@*/  x __attribute__((unused))
#else
#define UNUSED(x) x
#endif
#endif

