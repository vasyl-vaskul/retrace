#ifndef __RETRACE_HANDLERS_H__
#define __RETRACE_HANDLERS_H__

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
#endif
