/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006, 2007, 2014  Clifford Wolf <clifford@clifford.at>
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
 *  wt_textedit.c: Widget type 'textedit'
 */

#include "stfl_internals.h"

#include <string.h>
#include <stdlib.h>
#include <wctype.h>

static void wt_textedit_prepare(struct stfl_widget *w, struct stfl_form *f)
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

static void wt_textedit_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	int cursor_x = stfl_widget_getkv_int(w, L"cursor_x", 0);
	int cursor_y = stfl_widget_getkv_int(w, L"cursor_y", 0);

	int scroll_x = stfl_widget_getkv_int(w, L"scroll_x", 0);
	int scroll_y = stfl_widget_getkv_int(w, L"scroll_y", 0);

	if (cursor_x < scroll_x) {
		scroll_x = cursor_x;
		stfl_widget_setkv_int(w, L"scroll_x", scroll_x);
	}

	if (cursor_x >= scroll_x + w->w - 1) {
		scroll_x = cursor_x - w->w + 1;
		stfl_widget_setkv_int(w, L"scroll_x", scroll_x);
	}

	if (cursor_y < scroll_y) {
		scroll_y = cursor_y;
		stfl_widget_setkv_int(w, L"scroll_y", scroll_y);
	}

	if (cursor_y >= scroll_y + w->h - 1) {
		scroll_y = cursor_y - w->h + 1;
		stfl_widget_setkv_int(w, L"scroll_y", scroll_y);
	}

	const wchar_t *style_normal = stfl_widget_getkv_str(w, L"style_normal", L"");
	const wchar_t *style_end = stfl_widget_getkv_str(w, L"style_end", L"");

	struct stfl_widget *c;
	int clipped_cursor_x = cursor_x;
	int i, j;

	stfl_style(win, style_normal);
	for (i = 0, c = w->first_child; c && i < scroll_y + w->h; i++, c = c->next_sibling)
	{
		if (i < scroll_y)
			continue;

		const wchar_t *text = stfl_widget_getkv_str(c, L"text", L"");

		if (i == cursor_y)
			clipped_cursor_x = wcslen(text) < clipped_cursor_x ? wcslen(text) : clipped_cursor_x;

		for (j = 0; j < scroll_x && *text; j += wcwidth(*(text++))) { }
		mvwaddnwstr(win, w->y + i - scroll_y, w->x, text, w->w);
	}

	stfl_style(win, style_end);
	for (; i < scroll_y + w->h; i++)
		mvwaddnwstr(win, w->y + i - scroll_y, w->x, L"~",w->w);

	if (f->current_focus_id == w->id) {
		f->root->cur_x = f->cursor_x = w->x + clipped_cursor_x - scroll_x;
		f->root->cur_y = f->cursor_y = w->y + cursor_y - scroll_y;
	}
}

static int wt_textedit_process(struct stfl_widget *w, struct stfl_widget *fw, struct stfl_form *f, wchar_t ch, int isfunckey)
{
	int cursor_x = stfl_widget_getkv_int(w, L"cursor_x", 0);
	int cursor_y = stfl_widget_getkv_int(w, L"cursor_y", 0);
	int num_lines = 0, line_length = 0;

	struct stfl_widget *c_current_line = NULL;
	struct stfl_widget *c = w->first_child;
	while (c) {
		if (num_lines == cursor_y) {
			line_length = wcslen(stfl_widget_getkv_str(c, L"text", L""));
			c_current_line = c;
		}
		c = c->next_sibling;
		num_lines++;
	}

	if (c_current_line == NULL) {
		c_current_line = w->last_child;
		cursor_y = num_lines-1 > 0 ? num_lines-1 : 0;
	}

	if (c_current_line == NULL) {
		c_current_line = stfl_widget_new(L"listitem");
		w->last_child = c_current_line;
		w->first_child = c_current_line;
		num_lines = 1;
	}

	if (cursor_y > 0 && stfl_matchbind(w, ch, isfunckey, L"up", L"UP")) {
		stfl_widget_setkv_int(w, L"cursor_y", cursor_y-1);
		return 1;
	}
		
	if (cursor_y+1 < num_lines && stfl_matchbind(w, ch, isfunckey, L"down", L"DOWN")) {
		stfl_widget_setkv_int(w, L"cursor_y", cursor_y+1);
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"left", L"LEFT")) {
		cursor_x = cursor_x-1 < line_length-1 ? cursor_x-1 : line_length-1;
		stfl_widget_setkv_int(w, L"cursor_x", cursor_x > 0 ? cursor_x : 0);
		return 1;
	}
		
	if (stfl_matchbind(w, ch, isfunckey, L"right", L"RIGHT")) {
		cursor_x = cursor_x+1 < line_length ? cursor_x+1 : line_length;
		stfl_widget_setkv_int(w, L"cursor_x", cursor_x > 0 ? cursor_x : 0);
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"page_up", L"PPAGE")) {
		cursor_y = cursor_y - w->h + 1;
		cursor_y = cursor_y > 0 ? cursor_y : 0;
		cursor_y = cursor_y < num_lines ? cursor_y : num_lines-1;
		stfl_widget_setkv_int(w, L"cursor_y", cursor_y);
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"page_down", L"NPAGE")) {
		cursor_y = cursor_y + w->h - 1;
		cursor_y = cursor_y > 0 ? cursor_y : 0;
		cursor_y = cursor_y < num_lines ? cursor_y : num_lines-1;
		stfl_widget_setkv_int(w, L"cursor_y", cursor_y);
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"home", L"HOME ^A")) {
		stfl_widget_setkv_int(w, L"cursor_x", 0);
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"end", L"END ^E")) {
		stfl_widget_setkv_int(w, L"cursor_x", line_length);
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"delete", L"DC"))
	{
		if (c_current_line == NULL)
			return 0;

		if (cursor_x >= line_length) {
			if (c_current_line->next_sibling == NULL)
				return 0;
			const wchar_t *this_text = stfl_widget_getkv_str(c_current_line, L"text", L"");
			const wchar_t *next_text = stfl_widget_getkv_str(c_current_line->next_sibling, L"text", L"");
			wchar_t newtext[wcslen(this_text) + wcslen(next_text) + 1];
			wcscpy(newtext, this_text);
			wcscat(newtext, next_text);
			stfl_widget_setkv_int(w, L"cursor_x", line_length);
			stfl_widget_setkv_str(c_current_line, L"text", newtext);
			stfl_widget_free(c_current_line->next_sibling);
			return 1;
		}

		wchar_t newtext[line_length];
		const wchar_t *text = stfl_widget_getkv_str(c_current_line, L"text", L"");
		wmemcpy(newtext, text, cursor_x);
		wcscpy(newtext + cursor_x, text + cursor_x + 1);
		stfl_widget_setkv_str(c_current_line, L"text", newtext);
		return 1;
	}

	if (stfl_matchbind(w, ch, isfunckey, L"backspace", L"BACKSPACE"))
	{
		if (c_current_line == NULL)
			return 0;

		if (cursor_x > line_length)
			cursor_x = line_length;

		if (cursor_x == 0) {
			struct stfl_widget *c = w->first_child;
			while (c && c->next_sibling != c_current_line)
				c = c->next_sibling;
			if (c == NULL)
				return 0;
			const wchar_t *prev_text = stfl_widget_getkv_str(c, L"text", L"");
			const wchar_t *this_text = stfl_widget_getkv_str(c_current_line, L"text", L"");
			wchar_t newtext[wcslen(prev_text) + wcslen(this_text) + 1];
			wcscpy(newtext, prev_text);
			wcscat(newtext, this_text);
			stfl_widget_setkv_int(w, L"cursor_x", wcslen(prev_text));
			stfl_widget_setkv_int(w, L"cursor_y", cursor_y - 1);
			stfl_widget_setkv_str(c, L"text", newtext);
			stfl_widget_free(c_current_line);
			return 1;
		}

		wchar_t newtext[line_length];
		const wchar_t *text = stfl_widget_getkv_str(c_current_line, L"text", L"");
		wmemcpy(newtext, text, cursor_x-1);
		wcscpy(newtext + cursor_x - 1, text + cursor_x);
		stfl_widget_setkv_str(c_current_line, L"text", newtext);
		stfl_widget_setkv_int(w, L"cursor_x", cursor_x - 1);
		return 1;
	}

