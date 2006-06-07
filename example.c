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
 *  example.c: A little STFL example
 */

#include "stfl.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
	struct stfl_form *f = stfl_create("<example.stfl>");

	stfl_set(f, "value_a", "This is a little");
	stfl_set(f, "value_b", "test for STFL!");

	const char *event = 0;
	while (!event || strcmp(event, "ESC"))
		event = stfl_run(f, 0);

	stfl_return();

	printf("A: %s\n", stfl_get(f, "value_a"));
	printf("B: %s\n", stfl_get(f, "value_b"));
	printf("C: %s\n", stfl_get(f, "value_c"));

	stfl_free(f);

	return 0;
}

