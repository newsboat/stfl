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

#define STFL_PRIVATE 1
#include "stfl.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_ROWS 20
#define MAX_COLS 20

struct table_cell_data {
	struct stfl_widget *w;
	int vexpand, hexpand, spanpadding;
	int colspan_nr, rowspan_nr;
	int colspan, rowspan;
};

struct table_rowcol_data {
	int min, size;
	int expand;
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

			if (col_counter >= d->cols)
				d->cols = col_counter+1;
			if (row_counter >= d->rows)
				d->rows = row_counter+1;

			int colspan = stfl_widget_getkv_int(c, ".colspan", 1);
			int rowspan = stfl_widget_getkv_int(c, ".rowspan", 1);

			if (colspan > max_colspan)
				max_colspan = colspan;

			if (rowspan > max_rowspan)
				max_rowspan = rowspan;

			for (i=col_counter; i<col_counter+colspan; i++)
			for (j=row_counter; j<row_counter+rowspan; j++)
			{
				d->map[i][j] = calloc(1, sizeof(struct table_cell_data));

				if (i != col_counter || j != row_counter)
					d->map[i][j]->spanpadding = 1;

				d->map[i][j]->colspan_nr = i-col_counter;
				d->map[i][j]->rowspan_nr = j-row_counter;

				char *expand = stfl_widget_getkv_str(c, ".expand", "vh");
				d->map[i][j]->vexpand = strchr(expand, 'v') != 0;
				d->map[i][j]->hexpand = strchr(expand, 'h') != 0;

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

	for (i=1; i<max_colspan; i++)
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

	for (i=1; i<max_rowspan; i++)
	for (row_counter=0; row_counter < d->rows; row_counter++)
	for (col_counter=0; col_counter < d->cols; col_counter++)
	{
		if (d->map[col_counter][row_counter] == 0 ||
		    d->map[col_counter][row_counter]->vexpand == 0 ||
		    d->map[col_counter][row_counter]->spanpadding ||
		    d->map[col_counter][row_counter]->rowspan > i)
			continue;

		int expand_ok = 0;
		for (j=0; j < d->map[col_counter][row_counter]->colspan; j++)
			if (d->rowd[row_counter+j].expand) {
				expand_ok = 1;
				break;
			}
		if (expand_ok)
			continue;

		for (j=0; j < d->map[col_counter][row_counter]->colspan; j++)
			d->rowd[row_counter+j].expand = 1;
	}

	for (row_counter=0; row_counter < d->rows; row_counter++)
	for (col_counter=0; col_counter < d->cols; col_counter++)
	{
		int min_w = d->map[col_counter][row_counter]->w->min_w;
		int colspan = d->map[col_counter][row_counter]->colspan;
		int colspan_nr = d->map[col_counter][row_counter]->colspan_nr;
		int size_w = stfl_widget_getkv_int(d->map[col_counter][row_counter]->w, ".width", 1);
		if (size_w > min_w) min_w = size_w;
		min_w = min_w / colspan + (colspan_nr < min_w % colspan ? 1 : 0);

		int min_h = d->map[col_counter][row_counter]->w->min_h;
		int rowspan = d->map[col_counter][row_counter]->rowspan;
		int rowspan_nr = d->map[col_counter][row_counter]->rowspan_nr;
		int size_h = stfl_widget_getkv_int(d->map[col_counter][row_counter]->w, ".height", 1);
		if (size_h > min_h) min_h = size_h;
		min_h = min_h / rowspan + (rowspan_nr < min_h % rowspan ? 1 : 0);

		if (d->cold[col_counter].min < min_w)
			d->cold[col_counter].min = min_w;

		if (d->rowd[row_counter].min < min_h)
			d->rowd[row_counter].min = min_h;
	}

	w->min_h = w->min_w = 0;
	for (row_counter=0; row_counter < d->rows; row_counter++)
		w->min_h += d->rowd[row_counter].min;
	for (col_counter=0; col_counter < d->cols; col_counter++)
		w->min_w += d->cold[col_counter].min;
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
				struct stfl_widget *c = d->map[i][j]->w;

				c->x = x; c->w = 0;
				c->y = y; c->h = 0;

				for (k=i; k < i + d->map[i][j]->colspan; k++)
					c->w += d->cold[k].size;

				for (k=j; k < j + d->map[i][j]->rowspan; k++)
					c->h += d->rowd[k].size;

				c->type->f_draw(c, f, win);
			}
			x += d->cold[i].size;
		}
		y += d->rowd[j].size;
	}

	// stfl_widget_style(w, f, win);
	// FIXME: draw borders
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

