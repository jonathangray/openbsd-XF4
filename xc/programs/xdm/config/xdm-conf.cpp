! $Xorg: xdm-conf.cpp,v 1.3 2000/08/17 19:54:17 cpqbld Exp $
! $OpenBSD: xdm-conf.cpp,v 1.11 2005/05/27 22:30:20 matthieu Exp $
!
!
!
!
! $XFree86: xc/programs/xdm/config/xdm-conf.cpp,v 1.10 2002/11/30 19:11:32 herrb Exp $
!
DisplayManager.errorLogFile:	XDMLOGDIR/xdm.log
DisplayManager.pidFile:		XDMPIDDIR/xdm.pid
DisplayManager.keyFile:		XDMDIR/xdm-keys
DisplayManager.servers:		XDMDIR/Xservers
DisplayManager.accessFile:	XDMDIR/Xaccess
DisplayManager.willing:		SU nobody -c XDMDIR/Xwilling
! All displays should use authorization, but we cannot be sure
! X terminals may not be configured that way, so they will require
! individual resource settings.
DisplayManager*authorize:	true
! The following three resources set up display :0 as the console.
DisplayManager._0.setup:	XDMDIR/Xsetup_0
DisplayManager._0.startup:	XDMDIR/Startup_0
DisplayManager._0.reset:	XDMDIR/TakeConsole
!
DisplayManager*chooser:		CHOOSERPATH
DisplayManager*resources:	XDMDIR/Xresources
DisplayManager*session:		XDMDIR/Xsession
DisplayManager*authComplain:	true
#if HAS_DES_AUTH == YES
DisplayManager._0.authName:     MIT-MAGIC-COOKIE-1
#endif
#ifdef XPM
DisplayManager*loginmoveInterval:	10
#endif /* XPM */
!
DisplayManager*xrdb: XRDB
!
! SECURITY: do not listen for XDMCP or Chooser requests
! Comment out this line if you want to manage X terminals with xdm
DisplayManager.requestPort:	0
