/* $XConsortium: xf86Xinput.c /main/14 1996/10/27 11:05:25 kaleb $ */
/*
 * Copyright 1995-1999 by Frederic Lepied, France. <Lepied@XFree86.org>
 *                                                                            
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  Frederic   Lepied not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     Frederic  Lepied   makes  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.                   
 *                                                                            
 * FREDERIC  LEPIED DISCLAIMS ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL FREDERIC  LEPIED BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Xinput.c,v 3.22.2.16 1999/10/21 12:07:50 hohndel Exp $ */

#include "Xmd.h"
#include "XI.h"
#include "XIproto.h"
#include "xf86.h"
#include "Xpoll.h"
#include "xf86Priv.h"
#include "xf86_Config.h"
#include "xf86Xinput.h"
#include "xf86Procs.h"
#include "mipointer.h"

#ifdef DPMSExtension
#include "extensions/dpms.h"
#endif

#ifdef XFreeXDGA
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#include "exevents.h"	/* AddInputDevice */

#include "extnsionst.h"
#include "extinit.h"	/* LookupDeviceIntRec */

#include "windowstr.h"	/* screenIsSaved */

#include <stdarg.h>
#include <math.h>

/******************************************************************************
 * debugging macro
 *****************************************************************************/
#ifdef DBG
#undef DBG
#endif
#ifdef DEBUG
#undef DEBUG
#endif

static int      debug_level = 0;
#define DEBUG 1
#if DEBUG
#define DBG(lvl, f) {if ((lvl) <= debug_level) f;}
#else
#define DBG(lvl, f)
#endif

extern InputInfo inputInfo;

extern	int	DeviceKeyPress;
extern	int	DeviceKeyRelease;
extern	int	DeviceButtonPress;
extern	int	DeviceButtonRelease;
extern	int	DeviceMotionNotify;
extern	int	DeviceValuator;
extern	int	ProximityIn;
extern	int	ProximityOut;

#if !defined(DYNAMIC_MODULE)
#ifdef JOYSTICK_SUPPORT
extern DeviceAssocRec   joystick_assoc;
#endif
#ifdef WACOM_SUPPORT
extern DeviceAssocRec   wacom_stylus_assoc;
extern DeviceAssocRec   wacom_cursor_assoc;
extern DeviceAssocRec   wacom_eraser_assoc;
#endif
#ifdef ELOGRAPHICS_SUPPORT
extern DeviceAssocRec	elographics_assoc;
#endif
#ifdef MICROTOUCH_SUPPORT
extern DeviceAssocRec	MuT_finger_assoc;
extern DeviceAssocRec	MuT_stylus_assoc;
#endif
#ifdef SUMMASKETCH_SUPPORT
extern DeviceAssocRec summasketch_assoc;
#endif
#ifdef ACECAD_SUPPORT
extern DeviceAssocRec acecad_assoc;
#endif
#ifdef DIALBOX_SUPPORT
extern DeviceAssocRec dial_assoc;
#endif
#endif

extern DeviceAssocRec	mouse_assoc;
extern DeviceAssocRec	switch_assoc;

static int              num_devices;
static LocalDevicePtr	*localDevices;
static int              max_devices;

static LocalDevicePtr	switch_device;

static DeviceAssocPtr   *deviceAssoc = NULL;
static int		numAssoc = 0;
static int		maxAssoc = 0;

#define DEBUG_LEVEL	10010

static SymTabRec XinputTab[] = {
  { ENDSECTION,		"endsection"},
  { SUBSECTION,		"subsection" },
  { DEBUG_LEVEL,	"debuglevel" },
  { -1,			"" },
};

/***********************************************************************
 *
 * xf86AlwaysCoreControl --
 *	
 *	Control proc for the integer feedback that controls the always
 * core feature.
 *
 ***********************************************************************
 */
static void
xf86AlwaysCoreControl(DeviceIntPtr	device,
		      IntegerCtrl	*control)
{
}

/***********************************************************************
 *
 * Core devices functions --
 *	
 *	Test if device is the core device by checking the
 * value of always core feedback and the inputInfo struct.
 *
 ***********************************************************************
 */
int
xf86IsCorePointer(DeviceIntPtr	device)
{
    return(device == inputInfo.pointer);
}

static int
xf86ShareCorePointer(DeviceIntPtr	device)
{
    LocalDevicePtr	local = (LocalDevicePtr) device->public.devicePrivate;
    
    return((local->always_core_feedback &&
	    local->always_core_feedback->ctrl.integer_displayed));
}

int
xf86IsCoreKeyboard(DeviceIntPtr	device)
{
    LocalDevicePtr	local = (LocalDevicePtr) device->public.devicePrivate;
    
    return((local->flags & XI86_ALWAYS_CORE) ||
	   (device == inputInfo.keyboard));
}

void
xf86AlwaysCore(LocalDevicePtr	local,
	       Bool		always)
{
    if (always) {
	local->flags |= XI86_ALWAYS_CORE;
    } else {
	local->flags &= ~XI86_ALWAYS_CORE;
    }
}

/***********************************************************************
 *
 * xf86CheckButton --
 *	
 *	Test if the core pointer button state is coherent with
 * the button event to send.
 *
 ***********************************************************************
 */
Bool
xf86CheckButton(int	button,
		int	down)
{
    int	state = (inputInfo.pointer->button->state & 0x1f00) >> 8;
    int	check = (state & (1 << (button - 1)));
    
    if ((check && down) && (!check && !down)) {
	return FALSE;
    }

    return TRUE;
}

/***********************************************************************
 *
 * ReadInput --
 *	Wakeup handler to catch input and dispatch it to our
 *	input routines if necessary.
 *
 ***********************************************************************
 */
static void
ReadInput(pointer	block_data,
	  int		select_status,
	  pointer	read_mask)
{
  int			i;
  LocalDevicePtr	local_dev;
  fd_set*		LastSelectMask = (fd_set*) read_mask;
  fd_set		devices_with_input;
  extern fd_set		EnabledDevices;

  if (select_status < 1)
    return;

  XFD_ANDSET(&devices_with_input, LastSelectMask, &EnabledDevices);
  if (!XFD_ANYSET(&devices_with_input))
    return;

  for (i = 0; i < num_devices; i++) {
    local_dev = localDevices[i];
    if (local_dev->read_input &&
	(local_dev->fd >= 0) &&
        (FD_ISSET(local_dev->fd, ((fd_set *) read_mask)) != 0)) {
      (*local_dev->read_input)(local_dev);
      break;
    }
  }
}

