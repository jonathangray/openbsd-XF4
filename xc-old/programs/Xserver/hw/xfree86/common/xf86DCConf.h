/* $XFree86$ */

#include "xf86.h"
#include "xf86_Config.h"
#include "xf86_Option.h"

#ifndef DCConfig
extern
#endif
int DCpushToken;

#ifndef DCConfig
extern
#endif
LexRec DCval;

#ifndef DCConfig
extern
int xf86DCGetToken(
#if NeedFunctionPrototypes
char *,
SymTabRec [],
SymTabRec []
#endif
);
OFlagSet *xf86DCGetOption(
#if NeedFunctionPrototypes
char* Pointer,
OptFlagRec []
#endif
);
void xf86DCConfigError(
#if NeedFunctionPrototypes
char *
#endif
);
#endif
