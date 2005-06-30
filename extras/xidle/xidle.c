/*	$OpenBSD: xidle.c,v 1.1 2005/06/30 01:48:45 fgsch Exp $	*/
/*
 * Copyright (c) 2005 Federico G. Schwindt.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE OPENBSD PROJECT AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OPENBSD
 * PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


enum {
	width  = 2,
	height = 2
};

enum {
	north = 0x01,
	south = 0x02,
	east  = 0x04,
	west  = 0x08
};

struct xinfo {
	Display		*dpy;
	Window		 win;
	int		 coord_x;
	int		 coord_y;

	int		 saver_event;	/* Only if Xss ext is available */

	int		 saved_timeout;
	int		 saved_interval;
	int		 saved_pref_blank;
	int		 saved_allow_exp;
};

struct xinfo x;
char	*program = PATH_XLOCK;
int	 timeout = 0;
int	 position = north|west;

const struct option longopts[] = {
	{ "display",	required_argument,	NULL,		'd' },
	{ "program",	required_argument,	NULL,		'p' },
	{ "timeout",	required_argument,	NULL,		't' },

	{ "ne",		no_argument,		&position,	north|east },
	{ "nw",		no_argument,		&position,	north|west },
	{ "se",		no_argument,		&position,	south|east },
	{ "sw",		no_argument,		&position,	south|west },

	{ NULL,		0,			NULL,		0 }
};

extern char *__progname;

void	init_x(const char *, struct xinfo *);
void	close_x(struct xinfo *);
void	action(struct xinfo *);
void	dump_event(XEvent *, struct xinfo *);
__dead void	usage(void);
__dead void	handler(int);


__dead void
usage()
{
	fprintf(stderr, "usage: %s %s\n", __progname,
	    "[-display host:dpy] [-ne | -nw | -sw | -sw] "
	    "[-program path] [-timeout seconds]");
	exit(1);
}


void
init_x(const char *display, struct xinfo *xi)
{
	XSetWindowAttributes attr;
	Display *dpy;
	Window win;
	int error, event;
	int screen;

	dpy = XOpenDisplay(display);
	if (!dpy) {
		errx(1, "Unable to open display %s", XDisplayName(display));
		/* NOTREACHED */
	}

	screen = DefaultScreen(dpy);

	if (position & south)
		xi->coord_y = DisplayHeight(dpy, screen) - height;
	if (position & east)
		xi->coord_x = DisplayWidth(dpy, screen) - width;

	attr.override_redirect = True;
	win = XCreateWindow(dpy, DefaultRootWindow(dpy),
	    xi->coord_x, xi->coord_y, width, height, 0, 0, InputOnly,
	    CopyFromParent, CWOverrideRedirect,  &attr);

	XMapWindow(dpy, win);
	XSelectInput(dpy, win, EnterWindowMask|StructureNotifyMask);

	if (timeout > 0 && 
	    XScreenSaverQueryExtension(dpy, &event, &error) == True) {
		xi->saver_event = event;

		XGetScreenSaver(dpy, &xi->saved_timeout, &xi->saved_interval,
		    &xi->saved_pref_blank, &xi->saved_allow_exp);

		XSetScreenSaver(dpy, timeout, 0, DontPreferBlanking,
		    DontAllowExposures);
		XScreenSaverSelectInput(dpy, DefaultRootWindow(dpy),
		    ScreenSaverNotifyMask);
	} else if (timeout > 0)
		warnx("XScreenSaver extension not available. "
		    "Timeout disabled.");

	xi->dpy = dpy;
	xi->win = win;
}


void
close_x(struct xinfo *xi)
{
	XSetScreenSaver(xi->dpy, xi->saved_timeout, xi->saved_interval,
	    xi->saved_pref_blank, xi->saved_allow_exp);
	XDestroyWindow(xi->dpy, xi->win);
	XCloseDisplay(xi->dpy);
}


void
action(struct xinfo *xi)
{
	int dumb;

	switch (fork()) {
	case -1:
		err(1, "fork");
		/* NOTREACHED */

	case 0:
		execlp(program, program, (char *)NULL);
		exit(0);
		/* NOTREACHED */

	default:
		wait(&dumb);
		XSync(xi->dpy, True);
		break;
	}
}


__dead void
handler(int sig)
{
	close_x(&x);
	exit(0);
	/* NOTREACHED */
}


int
main(int argc, char **argv)
{
	char *display = NULL, *p;
	int pflag;
	int c;

	pflag = 0;
	while ((c = getopt_long_only(argc, argv, "", longopts, NULL)) != -1) {
		switch (c) {
		case 'd':
			display = optarg;
			break;

		case 'p':
			program = optarg;
			break;

		case 't':
			timeout = strtol(optarg, &p, 10);
			if (*p || timeout < 0) {
				errx(1, "illegal value -- %s", optarg);
				/* NOTREACHED */
			}
			break;

		case 0:
			if (pflag) {
				errx(1, "Cannot specify multiple positions");
				/* NOTREACHED */
			}
			pflag++;
			break;

		default:
			usage();
			/* NOTREACHED */
		}
	}

	bzero(&x, sizeof(struct xinfo));

	init_x(display, &x);

	signal(SIGINT, handler);
	signal(SIGTERM, handler);

	for (;;) {
		XCrossingEvent *e;
		XEvent ev;

		XNextEvent(x.dpy, &ev);

		switch (ev.type) {
		case MapNotify:
			XMapRaised(x.dpy, x.win);
			break;

		case EnterNotify:
			e = (XCrossingEvent *)&ev;

			sleep(2);

			XQueryPointer(x.dpy, x.win, &e->root, &e->window,
			    &e->x_root, &e->y_root, &e->x, &e->y, &e->state);

			/* Check it was for real. */
			if (e->y > x.coord_y + height ||
			    e->x > x.coord_x + width)
				break;
			/* FALLTHROUGH */

		default:
			if (ev.type == EnterNotify ||
			    (ev.type == x.saver_event &&
			    ((XScreenSaverNotifyEvent *)&ev)->forced == False))
				action(&x);
			break;
		}
	}

	/* NOTREACHED */
}
