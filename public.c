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
 *  public.c: Public STFL API
 */

#include "stfl_internals.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>

int stfl_api_allow_null_pointers = 1;

static const wchar_t *checkret(const wchar_t *txt)
{
	if (!stfl_api_allow_null_pointers && !txt)
		return L"";
	return txt;
}

struct stfl_form *stfl_create(const wchar_t *text)
{
	struct stfl_form *f = stfl_form_new();
	f->root = stfl_parser(text ? text : L"");
	stfl_check_setfocus(f, f->root);
	return f;
}

void stfl_free(struct stfl_form *f)
{
	stfl_form_free(f);
}

const wchar_t *stfl_run(struct stfl_form *f, int timeout)
{
	stfl_form_run(f, timeout);
	return checkret(f->event);
}

void stfl_reset()
{
	stfl_form_reset();
}

const wchar_t *stfl_get(struct stfl_form *f, const wchar_t *name)
{
	wchar_t *pseudovar_sep = name ? wcschr(name, L':') : 0;

	pthread_mutex_lock(&f->mtx);

	if (pseudovar_sep)
	{
		wchar_t w_name[pseudovar_sep-name+1];
		wmemcpy(w_name, name, pseudovar_sep-name);
		w_name[pseudovar_sep-name] = 0;

		struct stfl_widget *w = stfl_widget_by_name(f->root, w_name);
		static wchar_t ret_buffer[16];

		if (w == 0)
			goto this_is_not_a_pseudo_var;

		if (!wcscmp(pseudovar_sep+1, L"x")) {
			swprintf(ret_buffer, 16, L"%d", w->x);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
		if (!wcscmp(pseudovar_sep+1, L"y")) {
			swprintf(ret_buffer, 16, L"%d", w->y);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
		if (!wcscmp(pseudovar_sep+1, L"w")) {
			swprintf(ret_buffer, 16, L"%d", w->w);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
		if (!wcscmp(pseudovar_sep+1, L"h")) {
			swprintf(ret_buffer, 16, L"%d", w->h);
			return checkret(ret_buffer);
		}
		if (!wcscmp(pseudovar_sep+1, L"minw")) {
			swprintf(ret_buffer, 16, L"%d", w->min_w);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
		if (!wcscmp(pseudovar_sep+1, L"minh")) {
			swprintf(ret_buffer, 16, L"%d", w->min_h);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
	}

this_is_not_a_pseudo_var:;
	const wchar_t * tmpstr = stfl_getkv_by_name_str(f->root, name ? name : L"", 0);
	pthread_mutex_unlock(&f->mtx);
	return checkret(tmpstr);
}

void stfl_set(struct stfl_form *f, const wchar_t *name, const wchar_t *value)
{
	pthread_mutex_lock(&f->mtx);
	stfl_setkv_by_name_str(f->root, name ? name : L"", value ? value : L"");
	pthread_mutex_unlock(&f->mtx);
}

const wchar_t *stfl_get_focus(struct stfl_form *f)
{
	struct stfl_widget *fw;
	const wchar_t * tmpstr;
	pthread_mutex_lock(&f->mtx);
	fw = stfl_widget_by_id(f->root, f->current_focus_id);
	tmpstr = checkret(fw ? fw->name : 0);
	pthread_mutex_unlock(&f->mtx);
	return tmpstr;
}

void stfl_set_focus(struct stfl_form *f, const wchar_t *name)
{
	struct stfl_widget *fw;
	pthread_mutex_lock(&f->mtx);
	fw = stfl_widget_by_name(f->root, name ? name : L"");
	stfl_switch_focus(0, fw, f);
	pthread_mutex_unlock(&f->mtx);
}

const wchar_t *stfl_quote(const wchar_t *text)
{
	static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
	static pthread_key_t retbuffer_key;
	static int firstrun = 1;
	static wchar_t *retbuffer = 0;

	pthread_mutex_lock(&mtx);

	if (firstrun) {
		pthread_key_create(&retbuffer_key, free);
		firstrun = 0;
	}

	retbuffer = pthread_getspecific(retbuffer_key);

	if (retbuffer)
		free(retbuffer);

	retbuffer = stfl_quote_backend(text ? text : L"");

	pthread_setspecific(retbuffer_key, retbuffer);

	pthread_mutex_unlock(&mtx);
	return checkret(retbuffer);
}

const wchar_t *stfl_dump(struct stfl_form *f, const wchar_t *name, const wchar_t *prefix, int focus)
{
	static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
	static pthread_key_t retbuffer_key;
	static int firstrun = 1;
	static wchar_t *retbuffer = 0;
	struct stfl_widget *w;

	pthread_mutex_lock(&mtx);
	pthread_mutex_lock(&f->mtx);

	if (firstrun) {
		pthread_key_create(&retbuffer_key, free);
		firstrun = 0;
	}

	retbuffer = pthread_getspecific(retbuffer_key);

	if (retbuffer)
		free(retbuffer);

	w = name && *name ? stfl_widget_by_name(f->root, name) : f->root;
	retbuffer = stfl_widget_dump(w, prefix ? prefix : L"", focus ? f->current_focus_id : 0);

	pthread_setspecific(retbuffer_key, retbuffer);

	pthread_mutex_unlock(&f->mtx);
	pthread_mutex_unlock(&mtx);

	return checkret(retbuffer);
}

static void stfl_modify_before(struct stfl_widget *w, struct stfl_widget *n)
{
	if (!n || !w || !w->parent)
		return;

	struct stfl_widget **prev_p = &w->parent->first_child;
	struct stfl_widget *last_n = 0;

	while (*prev_p != w)
		prev_p = &(*prev_p)->next_sibling;

	*prev_p = n;
	while (n) {
		last_n = n;
		n->parent = w->parent;
		n = n->next_sibling;
	}

	last_n->next_sibling = w;
}

static void stfl_modify_after(struct stfl_widget *w, struct stfl_widget *n)
{
	if (!n || !w || !w->parent)
		return;

	struct stfl_widget *first_n = n;
	struct stfl_widget *last_n = 0;

	while (n) {
		last_n = n;
		n->parent = w->parent;
		n = n->next_sibling;
	}

	if (w->next_sibling)
		last_n->next_sibling = w->next_sibling;
	else
		w->parent->last_child = last_n;

	w->next_sibling = first_n;
}

static void stfl_modify_insert(struct stfl_widget *w, struct stfl_widget *n)
{
	if (!n || !w)
		return;

	struct stfl_widget *first_n = n;
	struct stfl_widget *last_n = 0;

	while (n) {
		last_n = n;
		n->parent = w;
		n = n->next_sibling;
	}

	if (w->first_child)
		last_n->next_sibling = w->first_child;
	else
		w->last_child = last_n;

	w->first_child = first_n;
}

static void stfl_modify_append(struct stfl_widget *w, struct stfl_widget *n)
{
	if (!n || !w)
		return;

	struct stfl_widget *first_n = n;
	struct stfl_widget *last_n = 0;

	while (n) {
		last_n = n;
		n->parent = w;
		n = n->next_sibling;
	}

	if (w->last_child)
		w->last_child->next_sibling = first_n;
	else
		w->first_child = first_n;

	w->last_child = last_n;
}

void stfl_modify(struct stfl_form *f, const wchar_t *name, const wchar_t *mode, const wchar_t *text)
{
	struct stfl_widget *n = stfl_parser(text ? text : L"");
	struct stfl_widget *w;

	pthread_mutex_lock(&f->mtx);
	
	w = stfl_widget_by_name(f->root, name ? name : L"");

	mode = mode ? mode : L"";

	if (!wcscmp(mode, L"replace")) {
		stfl_modify_after(w, n);
		stfl_widget_free(w);
		goto finish;
	}

	if (!wcscmp(mode, L"replace_inner")) {
		while (w->first_child)
			stfl_widget_free(w->first_child);
		stfl_modify_insert(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!wcscmp(mode, L"insert")) {
		stfl_modify_insert(w, n);
		goto finish;
	}

	if (!wcscmp(mode, L"insert_inner")) {
		stfl_modify_insert(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!wcscmp(mode, L"append")) {
		stfl_modify_append(w, n);
		goto finish;
	}

	if (!wcscmp(mode, L"append_inner")) {
		stfl_modify_append(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!wcscmp(mode, L"before")) {
		stfl_modify_before(w, n);
		goto finish;
	}

	if (!wcscmp(mode, L"before_inner")) {
		stfl_modify_before(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!wcscmp(mode, L"after")) {
		stfl_modify_after(w, n);
		goto finish;
	}

	if (!wcscmp(mode, L"after_inner")) {
		stfl_modify_after(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

finish:
	stfl_check_setfocus(f, f->root);
	pthread_mutex_unlock(&f->mtx);
	return;
}

const wchar_t *stfl_lookup(struct stfl_form *f, const wchar_t *path, const wchar_t *newname)
{
	return checkret(0);
}

const wchar_t *stfl_error()
{
	abort();
	return checkret(0);
}

void stfl_error_action(const wchar_t *mode)
{
	abort();
	return;
}

