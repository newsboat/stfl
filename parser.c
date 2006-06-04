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
 *  parser.c: STFL Form description file parser
 */

#include "stfl.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

static int read_type(const char **text, char **type)
{
	int len = strcspn(*text, " \t\r\n:");

	if ((*text)[len] == ':' || len == 0)
		return 0;

	*type = malloc(len+1);
	memcpy(*type, *text, len);
	(*type)[len] = 0;
	*text += len;

	return 1;
}

static int read_kv(const char **text, char **key, char **value)
{
	int len_k = strcspn(*text, " \t\r\n:");

	if ((*text)[len_k] != ':' || len_k == 0)
		return 0;

	*key = malloc(len_k+1);
	memcpy(*key, *text, len_k);
	(*key)[len_k] = 0;
	*text += len_k+1;

	int len_v = 0, i = 0, j = 0;
	while ((*text)[i] && (*text)[i] != ' ' && (*text)[i] != '\t' &&
	       (*text)[i] != '\r' && (*text)[i] != '\n')
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

	while ((*text)[i] && (*text)[i] != ' ' && (*text)[i] != '\t' &&
	       (*text)[i] != '\r' && (*text)[i] != '\n')
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

	while (1)
	{
		int indenting = 0;

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

		if (*text == 0)
			break;

		char *key, *value;
		assert(indenting >= 0);

		if (root)
		{
			while (current->parser_indent >= indenting) {
				current = current->parent;
				assert(current);
			}

			if (read_type(&text, &key) == 1)
			{
				struct stfl_widget *n = stfl_widget_new(key);
				assert(n != 0);
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
				current = n;
			}
			else
			if (read_kv(&text, &key, &value) == 1)
			{
				stfl_widget_setkv(current, key, value);
				free(key); free(value);
			}
			else
				assert(!"Syntax error");
		}
		else
		{
			assert(read_type(&text, &key) == 1);

			struct stfl_widget *n = stfl_widget_new(key);
			assert(n != 0);
			free(key);

			root = n;
			current = n;
		}

		while (*text != '\n' && *text != '\r')
		{
			while (*text == ' ' || *text == '\t')
				text++;

			if (*text != '\n' && *text != '\r') {
				assert(read_kv(&text, &key, &value) == 1);
				stfl_widget_setkv(current, key, value);
				free(key); free(value);
			}
		}
	}

	return root;
}

