; $Header: /tmp/OpenBSD-XF4-repo/xc-mit/server/ddx/hpbsd/input/Attic/cr16.s,v 1.1.1.1 2001/10/24 13:13:22 todd Exp $
        .SPACE  $TEXT$
        .SUBSPA $CODE$
        .export cr16
        .PROC
        .CALLINFO
        .ENTRY
cr16
        bv      (%rp)
	mfctl	16,%ret0
        .PROCEND
