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
 *  wt_table.c: Widget type 'table'
 */

#include "stfl_internals.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_ROWS 20
#define MAX_COLS 20

struct table_cell_data {
	struct stfl_widget *w;
	struct table_cell_data *mastercell;
	unsigned char vexpand, hexpand, spanpadding;
	unsigned char colspan_nr, rowspan_nr;
	unsigned char colspan, rowspan;
	unsigned char mc_border_l, mc_border_r;
	unsigned char mc_border_t, mc_border_b;
	unsigned char border_l, border_r;
	unsigned char border_t, border_b;
};

struct table_rowcol_data {
	unsigned char min, size;
	unsigned char expand;
};

struct table_data {
	int rows, cols;
	struct table_cell_data *map[MAX_COLS][MAX_ROWS];
	struct table_rowcol_data *rowd;
	struct table_rowcol_data *cold;
};

static void free_table_data(struct table_data *d)
{
	int i, j;

	for (i=0; i < MAX_COLS; i++)
	for (j=0; j < MAX_ROWS; j++)
		if (d->map[i][j])
			free(d->map[i][j]);

	free(d->rowd);
	free(d->cold);
	free(d);
}

static inline int max(int a, int b) {
	return a > b ? a : b;
}

static void wt_table_done(struct stfl_widget *w)
{
	if (w->internal_data)
		free_table_data(w->internal_data);
}

