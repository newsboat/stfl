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
 *  public.c: Public STFL API
 */

#include "stfl_internals.h"

#include <stdlib.h>
#include <string.h>

int stfl_api_allow_null_pointers = 1;

static const char *checkret(const char *txt)
{
	if (!stfl_api_allow_null_pointers && !txt)
		return "";
	return txt;
}

struct stfl_form *stfl_create(const char *text)
{
	struct stfl_form *f = stfl_form_new();
	f->root = stfl_parser(text ? text : "");
	stfl_check_setfocus(f, f->root);
	return f;
}

void stfl_free(struct stfl_form *f)
{
	stfl_form_free(f);
}

const char *stfl_run(struct stfl_form *f, int timeout)
{
	stfl_form_run(f, timeout);
	return checkret(f->event);
}

void stfl_reset()
{
	stfl_form_reset();
}

const char *stfl_get(struct stfl_form *f, const char *name)
{
	char *pseudovar_sep = name ? strchr(name, ':') : 0;

	pthread_mutex_lock(&f->mtx);

	if (pseudovar_sep)
	{
		char w_name[pseudovar_sep-name+1];
		memcpy(w_name, name, pseudovar_sep-name);
		w_name[pseudovar_sep-name] = 0;

		struct stfl_widget *w = stfl_widget_by_name(f->root, w_name);
		static char ret_buffer[16];

		if (!strcmp(pseudovar_sep+1, "x")) {
			snprintf(ret_buffer, 16, "%d", w->x);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
		if (!strcmp(pseudovar_sep+1, "y")) {
			snprintf(ret_buffer, 16, "%d", w->y);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
		if (!strcmp(pseudovar_sep+1, "w")) {
			snprintf(ret_buffer, 16, "%d", w->w);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
		if (!strcmp(pseudovar_sep+1, "h")) {
			snprintf(ret_buffer, 16, "%d", w->h);
			return checkret(ret_buffer);
		}
		if (!strcmp(pseudovar_sep+1, "minw")) {
			snprintf(ret_buffer, 16, "%d", w->min_w);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
		if (!strcmp(pseudovar_sep+1, "minh")) {
			snprintf(ret_buffer, 16, "%d", w->min_h);
			pthread_mutex_unlock(&f->mtx);
			return checkret(ret_buffer);
		}
		pthread_mutex_unlock(&f->mtx);
		return checkret(0);
	}

	{
		const char * tmpstr = stfl_getkv_by_name_str(f->root, name ? name : "", 0);
		pthread_mutex_unlock(&f->mtx);
		return checkret(tmpstr);
	}
}

void stfl_set(struct stfl_form *f, const char *name, const char *value)
{
	pthread_mutex_lock(&f->mtx);
	stfl_setkv_by_name_str(f->root, name ? name : "", value ? value : "");
	pthread_mutex_unlock(&f->mtx);
}

const char *stfl_get_focus(struct stfl_form *f)
{
	struct stfl_widget *fw;
	const char * tmpstr;
	pthread_mutex_lock(&f->mtx);
	fw = stfl_widget_by_id(f->root, f->current_focus_id);
	tmpstr = checkret(fw ? fw->name : 0);
	pthread_mutex_unlock(&f->mtx);
	return tmpstr;
}

void stfl_set_focus(struct stfl_form *f, const char *name)
{
	struct stfl_widget *fw;
	pthread_mutex_lock(&f->mtx);
	fw = stfl_widget_by_name(f->root, name ? name : "");
	stfl_switch_focus(0, fw, f);
	pthread_mutex_unlock(&f->mtx);
}

const char *stfl_quote(const char *text)
{
	static char *last_ret = 0;
	if (last_ret)
		free(last_ret);
	last_ret = stfl_quote_backend(text ? text : "");
	return checkret(last_ret);
}

const char *stfl_dump(struct stfl_form *f, const char *name, const char *prefix, int focus)
{
	static char *last_ret = 0;
	struct stfl_widget *w;
	pthread_mutex_lock(&f->mtx);
	w = name && *name ? stfl_widget_by_name(f->root, name) : f->root;
	if (last_ret)
		free(last_ret);
	last_ret = stfl_widget_dump(w, prefix ? prefix : "", focus ? f->current_focus_id : 0);
	pthread_mutex_unlock(&f->mtx);
	return checkret(last_ret);
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

void stfl_modify(struct stfl_form *f, const char *name, const char *mode, const char *text)
{
	struct stfl_widget *n = stfl_parser(text ? text : "");
	struct stfl_widget *w;

	pthread_mutex_lock(&f->mtx);
	
	w = stfl_widget_by_name(f->root, name ? name : "");

	mode = mode ? mode : "";

	if (!strcmp(mode, "replace")) {
		stfl_modify_after(w, n);
		stfl_widget_free(w);
		goto finish;
	}

	if (!strcmp(mode, "replace_inner")) {
		while (w->first_child)
			stfl_widget_free(w->first_child);
		stfl_modify_insert(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!strcmp(mode, "insert")) {
		stfl_modify_insert(w, n);
		goto finish;
	}

	if (!strcmp(mode, "insert_inner")) {
		stfl_modify_insert(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!strcmp(mode, "append")) {
		stfl_modify_append(w, n);
		goto finish;
	}

	if (!strcmp(mode, "append_inner")) {
		stfl_modify_append(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!strcmp(mode, "before")) {
		stfl_modify_before(w, n);
		goto finish;
	}

	if (!strcmp(mode, "before_inner")) {
		stfl_modify_before(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!strcmp(mode, "after")) {
		stfl_modify_after(w, n);
		goto finish;
	}

	if (!strcmp(mode, "after_inner")) {
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

const char *stfl_lookup(struct stfl_form *f, const char *path, const char *newname)
{
	return checkret(0);
}

const char *stfl_error()
{
	abort();
	return checkret(0);
}

void stfl_error_action(const char *mode)
{
	abort();
	return;
}

