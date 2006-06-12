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

include Makefile.cfg

CFLAGS += -I. -Wall -O0 -ggdb
LDLIBS += -lcurses

all: libstfl.a example

example: LDFLAGS += -L.
example: LDLIBS += -lstfl
example: libstfl.a

libstfl.a: public.o base.o parser.o dump.o style.o \
           wt_label.o wt_input.o wt_box.o wt_table.o wt_list.o
	rm -f $@
	ar qc $@ $^
	ranlib $@

clean:
	rm -f libstfl.a example core core.* *.o Makefile.deps
	rm -f spl/mod_stfl.so spl/example.db
	cd perl5 && perl Makefile.PL && make clean && rm -f Makefile.old
	rm -f perl5/stfl.i perl5/stfl_wrap.c perl5/stfl.pm perl5/build_ok
	rm -f python/stfl.i python/stfl.py python/stfl.pyc python/_stfl.so 
	rm -f python/stfl_wrap.c python/stfl_wrap.o

Makefile.deps: *.c *.h
	$(CC) -MM *.c > Makefile.deps

install: all
	mkdir -p $(prefix)/lib
	mkdir -p $(prefix)/include
	install -m 644 libstfl.a $(prefix)/lib/
	install -m 644 stfl.h $(prefix)/include/

ifeq ($(FOUND_SPL),1)
include spl/Makefile.snippet
endif

ifeq ($(FOUND_SWIG)$(FOUND_PERL5),11)
include perl5/Makefile.snippet
endif

ifeq ($(FOUND_SWIG)$(FOUND_PYTHON),11)
include python/Makefile.snippet
endif

.PHONY: all clean install install_spl

include Makefile.deps

