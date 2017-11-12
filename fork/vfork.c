/*
 * vfork() is obsolete but let's see how it works. This is the same program as
 * fork/fork.c aside from changing fork() into vfork() and letting both
 * processes execute the same amount of iterations. Now, the parent is
 * stopped until the child finishes which is easily seen from the output.
 *
 * See vfork() man page for details. This program could core dump if return
 * was used since a child is not supposed to use the parent's process address
 * space any other way than to exec() or _exit(). By using return, the
 * parent's stack would be changed. _exit() is used in favour of exit() to
 * avoid any atexit() routines to be called on behalf of the child.
 *
 * (c) jp@devnull.cz
 */

#define	_XOPEN_SOURCE 700	// needed for vfork()

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int
main(void)
{
	int i, pid;
	char *name;

	switch (pid = vfork()) {
	case -1:
		fprintf(stderr, "error\n");
		exit(1);
	case 0:
		printf("I'm child, PID is %d\n", getpid());
		name = "child";
		break;
	default:
		printf("I'm parent, PID is %d\n", getpid());
		name = "parent";
		break;
	}

	for (i = 0; i < 6; ++i) {
		printf("%s: loop %d\n", name, i);
		sleep(1);
	}

	/*
	 * If you return from here instead of exit()ing it may core dump based
	 * on the system used. It does so on FreeBSD 6.1, it doesn't on
	 * Solaris 10.
	 */
	_exit(0);
}
