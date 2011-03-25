#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

int debug_default = 2;
int SkipCertificateCheck = 0;
const char *certificate_filename = NULL;
int exists(const char *filename __attribute__ ((unused)))
{
	return (0);
}
void ProcessPendingEvents(void)
{
	return;
}

int x_socket(void)
{
	return (0);
}

int print_info(void *state __attribute__((unused)))
{
	return (0);
}

int Relax = 1;


int indices[12];
const char *sequence[][4] = {
	{NULL, NULL, NULL, NULL},
	{"prefix", " hello", NULL},
	{"pre", "fix", " hello", NULL},
	{"\r\n", ")\r\n", "prefix", NULL},
	{NULL, NULL, NULL, NULL},
};

/* trick tlscomm into believing it can read. */
ssize_t read(int s, void *buf, size_t buflen)
{
	int val = indices[s]++;

	if (sequence[s][val] == NULL) {
		indices[s]--;			/* make it stay */
		return 0;
	} else {
		strncpy(buf, sequence[s][val], buflen);
		printf("read: %s\n", sequence[s][val]);
		return (strlen(sequence[s][val]));
	}
}

int
select(int nfds, fd_set * r,
	   fd_set * w __attribute__ ((unused)),
	   fd_set * x __attribute__ ((unused)),
	   struct timeval *tv __attribute__ ((unused)))
{
	int i;
	int ready = 0;
	for (i = 0; i < nfds; i++) {
		if (FD_ISSET(i, r) && sequence[i][indices[i]] != NULL) {
			ready++;
		} else {
			FD_CLR(i, r);
		}
	}
	if (ready == 0) {
		printf("botched.\n");
	}
	return ready;
}

#define BUF_SIZE 1024

struct connection_state {
	int sd;
	char *name;
	/*@null@ */ void *tls_state;
	/*@null@ */ void *xcred;
	char unprocessed[BUF_SIZE];
	void *pc;					/* mailbox handle for debugging messages */
};

int
main(int argc __attribute__ ((unused)), char **argv
	 __attribute__ ((unused)))
{
	char buf[255];
	struct connection_state scs;
	scs.name = strdup("test");
	scs.unprocessed[0] = '\0';
	scs.pc = NULL;
	scs.tls_state = NULL;
	scs.xcred = NULL;
	alarm(10);

	for (scs.sd = 1; sequence[scs.sd][0] != NULL; scs.sd++) {
		memset(scs.unprocessed, 0, BUF_SIZE);
		printf("%d\n", tlscomm_expect(&scs, "prefix", buf, 255));
	}
	return 0;

}
