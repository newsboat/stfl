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
 *  stfl.i: The STFL SWIG bindings
 */

%module stfl
%{
#include <stdlib.h>
#include "stfl.h"
typedef struct stfl_form stfl_form;
%}

typedef struct {
} stfl_form;

%extend stfl_form
{
	stfl_form(char *text) {
		return stfl_create(text);
	}
	~stfl_form() {
		stfl_free(self);
	}
	const char *run(int timeout) {
		return stfl_run(self, timeout);
	}
	const char *stfl_get(const char *name) {
		return stfl_get(self, name);
	}
	void stfl_set(const char *name, const char *value) {
		return stfl_set(self, name, value);
	}
	const char *get_focus() {
		return stfl_get_focus(self);
	}
	void set_focus(const char *name) {
		stfl_set_focus(self, name);
	}
	const char *dump(const char *name, const char *prefix, int focus) {
		return stfl_dump(self, name, prefix, focus);
	}
	void modify(const char *name, const char *mode, const char *text) {
		stfl_modify(self, name, mode, text);
	}
	const char *lookup(const char *path, const char *newname) {
		return stfl_lookup(self, path, newname);
	}
}

extern struct stfl_form *stfl_create(const char *text);
// extern void stfl_free(struct stfl_form *f);

extern const char *stfl_run(struct stfl_form *f, int timeout);
extern void stfl_reset();

extern const char *stfl_get(struct stfl_form *f, const char *name);
extern void stfl_set(struct stfl_form *f, const char *name, const char *value);

extern const char *stfl_get_focus(struct stfl_form *f);
extern void stfl_set_focus(struct stfl_form *f, const char *name);

extern const char *stfl_quote(const char *text);
extern const char *stfl_dump(struct stfl_form *f, const char *name, const char *prefix, int focus);
extern void stfl_modify(struct stfl_form *f, const char *name, const char *mode, const char *text);
extern const char *stfl_lookup(struct stfl_form *f, const char *path, const char *newname);

extern const char *stfl_error();
extern void stfl_error_action(const char *mode);

%init %{
	atexit(stfl_reset);
%}