/***********************************************************************
 *
 * configExtendedInputSection --
 *
 ***********************************************************************
 */

void
xf86ConfigExtendedInputSection(LexPtr       val)
{
  int           i;
  int           token;

#if !defined(DYNAMIC_MODULE)
# ifdef JOYSTICK_SUPPORT
  xf86AddDeviceAssoc(&joystick_assoc);
# endif
# ifdef WACOM_SUPPORT
  xf86AddDeviceAssoc(&wacom_stylus_assoc);
  xf86AddDeviceAssoc(&wacom_cursor_assoc);
  xf86AddDeviceAssoc(&wacom_eraser_assoc);
# endif
# ifdef MICROTOUCH_SUPPORT
  xf86AddDeviceAssoc(&MuT_finger_assoc);
  xf86AddDeviceAssoc(&MuT_stylus_assoc);
# endif
# ifdef ELOGRAPHICS_SUPPORT
  xf86AddDeviceAssoc(&elographics_assoc);
# endif
# ifdef SUMMASKETCH_SUPPORT
  xf86AddDeviceAssoc(&summasketch_assoc);
# endif
# ifdef ACECAD_SUPPORT
  xf86AddDeviceAssoc(&acecad_assoc);
# endif
# ifdef DIALBOX_SUPPORT
  xf86AddDeviceAssoc(&dial_assoc);
# endif
#endif

  xf86AddDeviceAssoc(&mouse_assoc);
  
  num_devices = 0;
  max_devices = 3;
  localDevices = (LocalDevicePtr*) xalloc(sizeof(LocalDevicePtr)*max_devices);

  /* Create and initialize the special device Switch */
  localDevices[0] = switch_assoc.device_allocate();
  switch_device = localDevices[0];
  switch_device->flags |= XI86_CONFIGURED; /* no configuration available */
  num_devices++;

  while ((token = xf86GetToken(XinputTab)) != ENDSECTION)
    {
      if (token == SUBSECTION)
        {
          int   found = 0;
          
          if (xf86GetToken(NULL) != STRING)
            xf86ConfigError("SubSection name expected");
          
          for(i=0; !found && i<numAssoc; i++)
            {
              if (StrCaseCmp(val->str, deviceAssoc[i]->config_section_name) == 0)
                {
                  if (num_devices == max_devices) {
                    max_devices *= 2;
                    localDevices = (LocalDevicePtr*) xrealloc(localDevices,
                                                              sizeof(LocalDevicePtr)*max_devices);
                  }
                  localDevices[num_devices] = deviceAssoc[i]->device_allocate();
                  
                  if (localDevices[num_devices] && localDevices[num_devices]->device_config) 
                    {
                      (*localDevices[num_devices]->device_config)(localDevices,
                                                                  num_devices,
                                                                  num_devices+1,
                                                                  val);
                      localDevices[num_devices]->flags |= XI86_CONFIGURED;
                      num_devices++;
                    }
                  found = 1;
                }
            }
          if (!found)
            xf86ConfigError("Invalid SubSection name");
        }
      else if (token == DEBUG_LEVEL) {
	  if (xf86GetToken(NULL) != NUMBER)
	      xf86ConfigError("Option number expected");
	  debug_level = val->num;
	  if (xf86Verbose) {
#if DEBUG
	      ErrorF("%s XInput debug level sets to %d\n", XCONFIG_GIVEN,
		     debug_level);      
#else
	      ErrorF("%s XInput debug level not sets to %d because"
		     " debugging is not compiled\n", XCONFIG_GIVEN,
		     debug_level);      
#endif
	  }
      }
      else
	  xf86ConfigError("XInput keyword section expected");        
    }
}

/***********************************************************************
 *
 * xf86AddDeviceAssoc --
 * 
 *	Add an association to the array deviceAssoc. This is needed to
 * allow dynamic loading of devices to register themself.
 *
 ***********************************************************************
 */
void
xf86AddDeviceAssoc(DeviceAssocPtr	assoc)
{
    if (!deviceAssoc) {
	maxAssoc = 10;
	deviceAssoc = (DeviceAssocPtr*) xalloc(sizeof(DeviceAssocPtr)*maxAssoc);
    } else {
	if (maxAssoc == numAssoc) {
	    maxAssoc *= 2;
	    deviceAssoc = (DeviceAssocPtr*) xrealloc(deviceAssoc, sizeof(DeviceAssocPtr)*maxAssoc);
	}
    }
    deviceAssoc[numAssoc] = assoc;
    numAssoc++;
}

/***********************************************************************
 *
 * xf86XinputFinalizeInit --
 * 
 *	Create and initialize an integer feedback to control the always
 * core feature.
 *
 ***********************************************************************
 */
void
xf86XinputFinalizeInit(DeviceIntPtr	dev)
{
    LocalDevicePtr        local = (LocalDevicePtr)dev->public.devicePrivate;

    local->dxremaind = 0.0;
    local->dyremaind = 0.0;
    
    if (InitIntegerFeedbackClassDeviceStruct(dev, xf86AlwaysCoreControl) == FALSE) {
	ErrorF("Unable to init integer feedback for always core feature\n");
    } else {
	local->always_core_feedback = dev->intfeed;
	dev->intfeed->ctrl.integer_displayed = (local->flags & XI86_ALWAYS_CORE) ? 1 : 0;
    }
}

/***********************************************************************
 *
 * xf86EnableInputDevices --
 * 
 *	Enable input devices after a switch back from the console.
 *
 ***********************************************************************
 */
void
xf86EnableInputDevices()
{
  int			i;

  for (i = 0; i < num_devices; i++) {
    if (localDevices[i]->dev) {
      EnableDevice(localDevices[i]->dev);
    }
  }
}