static void wt_table_prepare(struct stfl_widget *w, struct stfl_form *f)
{
	struct table_data *d = calloc(1, sizeof(struct table_data));

	if (w->internal_data)
		free_table_data(w->internal_data);
	w->internal_data = d;

	d->rows = 1;
	int col_counter = 0;
	int row_counter = 0;
	int max_colspan = 0;
	int max_rowspan = 0;
	int i, j;

	struct stfl_widget *c = w->first_child;
	while (c) {
		if (!strcmp(c->type->name, "tablebr")) {
			if (c->next_sibling)
				row_counter++;
			col_counter = 0;
		} else {
			while (d->map[col_counter][row_counter])
				col_counter++;

			assert(col_counter < MAX_COLS && row_counter < MAX_ROWS);

			int colspan = stfl_widget_getkv_int(c, ".colspan", 1);
			int rowspan = stfl_widget_getkv_int(c, ".rowspan", 1);

			max_colspan = max(max_colspan, colspan);
			max_rowspan = max(max_rowspan, rowspan);

			d->cols = max(d->cols, col_counter+colspan);
			d->rows = max(d->rows, row_counter+rowspan);

			const char *expand = stfl_widget_getkv_str(c, ".expand", "vh");
			const char *spacer = stfl_widget_getkv_str(c, ".spacer", "");
			const char *border = stfl_widget_getkv_str(c, ".border", "");

			for (i=col_counter; i<col_counter+colspan; i++)
			for (j=row_counter; j<row_counter+rowspan; j++)
			{
				d->map[i][j] = calloc(1, sizeof(struct table_cell_data));
				d->map[i][j]->mastercell = d->map[col_counter][row_counter];

				if (i != col_counter || j != row_counter)
					d->map[i][j]->spanpadding = 1;

				d->map[i][j]->colspan_nr = i-col_counter;
				d->map[i][j]->rowspan_nr = j-row_counter;

				d->map[i][j]->vexpand = strchr(expand, 'v') != 0;
				d->map[i][j]->hexpand = strchr(expand, 'h') != 0;

				if (i == col_counter) {
					if (strchr(spacer, 'l') != 0) d->map[i][j]->border_l = 1;
					if (strchr(border, 'l') != 0) d->map[i][j]->border_l = 2;
				}

				if (i == col_counter+colspan-1) {
					if (strchr(spacer, 'r') != 0) d->map[i][j]->border_r = 1;
					if (strchr(border, 'r') != 0) d->map[i][j]->border_r = 2;
				}

				if (j == row_counter) {
					if (strchr(spacer, 't') != 0) d->map[i][j]->border_t = 1;
					if (strchr(border, 't') != 0) d->map[i][j]->border_t = 2;
				}

				if (j == row_counter+rowspan-1) {
					if (strchr(spacer, 'b') != 0) d->map[i][j]->border_b = 1;
					if (strchr(border, 'b') != 0) d->map[i][j]->border_b = 2;
				}

				if (i > 0 && d->map[i-1][j])
					d->map[i-1][j]->border_r = d->map[i][j]->border_l = max(d->map[i-1][j]->border_r, d->map[i][j]->border_l);

				if (j > 0 && d->map[i][j-1])
					d->map[i][j-1]->border_b = d->map[i][j]->border_t = max(d->map[i][j-1]->border_b, d->map[i][j]->border_t);

				d->map[i][j]->colspan = colspan;
				d->map[i][j]->rowspan = rowspan;
				d->map[i][j]->w = c;
			}

			col_counter += colspan;
		}
		c->type->f_prepare(c, f);
		c = c->next_sibling;
	}

	d->rowd = calloc(d->rows, sizeof(struct table_rowcol_data));
	d->cold = calloc(d->cols, sizeof(struct table_rowcol_data));

	for (i=1; i<=max_colspan; i++)
	for (row_counter=0; row_counter < d->rows; row_counter++)
	for (col_counter=0; col_counter < d->cols; col_counter++)
	{
		if (d->map[col_counter][row_counter] == 0 ||
		    d->map[col_counter][row_counter]->hexpand == 0 ||
		    d->map[col_counter][row_counter]->spanpadding ||
		    d->map[col_counter][row_counter]->colspan > i)
			continue;

		int expand_ok = 0;
		for (j=0; j < d->map[col_counter][row_counter]->colspan; j++)
			if (d->cold[col_counter+j].expand) {
				expand_ok = 1;
				break;
			}
		if (expand_ok)
			continue;

		for (j=0; j < d->map[col_counter][row_counter]->colspan; j++)
			d->cold[col_counter+j].expand = 1;
	}

	for (i=1; i<=max_rowspan; i++)
	for (row_counter=0; row_counter < d->rows; row_counter++)
	for (col_counter=0; col_counter < d->cols; col_counter++)
	{
		if (d->map[col_counter][row_counter] == 0 ||
		    d->map[col_counter][row_counter]->vexpand == 0 ||
		    d->map[col_counter][row_counter]->spanpadding ||
		    d->map[col_counter][row_counter]->rowspan > i)
			continue;

		int expand_ok = 0;
		for (j=0; j < d->map[col_counter][row_counter]->rowspan; j++)
			if (d->rowd[row_counter+j].expand) {
				expand_ok = 1;
				break;
			}
		if (expand_ok)
			continue;

		for (j=0; j < d->map[col_counter][row_counter]->rowspan; j++)
			d->rowd[row_counter+j].expand = 1;
	}

	for (row_counter=0; row_counter < d->rows; row_counter++)
	for (col_counter=0; col_counter < d->cols; col_counter++)
	{
		struct table_cell_data *m = d->map[col_counter][row_counter];
		m->mastercell->mc_border_l = max(m->mastercell->mc_border_l, m->border_l);
		m->mastercell->mc_border_r = max(m->mastercell->mc_border_r, m->border_r);
		m->mastercell->mc_border_t = max(m->mastercell->mc_border_t, m->border_t);
		m->mastercell->mc_border_b = max(m->mastercell->mc_border_b, m->border_b);
	}

	for (i=1; i<=max_colspan; i++)
	for (row_counter=0; row_counter < d->rows; row_counter++)
	for (col_counter=0; col_counter < d->cols; col_counter++)
	{
		struct table_cell_data *m = d->map[col_counter][row_counter];

		if (m == 0 || m->spanpadding || m->colspan > i)
			continue;

		int min_w = max(m->w->min_w, stfl_widget_getkv_int(m->w, ".width", 1));

		if (col_counter == 0 && m->mc_border_l)
			min_w += 3;

		if (m->mc_border_r)
			min_w += 3;

		int total = min_w;

		for (j=0; j<m->colspan; j++)
			total -= d->cold[col_counter+j].min;

		if (total <= 0)
			continue;

		int expandables = 0;

		for (j=0; j<m->colspan; j++)
			if (d->cold[col_counter+j].expand)
				expandables++;

		if (expandables > 0)
		{
			int per = total / expandables;
			int extra_per = total % expandables;
			for (j=0; j<m->colspan; j++)
				if (d->cold[col_counter+j].expand) {
					d->cold[col_counter+j].min += per;
					if (extra_per) {
						d->cold[col_counter+j].min++;
						extra_per--;
					}
				}
		}
		else
		{
			int per = total / m->colspan;
			int extra_per = total % m->colspan;
			for (j=0; j<m->colspan; j++) {
				d->cold[col_counter+j].min += per;
				if (extra_per) {
					d->cold[col_counter+j].min++;
					extra_per--;
				}
			}
		}
	}

	for (i=1; i<=max_rowspan; i++)
	for (row_counter=0; row_counter < d->rows; row_counter++)
	for (col_counter=0; col_counter < d->cols; col_counter++)
	{
		struct table_cell_data *m = d->map[col_counter][row_counter];

		if (m == 0 || m->spanpadding || m->rowspan > i)
			continue;

		int min_h = max(m->w->min_h, stfl_widget_getkv_int(m->w, ".height", 1));

		if (row_counter == 0 && m->mc_border_t)
			min_h++;

		if (m->mc_border_b)
			min_h++;

		int total = min_h;

		for (j=0; j<m->rowspan; j++)
			total -= d->rowd[row_counter+j].min;

		if (total <= 0)
			continue;

		int expandables = 0;

		for (j=0; j<m->rowspan; j++)
			if (d->rowd[row_counter+j].expand)
				expandables++;

		if (expandables > 0)
		{
			int per = total / expandables;
			int extra_per = total % expandables;
			for (j=0; j<m->rowspan; j++)
				if (d->rowd[row_counter+j].expand) {
					d->rowd[row_counter+j].min += per;
					if (extra_per) {
						d->rowd[row_counter+j].min++;
						extra_per--;
					}
				}
		}
		else
		{
			int per = total / m->rowspan;
			int extra_per = total % m->rowspan;
			for (j=0; j<m->rowspan; j++) {
				d->rowd[row_counter+j].min += per;
				if (extra_per) {
					d->rowd[row_counter+j].min++;
					extra_per--;
				}
			}
		}
	}

	w->min_h = w->min_w = 0;
	for (row_counter=0; row_counter < d->rows; row_counter++)
		w->min_h += d->rowd[row_counter].min;
	for (col_counter=0; col_counter < d->cols; col_counter++)
		w->min_w += d->cold[col_counter].min;
}

