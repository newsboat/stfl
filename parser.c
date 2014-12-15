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
 *  parser.c: STFL Form description file parser
 */

#include "stfl_internals.h"
#include "stfl_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MYWCSCSPN_SKIP_QUOTED	0x01
#define MYWCSCSPN_SKIP_NAMES	0x02

static size_t mywcscspn(const wchar_t *wcs, const wchar_t *reject, int flags)
{
	enum {
		PLAIN,
		NAME_BLOCK,
		SINGLE_QUOTE,
		SINGLE_QUOTE_NAME,
		DOUBLE_QUOTE,
		DOUBLE_QUOTE_NAME,
	} state = PLAIN;

	int len = 0;
	int i;

	while (1)
	{
		if (!wcs[len])
			return len;

		switch (state)
		{
		case PLAIN:
			if ((flags & MYWCSCSPN_SKIP_NAMES) && (wcs[len] == L'[')) {
				state = NAME_BLOCK;
				break;
			}
			if ((flags & MYWCSCSPN_SKIP_QUOTED) && (wcs[len] == L'\'')) {
				state = SINGLE_QUOTE;
				break;
			}
			if ((flags & MYWCSCSPN_SKIP_QUOTED) && (wcs[len] == L'\"')) {
				state = DOUBLE_QUOTE;
				break;
			}
			for (i=0; reject[i]; i++)
				if (wcs[len] == reject[i])
					return len;
			break;
		case NAME_BLOCK:
			if ((flags & MYWCSCSPN_SKIP_QUOTED) && (wcs[len] == L'\'')) {
				state = SINGLE_QUOTE_NAME;
				break;
			}
			if ((flags & MYWCSCSPN_SKIP_QUOTED) && (wcs[len] == L'\"')) {
				state = DOUBLE_QUOTE_NAME;
				break;
			}
			if (wcs[len] == L']')
				state = PLAIN;
			break;
		case SINGLE_QUOTE:
		case SINGLE_QUOTE_NAME:
			if (wcs[len] == L'\'')
				state = state == SINGLE_QUOTE ? PLAIN : NAME_BLOCK;
			break;
		case DOUBLE_QUOTE:
		case DOUBLE_QUOTE_NAME:
			if (wcs[len] == L'\"')
				state = state == DOUBLE_QUOTE ? PLAIN : NAME_BLOCK;
			break;
		}

		len++;
	}
}

static wchar_t *unquote(const wchar_t *text, int tlen)
{
	int len_v = 0, i, j;
	wchar_t *value;

	if (!text)
		return 0;

	for (i=0; (i<tlen || tlen<0) && text[i]; i++)
	{
		if (text[i] == L'\'')
			while (1) {
				if (++i == tlen && tlen >= 0)
					goto finish_len_v_loop;
				if (!text[i] || text[i] == L'\'') break;
				len_v++;
			}
		else
		if (text[i] == L'\"')
			while (1) {
				if (++i == tlen && tlen >= 0)
					goto finish_len_v_loop;
				if (!text[i] || text[i] == L'\"') break;
				len_v++;
			}
		else
			len_v++;
finish_len_v_loop:;
	}

	value = malloc(sizeof(wchar_t)*(len_v+1));

	for (i=j=0; text[i] && (i<tlen || tlen<0); i++)
	{
		if (text[i] == L'\'')
			while (1) {
				if (++i == tlen && tlen >= 0)
					goto finish_copy_loop;
				if (!text[i] || text[i] == L'\'') break;
				value[j++] = text[i];
			}
		else
		if (text[i] == L'\"')
			while (1) {
				if (++i == tlen && tlen >= 0)
					goto finish_copy_loop;
				if (!text[i] || text[i] == L'\"') break;
				value[j++] = text[i];
			}
		else
			value[j++] = text[i];
finish_copy_loop:;
	}

	value[j] = 0;
	assert(j == len_v);

	return value;
}

static void extract_name(wchar_t **key, wchar_t **name)
{
	int len = wcscspn(*key, L"[");

	if ((*key)[len] == 0) {
		*name = 0;
		return;
	}

	*name = compat_wcsdup(*key+len+1);
	*key = realloc(*key, sizeof(wchar_t)*(len+1));
	(*key)[len] = 0;

	len = mywcscspn(*name, L"]", MYWCSCSPN_SKIP_QUOTED);
	(*name)[len] = 0;
}