/***********************************************************************
 *
 * xf86DisableInputDevices --
 * 
 *	Disable input devices after a switch to the console.
 *
 ***********************************************************************
 */
void
xf86DisableInputDevices()
{
  int			i;

  for (i = 0; i < num_devices; i++) {
    if (localDevices[i]->dev) {
      DisableDevice(localDevices[i]->dev);
    }
  }
}

/***********************************************************************
 *
 * InitExtInput --
 * 
 *	Initialize any extended devices we might have. It is called from
 * ddx InitInput.
 *
 ***********************************************************************
 */

void
InitExtInput()
{
    DeviceIntPtr	dev;
    int		i;

    /* Register a Wakeup handler to handle input when generated */
    RegisterBlockAndWakeupHandlers((BlockHandlerProcPtr) NoopDDA, ReadInput,
				   NULL);

    /* Add each device */
    for (i = 0; i < num_devices; i++) {
	if (localDevices[i]->flags & XI86_CONFIGURED) {
	    int	open_on_init;

	    open_on_init = !(localDevices[i]->flags & XI86_NO_OPEN_ON_INIT) ||
		(localDevices[i]->flags & XI86_ALWAYS_CORE);
	    
	    dev = AddInputDevice(localDevices[i]->device_control,
				 open_on_init);
	    if (dev == NULL)
		FatalError("Too many input devices");
	    
	    localDevices[i]->atom = MakeAtom(localDevices[i]->name,
					     strlen(localDevices[i]->name),
					     TRUE);
	    dev->public.devicePrivate = (pointer) localDevices[i];
	    localDevices[i]->dev = dev;      

	    xf86XinputFinalizeInit(dev);
      
	    RegisterOtherDevice(dev);
	    if (serverGeneration == 1) 
		ErrorF("%s Adding extended device \"%s\" (type: %s)\n", XCONFIG_GIVEN,
		       localDevices[i]->name, localDevices[i]->type_name);
	}
    }
}


/***********************************************************************
 *
 * Caller:	ProcXOpenDevice
 *
 * This is the implementation-dependent routine to open an input device.
 * Some implementations open all input devices when the server is first
 * initialized, and never close them.  Other implementations open only
 * the X pointer and keyboard devices during server initialization,
 * and only open other input devices when some client makes an
 * XOpenDevice request.  This entry point is for the latter type of 
 * implementation.
 *
 * If the physical device is not already open, do it here.  In this case,
 * you need to keep track of the fact that one or more clients has the
 * device open, and physically close it when the last client that has
 * it open does an XCloseDevice.
 *
 * The default implementation is to do nothing (assume all input devices
 * are opened during X server initialization and kept open).
 *
 ***********************************************************************
 */

void
OpenInputDevice (dev, client, status)
    DeviceIntPtr dev;
    ClientPtr client;
    int *status;
{
    extern int	BadDevice;
    
    if (!dev->inited) {
	*status = BadDevice;
    } else {
	if (!dev->public.on) {
	    if (!EnableDevice(dev)) {
		*status = BadDevice;
	    } else {
		/* to prevent ProcXOpenDevice to call EnableDevice again */
		dev->startup = FALSE;
	    }
	}
    }
}


/***********************************************************************
 *
 * Caller:	ProcXChangeKeyboardDevice
 *
 * This procedure does the implementation-dependent portion of the work
 * needed to change the keyboard device.
 *
 * The X keyboard device has a FocusRec.  If the device that has been 
 * made into the new X keyboard did not have a FocusRec, 
 * ProcXChangeKeyboardDevice will allocate one for it.
 *
 * If you do not want clients to be able to focus the old X keyboard
 * device, call DeleteFocusClassDeviceStruct to free the FocusRec.
 *
 * If you support input devices with keys that you do not want to be 
 * used as the X keyboard, you need to check for them here and return 
 * a BadDevice error.
 *
 * The default implementation is to do nothing (assume you do want
 * clients to be able to focus the old X keyboard).  The commented-out
 * sample code shows what you might do if you don't want the default.
 *
 ***********************************************************************
 */

int
ChangeKeyboardDevice (old_dev, new_dev)
     DeviceIntPtr	old_dev;
     DeviceIntPtr	new_dev;
{
  /**********************************************************************
   * DeleteFocusClassDeviceStruct(old_dev);	 * defined in xchgptr.c *
   **********************************************************************/
  return !Success;
}


/***********************************************************************
 *
 * Caller:	ProcXChangePointerDevice
 *
 * This procedure does the implementation-dependent portion of the work
 * needed to change the pointer device.
 *
 * The X pointer device does not have a FocusRec.  If the device that
 * has been made into the new X pointer had a FocusRec, 
 * ProcXChangePointerDevice will free it.
 *
 * If you want clients to be able to focus the old pointer device that
 * has now become accessible through the input extension, you need to 
 * add a FocusRec to it here.
 *
 * The XChangePointerDevice protocol request also allows the client
 * to choose which axes of the new pointer device are used to move 
 * the X cursor in the X- and Y- directions.  If the axes are different
 * than the default ones, you need to keep track of that here.
 *
 * If you support input devices with valuators that you do not want to be 
 * used as the X pointer, you need to check for them here and return a 
 * BadDevice error.
 *
 * The default implementation is to do nothing (assume you don't want
 * clients to be able to focus the old X pointer).  The commented-out
 * sample code shows what you might do if you don't want the default.
 *
 ***********************************************************************
 */

int
#ifdef NeedFunctionPrototypes
ChangePointerDevice (
     DeviceIntPtr	old_dev,
     DeviceIntPtr	new_dev,
     unsigned char	x,
     unsigned char	y)
#else
ChangePointerDevice (old_dev, new_dev, x, y)
     DeviceIntPtr	old_dev, new_dev;
     unsigned char	x, y;
