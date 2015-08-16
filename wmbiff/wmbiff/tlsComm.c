/* tlsComm.c - primitive routines to aid TLS communication
   within wmbiff, without rewriting each mailbox access
   scheme.  These functions hide whether the underlying
   transport is encrypted.

   Neil Spring (nspring@cs.washington.edu) */

/* TODO: handle "* BYE" internally? */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#ifdef HAVE_GNUTLS_GNUTLS_H
#define USE_GNUTLS
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <sys/stat.h>
#endif
#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

#include "tlsComm.h"

#include "Client.h"				/* debugging messages */

/* if non-null, set to a file for certificate verification */
extern const char *certificate_filename;
/* if set, don't fail when dealing with a bad certificate.
   (continue to whine, though, as bad certs should be fixed) */
extern int SkipCertificateCheck;
/* gnutls: specify the priorities to use on the ciphers, key exchange methods,
   macs and compression methods. */
extern const char *tls;

/* WARNING: implcitly uses scs to gain access to the mailbox
   that holds the per-mailbox debug flag. */
#define TDM(lvl, args...) DM(scs->pc, lvl, "comm: " args)

/* how long to wait for the server to do its thing
   when we issue it a command (in seconds) */
#define EXPECT_TIMEOUT 40

/* this is the per-connection state that is maintained for
   each connection; BIG variables are for ssl (null if not
   used). */
#define BUF_SIZE 1024
struct connection_state {
	int sd;
	char *name;
#ifdef USE_GNUTLS
	gnutls_session_t tls_state;
	gnutls_certificate_credentials_t xcred;
#else
	/*@null@ */ void *tls_state;
	/*@null@ */ void *xcred;
#endif
	char unprocessed[BUF_SIZE];
	Pop3 pc;					/* mailbox handle for debugging messages */
};

/* gotta do our own line buffering, sigh */
int getline_from_buffer(char *readbuffer, char *linebuffer,
						int linebuflen);
void handle_gnutls_read_error(int readbytes, struct connection_state *scs);

void tlscomm_close(struct connection_state *scs)
{
	TDM(DEBUG_INFO, "%s: closing.\n",
		(scs->name != NULL) ? scs->name : "null");

	/* not ok to call this more than once */
	if (scs->tls_state) {
#ifdef USE_GNUTLS
		/* this next line seems capable of hanging... */
		/* gnutls_bye(scs->tls_state, GNUTLS_SHUT_RDWR); */
		/* so we'll try just _bye'ing the WR direction, which
		   should send the alert but not wait for a response. */
		gnutls_bye(scs->tls_state, GNUTLS_SHUT_WR);
		gnutls_certificate_free_credentials(scs->xcred);
		gnutls_deinit(scs->tls_state);
		scs->xcred = NULL;
#endif
	} else {
		(void) close(scs->sd);
	}
	scs->sd = -1;
	scs->tls_state = NULL;
	scs->xcred = NULL;
	free(scs->name);
	scs->name = NULL;
	free(scs);
}

extern int x_socket(void);
extern void ProcessPendingEvents(void);

/* this avoids blocking without using non-blocking i/o */
static int wait_for_it(int sd, int timeoutseconds)
{
	fd_set readfds;
	struct timeval tv;
	int ready_descriptors;
    int maxfd;
    int xfd;
    struct timeval time_now;
    struct timeval time_out;

    gettimeofday(&time_now, NULL);
    memcpy(&time_out, &time_now, sizeof(struct timeval));
    time_out.tv_sec += timeoutseconds;

    xfd = x_socket();
    maxfd = max(sd, xfd);

	do {
        do {
            ProcessPendingEvents();

            gettimeofday(&time_now, NULL);
            tv.tv_sec = max(time_out.tv_sec - time_now.tv_sec + 1, (time_t) 0); /* sloppy, but bfd */
            tv.tv_usec = 0;
            /* select will return if we have X stuff or we have comm stuff on sd */
            FD_ZERO(&readfds);
            FD_SET(sd, &readfds);
            // FD_SET(xfd, &readfds);
            ready_descriptors = select(maxfd + 1, &readfds, NULL, NULL, &tv);
            // DMA(DEBUG_INFO,
            //    "select %d/%d returned %d descriptor, %d\n",
            //    sd, timeoutseconds, ready_descriptors, FD_ISSET(sd, &readfds));

        } while(tv.tv_sec > 0 && (!FD_ISSET(sd, &readfds) || (errno == EINTR && ready_descriptors == -1)));

        FD_ZERO(&readfds);
        FD_SET(sd, &readfds);
        tv.tv_sec = 0; tv.tv_usec = 0;
        ready_descriptors = select(sd + 1, &readfds, NULL, NULL, &tv);
	}
	while (ready_descriptors == -1 && errno == EINTR);
	if (ready_descriptors == 0) {
		DMA(DEBUG_INFO,
			"select timed out after %d seconds on socket: %d\n",
			timeoutseconds, sd);
		return (0);
	} else if (ready_descriptors == -1) {
		DMA(DEBUG_ERROR,
			"select failed on socket %d: %s\n", sd, strerror(errno));
		return (0);
	}
	return (FD_ISSET(sd, &readfds));
}

