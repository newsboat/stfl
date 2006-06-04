/*
 *  STFL - The Simple Terminal Forms Library
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
 *  base.c: Core functions
 */

#include "stfl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct stfl_widget_type *stfl_widget_type_list[] = {
	&stfl_widget_type_label,
	&stfl_widget_type_input,
	&stfl_widget_type_vbox,
	&stfl_widget_type_hbox,
	0
};

static int id_counter = 0;

struct stfl_widget *stfl_widget_new(const char *type)
{
	struct stfl_widget_type *t;
	int i;

	for (i=0; (t = stfl_widget_type_list[i]) != 0; i++)
		if (!strcmp(t->name, type))
			break;

	if (!t)
		return 0;

	struct stfl_widget *w = calloc(1, sizeof(struct stfl_widget));
	w->id = ++id_counter;
	w->type = t;
	if (w->type->f_init)
		w->type->f_init(w);
	return w;
}

struct stfl_widget *stfl_widget_copy(struct stfl_widget *w)
{
	fprintf(stderr, "STFL Fatal Error: stfl_widget_copy() is not implemeted.\n");
	abort();
}

void stfl_widget_free(struct stfl_widget *w)
{
	while (w->first_child)
		stfl_widget_free(w->first_child);

	if (w->type->f_done)
		w->type->f_done(w);

	struct stfl_kv *kv = w->kv_list;
	while (kv) {
		struct stfl_kv *next = kv->next;
		free(kv->key);
		free(kv->value);
		if (kv->name)
			free(kv->name);
		free(kv);
		kv = next;
	}

	if (w->parent)
	{
		struct stfl_widget **pp = &w->parent->first_child;
		while (*pp != w) {
			pp = &(*pp)->next_sibling;
		}
		*pp = w->next_sibling;

		if (w->parent->last_child == w) {
			struct stfl_widget *p = w->parent->first_child;
			w->parent->last_child = 0;
			while (p) {
				w->parent->last_child = p;
				p = p->next_sibling;
			}
		}
	}

	if (w->name)
		free(w->name);

	free(w);
}

struct stfl_kv *stfl_widget_setkv_str(struct stfl_widget *w, const char *key, const char *value)
{
	struct stfl_kv *kv = w->kv_list;
	while (kv) {
		if (!strcmp(kv->key, key)) {
			free(kv->value);
			kv->value = strdup(value);
			return kv;
		}
		kv = kv->next;
	}

	kv = calloc(1, sizeof(struct stfl_kv));
	kv->widget = w;
	kv->key = strdup(key);
	kv->value = strdup(value);
	kv->id = ++id_counter;
	kv->next = w->kv_list;
	w->kv_list = kv;
	return kv;
}

extern struct stfl_kv *stfl_widget_setkv_int(struct stfl_widget *w, const char *key, int value)
{
	char newtext[64];
	snprintf(newtext, 64, "%d", value);
	return stfl_widget_setkv_str(w, key, newtext);
}

struct stfl_kv *stfl_widget_getkv(struct stfl_widget *w, const char *key)
{
	struct stfl_kv *kv = w->kv_list;
	while (kv) {
		if (!strcmp(kv->key, key))
			return kv;
		kv = kv->next;
	}

	if (*key == '$' && w->parent)
		return stfl_widget_getkv(w->parent, key);

	return 0;
}

int stfl_widget_getkv_int(struct stfl_widget *w, const char *key, int defval)
{
	struct stfl_kv *kv = stfl_widget_getkv(w, key);
	char *endptr;
	int ret;

	if (!kv || !kv->value[0])
		return defval;

	ret = strtol(kv->value, &endptr, 10);

	if (*endptr)
		return defval;

	return ret;
}

const char *stfl_widget_getkv_str(struct stfl_widget *w, const char *key, const char *defval)
{
	struct stfl_kv *kv = stfl_widget_getkv(w, key);
	return kv ? kv->value : defval;
}

struct stfl_widget *stfl_widget_by_name(struct stfl_widget *w, const char *name)
{
	if (w->name && !strcmp(w->name, name))
		return w;