#endif /* NeedFunctionPrototypes */
{
  /************************************************************************
    InitFocusClassDeviceStruct(old_dev);	* allow focusing old ptr*
    
    x_axis = x;					* keep track of new x-axis*
    y_axis = y;					* keep track of new y-axis*
    if (x_axis != 0 || y_axis != 1)
    axes_changed = TRUE;			* remember axes have changed*
    else
    axes_changed = FALSE;
   *************************************************************************/

  /* Return failure if we try with the Switch device */
  if (new_dev == switch_device->dev) {
    return !Success;
  }
  
  /*
   * We don't allow axis swap or other exotic features.
   */
  if (x == 0 && y == 1) {
      LocalDevicePtr	old_local = (LocalDevicePtr)old_dev->public.devicePrivate;
      LocalDevicePtr	new_local = (LocalDevicePtr)new_dev->public.devicePrivate;
      
      InitFocusClassDeviceStruct(old_dev);
    
      /* Restore Extended motion history information */
      old_dev->valuator->GetMotionProc   = old_local->motion_history_proc;
      old_dev->valuator->numMotionEvents = old_local->history_size;

      /* Save Extended motion history information */
      new_local->motion_history_proc = new_dev->valuator->GetMotionProc;
      new_local->history_size	     = new_dev->valuator->numMotionEvents;
      
      /* Set Core motion history information */
      new_dev->valuator->GetMotionProc   = miPointerGetMotionEvents;
      new_dev->valuator->numMotionEvents = miPointerGetMotionBufferSize();
      
    return Success;
  }
  else
    return !Success;
}


/***********************************************************************
 *
 * Caller:	ProcXCloseDevice
 *
 * Take care of implementation-dependent details of closing a device.
 * Some implementations may actually close the device, others may just
 * remove this clients interest in that device.
 *
 * The default implementation is to do nothing (assume all input devices
 * are initialized during X server initialization and kept open).
 *
 ***********************************************************************
 */

void
CloseInputDevice (d, client)
     DeviceIntPtr d;
     ClientPtr client;
{
  ErrorF("ProcXCloseDevice to close or not ?\n");
}


/***********************************************************************
 *
 * Caller:	ProcXListInputDevices
 *
 * This is the implementation-dependent routine to initialize an input 
 * device to the point that information about it can be listed.
 * Some implementations open all input devices when the server is first
 * initialized, and never close them.  Other implementations open only
 * the X pointer and keyboard devices during server initialization,
 * and only open other input devices when some client makes an
 * XOpenDevice request.  If some other process has the device open, the
 * server may not be able to get information about the device to list it.
 *
 * This procedure should be used by implementations that do not initialize
 * all input devices at server startup.  It should do device-dependent
 * initialization for any devices not previously initialized, and call
 * AddInputDevice for each of those devices so that a DeviceIntRec will be 
 * created for them.
 *
 * The default implementation is to do nothing (assume all input devices
 * are initialized during X server initialization and kept open).
 * The commented-out sample code shows what you might do if you don't want 
 * the default.
 *
 ***********************************************************************
 */

void
AddOtherInputDevices ()
{
}


/****************************************************************************
 *
 * Caller:	ProcXSetDeviceMode
 *
 * Change the mode of an extension device.
 * This function is used to change the mode of a device from reporting
 * relative motion to reporting absolute positional information, and
 * vice versa.
 * The default implementation below is that no such devices are supported.
 *
 ***********************************************************************
 */

int
SetDeviceMode (client, dev, mode)
     register	ClientPtr	client;
     DeviceIntPtr dev;
     int		mode;
{
  LocalDevicePtr        local = (LocalDevicePtr)dev->public.devicePrivate;

  if (local->switch_mode) {
    return (*local->switch_mode)(client, dev, mode);
  }
  else
    return BadMatch;
}


/***********************************************************************
 *
 * Caller:	ProcXSetDeviceValuators
 *
 * Set the value of valuators on an extension input device.
 * This function is used to set the initial value of valuators on
 * those input devices that are capable of reporting either relative
 * motion or an absolute position, and allow an initial position to be set.
 * The default implementation below is that no such devices are supported.
 *
 ***********************************************************************
 */

int
SetDeviceValuators (client, dev, valuators, first_valuator, num_valuators)
     register ClientPtr	client;
     DeviceIntPtr 	dev;
     int		*valuators;
     int		first_valuator;
     int		num_valuators;
{
  return BadMatch;
}


/***********************************************************************
 *
 * Caller:	ProcXChangeDeviceControl
 *
 * Change the specified device controls on an extension input device.
 *
 ***********************************************************************
 */

int
ChangeDeviceControl (client, dev, control)
     register ClientPtr	client;
     DeviceIntPtr	dev;
     xDeviceCtl		*control;
{
  LocalDevicePtr        local = (LocalDevicePtr)dev->public.devicePrivate;

  if (!local->control_proc) {
      return (BadMatch);
  }
  else {
      return (*local->control_proc)(local, control);
  }
}

/*
 * adapted from mieq.c to support extended events
 *
 */
extern	InputInfo 	inputInfo;

#define QUEUE_SIZE  256

typedef struct _Event {
    xEvent	event;
#ifdef XINPUT
  deviceValuator val;
#endif
    ScreenPtr	pScreen;
} EventRec, *EventPtr;

typedef struct _EventQueue {
    HWEventQueueType head, tail;
    CARD32	lastEventTime;	    /* to avoid time running backwards */
    Bool	lastMotion;
    EventRec	events[QUEUE_SIZE]; /* static allocation for signals */
    DevicePtr	pKbd, pPtr;	    /* device pointer, to get funcs */
    ScreenPtr	pEnqueueScreen;	    /* screen events are being delivered to */
    ScreenPtr	pDequeueScreen;	    /* screen events are being dispatched to */
} EventQueueRec, *EventQueuePtr;

static EventQueueRec xf86EventQueue;

Bool
xf86eqInit (pKbd, pPtr)
    DevicePtr	pKbd, pPtr;
{
    xf86EventQueue.head = xf86EventQueue.tail = 0;
    xf86EventQueue.lastEventTime = GetTimeInMillis ();
    xf86EventQueue.pKbd = pKbd;
    xf86EventQueue.pPtr = pPtr;
    xf86EventQueue.lastMotion = FALSE;
    xf86EventQueue.pEnqueueScreen = screenInfo.screens[0];
    xf86EventQueue.pDequeueScreen = xf86EventQueue.pEnqueueScreen;
    SetInputCheck (&xf86EventQueue.head, &xf86EventQueue.tail);
    return TRUE;
}

