#ifndef __RETRACE_TRACEFD_H__
#define __RETRACE_TRACEFD_H__

void init_tracefd_handlers(struct retrace_handle *handle);

struct dirinfo {
	SLIST_ENTRY(dirinfo) next;
	pid_t pid;
	DIR *dir;
	char *info;
};

SLIST_HEAD(dirinfo_h, dirinfo);

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

struct tracefd {
	struct fdinfo_h fdinfos;
	struct streaminfo_h streaminfos;
	struct dirinfo_h dirinfos;
};

void set_fdinfo(struct fdinfo_h *infos, pid_t pid, int fd, const char *info);
const struct fdinfo *get_fdinfo(struct fdinfo_h *infos, pid_t pid, int fd);

void set_dirinfo(struct dirinfo_h *infos, pid_t pid, DIR *dir,
	const char *info);
const struct dirinfo *get_dirinfo(struct dirinfo_h *infos, pid_t pid,
	DIR *dir);

void set_streaminfo(struct streaminfo_h *infos, pid_t pid, FILE *stream,
	const char *info);
const struct streaminfo *get_streaminfo(struct streaminfo_h *infos, pid_t pid,
	FILE *stream);
#endif
