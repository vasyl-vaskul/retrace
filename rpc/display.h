#ifndef __RETRACE_DISPLAY_H__
#define __RETRACE_DISPLAY_H__

#include <sys/queue.h>

#define DISPLAY_char(ep, c)	if (c) printf("'%c'", c); else printf("'\\0'");
#define DISPLAY_cmsghdr(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_cstring(ep, p)	display_string(ep, p)
#define DISPLAY_csockaddr(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_dir(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_dirent(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_file(ep, p)	display_stream(ep, p)
#define DISPLAY_fd(ep, i)	display_fd(ep, i)
#define DISPLAY_int(ep, i)	printf("%d", (i))
#define DISPLAY_long(ep, i)	printf("%ld", (i))
#define DISPLAY_mode_t(ep, m)	printf("0%.3o", m)
#define DISPLAY_msghdr(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_pid_t(ep, i)	DISPLAY_int(ep, i)
#define DISPLAY_pcvoid(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_pdirent(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_pint(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_psize_t(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_psocklen_t(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_pstring(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_pvoid(ep, p)	printf("%p", (p))
#define DISPLAY_size_t(ep, i)	DISPLAY_ulong(ep, i)
#define DISPLAY_sockaddr(ep, p)	DISPLAY_pvoid(ep, p)
#define DISPLAY_socklen_t(ep, i)	DISPLAY_uint(ep, i)
#define DISPLAY_ssize_t(ep, i)	DISPLAY_long(ep, i)
#define DISPLAY_string(ep, p)	display_string(ep, p)
#define DISPLAY_uint(ep, i)	printf("%u", (i))
#define DISPLAY_ulong(ep, i)	printf("%lu", (i))
#define DISPLAY_va_list(ep, ap)	printf("ap")

struct fdinfo {
	SLIST_ENTRY(fdinfo) next;
	pid_t pid;
	int fd;
	char *info;
};

SLIST_HEAD(fdinfo_h, fdinfo);

struct streaminfo {
	SLIST_ENTRY(streaminfo) next;
	pid_t pid;
	FILE *stream;
	char *info;
};

SLIST_HEAD(streaminfo_h, streaminfo);

struct display_info {
#if BACKTRACE
	int backtrace_functions[RPC_FUNCTION_COUNT];
	int backtrace_depth;
#endif
	int expand_strings;
	int tracefds;
	struct fdinfo_h fdinfos;
	struct streaminfo_h streaminfos;
};

void *display_buffer(void *buffer, size_t length);
void display_string(struct retrace_endpoint *ep, const char *s);
void display_fflags(struct retrace_endpoint *ep, int flags);

void display_fd(struct retrace_endpoint *ep, int fd);
void set_fdinfo(struct fdinfo_h *infos, pid_t pid, int fd, const char *info);
const struct fdinfo *get_fdinfo(struct fdinfo_h *infos, pid_t pid, int fd);

void display_stream(struct retrace_endpoint *ep, FILE *s);
void set_streaminfo(struct streaminfo_h *infos, pid_t pid, FILE *stream,
	const char *info);
const struct streaminfo *get_streaminfo(struct streaminfo_h *infos, pid_t pid,
	FILE *stream);
#endif