/*
 * Must be reentrant with ProcessInputEvents.  Assumption: xf86eqEnqueue
 * will never be interrupted.  If this is called from both signal
 * handlers and regular code, make sure the signal is suspended when
 * called from regular code.
 */

void
xf86eqEnqueue (e)
    xEvent	*e;
{
    int	oldtail, newtail;
    Bool    isMotion;
#ifdef XINPUT
    int     count;

    switch (e->u.u.type)
      {
      case KeyPress:
      case KeyRelease:
      case ButtonPress:
      case ButtonRelease:
      case MotionNotify:
        count = 1;
        break;
      default:
        if (!((deviceKeyButtonPointer *) e)->deviceid & MORE_EVENTS)
          {
            count = 1;
          }
        else
          {
          count = 2;
          }
        break;
      }
#endif

    oldtail = xf86EventQueue.tail;
    isMotion = e->u.u.type == MotionNotify;
    if (isMotion && xf86EventQueue.lastMotion && oldtail != xf86EventQueue.head)
    {
	if (oldtail == 0)
	    oldtail = QUEUE_SIZE;
	oldtail = oldtail - 1;
    }
    else
    {
    	newtail = oldtail + 1;
    	if (newtail == QUEUE_SIZE)
	    newtail = 0;
    	/* Toss events which come in late */
    	if (newtail == xf86EventQueue.head)
	    return;
	xf86EventQueue.tail = newtail;
    }
    
    xf86EventQueue.lastMotion = isMotion;
    xf86EventQueue.events[oldtail].event = *e;
#ifdef XINPUT
    if (count == 2)
    {
      xf86EventQueue.events[oldtail].val = *((deviceValuator *) (((deviceKeyButtonPointer *) e)+1));
    }
#endif
    /*
     * Make sure that event times don't go backwards - this
     * is "unnecessary", but very useful
     */
    if (e->u.keyButtonPointer.time < xf86EventQueue.lastEventTime &&
	xf86EventQueue.lastEventTime - e->u.keyButtonPointer.time < 10000)
    {
	xf86EventQueue.events[oldtail].event.u.keyButtonPointer.time =
	    xf86EventQueue.lastEventTime;
    }
    xf86EventQueue.events[oldtail].pScreen = xf86EventQueue.pEnqueueScreen;
}

/*
 * Call this from ProcessInputEvents()
 */
void
xf86eqProcessInputEvents ()
{
    EventRec	*e;
    int		x, y;
    xEvent	xe;
#ifdef XINPUT
    DeviceIntPtr                dev;
    int                         id, count;
    deviceKeyButtonPointer      *dev_xe;
#endif

    while (xf86EventQueue.head != xf86EventQueue.tail)
    {
	if (screenIsSaved == SCREEN_SAVER_ON)
	    SaveScreens (SCREEN_SAVER_OFF, ScreenSaverReset);
#ifdef DPMSExtension
        if (DPMSPowerLevel != DPMSModeOn)
            DPMSSet(DPMSModeOn);
#endif

	e = &xf86EventQueue.events[xf86EventQueue.head];
	/*
	 * Assumption - screen switching can only occur on motion events
	 */
	if (e->pScreen != xf86EventQueue.pDequeueScreen)
	{
	    xf86EventQueue.pDequeueScreen = e->pScreen;
	    x = e->event.u.keyButtonPointer.rootX;
	    y = e->event.u.keyButtonPointer.rootY;
	    if (xf86EventQueue.head == QUEUE_SIZE - 1)
	    	xf86EventQueue.head = 0;
	    else
	    	++xf86EventQueue.head;
	    NewCurrentScreen (xf86EventQueue.pDequeueScreen, x, y);
	}
	else
	{
	    xe = e->event;
	    if (xf86EventQueue.head == QUEUE_SIZE - 1)
	    	xf86EventQueue.head = 0;
	    else
	    	++xf86EventQueue.head;
	    switch (xe.u.u.type) 
	    {
	    case KeyPress:
	    case KeyRelease:
	    	(*xf86EventQueue.pKbd->processInputProc)
				(&xe, (DeviceIntPtr)xf86EventQueue.pKbd, 1);
	    	break;
#ifdef XINPUT
            case ButtonPress:
            case ButtonRelease:
            case MotionNotify:
	    	(*(inputInfo.pointer->public.processInputProc))
				(&xe, (DeviceIntPtr)inputInfo.pointer, 1);
                  break;

	    default:
              dev_xe = (deviceKeyButtonPointer *) e;
              id = dev_xe->deviceid & DEVICE_BITS;
              if (!(dev_xe->deviceid & MORE_EVENTS)) {
                count = 1;
              } else {
                count = 2;
              }
              dev = LookupDeviceIntRec(id);
              if (dev == NULL)
                {
                  ErrorF("LookupDeviceIntRec id=0x%x not found\n", id);
/*                   FatalError("xf86eqProcessInputEvents : device not found.\n");
 */
                  break;
                }
              if (!dev->public.processInputProc)
                {
                  FatalError("xf86eqProcessInputEvents : device has no input proc.\n");
                  break;
                }
              (*dev->public.processInputProc)(&e->event, dev, count);
#else
	    default:
	    	(*xf86EventQueue.pPtr->processInputProc)
				(&xe, (DeviceIntPtr)xf86EventQueue.pPtr, 1);
#endif
	    	break;
	    }
	}
    }
}

/* 
 * convenient functions to post events
 */

#define RELATIVE_CHECK(VALUATOR,IDX)				\
       {							\
	    if (!is_absolute) {					\
		(VALUATOR) += axisvals[(IDX)];			\
                if ((VALUATOR) < axes[(IDX)].min_value-1) {     \
                    (VALUATOR) = axes[(IDX)].min_value-1;       \
		}						\
		else if ((VALUATOR) > axes[(IDX)].max_value) {	\
		    (VALUATOR) = axes[(IDX)].max_value;		\
		}						\
		axisvals[(IDX)] = (VALUATOR);			\
	    }							\
    }


