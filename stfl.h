/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006  Clifford Wolf <clifford@clifford.at>
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
 *  stfl.h: The STFL C header file
 */

#ifndef STFL_H
#define STFL_H 1

#include <wchar.h>

#ifdef  __cplusplus
extern "C" {
#endif

extern int stfl_api_allow_null_pointers;

struct stfl_form;
struct stfl_ipool;

extern struct stfl_form *stfl_create(const wchar_t *text);
extern void stfl_free(struct stfl_form *f);

extern const wchar_t *stfl_run(struct stfl_form *f, int timeout);
extern void stfl_reset();

extern const wchar_t * stfl_get(struct stfl_form *f, const wchar_t *name);
extern void stfl_set(struct stfl_form *f, const wchar_t *name, const wchar_t *value);

extern const wchar_t *stfl_get_focus(struct stfl_form *f);
extern void stfl_set_focus(struct stfl_form *f, const wchar_t *name);

extern const wchar_t *stfl_quote(const wchar_t *text);
extern const wchar_t *stfl_dump(struct stfl_form *f, const wchar_t *name, const wchar_t *prefix, int focus);

extern void stfl_modify(struct stfl_form *f, const wchar_t *name, const wchar_t *mode, const wchar_t *text);
extern const wchar_t *stfl_lookup(struct stfl_form *f, const wchar_t *path, const wchar_t *newname);

extern const wchar_t *stfl_error();
extern void stfl_error_action(const wchar_t *mode);

extern struct stfl_ipool *stfl_ipool_create(const char *code);
extern void *stfl_ipool_add(struct stfl_ipool *pool, void *data);
extern const wchar_t *stfl_ipool_towc(struct stfl_ipool *pool, const char *buf);
extern const char *stfl_ipool_fromwc(struct stfl_ipool *pool, const wchar_t *buf);
extern void stfl_ipool_flush(struct stfl_ipool *pool);
extern void stfl_ipool_destroy(struct stfl_ipool *pool);

#ifdef __cplusplus
}
#endif

#endif

