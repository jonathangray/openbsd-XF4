/**
 ** Copyright (c) 2002 Matthieu Herrb
 **
 ** $Source: /tmp/OpenBSD-XF4-repo/xc/programs/Xserver/hw/kdrive/bsd/Attic/bsd.c,v $
 ** $Revision: 1.1 $
 ** $Date: 2002/06/07 07:01:32 $
 ** 
 **/
#ident "$Id: bsd.c,v 1.1 2002/06/07 07:01:32 matthieu Exp $"

#include "kdrive.h"

int
BsdInit(void)
{
	return 1;
}

void
BsdEnable(void)
{
}

Bool
BsdSpecialKey(KeySym sym)
{
	return FALSE;
}

void
BsdDisable(void)
{
}

void
BsdFini(void)
{
}

KdOsFuncs BsdFuncs = {
	BsdInit,
	BsdEnable,
	BsdSpecialKey,
	BsdDisable,
	BsdFini,
};

void
OsVendorInit(void)
{
	KdOsInit(&BsdFuncs);
}
