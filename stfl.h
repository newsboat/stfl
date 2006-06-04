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
 *  stfl.h: The STFL C header file
 */

#ifndef STFL_H
#define STFL_H 1

#include <curses.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct stfl_widget_type;
struct stfl_kv;
struct stfl_widget;
struct stfl_form;

struct stfl_widget_type {
	char *name;

	void (*f_init)(struct stfl_widget *w);
	void (*f_done)(struct stfl_widget *w);

	void (*f_enter)(struct stfl_widget *w);
	void (*f_leave)(struct stfl_widget *w);

	void (*f_getminwh)(struct stfl_widget *w);
	void (*f_draw)(struct stfl_widget *w, WINDOW *win);
	void (*f_run)(struct stfl_widget *w, struct stfl_form *f, WINDOW *win);
};

struct stfl_kv {
	struct stfl_kv *next;
	struct stfl_widget *widget;
	char *key, *value, *name;
	int id;
};

struct stfl_widget {
	struct stfl_widget *parent;
	struct stfl_widget *next_sibling;
	struct stfl_widget *first_child;
	struct stfl_widget *last_child;
	struct stfl_kv *kv_list;
	struct stfl_widget_type *type;
	int id, x, y, w, h, min_w, min_h;
	int parser_indent, allow_focus;
	void *internal_data;
	char *name;
};

#define STFL_EVENT_NONE		0
#define STFL_EVENT_TIMEOUT	1
#define STFL_EVENT_NEWVAL	2
#define STFL_EVENT_ENTER	3
#define STFL_EVENT_FKEY		4
#define STFL_EVENT_NEXT		5

struct stfl_form {
	struct stfl_widget *root;
	int current_focus_id;
	int event_type, event_value;
};

extern struct stfl_widget_type *stfl_widget_type_list[];
extern struct stfl_widget_type stfl_widget_type_label;
extern struct stfl_widget_type stfl_widget_type_vbox;
extern struct stfl_widget_type stfl_widget_type_hbox;

extern struct stfl_widget *stfl_widget_new(const char *type);
extern struct stfl_widget *stfl_widget_copy(struct stfl_widget *w);
extern void stfl_widget_free(struct stfl_widget *w);

extern struct stfl_kv *stfl_widget_setkv(struct stfl_widget *w, const char *key, const char *value);
extern struct stfl_kv *stfl_widget_getkv(struct stfl_widget *w, const char *key);

extern struct stfl_widget *stfl_widget_by_name(struct stfl_widget *w, const char *name);
extern struct stfl_widget *stfl_widget_by_id(struct stfl_widget *w, int id);

extern struct stfl_kv *stfl_kv_by_name(struct stfl_widget *w, const char *name);
extern struct stfl_kv *stfl_kv_by_id(struct stfl_widget *w, int id);

extern struct stfl_form *stfl_form_new();
extern void stfl_form_run(struct stfl_form *f, WINDOW *win);
extern void stfl_form_free(struct stfl_form *f);

extern struct stfl_widget *stfl_parser(const char *text);

#ifdef __cplusplus
}
#endif

#endif

