/*****************************************************************************
 *
 * xautolock
 * =========
 *
 * Authors   : M. Eyckmans (MCE) + S. De Troch (SDT)
 *
 * Date      :  22/07/1990
 *
 * Comments  :  - Patchlevel 1->6 for private use only.
 *              - Patchlevel  7 released: 07/04/1992
 *              - Patchlevel  8 released: 15/05/1992
 *              - Patchlevel  9 released: 24/01/1995
 *              - Patchlevel 10 released: 22/02/1995
 *              - Patchlevel 11 released: 03/11/1997
 *              - Patchlevel 12 released: 09/08/1998
 *              - Patchlevel 13 released: 13/08/1998
 *              - Patchlevel 14 released: 18/11/1998
 *              - Patchlevel 15 released: 13/03/1999
 *
 * Review    :  - 12/02/1992 (MCE) :
 *                . Hacked around a dxcalendar problem.
 *              - 21/02/1992 (MCE) :
 *                . Major rewrite.
 *              - 24/02/1992 (MCE) :
 *                . Removed an initialization bug.
 *              - 25/02/1992 (MCE) :
 *                . Added code to detect multiple invocations.
 *              - 06/03/1992 (MCE) :
 *                . Re-arranged the event loop in order to detect defunct
 *                  children as soon as possible.
 *              - 10/03/1992 (SDT & MCE) :
 *                . Added code to detect broken server connections.
 *              - 24/03/1992 (MCE) :
 *                . Don't reset the timeout counter after receiving a
 *                  synthetic or otherwise unexpected event.
 *              - 15/04/1992 (MCE) :
 *                . Changed the default locker to "xlock 2>&- 1>&-".
 *                . Fixed a couple of event mask bugs. (Thanks to
 *                  Jamie Zawinski <jwz@lucid.com> for running into these.)
 *                . Corrected a property type bug in CheckConnection ().
 *              - 20/04/1992 (MCE) :
 *                . Cut Main () into more managable pieces.
 *                . Periodically call XQueryPointer ().
 *              - 25/04/1992 (MCE) :
 *                . Added the `corners' feature. (Suggested by
 *                  Neil I. Weisenfeld <weisen@alw.nih.gov>.)
 *                . Fixed a problem with pseudo-root windows. 
 *                  (Thnaks to Chris Sherman <sherman@unx.sas.com>,
 *                  Nathan Edwards <nedwards@titan.trl.oz.au>, Dave 
 *                  Hayes <dave@elxr.jpl.nasa.gov>, and Tom McConnell
 *                  <tmcconne@sedona.intel.com> for pointing out 
 *                  the problem and testing the patch.)
 *                . Added `disable/enable on SIGHUP'. (Suggested by
 *                  Paul Smith <paul_smith@dg.com>.)
 *                . Added support for multi-headed displays. 
 *              - 28/04/1992 (MCE) :
 *                . Use the X resource manager.
 *              - 06/05/1992 (MCE) :
 *                . Fixed a few potential portability problems. (Thanks
 *                  to Paul Smith <paul_smith@dg.com> again.)
 *                . CheckConnection () now works properly on multi-headed
 *                  displays. (Thanks to Brian ? <brian@natinst.com> for
 *                  testing the `multi-headed' support.)
 *                . Better version of Sleep ().
 *                . Recognize X resources for class "Xautolock".
 *                . Don't update timer while sighupped.
 *                . Switched to vfork () and execl ().
 *                . New copyright notice.
 *              - 11/05/1992 (MCE) :
 *                . Close stdout and stderr instead of using "2>&- 1>&-".
 *                  (Suggested by Rainer Sinkwitz <sinkwitz@ifi.unizh.ch>.)
 *                . Added "-noclose" for debugging. 
 *              - 08/07/1992 (MCE) :
 *                . Efficiency improvements and code embellishments
 *                . Improved conditional "#include"s etc. (Thanks to
 *                  Jonathan I. Kamens <jik@pit-manager.mit.edu> and
 *                  Fred J.R. Appelman <fred@cv.ruu.nl>.)
 *                . Moved a couple of premature calls to free ().
 *                  (Purify sure is a great tool!)
 *                . Fixed a race condition related to the `corners'
 *                  feature.
 *                . Fixed a minor initialization bug. 
 *              - 21/12/1992 (MCE) :
 *                . Added code to circumvent a server initialisation bug
 *                  (OpenWindows 2.0 and 3.0) related to XQueryPointer ().
 *                  (Thanks to Eric Engstrom <engstrom@src.honeywell.com> 
 *                  for providing the patch.)
 *              - 22/06/1993 (MCE) :
 *                . Reset screensaver upon locking the screen.
 *                  (Suggested by Paul Mossip <mossip@vizlab.rutgers.edu>.)
 *                . Improved resource usage.
 *              - 13/08/1993 (MCE) :
 *                . Added "-cornerredelay" for reasons described in the
 *                  man page.
 *              - 23/12/1993 (MCE) :
 *                . Improved "#ifdef"s for SYSV. (Thanks to John Somerfield 
 *                  <John.Somerfield@barclays.co.uk>.)
 *              - 11/05/1994 (MCE) :
 *                . Corrected a "real stupid typo" ;-).
 *              - 25/08/1994 (MCE) :
 *                . More accurate "usage" message.
 *              - 21/09/1994 (MCE) :
 *                . Several minor code embellishments.
 *                . Better wording of the copyright statement.
 *                . Ported to VMS. (Thanks to Brian D. Reed
 *                  <bdr@cbnewsg.cb.att.com> for providing the 
 *                  nitty-gritty details.)
 *                . Resources now have a (dummy) resource class.
 *                  (Thanks to JF Bonhomme <johnny@cett.alcatel-alsthom.fr>
 *                  for pointing out that something had to be done here.)
 *                . Reworked resource processing. (Thanks to Joerg Lehrke
 *                  <jlehrke@wmi.physik.tu-muenchen.de> for providing 
 *                  the original patch (stripped by me).)
 *                . Create a dummy window for proper XKillCLient ()
 *                  behaviour when using xdm without XDMCP or similar.
 *                . Added "-nocloseout" and "-nocloseerr".
 *              - 14/10/1994 (MCE) :
 *                . Finally added Xidle support.
 *                . Return value of waitpid () on SYSV was being
 *                  used incorrectly.
 *              - 01/11/1994 (MCE) :
 *                . Added SIGUSR1 and SIGUSR2 support, as well as "-enable",
 *                  "-disable", "-toggle" options.(Thanks to Christopher 
 *                  Davis <ckd@loiosh.kei.com> for the initial patch.)
 *                . Renamed some stuff for better maintainability.
 *              - 06/11/1994 (MCE) :
 *                . Several minor corrections for VMS.
 *                . Added #define _HPUX_SOURCE for c89 on HP/UX.
 *                . Totally reworked time-keeping code to protect it
 *                  against incorrect implementations of sleep ().
 *              - 10/11/1994 (MCE) :
 *                . Added "-notifier" option. (Based on a suggestion
 *                  by Steve Woodard <woodard@peach.kodak.com>.)
 *                . Made the "xxx_SEMAPHORE_PID" stuff honour the
 *                  prog_name stuff.
 *                . Casting fixes related to use of time_t.
 *              - 21/11/1994 (MCE) :
 *                . Added "#ifdef"s as needed by Novell Unixware. 
 *                  (Thanks to Thanh Ma <tma@encore.com> for reporting this.)
 *                . Plugged a minor memory leak in the resource
 *                  management code.
 *              - 03/01/1995 (MCE) :
 *                . Finally solved the logout problems under OpenWinDows.
 *                  (Thanks to the many people who reported this one in
 *                  the past, and in particular to John Kent 
 *                  <badger@ssd.intel.com>for putting me on the right track.)
 *                . Some minor cosmetic changes.
 *              - 20/01/1995 (MCE) :
 *                . Take the modifier mask into account when looking
 *                  for pointer activity. (Idea taken from xscreensaver,
 *                  which is by Jamie Zawinski <jwz@mcom.com>.)
 *                . Fixed a minor oversight in option handling.
 *                . Fixed some uninitialised memory problems, a rare
 *                  null pointer dereference and a small memory leak.
 *                  (Purify sure is a great tool!)
 *              - 23/01/1995 (MCE) :
 *                . Fixed various things ProLint complained about.
 *                . Fixed yet another minor oversight in option handling.
 *              - 01/02/1995 (MCE) :
 *                . Added a few unused intialisations because otherwise
 *                  some compilers complain about them missing.
 *              - 21/02/1995 (MCE) :
 *                . Initial cleaning up of the #ifdef and #include stuff.
 *                . Be less pedantic when validating the notification
 *                  margin if the `corners' feature is not being used.
 *                . Fixed a horrificly stupid blooper that was sometimes 
 *                  causing the thing not to work at all. (Thanks to
 *                  Don Lewis <gdonl@gv.ssi1.com> and Ben Suurmeijer
 *                  <ben@telecom.ptt.nl> for attracting my attention 
 *                  to this one.)
 *              - 28/02/1995 (MCE) :
 *                . Added correct setup of the standard WM properties.
 *              - 21/04/1995 (MCE) :
 *                . Better wording of the copyright notice 
 *                  (no fundamental change).
 *              - 08/05/1995 (MCE) :
 *                . Fixed a race condition in SelectEvents ().
 *              - 15/05/1995 (MCE) :
 *                . Removed the INITIAL_SLEEP stuff.
 *              - 28/01/1996 (MCE) :
 *                . Minor stylistic editing.
 *              - 29/04/1996 (MCE) :
 *                . Fixed a serious problem with the Xidle support.
 *                  (Thanks to Lam N <lamn@cs.cuhk.hk> for attracting my
 *                  attention to this one.)
 *              - 07/05/1996 (MCE) :
 *                . Allow the user to exploit the entire range of
 *                  bell_percent values supported by XBell().
 *                  (Suggested by Eddy De Greef <degreef@imec.be>.)
 *              - 23/06/1996 (MCE) :
 *                . Incorporated the "-secure", "-killtime" and
 *                  "-killpid" options. (Based on a patch provided
 *                  by Tony Mione <mione@nbcs.rutgers.edu>.)
 *              - 25/06/1996 (MCE) :
 *                . General clean up.
 *              - 21/08/1996 (MCE) :
 *                . Do not free() the return value of getenv().
 *                  (Pointed out by Stefan Jais <stefan.jais@pr-steyr.ac.at>.)
 *              - 03/04/1997 (MCE) :
 *                . Cleaned up the (unreleased) debugging code.
 *                . Fixed some minor bugs concerning "-killpid" and
 *                  "-killtime".
 *              - 30/06/1997 (MCE) :
 *                . Added the "-resetsaver" option to aid people with
 *                  DPMS monitors. (Triggered by Ian Rawlings 
 *                  <ian@tarcus.demon.co.uk>.)
 *              - 10/08/1997 (MCE) :
 *                . On VMS, use LIB$SPAWN rather than vfork to work
 *                  around DECC/VAXC interoperability problems. (Taken
 *                  from one of the many VMS branch versions.) Untested!
 *                . Added code to properly handle the -locker option
 *                  to the VMS version. (Based on a patch by
 *                  Erez Gur <gur.erez@a1.tavis.iso.mts.dec.com>.) Untested!
 *              - 25/10/1997 (MCE) :
 *                . Merged in lots of VMS portability fixes taken 
 *                  from the latest VMS version. (Original code
 *                  by Hunter Goatley <goathunter@madgoat.com>.) Untested!
 *                . Replaced the "-killpid" option with the "-killer"
 *                  one to allow for more flexibility.
 *              - 27/10/1997 (MCE) :
 *                . Added support for the MIT ScreenSaver extension.
 *                . Made gcc -Wall shut up on Linux and HP-UX 10.
 *              - 14/11/1997 (MCE) :
 *                . Read resources from $HOME/.Xdefaults as a last resort.
 *                  (Suggested by Marc Baudoin <Marc.Baudoin@solsoft.com>.)
 *              - 08/04/1998 (MCE) :
 *                . Cleaned up some of the stuff that LCLint complained 
 *                  about (but definitely not all of it).
 *              - 03/06/1998 (MCE) :
 *                . Extended the (unreleased) debugging code somewhat.
 *              - 09/08/1998 (MCE) :
 *                . Plugged a memory leak in the $HOME/.Xdefaults code.
 *                . Updated this changelog to always mention the full 
 *                  name of people. Provided I know it, that is...
 *              - 11/08/1998 (MCE) :
 *                . Include pwd.h to make the $HOME/.Xdefaults stuff
 *                  compile on more platforms. (Problem reported by
 *                  Jens Schleusener <Jens.Schleusener@dlr.de>.)
 *              - 19/10/1998 (MCE) :
 *                . Added the "-exit" option. (Based on a patch by 
 *                  Yakov Yukhnovetsky <yakov.yukhnovetsky@telrad.co.il>.)
 *              - 05/02/1999 (MCE) :
 *                . The VMS port should now actually work more or less.
 *                  (Thanks to Jouk Jansen <joukj@hrem.stm.tudelft.nl>
 *                  for taking the trouble to actually submit a patch.)
 *                . Fixed an include file conflict on AIX. (Also thanks 
 *                  to Jouk Jansen <joukj@hrem.stm.tudelft.nl>.)
 *              - 15/02/1999 (MCE) :
 *                . Added the "-locknow", "-unlocknow", and "-nowlocker"
 *                  options. (Thanks to Timo J Rinne <tri@iki.fi> for the
 *                  original patch (slightly modified by me).)
 *                . Some minor clean up.
 *              - 16/02/1999 (MCE) :
 *                . Ripped out the use of signals to communicate with
 *                  other xautolock processes. This removes the limit
 *                  on the number of possible messages (yesterday's
 *                  change was pushing it very hard) and makes things 
 *                  more portable as well.
 *              - 11/03/1999 (MCE) :
 *                . Fixed the communication code to always do the right
 *                  thing when handling "-exit".
 *
 * ---------------------------------------------------------------------------
 *
 * Please send bug reports to eyckmans@imec.be.
 *
 * ---------------------------------------------------------------------------
 *
 * Copyright 1990, 1992-1999 by S. De Troch and M. Eyckmans (MCE).
 *
 * Permission to use, copy, modify and distribute this software and the
 * supporting documentation without fee is hereby granted, provided that
 *
 *  1 : Both the above copyright notice and this permission notice
 *      appear in all copies of both the software and the supporting
 *      documentation.
 *  2 : No commercial use is made out of it.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *****************************************************************************/



