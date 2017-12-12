/*
 * Example on writing data with select().  Use select/connect-only.c which
 * connects but does not read anything unless a 3rd parameter is provided.
 * Alternatively, you can use BSD (ie. macOS as well) netcat with -i option
 * (GNU/Linux netcat uses -i for writing only, not for reading, and is not
 * useful in this case):
 *
 *	nc -i 5 localhost 2222
 *
 * The process should stop in select() after some data has been sent over to the
 * remote side and filled up its kernel buffer for the connection there.  As the
 * client side does not read (or reads slowly), the server side eventually gets
 * a TCP window 0 (which you could see with tcpdump) after a short while as the
 * kernel buffer holding the data to be read by the client is limited in size.
 *
 * You might need to run the client from a different machine; it might not
 * behave as expected on a localhost.  System can treat that as a special case.
 *
 * To see more, use "-i 1" with netcat above, and from another terminal, trace
 * system calls (eg. use "strace -p <pid>" on Linux) performed by the server.
 * You will see that from time to time, the select() returns and the kernel
 * buffer is filled up while the netcat slowly reads the data.
 *
 * (c) jp@devnull.cz
 */

#define	_XOPEN_SOURCE	700

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define	BUF_LEN	1024

int
main(int argc, char **argv)
{
	fd_set wrfds;
	char buf[BUF_LEN];
	struct sockaddr_in sa;
	int s, newsock, n, total = 0, optval = 1;

	if (argc != 2)
		errx(1, "usage: %s <port>", basename(argv[0]));

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		err(1, "socket");

	sa.sin_family = AF_INET;
	sa.sin_port = htons((short) atoi(argv[1]));
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	memset(buf, '.', sizeof (buf));

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval,
	    sizeof (optval)) == -1) {
		err(1, "setsockopt");
	}

	if (bind(s, (struct sockaddr *)&sa, sizeof (sa)) == -1)
		err(1, "bind");

	if (listen(s, SOMAXCONN) == -1)
		err(1, "listen");

	if ((newsock = accept(s, NULL, 0)) == -1)
		err(1, "accept");

	/*
	 * Just write data to the remote side.  If the remote side does not read
	 * the data we should be finally put to sleep in select().
	 */
	for (;;) {
		/* Must do this each time before calling select(). */
		FD_ZERO(&wrfds);
		FD_SET(newsock, &wrfds);
		if (select(newsock + 1, NULL, &wrfds, NULL, NULL) == -1)
			err(1, "select");

		if (FD_ISSET(newsock, &wrfds)) {
			if ((n = write(newsock, buf, sizeof (buf))) == -1)
				err(1, "write");
			total = total + n;
			fprintf(stderr, "[ %d bytes written (total %d) ]\n",
			    n, total);
		}
	}

	/* Not reached. */
	close(newsock);
	close(s);

	return (0);
}
