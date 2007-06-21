/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006, 2007  Clifford Wolf <clifford@clifford.at>
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
 *  wt_input.c: Widget type 'input'
 */

#include "stfl_internals.h"

#include <string.h>
#include <stdlib.h>
#include <wctype.h>

static void wt_input_init(struct stfl_widget *w)
{
	w->allow_focus = 1;
}

static void fix_offset_pos(struct stfl_widget *w)
{
	int pos = stfl_widget_getkv_int(w, L"pos", 0);
	int offset = stfl_widget_getkv_int(w, L"offset", 0);
	const wchar_t* text = stfl_widget_getkv_str(w, L"text", L"");
	int text_len = wcslen(text);
	int changed = 0;
	int pos_width = 0;
	int offset_width = 0;
	int i;

	if (pos > text_len) {
		pos = text_len;
		changed = 1;
	}

	if (offset > text_len) {
		offset = text_len;
		changed = 1;
	}

	if (offset > pos) {
		offset = pos;
		changed = 1;
	}

	for (i=0;i<pos;++i) {
		pos_width += wcwidth(text[i]);
	}

	while (text[offset] && pos-offset >= w->w && pos_width-offset_width >= w->w && w->w > 0) {
		offset_width += wcwidth(text[offset]);
		offset++;
		changed = 1;
	}

	if (changed) {
		stfl_widget_setkv_int(w, L"pos", pos);
		stfl_widget_setkv_int(w, L"offset", offset);
	}
}

static void wt_input_prepare(struct stfl_widget *w, struct stfl_form *f)
{
	int size = stfl_widget_getkv_int(w, L"size", 5);

	w->min_w = size;
	w->min_h = 1;

	fix_offset_pos(w);
}

static void wt_input_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	int pos = stfl_widget_getkv_int(w, L"pos", 0);
	int offset = stfl_widget_getkv_int(w, L"offset", 0);
	const wchar_t *text = stfl_widget_getkv_str(w, L"text", L"");
	int pos_width = 0, offset_width = 0, i;

	stfl_widget_style(w, f, win);

	for (i=0; i<w->w; i++)
		mvwaddwstr(win, w->y, w->x+i, L" ");
	mvwaddnwstr(win, w->y, w->x, text+offset, wcswidth(text+offset,w->w));

	for (i=0;i<pos;++i) {
		pos_width += wcwidth(text[i]);
	}

	for (i=0;i<offset;++i) {
		offset_width += wcwidth(text[i]);
	}

	if (f->current_focus_id == w->id) {
		f->cursor_x = w->x + pos_width - offset_width;
		f->cursor_y = w->y;
	}
}

static int wt_input_process(struct stfl_widget *w, struct stfl_widget *fw, struct stfl_form *f, wchar_t ch, int isfunckey)
{
	int pos = stfl_widget_getkv_int(w, L"pos", 0);
	const wchar_t *text = stfl_widget_getkv_str(w, L"text", L"");
	int text_len = wcslen(text);

	if (pos > 0 && stfl_matchbind(w, ch, isfunckey, L"left", L"LEFT")) {
		stfl_widget_setkv_int(w, L"pos", pos-1);
		fix_offset_pos(w);
		return 1;
	}

	if (pos < text_len && stfl_matchbind(w, ch, isfunckey, L"right", L"RIGHT")) {
		stfl_widget_setkv_int(w, L"pos", pos+1);
		fix_offset_pos(w);
		return 1;
	}

	// pos1 / home / Ctrl-A
	if (stfl_matchbind(w, ch, isfunckey, L"home", L"HOME ^A")) {
		stfl_widget_setkv_int(w, L"pos", 0);
		fix_offset_pos(w);
		return 1;
	}

	// end / Ctrl-E
	if (stfl_matchbind(w, ch, isfunckey, L"end", L"END ^E")) {
		stfl_widget_setkv_int(w, L"pos", text_len);
		fix_offset_pos(w);
		return 1;
	}

	// delete
	if (stfl_matchbind(w, ch, isfunckey, L"delete", L"DC")) {
		if (pos == text_len)
			return 0;
		wchar_t newtext[text_len];
		wmemcpy(newtext, text, pos);
		wmemcpy(newtext+pos, text+pos+1, text_len-(pos+1));
		newtext[text_len-1] = 0;
		stfl_widget_setkv_str(w, L"text", newtext);
		fix_offset_pos(w);
		return 1;
	}

	// backspace
	if (stfl_matchbind(w, ch, isfunckey, L"backspace", L"BACKSPACE")) {
		if (pos == 0)
			return 0;
		wchar_t newtext[text_len];
		wmemcpy(newtext, text, pos-1);
		wmemcpy(newtext+pos-1, text+pos, text_len-pos);
		newtext[text_len-1] = 0;
		stfl_widget_setkv_str(w, L"text", newtext);
		stfl_widget_setkv_int(w, L"pos", pos-1);
		fix_offset_pos(w);
		return 1;
	}

	// 'normal' characters
	if (!isfunckey && iswprint(ch)) {
		wchar_t newtext[text_len + 2];
		wmemcpy(newtext, text, pos);
		newtext[pos] = ch;
		wmemcpy(newtext+pos+1, text+pos, text_len - pos);
		newtext[text_len + 1] = 0;
		stfl_widget_setkv_str(w, L"text", newtext);
		stfl_widget_setkv_int(w, L"pos", pos+1);
		fix_offset_pos(w);
		return 1;
	}

	return 0;
}

struct stfl_widget_type stfl_widget_type_input = {
	L"input",
	wt_input_init,
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_input_prepare,
	wt_input_draw,
	wt_input_process
};

