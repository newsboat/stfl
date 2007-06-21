/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006, 2007  Clifford Wolf <clifford@clifford.at>
 *  Copyright (C) 2006  Andreas Krennmair <ak@synflood.at>
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
 *  wt_textview.c: Widget type 'textview'
 */

#include "stfl_internals.h"

#include <string.h>

#if 0
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
#endif

static void wt_textview_prepare(struct stfl_widget *w, struct stfl_form *f)
{
	struct stfl_widget *c = w->first_child;
	
	w->min_w = 1;
	w->min_h = 5;

	if (c)
		w->allow_focus = 1;

	while (c) {
		const wchar_t * text = stfl_widget_getkv_str(c, L"text", L"");
		int len = wcswidth(text, wcslen(text));
		w->min_w = len > w->min_w ? len : w->min_w;
		c = c->next_sibling;
	}
}

static void wt_textview_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	//fix_offset_pos(w);

	int offset = stfl_widget_getkv_int(w, L"offset", 0);

	const wchar_t *style_normal = stfl_widget_getkv_str(w, L"style_normal", L"");
	const wchar_t *style_end = stfl_widget_getkv_str(w, L"style_end", L"");

	struct stfl_widget *c;
	int i;

	stfl_style(win, style_normal);
	for (i=0, c=w->first_child; c && i < offset+w->h; i++, c=c->next_sibling)
	{
		if (i < offset)
			continue;

		mvwaddnwstr(win, w->y+i-offset, w->x,
				stfl_widget_getkv_str(c, L"text", L""), w->w);
	}

	stfl_style(win, style_end);
	while (i<offset+w->h) {
		mvwaddnwstr(win,w->y+i-offset,w->x,L"~",w->w);
		++i;
	}

	if (f->current_focus_id == w->id)
		f->cursor_x = f->cursor_y = -1;
}

static int wt_textview_process(struct stfl_widget *w, struct stfl_widget *fw, struct stfl_form *f, wchar_t ch, int isfunckey)
{
	//int pos = stfl_widget_getkv_int(w, "pos", 0);
	int offset = stfl_widget_getkv_int(w,L"offset",0);
	int maxoffset = -1;

	struct stfl_widget *c = w->first_child;
	while (c) {
		maxoffset++;
		c = c->next_sibling;
	}

	if (offset > 0 && stfl_matchbind(w, ch, isfunckey, L"up", L"UP")) {
		stfl_widget_setkv_int(w, L"offset", offset-1);
		
		//fix_offset_pos(w);
		return 1;
	}
		
	if (offset < maxoffset && stfl_matchbind(w, ch, isfunckey, L"down", L"DOWN")) {
		stfl_widget_setkv_int(w, L"offset", offset+1);
		//fix_offset_pos(w);
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"page_up", L"PPAGE")) {
		if ((offset - w->h + 1) > 0) { // XXX: first page handling won't work with that
			stfl_widget_setkv_int(w, L"offset", offset - w->h + 1);
		} else {
			stfl_widget_setkv_int(w, L"offset", 0);
		}
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"page_down", L"NPAGE")) {
		if ((offset + w->h - 1) < maxoffset) { // XXX: last page handling won't work with that
			stfl_widget_setkv_int(w, L"offset", offset + w->h - 1);
		} else {
			stfl_widget_setkv_int(w, L"offset", maxoffset);
		}
		return 1;
	}

	return 0;
}

struct stfl_widget_type stfl_widget_type_textview = {
	L"textview",
	0, // f_init
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_textview_prepare,
	wt_textview_draw,
	wt_textview_process,
};