static void extract_class(wchar_t **key, wchar_t **cls)
{
	int len = wcscspn(*key, L"#");

	if ((*key)[len] == 0) {
		*cls = 0;
		return;
	}

	*cls = compat_wcsdup(*key+len+1);
	*key = realloc(*key, sizeof(wchar_t)*(len+1));
	(*key)[len] = 0;
}

static int read_type(const wchar_t **text, wchar_t **type, wchar_t **name, wchar_t **cls)
{
	int len = mywcscspn(*text, L" \t\r\n:{}", MYWCSCSPN_SKIP_QUOTED|MYWCSCSPN_SKIP_NAMES);

	if ((*text)[len] == L':' || len == 0)
		return 0;

	*type = malloc((len+1)*sizeof(wchar_t));
	wmemcpy(*type, *text, len);
	(*type)[len] = 0;
	*text += len;

	extract_name(type, name);
	extract_class(type, cls);

	return 1;
}

static int read_kv(const wchar_t **text, wchar_t **key, wchar_t **name, wchar_t **value)
{
	int len_k = mywcscspn(*text, L" \t\r\n:{}", MYWCSCSPN_SKIP_QUOTED|MYWCSCSPN_SKIP_NAMES);

	if ((*text)[len_k] != L':' || len_k == 0)
		return 0;

	*key = malloc((len_k+1)*sizeof(wchar_t));
	wmemcpy(*key, *text, len_k);
	(*key)[len_k] = 0;
	*text += len_k+1;

	extract_name(key, name);

	int qval_len = mywcscspn(*text, L" \t\r\n{}", MYWCSCSPN_SKIP_QUOTED);
	*value = unquote(*text, qval_len);
	*text += qval_len;

	return 1;
}

struct stfl_widget *stfl_parser(const wchar_t *text)
{
	struct stfl_widget *root = 0;
	struct stfl_widget *current = 0;
	int bracket_indenting = -1;
	int bracket_level = 0;

	while (1)
	{
		int indenting = 0;

		if (bracket_indenting >= 0)
		{
			while (*text == L' ' || *text == L'\t') text++;

			while (*text == L'}') {
				bracket_level--; text++;
				while (*text == L' ' || *text == L'\t') text++;
			}

			while (*text == L'{') {
				bracket_level++; text++;
				while (*text == L' ' || *text == L'\t') text++;
			}

			if (bracket_level == 0)
				bracket_indenting = -1;

			if (bracket_level < 0)
				goto parser_error;
		}
		else
			if (*text == L'}')
				goto parser_error;

		if (bracket_indenting >= 0)
		{
			while (*text == L' ' || *text == L'\t')
				text++;

			if (*text == L'\r' || *text == L'\n')
				goto parser_error;

			indenting = bracket_indenting + (bracket_level-1);
		}
		else
		{
			while (*text == L' ' || *text == L'\t' || *text == L'\r' || *text == L'\n') {
				if (*text == L'\r' || *text == L'\n')
					indenting = 0;
				else
				if (*text == L'\t')
					indenting = -1;
				else
				if (indenting >= 0)
					indenting++;
				text++;
			}

			if (*text == L'*') {
				while (*text && *text != L'\r' && *text != L'\n')
					text++;
				continue;
			}

			if (*text == L'{') {
				bracket_indenting = indenting;
				continue;
			}
		}

		if (*text == 0)
			break;

		wchar_t *key, *name, *cls, *value;
		if (indenting < 0)
			goto parser_error;

		if (*text == L'<')
		{
			int filename_len = wcscspn(++text, L">");
			wchar_t wfn[filename_len+1];

			wmemcpy(wfn, text, filename_len+1);
			wfn[filename_len] = 0;

			size_t len = wcstombs(NULL,wfn,0)+1;
			char filename[len];
			size_t rc = wcstombs(filename, wfn, len);
			assert(rc != (size_t)-1);

			text += filename_len;
			if (*text) text++;

			struct stfl_widget *n = stfl_parser_file(filename);
			if (!n) return 0;

			if (root)
			{
				while (current->parser_indent >= indenting) {
					current = current->parent;
					if (!current)
						goto parser_error;
				}

				n->parent = current;
				if (current->last_child) {
					current->last_child->next_sibling = n;
					current->last_child = n;
				} else {
					current->first_child = n;
					current->last_child = n;
				}

				n->parser_indent = indenting;
				current = n;
			}
			else
				root = n;
		}
		else
		if (root)
		{
			while (current->parser_indent >= indenting) {
				current = current->parent;
				if (!current)
					goto parser_error;
			}

			if (read_type(&text, &key, &name, &cls) == 1)
			{
				struct stfl_widget *n = stfl_widget_new(key);
				if (!n)
					goto parser_error;
				free(key);

				n->parent = current;
				if (current->last_child) {
					current->last_child->next_sibling = n;
					current->last_child = n;
				} else {
					current->first_child = n;
					current->last_child = n;
				}

				n->parser_indent = indenting;
				n->name = unquote(name, -1);
				free(name);
				n->cls = cls;
				current = n;
			}
			else
			if (read_kv(&text, &key, &name, &value) == 1)
			{
				struct stfl_kv *kv = stfl_widget_setkv_str(current, key, value);
				if (kv->name)
					free(kv->name);
				kv->name = unquote(name, -1);
				free(name);

				free(key);
				free(value);
			}
			else
				goto parser_error;
		}
		else
		{
			if (read_type(&text, &key, &name, &cls) == 0)
				goto parser_error;

			struct stfl_widget *n = stfl_widget_new(key);
			if (!n)
				goto parser_error;
			free(key);

			root = n;
			current = n;
			n->name = unquote(name, -1);
			free(name);
			n->cls = cls;
		}

		while (*text && *text != L'\n' && *text != L'\r' && *text != L'{' && *text != L'}')
		{
			while (*text == L' ' || *text == L'\t')
				text++;

			if (*text && *text != L'\n' && *text != L'\r' && *text != L'{' && *text != L'}')
			{
				if (read_kv(&text, &key, &name, &value) == 0)
					goto parser_error;

				struct stfl_kv *kv = stfl_widget_setkv_str(current, key, value);
				if (kv->name)
					free(kv->name);
				kv->name = unquote(name, -1);
				free(name);

				free(key);
				free(value);
			}
		}
	}