/*
 *  Have a guess what this does...
 *  ==============================
 *
 *  Warning for swm & tvtwm users : xautolock should *not* be compiled
 *  with vroot.h, because it needs to know the real root window.
 */

#if defined(hpux) || defined (__hpux)
#ifndef _HPUX_SOURCE
#define _HPUX_SOURCE
#endif /* _HPUX_SOURCE */
#endif /* hpux || __hpux */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifdef __STDC__
#include <string.h>
#else /* __STDC__ */
#include <strings.h>
#endif /* __STDC__ */

#ifdef VMS
#define HasVFork

#include <descrip.h>
#include <clidef.h>
#include <lib$routines.h>
#include <ssdef.h>
#include <ssdef.h>    
#include <processes.h>
#include <unixio.h>     /* Needed for close ()  */
#include <unixlib.h>    /* Needed for getpid () */
#endif /* VMS */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#ifdef HasXidle
#include "xidle.h"
#endif /* HasXidle */

#ifdef HasScreenSaver
#include <X11/extensions/scrnsaver.h>
#endif /* HasScreenSaver */

#include <time.h>
#include <signal.h>

#ifndef VMS
#include <pwd.h>
#include <sys/wait.h>
#endif /* VMS */

#include <sys/types.h>

#ifdef UNDEF
/*
 *  Somebody once told me to insert this, but later on others
 *  reported that it conflicts with sys/wait.h included above.
 *  Feel free to experiment if things don't compile out of the 
 *  box on your AIX version.
 */
#ifdef AIXV3    
#include <sys/m_wait.h>
#endif /* AIXV3 */
#endif

#if !defined (news1800) && !defined (sun386)

#if !defined (NOSTDHDRS)
#include <stdlib.h>
#endif /* !NOSTDHDRS */

#if !defined (apollo) && !defined (VMS)
#include <malloc.h>
#include <unistd.h>
#endif /* !apollo && !VMS */

#endif /* !news1800 && !sun386 */

#include "patchlevel.h"




/*
 *  Usefull macros and customization stuff
 *  ======================================
 */
#ifdef HasPrototypes
#define PP(x)                      x
#else /* HasPrototypes */
#define PP(x)                      ()
#endif /* HasPrototypes */


#define SECURE                     False   /* default -secure setting      */
#define BELL_PERCENT               40      /* as is says                   */

#define MIN_LOCK_MINS              1       /* minimum number of minutes
                                              before firing up the locker  */
#define LOCK_MINS                  10      /* default ...                  */
#define MAX_LOCK_MINS              60      /* maximum ...                  */

#define MIN_KILL_MINS              10      /* minimum number of minutes
                                              before firing up the killer  */
#define KILL_MINS                  20      /* default ...                  */
#define MAX_KILL_MINS              120     /* maximum ...                  */

#define CREATION_DELAY             30      /* should be > 10 and
                                              < min(45,(MIN_LOCK_MINS*30)) */
#define CORNER_SIZE                10      /* size in pixels of the
                                              force-lock areas             */
#define CORNER_DELAY               5       /* number of seconds to wait
                                              before forcing a lock        */
#ifdef VMS
#define SLOW_VMS_DELAY             15      /* explained in VMS.NOTES file  */
#endif /* VMS */

#define MES_DISABLE                1       /* as it says                   */
#define MES_ENABLE                 2       /* as it says                   */
#define MES_TOGGLE                 3       /* as it says                   */
#define MES_EXIT                   4       /* as it says                   */
#define MES_LOCKNOW		   5       /* as it says                   */
#define MES_UNLOCKNOW		   6       /* as it says                   */

#define APPLIC_CLASS               "Xautolock"
                                           /* application class.           */
#define DUMMY_RESOURCE_CLASS       "_xAx_" /* some versions of X don't
                                              like the old NULL class name,
                                              and I consider implementing
                                              real classes isn't worth it  */
#define SEM_PID "_SEMAPHORE_PID"           /* used to locate other 
					      xautolocks                   */
#define MESSAGE "_MESSAGE"                 /* used to talk to other
					      xautolocks                   */

#ifdef VMS
#define LOCKER                     "mcr sys$system:decw$pausesession"
#else /* VMS */
#define LOCKER                     "xlock" /* avoid the -allowRoot option! */
#endif /* VMS */
#define NOTIFIER                   ""
#define KILLER                     ""

#ifndef HasVFork
#define vfork                      fork
#endif /* HasVFork */

#define Main                       main
#define forever                    for (;;)
#define Min(a,b)                   ((a) < (b) ? (a) : (b))
#define Max(a,b)                   ((a) > (b) ? (a) : (b))
#define Error0(s)                  (Void) fprintf (stderr, (s))
#define Error1(s,a1)               (Void) fprintf (stderr, (s), (a1))
#define Error2(s,a1,a2)            (Void) fprintf (stderr, (s), (a1), (a2))

#define SetLockTrigger(delta)      (lock_trigger = time ((time_t*) NULL)      \
                                                         + (delta))           \

#define DisableKillTrigger()       (kill_trigger = 0)
#define SetKillTrigger(delta)      (kill_trigger = time ((time_t*) NULL)      \
                                                         + (delta))           \

static caddr_t                     c_ptr;  /* this is dirty */
#define Skeleton(t,s)              (c_ptr = (Caddrt) malloc ((Unsigned) (s)), \
                                      (c_ptr == (Caddrt) NULL)                \
                                    ? (Error0 ("Out of memory.\n"),           \
                                       exit (EXIT_FAILURE),                   \
                                       /*NOTREACHED*/ (t*) NULL               \
                                      )                                       \
                                    : (t*) c_ptr                              \
                                   )                                          \

#define New(tp)                    Skeleton (tp, sizeof (tp))
#define NewArray(tp,n)             Skeleton (tp, sizeof (tp) * (Unsigned) (n))




/*
 *  New types
 *  =========
 */
#if defined (apollo) || defined (news1800) 
typedef int                        (*XErrorHandler) PP((Display*,
                                                        XErrorEvent*));
#endif /* apollo || news1800 */

#if defined (news1800) || defined (sun386) 
typedef int                        pid_t;
#endif /* news1800  || sun386 */

#ifdef VMS

#if defined(VAX) && defined(vaxc)
typedef long                       pid_t;
#endif /* VMS && VAX && vaxc */

/*
 *  Different versions of the DEC C compiler for OpenVMS Alpha define
 *  pid_t in different ways. Later versions define either __PID_T (v5.2)
 *  or __DEV_T (V5.0) when pid_t is already typedef'ed. DEC C V1.3 did
 *  neither, so typedef if those aren't defined.
 *
 */
