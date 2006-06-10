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
 *  dump.c: Create STFL code from a widget tree
 */

#define _GNU_SOURCE

#include "stfl_internals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct txtnode {
	struct txtnode *prev;
	char *value;
	int len;
};

static void newtxt(struct txtnode **o, const char *fmt, ...)
{
	struct txtnode *n = calloc(1, sizeof(struct txtnode));

	n->prev = *o;
	*o = n;

	va_list ap;
	va_start(ap, fmt);
	vasprintf(&n->value, fmt, ap);
	n->len = strlen(n->value);
	va_end(ap);
}

static void myquote(struct txtnode **txt, const char *text)
{
	char q[2] = {'"', 0};
	int segment_len;

	if (strcspn(text, "'") > strcspn(text, "\""))
		q[0] = '\'';

	while (*text) {
		segment_len = strcspn(text, q);
		newtxt(txt, "%c%.*s%c", q[0], segment_len, text, q[0]);
		q[0] = q[0] == '"' ? '\'' : '"';
		text += segment_len;
	}
}

static void mydump(struct stfl_widget *w, const char *prefix, int focus_id, struct txtnode **txt)
{
	newtxt(txt, "{%s%s", w->id == focus_id ? "!" : "", w->type->name);

	if (w->cls)
		newtxt(txt, "#%s", w->cls);

	if (w->name)
		newtxt(txt, "[%s%s]", prefix, w->name);

	struct stfl_kv *kv = w->kv_list;
	while (kv)
	{
		if (kv->name)
			newtxt(txt, " %s[%s%s]:", kv->key, prefix, kv->name);
		else
			newtxt(txt, " %s:", kv->key);

		myquote(txt, kv->value);
		kv = kv->next;
	}

	struct stfl_widget *c = w->first_child;
	while (c) {
		mydump(c, prefix, focus_id, txt);
		c = c->next_sibling;
	}

	newtxt(txt, "}");
}

static char *txt2string(struct txtnode *txt)
{
	int string_len = 0;
	struct txtnode *t, *prev;

	for (t=txt; t; t=t->prev)
		string_len += t->len;

	char *string = malloc(string_len+1);
	int i = string_len;

	for (t=txt; t; t=prev)
	{
		i -= t->len;
		memcpy(string+i, t->value, t->len);

		prev = t->prev;
		free(t->value);
		free(t);
	}

	string[string_len] = 0;
	return string;
}

char *stfl_quote_backend(const char *text)
{
	struct txtnode *txt = 0;
	myquote(&txt, text);
	return txt2string(txt);
}

char *stfl_widget_dump(struct stfl_widget *w, const char *prefix, int focus_id)
{
	struct txtnode *txt = 0;
	mydump(w, prefix, focus_id, &txt);
	return txt2string(txt);
}

