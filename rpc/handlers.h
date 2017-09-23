#ifndef __RETRACE_HANDLERS_H__
#define __RETRACE_HANDLERS_H__

#include <fcntl.h>

#include "tracefd.h"

void set_default_handlers(struct retrace_handle *handle);

struct handler_info {
#if BACKTRACE
	int backtrace_depth;
#endif
	int expand_strings;
	int expand_structs;
	int tracefds;
	struct tracefd tracefd_info;
};

#define DISPLAYFN_open do {						\
	display_string(ep, params->pathname);				\
	printf(", ");							\
	display_fflags(ep, params->flags);				\
	if (params->flags & (O_CREAT | O_TMPFILE)) {			\
		printf(", ");						\
		DISPLAY_mode_t(ep, params->mode);			\
	}								\
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

#endif