#if     defined(__DECC) && defined(__alpha)     \
    && !defined(____PID_T) && !defined(__DEV_T)
typedef long                       pid_t;
#endif /* __DECC && __alpha && !____PID_T && !__DEV_T */

#endif /* VMS */


#define Void                       void     /* no typedef because of VAX */
typedef int                        Int;
typedef char                       Char;
typedef char*                      String;
typedef int                        Boolean;
typedef caddr_t                    Caddrt;
typedef unsigned int               Unsigned;
typedef unsigned long              Huge;

typedef Boolean                    (*OptAction)  PP((Display*, String));
typedef Void                       (*OptChecker) PP((Display*));

typedef enum
        {
          IGNORE,                                 /* ignore this corner  */
          DONT_LOCK,                              /* never lock          */
          FORCE_LOCK                              /* lock immediately    */
        } CornerAction;

typedef struct QueueItem_
        {
          Window                   window;        /* as it says          */
          time_t                   creationtime;  /* as it says          */
          struct QueueItem_*       next;          /* as it says          */
          struct QueueItem_*       prev;          /* as it says          */
        } aQueueItem, *QueueItem;

typedef struct Queue_
        {
          struct QueueItem_*       head;          /* as it says          */
          struct QueueItem_*       tail;          /* as it says          */
        } aQueue, *Queue;

typedef struct Opt_
        {
          const String             name;          /* as it says          */
          XrmOptionKind            kind;          /* as it says          */
          Caddrt                   value;         /* XrmOptionNoArg only */
          OptAction                action;        /* as it says          */
          OptChecker               checker;       /* as it says          */
        } anOpt, *Opt;




/*
 *  Function declarations
 *  =====================
 */
#if defined(news1800) 
extern Void*    malloc                PP((Unsigned));
#endif /* news1800 */
 
static Void     Usage                 PP((Int));
static Void     EvaluateTriggers      PP((Display*));
static Void     QueryIdleTime         PP((Display*, Bool));
static Void     QueryPointer          PP((Display*));
static Void     ProcessEvents         PP((Display*, Queue));
static Queue    NewQueue              PP((Void));
static Void     AddToQueue            PP((Queue, Window));
static Void     ProcessQueue          PP((Queue, Display*, time_t));
static Void     SelectEvents          PP((Display*, Window, Boolean));
static Void     CheckConnectionAndSendMessage
				      PP((Display*));
#ifdef VMS
static Int      PollSmPauseWindow     PP((Display*));
#endif /* VMS */
static Int      CatchFalseAlarm       PP((Display*, XErrorEvent));
static Void     ProcessOpts           PP((Display*, Int, String*));
static Boolean  HelpAction            PP((Display*, String));
static Boolean  VersionAction         PP((Display*, String));
static Boolean  LockerAction          PP((Display*, String));
static Boolean  NowLockerAction       PP((Display*, String));
static Boolean  KillerAction          PP((Display*, String));
static Boolean  NotifierAction        PP((Display*, String));
static Boolean  CornersAction         PP((Display*, String));
static Boolean  CornerSizeAction      PP((Display*, String));
static Boolean  CornerDelayAction     PP((Display*, String));
static Boolean  CornerReDelayAction   PP((Display*, String));
static Boolean  LockTimeAction        PP((Display*, String));
static Boolean  KillTimeAction        PP((Display*, String));
static Boolean  NotifyAction          PP((Display*, String));
static Boolean  BellAction            PP((Display*, String));
static Boolean  SecureAction          PP((Display*, String));
static Boolean  EnableAction          PP((Display*, String));
static Boolean  DisableAction         PP((Display*, String));
static Boolean  ToggleAction          PP((Display*, String));
static Boolean  ExitAction            PP((Display*, String));
static Boolean  LockNowAction         PP((Display*, String));
static Boolean  UnlockNowAction       PP((Display*, String));
static Boolean  ResetSaverAction      PP((Display*, String));
static Boolean  NoCloseOutAction      PP((Display*, String));
static Boolean  NoCloseErrAction      PP((Display*, String));
static Boolean  NoCloseAction         PP((Display*, String));
static Boolean  GetInteger            PP((String, Int*));
static Boolean  GetPositive           PP((String, Int*));
static Void     LockerChecker         PP((Display*));
static Void     NowLockerChecker      PP((Display*));
static Void     KillerChecker         PP((Display*));
static Void     NotifierChecker       PP((Display*));
static Void     LockTimeChecker       PP((Display*));
static Void     KillTimeChecker       PP((Display*));
static Void     NotifyChecker         PP((Display*));
static Void     CornerSizeChecker     PP((Display*));
static Void     CornerReDelayChecker  PP((Display*));
static Void     BellChecker           PP((Display*));
static Void     DisableByMessage      PP((Void));
static Void     EnableByMessage       PP((Void));
static Void     ToggleByMessage       PP((Void));
static Void     ExitByMessage         PP((Void));
static Void     LockNowByMessage      PP((Void));
static Void     UnlockNowByMessage    PP((Void));




/*
 *  Global variables
 *  ================
 */
static pid_t         locker_pid = 0;         /* as it says                 */
static time_t        lock_trigger = 0;       /* as it says                 */
static time_t        kill_trigger = 0;       /* as it says                 */
static String        prog_name;              /* as it says                 */
static String        locker = LOCKER;        /* as it says                 */
static String        now_locker = LOCKER;    /* as it says                 */
static String        notifier = NOTIFIER;    /* as it says                 */
static String        killer = KILLER;        /* as it says                 */
static time_t        lock_time = LOCK_MINS;  /* as it says                 */
static time_t        kill_time = KILL_MINS;  /* as it says                 */
static time_t        notify_margin;          /* as it says                 */
static Int           secure = SECURE;        /* as it says                 */
static Int           bell_percent = BELL_PERCENT;
                                             /* as it says                 */
static Int           corner_size = CORNER_SIZE;
                                             /* as it says                 */
static time_t        corner_delay = CORNER_DELAY;
                                             /* as it says                 */
static time_t        corner_redelay;         /* as it says                 */
static Boolean       bell_specified = False; /* as it says                 */
static Boolean       notifier_specified = False;
                                             /* as it says                 */
static Boolean       killer_specified = False;
                                             /* as it says                 */
static Boolean       kill_time_specified = False;
                                             /* as it says                 */
static Boolean       corner_redelay_specified = False;
                                             /* as it says                 */
static Boolean       notify_lock = False;    /* whether to notify the user
                                                before locking             */
static Boolean       disabled = False;       /* whether to ignore all
                                                timeouts                   */
static Boolean       lock_now = False;       /* whether to lock screen
                                                immediately                */
static Boolean       unlock_now = False;     /* whether to unlock screen
                                                immediately                */
static Int           message_to_send = 0;    /* message to send to an 
                                                already running xautolock  */
static Atom          semaphore;              /* semaphore property for 
						locating an already 
						running xautolock          */
static Atom          message_atom;           /* message property for
						talking to another 
						xautolock                  */
static Boolean       use_redelay = False;    /* as it says                 */
static CornerAction  corners[4] = { IGNORE, IGNORE, IGNORE, IGNORE };
                                             /* default CornerActions      */
static Boolean       reset_saver = False;    /* whether to reset the X 
						screensaver                */
static Boolean       close_out = True;       /* whether to close stdout    */
static Boolean       close_err = True;       /* whether to close stderr    */

#ifdef VMS
struct dsc$descriptor locker_d;              /* used to fire up the locker */
struct dsc$descriptor now_locker_d;          /* used to fire up the locker *
Int                   vms_status = 1;        /* locker completion status   */
#endif /* VMS */

static anOpt         options[] = 
                     {
                       {"help"         , XrmoptionNoArg , (Caddrt) ""  ,
			 HelpAction         , (OptChecker) NULL         },
                       {"version"      , XrmoptionNoArg , (Caddrt) ""  ,
			 VersionAction      , (OptChecker) NULL         },
                       {"locker"       , XrmoptionSepArg, (Caddrt) NULL,
			 LockerAction       , LockerChecker             },
                       {"nowlocker"    , XrmoptionSepArg, (Caddrt) NULL,
			 NowLockerAction    , NowLockerChecker          },
                       {"killer"       , XrmoptionSepArg, (Caddrt) NULL,
			 KillerAction       , KillerChecker             },
		       {"notifier"     , XrmoptionSepArg, (Caddrt) NULL,
			 NotifierAction     , NotifierChecker           },
		       {"corners"      , XrmoptionSepArg, (Caddrt) NULL,
			 CornersAction      , (OptChecker) NULL         },
                       {"cornersize"   , XrmoptionSepArg, (Caddrt) NULL,
			 CornerSizeAction   , CornerSizeChecker         },
                       {"cornerdelay"  , XrmoptionSepArg, (Caddrt) NULL,
			 CornerDelayAction  , (OptChecker) NULL         },
                       {"cornerredelay", XrmoptionSepArg, (Caddrt) NULL,
			 CornerReDelayAction, CornerReDelayChecker      },
                       {"killtime"     , XrmoptionSepArg, (Caddrt) NULL,
			 KillTimeAction     , KillTimeChecker           },
                       {"time"         , XrmoptionSepArg, (Caddrt) NULL,
			 LockTimeAction     , LockTimeChecker           },
                       {"notify"       , XrmoptionSepArg, (Caddrt) NULL,
			 NotifyAction       , NotifyChecker             },
                       {"bell"         , XrmoptionSepArg, (Caddrt) NULL,
			 BellAction         , BellChecker               },
                       {"secure"       , XrmoptionNoArg , (Caddrt) ""  ,
			 SecureAction       , (OptChecker) NULL         },
                       {"enable"       , XrmoptionNoArg , (Caddrt) ""  ,
			 EnableAction       , (OptChecker) NULL         },
                       {"disable"      , XrmoptionNoArg , (Caddrt) ""  ,
			 DisableAction      , (OptChecker) NULL         },
                       {"toggle"       , XrmoptionNoArg , (Caddrt) ""  ,
			 ToggleAction       , (OptChecker) NULL         },
                       {"exit"         , XrmoptionNoArg , (Caddrt) ""  ,
			 ExitAction         , (OptChecker) NULL         },
                       {"locknow"      , XrmoptionNoArg , (Caddrt) ""  ,
			 LockNowAction      , (OptChecker) NULL         },
                       {"unlocknow"    , XrmoptionNoArg , (Caddrt) ""  ,
			 UnlockNowAction    , (OptChecker) NULL         },
                       {"resetsaver"   , XrmoptionNoArg , (Caddrt) ""  ,
			 ResetSaverAction   , (OptChecker) NULL         },
                       {"noclose"      , XrmoptionNoArg , (Caddrt) ""  ,
			 NoCloseAction      , (OptChecker) NULL         },
                       {"nocloseout"   , XrmoptionNoArg , (Caddrt) ""  ,
			 NoCloseOutAction   , (OptChecker) NULL         },
                       {"nocloseerr"   , XrmoptionNoArg , (Caddrt) ""  ,
			 NoCloseErrAction   , (OptChecker) NULL         },
                     };                      /* as it says, the order is
                                                important!                 */




