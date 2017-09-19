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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "frontend.h"
#include "display.h"

static char *
format_char(char *buf, int c)
{
	static const char hex[] = "0123456789ABCDEF";

	if (isprint(c))
		*(buf++) = c;
	else {
		*(buf++) = '\\';
		switch (c) {
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
display_fd(struct retrace_endpoint *ep, int fd)
{
	struct display_info *di = ep->handle->user_data;
	const struct fdinfo *fi = NULL;

	if (di->tracefds)
		fi = get_fdinfo(&di->fdinfos, ep->pid, fd);

	if (fi != NULL)
		printf("%d:%s", fd, fi->info);
	else
		printf("%d", fd);
}

void
set_fdinfo(struct fdinfo_h *infos, pid_t pid, int fd,
	const char *info)
{
	struct fdinfo *fi;

	fi = (struct fdinfo *)get_fdinfo(infos, pid, fd);
	if (fi != NULL) {
		SLIST_REMOVE(infos, fi, fdinfo, next);
		free(fi);
	}

	if (info == NULL)
		return;

	fi = malloc(sizeof(struct fdinfo) + strlen(info) + 1);
	if (fi != NULL) {
		fi->fd = fd;
		fi->pid = pid;
		fi->info = (char *)&fi[1];
		strcpy(fi->info, info);
		SLIST_INSERT_HEAD(infos, fi, next);
	}
}

const struct fdinfo *
get_fdinfo(struct fdinfo_h *infos, pid_t pid, int fd)
{
	const struct fdinfo *fi;

	SLIST_FOREACH(fi, infos, next)
		if (fi->pid == pid && fi->fd == fd)
			return fi;

	return NULL;
}

void
display_stream(struct retrace_endpoint *ep, FILE *stream)
{
	struct display_info *di = ep->handle->user_data;
	const struct streaminfo *si = NULL;

	if (di->tracefds)
		si = get_streaminfo(&di->streaminfos, ep->pid, stream);

	if (si != NULL)
		printf("%p:%s", stream, si->info);
	else
		printf("%p", stream);
}

void
set_streaminfo(struct streaminfo_h *infos, pid_t pid, FILE *stream,
	const char *info)
{
	struct streaminfo *si;

	si = (struct streaminfo *)get_streaminfo(infos, pid, stream);
	if (si != NULL) {
		SLIST_REMOVE(infos, si, streaminfo, next);
		free(si);
	}

	if (info == NULL)
		return;

	si = malloc(sizeof(struct streaminfo) + strlen(info) + 1);
	if (si != NULL) {
		si->stream = stream;
		si->pid = pid;
		si->info = (char *)&si[1];
		strcpy(si->info, info);
		SLIST_INSERT_HEAD(infos, si, next);
	}
}

const struct streaminfo *
get_streaminfo(struct streaminfo_h *infos, pid_t pid, FILE *stream)
{
	const struct streaminfo *si;

	SLIST_FOREACH(si, infos, next)
		if (si->pid == pid && si->stream == stream)
			return si;

	return NULL;
}
