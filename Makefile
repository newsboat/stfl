#
#  STFL - The Simple Terminal Forms Library
#  Copyright (C) 2006  Clifford Wolf <clifford@clifford.at>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

CFLAGS += -Wall -O0 -ggdb
LDLIBS += -lcurses

all: libstfl.a example

example: LDFLAGS += -L.
example: LDLIBS += -lstfl
example: libstfl.a

libstfl.a: base.o parser.o \
		wt_label.o wt_input.o wt_vbox.o wt_hbox.o
	rm -f $@
	ar qc $@ $^
	ranlib $@

clean:
	rm -f libstfl.a example
	rm -f core core.* *.o
	rm -f Makefile.deps

Makefile.deps: *.c *.h
	$(CC) -MM *.c > Makefile.deps

include Makefile.deps

