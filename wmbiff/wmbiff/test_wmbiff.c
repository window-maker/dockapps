#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE___ATTRIBUTE__
#define UNUSED(x) /*@unused@*/  x __attribute__((unused))
#else
#define UNUSED(x) x
#endif

#ifdef HAVE_MEMFROB
#define DEFROB(x) memfrob(x, x ## _len)
#define ENFROB(x) memfrob(x, x ## _len)
#else
#define DEFROB(x)
#define ENFROB(x)
#endif

#include <unistd.h>

#include "Client.h"
#include "passwordMgr.h"
#include "tlsComm.h"
#include "charutil.h"

void ProcessPendingEvents(void)
{
	return;
}

int x_socket(void)
{
	return (0);
}

int debug_default = DEBUG_INFO;
int Relax = 1;

/* return 1 if fail, 0 if success */
int test_backtickExpand(void)
{
	const char *tests[] = { "prefix`echo 1`suffix",
		"prefix`echo 1`",
		"`echo 1`",
		"`echo a b` c",
		"`false`",
		NULL
	};
	const char *solns[] = { "prefix1suffix",
		"prefix1",
		"1",
		"a b c",
		"",
		NULL
	};
	int i;
	int retval = 0;
	for (i = 0; tests[i] != NULL; i++) {
		char *x = backtickExpand(NULL, tests[i]);
		if (strcmp(x, solns[i]) != 0) {
			printf("test failed: [%s] != [%s]\n", tests[i], solns[i]);
			retval = 1;
		}
		free(x);
	}
	printf("backtick tests complete\n");
	return (retval);
}

#define CKSTRING(x,shouldbe) if(strcmp(x,shouldbe)) { \
printf("FAILED: expected '" #shouldbe "' but got '%s'\n", x); \
 return 1; } else { printf("good: '" shouldbe "' == '%s'\n", x); }

/* return 1 if fail, 0 if success */
int test_passwordMgr(void)
{
	const char *b;
	mbox_t m;
	strcpy(m.label, "x");

	/* sh is almost certainly conforming; owned by root, etc. */
	if (!permissions_ok(NULL, "/bin/sh")) {
		printf("FAILURE: permission checker failed on /bin/sh.");
		return (1);
	}
	/* tmp is definitely bad; and better be og+w */
	if (permissions_ok(NULL, "/tmp")) {
		printf("FAILURE: permission checker failed on /tmp.");
		return (1);
	}
	/* TODO: also find some user-owned binary that shouldn't be g+w */
	printf("SUCCESS: permission checker sanity check went well.\n");

	/* *** */
	m.askpass = "echo xlnt; #";

	b = passwordFor("bill", "ted", &m, 0);
	if (strcmp(b, "xlnt") != 0) {
		printf("FAILURE: expected 'xlnt' got '%s'\n", b);
		return (1);
	}
	printf("SUCCESS: expected 'xlnt' got '%s'\n", b);

	/* *** */
	m.askpass = "should be cached";
	b = passwordFor("bill", "ted", &m, 0);
	if (strcmp(b, "xlnt") != 0) {
		printf("FAILURE: expected 'xlnt' got '%s'\n", b);
		return (1);
	}

	printf("SUCCESS: cached 'xlnt' correctly\n");

	/* *** */
	m.askpass = "echo abcdefghi1abcdefghi2abcdefghi3a; #";

	b = passwordFor("abbot", "costello", &m, 0);
	if (strcmp(b, "abcdefghi1abcdefghi2abcdefghi3a") != 0) {
		printf
			("FAILURE: expected 'abcdefghi1abcdefghi2abcdefghi3a' got '%s'\n",
			 b);
		return (1);
	}
	printf
		("SUCCESS: expected 'abcdefghi1abcdefghi2abcdefghi3a' got '%s'\n",
		 b);

	/* try overflowing the buffer */
	m.askpass = "echo abcdefghi1abcdefghi2abcdefghi3ab; #";
	b = passwordFor("laverne", "shirley", &m, 0);
	/* should come back truncated to fill the buffer */
	if (strcmp(b, "abcdefghi1abcdefghi2abcdefghi3a") != 0) {
		printf
			("FAILURE: expected 'abcdefghi1abcdefghi2abcdefghi3a' got '%s'\n",
			 b);
		return (1);
	}
	printf
		("SUCCESS: expected 'abcdefghi1abcdefghi2abcdefghi3a' got '%s'\n",
		 b);

	/* make sure we still have the old one */
	b = passwordFor("bill", "ted", &m, 0);
	if (strcmp(b, "xlnt") != 0) {
		printf("FAILURE: expected 'xlnt' got '%s'\n", b);
		return (1);
	}
	printf("SUCCESS: expected 'xlnt' got '%s'\n", b);

	/* what it's like if ssh-askpass is cancelled - should drop the mailbox */
#if 0
	/* will exit on our behalf; not so good for continued testing. */
	m.askpass = "echo -n ; #";
	b = passwordFor("abort", "me", &m, 0);
	if (strcmp(b, "") != 0) {
		printf("FAILURE: expected '' got '%s'\n", b);
		return (1);
	}
	printf("SUCCESS: expected '' got '%s'\n", b);
#endif

	/* what it's like if ssh-askpass is ok'd with an empty password. */
	m.askpass = "echo ; #";
	b = passwordFor("try", "again", &m, 0);
	if (b == NULL || strcmp(b, "") != 0) {
		printf("FAILURE: expected '' got '%s'\n", b ? b : "(null)");
		return (1);
	}
	printf("SUCCESS: expected '' got '%s'\n", b);

	m.askpass = "echo \"rt*m\"; #";
	b = passwordFor("faq", "faq", &m, 0);
	if (strcmp(b, "rt*m") != 0) {
		printf("FAILURE: expected '' got '%s'\n", b);
		return (1);
	}
	printf("SUCCESS: expected 'rt*m' got '%s'\n", b);
	return (0);
}

