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
 *  example.c: A little STFL example
 */

#include "stfl.h"

int main()
{
	initscr(); cbreak(); noecho();
	nonl(); keypad(stdscr, TRUE);

	struct stfl_form *f = stfl_form_new();
	f->root = stfl_parser(
		"vbox						\n"
		"  label text:'Little STFL example program'	\n"
		"  hbox						\n"
		"    label					\n"
		"      text:'Field A:'				\n"
		"    input					\n"
		"      text:'This is'				\n"
		"  hbox						\n"
		"    label					\n"
		"      text:'Field B:'				\n"
		"    input					\n"
		"      text:'a test..'				\n"
		"  label text:'Happy hacking!'			\n"
	);

	while (1) {
		stfl_form_run(f, stdscr);
		if (f->event_type == STFL_EVENT_KEY_ESC)
			break;
	}

	stfl_form_free(f);

	endwin();

	return 0;
}