void
xf86PostMotionEvent(DeviceIntPtr	device,
		    int			is_absolute,
		    int			first_valuator,
		    int			num_valuators,
		    ...)
{
    va_list			var;
    int				loop;
    xEvent			xE[2];
    deviceKeyButtonPointer	*xev  = (deviceKeyButtonPointer*) xE;
    deviceValuator		*xv   = (deviceValuator*) xev+1;
    LocalDevicePtr		local = (LocalDevicePtr) device->public.devicePrivate;
    char			*buff = 0;
    Time			current;
    Bool			is_core = xf86IsCorePointer(device);
    Bool			is_shared = xf86ShareCorePointer(device);
    ValuatorClassPtr		val = device->valuator;
    int				*axisvals;
    AxisInfoPtr			axes;
    int				dx, dy;
    float			mult;

    DBG(5, ErrorF("xf86PostMotionEvent BEGIN 0x%x(%s) switch=0x%x is_core=%s is_shared=%s is_absolute=%s\n",
		  device, device->name, switch_device,
		  is_core ? "True" : "False",
		  is_shared ? "True" : "False",
		  is_absolute ? "True" : "False"));
    
    if (is_core || is_shared) {
      xf86SwitchCoreDevice(switch_device, device);
    }
    
    current = GetTimeInMillis();
    
    if (!is_core) {
      if (HAS_MOTION_HISTORY(local)) {
	buff = ((char *)local->motion_history +
		(sizeof(INT32) * local->dev->valuator->numAxes + sizeof(Time)) * local->last);
      }
    }

    if (num_valuators && (!val || (first_valuator + num_valuators > val->numAxes))) {
	ErrorF("Bad valuators reported for device \"%s\"\n", device->name);
	return;
    }

    axisvals = val->axisVal;
    axes = val->axes;
    
    va_start(var, num_valuators);

    for(loop=0; loop<num_valuators; loop++) {
	switch (loop % 6) {
	case 0:
	    xv->valuator0 = va_arg(var, int);
	    if (loop == num_valuators)
		RELATIVE_CHECK(xv->valuator0, loop+first_valuator);
	    break;
	case 1:
	    xv->valuator1 = va_arg(var, int);

	    DBG(5, ErrorF("xf86PostMotionEvent v0=%d v1=%d\n", xv->valuator0, xv->valuator1));
	    
	    if (loop == 1 && !is_absolute && device->ptrfeed && device->ptrfeed->ctrl.num) {
		/* modeled from xf86Events.c */
		if (device->ptrfeed->ctrl.threshold) {
		    if ((abs(xv->valuator0) + abs(xv->valuator1)) >= device->ptrfeed->ctrl.threshold) {
			xv->valuator0 = (xv->valuator0 * device->ptrfeed->ctrl.num) /
			    device->ptrfeed->ctrl.den;
			xv->valuator1 = (xv->valuator1 * device->ptrfeed->ctrl.num) /
			    device->ptrfeed->ctrl.den;
		    }
		}
		else if (xv->valuator0 || xv->valuator1) {
		    dx = xv->valuator0;
		    dy = xv->valuator1;
		    mult = pow((float)(dx*dx+dy*dy),
			       ((float)(device->ptrfeed->ctrl.num) /
				(float)(device->ptrfeed->ctrl.den) - 1.0) / 
			       2.0) / 2.0;
		    DBG(6, ErrorF("mult=%f dxremaind=%f dyremaind=%f\n",
				  mult, local->dxremaind, local->dyremaind));
		    if (dx) {
			local->dxremaind = mult * (float)dx + local->dxremaind;
			xv->valuator0 = dx = (int)local->dxremaind;
			local->dxremaind = local->dxremaind - (float)dx;
		    }
		    if (dy) {
			local->dyremaind = mult * (float)dy + local->dyremaind;
			xv->valuator1 = dy = (int)local->dyremaind;
			local->dyremaind = local->dyremaind - (float)dy;
		    }
		}
		DBG(6, ErrorF("xf86PostMotionEvent acceleration v0=%d v1=%d\n", xv->valuator0, xv->valuator1));
	    }

            /* mr Sat Jul  5 13:46:55 MET 1997
             * fix to recognize XWarpCursor requests
	     * FL Thu Nov 12 07:42:03 1998
	     * Fix the fix to revert x/y coordinates to valuators space.
	     * This has to be done only for relative devices which control
	     * the core pointer.
             */
            if ((loop == 1) && !is_absolute && (is_core || is_shared)) {
               int x1,y1;
               miPointerPosition(&x1,&y1);
               if (x1!=local->old_x || y1!=local->old_y ) {
		 if (!local->reverse_conversion_proc) {
		   axisvals[loop+first_valuator-1] = x1;
		   axisvals[loop+first_valuator] = y1;
		 }
		 else {
		   (*local->reverse_conversion_proc)(local, x1, y1, axisvals);
		 }
		 DBG(5, ErrorF("xf86PostMotionEvent(mr) x1=%d y1=%d\n", x1,y1));
	       }
            }
	    RELATIVE_CHECK(xv->valuator0, loop+first_valuator-1);
	    RELATIVE_CHECK(xv->valuator1, loop+first_valuator);
	    break;
	case 2:
	    xv->valuator2 = va_arg(var, int);
	    RELATIVE_CHECK(xv->valuator2, loop+first_valuator);
	    break;
	case 3:
	    xv->valuator3 = va_arg(var, int);
	    RELATIVE_CHECK(xv->valuator3, loop+first_valuator);
	    break;
	case 4:
	    xv->valuator4 = va_arg(var, int);
	    RELATIVE_CHECK(xv->valuator4, loop+first_valuator);
	    break;
	case 5:
	    xv->valuator5 = va_arg(var, int);
	    RELATIVE_CHECK(xv->valuator5, loop+first_valuator);
	    break;
	}
	if ((loop % 6 == 5) || (loop == num_valuators - 1)) {
	    xv->num_valuators = (loop % 6) + 1;
	    xv->first_valuator = first_valuator + (loop / 6) * 6;
	    
	    if (!is_core) {
		xev->type = DeviceMotionNotify;
		xev->detail = 0;
		xf86Info.lastEventTime = xev->time = current;
		xev->deviceid = device->id | MORE_EVENTS;
            
		xv->type = DeviceValuator;
		xv->deviceid = device->id;
	    
		xv->device_state = 0;
		
		if (HAS_MOTION_HISTORY(local)) {
		    *(Time*)buff = current;
		    memcpy(buff+sizeof(Time)+sizeof(INT32)*xv->first_valuator, &xv->valuator0,
			   sizeof(INT32)*xv->num_valuators);
		}
		
		xf86eqEnqueue(xE);
	    }
	    if ((is_core || is_shared) && (num_valuators >= 2)) {
	        int	x, y;

		if ((*local->conversion_proc)(local,
					      xv->first_valuator,
					      xv->num_valuators,
					      xv->valuator0,
					      xv->valuator1,
					      xv->valuator2,
					      xv->valuator3,
					      xv->valuator4,
					      xv->valuator5,
					      &x, &y) == FALSE) {
		    DBG(4, ErrorF("xf86PostMotionEvent conversion failed\n"));
		    continue;
		}

		DBG(4, ErrorF("xf86PostMotionEvent x=%d y=%d\n", x, y));

		if (x == local->old_x && y == local->old_y) {
		    DBG(4, ErrorF("xf86PostMotionEvent same cursor position continuing\n"));
		    continue;
		}
		
		local->old_x = x;
		local->old_y = y;
		
		xf86Info.lastEventTime = current;

		/* FL [Sat Jun 14 14:32:01 1997]
		 * needs to integrate with DGA and XTEST event posting
		 */

		miPointerAbsoluteCursor(x, y, xf86Info.lastEventTime); 

		if (!is_shared)
		  break;
	    }
	}
    }
    va_end(var);
    if (HAS_MOTION_HISTORY(local)) {
	local->last = (local->last + 1) % device->valuator->numMotionEvents;
	if (local->last == local->first)
	    local->first = (local->first + 1) % device->valuator->numMotionEvents;
    }
    DBG(5, ErrorF("xf86PostMotionEvent END   0x%x(%s) switch=0x%x is_core=%s is_shared=%s\n",
		  device, device->name, switch_device,
		  is_core ? "True" : "False",
		  is_shared ? "True" : "False"));
}