#if 0
	if (stfl_matchbind(w, ch, isfunckey, L"backspace", L"BACKSPACE")) {
		if (pos == 0)
			return 0;
		wchar_t newtext[text_len];
		wmemcpy(newtext, text, pos-1);
		wcscpy(newtext + pos - 1, text + pos);
		stfl_widget_setkv_str(w, L"text", newtext);
		stfl_widget_setkv_int(w, L"pos", pos-1);
		fix_offset_pos(w);
		return 1;
	}
#endif

	if (stfl_matchbind(w, ch, isfunckey, L"enter", L"ENTER"))
	{
		if (c_current_line == NULL) {
			c_current_line = stfl_widget_new(L"listitem");
			c_current_line->parent = w;
			if (w->last_child)
				w->last_child->next_sibling = c_current_line;
			else
				w->first_child = c_current_line;
			w->last_child = c_current_line;
			return 1;
		}

		if (cursor_x > line_length)
			cursor_x = line_length;

		c = stfl_widget_new(L"listitem");
		c->parent = w;
		c->next_sibling = c_current_line->next_sibling;
		c_current_line->next_sibling = c;

		if (w->last_child == c_current_line)
			w->last_child = c;

		const wchar_t *text = stfl_widget_getkv_str(c_current_line, L"text", L"");
		stfl_widget_setkv_str(c, L"text", text + cursor_x);

		wchar_t newtext[cursor_x+1];
		wmemcpy(newtext, text, cursor_x);
		newtext[cursor_x] = 0;
		stfl_widget_setkv_str(c_current_line, L"text", newtext);

		stfl_widget_setkv_int(w, L"cursor_x", 0);
		stfl_widget_setkv_int(w, L"cursor_y", cursor_y + 1);
		return 1;
	}

	if (!isfunckey && iswprint(ch))
	{
		if (c_current_line == NULL) {
			c_current_line = stfl_widget_new(L"listitem");
			c_current_line->parent = w;
			if (w->last_child)
				w->last_child->next_sibling = c_current_line;
			else
				w->first_child = c_current_line;
			w->last_child = c_current_line;
		}

		if (cursor_x > line_length)
			cursor_x = line_length;

		wchar_t newtext[line_length + 1];
		const wchar_t *text = stfl_widget_getkv_str(c_current_line, L"text", L"");
		wmemcpy(newtext, text, cursor_x);
		newtext[cursor_x] = ch;
		wcscpy(newtext + cursor_x + 1, text + cursor_x);

		stfl_widget_setkv_int(w, L"cursor_x", cursor_x+1);
		stfl_widget_setkv_str(c_current_line, L"text", newtext);
		return 1;
	}

	return 0;
}

struct stfl_widget_type stfl_widget_type_textedit = {
	L"textedit",
	0, // f_init
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_textedit_prepare,
	wt_textedit_draw,
	wt_textedit_process,
};

