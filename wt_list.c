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

static void wt_list_prepare(struct stfl_widget *w, struct stfl_form *f)
{
	struct stfl_widget *c = w->first_child;
	
	w->min_w = 1;
	w->min_h = 5;

	while (c) {
		int len = strlen(stfl_widget_getkv_str(c, "text", ""));
		w->min_w = len > w->min_w ? len : w->min_w;
		c = c->next_sibling;
	}
}

static void wt_list_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	stfl_widget_style(w, f, win);

	int offset = stfl_widget_getkv_int(w, "offset", 0);
	// int pos = stfl_widget_getkv_int(w, "pos", 0);

	struct stfl_widget *c;
	int i;

	for (i=0, c=w->first_child; c && i < offset+w->h; i++, c=c->next_sibling)
	{
		if (i < offset)
			continue;

		mvwaddnstr(win, w->y+i-offset, w->x,
				stfl_widget_getkv_str(c, "text", ""), w->w);
	}
}

struct stfl_widget_type stfl_widget_type_list = {
	"list",
	0, // f_init
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_list_prepare,
	wt_list_draw,
	0  // f_process
};

static void wt_listitem_prepare(struct stfl_widget *w, struct stfl_form *f) { }
static void wt_listitem_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win) { }

struct stfl_widget_type stfl_widget_type_listitem = {
	"listitem",
	0, // f_init
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_listitem_prepare,
	wt_listitem_draw,
	0  // f_process
};

