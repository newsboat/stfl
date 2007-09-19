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
 *  wt_list.c: Widget type 'list'
 */

#include "stfl_internals.h"

#include <string.h>

static void fix_offset_pos(struct stfl_widget *w)
{
	int offset = stfl_widget_getkv_int(w, L"offset", 0);
	int pos = stfl_widget_getkv_int(w, L"pos", 0);

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
		stfl_widget_setkv_int(w, L"offset", offset);

	if (pos != orig_pos)
		stfl_widget_setkv_int(w, L"pos", pos);
}

static void wt_list_prepare(struct stfl_widget *w, struct stfl_form *f)
{
	struct stfl_widget *c = w->first_child;
	
	w->min_w = 1;
	w->min_h = 5;

	if (c)
		w->allow_focus = 1;

	while (c) {
		const wchar_t * text = stfl_widget_getkv_str(c, L"text", L"");
		int len = wcswidth(text,wcslen(text));
		w->min_w = len > w->min_w ? len : w->min_w;
		c = c->next_sibling;
	}
}

static void wt_list_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	const wchar_t * text;
	fix_offset_pos(w);

	int offset = stfl_widget_getkv_int(w, L"offset", 0);
	int pos = stfl_widget_getkv_int(w, L"pos", 0);

	const wchar_t *style_focus = stfl_widget_getkv_str(w, L"style_focus", L"");
	const wchar_t *style_selected = stfl_widget_getkv_str(w, L"style_selected", L"");
	const wchar_t *style_normal = stfl_widget_getkv_str(w, L"style_normal", L"");

	struct stfl_widget *c;
	int i, j;

	if (f->current_focus_id == w->id)
		f->cursor_x = f->cursor_y = -1;

	for (i=0, c=w->first_child; c && i < offset+w->h; i++, c=c->next_sibling)
	{
		if (i < offset)
			continue;

		if (i == pos) {
			if (f->current_focus_id == w->id) {
				stfl_style(win, style_focus);
				f->cursor_y = w->y+i-offset;
				f->cursor_x = w->x;
			} else
				stfl_style(win, style_selected);

			stfl_widget_setkv_str(w, L"pos_name", c->name ? c->name : L"");	
		} else
			stfl_style(win, style_normal);

		text = stfl_widget_getkv_str(c, L"text", L"");
		
		if (wcswidth(text,wcslen(text)) < w->w) {
			wchar_t *fillup = malloc(sizeof(wchar_t)*(w->w - wcswidth(text,wcslen(text)) + 1));
			for (j=0;j < w->w - wcswidth(text,wcslen(text));++j) {
				fillup[j] = ' ';
			}
			fillup[w->w - wcswidth(text,wcslen(text))] = '\0';
			mvwaddnwstr(win, w->y+i-offset, w->x + wcswidth(text,wcslen(text)), fillup, wcswidth(fillup,wcslen(fillup)));
			free(fillup);
		}

		mvwaddnwstr(win, w->y+i-offset, w->x, text, w->w);
	}
}

static int wt_list_process(struct stfl_widget *w, struct stfl_widget *fw, struct stfl_form *f, wchar_t ch, int isfunckey)
{
	int pos = stfl_widget_getkv_int(w, L"pos", 0);
	int maxpos = -1;

	struct stfl_widget *c = w->first_child;
	while (c) {
		maxpos++;
		c = c->next_sibling;
	}

	if (pos > 0 && stfl_matchbind(w, ch, isfunckey, L"up", L"UP")) {
		stfl_widget_setkv_int(w, L"pos", pos-1);
		fix_offset_pos(w);
		return 1;
	}
		
	if (pos < maxpos && stfl_matchbind(w, ch, isfunckey, L"down", L"DOWN")) {
		stfl_widget_setkv_int(w, L"pos", pos+1);
		fix_offset_pos(w);
		return 1;
	}
	
	if (stfl_matchbind(w, ch, isfunckey, L"page_down", L"NPAGE")) {
		if (pos < maxpos - w->h) stfl_widget_setkv_int(w, L"pos", pos + w->h);
		else stfl_widget_setkv_int(w, L"pos", maxpos);
		fix_offset_pos(w);
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"page_up", L"PPAGE")) {
		if (pos > w->h) stfl_widget_setkv_int(w, L"pos", pos - w->h);
		else stfl_widget_setkv_int(w, L"pos", 0);
		fix_offset_pos(w);
		return 1;
	}

	return 0;
}

struct stfl_widget_type stfl_widget_type_list = {
	L"list",
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
	if (w->parent && !wcscmp(w->parent->type->name, L"list"))
		w->parent->allow_focus = 1;
}

static void wt_listitem_done(struct stfl_widget *w)
{
	if (w->parent && !wcscmp(w->parent->type->name, L"list") &&
	    w->parent->first_child == w && w->parent->last_child == w)
		w->parent->allow_focus = 0;
}

static void wt_listitem_prepare(struct stfl_widget *w, struct stfl_form *f) { }
static void wt_listitem_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win) { }

struct stfl_widget_type stfl_widget_type_listitem = {
	L"listitem",
	wt_listitem_init,
	wt_listitem_done,
	0, // f_enter 
	0, // f_leave
	wt_listitem_prepare,
	wt_listitem_draw,
	0  // f_process
};

