#ifndef __RETRACE_HANDLERS_H__
#define __RETRACE_HANDLERS_H__

#include <fcntl.h>

void get_handlers(retrace_precall_handler_t *pre,
    retrace_postcall_handler_t *post);

#define FILEOPEN_fopen do {						\
	int x;								\
	char path[1024], mode[10], info[1024];				\
									\
	if (result == NULL)						\
		break;							\
									\
	x = retrace_fetch_string(ep->fd, params->path, path,		\
	    sizeof(path));						\
	if (x == 0)							\
		break;							\
	x = retrace_fetch_string(ep->fd, params->mode, mode,		\
	    sizeof(mode));						\
	if (x == 0)							\
		break;							\
	snprintf(info, sizeof(info), "fopen(\"%s\", \"%s\")",		\
	    path, mode);						\
	set_streaminfo(&display_info->streaminfos, ep->pid,		\
	    result, info);						\
} while (0);

#define FILECLOSE_fclose do {						\
	set_streaminfo(&display_info->streaminfos, ep->pid,		\
	    params->stream, NULL);					\
} while (0);

#define DISPLAYFN_open do {						\
	display_string(ep, params->pathname);				\
	printf(", ");							\
	display_fflags(ep, params->flags);				\
	if (params->flags & (O_CREAT | O_TMPFILE)) {			\
		printf(", ");						\
		DISPLAY_mode_t(ep, params->mode);			\
	}								\
} while (0);

#define FDOPEN_open do {						\
	int x;								\
	char path[1024], info[1024];					\
									\
	if (result == -1)						\
		break;							\
									\
	x = retrace_fetch_string(ep->fd, params->pathname, path,	\
	    sizeof(path));						\
	if (x == 0)							\
		break;							\
	if (params->flags & (O_CREAT | O_TMPFILE))			\
		snprintf(info, sizeof(info),				\
		    "open(\"%s\", 0x%x, 0%.3o)",			\
		    path, params->flags, params->mode);			\
	else								\
		snprintf(info, sizeof(info), "open(\"%s\", 0x%x)",	\
		    path, params->flags);				\
	set_fdinfo(&display_info->fdinfos, ep->pid, result, info);	\
} while (0);

#define DISPLAYFN_openat do {						\
	DISPLAY_int(ep, params->dirfd);					\
	printf(", ");							\
	display_string(ep, params->pathname);				\
	printf(", ");							\
	display_fflags(ep, params->flags);				\
	if (params->flags & (O_CREAT | O_TMPFILE)) {			\
		printf(", ");						\
		DISPLAY_mode_t(ep, params->mode);			\
	}								\
} while (0);

#define FDOPEN_openat do {						\
	int x;								\
	char path[1024], info[1024];					\
									\
	if (result == -1)						\
		break;							\
									\
	x = retrace_fetch_string(ep->fd, params->pathname, path,	\
	    sizeof(path));						\
	if (x == 0)							\
		break;							\
	if (params->flags & (O_CREAT | O_TMPFILE))			\
		snprintf(info, sizeof(info),				\
		    "openat(%d, \"%s\", 0x%x, 0%.3o)",			\
		    params->dirfd, path, params->flags, params->mode);	\
	else								\
		snprintf(info, sizeof(info),				\
		"openat(%d, \"%s\", 0x%x)",				\
		    params->dirfd, path, params->flags);		\
	set_fdinfo(&display_info->fdinfos, ep->pid,			\
	    result, info);						\
} while (0);

#define FDCLOSE_close do {						\
	set_fdinfo(&display_info->fdinfos, ep->pid, params->fd, NULL);	\
} while (0);

#endif