void make_corner(WINDOW *win, int x, int y, int left, int right, int up, int down)
{
	switch ((left?01000:0) | (right?00100:0) | (up?00010:0) | (down?00001:0))
	{
	case 00000: // LEFT-RIGHT-UP-DOWN
		break;
	case 00001: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_VLINE);
		break;
	case 00010: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_VLINE);
		break;
	case 00011: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_VLINE);
		break;
	case 00100: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_HLINE);
		break;
	case 00101: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_ULCORNER);
		break;
	case 00110: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_LLCORNER);
		break;
	case 00111: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_LTEE);
		break;
	case 01000: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_HLINE);
		break;
	case 01001: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_URCORNER);
		break;
	case 01010: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_LRCORNER);
		break;
	case 01011: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_RTEE);
		break;
	case 01100: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_HLINE);
		break;
	case 01101: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_TTEE);
		break;
	case 01110: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_BTEE);
		break;
	case 01111: // LEFT-RIGHT-UP-DOWN
		mvwaddch(win, y, x, ACS_PLUS);
		break;
	}
}

static void wt_table_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win)
{
	struct table_data *d = w->internal_data;
	int i, j, k, extra, extra_counter;

	extra = w->h - w->min_h;
	extra_counter = 0;
	for (i=0; i < d->rows; i++) {
		if (d->rowd[i].expand)
			extra_counter++;
	}
	for (i=0; i < d->rows; i++) {
		if (d->rowd[i].expand) {
			int e = extra / extra_counter--;
			d->rowd[i].size = d->rowd[i].min + e;
			extra -= e;
		} else
			d->rowd[i].size = d->rowd[i].min;
	}

	extra = w->w - w->min_w;
	extra_counter = 0;
	for (i=0; i < d->cols; i++) {
		if (d->cold[i].expand)
			extra_counter++;
	}
	for (i=0; i < d->cols; i++) {
		if (d->cold[i].expand) {
			int e = extra / extra_counter--;
			d->cold[i].size = d->cold[i].min + e;
			extra -= e;
		} else
			d->cold[i].size = d->cold[i].min;
	}

	int y = w->y;
	for (j=0; j < d->rows; j++)
	{
		int x = w->x;
		for (i=0; i < d->cols; i++)
		{
			if (d->map[i][j] && !d->map[i][j]->spanpadding)
			{
				struct table_cell_data *m = d->map[i][j];
				struct stfl_widget *c = m->w;

				c->x = x; c->w = 0;
				c->y = y; c->h = 0;

				for (k=i; k < i + d->map[i][j]->colspan; k++)
					c->w += d->cold[k].size;

				for (k=j; k < j + d->map[i][j]->rowspan; k++)
					c->h += d->rowd[k].size;

				if (m->mc_border_l && i == 0) {
					c->x += 3;
					c->w -= 3;
				}

				if (m->mc_border_t && j == 0) {
					c->y++;
					c->h--;
				}

				if (m->mc_border_r)
					c->w -= 3;

				if (m->mc_border_b)
					c->h--;

				c->type->f_draw(c, f, win);
			}
			x += d->cold[i].size;
		}
		y += d->rowd[j].size;
	}

	stfl_widget_style(w, f, win);

	y = w->y;
	for (j=0; j < d->rows; j++)
	{
		int x = w->x;
		for (i=0; i < d->cols; i++)
		{
			if (d->map[i][j])
			{
				struct table_cell_data *m = d->map[i][j];
				int box_x = x, box_w = d->cold[i].size;
				int box_y = y, box_h = d->rowd[j].size;

				if (i == 0) {
					if (m->border_l > 1 && box_h > (j ? 1 : 2)) {
						wmove(win, box_y+(j ? 0 : 1), box_x+1);
						wvline(win, ACS_VLINE, box_h - (j ? 1 : 2));
					}
				} else {
					box_x -= 3;
					box_w += 3;
				}

				if (j == 0) {
					if (m->border_t > 1 && box_w > 4) {
						wmove(win, box_y, box_x+2);
						whline(win, ACS_HLINE, box_w-4);
					}
				} else {
					box_y--;
					box_h++;
				}

				if (m->border_r > 1 && box_h > 2) {
					wmove(win, box_y+1, box_x+box_w-2);
					wvline(win, ACS_VLINE, box_h-2);
				}

				if (m->border_b > 1 && box_w > 4) {
					wmove(win, box_y+box_h-1, box_x+2);
					whline(win, ACS_HLINE, box_w-4);
				}

				int left, right, up, down;

				struct table_cell_data *left_m = i > 0 ? d->map[i-1][j] : 0;
				struct table_cell_data *right_m = i < d->cols-1 ? d->map[i+1][j] : 0;

				struct table_cell_data *up_m = j > 0 ? d->map[i][j-1] : 0;
				struct table_cell_data *down_m = j < d->rows-1 ? d->map[i][j+1] : 0;

				// upper left corner
				if (i == 0 && j == 0) {
					left = left_m ? left_m->border_t : 0;
					right = m->border_t;
					up = up_m ? up_m->border_l : 0;
					down = m->border_l;
					make_corner(win, box_x+1, box_y, left>1, right>1, up>1, down>1);
				}

				// lower left corner
				if (i == 0) {
					left = left_m ? left_m->border_b : 0;
					right = m->border_b;
					up = m->border_l;
					down = down_m ? down_m->border_l : 0;
					make_corner(win, box_x+1, box_y+box_h-1, left>1, right>1, up>1, down>1);
				}

				// upper right corner
				if (j == 0) {
					left = m->border_t;
					right = right_m ? right_m->border_t : 0;
					up = up_m ? up_m->border_r : 0;
					down = m->border_r;
					make_corner(win, box_x+box_w-2, box_y, left>1, right>1, up>1, down>1);
				}

				// lower right corner
				left = m->border_b;
				right = right_m ? right_m->border_b : 0;
				up = m->border_r;
				down = down_m ? down_m->border_r : 0;
				make_corner(win, box_x+box_w-2, box_y+box_h-1, left>1, right>1, up>1, down>1);
			}
			x += d->cold[i].size;
		}
		y += d->rowd[j].size;
	}
}

struct stfl_widget_type stfl_widget_type_table = {
	"table",
	0, // f_init
	wt_table_done,
	0, // f_enter 
	0, // f_leave
	wt_table_prepare,
	wt_table_draw,
	0  // f_process
};

static void wt_tablebr_prepare(struct stfl_widget *w, struct stfl_form *f) { }
static void wt_tablebr_draw(struct stfl_widget *w, struct stfl_form *f, WINDOW *win) { }

struct stfl_widget_type stfl_widget_type_tablebr = {
	"tablebr",
	0, // f_init
	0, // f_done
	0, // f_enter 
	0, // f_leave
	wt_tablebr_prepare,
	wt_tablebr_draw,
	0  // f_process
};

