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
 *  wt_input.c: Widget type 'input'
 */

#include "stfl_internals.h"

#include <string.h>
#include <stdlib.h>

static void wt_input_init(struct stfl_widget *w)
{
	w->allow_focus = 1;
}

static void fix_offset_pos(struct stfl_widget *w)
{
	int pos = stfl_widget_getkv_int(w, "pos", 0);
	int offset = stfl_widget_getkv_int(w, "offset", 0);
	int text_len = strlen(stfl_widget_getkv_str(w, "text", ""));
	int changed = 0;

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

	while (pos-offset >= w->w && w->w > 0) {
		offset++;
		changed = 1;
	}

	if (changed) {
		stfl_widget_setkv_int(w, "pos", pos);
		stfl_widget_setkv_int(w, "offset", offset);
	}
}

static void wt_input_prepare(struct stfl_widget *w, struct stfl_form *f)
{
	int size = stfl_widget_getkv_int(w, "size", 5);

	w->min_w = size;
	w->min_h = 1;

	fix_offset_pos(w);
}

static void wt_input_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	int pos = stfl_widget_getkv_int(w, "pos", 0);
	int offset = stfl_widget_getkv_int(w, "offset", 0);
	const char *text = stfl_widget_getkv_str(w, "text", "");
	int i;

	stfl_widget_style(w, f, win);

	for (i=0; i<w->w; i++)
		mvwaddch(win, w->y, w->x+i, ' ');
	mvwaddnstr(win, w->y, w->x, text+offset, w->w);

	if (f->current_focus_id == w->id) {
		f->cursor_x = w->x + pos - offset;
		f->cursor_y = w->y;
	}
}

static int wt_input_process(struct stfl_widget *w, struct stfl_widget *fw, struct stfl_form *f, int ch)
{
	int pos = stfl_widget_getkv_int(w, "pos", 0);
	const char *text = stfl_widget_getkv_str(w, "text", "");
	int text_len = strlen(text);
	int modal = stfl_widget_getkv_int(w, "modal", 0);

	if (modal && ((ch == '\t') || (ch == KEY_LEFT && pos <= 0) || (ch == KEY_RIGHT && pos >= text_len) || (ch == KEY_UP) || (ch == KEY_DOWN)))
		return 1;

	if (ch == KEY_LEFT && pos > 0) {
		stfl_widget_setkv_int(w, "pos", pos-1);
		fix_offset_pos(w);
		return 1;
	}

	if (ch == KEY_RIGHT && pos < text_len) {
		stfl_widget_setkv_int(w, "pos", pos+1);
		fix_offset_pos(w);
		return 1;
	}

	// pos1 / home / Ctrl-A
	if (ch == KEY_HOME || ch == 1) {
		stfl_widget_setkv_int(w, "pos", 0);
		fix_offset_pos(w);
		return 1;
	}

	// end / Ctrl-E
	if (ch == KEY_END || ch == 5) {
		stfl_widget_setkv_int(w, "pos", text_len);
		fix_offset_pos(w);
		return 1;
	}

	// delete
	if (ch == KEY_DC) {
		if (pos == text_len)
			return 0;
		char newtext[text_len];
		memcpy(newtext, text, pos);
		memcpy(newtext+pos, text+pos+1, text_len-(pos+1));
		newtext[text_len-1] = 0;
		stfl_widget_setkv_str(w, "text", newtext);
		fix_offset_pos(w);
		return 1;
	}

	// backspace
	if (ch == KEY_BACKSPACE || ch == 127) {
		if (pos == 0)
			return 0;
		char newtext[text_len];
		memcpy(newtext, text, pos-1);
		memcpy(newtext+pos-1, text+pos, text_len-pos);
		newtext[text_len-1] = 0;
		stfl_widget_setkv_str(w, "text", newtext);
		stfl_widget_setkv_int(w, "pos", pos-1);
		fix_offset_pos(w);
		return 1;
	}

	// 'normal' characters
	if (ch >= 32 && ch <= 255) {
		char newtext[text_len + 2];
		memcpy(newtext, text, pos);
		newtext[pos] = ch;
		memcpy(newtext+pos+1, text+pos, text_len - pos);
		newtext[text_len + 1] = 0;
		stfl_widget_setkv_str(w, "text", newtext);
		stfl_widget_setkv_int(w, "pos", pos+1);
		fix_offset_pos(w);
		return 1;
	}

	return 0;
}

struct stfl_widget_type stfl_widget_type_input = {
	"input",
	wt_input_init,
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_input_prepare,
	wt_input_draw,
	wt_input_process
};

