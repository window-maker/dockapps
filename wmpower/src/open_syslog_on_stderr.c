/***************************************************************************
                   open_syslog_on_stderr.c  -  description
                             -------------------
    begin                : Feb 10 2003
    copyright            : (C) 2003,2004,2005 by Noberasco Michele
    e-mail               : noberasco.gnu@disi.unige.it
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.              *
 *                                                                         *
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>



/* this is the thread that will actually read
 * from our redirected stderr, forwarding to syslog
 */
void syslog_writer(char *filename)
{
	FILE *fp = fopen(filename, "r");
	char  buf[1024];

	if (!fp)
	{ /* we write this to stdout as stderr is probably compromised at this point */
		printf("wmpower: syslog_writer thread: fatal error: unable to hook to main thread's stderr!\n");
		abort();
	}

	/* inititalize syslog writer */
	openlog("wmpower", 0, LOG_USER);

	/* we remove it so that noone will tamper */
	unlink(filename);

	while (1)
	{
		fflush(NULL);

		while (fgets(buf, 1023, fp))
			syslog(LOG_INFO, "%s", buf);

		sleep(1);
	}
}

/*
 * Redirect writes to sterr on syslog...
 * we achieve this by starting a new thread, creating a file
 * that the father will write to, and the child read from,
 * rewriting it to syslog. Also, we unlink() the file as soon
 * as both father and child have it open, so that no other
 * process will be able to tamper with it.
 */
void open_syslog_on_stderr(void)
{
	pthread_t  thread;
	char      *filename;
	int        fd;

	filename = (char*) malloc(19*sizeof(char));
	
	if (!filename)
	{
		fprintf(stderr, "wmpower: fatal error: failed allocating memory!\n");
		abort();
	}

	strcpy(filename, "/tmp/wmpowerXXXXXX");

	fd = mkstemp(filename);
	if (fd == -1)
	{
		fprintf(stderr, "wmpower: fatal error: could not create temporary file!\n");
		abort();
	}

	/* set file mode to 600 */
	if (chmod(filename, S_IRUSR|S_IWUSR))
	{
		fprintf(stderr, "wmpower: fatal error: cannot chmod() file!\n");
		abort();		
	}

	if (!freopen(filename, "w", stderr))
	{
		fprintf(stderr, "wmpower: fatal error: unable to redirect stderr!\n");
		abort();
	}

	/* close the file descriptor as we have no need for it */
	close(fd);
	
	/* spawn our syslog_writer baby */
	if (pthread_create(&thread, NULL, (void*)(&syslog_writer), filename))
	{
		fprintf(stderr, "wmpower: fatal error: failed to create syslog-writer thread!\n");
		abort();
	}
}
