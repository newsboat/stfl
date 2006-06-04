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
 *  wt_hbox.c: Widget type 'hbox'
 */

#include "stfl.h"

static void wt_hbox_getminwh(struct stfl_widget *w)
{
	w->min_w = 0;
	w->min_h = 0;

	struct stfl_widget *c = w->first_child;
	while (c) {
		c->type->f_getminwh(c);
		if (w->min_h < c->min_h)
			w->min_h = c->min_h;
		w->min_w += c->min_w;
		c = c->next_sibling;
	}
}

static void wt_hbox_draw(struct stfl_widget *w, WINDOW *win)
{
	int num_children = 0, i;
	int sizes_total = 0;

	struct stfl_widget *c = w->first_child;
	while (c) {
		num_children++;
		sizes_total += c->min_w;
		c = c->next_sibling;
	}

	int sizes_extra = w->w - sizes_total;
	int current_x = w->x;

	c = w->first_child;
	for (i=0; c; i++) {
		int extra = sizes_extra / (num_children-i);
		sizes_extra -= extra;
		
		c->y = w->y;
		c->x = current_x;
		c->h = w->h;
		c->w = c->min_w + extra;
		current_x += c->w;

		c->type->f_draw(c, win);
		c = c->next_sibling;
	}
}

struct stfl_widget_type stfl_widget_type_hbox = {
	"hbox",
	0, // f_init
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_hbox_getminwh,
	wt_hbox_draw,
	0 // f_run
};

