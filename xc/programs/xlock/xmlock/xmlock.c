#if !defined( lint ) && !defined( SABER )
static const char sccsid[] = "@(#)xmlock.c	4.08 98/02/18 xlockmore";

#endif

/*-
 * xmlock.c - main file for xmlock, the gui interface to xlock.
 *
 * Copyright (c) 1996 by Charles Vidal
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * Nov-96: Continual minor improvements by Charles Vidal and David Bagley.
 * Oct-96: written.
 */

/*-
  XmLock Problems
  1. Allowing only one in -inroot.  Need a way to kill it.
  2. XLock resources need to be read and used to set initial values.
  3. Integer and floating point and string input.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define HAVE_UNISTD_H 1
#endif /* HAVE_CONFIG_H */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>

#ifdef VMS
#include <descrip.h>
#include <lib$routines.h>
#endif

/* #include <Xm/XmAll.h> Does not work on my version of Lesstif */
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>

#if USE_XMU
#include <X11/Xmu/Editres.h>
#endif

#include "bitmaps/m-xlock.xbm"	/* icon bitmap */

/* like an enum */
#define  LAUNCH	0
#define  ROOT   1
#define  WINDOW 2
#define  EXIT   3

/* number of buttons, toggles, and string options */
#define  PUSHBUTTONS  4
#define  TOGGLES  9
#define  OPTIONS  8

#define XLOCK "xlock"
#define WINDOW_WIDTH 160
#define WINDOW_HEIGHT 100
#define WINDOW_GEOMETRY "160x100"

/* extern variable */
extern char *c_Options[OPTIONS];
extern Widget Menuoption;

static pid_t numberprocess = -1;	/* PID of xlock */

extern void Setup_Option(Widget MenuBar);

/* Widget */
Widget      toplevel;
static Widget Labelxlock;

static Widget ScrolledListModes, PushButtons[PUSHBUTTONS], Toggles[TOGGLES];

 /*Resource string */
#include "modes.h"

/* some resources of buttons and toggles not really good programming :( */

static char *r_PushButtons[PUSHBUTTONS] =
{
	"Launch",
	"In Root",
	"In Window",
	"Exit"
};

static char *r_Toggles[TOGGLES] =
{
	"mono",
	"nolock",
	"remote",
	"allowroot",
	"enablesaver",
	"allowaccess",
	"grabmouse",
	"echokeys",
	"usefirst"
};

char       *r_Options[OPTIONS] =
{
	"username",
	"password",
	"info",
	"validate",
	"invalid",
	"geometry",
	"font",
	"program"
};

static int  numberinlist = 0;

/* CallBack */
static void
f_PushButtons(Widget w, XtPointer client_data, XtPointer call_data)
{
	int         i, n;
	char        command[500];

#ifdef VMS
	int         mask = 17;
	struct dsc$descriptor_d vms_image;

#endif

	(void) sprintf(command, "%s ", XLOCK);

/* booleans (+/-) options */

	for (i = 0; i < TOGGLES; i++) {
		if (XmToggleButtonGetState(Toggles[i])) {
			(void) strcat(command, "-");
			(void) strcat(command, r_Toggles[i]);
			(void) strcat(command, " ");
		}
	}
	for (i = 0; i < OPTIONS; i++)
		if (c_Options[i] != NULL) {
			(void) strcat(command, "-");
			(void) strcat(command, r_Options[i]);
			(void) strcat(command, " ");
			(void) strcat(command, c_Options[i]);
			(void) strcat(command, " ");
		}
	switch ((int) client_data) {
		case LAUNCH:
			/* the default value then nothing to do */
			break;
		case WINDOW:
			(void) strcat(command, "-inwindow ");
			break;
		case ROOT:
			(void) strcat(command, "-inroot ");
			break;
		case EXIT:
			if (numberprocess != -1) {
				(void) kill(numberprocess, SIGKILL);
				(void) wait(&n);
			}
			exit(0);
			break;
	}
	(void) strcat(command, "-mode ");
	(void) strcat(command, LockProcs[numberinlist].cmdline_arg);
#ifdef VMS
	vms_image.dsc$w_length = strlen(command);
	vms_image.dsc$a_pointer = command;
	vms_image.dsc$b_class = DSC$K_CLASS_S;
	vms_image.dsc$b_dtype = DSC$K_DTYPE_T;
	(void) printf("%s\n", command);
	(void) lib$spawn(&vms_image, 0, 0, &mask);
#else
	(void) strcat(command, " & ");
	(void) printf("%s\n", command);
	(void) system(command);
#endif
}

static void
f_ScrolledListModes(Widget w, XtPointer client_data, XtPointer call_data)
{
	char        numberwidget[50];
	char        str[50];
	int         n;

	numberinlist = ((XmListCallbackStruct *) call_data)->item_position - 1;
	(void) sprintf(numberwidget, "%ld", XtWindow(Labelxlock));
	(void) sprintf(str, "%s", LockProcs[numberinlist].cmdline_arg);
	if (numberprocess != -1) {
		(void) kill(numberprocess, SIGKILL);
		(void) wait(&n);
	}
#ifdef VMS
#define FORK vfork
#else
#define FORK fork
#endif
	if ((numberprocess = FORK()) == -1)
		(void) fprintf(stderr, "Fork error\n");
	else if (numberprocess == 0) {
		(void) execlp(XLOCK, XLOCK, "-parent", numberwidget,
			      "-mode", str, "-geometry", WINDOW_GEOMETRY, "-delay", "100000",
			      "-nolock", "-inwindow", "+install", 0);
	}
}