/* exported for testing */
extern int
getline_from_buffer(char *readbuffer, char *linebuffer, int linebuflen)
{
	char *p, *q;
	int i;
	/* find end of line (stopping if linebuflen is too small. */
	for (p = readbuffer, i = 0;
		 *p != '\n' && *p != '\0' && i < linebuflen - 1; p++, i++);

	/* gobble \n if it starts the line. */
	if (*p == '\n') {
		/* grab the end of line too! and then advance past
		   the newline */
		i++;
		p++;
	} else {
		/* TODO -- perhaps we should return no line at all
		   here, as it might be incomplete.  don't want to
		   break anything though. */
		DMA(DEBUG_INFO, "expected line doesn't end on its own.\n");
	}

	if (i != 0) {
		/* copy a line into the linebuffer */
		strncpy(linebuffer, readbuffer, (size_t) i);
		/* sigh, null terminate */
		linebuffer[i] = '\0';
		/* shift the rest over; this could be done
		   instead with strcpy... I think. */
		q = readbuffer;
		if (*p != '\0') {
			while (*p != '\0') {
				*(q++) = *(p++);
			}
		}
		/* null terminate */
		*(q++) = *(p++);
		/* return the length of the line */
	}
	if (i < 0 || i > linebuflen) {
		DM((Pop3) NULL, DEBUG_ERROR, "bork bork bork!: %d %d\n", i,
		   linebuflen);
	}
	return i;
}

/* eat lines, until one starting with prefix is found;
   this skips 'informational' IMAP responses */
/* the correct response to a return value of 0 is almost
   certainly tlscomm_close(scs): don't _expect() anything
   unless anything else would represent failure */