#define CKINT(x,shouldbe) if(x != shouldbe) { \
printf("Failed: expected '" #shouldbe "' but got '%d'\n", x); \
 return 1; }
int test_imap4creator(void)
{
	mbox_t m;

	if (imap4Create(&m, "imap:foo:@bar/mybox")) {
		return 1;
	}
	CKSTRING(m.path, "mybox");
	CKSTRING(m.u.pop_imap.serverName, "bar");
	CKINT(m.u.pop_imap.serverPort, 143);

	if (imap4Create(&m, "imap:foo:@bar/\"mybox\"")) {
		return 1;
	}
	CKSTRING(m.path, "\"mybox\"");
	CKSTRING(m.u.pop_imap.serverName, "bar");
	CKINT(m.u.pop_imap.serverPort, 143);

	if (imap4Create(&m, "imap:foo:@192.168.1.1/\"mybox\"")) {
		printf
			("FAILED: to create IMAP box with IP address for servername\n");
		return 1;
	}
	CKSTRING(m.path, "\"mybox\"");
	CKSTRING(m.u.pop_imap.serverName, "192.168.1.1");
	CKINT(m.u.pop_imap.serverPort, 143);

	if (imap4Create(&m, "imap:foo:@bar/\"space box\"")) {
		return 1;
	}
	CKSTRING(m.path, "\"space box\"");
	CKSTRING(m.u.pop_imap.serverName, "bar");
	CKINT(m.u.pop_imap.serverPort, 143);

	if (imap4Create(&m, "imap:user pass bar/\"space box\"")) {
		return 1;
	}
	CKSTRING(m.path, "\"space box\"");
	CKSTRING(m.u.pop_imap.serverName, "bar");
	CKINT(m.u.pop_imap.serverPort, 143);

	if (imap4Create(&m, "imap:star *as* star/\"space box\"")) {
		return 1;
	}
	printf("mmm %s", (m.u.pop_imap.password));
	DEFROB(m.u.pop_imap.password);
	CKSTRING(m.u.pop_imap.password, "*as*");
	CKINT(m.u.pop_imap.serverPort, 143);
	if (imap4Create(&m, "imap:user:*as*@bar/blah")) {
		return 1;
	}

	DEFROB(m.u.pop_imap.password);
	CKSTRING(m.u.pop_imap.password, "*as*");
	CKINT(m.u.pop_imap.serverPort, 143);

	if (imap4Create(&m, "imap:user pass bar/\"space box\" 12")) {
		return 1;
	}
	CKSTRING(m.path, "\"space box\"");
	CKSTRING(m.u.pop_imap.serverName, "bar");
	CKINT(m.u.pop_imap.serverPort, 12);

	if (imap4Create(&m, "imap:foo:@bar/\"mybox\":12")) {
		return 1;
	}
	CKSTRING(m.path, "\"mybox\"");
	CKSTRING(m.u.pop_imap.serverName, "bar");
	CKINT(m.u.pop_imap.serverPort, 12);

	if (imap4Create(&m, "imap:foo:@bar/\"mybox\":12 auth")) {
		return 1;
	}
	CKSTRING(m.path, "\"mybox\"");
	CKSTRING(m.u.pop_imap.serverName, "bar");
	CKINT(m.u.pop_imap.serverPort, 12);
	CKSTRING(m.u.pop_imap.authList, "auth");

	if (imap4Create(&m, "imap:foo:@bar/\"mybox\":12 cram-md5 plaintext")) {
		return 1;
	}
	CKSTRING(m.u.pop_imap.authList, "cram-md5 plaintext");

	if (imap4Create(&m, "imap:foo:@bar/\"mybox\":12 CRAm-md5 plainTEXt")) {
		return 1;
	}
	CKSTRING(m.u.pop_imap.authList, "cram-md5 plaintext");

	/* doesn't really matter, as the # is gobbled by the parser as a comment. */
	if (imap4Create
		(&m, "imap:harry:has#pass@bar/\"mybox\":12 CRAm-md5 plainTEXt")) {
		return 1;
	}
	CKSTRING(m.u.pop_imap.userName, "harry");
	DEFROB(m.u.pop_imap.password);
	CKSTRING(m.u.pop_imap.password, "has#pass");


	if (pop3Create(&m, "pop3:foo:@bar:12 cram-md5 plaintext")) {
		return 1;
	}
	CKSTRING(m.u.pop_imap.authList, "cram-md5 plaintext");

	/* should not parse this; it is ambiguous. */
	if (!imap4Create(&m, "imap:foo:mi@ta@bar/mybox") && !Relax) {
		return 1;
	}

	/* should not parse this; it is ambiguous. */
	if (!imap4Create(&m, "imap:user pa ss bar/\"space box\" 12") && !Relax) {
		return 1;
	}

	/* should not parse this; it is ambiguous. */
	if (!pop3Create(&m, "pop3:user pa ss bar 12") && !Relax) {
		return 1;
	}

	return 0;
}


