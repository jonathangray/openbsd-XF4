/*
 * Copyright (c) 1995, 1996, 1997 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the Kungliga Tekniska
 *      H�gskolan and its contributors.
 * 
 * 4. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* $KTH: kx.h,v 1.28 1997/11/09 11:14:38 joda Exp $ */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xauth.h>

#include <kerberosIV/krb.h>

#include <err.h>

int copy_encrypted (int fd1, int fd2, des_cblock *iv,
		    des_key_schedule schedule);

extern char x_socket[];
extern u_int32_t display_num;
extern char display[];
extern int display_size;
extern char xauthfile[];
extern int xauthfile_size;
extern u_char cookie[];
extern size_t cookie_len;

int get_xsockets (int *unix_socket, int *tcp_socket);
int connect_local_xsocket (unsigned dnr);
int create_and_write_cookie (char *xauthfile,
			     size_t size,
			     u_char *cookie,
			     size_t sz);
int verify_and_remove_cookies (int fd, int sock);
int replace_cookie(int xserver, int fd, char *filename);

int suspicious_address (int sock, struct sockaddr_in addr);

int
write_encrypted (int fd, void *buf, size_t len, des_key_schedule schedule,
		 des_cblock *session, struct sockaddr_in *me,
		 struct sockaddr_in *him);

int
read_encrypted (int fd, void *buf, size_t len, void **ret,
		des_key_schedule schedule, des_cblock *session,
		struct sockaddr_in *him, struct sockaddr_in *me);


#define KX_PORT 2111

#define KX_OLD_VERSION "KXSERV.1"
#define KX_VERSION "KXSERV.2"

#define COOKIE_TYPE "MIT-MAGIC-COOKIE-1"

enum { INIT = 0, ACK = 1, NEW_CONN = 2, ERROR = 3 };

enum kx_flags { PASSIVE = 1, KEEP_ALIVE = 2 };

typedef enum kx_flags kx_flags;