int
tlscomm_expect(struct connection_state *scs,
			   const char *prefix, char *linebuf, int buflen)
{
	int prefixlen = (int) strlen(prefix);
	int buffered_bytes = 0;
	memset(linebuf, 0, buflen);
	TDM(DEBUG_INFO, "%s: expecting: %s\n", scs->name, prefix);
	/*     if(scs->unprocessed[0]) {
	   TDM(DEBUG_INFO, "%s: buffered: %s\n", scs->name, scs->unprocessed);
	   } */
	while (scs->unprocessed[0] != '\0'
		   || wait_for_it(scs->sd, EXPECT_TIMEOUT)) {
		if (scs->unprocessed[buffered_bytes] == '\0') {
			int thisreadbytes;
#ifdef USE_GNUTLS
			if (scs->tls_state) {
				/* BUF_SIZE - 1 leaves room for trailing \0 */
				thisreadbytes =
					gnutls_read(scs->tls_state,
								&scs->unprocessed[buffered_bytes],
								BUF_SIZE - 1 - buffered_bytes);
				if (thisreadbytes < 0) {
					handle_gnutls_read_error(thisreadbytes, scs);
					return 0;
				}
			} else
#endif
			{
				thisreadbytes =
					read(scs->sd, &scs->unprocessed[buffered_bytes],
						 BUF_SIZE - 1 - buffered_bytes);
				if (thisreadbytes < 0) {
					TDM(DEBUG_ERROR, "%s: error reading: %s\n",
						scs->name, strerror(errno));
					return 0;
				}
			}
			buffered_bytes += thisreadbytes;
			/* force null termination */
			scs->unprocessed[buffered_bytes] = '\0';
			if (buffered_bytes == 0) {
				return 0;		/* bummer */
			}
		} else {
			buffered_bytes = strlen(scs->unprocessed);
		}
		while (buffered_bytes >= prefixlen) {
			int linebytes;
			linebytes =
				getline_from_buffer(scs->unprocessed, linebuf, buflen);
			if (linebytes == 0) {
				buffered_bytes = 0;
			} else {
				buffered_bytes -= linebytes;
				if (strncmp(linebuf, prefix, prefixlen) == 0) {
					TDM(DEBUG_INFO, "%s: got: %*s", scs->name,
						linebytes, linebuf);
					return 1;	/* got it! */
				}
				TDM(DEBUG_INFO, "%s: dumped(%d/%d): %.*s", scs->name,
					linebytes, buffered_bytes, linebytes, linebuf);
			}
		}
	}
	if (buffered_bytes == -1) {
		TDM(DEBUG_INFO, "%s: timed out while expecting '%s'\n",
			scs->name, prefix);
	} else {
		TDM(DEBUG_ERROR, "%s: expecting: '%s', saw (%d): %s%s",
			scs->name, prefix, buffered_bytes, linebuf,
			/* only print the newline if the linebuf lacks it */
			(linebuf[strlen(linebuf) - 1] == '\n') ? "\n" : "");
	}
	return 0;					/* wait_for_it failed */
}

int tlscomm_gets(char *buf, int buflen, struct connection_state *scs)
{
	return (tlscomm_expect(scs, "", buf, buflen));
}

void tlscomm_printf(struct connection_state *scs, const char *format, ...)
{
	va_list args;
	char buf[1024];
	int bytes;
	ssize_t unused __attribute__((unused));

	if (scs == NULL) {
		DMA(DEBUG_ERROR, "null connection to tlscomm_printf\n");
		abort();
	}
	va_start(args, format);
	bytes = vsnprintf(buf, 1024, format, args);
	va_end(args);

	if (scs->sd != -1) {
#ifdef USE_GNUTLS
		if (scs->tls_state) {
			int written = gnutls_write(scs->tls_state, buf, bytes);
			if (written < bytes) {
				TDM(DEBUG_ERROR,
					"Error %s prevented writing: %*s\n",
					gnutls_strerror(written), bytes, buf);
				return;
			}
		} else
#endif
			/* Why???? */
			unused = write(scs->sd, buf, bytes);
	} else {
		printf
			("warning: tlscomm_printf called with an invalid socket descriptor\n");
		return;
	}
	TDM(DEBUG_INFO, "wrote %*s", bytes, buf);
}

/* most of this file only makes sense if using TLS. */
#ifdef USE_GNUTLS
#include "gnutls-common.h"

static void
bad_certificate(const struct connection_state *scs, const char *msg)
{
	TDM(DEBUG_ERROR, "%s", msg);
	if (!SkipCertificateCheck) {
		TDM(DEBUG_ERROR, "to ignore this error, run wmbiff "
			"with the -skip-certificate-check option\n");
		exit(1);
	}
}

static void
warn_certificate(const struct connection_state *scs, const char *msg)
{
	if (!SkipCertificateCheck) {
		TDM(DEBUG_ERROR, "%s", msg);
		TDM(DEBUG_ERROR, "to ignore this warning, run wmbiff "
			"with the -skip-certificate-check option\n");
	}
}

/* a start of a hack at verifying certificates.  does not
   provide any security at all.  I'm waiting for either
   gnutls to make this as easy as it should be, or someone
   to port Andrew McDonald's gnutls-for-mutt patch.
*/

#define CERT_SEP "-----BEGIN"