void
xf86PostProximityEvent(DeviceIntPtr	device,
		       int		is_in,
		       int		first_valuator,
		       int		num_valuators,
		       ...)
{
    va_list			var;
    int				loop;
    xEvent			xE[2];
    deviceKeyButtonPointer	*xev = (deviceKeyButtonPointer*) xE;
    deviceValuator		*xv = (deviceValuator*) xev+1;
    ValuatorClassPtr		val = device->valuator;
    Bool			is_core = xf86IsCorePointer(device);
    Bool			is_absolute = val && ((val->mode & 1) == Relative);

    if (is_core) {
	return;
    }
  
    if (num_valuators && (!val || (first_valuator + num_valuators > val->numAxes))) {
	ErrorF("Bad valuators reported for device \"%s\"\n", device->name);
	return;
    }

    xev->type = is_in ? ProximityIn : ProximityOut;
    xev->detail = 0;
    xev->deviceid = device->id | MORE_EVENTS;
	
    xv->type = DeviceValuator;
    xv->deviceid = device->id;
    xv->device_state = 0;

    if ((device->valuator->mode & 1) == Relative) {
	num_valuators = 0;
    }
  
    if (num_valuators != 0) {
	int	*axisvals = val->axisVal;
	    
	va_start(var, num_valuators);

	for(loop=0; loop<num_valuators; loop++) {
	    switch (loop % 6) {
	    case 0:
		xv->valuator0 = is_absolute ? va_arg(var, int) : axisvals[loop]; 
		break;
	    case 1:
		xv->valuator1 = is_absolute ? va_arg(var, int) : axisvals[loop];
		break;
	    case 2:
		xv->valuator2 = is_absolute ? va_arg(var, int) : axisvals[loop];
		break;
	    case 3:
		xv->valuator3 = is_absolute ? va_arg(var, int) : axisvals[loop];
		break;
	    case 4:
		xv->valuator4 = is_absolute ? va_arg(var, int) : axisvals[loop];
		break;
	    case 5:
		xv->valuator5 = is_absolute ? va_arg(var, int) : axisvals[loop];
		break;
	    }
	    if ((loop % 6 == 5) || (loop == num_valuators - 1)) {
		xf86Info.lastEventTime = xev->time = GetTimeInMillis();

		xv->num_valuators = (loop % 6) + 1;
		xv->first_valuator = first_valuator + (loop / 6) * 6;
	
		xf86eqEnqueue(xE);
	    }
	}
	va_end(var);
    }
    else {
	/* no valuator */
	xf86Info.lastEventTime = xev->time = GetTimeInMillis();

	xv->num_valuators = 0;
	xv->first_valuator = 0;
      
	xf86eqEnqueue(xE);
    }
}

