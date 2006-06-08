#
#  STFL - The Structured Terminal Forms Language/Library
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

prefix = /usr/local

CFLAGS += -I. -Wall -O0 -ggdb
LDLIBS += -lcurses

all: libstfl.a example

example: LDFLAGS += -L.
example: LDLIBS += -lstfl
example: libstfl.a

libstfl.a: public.o base.o parser.o dump.o style.o \
           wt_label.o wt_input.o wt_box.o wt_table.o
	rm -f $@
	ar qc $@ $^
	ranlib $@

clean:
	rm -f libstfl.a example
	rm -f core core.* *.o
	rm -f Makefile.deps
	rm -f spl/mod_stfl.so

Makefile.deps: *.c *.h
	$(CC) -MM *.c > Makefile.deps

install: all
	mkdir -p $(prefix)/lib
	mkdir -p $(prefix)/include
	install -m 644 libstfl.a $(prefix)/lib/
	install -m 644 stfl.h $(prefix)/include/


### Build and install STFL SPL module if SPL is installed ###

ifneq ($(shell spl-config 2>/dev/null),)

spl/mod_stfl.so: libstfl.a stfl.h spl/mod_stfl.c
	gcc -shared $(CFLAGS) $(LDFLAGS) spl/mod_stfl.c \
			-L. -lstfl $(LDLIBS) -o spl/mod_stfl.so

all: spl/mod_stfl.so

install_spl: spl/mod_stfl.so
	mkdir -p $(prefix)/lib/spl_modules
	install spl/mod_stfl.so $(prefix)/lib/spl_modules/

install: install_spl

endif


### The glory final hacks ###

.PHONY: all clean install install_spl

include Makefile.deps