/* this bit is based on read_ca_file() in gnutls */
static int tls_compare_certificates(const gnutls_datum_t * peercert)
{
	gnutls_datum_t cert;
	unsigned char *ptr;
	FILE *fd1;
	int ret;
	gnutls_datum_t b64_data;
	unsigned char *b64_data_data;
	struct stat filestat;

	if (stat(certificate_filename, &filestat) == -1)
		return 0;

	b64_data.size = filestat.st_size + 1;
	b64_data_data = (unsigned char *) malloc(b64_data.size);
	b64_data_data[b64_data.size - 1] = '\0';
	b64_data.data = b64_data_data;

	fd1 = fopen(certificate_filename, "r");
	if (fd1 == NULL) {
		return 0;
	}

	b64_data.size = fread(b64_data.data, 1, b64_data.size, fd1);
	fclose(fd1);

	do {
		ret = gnutls_pem_base64_decode_alloc(NULL, &b64_data, &cert);
		if (ret != 0) {
			free(b64_data_data);
			return 0;
		}

		ptr = (unsigned char *) strstr((char *) b64_data.data, CERT_SEP) + 1;
		ptr = (unsigned char *) strstr((char *) ptr, CERT_SEP);

		b64_data.size = b64_data.size - (ptr - b64_data.data);
		b64_data.data = ptr;

		if (cert.size == peercert->size) {
			if (memcmp(cert.data, peercert->data, cert.size) == 0) {
				/* match found */
				gnutls_free(cert.data);
				free(b64_data_data);
				return 1;
			}
		}

		gnutls_free(cert.data);
	} while (ptr != NULL);

	/* no match found */
	free(b64_data_data);
	return 0;
}


static void
tls_check_certificate(struct connection_state *scs,
					  const char *remote_hostname)
{
	int ret;
	unsigned int certstat;
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size = 0;
	gnutls_x509_crt_t cert;

	if (gnutls_auth_get_type(scs->tls_state) != GNUTLS_CRD_CERTIFICATE) {
		bad_certificate(scs, "Unable to get certificate from peer.\n");
		return;	/* bad_cert will exit if -skip-certificate-check was not given */
	}
	ret = gnutls_certificate_verify_peers2(scs->tls_state, &certstat);

	if (ret < 0) {
		char errbuf[1024];

		snprintf(errbuf, 1024, "could not verify certificate: %s (%d).\n",
			gnutls_strerror(ret), ret);
		bad_certificate(scs, (ret == GNUTLS_E_NO_CERTIFICATE_FOUND ?
			"server presented no certificate.\n" :
			errbuf));
		return;
#ifdef GNUTLS_CERT_CORRUPTED
	} else if (certstat & GNUTLS_CERT_CORRUPTED) {
		bad_certificate(scs, "server's certificate is corrupt.\n");
#endif
	} else if (certstat & GNUTLS_CERT_REVOKED) {
		bad_certificate(scs, "server's certificate has been revoked.\n");
	} else if (certstat & GNUTLS_CERT_EXPIRED) {
		bad_certificate(scs, "server's certificate is expired.\n");
	} else if (certstat & GNUTLS_CERT_INSECURE_ALGORITHM) {
		warn_certificate(scs, "server's certificate use an insecure algorithm.\n");
	} else if (certstat & GNUTLS_CERT_INVALID) {
		if (gnutls_certificate_type_get(scs->tls_state) == GNUTLS_CRT_X509) {
			/* bad_certificate(scs, "server's certificate is not trusted.\n"
			   "there may be a problem with the certificate stored in your certfile\n"); */
		} else {
			bad_certificate(scs,
				"server's certificate is invalid or not X.509.\n"
				"there may be a problem with the certificate stored in your certfile\n");
		}
#if defined(GNUTLS_CERT_SIGNER_NOT_FOUND)
	} else if (certstat & GNUTLS_CERT_SIGNER_NOT_FOUND) {
		TDM(DEBUG_INFO, "server's certificate is not signed.\n");
		TDM(DEBUG_INFO,
			"to verify that a certificate is trusted, use the certfile option.\n");
#endif

#if defined(GNUTLS_CERT_NOT_TRUSTED)
	} else if (certstat & GNUTLS_CERT_NOT_TRUSTED) {
		TDM(DEBUG_INFO, "server's certificate is not trusted.\n");
		TDM(DEBUG_INFO,
			"to verify that a certificate is trusted, use the certfile option.\n");
#endif
	}

	if (gnutls_x509_crt_init(&cert) < 0) {
		bad_certificate(scs,
			"Unable to initialize certificate data structure");
	}


	/* not checking for not-yet-valid certs... this would make sense
	   if we weren't just comparing to stored ones */
	cert_list =
		gnutls_certificate_get_peers(scs->tls_state, &cert_list_size);

	if (gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER) <
		0) {
		bad_certificate(scs, "Error processing certificate data");
	}

	if (gnutls_x509_crt_get_expiration_time(cert) < time(NULL)) {
		bad_certificate(scs, "server's certificate has expired.\n");
	} else if (gnutls_x509_crt_get_activation_time(cert)
			   > time(NULL)) {
		bad_certificate(scs, "server's certificate is not yet valid.\n");
	} else {
		TDM(DEBUG_INFO, "certificate passed time check.\n");
	}

	if (gnutls_x509_crt_check_hostname(cert, remote_hostname) == 0) {
		char certificate_hostname[256];
		size_t buflen = 255;
		gnutls_x509_crt_get_dn(cert, certificate_hostname, &buflen);
		/* gnutls_x509_extract_certificate_dn(&cert_list[0], &dn); */
		TDM(DEBUG_INFO,
			"server's certificate (%s) does not match its hostname (%s).\n",
			certificate_hostname, remote_hostname);
		bad_certificate(scs,
						"server's certificate does not match its hostname.\n");
	} else {
		if ((scs->pc)->debug >= DEBUG_INFO) {
			char certificate_hostname[256];
			size_t buflen = 255;
			gnutls_x509_crt_get_dn(cert, certificate_hostname, &buflen);
			/* gnutls_x509_extract_certificate_dn(&cert_list[0], &dn); */
			TDM(DEBUG_INFO,
				"server's certificate (%s) matched its hostname (%s).\n",
				certificate_hostname, remote_hostname);
		}
	}

	if (certificate_filename != NULL &&
		tls_compare_certificates(&cert_list[0]) == 0) {
		bad_certificate(scs,
			"server's certificate was not found in the certificate file.\n");
	}

	gnutls_x509_crt_deinit(cert);

	TDM(DEBUG_INFO, "certificate check ok.\n");
	return;
}

