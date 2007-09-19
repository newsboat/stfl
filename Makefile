#
#  STFL - The Structured Terminal Forms Language/Library
#  Copyright (C) 2006, 2007  Clifford Wolf <clifford@clifford.at>
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 3 of the License, or (at your option) any later version.
#  
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#  
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301 USA
#

include Makefile.cfg

export CC = gcc -pthread
export CFLAGS += -I. -Wall -Os -ggdb -D_GNU_SOURCE -fPIC
export LDLIBS += -lncursesw

all: libstfl.a example

example: libstfl.a example.o

libstfl.a: public.o base.o parser.o dump.o style.o binding.o iconv.o \
           $(patsubst %.c,%.o,$(wildcard widgets/*.c))
	rm -f $@
	ar qc $@ $^
	ranlib $@

clean:
	rm -f libstfl.a example core core.* *.o Makefile.deps
	rm -f widgets/*.o spl/mod_stfl.so spl/example.db
	cd perl5 && perl Makefile.PL && make clean && rm -f Makefile.old
	rm -f perl5/stfl_wrap.c perl5/stfl.pm perl5/build_ok
	rm -f python/stfl.py python/stfl.pyc python/_stfl.so 
	rm -f python/stfl_wrap.c python/stfl_wrap.o
	rm -f ruby/Makefile ruby/stfl_wrap.c ruby/stfl_wrap.o
	rm -f ruby/stfl.so ruby/build_ok Makefile.deps_new

Makefile.deps: *.c widgets/*.c *.h
	$(CC) -I. -MM *.c > Makefile.deps_new
	$(CC) -I. -MM widgets/*.c | sed 's,^wt_[^ ]*\.o: ,widgets/&,' >> Makefile.deps_new
	mv -f Makefile.deps_new Makefile.deps

install: all
	mkdir -p $(DESTDIR)$(prefix)/lib
	mkdir -p $(DESTDIR)$(prefix)/include
	install -m 644 libstfl.a $(DESTDIR)$(prefix)/lib/
	install -m 644 stfl.h $(DESTDIR)$(prefix)/include/

ifeq ($(FOUND_SPL),1)
include spl/Makefile.snippet
endif

ifeq ($(FOUND_SWIG)$(FOUND_PERL5),11)
include perl5/Makefile.snippet
endif

ifeq ($(FOUND_SWIG)$(FOUND_PYTHON),11)
include python/Makefile.snippet
endif

ifeq ($(FOUND_SWIG)$(FOUND_RUBY),11)
include ruby/Makefile.snippet
endif

.PHONY: all clean install install_spl

include Makefile.deps

