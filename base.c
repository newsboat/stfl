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

#include "stfl_internals.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <wchar.h>

struct stfl_widget_type *stfl_widget_types[] = {
	&stfl_widget_type_label,
	&stfl_widget_type_input,
	&stfl_widget_type_vbox,
	&stfl_widget_type_hbox,
	&stfl_widget_type_table,
	&stfl_widget_type_tablebr,
	&stfl_widget_type_list,
	&stfl_widget_type_listitem,
	&stfl_widget_type_textview,
	0
};

int id_counter = 0;
int curses_active = 0;

struct stfl_widget *stfl_widget_new(const wchar_t *type)
{
	struct stfl_widget_type *t;
	int setfocus = 0;
	int i;

	while (*type == '!') {
		setfocus = 1;
		type++;
	}

	for (i=0; (t = stfl_widget_types[i]) != 0; i++)
		if (!wcscmp(t->name, type))
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

extern struct stfl_kv *stfl_widget_setkv_int(struct stfl_widget *w, const wchar_t *key, int value)
{
	wchar_t newtext[64];
	swprintf(newtext, 64, L"%d", value);
	return stfl_widget_setkv_str(w, key, newtext);
}

struct stfl_kv *stfl_widget_setkv_str(struct stfl_widget *w, const wchar_t *key, const wchar_t *value)
{
	struct stfl_kv *kv = w->kv_list;
	while (kv) {
		if (!wcscmp(kv->key, key)) {
			free(kv->value);
			kv->value = wcsdup(value);
			return kv;
		}
		kv = kv->next;
	}

	kv = calloc(1, sizeof(struct stfl_kv));
	kv->widget = w;
	kv->key = wcsdup(key);
	kv->value = wcsdup(value);
	kv->id = ++id_counter;
	kv->next = w->kv_list;
	w->kv_list = kv;
	return kv;
}

extern struct stfl_kv *stfl_setkv_by_name_int(struct stfl_widget *w, const wchar_t *name, int value)
{
	wchar_t newtext[64];
	swprintf(newtext, 64, L"%d", value);
	return stfl_setkv_by_name_str(w, name, newtext);
}

extern struct stfl_kv *stfl_setkv_by_name_str(struct stfl_widget *w, const wchar_t *name, const wchar_t *value)
{
	struct stfl_kv *kv = stfl_kv_by_name(w, name);

	if (!kv)
		return 0;

	free(kv->value);
	kv->value = wcsdup(value);
	return kv;
}

static struct stfl_kv *stfl_widget_getkv_worker(struct stfl_widget *w, const wchar_t *key)
{
	struct stfl_kv *kv = w->kv_list;
	while (kv) {
		if (!wcscmp(kv->key, key))
			return kv;
		kv = kv->next;
	}
	return 0;
}

struct stfl_kv *stfl_widget_getkv(struct stfl_widget *w, const wchar_t *key)
{
	struct stfl_kv *kv = stfl_widget_getkv_worker(w, key);
	if (kv) return kv;

	int key1_len = wcslen(key) + 2;
	wchar_t key1[key1_len];

	int key2_len = key1_len + wcslen(w->type->name) + 1;
	wchar_t key2[key2_len];

	int key3_len = w->cls ? key1_len + wcslen(w->cls) + 1 : 0;
	wchar_t key3[key3_len];

	swprintf(key1, key1_len, L"@%ls", key);
	swprintf(key2, key2_len, L"@%ls#%ls", w->type->name, key);

	if (key3_len)
		swprintf(key3, key3_len, L"@%ls#%ls", w->cls, key);

	while (w)
	{
		if (key3_len) {
			kv = stfl_widget_getkv_worker(w, key3);
			if (kv) return kv;
		}

		kv = stfl_widget_getkv_worker(w, key2);
		if (kv) return kv;

		kv = stfl_widget_getkv_worker(w, key1);
		if (kv) return kv;

		w = w->parent;
	}

