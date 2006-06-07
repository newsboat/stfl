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
 *  base.c: Core functions
 */

#define STFL_PRIVATE 1
#include "stfl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

struct stfl_widget_type *stfl_widget_type_list[] = {
	&stfl_widget_type_label,
	&stfl_widget_type_input,
	&stfl_widget_type_vbox,
	&stfl_widget_type_hbox,
	&stfl_widget_type_table,
	&stfl_widget_type_tablebr,
	0
};

int id_counter = 0;
int curses_active = 0;

struct stfl_widget *stfl_widget_new(const char *type)
{
	struct stfl_widget_type *t;
	int setfocus = 0;
	int i;

	while (*type == '!') {
		setfocus = 1;
		type++;
	}

	for (i=0; (t = stfl_widget_type_list[i]) != 0; i++)
		if (!strcmp(t->name, type))
			break;

	if (!t)
		return 0;

	struct stfl_widget *w = calloc(1, sizeof(struct stfl_widget));
	w->id = ++id_counter;
	w->type = t;
	w->setfocus = setfocus;
	if (w->type->f_init)
		w->type->f_init(w);
	return w;
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

	if (w->cls)
		free(w->cls);

	free(w);
}

extern struct stfl_kv *stfl_widget_setkv_int(struct stfl_widget *w, const char *key, int value)
{
	char newtext[64];
	snprintf(newtext, 64, "%d", value);
	return stfl_widget_setkv_str(w, key, newtext);
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

extern struct stfl_kv *stfl_setkv_by_name_int(struct stfl_widget *w, const char *name, int value)
{
	char newtext[64];
	snprintf(newtext, 64, "%d", value);
	return stfl_setkv_by_name_str(w, name, newtext);
}

extern struct stfl_kv *stfl_setkv_by_name_str(struct stfl_widget *w, const char *name, const char *value)
{
	struct stfl_kv *kv = stfl_kv_by_name(w, name);

	if (!kv)
		return 0;

	free(kv->value);
	kv->value = strdup(value);
	return kv;
}

static struct stfl_kv *stfl_widget_getkv_worker(struct stfl_widget *w, const char *key)
{
	struct stfl_kv *kv = w->kv_list;
	while (kv) {
		if (!strcmp(kv->key, key))
			return kv;
		kv = kv->next;
	}
	return 0;
}

struct stfl_kv *stfl_widget_getkv(struct stfl_widget *w, const char *key)
{
	struct stfl_kv *kv = stfl_widget_getkv_worker(w, key);
	if (kv) return kv;

	if (*key == '@')
	{
		if (strchr(key, '.') == 0 && strchr(key, '#') == 0)
		{
			int newkey_len = strlen(w->type->name) +
					strlen(w->cls ? w->cls : "") + strlen(key) + 2;
			char newkey[newkey_len];

			if (w->cls) {
				snprintf(newkey, newkey_len, "@%s#%s", w->cls, key+1);
				kv = stfl_widget_getkv(w, newkey);
				if (kv) return kv;
			}

			snprintf(newkey, newkey_len, "@%s.%s", w->type->name, key+1);
			kv = stfl_widget_getkv(w, newkey);
			if (kv) return kv;
		}

		while (w->parent) {
			w = w->parent;
			kv = stfl_widget_getkv(w, key);
			if (kv) return kv;
		}
	}

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

int stfl_getkv_by_name_int(struct stfl_widget *w, const char *name, int defval)
{
	struct stfl_kv *kv = stfl_kv_by_name(w, name);
	char *endptr;
	int ret;

	if (!kv || !kv->value[0])
		return defval;

	ret = strtol(kv->value, &endptr, 10);

	if (*endptr)
		return defval;

	return ret;
}

const char *stfl_getkv_by_name_str(struct stfl_widget *w, const char *name, const char *defval)
{
	struct stfl_kv *kv = stfl_kv_by_name(w, name);
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

struct stfl_widget *stfl_find_child_tree(struct stfl_widget *w, struct stfl_widget *c)
{
	while (c) {
		if (c->parent == w)
			return c;
		c = c->parent;
	}
	return 0;
}

extern struct stfl_widget *stfl_find_first_focusable(struct stfl_widget *w)
{
	if (w->allow_focus)
		return w;

	struct stfl_widget *c = w->first_child;
	while (c) {
		struct stfl_widget *r = stfl_find_first_focusable(c);
		if (r)
			return r;
		c = c->next_sibling;
	}

	return 0;
}

int stfl_focus_prev(struct stfl_widget *w, struct stfl_widget *old_fw, struct stfl_form *f)
{
	struct stfl_widget *stop = stfl_find_child_tree(w, old_fw);

	assert(stop);

	while (w->first_child != stop)
	{
		struct stfl_widget *c = w->first_child;
		while (c->next_sibling != stop)
			c = c->next_sibling;

		struct stfl_widget *new_fw = stfl_find_first_focusable(c);
		if (new_fw) {
			if (old_fw->type->f_leave)
				old_fw->type->f_leave(old_fw, f);

			if (new_fw->type->f_enter)
				new_fw->type->f_enter(new_fw, f);

			f->current_focus_id = new_fw->id;
			return 1;
		}

		stop = c;
	}

	return 0;
}

int stfl_focus_next(struct stfl_widget *w, struct stfl_widget *old_fw, struct stfl_form *f)
{
	struct stfl_widget *c = stfl_find_child_tree(w, old_fw);
	
	assert(c);
	c = c->next_sibling;

	while (c) {
		struct stfl_widget *new_fw = stfl_find_first_focusable(c);
		if (new_fw) {
			if (old_fw->type->f_leave)
				old_fw->type->f_leave(old_fw, f);

			if (new_fw->type->f_enter)
				new_fw->type->f_enter(new_fw, f);

			f->current_focus_id = new_fw->id;
			return 1;
		}
		c = c->next_sibling;
	}

	return 0;
}

int stfl_switch_focus(struct stfl_widget *old_fw, struct stfl_widget *new_fw, struct stfl_form *f)
{
	if (!new_fw || !new_fw->allow_focus)
		return 0;

	if (!old_fw && f->current_focus_id)
		old_fw = stfl_widget_by_id(f->root, f->current_focus_id);

	if (old_fw && old_fw->type->f_leave)
		old_fw->type->f_leave(old_fw, f);

	if (new_fw->type->f_enter)
		new_fw->type->f_enter(new_fw, f);

	f->current_focus_id = new_fw->id;
	return 1;
}

struct stfl_form *stfl_form_new()
{
	struct stfl_form *f = calloc(1, sizeof(struct stfl_form));
	return f;
}

int stfl_form_run(struct stfl_form *f, int timeout)
{
	if (f->event)
		free(f->event);
	f->event = 0;

	if (!f->root) {
		fprintf(stderr, "STFL Fatal Error: Called stfl_form_run() without root widget.\n");
		abort();
		return 0;
	}

	if (!curses_active)
	{
		initscr();
		cbreak();
		noecho();
		nonl();
		keypad(stdscr, TRUE);
		start_color();
		use_default_colors();
		curses_active = 1;
	}

	f->root->type->f_prepare(f->root, f);

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

		if (fw && fw->type->f_enter)
			fw->type->f_enter(fw, f);
	}

	f->current_focus_id = fw ? fw->id : 0;

	getbegyx(stdscr, f->root->y, f->root->x);
	getmaxyx(stdscr, f->root->h, f->root->w);

	werase(stdscr);
	f->root->type->f_draw(f->root, f, stdscr);
	refresh();

	wtimeout(stdscr, timeout <= 0 ? -1 : timeout);
	int ch = mvwgetch(stdscr, f->cursor_y, f->cursor_x);
	struct stfl_widget *w = fw;

	while (w) {
		if (w->type->f_process && w->type->f_process(w, fw, f, ch))
			return 0;
		w = w->parent;
	}

	if (ch == '\r' || ch == '\n') {
		f->event = strdup("ENTER");
		return 0;
	}

	if (ch == 27) {
		f->event = strdup("ESC");
		return 0;
	}

	if (KEY_F(0) <= ch && ch <= KEY_F(63)) {
		f->event = malloc(4);
		snprintf(f->event, 4, "F%d", ch - KEY_F0);
		return 0;
	}

	if (ch == '\t')
	{
		struct stfl_widget *old_fw = fw = stfl_widget_by_id(f->root, f->current_focus_id);
		do {
			if (fw->first_child)
				fw = fw->first_child;
			else
			if (fw->next_sibling)
				fw = fw->next_sibling;
			else
				fw = fw->parent ? fw->parent->next_sibling : 0;

			if (!fw && old_fw)
				fw = f->root;
		} while (fw && !fw->allow_focus);

		if (old_fw != fw)
		{
			if (old_fw && old_fw->type->f_leave)
				old_fw->type->f_leave(old_fw, f);

			if (fw->type->f_enter)
				fw->type->f_enter(fw, f);

			f->current_focus_id = fw ? fw->id : 0;
		}

		return 0;
	}

#if 0
	fprintf(stderr, ">> Unhandled input char: %d 0%o 0x%x\n", ch, ch, ch);
#endif

	return 1;
}

void stfl_form_return()
{
	if (curses_active) {
		endwin();
		curses_active = 0;
	}
}

void stfl_form_free(struct stfl_form *f)
{
	if (f->root)
		stfl_widget_free(f->root);
	free(f);
}

void stfl_check_setfocus(struct stfl_form *f, struct stfl_widget *w)
{
	if (w->setfocus) {
		f->current_focus_id = w->id;
		w->setfocus = 0;
	}

	w = w->first_child;
	while (w) {
		stfl_check_setfocus(f, w);
		w = w->next_sibling;
	}
}

