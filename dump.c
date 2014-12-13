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
 *  dump.c: Create STFL code from a widget tree
 */

#include "stfl_internals.h"
#include "stfl_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct txtnode {
	struct txtnode *prev;
	wchar_t *value;
	int len;
};

static void newtxt(struct txtnode **o, const wchar_t *fmt, ...)
		/* __attribute__ ((format (wprintf, 2, 3))) */
{
	struct txtnode *n = calloc(1, sizeof(struct txtnode));

	n->prev = *o;
	*o = n;

	va_list ap;
	va_start(ap, fmt);
	wchar_t *buf = malloc(4096 * sizeof(wchar_t));
	int buf_len = 4096;
	while (1) {
		int rc = vswprintf(buf, buf_len, fmt, ap);
		if (rc < 0) {
			free(buf);
			buf = NULL;
			break;
		}
		if ((rc + 1) < buf_len) {
			buf = realloc(buf, (rc + 1) * sizeof(wchar_t));
			break;
		}
		buf_len = buf_len * 2;
		buf = realloc(buf, buf_len * sizeof(wchar_t));
	}
	n->value = buf;
	if (n->value)
		n->len = wcslen(n->value);
	else
		n->len = 0;
	va_end(ap);
}

static void myquote(struct txtnode **txt, const wchar_t *text)
{
	wchar_t q[2] = {L'"', 0};
	int segment_len;

	if (wcscspn(text, L"'") > wcscspn(text, L"\""))
		q[0] = L'\'';

	while (*text) {
		segment_len = wcscspn(text, q);
		newtxt(txt, L"%c%.*ls%c", q[0], segment_len, text, q[0]);
		q[0] = q[0] == L'"' ? L'\'' : L'"';
		text += segment_len;
	}
}

static void mydump(struct stfl_widget *w, const wchar_t *prefix, int focus_id, struct txtnode **txt)
{
	newtxt(txt, L"{%ls%ls", w->id == focus_id ? L"!" : L"", w->type->name);

	if (w->cls)
		newtxt(txt, L"#%ls", w->cls);

	if (w->name) {
		newtxt(txt, L"[");
		myquote(txt, prefix);
		myquote(txt, w->name);
		newtxt(txt, L"]");
	}

	struct stfl_kv *kv = w->kv_list;
	while (kv)
	{
		if (kv->name) {
			newtxt(txt, L" %ls[", kv->key);
			myquote(txt, prefix);
			myquote(txt, kv->name);
			newtxt(txt, L"]:");
		} else
			newtxt(txt, L" %ls:", kv->key);

		myquote(txt, kv->value);
		kv = kv->next;
	}

	struct stfl_widget *c = w->first_child;
	while (c) {
		mydump(c, prefix, focus_id, txt);
		c = c->next_sibling;
	}

	newtxt(txt, L"}");
}

static void mytext(struct stfl_widget *w, struct txtnode **txt)
{
	if (!wcscmp(w->type->name, L"listitem"))
	{
		struct stfl_kv *kv = w->kv_list;
		while (kv) {
			if (!wcscmp(kv->key, L"text"))
				newtxt(txt, L"%ls\n", kv->value);
			kv = kv->next;
		}
	}

	struct stfl_widget *c = w->first_child;
	while (c) {
		mytext(c, txt);
		c = c->next_sibling;
	}
}

static wchar_t *txt2string(struct txtnode *txt)
{
	int string_len = 0;
	struct txtnode *t, *prev;

	for (t=txt; t; t=t->prev)
		string_len += t->len;

	wchar_t *string = malloc(sizeof(wchar_t)*(string_len+1));
	int i = string_len;

	for (t=txt; t; t=prev)
	{
		i -= t->len;
		wmemcpy(string+i, t->value, t->len);

		prev = t->prev;
		free(t->value);
		free(t);
	}

	string[string_len] = 0;
	return string;
}

wchar_t *stfl_quote_backend(const wchar_t *text)
{
	struct txtnode *txt = 0;
	myquote(&txt, text);
	return txt2string(txt);
}

wchar_t *stfl_widget_dump(struct stfl_widget *w, const wchar_t *prefix, int focus_id)
{
	struct txtnode *txt = 0;
	mydump(w, prefix, focus_id, &txt);
	return txt2string(txt);
}

wchar_t *stfl_widget_text(struct stfl_widget *w)
{
	struct txtnode *txt = 0;
	mytext(w, &txt);
	return txt2string(txt);
}