	return 0;
}

int stfl_widget_getkv_int(struct stfl_widget *w, const wchar_t *key, int defval)
{
	struct stfl_kv *kv = stfl_widget_getkv(w, key);
	int ret;

	if (!kv || !kv->value[0])
		return defval;

	if (swscanf(kv->value,L"%d",&ret) < 1)
		return defval;

	return ret;
}

const wchar_t *stfl_widget_getkv_str(struct stfl_widget *w, const wchar_t *key, const wchar_t *defval)
{
	struct stfl_kv *kv = stfl_widget_getkv(w, key);
	return kv ? kv->value : defval;
}

int stfl_getkv_by_name_int(struct stfl_widget *w, const wchar_t *name, int defval)
{
	struct stfl_kv *kv = stfl_kv_by_name(w, name);
	int ret;

	if (!kv || !kv->value[0])
		return defval;

	if (swscanf(kv->value,L"%d",&ret) < 1)
		return defval;

	return ret;
}

const wchar_t *stfl_getkv_by_name_str(struct stfl_widget *w, const wchar_t *name, const wchar_t *defval)
{
	struct stfl_kv *kv = stfl_kv_by_name(w, name);
	return kv ? kv->value : defval;
}

struct stfl_widget *stfl_widget_by_name(struct stfl_widget *w, const wchar_t *name)
{
	if (w->name && !wcscmp(w->name, name))
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

struct stfl_kv *stfl_kv_by_name(struct stfl_widget *w, const wchar_t *name)
{
	struct stfl_kv *kv = w->kv_list;
	while (kv) {
		if (kv->name && !wcscmp(kv->name, name))
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
	if (f) {
		pthread_mutex_init(&f->mtx, NULL);
	}
	return f;
}

void stfl_form_event(struct stfl_form *f, wchar_t *event)
{
	struct stfl_event **ep = &f->event_queue;
	struct stfl_event *e = calloc(1, sizeof(struct stfl_event));
	e->event = event;
	while (*ep)
		ep = &(*ep)->next;
	*ep = e;
}

static struct stfl_widget* stfl_gather_focus_widget(struct stfl_form* f) {
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
	return fw;
}

void stfl_form_run(struct stfl_form *f, int timeout)
{
	pthread_mutex_lock(&f->mtx);

	if (f->event)
		free(f->event);
	f->event = 0;

	if (timeout >= 0 && f->event_queue)
		goto unshift_next_event;

	if (!f->root) {
		fprintf(stderr, "STFL Fatal Error: Called stfl_form_run() without root widget.\n");
		abort();
	}

	if (!curses_active)
	{
		initscr();
		cbreak();
		noecho();
		nonl();
		keypad(stdscr, TRUE);
		doupdate();
		start_color();
		use_default_colors();
		wbkgdset(stdscr, ' ');
		curses_active = 1;
	}

	f->root->type->f_prepare(f->root, f);

	struct stfl_widget *fw = stfl_gather_focus_widget(f);
	f->current_focus_id = fw ? fw->id : 0;

	getbegyx(stdscr, f->root->y, f->root->x);
	getmaxyx(stdscr, f->root->h, f->root->w);

	werase(stdscr);
	f->root->type->f_draw(f->root, f, stdscr);
	refresh();

	if (timeout < 0) {
		pthread_mutex_unlock(&f->mtx);
		return;
	}

	wtimeout(stdscr, timeout == 0 ? -1 : timeout);
	wmove(stdscr, f->cursor_y, f->cursor_x);

	wint_t wch;
	pthread_mutex_unlock(&f->mtx);
	int rc = wget_wch(stdscr, &wch);
	pthread_mutex_lock(&f->mtx);

	/* fw may be invalid, regather it */
	fw = stfl_gather_focus_widget(f);
	f->current_focus_id = fw ? fw->id : 0;

	struct stfl_widget *w = fw;

	if (rc == ERR) {
		stfl_form_event(f, wcsdup(L"TIMEOUT"));
		goto unshift_next_event;
	}

	while (w) {
		if (w->type->f_process && w->type->f_process(w, fw, f, wch, rc == KEY_CODE_YES))
			goto unshift_next_event;
		w = w->parent;
	}

	if (wch == L'\r' || wch == L'\n') {
		stfl_form_event(f, wcsdup(L"ENTER"));
		goto unshift_next_event;
	}

	if (wch == 27) {
		stfl_form_event(f, wcsdup(L"ESC"));
		goto unshift_next_event;
	}

	if (rc == KEY_CODE_YES && KEY_F(0) <= wch && wch <= KEY_F(63)) {
		wchar_t *event = malloc(4 * sizeof(wchar_t));
		swprintf(event, 4, L"F%d", wch - KEY_F0);
		stfl_form_event(f, event);
		goto unshift_next_event;
	}

	if (wch == L'\t')
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

		goto unshift_next_event;
	}

	if (rc != KEY_CODE_YES) {
		wchar_t *event = malloc(16 * sizeof(wchar_t));
		swprintf(event, 16, L"CHAR(%u)", wch);
		stfl_form_event(f, event);
		goto unshift_next_event;
	}

unshift_next_event:;
	struct stfl_event *e = f->event_queue;
	if (e) {
		f->event_queue = e->next;
		f->event = e->event;
		free(e);
	}

	pthread_mutex_unlock(&f->mtx);
}

void stfl_form_reset()
{
	if (curses_active) {
		endwin();
		curses_active = 0;
	}
}

void stfl_form_free(struct stfl_form *f)
{
	pthread_mutex_lock(&f->mtx);
	if (f->root)
		stfl_widget_free(f->root);
	if (f->event)
		free(f->event);
	pthread_mutex_unlock(&f->mtx);
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

