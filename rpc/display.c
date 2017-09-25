/*
 * Copyright (c) 2017, [Ribose Inc](https://www.ribose.com).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../config.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "frontend.h"
#include "display.h"
#include "tracefd.h"
#include "handlers.h"

struct flag_name {
	int flag;
	const char *name;
};

static char *
format_char(char *buf, int c)
{
	static const char hex[] = "0123456789ABCDEF";

	if (isprint(c) && c != '\\')
		*(buf++) = c;
	else {
		*(buf++) = '\\';
		switch (c) {
		case '\\':
			*(buf++) = '\\';
			break;
		case '\0':
			*(buf++) = '0';
			break;
		case '\a':
			*(buf++) = 'a';
			break;
		case '\b':
			*(buf++) = 'b';
			break;
		case '\t':
			*(buf++) = 't';
			break;
		case '\n':
			*(buf++) = 'n';
			break;
		case '\v':
			*(buf++) = 'v';
			break;
		case '\f':
			*(buf++) = 'f';
			break;
		case '\r':
			*(buf++) = 'r';
			break;
		default:
			*(buf++) = 'x';
			*(buf++) = hex[c >> 4 & 0xf];
			*(buf++) = hex[c & 0xf];
			break;
		}
	}
	return buf;
}

void
print_string(const char *s)
{
	char buf[256], *p = buf, *e = buf + 250;

	while (*s) {
		if (p >= e) {
			*p = '\0';
			printf("%s", buf);
			p = buf;
		}
		p = format_char(p, *s);
		s++;
	}
	if (p != buf) {
		*p = '\0';
		printf("%s", buf);
	}
}

void
display_string(struct retrace_endpoint *ep, const char *s)
{
	struct display_info *di = ep->handle->user_data;
	char buf[di->expand_strings + 1];
	int snipped;

	if (s && di->expand_strings) {
		buf[di->expand_strings] = '\0';
		retrace_fetch_string(ep->fd, s, buf,
		    di->expand_strings + 1);
		snipped = buf[di->expand_strings] != '\0';
		buf[di->expand_strings] = '\0';
		printf("\"");
		print_string(buf);
		printf(snipped ? "\"+" : "\"");
	} else {
		printf("%p", s);
	}
}

void
display_char(int c)
{
	char buf[16], *e;

	e = format_char(buf, c);
	*e = '\0';
	printf("'%s'", buf);
}

void
display_fd(struct retrace_endpoint *ep, int fd)
{
	struct handler_info *hi = ep->handle->user_data;
	const struct fdinfo *fi = NULL;

	if (hi->tracefds)
		fi = get_fdinfo(&hi->tracefd_info.fdinfos, ep->pid, fd);

	if (fi != NULL)
		printf("%d:%s", fd, fi->info);
	else
		printf("%d", fd);
}

void
display_stream(struct retrace_endpoint *ep, FILE *stream)
{
	struct handler_info *hi = ep->handle->user_data;
	const struct streaminfo *si = NULL;

	if (hi->tracefds)
		si = get_streaminfo(&hi->tracefd_info.streaminfos,
		    ep->pid, stream);

	if (si != NULL)
		printf("%p:%s", stream, si->info);
	else
		printf("%p", stream);
}

void
display_dir(struct retrace_endpoint *ep, DIR *dir)
{
	struct handler_info *hi = ep->handle->user_data;
	const struct dirinfo *dirinfo = NULL;

	if (hi->tracefds)
		dirinfo = get_dirinfo(&hi->tracefd_info.dirinfos,
		    ep->pid, dir);

	if (dirinfo != NULL)
		printf("%p:%s", dir, dirinfo->info);
	else
		printf("%p", dir);
}

void
display_fflags(struct retrace_endpoint *ep, int flags)
{
	static struct flag_name flag_names[] = {
		{O_APPEND,	"O_APPEND"},
		{O_ASYNC,	"O_ASYNC"},
		{O_CLOEXEC,	"O_CLOEXEC"},
		{O_CREAT,	"O_CREAT"},
		{O_DIRECT,	"O_DIRECT"},
		{O_DIRECTORY,	"O_DIRECTORY"},
		{O_DSYNC,	"O_DSYNC"},
		{O_EXCL,	"O_EXCL"},
		{O_LARGEFILE,	"O_LARGEFILE"},
		{O_NOATIME,	"O_NOATIME"},
		{O_NOCTTY,	"O_NOCTTY"},
		{O_NOFOLLOW,	"O_NOFOLLOW"},
		{O_NONBLOCK,	"O_NONBLOCK"},
		{O_NDELAY,	"O_NDELAY"},
		{O_PATH,	"O_PATH"},
		{O_SYNC,	"O_SYNC"},
		{O_TMPFILE,	"O_TMPFILE"},
		{O_TRUNC,	"O_TRUNC"},
		{0,	NULL} };
	struct flag_name *f;

	switch (flags & O_ACCMODE) {
	case O_RDONLY:
		printf("O_RDONLY");
		break;
	case O_WRONLY:
		printf("O_WRONLY");
		break;
	case O_RDWR:
		printf("O_RDWR");
		break;
	}

	flags &= ~O_ACCMODE;

	for (f = flag_names; f->name != NULL; ++f) {
		if ((flags & f->flag) != 0)
			printf(" | %s", f->name);
		flags &= ~f->flag;
	}

	if (flags != 0)
		printf(" | UNKNOWN(%x)", flags);
}

void
display_msgflags(struct retrace_endpoint *ep, int flags)
{
	static struct flag_name flag_names[] = {
		{MSG_CONFIRM,	"MSG_CONFIRM"},
		{MSG_DONTROUTE,	"MSG_DONTROUTE"},
		{MSG_DONTWAIT,	"MSG_DONTWAIT"},
		{MSG_EOR,	"MSG_EOR"},
		{MSG_MORE,	"MSG_MORE"},
		{MSG_NOSIGNAL,	"MSG_NOSIGNAL"},
		{MSG_OOB,	"MSG_OOB"},
		{MSG_CMSG_CLOEXEC,	"MSG_CMSG_CLOEXEC"},
		{MSG_DONTWAIT,	"MSG_DONTWAIT"},
		{MSG_ERRQUEUE,	"MSG_ERRQUEUE"},
		{MSG_PEEK,	"MSG_PEEK"},
		{MSG_TRUNC,	"MSG_TRUNC"},
		{MSG_WAITALL,	"MSG_WAITALL"},
		{0,	NULL} };
	struct flag_name *f;
	const char *fmt = "%s";

	if (flags == 0)
		printf("0");
	else {
		for (f = flag_names; f->name != NULL; ++f) {
			if ((flags & f->flag) != 0) {
				printf(fmt, f->name);
				fmt = " | %s";
			}
		}
	}

	if (flags != 0) {
		printf(fmt, "UNKNOWN");
		printf("(%x)", flags);
	}
}

void
display_errno(int _errno)
{
	static struct flag_name flag_names[] = {
		{E2BIG,	"E2BIG"},
		{EACCES,	"EACCES"},
		{EADDRINUSE,	"EADDRINUSE"},
		{EADDRNOTAVAIL,	"EADDRNOTAVAIL"},
		{EAFNOSUPPORT,	"EAFNOSUPPORT"},
		{EAGAIN,	"EAGAIN"},
		{EALREADY,	"EALREADY"},
		{EBADE,	"EBADE"},
		{EBADF,	"EBADF"},
		{EBADFD,	"EBADFD"},
		{EBADMSG,	"EBADMSG"},
		{EBADR,	"EBADR"},
		{EBADRQC,	"EBADRQC"},
		{EBADSLT,	"EBADSLT"},
		{EBUSY,	"EBUSY"},
		{ECANCELED,	"ECANCELED"},
		{ECHILD,	"ECHILD"},
		{ECHRNG,	"ECHRNG"},
		{ECOMM,	"ECOMM"},
		{ECONNABORTED,	"ECONNABORTED"},
		{ECONNREFUSED,	"ECONNREFUSED"},
		{ECONNRESET,	"ECONNRESET"},
		{EDEADLK,	"EDEADLK"},
		{EDEADLOCK,	"EDEADLOCK"},
		{EDESTADDRREQ,	"EDESTADDRREQ"},
		{EDOM,	"EDOM"},
		{EDQUOT,	"EDQUOT"},
		{EEXIST,	"EEXIST"},
		{EFAULT,	"EFAULT"},
		{EFBIG,	"EFBIG"},
		{EHOSTDOWN,	"EHOSTDOWN"},
		{EHOSTUNREACH,	"EHOSTUNREACH"},
		{EIDRM,	"EIDRM"},
		{EILSEQ,	"EILSEQ"},
		{EINPROGRESS,	"EINPROGRESS"},
		{EINTR,	"EINTR"},
		{EINVAL,	"EINVAL"},
		{EIO,	"EIO"},
		{EISCONN,	"EISCONN"},
		{EISDIR,	"EISDIR"},
		{EISNAM,	"EISNAM"},
		{EKEYEXPIRED,	"EKEYEXPIRED"},
		{EKEYREJECTED,	"EKEYREJECTED"},
		{EKEYREVOKED,	"EKEYREVOKED"},
		{EL2HLT,	"EL2HLT"},
		{EL2NSYNC,	"EL2NSYNC"},
		{EL3HLT,	"EL3HLT"},
		{EL3RST,	"EL3RST"},
		{ELIBACC,	"ELIBACC"},
		{ELIBBAD,	"ELIBBAD"},
		{ELIBMAX,	"ELIBMAX"},
		{ELIBSCN,	"ELIBSCN"},
		{ELIBEXEC,	"ELIBEXEC"},
		{ELOOP,	"ELOOP"},
		{EMEDIUMTYPE,	"EMEDIUMTYPE"},
		{EMFILE,	"EMFILE"},
		{EMLINK,	"EMLINK"},
		{EMSGSIZE,	"EMSGSIZE"},
		{EMULTIHOP,	"EMULTIHOP"},
		{ENAMETOOLONG,	"ENAMETOOLONG"},
		{ENETDOWN,	"ENETDOWN"},
		{ENETRESET,	"ENETRESET"},
		{ENETUNREACH,	"ENETUNREACH"},
		{ENFILE,	"ENFILE"},
		{ENOBUFS,	"ENOBUFS"},
		{ENODATA,	"ENODATA"},
		{ENODEV,	"ENODEV"},
		{ENOENT,	"ENOENT"},
		{ENOEXEC,	"ENOEXEC"},
		{ENOKEY,	"ENOKEY"},
		{ENOLCK,	"ENOLCK"},
		{ENOLINK,	"ENOLINK"},
		{ENOMEDIUM,	"ENOMEDIUM"},
		{ENOMEM,	"ENOMEM"},
		{ENOMSG,	"ENOMSG"},
		{ENONET,	"ENONET"},
		{ENOPKG,	"ENOPKG"},
		{ENOPROTOOPT,	"ENOPROTOOPT"},
		{ENOSPC,	"ENOSPC"},
		{ENOSR,	"ENOSR"},
		{ENOSTR,	"ENOSTR"},
		{ENOSYS,	"ENOSYS"},
		{ENOTBLK,	"ENOTBLK"},
		{ENOTCONN,	"ENOTCONN"},
		{ENOTDIR,	"ENOTDIR"},
		{ENOTEMPTY,	"ENOTEMPTY"},
		{ENOTSOCK,	"ENOTSOCK"},
		{ENOTSUP,	"ENOTSUP"},
		{ENOTTY,	"ENOTTY"},
		{ENOTUNIQ,	"ENOTUNIQ"},
		{ENXIO,	"ENXIO"},
		{EOPNOTSUPP,	"EOPNOTSUPP"},
		{EOVERFLOW,	"EOVERFLOW"},
		{EPERM,	"EPERM"},
		{EPFNOSUPPORT,	"EPFNOSUPPORT"},
		{EPIPE,	"EPIPE"},
		{EPROTO,	"EPROTO"},
		{EPROTONOSUPPORT,	"EPROTONOSUPPORT"},
		{EPROTOTYPE,	"EPROTOTYPE"},
		{ERANGE,	"ERANGE"},
		{EREMCHG,	"EREMCHG"},
		{EREMOTE,	"EREMOTE"},
		{EREMOTEIO,	"EREMOTEIO"},
		{ERESTART,	"ERESTART"},
		{EROFS,	"EROFS"},
		{ESHUTDOWN,	"ESHUTDOWN"},
		{ESPIPE,	"ESPIPE"},
		{ESOCKTNOSUPPORT,	"ESOCKTNOSUPPORT"},
		{ESRCH,	"ESRCH"},
		{ESTALE,	"ESTALE"},
		{ESTRPIPE,	"ESTRPIPE"},
		{ETIME,	"ETIME"},
		{ETIMEDOUT,	"ETIMEDOUT"},
		{ETXTBSY,	"ETXTBSY"},
		{EUCLEAN,	"EUCLEAN"},
		{EUNATCH,	"EUNATCH"},
		{EUSERS,	"EUSERS"},
		{EWOULDBLOCK,	"EWOULDBLOCK"},
		{EXDEV,	"EXDEV"},
		{EXFULL,	"EXFULL"},
		{0,	NULL} };
	struct flag_name *f;

	for (f = flag_names; f->name; ++f) {
		if (f->flag == _errno) {
			printf(" [%s:%d])", f->name, _errno);
			return;
		}
	}
	printf(" [errno:%d]", _errno);
}

void
display_domain(int domain)
{
	static struct flag_name flag_names[] = {
		{AF_UNIX,	"AF_UNIX"},
		{AF_LOCAL,	"AF_LOCAL"},
		{AF_INET,	"AF_INET"},
		{AF_INET6,	"AF_INET6"},
		{AF_IPX,	"AF_IPX"},
		{AF_NETLINK,	"AF_NETLINK"},
		{AF_X25,	"AF_X25"},
		{AF_AX25,	"AF_AX25"},
		{AF_ATMPVC,	"AF_ATMPVC"},
		{AF_APPLETALK,	"AF_APPLETALK"},
		{AF_PACKET,	"AF_PACKET"},
		{AF_ALG,	"AF_ALG"},
		{0,	NULL} };
	struct flag_name *f;

	for (f = flag_names; f->name; ++f) {
		if (f->flag == domain) {
			printf("%s", f->name);
			return;
		}
	}
	printf("unknown:%d", domain);
}

void
display_protocol(int protocol)
{
	/* from /etc/protocols */
	/* TODO: read from file at startup */
	static struct flag_name flag_names[] = {
		{0,	"IP"},
		{0,	"HOPOPT"},
		{1,	"ICMP"},
		{2,	"IGMP"},
		{3,	"GGP"},
		{4,	"IP-ENCAP"},
		{5,	"ST"},
		{6,	"TCP"},
		{8,	"EGP"},
		{9,	"IGP"},
		{12,	"PUP"},
		{17,	"UDP"},
		{20,	"HMP"},
		{22,	"XNS-IDP"},
		{27,	"RDP"},
		{29,	"ISO-TP4"},
		{33,	"DCCP"},
		{36,	"XTP"},
		{37,	"DDP"},
		{38,	"IDPR-CMTP"},
		{41,	"IPv6"},
		{43,	"IPv6-Route"},
		{44,	"IPv6-Frag"},
		{45,	"IDRP"},
		{46,	"RSVP"},
		{47,	"GRE"},
		{0,	NULL} };
	struct flag_name *f;

	for (f = flag_names; f->name; ++f) {
		if (f->flag == protocol) {
			printf("%s", f->name);
			return;
		}
	}
	printf("unknown:%d", protocol);
}

