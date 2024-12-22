/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006, 2007  Clifford Wolf <clifford@clifford.at>
 *  Copyright (C) 2014  Davor Ocelic <docelic@spinlocksolutions.com>
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
 *  wt_checkbox.c: Widget type 'checkbox'
 */

#include "stfl_internals.h"

#include <string.h>
#include <stdlib.h>

static void wt_checkbox_init(struct stfl_widget *w)
{
	w->allow_focus = 1;
}

static void wt_checkbox_prepare(struct stfl_widget *w, struct stfl_form *f)
{
	const wchar_t * text = stfl_widget_getkv_int(w, L"value", 0) ?
			stfl_widget_getkv_str(w, L"text_1", L"[X]") :
			stfl_widget_getkv_str(w, L"text_0", L"[ ]");

	w->min_w = wcswidth(text, wcslen(text));
	w->min_h = 1;
}

static void wt_checkbox_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	const wchar_t * text;
	unsigned int i;

	int is_richtext = stfl_widget_getkv_int(w, L"richtext", 0);

	const wchar_t * style = stfl_widget_getkv_str(w, L"style_normal", L"");

	stfl_widget_style(w, f, win);

	text = stfl_widget_getkv_int(w, L"value", 0) ? 
			stfl_widget_getkv_str(w, L"text_1", L"[X]") :
			stfl_widget_getkv_str(w, L"text_0", L"[ ]");

	if (w->w >= 0) {
		wchar_t *fillup = calloc(w->w + 1, sizeof(wchar_t));
		for (i=0;i < w->w;++i) {
			fillup[i] = L' ';
		}
		fillup[w->w] = L'\0';
		mvwaddnwstr(win, w->y, w->x, fillup, wcswidth(fillup,wcslen(fillup)));
		free(fillup);
	}

	if (is_richtext)
		stfl_print_richtext(w, win, w->y, w->x, text, w->w, style, 0);
	else
		mvwaddnwstr(win, w->y, w->x, text, w->w);

	if (f->current_focus_id == w->id) {
		f->root->cur_x = f->cursor_x = w->x + stfl_widget_getkv_int(w, L"pos", 1);
		f->root->cur_y = f->cursor_y = w->y;
	}
}

static int wt_checkbox_process(struct stfl_widget *w, struct stfl_widget *fw, struct stfl_form *f, wchar_t ch, int isfunckey)
{
	if (stfl_matchbind(w, ch, isfunckey, L"toggle", L"ENTER SPACE")) {
		int value = stfl_widget_getkv_int(w, L"value", 0);
		stfl_widget_setkv_int(w, L"value", !value);
		return 1;
	}

	return 0;
}

struct stfl_widget_type stfl_widget_type_checkbox = {
	L"checkbox",
	wt_checkbox_init,
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_checkbox_prepare,
	wt_checkbox_draw,
	wt_checkbox_process,
};