/*
 *  Resource database related functions
 *  ===================================
 *
 *  Support functions
 *  -----------------
 */
static Boolean  GetInteger (arg, in)
String  arg;  /* string to scan                  */
Int*    in;   /* adress where to store the stuff */

{
  Char  c;          /* dummy            */
  Int   old = *in;  /* backup old value */

  if (sscanf (arg, "%d%c", in, &c) == 1)
  {
    return True;
  }
  
  *in = old;
  return False;
}



static Boolean  GetPositive (arg, pos)
String  arg;  /* string to scan                  */
Int*    pos;  /* adress where to store the stuff */

{
  Char  c;           /* dummy            */
  Int   old = *pos;  /* backup old value */

  if (   sscanf (arg, "%d%c", pos, &c) == 1
      && *pos >= 0
     )
  {
    return True;
  }
  
  *pos = old;
  return False;
}


/*
 *  Action functions
 *  ----------------
 */
/*ARGSUSED*/
static Boolean  HelpAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  Usage (EXIT_SUCCESS);

  /*NOTREACHED*/
  return True;  /* for lint and gcc */
}


/*ARGSUSED*/
static Boolean  VersionAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  Error2 ("%s : patchlevel %s\n", prog_name, PATCHLEVEL);
  exit (EXIT_SUCCESS);

  /*NOTREACHED*/
  return True;  /* for lint and gcc */
}


/*ARGSUSED*/
static Boolean  LockerAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  now_locker = locker = arg;
  return True;
}


/*ARGSUSED*/
static Boolean  NowLockerAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  now_locker = arg;
  return True;
}


/*ARGSUSED*/
static Boolean  KillerAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  killer_specified = True;
  killer = arg;
  return True;
}


/*ARGSUSED*/
static Boolean  NotifierAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  notifier_specified = True;
  notifier = arg;
  return True;
}


/*ARGSUSED*/
static Boolean  CornersAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  Int  c;  /* loop counter */

  if (strlen (arg) != 4) return False;
  
  for (c = -1; ++c < 4; )
  {
    switch (arg[c])
    {
      case '0' :
        corners[c] = IGNORE;
        continue;

      case '-' :
        corners[c] = DONT_LOCK;
        continue;

      case '+' :
        corners[c] = FORCE_LOCK;
        continue;

      default :
        return False;
    }
  }

  return True;
}


/*ARGSUSED*/
static Boolean  CornerSizeAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  return GetPositive (arg, &corner_size);
}


/*ARGSUSED*/
static Boolean  CornerDelayAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  Int      tmp = 0;  /* temporary storage */
  Boolean  res;      /* return value      */

  if ((res = GetPositive (arg, &tmp))) /* = intended */
  {
    corner_delay = (time_t) tmp;
  }

  return res;
}


/*ARGSUSED*/
static Boolean  CornerReDelayAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  Int      tmp = 0;  /* temporary storage */
  Boolean  res;      /* return value      */

  if ((res = GetPositive (arg, &tmp))) /* = intended */
  {
    corner_redelay = (time_t) tmp;
  }

  corner_redelay_specified = True;
  return res;
}


/*ARGSUSED*/
static Boolean  LockTimeAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  Int      tmp = 0 ;  /* temporary storage */
  Boolean  res;       /* return value      */

  if ((res = GetPositive (arg, &tmp))) /* = intended */
  {
    lock_time = (time_t) tmp; 
  }

  return res;
}


/*ARGSUSED*/
static Boolean  KillTimeAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  Int      tmp = 0 ;  /* temporary storage */
  Boolean  res;       /* return value      */

  if ((res = GetPositive (arg, &tmp))) /* = intended */
  {
    kill_time = (time_t) tmp;
  }

  kill_time_specified = True;
  return res;
}


/*ARGSUSED*/
static Boolean  NotifyAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  Int  tmp = 0;  /* temporary storage */

  if ((notify_lock = GetPositive (arg, &tmp))) /* = intended */
  {
    notify_margin  = (time_t) tmp;
  }

  return notify_lock;
}


/*ARGSUSED*/
static Boolean  BellAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  bell_specified = True;
  return GetInteger (arg, &bell_percent);
}


/*ARGSUSED*/
static Boolean  SecureAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  secure = True;
  return True;
}


/*ARGSUSED*/
static Boolean  DisableAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* program name    */

{
  if (message_to_send) return False;
  message_to_send = MES_DISABLE;
  return True;  
}


/*ARGSUSED*/
static Boolean  EnableAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* program name    */

{
  if (message_to_send) return False;
  message_to_send = MES_ENABLE;
  return True;  
}


/*ARGSUSED*/
static Boolean  ToggleAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* program name    */

{
  if (message_to_send) return False;
  message_to_send = MES_TOGGLE;
  return True;  
}


/*ARGSUSED*/
static Boolean  ExitAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* program name    */

{
  if (message_to_send) return False;
  message_to_send = MES_EXIT;
  return True;  
}


/*ARGSUSED*/
static Boolean  LockNowAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* program name    */

{
  if (message_to_send) return False;
  message_to_send = MES_LOCKNOW;
  return True;  
}


/*ARGSUSED*/
static Boolean  UnlockNowAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* program name    */

{
  if (message_to_send) return False;
  message_to_send = MES_UNLOCKNOW;
  return True;  
}


/*ARGSUSED*/
static Boolean  ResetSaverAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  reset_saver = True;
  return True;
}


/*ARGSUSED*/
static Boolean  NoCloseOutAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  close_out = False;
  return True;
}


/*ARGSUSED*/
static Boolean  NoCloseErrAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  close_err = False;
  return True;
}


/*ARGSUSED*/
static Boolean  NoCloseAction (d, arg)
Display*  d;    /* display pointer */
String    arg;  /* argument value  */

{
  (Void) NoCloseOutAction (d, arg);
  (Void) NoCloseErrAction (d, arg);
  return True;
}



/*
 *  Consistency checkers
 *  --------------------
 */
/*ARGSUSED*/
static Void  LockTimeChecker (d)
Display*  d;  /* display pointer */

{
  if (lock_time < MIN_LOCK_MINS)
  {
    Error1 ("Setting lock time to minimum value of %ld minute(s).\n",
            (long) (lock_time = MIN_LOCK_MINS));
  }
  else if (lock_time > MAX_LOCK_MINS)
  {
    Error1 ("Setting lock time to maximum value of %ld minute(s).\n",
            (long) (lock_time = MAX_LOCK_MINS));
  }

  lock_time *= 60; /* convert to seconds */
}


/*ARGSUSED*/
static Void  KillTimeChecker (d)
Display*  d;  /* display pointer */

{
  if (kill_time_specified && !killer_specified)
  {
    Error0 ("Using -killtime without -killer makes no sense.\n");
    return;
  }
 
  if (kill_time < MIN_KILL_MINS)
  {
    Error1 ("Setting kill time to minimum value of %ld minute(s).\n",
            (long) (kill_time = MIN_KILL_MINS));
  }
  else if (kill_time > MAX_KILL_MINS)
  {
    Error1 ("Setting kill time to maximum value of %ld minute(s).\n",
            (long) (kill_time = MAX_KILL_MINS));
  }

  kill_time *= 60; /* convert to seconds */
}


/*ARGSUSED*/
static Void  LockerChecker (d)
Display*  d;  /* display pointer */

{
#ifndef VMS
  String  tmp;  /* as it says */

 /*
  *  Let's manipulate the locker command a bit
  *  in order to reduce resource usage. 
  */
  (Void) sprintf (tmp = NewArray (Char, strlen (locker) + 6),
		  "exec %s", locker);
  locker = tmp;
#else /* VMS */
 /*
  *  Translate things to something that VMS knows how to handle.
  */
  locker_d.dsc$w_length = (unsigned short) strlen (locker);
  locker_d.dsc$b_class = DSC$K_CLASS_S;
  locker_d.dsc$b_dtype = DSC$K_DTYPE_T;
  locker_d.dsc$a_pointer = locker;
#endif /* VMS */
}


/*ARGSUSED*/
static Void  NowLockerChecker (d)
Display*  d;  /* display pointer */

{
#ifndef VMS
  String  tmp;  /* as it says */

 /*
  *  Let's manipulate the now_locker command a bit
  *  in order to reduce resource usage. 
  */
  (Void) sprintf (tmp = NewArray (Char, strlen (now_locker) + 6),
		  "exec %s", now_locker);
  now_locker = tmp;
#else /* VMS */
 /*
  *  Translate things to something that VMS knows how to handle.
  */
  now_locker_d.dsc$w_length = (unsigned short) strlen (now_locker);
  now_locker_d.dsc$b_class = DSC$K_CLASS_S;
  now_locker_d.dsc$b_dtype = DSC$K_DTYPE_T;
  now_locker_d.dsc$a_pointer = now_locker;
#endif /* VMS */
}


/*ARGSUSED*/
static Void  NotifierChecker (d)
Display*  d;  /* display pointer */

{
  if (strcmp (notifier, ""))
  {
    if (!notify_lock)
    {
      Error0 ("Using -notifier without -notify makes no sense.\n");
    }
#ifndef VMS
    else
    {
      String  tmp;  /* as it says */

     /*
      *  Add an `&' to the notifier command, so that it always gets put 
      *  run as a background process and things will work out properly 
      *  later. The rationale behind this hack is explained elsewhere.
      */
      (Void) sprintf (tmp = NewArray (Char, strlen (notifier) + 3),
		      "%s &", notifier);
      notifier = tmp;
    }
#endif /* !VMS */
  }
}


/*ARGSUSED*/
static Void  KillerChecker (d)
Display*  d;  /* display pointer */

{
#ifndef VMS
  if (strcmp (killer, ""))
  {
    String  tmp;  /* as it says */

   /*
    *  Add an `&' to the notifier command, so that it always gets 
    *  run as a background process and things will work out properly 
    *  later. The rationale behind this hack is explained elsewhere.
    */
    (Void) sprintf (tmp = NewArray (Char, strlen (killer) + 3),
		    "%s &", killer);
    killer = tmp;
  }
#endif /* !VMS */
}


