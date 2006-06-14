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
 *  stfl_internals.h: The STFL C header file (Internal STFL APIs)
 */

#ifndef STFL__INTERNALS_H
#define STFL__INTERNALS_H 1

#ifdef  __cplusplus
extern "C" {
#endif

#include "stfl.h"
#include <curses.h>

struct stfl_widget_type;
struct stfl_kv;
struct stfl_widget;

struct stfl_widget_type {
	char *name;

	void (*f_init)(struct stfl_widget *w);
	void (*f_done)(struct stfl_widget *w);

	void (*f_enter)(struct stfl_widget *w, struct stfl_form *f);
	void (*f_leave)(struct stfl_widget *w, struct stfl_form *f);

	void (*f_prepare)(struct stfl_widget *w, struct stfl_form *f);
	void (*f_draw)(struct stfl_widget *w, struct stfl_form *f, WINDOW *win);
	int (*f_process)(struct stfl_widget *w, struct stfl_widget *fw, struct stfl_form *f, int ch);
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
	int setfocus;
	void *internal_data;
	char *name, *cls;
};

struct stfl_event {
	struct stfl_event *next;
	char *event;
};

struct stfl_form {
	struct stfl_widget *root;
	int current_focus_id;
	int cursor_x, cursor_y;
	struct stfl_event *event_queue;
	char *event;
};

extern struct stfl_widget_type *stfl_widget_types[];

extern struct stfl_widget_type stfl_widget_type_label;
extern struct stfl_widget_type stfl_widget_type_input;
extern struct stfl_widget_type stfl_widget_type_vbox;
extern struct stfl_widget_type stfl_widget_type_hbox;
extern struct stfl_widget_type stfl_widget_type_table;
extern struct stfl_widget_type stfl_widget_type_tablebr;
extern struct stfl_widget_type stfl_widget_type_list;
extern struct stfl_widget_type stfl_widget_type_listitem;

extern struct stfl_widget *stfl_widget_new(const char *type);
extern void stfl_widget_free(struct stfl_widget *w);

extern struct stfl_kv *stfl_widget_setkv_int(struct stfl_widget *w, const char *key, int value);
extern struct stfl_kv *stfl_widget_setkv_str(struct stfl_widget *w, const char *key, const char *value);

extern struct stfl_kv *stfl_setkv_by_name_int(struct stfl_widget *w, const char *name, int value);
extern struct stfl_kv *stfl_setkv_by_name_str(struct stfl_widget *w, const char *name, const char *value);

extern struct stfl_kv *stfl_widget_getkv(struct stfl_widget *w, const char *key);
extern int stfl_widget_getkv_int(struct stfl_widget *w, const char *key, int defval);
extern const char *stfl_widget_getkv_str(struct stfl_widget *w, const char *key, const char *defval);

extern int stfl_getkv_by_name_int(struct stfl_widget *w, const char *name, int defval);
extern const char *stfl_getkv_by_name_str(struct stfl_widget *w, const char *name, const char *defval);

extern struct stfl_widget *stfl_widget_by_name(struct stfl_widget *w, const char *name);
extern struct stfl_widget *stfl_widget_by_id(struct stfl_widget *w, int id);

extern struct stfl_kv *stfl_kv_by_name(struct stfl_widget *w, const char *name);
extern struct stfl_kv *stfl_kv_by_id(struct stfl_widget *w, int id);

extern struct stfl_widget *stfl_find_child_tree(struct stfl_widget *w, struct stfl_widget *c);
extern struct stfl_widget *stfl_find_first_focusable(struct stfl_widget *w);
extern int stfl_switch_focus(struct stfl_widget *old_fw, struct stfl_widget *new_fw, struct stfl_form *f);

extern int stfl_focus_prev(struct stfl_widget *w, struct stfl_widget *old_fw, struct stfl_form *f);
extern int stfl_focus_next(struct stfl_widget *w, struct stfl_widget *old_fw, struct stfl_form *f);

extern struct stfl_form *stfl_form_new();
extern void stfl_form_event(struct stfl_form *f, char *event);
extern void stfl_form_run(struct stfl_form *f, int timeout);
extern void stfl_form_reset();
extern void stfl_form_free(struct stfl_form *f);

extern void stfl_check_setfocus(struct stfl_form *f, struct stfl_widget *w);

extern struct stfl_widget *stfl_parser(const char *text);
extern struct stfl_widget *stfl_parser_file(const char *filename);

extern char *stfl_quote_backend(const char *text);
extern char *stfl_widget_dump(struct stfl_widget *w, const char *prefix, int focus_id);

extern void stfl_style(WINDOW *win, const char *style);
extern void stfl_widget_style(struct stfl_widget *w, struct stfl_form *f, WINDOW *win);

#ifdef __cplusplus
}
#endif

#endif

