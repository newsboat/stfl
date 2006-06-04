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
 *  wt_label.c: Widget type 'label'
 */

#include "stfl.h"

#include <string.h>

static void wt_label_getminwh(struct stfl_widget *w)
{
	struct stfl_kv *val = stfl_widget_getkv(w, "text");
	w->min_w = !val || !val->value[0] ? 1 : strlen(val->value);
	w->min_h = 1;
}

static void wt_label_draw(struct stfl_widget *w, WINDOW *win)
{
	struct stfl_kv *val = stfl_widget_getkv(w, "text");
	if (val)
		mvwaddstr(win, w->y, w->x, val->value);
}

struct stfl_widget_type stfl_widget_type_label = {
	"label",
	0, // f_init
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_label_getminwh,
	wt_label_draw,
	0 // f_run
};