/*ARGSUSED*/
static Void  NotifyChecker (d)
Display*  d;  /* display pointer */

{
  if (   notify_lock
      && (   corners[0] == FORCE_LOCK  
	  || corners[1] == FORCE_LOCK
	  || corners[2] == FORCE_LOCK
	  || corners[3] == FORCE_LOCK))
  {
    time_t min_delay = Min (corner_delay, corner_redelay);

    if (notify_margin > min_delay)
    {
      Error1 ("Notification time reset to %ld second(s).\n",
              (long) (notify_margin = min_delay));
    }

    if (notify_margin > lock_time / 2)
    {
      Error1 ("Notification time reset to %ld seconds.\n",
              (long) (notify_margin = lock_time / 2));
    }
  }
}


/*ARGSUSED*/
static Void  BellChecker (d)
Display*  d;  /* display pointer */

{
  if (bell_specified)
  {
    if (!notify_lock)
    {
      Error0 ("Using -bell without -notify makes no sense.\n");
      bell_percent = 0;
    }
    else if (notifier_specified)
    {
      Error0 ("Using both -bell and -notifier makes no sense.\n");
      bell_percent = 0;
    }
  }

  if (   bell_percent < -100
      || bell_percent >  100)
  {
    Error1 ("Bell percentage reset to %d%%.\n",
            bell_percent = BELL_PERCENT);
  }
}


/*ARGSUSED*/
static Void  CornerSizeChecker (d)
Display*  d;  /* display pointer */

{
  Int      s;                /* screen index   */
  Screen*  scr;              /* screen pointer */
  Int      max_corner_size;  /* as it says     */

  for (max_corner_size = 32000, s = -1; ++s < ScreenCount (d); )
  {
    scr = ScreenOfDisplay (d, s);

    if (   max_corner_size > WidthOfScreen (scr) / 4
        || max_corner_size > HeightOfScreen (scr) / 4)
    {
      max_corner_size = Min (WidthOfScreen (scr), HeightOfScreen (scr)) / 4;
    }
  }

  if (corner_size > max_corner_size)
  {
    Error1 ("Corner size reset to %d pixels.\n",
            corner_size = max_corner_size);
  }
}


/*ARGSUSED*/
static Void  CornerReDelayChecker (d)
Display*  d;  /* display pointer */

{
  if (!corner_redelay_specified)
  {
    corner_redelay = corner_delay;
  }
}



/*
 *  Function for informing the user about syntax errors
 *  ---------------------------------------------------
 */
static Void  Usage (exit_code)
Int  exit_code;  /* as it says */

{
  String  blanks;  /* string full of blanks */
  size_t  len;     /* number of blanks      */


 /*
  *  The relative overhead is enormous here, but who cares.
  *  I'm a perfectionist and Usage () doesn't return anyway.
  */
  len = strlen ("Usage :  ") + strlen (prog_name);
  (Void) memset (blanks = NewArray (Char, len + 1), ' ', len);
  blanks[len] = '\0';


 /*
  *  This is where the actual work gets done...
  */
  Error0 ("\n");
  Error1 ("Usage : %s ", prog_name);
  Error0 ("[-help][-version][-time mins][-locker locker]\n");
  Error1 ("%s[-killtime mins][-killer killer]\n", blanks);
  Error1 ("%s[-notify margin][-notifier notifier][-bell percent]\n", blanks);
  Error1 ("%s[-corners xxxx][-cornerdelay secs]\n", blanks);
  Error1 ("%s[-cornerredelay secs][-cornersize pixels]\n", blanks);
  Error1 ("%s[-nocloseout][-nocloseerr][-noclose]\n", blanks);
  Error1 ("%s[-enable][-disable][-toggle][-exit][-secure]\n", blanks);
  Error1 ("%s[-locknow][-unlocknow][-nowlocker locker]\n", blanks);
  Error1 ("%s[-resetsaver]\n", blanks);

  Error0 ("\n");
  Error0 (" -help               : print this message and exit.\n");
  Error0 (" -version            : print version number and exit.\n");
  Error0 (" -time mins          : time before locking the screen");
  Error2 (" [%d <= mins <= %d].\n", MIN_LOCK_MINS, MAX_LOCK_MINS);
  Error0 (" -locker locker      : program used to lock.\n");
  Error0 (" -nowlocker locker   : program used to lock immediately.\n");
  Error0 (" -killtime killmins  : time after locking at which to run\n");
  Error2 ("                       the killer [%d <= killmins <= %d].\n",
                                  MIN_KILL_MINS, MAX_KILL_MINS);
  Error0 (" -killer killer      : program used to kill.\n");
  Error0 (" -notify margin      : notify this many seconds before locking.\n");
  Error0 (" -notifier notifier  : program used to notify.\n");
  Error0 (" -bell percent       : loudness of notification beeps.\n");
  Error0 (" -corners xxxx       : corner actions (0, +, -) in this order :\n");
  Error0 ("                       topleft topright bottomleft bottomright\n");
  Error0 (" -cornerdelay secs   : time to lock screen in a `+' corner.\n");
  Error0 (" -cornerredelay secs : time to relock screen in a `+' corner.\n");
  Error0 (" -cornersize pixels  : size of corner areas.\n");
  Error0 (" -nocloseout         : do not close stdout.\n");
  Error0 (" -nocloseerr         : do not close stderr.\n");
  Error0 (" -noclose            : close neither stdout nor stderr.\n");
  Error0 (" -enable             : enable a running xautolock.\n");
  Error0 (" -disable            : disable a running xautolock.\n");
  Error0 (" -toggle             : toggle a running xautolock.\n");
  Error0 (" -locknow            : tell a running xautolock to lock.\n");
  Error0 (" -unlocknow          : tell a running xautolock to unlock.\n");
  Error0 (" -exit               : kill a running xautolock.\n");
  Error0 (" -secure             : ignore enable, disable, toggle,\n");
  Error0 ("                       locknow, and unlocknow messages.\n");
  Error0 (" -resetsaver         : reset the screensaver when starting "
                                  "the locker.\n");

  Error0 ("\n");
  Error0 ("Defaults :\n");

  Error0 ("\n");
  Error1 ("  time          : %d minutes\n"  , LOCK_MINS   );
  Error1 ("  locker        : %s\n"          , LOCKER      );
  Error1 ("  nowlocker     : %s\n"          , LOCKER      );
  Error1 ("  killtime      : %d minutes\n"  , KILL_MINS   );
  Error0 ("  killer        : none\n"                      );
  Error0 ("  notify        : don't notify\n"              );
  Error0 ("  notifier      : none\n"                      );
  Error1 ("  bell          : %d%%\n"        , BELL_PERCENT);
  Error0 ("  corners       : 0000\n"                      );
  Error1 ("  cornerdelay   : %d seconds\n"  , CORNER_DELAY);
  Error1 ("  cornerredelay : %d seconds\n"  , CORNER_DELAY);
  Error1 ("  cornersize    : %d pixels\n"   , CORNER_SIZE );

  Error0 ("\n");
  Error1 ("Patchlevel : %s\n", PATCHLEVEL);

  Error0 ("\n");

  exit (exit_code);
}



/*
 *  Function for processing command line arguments and defaults
 *  -----------------------------------------------------------
 */
static Void  ProcessOpts (d, argc, argv)
Display*  d;       /* display pointer     */
Int       argc;    /* number of arguments */
String    argv[];  /* array of arguments  */

{
  Int                nof_options = sizeof (options) / sizeof (options[0]);
                                /* number of supported options   */
  Int                j;         /* loop counter                  */
  Unsigned           l;         /* temporary storage             */
  Unsigned           max_l;     /* temporary storage             */
  Char*              dummy;     /* as it says                    */
  String             fullname;  /* full resource name            */
  String             str;       /* temporary storage             */
  XrmValue           value;     /* resource value container      */
  XrmOptionDescList  xoptions;  /* optionslist in Xlib format    */
  XrmDatabase        resc_db = (XrmDatabase) NULL;
                                /* resource file database        */
  XrmDatabase        cmdl_db = (XrmDatabase) NULL;
                                /* command line options database */


 /*
  *  Collect defaults from various places except the command line into one
  *  resource database, then parse the command line options into an other.
  *  Both databases are not merged, because we want to know where exactly
  *  each resource value came from.
  *
  *  One day I might extend this stuff to fully cover *all* possible
  *  resource value sources, but... One of the problems is that various
  *  pieces of documentation make conflicting claims with respect to the
  *  proper order in which resource value sources should be accessed.
  */
  XrmInitialize();

  if (XResourceManagerString (d) != (String) NULL)
  {
    XrmMergeDatabases (XrmGetStringDatabase (XResourceManagerString (d)),
		       &resc_db);
  }
  else if ((str = getenv ("XENVIRONMENT")) != (String) NULL)
  {
    XrmMergeDatabases (XrmGetFileDatabase (str), &resc_db);
  }
#ifndef VMS
  else
  {
    struct passwd *passwd;
    String home, path;
    XrmDatabase Xdefaults;
 
    passwd = getpwuid (getuid ());

    if (passwd != NULL)
    {
      home = passwd->pw_dir;
    }
    else
    {
      home = getenv ("HOME");

      if (home == NULL)
      {
        home = ".";
      }
    }

    path = NewArray (Char, strlen (home) + strlen ("/.Xdefaults") + 1);
    (Void) sprintf (path, "%s/.Xdefaults", home);
    Xdefaults = XrmGetFileDatabase (path);

    if (Xdefaults != NULL)
    {
      XrmMergeDatabases (Xdefaults, &resc_db);
    }

    free (path);
  }
#endif /* VMS */

  xoptions = NewArray (XrmOptionDescRec, nof_options);

  for (j = -1, max_l = 0; ++j < nof_options; )
  {
    l = strlen (options[j].name) + 1;
    max_l = Max (max_l, l);

    (Void) sprintf (xoptions[j].option = NewArray (Char, l + 1),
	            "-%s", options[j].name);
    (Void) sprintf (xoptions[j].specifier = NewArray (Char, l + 1),
                    ".%s", options[j].name);
    xoptions[j].argKind = options[j].kind;
    xoptions[j].value = options[j].value;
  }

  XrmParseCommand (&cmdl_db, xoptions, nof_options, prog_name, &argc, argv);

  if (--argc) Usage (EXIT_FAILURE);


 /*
  *  Let's be perfect...
  */
  {
    Unsigned  class_l = strlen (APPLIC_CLASS);  /* temporary storage */
    Unsigned  prog_l  = strlen (prog_name);     /* temporary storage */

    fullname = NewArray (Char, Max (prog_l, class_l) + max_l + 1);
  }


 /*
  *  Call the action functions.
  */
  for (j = -1; ++j < nof_options; )
  {
    (Void) sprintf (fullname, "%s%s", prog_name, xoptions[j].specifier);

    if (   XrmGetResource (cmdl_db, fullname, DUMMY_RESOURCE_CLASS,
                           &dummy, &value)
        == True)
    {
      if (!(*(options[j].action)) (d, value.addr))
      {
	Usage (EXIT_FAILURE); 
      }
    }
    else if (   XrmGetResource (resc_db, fullname, DUMMY_RESOURCE_CLASS,
                                &dummy, &value)
             == True)
    {
      if (!(*(options[j].action)) (d, value.addr))
      {
        Error2 ("Can't interprete \"%s\" for \"%s\", using default.\n", 
                value.addr, fullname);
      }
    }
    else
    {
      (Void) sprintf (fullname, "%s%s", APPLIC_CLASS, xoptions[j].specifier);

      if (   (   XrmGetResource (resc_db, fullname, DUMMY_RESOURCE_CLASS,
                                 &dummy, &value)
              == True)
          && !(*(options[j].action)) (d, value.addr))
      {
        Error2 ("Can't interprete \"%s\" for \"%s\", using default.\n", 
                value.addr, fullname);
      }
    }
  }


 /*
  *  Call the consistency checkers.
  */
  for (j = -1; ++j < nof_options; )
  {
    if (options[j].checker != (OptChecker) NULL)
    {
      (*(options[j].checker)) (d);
    }
  }


 /*
  *  General clean up.
  */
  XrmDestroyDatabase (cmdl_db);
  XrmDestroyDatabase (resc_db);

  for (j = -1; ++j < nof_options; )
  {
    free (xoptions[j].option);
    free (xoptions[j].specifier);
  }

  free (xoptions);
  free (fullname);
}