void
xf86PostButtonEvent(DeviceIntPtr	device,
		    int			is_absolute,
		    int			button,
		    int			is_down,
		    int			first_valuator,
		    int			num_valuators,
		    ...)
{
    va_list			var;
    int				loop;
    xEvent			xE[2];
    deviceKeyButtonPointer	*xev	        = (deviceKeyButtonPointer*) xE;
    deviceValuator		*xv	        = (deviceValuator*) xev+1;
    ValuatorClassPtr		val		= device->valuator;
    Bool			is_core		= xf86IsCorePointer(device);
    Bool			is_shared       = xf86ShareCorePointer(device);
    
    DBG(5, ErrorF("xf86PostButtonEvent BEGIN 0x%x(%s) button=%d down=%s is_core=%s is_shared=%s is_absolute=%s\n",
		  device, device->name, button,
		  is_down ? "True" : "False",
		  is_core ? "True" : "False",
		  is_shared ? "True" : "False",
		  is_absolute ? "True" : "False"));
    
    /* Check the core pointer button state not to send an inconsistent
     * event. This can happen with the AlwaysCore feature.
     */
    if ((is_core || is_shared) && !xf86CheckButton(button, is_down)) {
	return;
    }
    
    if (num_valuators && (!val || (first_valuator + num_valuators > val->numAxes))) {
	ErrorF("Bad valuators reported for device \"%s\"\n", device->name);
	return;
    }

    if (is_core || is_shared) {
	xf86SwitchCoreDevice(switch_device, device);
    }

    if (!is_core) {
	xev->type = is_down ? DeviceButtonPress : DeviceButtonRelease;
	xev->detail = button;
	xev->deviceid = device->id | MORE_EVENTS;
	    
	xv->type = DeviceValuator;
	xv->deviceid = device->id;
	xv->device_state = 0;

	if (num_valuators != 0) {
	    int			*axisvals = val->axisVal;
	    
	    va_start(var, num_valuators);
      
	    for(loop=0; loop<num_valuators; loop++) {
		switch (loop % 6) {
		case 0:
		    xv->valuator0 = is_absolute ? va_arg(var, int) : axisvals[loop];
		    break;
		case 1:
		    xv->valuator1 = is_absolute ? va_arg(var, int) : axisvals[loop];
		    break;
		case 2:
		    xv->valuator2 = is_absolute ? va_arg(var, int) : axisvals[loop];
		    break;
		case 3:
		    xv->valuator3 = is_absolute ? va_arg(var, int) : axisvals[loop];
		    break;
		case 4:
		    xv->valuator4 = is_absolute ? va_arg(var, int) : axisvals[loop];
		    break;
		case 5:
		    xv->valuator5 = is_absolute ? va_arg(var, int) : axisvals[loop];
		    break;
		}
		if ((loop % 6 == 5) || (loop == num_valuators - 1)) {
		    xf86Info.lastEventTime = xev->time = GetTimeInMillis();
		    xv->num_valuators = (loop % 6) + 1;
		    xv->first_valuator = first_valuator + (loop / 6) * 6;
		    xf86eqEnqueue(xE);
		}
	    }
	    va_end(var);
	}
	else {
	    /* no valuator */
	    xf86Info.lastEventTime = xev->time = GetTimeInMillis();
	    xv->num_valuators = 0;
	    xv->first_valuator = 0;
	    xf86eqEnqueue(xE);
	}
    }

    if (is_core || is_shared) {
	/* core pointer */
	int       cx, cy;
      
	GetSpritePosition(&cx, &cy);
      
	xE->u.u.type = is_down ? ButtonPress : ButtonRelease;
	xE->u.u.detail =  device->button->map[button];
	xE->u.keyButtonPointer.rootY = cx;
	xE->u.keyButtonPointer.rootX = cy;
	xf86Info.lastEventTime = xE->u.keyButtonPointer.time = GetTimeInMillis();
	xf86eqEnqueue(xE);
    }
    DBG(5, ErrorF("xf86PostButtonEvent END\n"));
}

void
xf86PostKeyEvent(DeviceIntPtr	device,
		 unsigned int	key_code,
		 int		is_down,
		 int		is_absolute,
		 int		first_valuator,
		 int		num_valuators,
		 ...)
{
    va_list			var;
    int				loop;
    xEvent			xE[2];
    deviceKeyButtonPointer	*xev = (deviceKeyButtonPointer*) xE;
    deviceValuator		*xv = (deviceValuator*) xev+1;
    
    va_start(var, num_valuators);

    for(loop=0; loop<num_valuators; loop++) {
	switch (loop % 6) {
	case 0:
	    xv->valuator0 = va_arg(var, int);
	    break;
	case 1:
	    xv->valuator1 = va_arg(var, int);
	    break;
	case 2:
	    xv->valuator2 = va_arg(var, int);
	    break;
	case 3:
	    xv->valuator3 = va_arg(var, int);
	    break;
	case 4:
	    xv->valuator4 = va_arg(var, int);
	    break;
	case 5:
	    xv->valuator5 = va_arg(var, int);
	    break;
	}
	if (((loop % 6 == 5) || (loop == num_valuators - 1))) {
	    xev->type = is_down ? DeviceKeyPress : DeviceKeyRelease;
	    xev->detail = key_code;
	    
	    xf86Info.lastEventTime = xev->time = GetTimeInMillis();
	    xev->deviceid = device->id | MORE_EVENTS;
	    
	    xv->type = DeviceValuator;
	    xv->deviceid = device->id;
	    xv->device_state = 0;
	    /* if the device is in the relative mode we don't have to send valuators */
	    xv->num_valuators = is_absolute ? (loop % 6) + 1 : 0;
	    xv->first_valuator = first_valuator + (loop / 6) * 6;
	    
	    xf86eqEnqueue(xE);
	    /* if the device is in the relative mode only one event is needed */
	    if (!is_absolute) break;
	}
    }
    va_end(var);
}

/* 
 * Motion history management.
 */

void
xf86MotionHistoryAllocate(LocalDevicePtr	local)
{
    ValuatorClassPtr	valuator = local->dev->valuator;
    
    if (!HAS_MOTION_HISTORY(local))
	return;
    
    local->motion_history = xalloc((sizeof(INT32) * valuator->numAxes + sizeof(Time))
				   * valuator->numMotionEvents);
    local->first = 0;
    local->last	 = 0;
}

int
xf86GetMotionEvents(DeviceIntPtr	dev,
		    xTimecoord		*buff,
		    unsigned long	start,
		    unsigned long	stop,
		    ScreenPtr		pScreen)
{
    LocalDevicePtr	local	 = (LocalDevicePtr)dev->public.devicePrivate;
    ValuatorClassPtr	valuator = dev->valuator;
    int			num  	 = 0;
    int			loop	 = local->first;
    int			size;
    Time		current;
    
    if (!HAS_MOTION_HISTORY(local))
	return 0;

    size = (sizeof(INT32) * valuator->numAxes + sizeof(Time));

    while (loop != local->last) {
	current = *(Time*)(((char *)local->motion_history)+loop*size);
	if (current > stop)
	    return num;
	if (current >= start) {
	    memcpy(((char *)buff)+size*num,
		   ((char *)local->motion_history)+loop*size, size);
	    num++;
	}
	loop = (loop + 1) % valuator->numMotionEvents;
    }
    return num;
}

/* end of xf86Xinput.c */
