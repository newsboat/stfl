/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006, 2007  Clifford Wolf <clifford@clifford.at>
 *  Copyright (C) 2006  Andreas Krennmair <ak@synflood.at>
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
 *  wt_textview.c: Widget type 'textview'
 */

#include "stfl_internals.h"

#include <string.h>
#include <stdlib.h>

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

unsigned int compute_len_from_width(const wchar_t *p, unsigned int width)
{
	unsigned int len = 0;
	unsigned int end_loop = 0;
	while (p && !end_loop) {
		if (wcwidth(*p) > width) {
			end_loop = 1;
		} else {
			width -= wcwidth(*p);
			p++;
			len++;
		}
	}
	return len;
}

static void print_richtext(struct stfl_widget *w, WINDOW *win, unsigned int y, unsigned int x, const wchar_t * text, unsigned int width, const wchar_t * style_normal)
{
	const wchar_t *p = text;

	unsigned int end_col = x + width;

	while (*p) {
		unsigned int len = compute_len_from_width(p, end_col - x);
		const wchar_t *p1 = wcschr(p, L'<');
		if (NULL == p1) {
			mvwaddnwstr(win, y, x, p, len);
			break;
		} else {
			const wchar_t *p2 = wcschr(p1 + 1, L'>');

			if (len > (p1 - p))
				len = p1 - p;
			mvwaddnwstr(win, y, x, p, len);
			x += len;

			if (p2) {
				wchar_t stylename[p2 - p1];
				wmemcpy(stylename, p1 + 1, p2 - p1 - 1);
				stylename[p2 - p1 - 1] = L'\0';
				if (wcscmp(stylename,L"")==0) {
					mvwaddnwstr(win, y, x, L"<", 1);
					++x;
				} else if (wcscmp(stylename, L"/")==0) {
					stfl_style(win, style_normal);
				} else {
					wchar_t lookup_stylename[128];
					const wchar_t * style;
					/* TODO: add support for style_%ls_focus */
					swprintf(lookup_stylename, sizeof(lookup_stylename)/sizeof(*lookup_stylename), L"style_%ls_normal", stylename);
					style = stfl_widget_getkv_str(w, lookup_stylename, L"");
					stfl_style(win, style);
				}
				p = p2 + 1;
			} else {
				break;
			}
		}
	}
}

static void wt_textview_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	//fix_offset_pos(w);

	int offset = stfl_widget_getkv_int(w, L"offset", 0);
	int is_richtext = stfl_widget_getkv_int(w, L"richtext", 0);

	const wchar_t *style_normal = stfl_widget_getkv_str(w, L"style_normal", L"");
	const wchar_t *style_end = stfl_widget_getkv_str(w, L"style_end", L"");

	struct stfl_widget *c;
	int i;

	stfl_style(win, style_normal);
	for (i=0, c=w->first_child; c && i < offset+w->h; i++, c=c->next_sibling)
	{
		const wchar_t *text = stfl_widget_getkv_str(c, L"text", L"");

		if (i < offset) {
			if (is_richtext)
				print_richtext(w, win, w->y, w->x, text, 0, style_normal);
			continue;
		}

		if (is_richtext) {
			print_richtext(w, win, w->y+i-offset, w->x, text, w->w, style_normal);
		} else {
			mvwaddnwstr(win, w->y+i-offset, w->x, text, w->w);
		}
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

