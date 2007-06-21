/*
 *  STFL - The Structured Terminal Forms Language/Library
 *  Copyright (C) 2006, 2007  Clifford Wolf <clifford@clifford.at>
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

#include <langinfo.h>
#include <locale.h>

int main()
{
	if (!setlocale(LC_ALL,""))
		fprintf(stderr, "WARING: Can't set locale!\n");

	struct stfl_ipool *ipool = stfl_ipool_create(nl_langinfo(CODESET));
	struct stfl_form *f = stfl_create(L"<example.stfl>");

	stfl_set(f, L"value_a", L"This is a little");
	stfl_set(f, L"value_b", stfl_ipool_towc(ipool, "test for STFL!"));
	stfl_ipool_flush(ipool);

	const wchar_t *event = 0;
	while (!event || wcscmp(event, L"ESC"))
		event = stfl_run(f, 0);

	stfl_reset();

	printf("A: %ls\n", stfl_get(f, L"value_a"));
	printf("B: %ls\n", stfl_get(f, L"value_b"));
	printf("C: %s\n", stfl_ipool_fromwc(ipool, stfl_get(f, L"value_c")));
	stfl_ipool_flush(ipool);

	stfl_free(f);
	stfl_ipool_destroy(ipool);

	return 0;
}

