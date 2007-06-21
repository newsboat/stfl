/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2007  Clifford Wolf <clifford@clifford.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  iconv.c: Helper functions for widechar conversion
 */

#include "stfl.h"

#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <iconv.h>
#include <errno.h>

struct stfl_ipool_entry {
	void *data;
	struct stfl_ipool_entry *next;
};

struct stfl_ipool {
	iconv_t to_wc_desc;
	iconv_t from_wc_desc;
	char *code;
	struct stfl_ipool_entry *list;
};

struct stfl_ipool *stfl_ipool_create(const char *code)
{
	struct stfl_ipool *pool = malloc(sizeof(struct stfl_ipool));

	pool->to_wc_desc = (iconv_t)(-1);
	pool->from_wc_desc = (iconv_t)(-1);

	pool->code = strdup(code);
	pool->list = 0;

	return pool;
}

void *stfl_ipool_add(struct stfl_ipool *pool, void *data)
{
	struct stfl_ipool_entry *entry = malloc(sizeof(struct stfl_ipool_entry));

	entry->data = data;
	entry->next = pool->list;
	pool->list = entry;

	return data;
}


const wchar_t *stfl_ipool_towc(struct stfl_ipool *pool, const char *buf)
{
	if (!pool || !buf)
		return 0;

	if (!strcmp("WCHAR_T", pool->code))
		return (wchar_t*)buf;

	if (pool->to_wc_desc == (iconv_t)(-1))
		pool->to_wc_desc = iconv_open("WCHAR_T", pool->code);

	if (pool->to_wc_desc == (iconv_t)(-1))
		return 0;

	char *inbuf = (char*)buf;
	size_t inbytesleft = strlen(buf);

	int buffer_size = inbytesleft * 2 + 16;
	int buffer_pos = 0;
	char *buffer = NULL;

grow_buffer_retry:;
	buffer_size += inbytesleft * 2;
	buffer = realloc(buffer, buffer_size);

retry_without_growing:;
	char *outbuf = buffer + buffer_pos;
	size_t outbytesleft = buffer_size - buffer_pos;

	iconv(pool->to_wc_desc, NULL, NULL, NULL, NULL);
	int rc = iconv(pool->to_wc_desc, &inbuf, &inbytesleft, &outbuf, &outbytesleft);

	buffer_pos = outbuf - buffer;

	if (rc == -1 && errno == E2BIG)
		goto grow_buffer_retry;

	if (rc == -1 && (errno == EILSEQ || errno == EINVAL)) {
		// just copy this char as it is (e.g. when input is broken utf-8 with some latin1 chars)
		if (outbytesleft < sizeof(wchar_t))
			goto grow_buffer_retry;
		*((wchar_t*)outbuf) = *(unsigned char*)inbuf;
		buffer_pos += sizeof(wchar_t);
		inbuf++;
		inbytesleft--;
		goto retry_without_growing;
	}

	if (rc == -1) {
		free(buffer);
		return 0;
	}

	if (outbytesleft < sizeof(wchar_t))
		buffer = realloc(buffer, buffer_size+sizeof(wchar_t));
	*((wchar_t*)outbuf) = 0;

	return stfl_ipool_add(pool, buffer);
}

const char *stfl_ipool_fromwc(struct stfl_ipool *pool, const wchar_t *buf)
{
	if (!pool || !buf)
		return 0;

	if (!strcmp("WCHAR_T", pool->code))
		return (char*)buf;

	if (pool->from_wc_desc == (iconv_t)(-1))
		pool->from_wc_desc = iconv_open(pool->code, "WCHAR_T");

	if (pool->from_wc_desc == (iconv_t)(-1))
		return 0;

	char *inbuf = (char*)buf;
	size_t inbytesleft = wcslen(buf)*sizeof(wchar_t);

	int buffer_size = inbytesleft + 16;
	int buffer_pos = 0;
	char *buffer = NULL;

grow_buffer_retry:;
	buffer_size += inbytesleft;
	buffer = realloc(buffer, buffer_size);

retry_without_growing:;
	char *outbuf = buffer + buffer_pos;
	size_t outbytesleft = buffer_size - buffer_pos;

	iconv(pool->from_wc_desc, NULL, NULL, NULL, NULL);
	int rc = iconv(pool->from_wc_desc, &inbuf, &inbytesleft, &outbuf, &outbytesleft);

	buffer_pos = outbuf - buffer;

	if (rc == -1 && errno == E2BIG)
		goto grow_buffer_retry;

	if (rc == -1 && (errno == EILSEQ || errno == EINVAL)) {
		// just copy a '?' to the output stream
		if (outbytesleft < 1)
			goto grow_buffer_retry;
		*outbuf = '?';
		buffer_pos++;
		inbuf += sizeof(wchar_t);
		inbytesleft -= sizeof(wchar_t);
		goto retry_without_growing;
	}

	if (rc == -1) {
		free(buffer);
		return 0;
	}

	if (outbytesleft < 1)
		buffer = realloc(buffer, buffer_size+1);
	*outbuf = 0;

	return stfl_ipool_add(pool, buffer);
}

void stfl_ipool_flush(struct stfl_ipool *pool)
{
	if (!pool)
		return;

	struct stfl_ipool_entry *l;

	while (pool->list) {
		l = pool->list;
		pool->list = l->next;
		free(l->data);
		free(l);
	}
}

void stfl_ipool_destroy(struct stfl_ipool *pool)
{
	if (!pool)
		return;

	stfl_ipool_flush(pool);
	free(pool->code);

	if (pool->to_wc_desc != (iconv_t)(-1))
		iconv_close(pool->to_wc_desc);

	if (pool->from_wc_desc != (iconv_t)(-1))
		iconv_close(pool->from_wc_desc);

	free(pool);
}

