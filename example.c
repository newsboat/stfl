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
 *  example.c: A little STFL example
 */

#include "stfl.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <langinfo.h>
#endif

int main()
{
    if (!setlocale(LC_ALL,""))
        fprintf(stderr, "WARNING: Can't set locale!\n");

    struct stfl_ipool *ipool;

    #ifdef _WIN32
        // Windows specific initialization
        // Get the current Windows ANSI code page
        UINT codePage = GetACP();
        wchar_t codePageStr[10];
        swprintf(codePageStr, 10, L"%u", codePage);
        ipool = stfl_ipool_create(codePageStr);
    #else
        // POSIX specific initialization
        ipool = stfl_ipool_create(nl_langinfo(CODESET));
    #endif
	struct stfl_form *f = stfl_create(L"<example.stfl>");

	stfl_set(f, L"value_a", L"This is a little");
	stfl_set(f, L"value_b", stfl_ipool_towc(ipool, "test for STFL!"));
	stfl_ipool_flush(ipool);

	const wchar_t *event = 0;
	while (1) {
		event = stfl_run(f, 0);
		if (event) {
			if (!wcscmp(event, L"ESC"))
				break;
			else if (!wcscmp(event, L"^L"))
				stfl_redraw();
		}
	}

	stfl_reset();
	printf("%ls", stfl_text(f, L"textedit"));

	printf("A: %ls\n", stfl_get(f, L"value_a"));
	printf("B: %ls\n", stfl_get(f, L"value_b"));
	printf("C: %s\n", stfl_ipool_fromwc(ipool, stfl_get(f, L"value_c")));
	stfl_ipool_flush(ipool);

	stfl_free(f);
	stfl_ipool_destroy(ipool);

	return 0;
}

