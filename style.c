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
 *  style.c: Helper functions for text colors and attributes
 */

#include "stfl_internals.h"

#include <string.h>
#include <stdlib.h>

static int stfl_colorpair_begin = -1;
static int stfl_colorpair_counter = -1;

void stfl_style(WINDOW *win, const char *style)
{
	int bg_color = -1, fg_color = -1, attr = A_NORMAL;

	style += strspn(style, " \t");

	while (*style)
	{
		int field_len = strcspn(style, ",");
		char field[field_len+1];
		memcpy(field, style, field_len);
		field[field_len] = 0;
		style += field_len;

		if (*style == ',')
			style++;

		char *sepp = field;
		char *key = strsep(&sepp, "=");
		char *value = strsep(&sepp, "");

		if (!key || !value) continue;

		key += strspn(key, " \t");
		key = strsep(&key, " \t");

		value += strspn(value, " \t");
		value = strsep(&value, " \t");

		if (!strcmp(key, "bg") || !strcmp(key, "fg"))
		{
			int color = -1;
			if (!strcmp(value, "black"))
				color = COLOR_BLACK;
			else
			if (!strcmp(value, "red"))
				color = COLOR_RED;
			else
			if (!strcmp(value, "green"))
				color = COLOR_GREEN;
			else
			if (!strcmp(value, "yellow"))
				color = COLOR_YELLOW;
			else
			if (!strcmp(value, "blue"))
				color = COLOR_BLUE;
			else
			if (!strcmp(value, "magenta"))
				color = COLOR_MAGENTA;
			else
			if (!strcmp(value, "cyan"))
				color = COLOR_CYAN;
			else
			if (!strcmp(value, "white"))
				color = COLOR_WHITE;
			else {
				fprintf(stderr, "STFL Style Error: Unknown %s color: '%s'\n", key, value);
				abort();
			}

			if (!strcmp(key, "bg"))
				bg_color = color;
			else
				fg_color = color;
		}
		else
		if (!strcmp(key, "attr"))
		{
			if (!strcmp(value, "standout"))
				attr |= A_STANDOUT;
			else
			if (!strcmp(value, "underline"))
				attr |= A_UNDERLINE;
			else
			if (!strcmp(value, "reverse"))
				attr |= A_REVERSE;
			else
			if (!strcmp(value, "blink"))
				attr |= A_BLINK;
			else
			if (!strcmp(value, "dim"))
				attr |= A_DIM;
			else
			if (!strcmp(value, "bold"))
				attr |= A_BOLD;
			else
			if (!strcmp(value, "protect"))
				attr |= A_PROTECT;
			else
			if (!strcmp(value, "invis"))
				attr |= A_INVIS;
			else {
				fprintf(stderr, "STFL Style Error: Unknown attribute: '%s'\n", value);
				abort();
			}
		}
		else {
			fprintf(stderr, "STFL Style Error: Unknown keyword: '%s'\n", key);
			abort();
		}
	}

	int i;
	short f, b;

	if (stfl_colorpair_begin < 0)
		stfl_colorpair_begin = COLOR_PAIRS-1;

	if (stfl_colorpair_counter < 0)
		stfl_colorpair_counter = stfl_colorpair_begin;

	for (i=stfl_colorpair_begin; i>stfl_colorpair_counter; i--) {
		pair_content(i, &f, &b);
		if ((f == fg_color || (f == 255 && fg_color == -1)) &&
		    (b == bg_color || (b == 255 && bg_color == -1)))
			break;
	}

	if (i == stfl_colorpair_counter) {
		if (stfl_colorpair_counter > 16) {
			init_pair(i, fg_color, bg_color);
			stfl_colorpair_counter--;
		} else
			i = 0;
	}

	wattrset(win, attr | COLOR_PAIR(i));
}

void stfl_widget_style(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	const char *style = "";

	if (f->current_focus_id == w->id)
		style = stfl_widget_getkv_str(w, "style_focus", "");

	if (*style == 0)
		style = stfl_widget_getkv_str(w, "style_normal", "");

	stfl_style(win, style);
}

