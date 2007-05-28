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
 *  wt_label.c: Widget type 'label'
 */

#include "stfl_internals.h"

#include <string.h>
#include <alloca.h>

static void wt_label_prepare(struct stfl_widget *w, struct stfl_form *f)
{
	const wchar_t * text = stfl_widget_getkv_str(w, L"text", L"");
	w->min_w = wcswidth(text, wcslen(text));
	w->min_h = 1;
}

static void wt_label_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	const wchar_t * text;
	wchar_t * fillup;
	unsigned int i;


	stfl_widget_style(w, f, win);

	text = stfl_widget_getkv_str(w,L"text",L"");

	if (wcswidth(text,wcslen(text)) < w->w) {
		fillup = alloca(sizeof(wchar_t)*(w->w - wcswidth(text,wcslen(text)) + 1));
		for (i=0;i < w->w - wcswidth(text,wcslen(text));++i) {
			fillup[i] = L' ';
		}
		fillup[w->w - wcswidth(text,wcslen(text))] = L'\0';
		mvwaddnwstr(win, w->y, w->x + wcswidth(text,wcslen(text)), fillup, wcswidth(fillup,wcslen(fillup)));
	}

	mvwaddnwstr(win, w->y, w->x, text, w->w);
}

struct stfl_widget_type stfl_widget_type_label = {
	L"label",
	0, // f_init
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_label_prepare,
	wt_label_draw,
	0  // f_process
};

