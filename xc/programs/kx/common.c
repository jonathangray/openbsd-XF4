/* $KTH: common.c,v 1.34 1997/12/09 17:22:47 joda Exp $ */

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

#include "kx.h"
#include <fcntl.h>

char x_socket[MAXPATHLEN];

u_int32_t display_num;
char display[MAXPATHLEN];
int display_size = sizeof(display);
char xauthfile[MAXPATHLEN];
int xauthfile_size = sizeof(xauthfile);
u_char cookie[16];
size_t cookie_len = sizeof(cookie);

static int
do_enccopy (int fd1, int fd2, int mode, des_cblock *iv,
	    des_key_schedule schedule, int *num)
{
     int ret;
     u_char buf[BUFSIZ];

     ret = read (fd1, buf, sizeof(buf));
     if (ret == 0)
	  return 0;
     if (ret < 0) {
	 warn ("read");
	 return ret;
     }
#ifndef NOENCRYPTION
     des_cfb64_encrypt (buf, buf, ret, schedule, iv,
			num, mode);
#endif
     ret = krb_net_write (fd2, buf, ret);
     if (ret < 0) {
	 warn ("write");
	 return ret;
     }
     return 1;
}

/*
 * Copy data from `fd1' to `fd2', encrypting it.  Data in the other
 * direction is of course, decrypted.
 */

int
copy_encrypted (int fd1, int fd2, des_cblock *iv,
		des_key_schedule schedule)
{
     des_cblock iv1, iv2;
     int num1 = 0, num2 = 0;

     memcpy (iv1, *iv, sizeof(iv1));
     memcpy (iv2, *iv, sizeof(iv2));
     for (;;) {
	  fd_set fdset;
	  int ret;

	  FD_ZERO(&fdset);
	  FD_SET(fd1, &fdset);
	  FD_SET(fd2, &fdset);
#ifndef max
#define max(a,b) (a > b ? a : b)
#endif

	  ret = select (max(fd1, fd2)+1, &fdset, NULL, NULL, NULL);
	  if (ret < 0 && errno != EINTR) {
	      warn ("select");
	      return 1;
	  }
	  if (FD_ISSET(fd1, &fdset)) {
	       ret = do_enccopy (fd1, fd2, DES_ENCRYPT, &iv1, schedule, &num1);
	       if (ret <= 0)
		    return ret;
	  }
	  if (FD_ISSET(fd2, &fdset)) {
	       ret = do_enccopy (fd2, fd1, DES_DECRYPT, &iv2, schedule, &num2);
	       if (ret <= 0)
		    return ret;
	  }
     }
}

#ifndef X_UNIX_PATH
#define X_UNIX_PATH "/tmp/.X11-unix/X"
#endif

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK 0x7f000001
#endif

/*
 * Allocate and listen on a local X server socket and a TCP socket.
 * Return the display number.
 */

int
get_xsockets (int *unix_socket, int *tcp_socket)
{
     int unixfd, tcpfd = -1;
     struct sockaddr_un unixaddr;
     struct sockaddr_in tcpaddr;
     int dpy;
     int oldmask;
     struct in_addr local;
     char *dir, *p;

     if((dir = strdup (X_UNIX_PATH)) == NULL)
	 errx (1, "strdup: out of memory");
     p = strrchr (dir, '/');
     if (p)
       *p = '\0';

     oldmask = umask(0);
     mkdir (dir, 01777);
     chmod (dir, 01777);
     umask (oldmask);
     free (dir);

     memset(&local, 0, sizeof(local));
     local.s_addr = htonl(INADDR_LOOPBACK);

     for(dpy = 4; dpy < 256; ++dpy) {
	 unixfd = socket (AF_UNIX, SOCK_STREAM, 0);
	 if (unixfd < 0)
	     err (1, "socket AF_UNIX");
	 memset (&unixaddr, 0, sizeof(unixaddr));
	 unixaddr.sun_family = AF_UNIX;
	 snprintf (unixaddr.sun_path, sizeof(unixaddr.sun_path),
		   X_UNIX_PATH "%u", dpy);
	 if(bind(unixfd,
		 (struct sockaddr *)&unixaddr,
		 sizeof(unixaddr)) < 0) {
	     close (unixfd);
	     if (errno == EADDRINUSE ||
		 errno == EACCES) /* Cray return EACCESS */
		 continue;
	     else
		 return -1;
	 }

	 if (tcp_socket) {
	     int one = 1;

	     tcpfd = socket (AF_INET, SOCK_STREAM, 0);
	     if (tcpfd < 0)
		 err (1, "socket AF_INET");
	     setsockopt (tcpfd, IPPROTO_TCP, TCP_NODELAY, (void *)&one,
			 sizeof(one));
	     memset (&tcpaddr, 0, sizeof(tcpaddr));
	     tcpaddr.sin_family = AF_INET;
	     tcpaddr.sin_addr = local;
	     tcpaddr.sin_port = htons(6000 + dpy);
	     if (bind (tcpfd, (struct sockaddr *)&tcpaddr,
		       sizeof(tcpaddr)) < 0) {
		 close (unixfd);
		 close (tcpfd);
		 if (errno == EADDRINUSE)
		     continue;
		 else
		     return -1;
	     }
	 }
	 break;
     }
     if (dpy == 256)
	 errx (1, "no free x-servers");
     if (listen (unixfd, SOMAXCONN) < 0)
	 err (1, "listen");
     if (tcp_socket)
	 if (listen (tcpfd, SOMAXCONN) < 0)
	     err (1, "listen");
     strcpy(x_socket, unixaddr.sun_path);
     *unix_socket = unixfd;
     if (tcp_socket)
	 *tcp_socket = tcpfd;
     return dpy;
}

