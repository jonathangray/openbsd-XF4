! $Xorg: Xresources,v 1.3 2000/08/17 19:54:17 cpqbld Exp $
! $OpenBSD: Xres.cpp,v 1.9 2002/09/19 20:06:51 matthieu Exp $
!
!
!
!
! $XFree86: xc/programs/xdm/config/Xres.cpp,v 1.4 2001/01/22 22:32:34 dawes Exp $
!
#define BS \ /* cpp can be trickier than m4 */
#define NLBS \n\ /* don't remove these comments */
xlogin*login.translations: #override BS
	Ctrl<Key>R: abort-display()NLBS
	<Key>F1: set-session-argument(failsafe) finish-field()NLBS
	<Key>Left: move-backward-character()NLBS
	<Key>Right: move-forward-character()NLBS
	<Key>Home: move-to-begining()NLBS
	<Key>End: move-to-end()NLBS\
	Ctrl<Key>KP_Enter: set-session-argument(failsafe) finish-field()NLBS
	<Key>KP_Enter: set-session-argument() finish-field()NLBS
	Ctrl<Key>Return: set-session-argument(failsafe) finish-field()NLBS
	<Key>Return: set-session-argument() finish-field()
#ifndef XPM
xlogin*greeting: CLIENTHOST
xlogin*namePrompt: login:\040
#else
xlogin*greeting: Welcome to CLIENTHOST
xlogin*namePrompt: \040\040\040\040\040\040\040Login:
#endif /* XPM */
xlogin*fail: Login incorrect
#ifdef XPM
/**/#if WIDTH > 800
xlogin*greetFont: -adobe-helvetica-bold-o-normal--24-240-75-75-p-138-iso8859-1
xlogin*font: -adobe-helvetica-medium-r-normal--18-180-75-75-p-103-iso8859-1
xlogin*promptFont: -adobe-helvetica-bold-r-normal--18-180-75-75-p-103-iso8859-1
xlogin*failFont: -adobe-helvetica-bold-r-normal--18-180-75-75-p-103-iso8859-1
/**/#else
xlogin*greetFont: -adobe-helvetica-bold-o-normal--17-120-100-100-p-92-iso8859-1
xlogin*font: -adobe-helvetica-medium-r-normal--12-120-75-75-p-69-iso8859-1
xlogin*promptFont: -adobe-helvetica-bold-r-normal--12-120-75-75-p-69-iso8859-1
xlogin*failFont: -adobe-helvetica-bold-o-normal--14-140-75-75-p-82-iso8859-1
/**/#endif
#endif /* XPM */

/**/#if !(defined(bpp1) || defined(bpp4) || defined(bpp8) || defined(bpp15))
/**/# if PLANES < 4 || defined(Hp300Architecture)
/**/#  ifndef bpp1
/**/#   define bpp1
/**/#  endif
/**/# else
/**/#  if PLANES > 4
/**/#   if PLANES > 8
/**/#    ifndef bpp15
/**/#     define bpp15
/**/#    endif
/**/#   else
/**/#    ifndef bpp8
/**/#     define bpp8
/**/#    endif bpp8
/**/#   endif
/**/#  else
/**/#   ifndef bpp4
/**/#    define bpp4
/**/#   endif
/**/#  endif
/**/# endif
/**/#endif  //**/* If manual override */**//

/**/#ifndef bpp1
#ifndef XPM
xlogin*greetColor: CadetBlue
#else
xlogin*borderWidth: 2
xlogin*frameWidth: 5
xlogin*innerFramesWidth: 2

! top/left border
xlogin*hiColor: #6d9df2

! bottom/right border
xlogin*shdColor: #384c70

! 'Welcome to..' text color
xlogin*greetColor: #000000
#endif /* XPM */
/**/#if defined(bpp4) || defined(bpp8) || defined(bpp15)
! flood fill
!xlogin*background: #2559a5
xlogin*background: #5171b5
/**/#endif
xlogin*failColor: red

! 'Login:' and 'Password:'
*Foreground: #fbfeff

! border/shadow
*Background: #000000
/**/#else
#ifdef XPM
xlogin*borderWidth: 3
xlogin*frameWidth: 5
xlogin*innerFramesWidth: 1
xlogin*shdColor: white
xlogin*hiColor: white
xlogin*greetColor: white
xlogin*background: black
xlogin*failColor: white
xlogin*promptColor: white
*Foreground: white
*Background: black
#else
xlogin*borderWidth: 3
xlogin*Foreground: black
xlogin*Background: white
#endif /* XPM */
/**/#endif
#if defined(XPM)
/**/#ifdef bpp1
xlogin*logoFileName: BITMAPDIR/**//OpenBSD_1bpp.xpm
/**/#endif
/**/#ifdef bpp4
xlogin*logoFileName: BITMAPDIR/**//OpenBSD_4bpp.xpm
/**/#endif
/**/#ifdef bpp8
xlogin*logoFileName: BITMAPDIR/**//OpenBSD_8bpp.xpm
/**/#endif
/**/#ifdef bpp15
xlogin*logoFileName: BITMAPDIR/**//OpenBSD_15bpp.xpm
/**/#endif
#if ! defined(Hp300Architecture)
xlogin*useShape: true
xlogin*logoPadding: 10
#endif /* Hp300Architecture */
#endif /* XPM */
! comment out to disable root logins
xlogin.Login.allowRootLogin:	true

XConsole.text.geometry:	480x130
XConsole.verbose:	true
XConsole*iconic:	true
#ifdef XPM
XConsole*background:	black
XConsole*foreground:	white
XConsole*borderWidth:	2
XConsole*borderColor:   grey
#endif /* XPM */
XConsole*font:		fixed

Chooser*geometry:		700x500+300+200
Chooser*allowShellResize:	false
Chooser*viewport.forceBars:	true
Chooser*label.font:		*-new century schoolbook-bold-i-normal-*-240-*
Chooser*label.label:		XDMCP Host Menu from CLIENTHOST
Chooser*list.font:		-*-*-medium-r-normal-*-*-230-*-*-c-*-iso8859-1
Chooser*Command.font:		*-new century schoolbook-bold-r-normal-*-180-*
