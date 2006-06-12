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

struct stfl_form *stfl_create(const char *text) {
	struct stfl_form *f = stfl_form_new();
	f->root = stfl_parser(text ? text : "");
	stfl_check_setfocus(f, f->root);
	return f;
}

void stfl_free(struct stfl_form *f) {
	stfl_form_free(f);
}

const char *stfl_run(struct stfl_form *f, int timeout) {
	stfl_form_run(f, timeout);
	return f->event;
}

void stfl_return() {
	stfl_form_return();
}

const char *stfl_get(struct stfl_form *f, const char *name) {
	return stfl_getkv_by_name_str(f->root, name ? name : "", 0);
}

void stfl_set(struct stfl_form *f, const char *name, const char *value) {
	stfl_setkv_by_name_str(f->root, name ? name : "", value ? value : "");
}

const char *stfl_get_focus(struct stfl_form *f) {
	struct stfl_widget *fw = stfl_widget_by_id(f->root, f->current_focus_id);
	return fw ? fw->name : 0;
}

void stfl_set_focus(struct stfl_form *f, const char *name) {
	struct stfl_widget *fw = stfl_widget_by_name(f->root, name ? name : "");
	stfl_switch_focus(0, fw, f);
}

const char *stfl_quote(const char *text)
{
	static char *last_ret = 0;
	if (last_ret)
		free(last_ret);
	last_ret = stfl_quote_backend(text ? text : "");
	return last_ret;
}

const char *stfl_dump(struct stfl_form *f, const char *name, const char *prefix, int focus)
{
	static char *last_ret = 0;
	struct stfl_widget *w;
	w = name && *name ? stfl_widget_by_name(f->root, name) : f->root;
	if (last_ret)
		free(last_ret);
	last_ret = stfl_widget_dump(w, prefix ? prefix : "", focus ? f->current_focus_id : 0);
	return last_ret;
}

static void stfl_import_before(struct stfl_widget *w, struct stfl_widget *n)
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

static void stfl_import_after(struct stfl_widget *w, struct stfl_widget *n)
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

static void stfl_import_insert(struct stfl_widget *w, struct stfl_widget *n)
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

static void stfl_import_append(struct stfl_widget *w, struct stfl_widget *n)
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

void stfl_import(struct stfl_form *f, const char *name, const char *mode, const char *text)
{
	struct stfl_widget *w = stfl_widget_by_name(f->root, name ? name : "");
	struct stfl_widget *n = stfl_parser(text ? text : "");

	mode = mode ? mode : "";

	if (!strcmp(mode, "replace")) {
		stfl_import_after(w, n);
		stfl_widget_free(w);
		goto finish;
	}

	if (!strcmp(mode, "replace_inner")) {
		while (w->first_child)
			stfl_widget_free(w->first_child);
		stfl_import_insert(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!strcmp(mode, "insert")) {
		stfl_import_insert(w, n);
		goto finish;
	}

	if (!strcmp(mode, "insert_inner")) {
		stfl_import_insert(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!strcmp(mode, "append")) {
		stfl_import_append(w, n);
		goto finish;
	}

	if (!strcmp(mode, "append_inner")) {
		stfl_import_append(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!strcmp(mode, "before")) {
		stfl_import_before(w, n);
		goto finish;
	}

	if (!strcmp(mode, "before_inner")) {
		stfl_import_before(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

	if (!strcmp(mode, "after")) {
		stfl_import_after(w, n);
		goto finish;
	}

	if (!strcmp(mode, "after_inner")) {
		stfl_import_after(w, n->first_child);
		n->first_child = n->last_child = 0;
		stfl_widget_free(n);
		goto finish;
	}

finish:
	stfl_check_setfocus(f, f->root);
	return;
}

const char *stfl_lookup(struct stfl_form *f, const char *path, const char *newname)
{
	return 0;
}

const char *stfl_error()
{
	abort();
	return 0;
}

void stfl_error_action(const char *mode)
{
	abort();
	return;
}