	w = w->first_child;
	while (w) {
		struct stfl_widget *r = stfl_widget_by_name(w, name);
		if (r) return r;
		w = w->next_sibling;
	}

	return 0;
}

struct stfl_widget *stfl_widget_by_id(struct stfl_widget *w, int id)
{
	if (w->id == id)
		return w;

	w = w->first_child;
	while (w) {
		struct stfl_widget *r = stfl_widget_by_id(w, id);
		if (r) return r;
		w = w->next_sibling;
	}

	return 0;
}

struct stfl_kv *stfl_kv_by_name(struct stfl_widget *w, const char *name)
{
	struct stfl_kv *kv = w->kv_list;
	while (kv) {
		if (kv->name && !strcmp(kv->name, name))
			return kv;
		kv = kv->next;
	}

	w = w->first_child;
	while (w) {
		struct stfl_kv *r = stfl_kv_by_name(w, name);
		if (r) return r;
		w = w->next_sibling;
	}

	return 0;
}

struct stfl_kv *stfl_kv_by_id(struct stfl_widget *w, int id)
{
	struct stfl_kv *kv = w->kv_list;
	while (kv) {
		if (kv->id == id)
			return kv;
		kv = kv->next;
	}

	w = w->first_child;
	while (w) {
		struct stfl_kv *r = stfl_kv_by_id(w, id);
		if (r) return r;
		w = w->next_sibling;
	}

	return 0;
}

int stfl_core_events(struct stfl_widget *w, struct stfl_form *f, WINDOW *win, int ch)
{
	if (ch == '\r' || ch == '\n') {
		f->event_type = STFL_EVENT_KEY_ENTER;
		return 1;
	}

	if (ch == 27) {
		f->event_type = STFL_EVENT_KEY_ESC;
		return 1;
	}

	if (KEY_F(0) <= ch && ch <= KEY_F(63)) {
		f->event_type = STFL_EVENT_KEY_FN;
		f->event_value = ch - KEY_F0;
		return 1;
	}

	if (ch == '\t') {
		f->event_type = STFL_EVENT_NEXT_TAB;
		return 1;
	}

	if (ch == KEY_LEFT) {
		f->event_type = STFL_EVENT_NEXT_LEFT;
		return 1;
	}

	if (ch == KEY_RIGHT) {
		f->event_type = STFL_EVENT_NEXT_RIGHT;
		return 1;
	}

	if (ch == KEY_UP) {
		f->event_type = STFL_EVENT_NEXT_UP;
		return 1;
	}

	if (ch == KEY_DOWN) {
		f->event_type = STFL_EVENT_NEXT_DOWN;
		return 1;
	}

	return 0;
}

struct stfl_form *stfl_form_new()
{
	struct stfl_form *f = calloc(1, sizeof(struct stfl_form));
	return f;
}

void stfl_form_run(struct stfl_form *f, WINDOW *win)
{
	f->event_type = STFL_EVENT_NONE;
	f->event_value = 0;

	if (!f->root)
		return;

	f->root->type->f_getminwh(f->root);

	getbegyx(win, f->root->y, f->root->x);
	getmaxyx(win, f->root->h, f->root->w);

	werase(win);
	f->root->type->f_draw(f->root, win);
	refresh();

	struct stfl_widget *fw = stfl_widget_by_id(f->root, f->current_focus_id);

	if (fw == 0)
	{
		fw = f->root;
		while (fw)
		{
			if (fw->allow_focus)
				break;

			if (fw->first_child)
				fw = fw->first_child;
			else
			if (fw->next_sibling)
				fw = fw->next_sibling;
			else
				fw = fw->parent ? fw->parent->next_sibling : 0;
		}
	}

	if (fw != 0) {
		fw->type->f_run(fw, f, win);
		f->current_focus_id = fw->id;
	}
}

void stfl_form_free(struct stfl_form *f)
{
	if (f->root)
		stfl_widget_free(f->root);
	free(f);
}