struct connection_state *initialize_gnutls(intptr_t sd, char *name, Pop3 pc,
										   const char *remote_hostname)
{
	static int gnutls_initialized;
	int zok;
	struct connection_state *scs = malloc(sizeof(struct connection_state));
	memset(scs, 0, sizeof(struct connection_state));	/* clears the unprocessed buffer */

	scs->pc = pc;

	assert(sd >= 0);

	if (gnutls_initialized == 0) {
		assert(gnutls_global_init() == 0);
		gnutls_initialized = 1;
	}

	assert(gnutls_init(&scs->tls_state, GNUTLS_CLIENT) == 0);
	{
		const char *err_pos;
		if (GNUTLS_E_SUCCESS != gnutls_priority_set_direct(scs->tls_state, tls, &err_pos)) {
			DMA(DEBUG_ERROR,
				"Unable to set the priorities to use on the ciphers, "
				"key exchange methods, macs and/or compression methods.\n"
				"See 'tls' parameter in config file: '%s'.\n",
				err_pos);
			exit(1);
		}

		/* no client private key */
		if (gnutls_certificate_allocate_credentials(&scs->xcred) < 0) {
			DMA(DEBUG_ERROR, "gnutls memory error\n");
			exit(1);
		}

		/* certfile seems to work. */
		if (certificate_filename != NULL) {
			if (!exists(certificate_filename)) {
				DMA(DEBUG_ERROR,
					"Certificate file (certfile=) %s not found.\n",
					certificate_filename);
				exit(1);
			}
			zok = gnutls_certificate_set_x509_trust_file(scs->xcred,
					(char *)
					certificate_filename,
					GNUTLS_X509_FMT_PEM);
			if (zok < 0) {
				DMA(DEBUG_ERROR,
					"GNUTLS did not like your certificate file %s (%d).\n",
					certificate_filename, zok);
				gnutls_perror(zok);
				exit(1);
			}
		}

		gnutls_cred_set(scs->tls_state, GNUTLS_CRD_CERTIFICATE,
						scs->xcred);
		gnutls_transport_set_ptr(scs->tls_state,
								 (gnutls_transport_ptr_t) sd);
		do {
			zok = gnutls_handshake(scs->tls_state);
		}
		while (zok == GNUTLS_E_INTERRUPTED || zok == GNUTLS_E_AGAIN);

		tls_check_certificate(scs, remote_hostname);
	}

	if (zok < 0) {
		TDM(DEBUG_ERROR, "%s: Handshake failed\n", name);
		TDM(DEBUG_ERROR, "%s: This may be a problem in gnutls, "
			"which is under development\n", name);
		TDM(DEBUG_ERROR,
			"%s: This copy of wmbiff was compiled with \n"
			"  gnutls version %s.\n", name, LIBGNUTLS_VERSION);
		gnutls_perror(zok);
		if (scs->pc->u.pop_imap.serverPort != 143 /* starttls */ ) {
			TDM(DEBUG_ERROR,
				"%s: Please run 'gnutls-cli-debug -p %d %s' to test ssl directly.\n"
				"  That tool provides a lower-level test of gnutls with your server.\n",
				name, scs->pc->u.pop_imap.serverPort, remote_hostname);
		}
		gnutls_deinit(scs->tls_state);
		free(scs);
		return (NULL);
	} else {
		TDM(DEBUG_INFO, "%s: Handshake was completed\n", name);
		if (scs->pc->debug >= DEBUG_INFO)
			print_info(scs->tls_state, remote_hostname);
		scs->sd = sd;
		scs->name = name;
	}
	return (scs);
}