/*
 *
 */

int
connect_local_xsocket (unsigned dnr)
{
     int fd;
     struct sockaddr_un addr;

     fd = socket (AF_UNIX, SOCK_STREAM, 0);
     if (fd < 0)
	 err (1, "socket AF_UNIX");
     addr.sun_family = AF_UNIX;
     snprintf (addr.sun_path, sizeof(addr.sun_path),
	       X_UNIX_PATH "%u", dnr);
     if (connect (fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	 err (1, "connect");
     return fd;
}

void
get_rand_data(unsigned char *data, int size)
{
    int i;
    char *rnd_devices[] = {"/dev/srandom",
                           "/dev/urandom",
			   "/dev/prandom",
			   "/dev/arandom",
                           "/dev/random",
                           NULL};
    char **p;

    for(p = rnd_devices; *p; p++) {
      int fd = open(*p, O_RDONLY | O_NDELAY);
      
      if(fd >= 0 && read(fd, data, size) == size) {
        close(fd);
        return;
      }
      close(fd);
    }

    for (i = 0; i < size; i++)
	data[i] = random();
}

int
create_and_write_cookie (char *xauthfile,
			 size_t size,
			 u_char *cookie,
			 size_t sz)
{
     Xauth auth;
     char tmp[64];
     int fd;
     FILE *f;
     char hostname[MAXHOSTNAMELEN];
     struct in_addr loopback;
     struct hostent *h;
     int i;

     gethostname (hostname, sizeof(hostname));
     loopback.s_addr = htonl(INADDR_LOOPBACK);
     
     auth.family = FamilyLocal;
     auth.address = hostname;
     auth.address_length = strlen(auth.address);
     snprintf (tmp, sizeof(tmp), "%d", display_num);
     auth.number_length = strlen(tmp);
     auth.number = tmp;
     auth.name = COOKIE_TYPE;
     auth.name_length = strlen(auth.name);
     auth.data_length = sz;
     auth.data = (char*)cookie;
     get_rand_data (cookie, sz);

     strncpy(xauthfile, "/tmp/AXXXXX", size);
     xauthfile[size-1] = 0;
     fd = mkstemp(xauthfile);
     if(fd < 0)
	 return 1;
     f = fdopen(fd, "r+");
     if(f == NULL){
	 close(fd);
	 return 1;
     }
     if(XauWriteAuth(f, &auth) == 0) {
	  fclose(f);
	  return 1;
     }

     h = gethostbyname (hostname);
     if (h == NULL) {
	  fclose (f);
	  return 1;
     }

     /*
      * I would like to write a cookie for localhost:n here, but some
      * stupid code in libX11 will not look for cookies of that type,
      * so we are forced to use FamilyWild instead.
      */

     auth.family  = FamilyWild;
     auth.address_length = 0;

#if 0 /* XXX */
     auth.address = (char *)&loopback;
     auth.address_length = sizeof(loopback);
#endif

     if (XauWriteAuth(f, &auth) == 0) {
	  fclose (f);
	  return 1;
     }

     if(fclose(f))
	  return 1;
     return 0;
}

/*
 * Verify and remove cookies.  Read and parse a X-connection from
 * `fd'. Check the cookie used is the same as in `cookie'.  Remove the
 * cookie and copy the rest of it to `sock'.
 * Return 0 iff ok.
 */

int
verify_and_remove_cookies (int fd, int sock)
{
     u_char beg[12];
     int bigendianp;
     unsigned n, d, npad, dpad;
     char *protocol_name, *protocol_data;
     u_char zeros[6] = {0, 0, 0, 0, 0, 0};

     if (krb_net_read (fd, beg, sizeof(beg)) != sizeof(beg))
	  return 1;
     if (krb_net_write (sock, beg, 6) != 6)
	  return 1;
     bigendianp = beg[0] == 'B';
     if (bigendianp) {
	  n = (beg[6] << 8) | beg[7];
	  d = (beg[8] << 8) | beg[9];
     } else {
	  n = (beg[7] << 8) | beg[6];
	  d = (beg[9] << 8) | beg[8];
     }
     npad = (4 - (n % 4)) % 4;
     dpad = (4 - (d % 4)) % 4;
     protocol_name = malloc(n + npad);
     if (protocol_name == NULL)
	 return 1;
     protocol_data = malloc(d + dpad);
     if (protocol_data == NULL)
	 goto fail;
     if (krb_net_read (fd, protocol_name, n + npad) != n + npad)
	 goto fail;
     if (krb_net_read (fd, protocol_data, d + dpad) != d + dpad)
	 goto fail;
     if (strncmp (protocol_name, COOKIE_TYPE, strlen(COOKIE_TYPE)) != 0)
	 goto fail;
     if (d != cookie_len ||
	 memcmp (protocol_data, cookie, cookie_len) != 0)
	 goto fail;
     free (protocol_name);
     free (protocol_data);
     if (krb_net_write (sock, zeros, 6) != 6)
	  return 1;
     return 0;
fail:
     free (protocol_name);
     free (protocol_data);
     return 1;
}

/*
 * Get rid of the cookie that we were sent and get the correct one
 * from our own cookie file instead.
 */

int
replace_cookie(int xserver, int fd, char *filename)
{
     u_char beg[12];
     int bigendianp;
     unsigned n, d, npad, dpad;
     Xauth *auth;
     FILE *f;
     u_char zeros[6] = {0, 0, 0, 0, 0, 0};

     if (krb_net_read (fd, beg, sizeof(beg)) != sizeof(beg))
	  return 1;
     if (krb_net_write (xserver, beg, 6) != 6)
	  return 1;
     bigendianp = beg[0] == 'B';
     if (bigendianp) {
	  n = (beg[6] << 8) | beg[7];
	  d = (beg[8] << 8) | beg[9];
     } else {
	  n = (beg[7] << 8) | beg[6];
	  d = (beg[9] << 8) | beg[8];
     }
     if (n != 0 || d != 0)
	  return 1;
     f = fopen(filename, "r");
     if (f) {
	  u_char len[6] = {0, 0, 0, 0, 0, 0};

	  auth = XauReadAuth(f);
	  fclose(f);
	  n = auth->name_length;
	  d = auth->data_length;
	  if (bigendianp) {
	       len[0] = n >> 8;
	       len[1] = n & 0xFF;
	       len[2] = d >> 8;
	       len[3] = d & 0xFF;
	  } else {
	       len[0] = n & 0xFF;
	       len[1] = n >> 8;
	       len[2] = d & 0xFF;
	       len[3] = d >> 8;
	  }
	  if (krb_net_write (xserver, len, 6) != 6)
	       return 1;
	  if(krb_net_write (xserver, auth->name, n) != n)
	       return 1;
	  npad = (4 - (n % 4)) % 4;
	  if (npad) { 
	       if (krb_net_write (xserver, zeros, npad) != npad)
		    return 1;
	  }
	  if (krb_net_write (xserver, auth->data, d) != d)
	       return 1;
	  dpad = (4 - (d % 4)) % 4;
	  if (dpad) { 
	       if (krb_net_write (xserver, zeros, dpad) != dpad)
		    return 1;
	  }
	  XauDisposeAuth(auth);
     } else {
	  if(krb_net_write(xserver, zeros, 6) != 6)
	       return 1;
     }
     return 0;
}




/*
 * Some simple controls on the address and corresponding socket
 */

int
suspicious_address (int sock, struct sockaddr_in addr)
{
    char data[40];
    int len = sizeof(data);

    return addr.sin_addr.s_addr != htonl(INADDR_LOOPBACK)
	|| getsockopt (sock, IPPROTO_IP, IP_OPTIONS, data, &len) < 0
	|| len != 0;
}
