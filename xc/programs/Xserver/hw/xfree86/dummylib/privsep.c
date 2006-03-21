/**
 ** Public domain
 **/
/* $OpenBSD: privsep.c,v 1.3 2006/03/21 18:53:58 matthieu Exp $ */
#include <sys/types.h>
#include <fcntl.h>

int 
priv_init(uid_t uid, gid_t gid)
{
	return 0;
}

int
priv_open_device(char *path)
{
	return open(path, O_RDWR);
}