/* moved down here, to keep from interrupting the flow with
   verbose error crap */
void handle_gnutls_read_error(int readbytes, struct connection_state *scs)
{
	if (gnutls_error_is_fatal(readbytes) == 1) {
		TDM(DEBUG_ERROR,
			"%s: Received corrupted data(%d) - server has terminated the connection abnormally\n",
			scs->name, readbytes);
	} else {
		if (readbytes == GNUTLS_E_WARNING_ALERT_RECEIVED
			|| readbytes == GNUTLS_E_FATAL_ALERT_RECEIVED)
			TDM(DEBUG_ERROR, "* Received alert [%d]\n",
				gnutls_alert_get(scs->tls_state));
		if (readbytes == GNUTLS_E_REHANDSHAKE)
			TDM(DEBUG_ERROR, "* Received HelloRequest message\n");
	}
	TDM(DEBUG_ERROR,
		"%s: gnutls error reading: %s\n",
		scs->name, gnutls_strerror(readbytes));
}

#else
/* declare stubs when tls isn't compiled in */
struct connection_state *initialize_gnutls(UNUSED(intptr_t sd),
										   UNUSED(char *name),
										   UNUSED(Pop3 pc),
										   UNUSED(const char
												  *remote_hostname))
{
	DM(pc, DEBUG_ERROR,
	   "FATAL: tried to initialize ssl when ssl wasn't compiled in.\n");
	exit(EXIT_FAILURE);
}
#endif

/* either way: */
struct connection_state *initialize_unencrypted(int sd,
												/*@only@ */ char *name,
												Pop3 pc)
{
	struct connection_state *ret = malloc(sizeof(struct connection_state));
	assert(sd >= 0);
	assert(ret != NULL);
	memset(ret, 0, sizeof(struct connection_state));	/* clears the unprocessed buffer */
	ret->sd = sd;
	ret->name = name;
	ret->tls_state = NULL;
	ret->xcred = NULL;
	ret->pc = pc;
	return (ret);
}

/* bad seed connections that can't be setup */
/*@only@*/
struct connection_state *initialize_blacklist( /*@only@ */ char *name)
{
	struct connection_state *ret = malloc(sizeof(struct connection_state));
	assert(ret != NULL);
	ret->sd = -1;
	ret->name = name;
	ret->tls_state = NULL;
	ret->xcred = NULL;
	ret->pc = NULL;
	return (ret);
}


int tlscomm_is_blacklisted(const struct connection_state *scs)
{
	return (scs != NULL && scs->sd == -1);
}

/* vim:set ts=4: */
/*
 * Local Variables:
 * tab-width: 4
 * c-indent-level: 4
 * c-basic-offset: 4
 * End:
 */
