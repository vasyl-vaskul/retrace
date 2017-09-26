#ifndef __RETRACE_HANDLERS_H__
#define __RETRACE_HANDLERS_H__

#include "tracefd.h"

void set_log_handlers(struct retrace_handle *handle);

struct handler_info {
#if BACKTRACE
	int backtrace_depth;
#endif
	int expand_buffers;
	int expand_strings;
	int expand_structs;
	int tracefds;
	struct tracefd tracefd_info;
};

#endif