	if (root)
		return root;

parser_error:;
	int i;

	fprintf(stderr, "STFL Parser Error near '");

	for (i=0; *text && i<20; i++, text++)
		if (*text == L'\n')
			fprintf(stderr, "\\n");
		else
		if (*text == L'\t')
			fprintf(stderr, " ");
		else
		if (*text < 32)
			fprintf(stderr, "\\%03lo", (long unsigned int)*text);
		else
			fprintf(stderr, "%lc", (wint_t)*text);

	fprintf(stderr, "'.\r\n");
	abort();

	return 0;
}

struct stfl_widget *stfl_parser_file(const char *filename)
{
	FILE *f = fopen(filename, "r");

	if (!f) {
		fprintf(stderr, "STFL Parser Error: Can't read file '%s'!\n", filename);
		abort();
		return 0;
	}

	int len = 0;
	char *text = 0;

	while (1) {
		int pos = len;
		text = realloc(text, len += 4096);
		pos += fread(text+pos, 1, 4096, f);
		if (pos < len) {
			text[pos] = 0;
			fclose(f);
			break;
		}
	}

	const char * text1 = text;
	size_t wtextsize = mbsrtowcs(NULL,&text1,strlen(text1)+1,NULL)+1;
	wchar_t * wtext = malloc(sizeof(wchar_t)*wtextsize);

	size_t rc = mbstowcs(wtext, text, wtextsize);
	assert(rc != (size_t)-1);

#if 0
	fprintf(stderr,"strlen(text) = %u wcslen(wtext) = %u rc = %u wtextsize = %u\n", strlen(text), wcslen(wtext), rc, wtextsize);
	fprintf(stderr,"this is where is fucked up: `%lc' `%lc' `%lc' `%lc' `%lc'\n",text1[0],text1[1],text1[2],text1[3],text1[4]);
	fprintf(stderr,"original: `%s'\n", text);
	fprintf(stderr,"converted: `%ls'\n", wtext);
#endif

	struct stfl_widget *w = stfl_parser(wtext);
	free(text);
	free(wtext);

	return w;
}

