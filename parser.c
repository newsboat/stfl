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
 *  parser.c: STFL Form description file parser
 */

#include "stfl_internals.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void extract_name(char **key, char **name)
{
	int len = strcspn(*key, "[");

	if ((*key)[len] == 0) {
		*name = 0;
		return;
	}

	*name = strdup(*key+len+1);
	*key = realloc(*key, len+1);
	(*key)[len] = 0;

	len = strcspn(*name, "]");
	(*name)[len] = 0;
}

static void extract_class(char **key, char **cls)
{
	int len = strcspn(*key, "#");

	if ((*key)[len] == 0) {
		*cls = 0;
		return;
	}

	*cls = strdup(*key+len+1);
	*key = realloc(*key, len+1);
	(*key)[len] = 0;
}

static int read_type(const char **text, char **type, char **name, char **cls)
{
	int len = strcspn(*text, " \t\r\n:{}");

	if ((*text)[len] == ':' || len == 0)
		return 0;

	*type = malloc(len+1);
	memcpy(*type, *text, len);
	(*type)[len] = 0;
	*text += len;

	extract_name(type, name);
	extract_class(type, cls);

	return 1;
}

static int read_kv(const char **text, char **key, char **name, char **value)
{
	int len_k = strcspn(*text, " \t\r\n:{}");

	if ((*text)[len_k] != ':' || len_k == 0)
		return 0;

	*key = malloc(len_k+1);
	memcpy(*key, *text, len_k);
	(*key)[len_k] = 0;
	*text += len_k+1;

	extract_name(key, name);

	int len_v = 0, i = 0, j = 0;
	while ((*text)[i] && (*text)[i] != ' ' && (*text)[i] != '{' && (*text)[i] != '}' &&
	       (*text)[i] != '\t' && (*text)[i] != '\r' && (*text)[i] != '\n')
	{
		if ((*text)[i] == '\'')
			while ((*text)[++i] != '\'') len_v++;
		else
		if ((*text)[i] == '\"')
			while ((*text)[++i] != '\"') len_v++;
		len_v++;
		i++;
	}

	*value = malloc(len_v+1);
	i = 0;

	while ((*text)[i] && (*text)[i] != ' ' && (*text)[i] != '{' && (*text)[i] != '}' &&
	       (*text)[i] != '\t' && (*text)[i] != '\r' && (*text)[i] != '\n')
	{
		if ((*text)[i] == '\'')
			while ((*text)[++i] != '\'')
				(*value)[j++] = (*text)[i];
		else
		if ((*text)[i] == '\"')
			while ((*text)[++i] != '\"')
				(*value)[j++] = (*text)[i];
		else
			(*value)[j++] = (*text)[i];
		i++;
	}
	
	(*value)[j] = 0;
	*text += i;

	return 1;
}

struct stfl_widget *stfl_parser(const char *text)
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
			while (*text == ' ' || *text == '\t') text++;

			while (*text == '}') {
				bracket_level--; text++;
				while (*text == ' ' || *text == '\t') text++;
			}

			while (*text == '{') {
				bracket_level++; text++;
				while (*text == ' ' || *text == '\t') text++;
			}

			if (bracket_level == 0)
				bracket_indenting = -1;

			if (bracket_level < 0)
				goto parser_error;
		}
		else
			if (*text == '}')
				goto parser_error;

		if (bracket_indenting >= 0)
		{
			while (*text == ' ' || *text == '\t')
				text++;

			if (*text == '\r' || *text == '\n')
				goto parser_error;

			indenting = bracket_indenting + (bracket_level-1);
		}
		else
		{
			while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
				if (*text == '\r' || *text == '\n')
					indenting = 0;
				else
				if (*text == '\t')
					indenting = -1;
				else
				if (indenting >= 0)
					indenting++;
				text++;
			}

			if (*text == '*') {
				while (*text && *text != '\r' && *text != '\n')
					text++;
				continue;
			}

			if (*text == '{') {
				bracket_indenting = indenting;
				continue;
			}
		}

		if (*text == 0)
			break;

		char *key, *name, *cls, *value;
		if (indenting < 0)
			goto parser_error;

		if (*text == '<')
		{
			int filename_len = strcspn(++text, ">");
			char filename[filename_len+1];

			memcpy(filename, text, filename_len);
			filename[filename_len] = 0;

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
				n->name = name;
				n->cls = cls;
				current = n;
			}
			else
			if (read_kv(&text, &key, &name, &value) == 1)
			{
				struct stfl_kv *kv = stfl_widget_setkv_str(current, key, value);
				if (kv->name)
					free(kv->name);
				kv->name = name;

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
			n->name = name;
			n->cls = cls;
		}

		while (*text && *text != '\n' && *text != '\r' && *text != '{' && *text != '}')
		{
			while (*text == ' ' || *text == '\t')
				text++;

			if (*text && *text != '\n' && *text != '\r' && *text != '{' && *text != '}')
			{
				if (read_kv(&text, &key, &name, &value) == 0)
					goto parser_error;

				struct stfl_kv *kv = stfl_widget_setkv_str(current, key, value);
				if (kv->name)
					free(kv->name);
				kv->name = name;

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
		if (*text == '\n')
			fprintf(stderr, "\\n");
		else
		if (*text == '\t')
			fprintf(stderr, " ");
		else
		if (*text < 32)
			fprintf(stderr, "\\%03o", *text);
		else
			fprintf(stderr, "%c", *text);

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

	struct stfl_widget *w = stfl_parser(text);
	free(text);

	return w;
}