/* Setup Widget */
static void
Setup_Widget(Widget father)
{
	Arg         args[15];
	int         i, ac = 0;
	Widget      Row, PushButtonRow, TogglesRow, ModeLabel, SwitchLabel;
	char        string[160];
	XmString    label_str;
	XmString    TabXmStr[numprocs];

	ac = 0;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNheight, WINDOW_HEIGHT);
	ac++;
	SwitchLabel = XmCreateLabel(father, "Switches", args, ac);
	XtManageChild(SwitchLabel);
	ac = 0;
	XtSetArg(args[ac], XmNheight, WINDOW_HEIGHT);
	ac++;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	ModeLabel = XmCreateLabel(father, "Modes", args, ac);
	XtManageChild(ModeLabel);
	ac = 0;
	XtSetArg(args[ac], XmNwidth, WINDOW_WIDTH);
	ac++;
	XtSetArg(args[ac], XmNheight, WINDOW_HEIGHT);
	ac++;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNleftWidget, ModeLabel);
	ac++;
	Labelxlock = XmCreateLabel(father, "Window", args, ac);
	XtManageChild(Labelxlock);

/* buttons in the bottom */
	ac = 0;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	PushButtonRow = XmCreateRowColumn(father, "PushButtonRow", args, ac);

	Menuoption = XmCreateMenuBar(PushButtonRow, "MenuBar", NULL, 0);
	XtManageChild(Menuoption);

	for (i = 0; i < PUSHBUTTONS; i++) {
		ac = 0;
		label_str = XmStringCreate(r_PushButtons[i], XmSTRING_DEFAULT_CHARSET);
		XtSetArg(args[ac], XmNlabelString, label_str);
		ac++;
		PushButtons[i] = XmCreatePushButton(PushButtonRow, r_PushButtons[i],
						    args, ac);
		XmStringFree(label_str);
		XtAddCallback(PushButtons[i], XmNactivateCallback, f_PushButtons,
			      (XtPointer) i);
		XtManageChild(PushButtons[i]);
	}
	XtManageChild(PushButtonRow);

/* list and toggles in row like that (row(list)(TogglesRow(toggles...))) */
	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNtopWidget, Labelxlock);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNbottomWidget, PushButtonRow);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	Row = XmCreateRowColumn(father, "Row", args, ac);

	for (i = 0; i < numprocs; i++) {
		(void) sprintf(string, "%-14s%s", LockProcs[i].cmdline_arg,
			       LockProcs[i].desc);
		TabXmStr[i] = XmStringCreate(string, XmSTRING_DEFAULT_CHARSET);
	}
	ac = 0;
	XtSetArg(args[ac], XmNitems, TabXmStr);
	ac++;
	XtSetArg(args[ac], XmNitemCount, numprocs);
	ac++;
	XtSetArg(args[ac], XmNvisibleItemCount, 10);
	ac++;
	ScrolledListModes = XmCreateScrolledList(Row, "ScrolledListModes",
						 args, ac);
	XtAddCallback(ScrolledListModes, XmNbrowseSelectionCallback,
		      f_ScrolledListModes, NULL);
	XtManageChild(ScrolledListModes);

	TogglesRow = XmCreateRowColumn(Row, "TogglesRow", NULL, 0);
	for (i = 0; i < TOGGLES; i++) {
		ac = 0;
		label_str = XmStringCreate(r_Toggles[i], XmSTRING_DEFAULT_CHARSET);
		XtSetArg(args[ac], XmNlabelString, label_str);
		ac++;
		Toggles[i] = XmCreateToggleButton(TogglesRow, r_Toggles[i], args, ac);
		XmStringFree(label_str);
		XtManageChild(Toggles[i]);
	}
	XtManageChild(TogglesRow);

	XtManageChild(Row);
	for (i = 0; i < numprocs; i++) {
		XmStringFree(TabXmStr[i]);
	}
}

int
main(int argc, char **argv)
{
	Widget      form;
	Arg         args[15];

/* PURIFY 4.0.1 on Solaris 2 reports an unitialized memory read on the next
   line. */
	toplevel = XtInitialize(argv[0], "XmLock", NULL, 0, &argc, argv);
	XtSetArg(args[0], XtNiconPixmap,
		 XCreateBitmapFromData(XtDisplay(toplevel),
				       RootWindowOfScreen(XtScreen(toplevel)),
			    (char *) image_bits, image_width, image_height));
	XtSetValues(toplevel, args, 1);
	/* creation Widget */
/* PURIFY 4.0.1 on Solaris 2 reports an unitialized memory read on the next
   line. */
	form = XmCreateForm(toplevel, "Form", NULL, 0);
	Setup_Widget(form);
	Setup_Option(Menuoption);
	XtManageChild(form);
	XtRealizeWidget(toplevel);
#if USE_XMU
	XtAddEventHandler(toplevel, (EventMask) 0, TRUE,
			  (XtEventHandler) _XEditResCheckMessages, NULL);
/* With this handler you can use editres */
#endif
	XtMainLoop();
#ifdef VMS
	return 1;
#else
	return 0;
#endif
}
