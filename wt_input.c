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
 *  wt_input.c: Widget type 'input'
 */

#include "stfl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void wt_input_init(struct stfl_widget *w)
{
	w->allow_focus = 1;
}

static void wt_input_getminwh(struct stfl_widget *w)
{
	struct stfl_kv *val = stfl_widget_getkv(w, "size");
	w->min_w = !val ? 5 : atoi(val->value);
	w->min_h = 1;
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

	while (pos-offset >= w->w) {
		offset++;
		changed = 1;
	}

	if (changed) {
		stfl_widget_setkv_int(w, "pos", pos);
		stfl_widget_setkv_int(w, "offset", offset);
	}
}

static void wt_input_draw(struct stfl_widget *w, WINDOW *win)
{
	fix_offset_pos(w);

	int offset = stfl_widget_getkv_int(w, "offset", 0);
	const char *text = stfl_widget_getkv_str(w, "text", "");
	int i;

	wattron(win, A_UNDERLINE);
	for (i=0; i<w->w; i++)
		mvwaddch(win, w->y, w->x+i, ' ');
	mvwaddnstr(win, w->y, w->x, text+offset, w->w);
	wattroff(win, A_UNDERLINE);
}

static void wt_input_run(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	fix_offset_pos(w);

	int pos = stfl_widget_getkv_int(w, "pos", 0);
	int offset = stfl_widget_getkv_int(w, "offset", 0);
	const char *text = stfl_widget_getkv_str(w, "text", "");
	int text_len = strlen(text);

	int ch = mvwgetch(win, w->y, w->x + pos - offset);

	if (ch == KEY_LEFT && pos > 0) {
		pos--;
		goto finish;
	}

	if (ch == KEY_RIGHT && pos <= text_len) {
		pos++;
		goto finish;
	}

	if (stfl_core_events(w, f, win, ch))
		goto finish;

	// pos1 / home / Ctrl-A
	if (ch == KEY_HOME || ch == 1) {
		pos = 0;
		goto finish;
	}

	// end / Ctrl-E
	if (ch == KEY_END || ch == 5) {
		pos = text_len;
		goto finish;
	}

	// delete
	if (ch == KEY_DC) {
		if (pos == text_len)
			goto finish;
		char newtext[text_len];
		memcpy(newtext, text, pos);
		memcpy(newtext+pos, text+pos+1, text_len-(pos+1));
		newtext[text_len-1] = 0;
		stfl_widget_setkv_str(w, "text", newtext);
		goto finish;
	}

	// backspace
	if (ch == KEY_BACKSPACE || ch == 127) {
		if (pos == 0)
			goto finish;
		char newtext[text_len];
		memcpy(newtext, text, pos-1);
		memcpy(newtext+pos-1, text+pos, text_len-pos);
		newtext[text_len-1] = 0;
		stfl_widget_setkv_str(w, "text", newtext);
		pos--;
		goto finish;
	}

	// 'normal' characters
	if (ch >= 32 && ch <= 255) {
		char newtext[text_len + 2];
		memcpy(newtext, text, pos);
		newtext[pos] = ch;
		memcpy(newtext+pos+1, text+pos, text_len - pos);
		newtext[text_len + 1] = 0;
		stfl_widget_setkv_str(w, "text", newtext);
		pos++;
		goto finish;
	}

	fprintf(stderr, ">> Unhandled input char: %d 0%o 0x%x\n", ch, ch, ch);

finish:;
	stfl_widget_setkv_int(w, "pos", pos);
	stfl_widget_setkv_int(w, "offset", offset);
	fix_offset_pos(w);
}

struct stfl_widget_type stfl_widget_type_input = {
	"input",
	wt_input_init,
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_input_getminwh,
	wt_input_draw,
	wt_input_run
};