void
display_socktype(int socktype)
{
	static struct flag_name flag_names[] = {
		{SOCK_STREAM,	"SOCK_STREAM"},
		{SOCK_DGRAM,	"SOCK_DGRAM"},
		{SOCK_SEQPACKET,	"SOCK_SEQPACKET"},
		{SOCK_RAW,	"SOCK_RAW"},
		{SOCK_RDM,	"SOCK_RDM"},
		{SOCK_PACKET,	"SOCK_PACKET"},
		{SOCK_NONBLOCK,	"SOCK_NONBLOCK"},
		{SOCK_CLOEXEC,	"SOCK_CLOEXEC"},
		{0,	NULL} };
	struct flag_name *f;

	for (f = flag_names; f->name; ++f) {
		if (f->flag == socktype) {
			printf("%s", f->name);
			return;
		}
	}
	printf("unknown:%d", socktype);
}

void
display_sockaddr(struct retrace_endpoint *ep, const struct sockaddr* addr, socklen_t len)
{
	struct display_info *di = ep->handle->user_data;
	struct sockaddr_storage raddr;
	int x;
	char buf[256];

	assert (sizeof(struct sockaddr_un) <= sizeof(raddr));

	if (!di->expand_structs || addr == NULL) {
		DISPLAY_pvoid(ep, addr);
		return;
	}

	if (len > sizeof(raddr))
		len = sizeof(raddr);

	x = retrace_fetch_memory(ep->fd, addr, &raddr, len);
	if (x == 0) {
		DISPLAY_pvoid(ep, addr);
		return;
	}

	if (raddr.ss_family == AF_UNIX) {
		struct sockaddr_un *sa = (struct sockaddr_un *)&raddr;
		printf("{AF_UNIX, %*s}", (int)sizeof(sa->sun_path),
		    sa->sun_path);
	} else if (raddr.ss_family == AF_INET) {
		struct sockaddr_in *sa = (struct sockaddr_in *)&raddr;
		printf("{AF_INET, %d, %s}", sa->sin_port,
		    inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf)));
	} else if (raddr.ss_family == AF_INET6) {
		struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&raddr;
		printf("{AF_INET6, %d, %s}", sa->sin6_port,
		    inet_ntop(AF_INET6, &sa->sin6_addr, buf, sizeof(buf)));
	}
}

void
display_buffer(struct retrace_endpoint *ep, const void* addr, size_t len)
{
}
