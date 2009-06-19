/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006, 2007  Clifford Wolf <clifford@clifford.at>
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
 *  style.c: Helper functions for text colors and attributes
 */

#include "stfl_internals.h"

#include <string.h>
#include <stdlib.h>
#include <wchar.h>

static wchar_t *wcssep(wchar_t **stringp, const wchar_t *delim) {
	register wchar_t *tmp=*stringp;
	register wchar_t *tmp2=tmp;
	register const wchar_t *tmp3;
	if (!*stringp) return 0;
	for (tmp2=tmp; *tmp2; ++tmp2) {
		for (tmp3=delim; *tmp3; ++tmp3)
			if (*tmp2==*tmp3) {       /* delimiter found */
				*tmp2=0;
				*stringp=tmp2+1;
				return tmp;
			}
	}
	*stringp=0;
	return tmp;
}


#define STFL_MAX_COLOR_PAIRS 256
static int stfl_colorpair_bg[STFL_MAX_COLOR_PAIRS];
static int stfl_colorpair_fg[STFL_MAX_COLOR_PAIRS];
int stfl_colorpair_counter = 1;

void stfl_style(WINDOW *win, const wchar_t *style)
{
	int bg_color = -1, fg_color = -1, attr = A_NORMAL;

	style += wcsspn(style, L" \t");

	while (*style)
	{
		int field_len = wcscspn(style, L",");
		wchar_t field[field_len+1];
		wmemcpy(field, style, field_len);
		field[field_len] = 0;
		style += field_len;

		if (*style == L',')
			style++;

		wchar_t *sepp = field;
		wchar_t *key = wcssep(&sepp, L"=");
		wchar_t *value = wcssep(&sepp, L"");

		if (!key || !value) continue;

		key += wcsspn(key, L" \t");
		key = wcssep(&key, L" \t");

		value += wcsspn(value, L" \t");
		value = wcssep(&value, L" \t");

		if (!wcscmp(key, L"bg") || !wcscmp(key, L"fg"))
		{
			int color = -1;
			if (!wcscmp(value, L"black"))
				color = COLOR_BLACK;
			else
			if (!wcscmp(value, L"red"))
				color = COLOR_RED;
			else
			if (!wcscmp(value, L"green"))
				color = COLOR_GREEN;
			else
			if (!wcscmp(value, L"yellow"))
				color = COLOR_YELLOW;
			else
			if (!wcscmp(value, L"blue"))
				color = COLOR_BLUE;
			else
			if (!wcscmp(value, L"magenta"))
				color = COLOR_MAGENTA;
			else
			if (!wcscmp(value, L"cyan"))
				color = COLOR_CYAN;
			else
			if (!wcscmp(value, L"white"))
				color = COLOR_WHITE;
			else
			if (!wcsncmp(value, L"color", 5)) {
				color = wcstoul(value + 5, NULL, 0);
			}
			else {
				fprintf(stderr, "STFL Style Error: Unknown %ls color: '%ls'\n", key, value);
				abort();
			}

			if (!wcscmp(key, L"bg"))
				bg_color = color;
			else
				fg_color = color;
		}
		else
		if (!wcscmp(key, L"attr"))
		{
			if (!wcscmp(value, L"standout"))
				attr |= A_STANDOUT;
			else
			if (!wcscmp(value, L"underline"))
				attr |= A_UNDERLINE;
			else
			if (!wcscmp(value, L"reverse"))
				attr |= A_REVERSE;
			else
			if (!wcscmp(value, L"blink"))
				attr |= A_BLINK;
			else
			if (!wcscmp(value, L"dim"))
				attr |= A_DIM;
			else
			if (!wcscmp(value, L"bold"))
				attr |= A_BOLD;
			else
			if (!wcscmp(value, L"protect"))
				attr |= A_PROTECT;
			else
			if (!wcscmp(value, L"invis"))
				attr |= A_INVIS;
			else {
				fprintf(stderr, "STFL Style Error: Unknown attribute: '%ls'\n", value);
				abort();
			}
		}
		else {
			fprintf(stderr, "STFL Style Error: Unknown keyword: '%ls'\n", key);
			abort();
		}
	}

	short f, b;
	pair_content(0, &f, &b);

	if (fg_color < 0 || fg_color >= COLORS)
		fg_color = f;

	if (bg_color < 0 || bg_color >= COLORS)
		bg_color = b;

	int i;
	for (i=1; i<stfl_colorpair_counter; i++) {
		if (stfl_colorpair_fg[i] == fg_color && stfl_colorpair_bg[i] == bg_color)
			break;
	}

	if (i == stfl_colorpair_counter) {
		if (i == COLOR_PAIRS) {
			fprintf(stderr, "Ncurses limit of color pairs (%d) reached!\n", COLOR_PAIRS);
			abort();
		}
		if (i == STFL_MAX_COLOR_PAIRS) {
			fprintf(stderr, "Internal STFL limit of color pairs (%d) reached!\n", STFL_MAX_COLOR_PAIRS);
			abort();
		}
		init_pair(i, fg_color, bg_color);
		stfl_colorpair_fg[i] = fg_color;
		stfl_colorpair_bg[i] = bg_color;
		stfl_colorpair_counter++;
	}

	wattrset(win, attr);
	wcolor_set(win, i, NULL);
}

void stfl_widget_style(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	const wchar_t *style = L"";

	if (f->current_focus_id == w->id)
		style = stfl_widget_getkv_str(w, L"style_focus", L"");

	if (*style == 0)
		style = stfl_widget_getkv_str(w, L"style_normal", L"");

	stfl_style(win, style);
}

