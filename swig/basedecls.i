/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006, 2007  Clifford Wolf <clifford@clifford.at>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *  
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *
 *  basedecls.i: SWIG bindings core
 */

%module stfl

%{

#include <stdlib.h>
#include <wchar.h>
#include "stfl.h"

typedef struct stfl_form stfl_form;

struct stfl_ipool *ipool = 0;

static void inline ipool_reset() {
	if (!ipool)
		ipool = stfl_ipool_create("UTF8");
	stfl_ipool_flush(ipool);
}

static void ipool_destroy() {
	stfl_ipool_destroy(ipool);
	ipool = 0;
}

#define TOWC(_t) stfl_ipool_towc(ipool, _t)
#define FROMWC(_t) stfl_ipool_fromwc(ipool, _t)

%}

typedef struct {
} stfl_form;

%extend stfl_form
{
	stfl_form(char *text) {
		ipool_reset();
		return stfl_create(TOWC(text));
	}
	~stfl_form() {
		ipool_reset();
		stfl_free(self);
	}
	const char *run(int timeout) {
		ipool_reset();
		return FROMWC(stfl_run(self, timeout));
	}
	const char *get(const char *name) {
		ipool_reset();
		return FROMWC(stfl_get(self, TOWC(name)));
	}
	void set(const char *name, const char *value) {
		ipool_reset();
		return stfl_set(self, TOWC(name), TOWC(value));
	}
	const char *get_focus() {
		ipool_reset();
		return FROMWC(stfl_get_focus(self));
	}
	void set_focus(const char *name) {
		ipool_reset();
		stfl_set_focus(self, TOWC(name));
	}
	const char *dump(const char *name, const char *prefix, int focus) {
		ipool_reset();
		return FROMWC(stfl_dump(self, TOWC(name), TOWC(prefix), focus));
	}
	void modify(const char *name, const char *mode, const char *text) {
		ipool_reset();
		stfl_modify(self, TOWC(name), TOWC(mode), TOWC(text));
	}
	const char *lookup(const char *path, const char *newname) {
		ipool_reset();
		return FROMWC(stfl_lookup(self, TOWC(path), TOWC(newname)));
	}
}

%{

static struct stfl_form *stfl_create_wrapper(const char *text)
{
	ipool_reset();
	return stfl_create(TOWC(text));
}

static const char *stfl_run_wrapper(struct stfl_form *f, int timeout)
{
	ipool_reset();
	return FROMWC(stfl_run(f, timeout));
}

static const char *stfl_get_wrapper(struct stfl_form *f, const char *name)
{
	ipool_reset();
	return FROMWC(stfl_get(f, TOWC(name)));
}

static void stfl_set_wrapper(struct stfl_form *f, const char *name, const char *value)
{
	ipool_reset();
	return stfl_set(f, TOWC(name), TOWC(value));
}

static const char *stfl_get_focus_wrapper(struct stfl_form *f)
{
	ipool_reset();
	return FROMWC(stfl_get_focus(f));
}

static void stfl_set_focus_wrapper(struct stfl_form *f, const char *name)
{
	ipool_reset();
	stfl_set_focus(f, TOWC(name));
}

static const char *stfl_quote_wrapper(const char *text)
{
	ipool_reset();
	return FROMWC(stfl_quote(TOWC(text)));
}

static const char *stfl_dump_wrapper(struct stfl_form *f, const char *name, const char *prefix, int focus)
{
	ipool_reset();
	return FROMWC(stfl_dump(f, TOWC(name), TOWC(prefix), focus));
}

static void stfl_modify_wrapper(struct stfl_form *f, const char *name, const char *mode, const char *text)
{
	ipool_reset();
	stfl_modify(f, TOWC(name), TOWC(mode), TOWC(text));
}

static const char *stfl_lookup_wrapper(struct stfl_form *f, const char *path, const char *newname)
{
	ipool_reset();
	return FROMWC(stfl_lookup(f, TOWC(path), TOWC(newname)));
}

static const char *stfl_error_wrapper()
{
	ipool_reset();
	return FROMWC(stfl_error());
}

static void stfl_error_action_wrapper(const char *mode)
{
	ipool_reset();
	stfl_error_action(TOWC(mode));
}

%}

static struct stfl_form *stfl_create_wrapper(const char *text);
static const char *stfl_run_wrapper(struct stfl_form *f, int timeout);
static const char *stfl_get_wrapper(struct stfl_form *f, const char *name);
static void stfl_set_wrapper(struct stfl_form *f, const char *name, const char *value);
static const char *stfl_get_focus_wrapper(struct stfl_form *f);
static void stfl_set_focus_wrapper(struct stfl_form *f, const char *name);
static const char *stfl_quote_wrapper(const char *text);
static const char *stfl_dump_wrapper(struct stfl_form *f, const char *name, const char *prefix, int focus);
static void stfl_modify_wrapper(struct stfl_form *f, const char *name, const char *mode, const char *text);
static const char *stfl_lookup_wrapper(struct stfl_form *f, const char *path, const char *newname);
static const char *stfl_error_wrapper();
static void stfl_error_action_wrapper(const char *mode);
extern void stfl_reset();

%init %{
	atexit(stfl_reset);
	atexit(ipool_destroy);
%}