int test_getline_from_buffer(void)
{
#define LINE_BUF_LEN 256
	char linebuf[LINE_BUF_LEN];
	char scratchbuf[LINE_BUF_LEN];

	/* try to ensure that even an endless loop terminates */
	alarm(100);
	strcpy(scratchbuf, "\r\n");
	getline_from_buffer(scratchbuf, linebuf, LINE_BUF_LEN);
	if (strlen(scratchbuf) != 0) {
		printf("FAILURE: scratchbuf not snarfed\n");
		return (1);
	}
	if (strlen(linebuf) != 2) {
		printf("FAILURE: linebuf not populated\n");
		return (1);
	}
	strcpy(scratchbuf, "\n");
	getline_from_buffer(scratchbuf, linebuf, LINE_BUF_LEN);
	if (strlen(scratchbuf) != 0) {
		printf("FAILURE: scratchbuf not snarfed\n");
		return (1);
	}
	if (strlen(linebuf) != 1) {
		printf("FAILURE: linebuf not populated\n");
		return (1);
	}

	alarm(0);
	return (0);
}

int test_charutil(void)
{

	char *v = strdup("abc#def");

	StripComment(v);
	if (strcmp(v, "abc#def") != 0) {
		printf("FAILURE: comment stripper stripped when it shouldn't\n");
		return 1;
	}

	v = strdup("abc #def");

	StripComment(v);
	if (strcmp(v, "abc ") != 0) {
		printf("FAILURE: comment stripper should've stripped\n");
		return 1;
	}


	return 0;
}

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
int test_sock_connect(void)
{
	struct sockaddr_in addr;
	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	socklen_t addrlen = sizeof(struct sockaddr_in);
	if (s < 0) {
		perror("socket");
		return 1;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = 0;
	addr.sin_port = 0;
	if (bind(s, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return 1;
	}
	getsockname(s, (struct sockaddr *)&addr, &addrlen);
	if (listen(s, 5) < 0) {
		perror("listen");
		return 1;
	}
	if (sock_connect("127.0.0.1", htons(addr.sin_port)) < 0) {
		return 1;
	}
	if (sock_connect("localhost", htons(addr.sin_port)) < 0) {
		return 1;
	}
	return 0;
}


int print_info(UNUSED(void *state))
{
	return (0);
}
const char *certificate_filename = NULL;
const char *tls = "NORMAL";
int SkipCertificateCheck = 0;
int exists(UNUSED(const char *filename))
{
	return (0);
}


// void initialize_blacklist(void) { }
// void tlscomm_printf(UNUSED(int x), UNUSED(const char *f), ...) { }
// void tlscomm_expect(void) {  }
// void tlscomm_close() {  }
// int tlscomm_is_blacklisted(UNUSED(const char *x)) {  return 1; }
// void initialize_gnutls(void) {  }
// int sock_connect(UNUSED(const char *n), UNUSED(int p)) { return 1; } /* stdout */
// void initialize_unencrypted(void) {  }

int main(UNUSED(int argc), UNUSED(char *argv[]))
{

	if (test_backtickExpand() || test_passwordMgr()) {
		printf("SOME TESTS FAILED!\n");
		exit(EXIT_FAILURE);
	}

	Relax = 0;
	if (test_imap4creator()) {
		printf("SOME TESTS FAILED!\n");
		exit(EXIT_FAILURE);
	}

	if (test_sock_connect()) {
		printf("SOME TESTS FAILED!\n");
		exit(EXIT_FAILURE);
	}

	Relax = 1;
	if (test_imap4creator()) {
		printf("SOME TESTS FAILED!\n");
		exit(EXIT_FAILURE);
	}

	if (test_getline_from_buffer()) {
		printf("SOME TESTS FAILED!\n");
		exit(EXIT_FAILURE);
	}

	if (test_charutil()) {
		printf("SOME TESTS FAILED!\n");
		exit(EXIT_FAILURE);
	}

	if (test_sock_connect()) {
		printf("SOME TESTS FAILED!\n");
		exit(EXIT_FAILURE);
	}

	printf("Success! on all tests.\n");
	exit(EXIT_SUCCESS);
}

/* vim:set ts=4: */
/*
 * Local Variables:
 * tab-width: 4
 * c-indent-level: 4
 * c-basic-offset: 4
 * End:
 */