/*
 *  Functions related to the window queue
 *  =====================================
 *
 *  Function for creating a new queue
 *  ---------------------------------
 */
static Queue  NewQueue ()

{
  Queue  queue;  /* return value */

  queue = New (aQueue);
  queue->tail = New (aQueueItem);
  queue->head = New (aQueueItem);

  queue->tail->next = queue->head;
  queue->head->prev = queue->tail;
  queue->tail->prev = queue->head->next = (QueueItem) NULL;

  return queue;
}



/*
 *  Function for adding an item to a queue
 *  --------------------------------------
 */
static Void  AddToQueue (queue, window)
Queue   queue;   /* as it says */
Window  window;  /* as it says */

{
  QueueItem  new;  /* new item */

  new = New (aQueueItem);

  new->window = window;
  new->creationtime = time ((time_t*) NULL);
  new->next = queue->tail->next;
  new->prev = queue->tail;
  queue->tail->next->prev = new;
  queue->tail->next = new;
}



/*
 *  Function for processing those entries that are old enough
 *  ---------------------------------------------------------
 */
static Void  ProcessQueue (queue, d, age)
Queue     queue;  /* as it says      */
Display*  d;      /* display pointer */
time_t    age;    /* required age    */

{
  QueueItem  current;  /* as it says */
  time_t     now;      /* as it says */

  (Void) time (&now);
  current = queue->head->prev;

  while (   current->prev
         && current->creationtime + age < now
        )
  {
    SelectEvents (d, current->window, False);
    current = current->prev;
    free (current->next);
  }

  current->next = queue->head;
  queue->head->prev = current;
}




/*
 *  Functions related to (the lack of) user activity
 *  ================================================
 *
 *  Function for processing the event queue
 *  ---------------------------------------
 */
static Void  ProcessEvents (d, queue)
Display*  d;      /* display pointer */
Queue     queue;  /* as it says      */

{
  XEvent  event;  /* as it says */


 /*
  *  Read whatever is available for reading.
  */
  while (XPending (d))
  {
    if (XCheckMaskEvent (d, SubstructureNotifyMask, &event))
    {
      if (event.type == CreateNotify)
      {
        AddToQueue (queue, event.xcreatewindow.window);
      }
    }
    else
    {
      (Void) XNextEvent (d, &event);
    }


   /*
    *  Reset the triggers if and only if the event is a
    *  KeyPress event *and* was not generated by XSendEvent ().
    */
    if (   event.type == KeyPress
        && !event.xany.send_event)
    {
      SetLockTrigger (lock_time);
      if (kill_trigger) SetKillTrigger (kill_time);
    }
  }


 /*
  *  Check the window queue for entries that are older than
  *  CREATION_DELAY seconds.
  */
  ProcessQueue (queue, d, (time_t) CREATION_DELAY);
}



/*
 *  Function for gettint the idle time
 *  ----------------------------------
 */
/*ARGSUSED*/
static Void  QueryIdleTime (d, use_xidle)
Display*  d;          /* display pointer         */
Bool      use_xidle;  /* use xidle to do the job */

{
  Time  idle_time = 0;  /* millisecs since last input event */

#ifdef HasXidle
  if (use_xidle)
  {
    XGetIdleTime (d, &idle_time);
  }
  else
#endif /* HasXIdle */
  {
#ifdef HasScreenSaver
    static XScreenSaverInfo* mit_info = 0; 

    if (!mit_info)
    {
      mit_info = XScreenSaverAllocInfo ();
    }

    XScreenSaverQueryInfo (d, DefaultRootWindow (d), mit_info);
    idle_time = mit_info->idle;
#endif /* HasScreenSaver */
  }

  if (idle_time < 1000)  
  {
    SetLockTrigger (lock_time);
    if (kill_trigger) SetKillTrigger (kill_time);
  }
}



/*
 *  Function for monitoring pointer movements
 *  -----------------------------------------
 */
static Void  QueryPointer (d)
Display*  d;  /* display pointer */

{
  Window           dummy_w;            /* as it says                    */
  Int              dummy_c;            /* as it says                    */
  Unsigned         mask;               /* modifier mask                 */
  Int              root_x;             /* as it says                    */
  Int              root_y;             /* as it says                    */
  Int              corner;             /* corner index                  */
  time_t           now;                /* as it says                    */
  time_t           new_trigger;        /* temporary storage             */
  Int              i;                  /* loop counter                  */
  static Window    root;               /* root window the pointer is on */
  static Screen*   screen;             /* screen the pointer is on      */
  static Unsigned  prev_mask = 0;      /* as it says                    */
  static Int       prev_root_x = -1;   /* as it says                    */
  static Int       prev_root_y = -1;   /* as it says                    */
  static Boolean   first_call = True;  /* as it says                    */


 /*
  *  Have a guess...
  */
  if (first_call)
  {
    first_call = False;
    root = DefaultRootWindow (d);
    screen = ScreenOfDisplay (d, DefaultScreen (d));
  }


 /*
  *  Find out whether the pointer has moved. Using XQueryPointer for this
  *  is gross, but it also is the only way never to mess up propagation
  *  of pointer events.
  *
  *  Remark : Unlike XNextEvent(), XPending () doesn't notice if the
  *           connection to the server is lost. For this reason, earlier
  *           versions of xautolock periodically called XNoOp (). But
  *           why not let XQueryPointer () do the job for us, since
  *           we now call that periodically anyway?
  */
  if (!XQueryPointer (d, root, &root, &dummy_w, &root_x, &root_y,
                      &dummy_c, &dummy_c, &mask))
  {
   /*
    *  Pointer has moved to another screen, so let's find out which one.
    */
    for (i = -1; ++i < ScreenCount (d); ) 
    {
      if (root == RootWindow (d, i)) 
      {
        screen = ScreenOfDisplay (d, i);
        break;
      }
    }
  }

  if (   root_x == prev_root_x
      && root_y == prev_root_y
      && mask == prev_mask)
  {
   /*
    *  If the pointer has not moved since the previous call and 
    *  is inside one of the 4 corners, we act according to the
    *  contents of the "corners" array.
    *
    *  If root_x and root_y are less than zero, don't lock even if
    *  FORCE_LOCK is set in the upper-left corner. Why? 'cause
    *  on initial server startup, IF the pointer is never moved,
    *  XQueryPointer returns values less than zero (only some
    *  servers, Openwindows 2.0 and 3.0 in particular).
    */
    if (   (corner = 0,
               root_x <= corner_size && root_x >= 0
            && root_y <= corner_size && root_y >= 0)
        || (corner++,
               root_x >= WidthOfScreen  (screen) - corner_size - 1
            && root_y <= corner_size)
        || (corner++,
               root_x <= corner_size
            && root_y >= HeightOfScreen (screen) - corner_size - 1)
        || (corner++,
               root_x >= WidthOfScreen  (screen) - corner_size - 1
            && root_y >= HeightOfScreen (screen) - corner_size - 1))
    {
      (Void) time (&now);

      switch (corners[corner])
      {
        case FORCE_LOCK :
          new_trigger =   now - 1
                        + (use_redelay ? corner_redelay : corner_delay);

          if (new_trigger < lock_trigger)
          {
            SetLockTrigger (new_trigger - now);
          }
          break;

        case DONT_LOCK :
          SetLockTrigger (lock_time);
          if (kill_trigger) SetKillTrigger (kill_time);

#ifdef __GNUC__
	default: /* Makes gcc -Wall shut up. */
#endif /* __GNUC__ */
      }
    }
  }
  else
  {
    use_redelay = False;
    prev_root_x = root_x;
    prev_root_y = root_y;
    prev_mask = mask;

    SetLockTrigger (lock_time);
    if (kill_trigger) SetKillTrigger (kill_time);
  }
}



/*
 *  Function for deciding whether to lock or kill
 *  ---------------------------------------------
 */
/*ARGSUSED*/
static Void  EvaluateTriggers (d)
Display*  d;  /* display pointer */

