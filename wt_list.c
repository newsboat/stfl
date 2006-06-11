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
 *  wt_list.c: Widget type 'list'
 */

#include "stfl_internals.h"

#include <string.h>

static void fix_offset_pos(struct stfl_widget *w)
{
	int offset = stfl_widget_getkv_int(w, "offset", 0);
	int pos = stfl_widget_getkv_int(w, "pos", 0);

	int orig_offset = offset;
	int orig_pos = pos;

	while (pos < offset)
		offset--;

	if (w->h > 0)
		while (pos >= offset+w->h)
			offset++;

	int maxpos = -1;
	struct stfl_widget *c = w->first_child;
	while (c) {
		maxpos++;
		c = c->next_sibling;
	}

	if (maxpos >= 0 && pos > maxpos)
		pos = maxpos;

	if (offset != orig_offset)
		stfl_widget_setkv_int(w, "offset", offset);

	if (pos != orig_pos)
		stfl_widget_setkv_int(w, "pos", pos);
}

static void wt_list_prepare(struct stfl_widget *w, struct stfl_form *f)
{
	struct stfl_widget *c = w->first_child;
	
	w->min_w = 1;
	w->min_h = 5;

	if (c)
		w->allow_focus = 1;

	while (c) {
		int len = strlen(stfl_widget_getkv_str(c, "text", ""));
		w->min_w = len > w->min_w ? len : w->min_w;
		c = c->next_sibling;
	}
}

static void wt_list_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	fix_offset_pos(w);

	int offset = stfl_widget_getkv_int(w, "offset", 0);
	int pos = stfl_widget_getkv_int(w, "pos", 0);

	const char *style_focus = stfl_widget_getkv_str(w, "style_focus", "");
	const char *style_selected = stfl_widget_getkv_str(w, "style_selected", "");
	const char *style_normal = stfl_widget_getkv_str(w, "style_normal", "");

	struct stfl_widget *c;
	int i;

	for (i=0, c=w->first_child; c && i < offset+w->h; i++, c=c->next_sibling)
	{
		if (i < offset)
			continue;

		if (i == pos) {
			if (f->current_focus_id == w->id)
				stfl_style(win, style_focus);
			else
				stfl_style(win, style_selected);

			stfl_widget_setkv_str(w, "pos_name", c->name ? c->name : "");	
		} else
			stfl_style(win, style_normal);

		mvwaddnstr(win, w->y+i-offset, w->x,
				stfl_widget_getkv_str(c, "text", ""), w->w);
	}

	if (f->current_focus_id == w->id)
		f->cursor_x = f->cursor_y = -1;
}

static int wt_list_process(struct stfl_widget *w, struct stfl_widget *fw, struct stfl_form *f, int ch)
{
	int pos = stfl_widget_getkv_int(w, "pos", 0);
	int maxpos = -1;

	struct stfl_widget *c = w->first_child;
	while (c) {
		maxpos++;
		c = c->next_sibling;
	}

	if (ch == KEY_UP && pos > 0) {
		stfl_widget_setkv_int(w, "pos", pos-1);
		fix_offset_pos(w);
		return 1;
	}
		
	if (ch == KEY_DOWN && pos < maxpos) {
		stfl_widget_setkv_int(w, "pos", pos+1);
		fix_offset_pos(w);
		return 1;
	}

	return 0;
}

struct stfl_widget_type stfl_widget_type_list = {
	"list",
	0, // f_init
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_list_prepare,
	wt_list_draw,
	wt_list_process,
};

static void wt_listitem_init(struct stfl_widget *w)
{
	if (w->parent && !strcmp(w->parent->type->name, "list"))
		w->parent->allow_focus = 1;
}

static void wt_listitem_done(struct stfl_widget *w)
{
	if (w->parent && !strcmp(w->parent->type->name, "list") &&
	    w->parent->first_child == w && w->parent->last_child == w)
		w->parent->allow_focus = 0;
}

static void wt_listitem_prepare(struct stfl_widget *w, struct stfl_form *f) { }
static void wt_listitem_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win) { }

struct stfl_widget_type stfl_widget_type_listitem = {
	"listitem",
	wt_listitem_init,
	wt_listitem_done,
	0, // f_enter 
	0, // f_leave
	wt_listitem_prepare,
	wt_listitem_draw,
	0  // f_process
};