{
  static time_t  prev_notification = 0;  /* as it says  */
  time_t         now = 0;                /* as it says  */


 /*
  *  Obvious things first.
  *
  *  The triggers are being moved all the time while in disabled
  *  mode in order to make absolutely sure we cannot run into
  *  trouble by an enable message coming in in an odd moment.
  *  Otherwise we possibly might lock or kill too soon.
  */
  if (disabled)
  {
    SetLockTrigger (lock_time);
    if (kill_trigger) SetKillTrigger (kill_time);
  }

 /*
  *  Next, wait for (or kill, if we were so told) the previous 
  *  locker (if any). Note that this must also be done while in
  *  disabled mode. Not only to avoid a potential zombie proces
  *  hanging around until we are re-enabled, but also to prevent
  *  us from incorrectly setting a kill trigger at the moment 
  *  when we are finally re-enabled.
  */
#ifdef VMS
  if (vms_status != 1)  
  {
#else /* VMS */
  if (locker_pid)
  {
#if !defined (UTEKV) && !defined (SYSV) && !defined(SVR4)
    union wait  status;      /* childs process status */
#else /* !UTEKV && !SYSV && !SVR4 */
    int         status = 0;  /* childs process status */
#endif /* !UTEKV && !SYSV && !SVR4 */

    if (unlock_now)
    {
      (Void) kill (locker_pid, SIGTERM);
    }

#if !defined (UTEKV) && !defined (SYSV) && !defined(SVR4)
    if (wait3 (&status, WNOHANG, (struct rusage*) NULL))
#else /* !UTEKV && !SYSV && !SVR4 */
    if (waitpid (-1, &status, WNOHANG)) 
#endif /* !UTEKV && !SYSV && !SVR4 */
    {

     /*
      *  If the locker exited normally, we disable any pending kill
      *  trigger. Otherwise, we assume that it either has crashed or
      *  was not able to lock the display because of an existing
      *  locker (which may have been started manually). In both of
      *  the later cases, disabling the kill trigger would open a
      *  loop hole.
      */
      if (   WIFEXITED (status)
	  && WEXITSTATUS (status) == EXIT_SUCCESS)
      {
        DisableKillTrigger ();
      }

      use_redelay = True;
      locker_pid = 0;
    }
#endif /* VMS */

    SetLockTrigger (lock_time);

   /*
    *  No return here! The pointer may be sitting in a corner, while
    *  parameter settings may be such that we need to start another
    *  locker without further delay. If you think this cannot happen,
    *  consider the case in which the locker simply crashed.
    */
  }

  unlock_now = False;

 /*
  *  Only now is it safe to return if we are in disabled mode.
  */
  if (disabled) return;


 /*
  *  What time is it?
  */
  (Void) time (&now);
  
 /*
  *  Is it time to run the killer command?
  */
  if (kill_trigger && now >= kill_trigger)
  {
   /*
    *  There is a dirty trick here. On the one hand, we don't want
    *  to block until the killer returns, but on the other one
    *  we don't want to have it interfere with the wait () stuff we 
    *  do to keep track of the locker. To obtain both, the killer
    *  command has already been patched by KillerChecker () so that
    *  it gets backgrounded by the shell started by system ().
    *
    *  For the time being, VMS users are out of luck: their xautolock
    *  will indeed block until the killer returns.
    */
    (Void) system (killer);
    SetKillTrigger (kill_time);
  }


 /*
  *  Now trigger the notifier if required. 
  */
  if (   notify_lock
      && now + notify_margin >= lock_trigger
      && prev_notification < now - notify_margin - 1)
  {
    if (notifier_specified)
    {
     /*
      *  Here we use the same dirty trick as for the killer command.
      */
      (Void) system (notifier);
    }
    else
    {
      (Void) XBell (d, bell_percent);
      (Void) XSync (d, 0);
    }
    prev_notification = now;
  }


 /*
  *  Finally fire up the locker if time has somehow come. 
  */
  if (   lock_now
      || now >= lock_trigger)
  {
#ifdef VMS
    if (vms_status == 1)
#else /* VMS */
    if (!locker_pid)
#endif /* VMS */
    {
      switch (locker_pid = vfork ())
      {
        case -1 :
          locker_pid = 0;
          break;
  
        case 0 :
          (Void) close (ConnectionNumber (d));
#ifdef VMS
          vms_status = 0;

	  if (lock_now)
	  {
	    locker_pid = lib$spawn (&now_locker_d, 0, 0, &1, 0, 0,
				    &vms_status);
	  }
	  else
	  {
	    locker_pid = lib$spawn (&locker_d, 0, 0, &1, 0, 0, 
				    &vms_status);
	  }

	  if (!(locker_pid & 1)) exit (locker_pid);

#ifdef SLOW_VMS
          (Void) sleep (SLOW_VMS_DELAY); 
#endif /* SLOW_VMS */
#else /* VMS */
	  if (lock_now)
	  {
            (Void) execl ("/bin/sh", "sh", "-c", now_locker, (String) NULL);
	  }
	  else
	  {
            (Void) execl ("/bin/sh", "sh", "-c", locker, (String) NULL); 
	  }
#endif /* VMS */
          _exit (EXIT_FAILURE);
  
        default :
         /*
          *  In general, xautolock should keep its fingers off the real
          *  screensaver because no universally acceptable policy can 
          *  be defined. In no case should it decide to disable or enable 
          *  it all by itself. Setting the screensaver policy is something
          *  the locker should take care of. After all, xautolock is not
          *  supposed to know what the "locker" does and doesn't do. 
          *  People might be using xautolock for totally different
          *  purposes (which, by the way, is why it will accept a
          *  different set of X resources after being renamed).
          *
          *  Nevertheless, simply resetting the screensaver is a
          *  convenience action that aids many xlock users, and doesn't
          *  harm anyone (*). The problem with older versions of xlock 
	  *  is that they can be told to replace (= disable) the real
	  *  screensaver, but forget to reset that same screensaver if
	  *  it was already active at the time xlock starts. I guess 
	  *  xlock initially wasn't designed to be run without a user
	  *  actually typing the comand ;-).
	  *
	  *  (*) Well, at least it used not to harm anyone, but with the
	  *      advent of DPMS monitors, it now can mess up the power
	  *      saving setup. Hence we better make it optional. 
	  *
	  *      Also, later xlock versions also unconditionally call
	  *      XResetScreenSaver, yielding the same kind of problems
	  *      with DPMS that xautolock did. The latest and greatest
	  *      xlock also has (or will have) a -resetsaver option.
          */
	  if (reset_saver) (Void) XResetScreenSaver(d);
  
          SetLockTrigger (lock_time);
          (Void) XSync (d,0);
      }

     /*
      *  Once the locker is running, all that needs to be done 
      *  is to set the kill_trigger if needed. 
      */
      if (killer_specified) SetKillTrigger (kill_time);

      use_redelay = False;
    }

    lock_now = False;
  }
}




/*
 *  Miscellaneous functions
 *  =======================
 *
 *  X Error handler
 *  ---------------
 */
/*ARGSUSED*/
static Int  CatchFalseAlarm (d, event)
Display*     d;      /* display pointer */
XErrorEvent  event;  /* error event     */

{
  return 0;
}



/*
 *  MES_DISABLE message handler
 *  ---------------------------
 */
static Void  DisableByMessage ()

{
 /*
  *  The order in which things are done is rather important here.
  */
  if (!secure)
  {
    SetLockTrigger (lock_time);
    DisableKillTrigger ();
    disabled = True;
  }
}



/*
 *  MES_ENABLE message handler
 *  --------------------------
 */
static Void  EnableByMessage ()

{
  if (!secure) 
  {
    SetLockTrigger (lock_time);
    if (killer_specified && locker_pid) SetKillTrigger (kill_time);
    disabled = False;
  }
}



/*
 *  MES_TOGGLE message handler
 *  --------------------------
 */
static Void  ToggleByMessage ()

{
  if (!secure)
  {
    SetLockTrigger (lock_time);

    if ((disabled = !disabled)) /* = intended */
    {
      DisableKillTrigger ();
    }
    else
    {
      if (killer_specified && locker_pid) SetKillTrigger (kill_time);
    }
  }
}



/*
 *  MES_EXIT message handler
 *  ------------------------
 */
static Void  ExitByMessage ()

{
  if (!secure)
  {
    Error0 ("Exiting. Bye Bye...\n");
    exit (0);
  }
}



/*
 *  MES_LOCKNOW message handler
 *  ---------------------------
 */
static Void  LockNowByMessage ()

{
  if (!secure && !disabled) 
  {
    lock_now = True;
  }
}



/*
 *  MES_UNLOCKNOW message handler
 *  -----------------------------
 */
static Void  UnlockNowByMessage ()

{
  if (!secure && !disabled) 
  {
    unlock_now = True;
  }
}



/*
 *  Function for creating the communication atoms
 *  ---------------------------------------------
 */
static Void  GetAtoms (d)
Display*  d;  /* display pointer */

{
  String  sem;  /* semaphore property name */
  String  mes;  /* message property name   */
  Char*   ptr;  /* iterator                */

  sem = NewArray (Char, strlen (prog_name) + strlen (SEM_PID) + 1);
  (Void) sprintf (sem, "%s%s", prog_name, SEM_PID);
  for (ptr = sem; *ptr; ++ptr) *ptr = (Char) toupper (*ptr);
  semaphore = XInternAtom (d, sem, False);
  free (sem);

  mes = NewArray (Char, strlen (prog_name) + strlen (MESSAGE) + 1);
  (Void) sprintf (mes, "%s%s", prog_name, MESSAGE);
  for (ptr = mes; *ptr; ++ptr) *ptr = (Char) toupper (*ptr);
  message_atom = XInternAtom (d, mes, False);
  free (mes);
}



/*
 *  Function for looking for messages from another xautolock
 *  --------------------------------------------------------
 */
static Void  LookForMessages (d)
Display*  d;  /* display pointer */

{
  Window  r;          /* root window            */
  Atom    type;       /* actual property type   */
  Int     format;     /* dummy                  */
  Huge    nof_items;  /* dummy                  */
  Huge    after;      /* dummy                  */
  Int*    contents;   /* message property value */


 /*
  *  It would be cleaner (more X-like, using less cpu time and
  *  network bandwith, ...) to implement this functionality by
  *  keeping an eye open for PropertyNotify events on the root
  *  window. Maybe I'll rewrite it one day to do just that, but
  *  the current approach works just fine too.
  *
  *  Note that we must clear the property before acting on it!
  *  Otherwise funny things can happen on receipt of MES_EXIT.
  */
  r = RootWindowOfScreen (ScreenOfDisplay (d, 0));

  (Void) XGetWindowProperty (d, r, message_atom, 0L, 2L,
                             False, AnyPropertyType, &type,
			     &format, &nof_items, &after,
			     (unsigned char**) &contents);

  XDeleteProperty (d, r, message_atom);
  XFlush (d);

  if (type == XA_INTEGER)
  {
    switch (*contents)
    {
      case MES_DISABLE:
       DisableByMessage ();
       break;

      case MES_ENABLE:
       EnableByMessage ();
       break;

      case MES_TOGGLE:
       ToggleByMessage ();
       break;

      case MES_LOCKNOW:
       LockNowByMessage ();
       break;

      case MES_UNLOCKNOW:
       UnlockNowByMessage ();
       break;

      case MES_EXIT:
       ExitByMessage (); 
       break;

      default:
       /* unknown message, ignore silently */
       break;
    }
  }

  (Void) XFree ((Char*) contents);
}



/*
 *  Function for finding out whether another xautolock is already running
 *  ---------------------------------------------------------------------
 */
static Void  CheckConnectionAndSendMessage (d)
Display*  d;  /* display pointer */

{
  pid_t   pid;        /* as it says               */
  Window  r;          /* root window              */
  Atom    type;       /* actual property type     */
  Int     format;     /* dummy                    */
  Huge    nof_items;  /* dummy                    */
  Huge    after;      /* dummy                    */
  pid_t*  contents;   /* semaphore property value */

  r = RootWindowOfScreen (ScreenOfDisplay (d, 0));

  (Void) XGetWindowProperty (d, r, semaphore, 0L, 2L, False,
                             AnyPropertyType, &type, &format,
			     &nof_items, &after,
                             (unsigned char**) &contents);

  if (type == XA_INTEGER)
  {
   /*
    *  This breaks if the other xautolock is not on the same machine.
    */
    if (kill (*contents, 0))
    {
      if (message_to_send)
      {
        Error1 ("No process with PID %d, or the process "
		"is owned bu another user.\n", *contents);
        exit (EXIT_FAILURE);
      }
    }
    else if (message_to_send)
    {
      (Void) XChangeProperty (d, r, message_atom, XA_INTEGER, 
                              8, PropModeReplace, 
			      (unsigned char*) &message_to_send, 
			      (int) sizeof (message_to_send));
      XFlush (d);
      exit (EXIT_SUCCESS);
    }
    else
    {
#ifdef VMS
      Error2 ("%s is already running (PID %x).\n", prog_name, *contents);
#else /* VMS */
      Error2 ("%s is already running (PID %d).\n", prog_name, *contents);
#endif /* VMS */
      exit (EXIT_FAILURE);
    }
  }
  else if (message_to_send)
  {
    Error1 ("Could not locate a running %s.\n", prog_name);
    exit (EXIT_FAILURE);
  }

  pid = getpid ();
  (Void) XChangeProperty (d, r, semaphore, XA_INTEGER, 8, 
                          PropModeReplace, (unsigned char*) &pid,
			  (int) sizeof (pid));

  (Void) XFree ((Char*) contents);
}



#ifdef VMS /* This function is no longer used. */
/*
 *  Function for polling the Session Manager Pause Window on VMS
 *  ------------------------------------------------------------
 */
static Int  PollSmPauseWindow (d)
Display* d;

{
  static Window*  w = (Window*) NULL;  /* PauseWindow window ID     */
  Atom            pause;               /* property we're after      */
  Atom            type;                /* real type of the property */
  Int             format;              /* dummy                     */
  Huge            nof_items;           /* dummy                     */
  Huge            bytes;               /* dummy                     */
  Window          r;                   /* root window               */


 /*
  *  This is not logically correct, because we are ignoring 
  *  the possibility that the machine has multiple displays. 
  */
  r = RootWindowOfScreen (ScreenOfDisplay (d, 0));

  if (w) (Void) XFree (w);

  pause = XInternAtom (d, "_DEC_SM_PAUSE_WINDOW", True);

  return (    (   XGetWindowProperty (d, r, pause, 0, 1, False, XA_WINDOW,
                                      &type, &format, &nof_items, &bytes,
                                      (unsigned char**) &w)
                == Success)
           && type != None);
}
#endif /* VMS */



/*
 *  Function for selecting events on a tree of windows
 *  --------------------------------------------------
 */
static Void  SelectEvents (d, window, substructure_only)
Display*  d;                  /* display pointer   */
Window    window;             /* window            */
Boolean   substructure_only;  /* as it says        */

{
  Window             root;              /* root window of this window */
  Window             parent;            /* parent of this window      */
  Window*            children;          /* children of this window    */
  Unsigned           nof_children = 0;  /* number of children         */
  Unsigned           i;                 /* loop counter               */
  XWindowAttributes  attribs;           /* attributes of the window   */


 /*
  *  Start by querying the server about the root and parent windows.
  */
  if (!XQueryTree (d, window, &root, &parent, &children, &nof_children))
  {
    return;
  }

  if (nof_children) (Void) XFree ((Char*) children);


 /*
  *  Build the appropriate event mask. The basic idea is that we don't
  *  want to interfere with the normal event propagation mechanism if
  *  we don't have to.
  */
  if (substructure_only)
  {
    (Void) XSelectInput (d, window, SubstructureNotifyMask);
  }
  else
  {
    if (parent == None)  /* the *real* rootwindow */
    {
      attribs.all_event_masks = 
      attribs.do_not_propagate_mask = KeyPressMask;
    }
    else if (XGetWindowAttributes (d, window, &attribs) == 0)
    {
      return;
    }

    (Void) XSelectInput (d, window,   SubstructureNotifyMask
                                    | (  (  attribs.all_event_masks
                                          | attribs.do_not_propagate_mask)
                                       & KeyPressMask));
  }


 /*
  *  Now ask for the list of children again, since it might have changed
  *  in between the last time and us selecting SubstructureNotifyMask.
  *
  *  There is a (very small) chance that we might process a subtree twice:
  *  child windows that have been created after our XSelectinput () has
  *  been processed but before we get to the XQueryTree () bit will be in
  *  this situation. This is harmless. It could be avoided by using
  *  XGrabServer (), but that'd be an impolite thing to do, and since it
  *  isn't required...
  */
  if (!XQueryTree (d, window, &root, &parent, &children, &nof_children))
  {
    return;
  }


 /*
  *  Now do the same thing for all children.
  */
  for (i = 0; i < nof_children; ++i)
  {
    SelectEvents (d, children[i], substructure_only);
  }

  if (nof_children) (Void) XFree ((Char*) children);
}



/*
 *  Main function
 *  -------------
 */
Int  Main (argc, argv)
Int     argc;    /* number of arguments */
String  argv[];  /* array of arguments  */

{
  Display*  d;                  /* display pointer   */
  Window    r;                  /* root window       */
  Int       s;                  /* screen index      */
  Queue     queue = 0;          /* as it says        */
  Char*     ptr;                /* temporary storage */
  Boolean   use_xidle = False;  /* as it says        */
  Boolean   use_mit = False;    /* as it says        */


 /*
  *  Beautify argv[0] and remember it for later use.
  */
#ifdef VMS
  if (ptr = strrchr (argv[0], ']'))
  {
    prog_name = ptr + 1;
  }
  else
  {
    prog_name = argv[0];
  }

  if ((ptr = strchr (prog_name, '.'))) /* = intended */
  {
    *ptr = '\0';
  }
#else /* VMS */
  if ((ptr = strrchr (argv[0], '/'))) /* = intended */
  {
    prog_name = ptr + 1;
  }
  else
  {
    prog_name = argv[0];
  }
#endif /* VMS */


 /*
  *  Find out whether there actually is a server on the other side...
  */
  if (   (d = XOpenDisplay ((String) NULL))
      == (Display*) NULL)
  {
    Error1 ("Couldn't connect to %s\n", XDisplayName ((String) NULL));
    exit (EXIT_FAILURE);
  }
  
  
 /*
  *  Get ourselves a dummy window in order to allow display and/or
  *  session managers etc. to use XKillClient() on us (e.g. xdm when
  *  not using XDMCP).
  * 
  *  I'm not sure whether the window needs to be mapped for xdm, but
  *  the default setup Sun uses for OpenWindows and olwm definitely
  *  requires it to be mapped.
  *
  *  If we're doing all this anyway, we might as well set the correct
  *  WM properties on the window as a convenience.
  */
  {
    Window                our_win;     /* as it says */
    XTextProperty         name_prop;   /* as it says */
    XClassHint*           class_info;  /* as it says */
    XSetWindowAttributes  attribs;     /* as it says */

    attribs.override_redirect = True;
    our_win = XCreateWindow (d, DefaultRootWindow (d), -1, -1, 1, 1, 0,
		             CopyFromParent, InputOnly, CopyFromParent,
			     CWOverrideRedirect, &attribs);

    class_info = XAllocClassHint ();
    class_info->res_class = APPLIC_CLASS;
    (Void) XStringListToTextProperty (argv, 1, &name_prop);
    XSetWMProperties (d, our_win, &name_prop, (XTextProperty*) NULL,
		      argv, argc, (XSizeHints*) NULL, (XWMHints*) NULL,
		      class_info);
    (Void) XFree (name_prop.value);
    (Void) XFree (class_info);

    (Void) XMapWindow (d, our_win);
  }


 /*
  *  Some initializations.
  */
  ProcessOpts (d, argc, argv);

  GetAtoms (d);

  CheckConnectionAndSendMessage (d);

  if (close_out) (Void) fclose (stdout);
  if (close_err) (Void) fclose (stderr);

  (Void) XSetErrorHandler ((XErrorHandler) CatchFalseAlarm);

#ifdef HasXidle
  {
    Int dummy;  /* as it says */
    use_xidle = XidleQueryExtension (d, &dummy, &dummy);
  }
#endif /* HasXidle */

#ifdef HasScreenSaver
  if (!use_xidle)
  {
    Int dummy;  /* as it says */
    use_mit = XScreenSaverQueryExtension (d, &dummy, &dummy);
  }
#endif /* HasScreenSaver */

  (Void) XSync (d, 0);

  if (!use_xidle && !use_mit)
  {
    queue = NewQueue ();
  
    for (s = -1; ++s < ScreenCount (d); )
    {
      AddToQueue (queue, r = RootWindowOfScreen (ScreenOfDisplay (d, s)));
      SelectEvents (d, r, True);
    }
  }

  SetLockTrigger (lock_time);


 /*
  *  Main event loop.
  */
  forever
  {
    LookForMessages (d);

    if (use_xidle || use_mit)
    {
      QueryIdleTime (d, use_xidle);
    }
    else
    {
      ProcessEvents (d, queue);
    }

    QueryPointer (d);  /* Overkill if Xidle or MIT screensaver
                          is being used, but it works.        */
    EvaluateTriggers (d);

   /*
    *  It seems that on some operating systems (VMS to name just one)
    *  sleep () can be vastly inaccurate: sometimes 60 calls to sleep (1)
    *  add up to only 30 seconds or even less of sleeping. Therefore,
    *  as of patchlevel 9 we no longer rely on it for keeping track of
    *  time. The only reason why we still call it, is to make xautolock
    *  (which after all uses a busy-form-of-waiting algorithm), less
    *  processor hungry.
    */
    (Void) sleep (1);
  }

#ifdef lint
  /*NOTREACHED*/
  return 0;
#endif /* lint */
}

