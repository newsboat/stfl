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

ifeq ($(OS),Windows_NT)
    detected_OS := Windows
    SHARED_LIB_EXT := dll
    EXTRA_OBJS := wcwidth.o
else
    detected_OS := $(shell uname)
    SHARED_LIB_EXT := so
    EXTRA_OBJS :=
endif
include Makefile.cfg

export CC = gcc -pthread
export CFLAGS += -I. -Wall -Os -ggdb -D_GNU_SOURCE -fPIC
export LDLIBS += $(shell pkg-config --libs ncursesw)
export CFLAGS += $(shell pkg-config --cflags ncursesw)
VERSION := 0.24
ifeq ($(detected_OS),Windows)
    SHARED_LIB_NAME := libstfl.$(VERSION).$(SHARED_LIB_EXT)
    CFLAGS += -DNCURSES_STATIC
    export LDLIBS += $(shell pkg-config --libs iconv)
    export CFLAGS += $(shell pkg-config --cflags iconv)
VERSION := 0.24
else
    SHARED_LIB_NAME := libstfl.so.$(VERSION)
    SONAME := libstfl.so.0
endif

all: $(SHARED_LIB_NAME) libstfl.a example

example: libstfl.a example.o

libstfl.a: public.o base.o parser.o dump.o style.o binding.o iconv.o $(EXTRA_OBJS) \
           $(patsubst %.c,%.o,$(wildcard widgets/*.c))
	rm -f $@
	ar qc $@ $^
	ranlib $@

$(SHARED_LIB_NAME): $(EXTRA_OBJS) public.o base.o parser.o dump.o style.o binding.o iconv.o \
                    $(patsubst %.c,%.o,$(wildcard widgets/*.c))
ifeq ($(detected_OS),Windows)
	$(CC) -shared -o $@ $(CFLAGS) $^ $(LDLIBS)
else
	$(CC) -shared -Wl,-soname,$(SONAME) -o $@ $(CFLAGS) $^ $(LDLIBS)
endif

ifeq ($(detected_OS),Windows)
wcwidth.o: wcwidth.c
	$(CC) $(CFLAGS) -c -o $@ $^
endif

clean:
	rm -f libstfl.a example core core.* *.o Makefile.deps
	rm -f widgets/*.o spl/mod_stfl.so spl/example.db
	cd perl5 && perl Makefile.PL && make clean && rm -f Makefile.old
	rm -f perl5/stfl_wrap.c perl5/stfl.pm perl5/build_ok
	rm -f python/stfl.py python/stfl.pyc python/_stfl.so 
	rm -f python/stfl_wrap.c python/stfl_wrap.o
	rm -f ruby/Makefile ruby/stfl_wrap.c ruby/stfl_wrap.o
	rm -f ruby/stfl.so ruby/build_ok Makefile.deps_new
	rm -f stfl.pc libstfl.so libstfl.so.*

Makefile.deps: *.c widgets/*.c *.h
	$(CC) -I. -MM *.c > Makefile.deps_new
	$(CC) -I. -MM widgets/*.c | sed 's,^wt_[^ ]*\.o: ,widgets/&,' >> Makefile.deps_new
	mv -f Makefile.deps_new Makefile.deps

install: all stfl.pc
	mkdir -p $(DESTDIR)$(prefix)/$(libdir)/pkgconfig
	mkdir -p $(DESTDIR)$(prefix)/include
	install -m 644 libstfl.a $(DESTDIR)$(prefix)/$(libdir)
	install -m 644 stfl.h $(DESTDIR)$(prefix)/include/
	install -m 644 stfl.pc $(DESTDIR)$(prefix)/$(libdir)/pkgconfig/

    # Handle installation of shared library based on the operating system
ifeq ($(detected_OS),Windows)
	install -m 644 libstfl.$(VERSION).dll $(DESTDIR)$(prefix)/$(libdir)
	cp $(DESTDIR)$(prefix)/$(libdir)/libstfl.$(VERSION).dll $(DESTDIR)$(prefix)/$(libdir)/libstfl.dll
else
	install -m 644 libstfl.so.$(VERSION) $(DESTDIR)$(prefix)/$(libdir)
	ln -fs libstfl.so.$(VERSION) $(DESTDIR)$(prefix)/$(libdir)/libstfl.so
endif
stfl.pc: stfl.pc.in
	sed 's,@VERSION@,$(VERSION),g' < $< | sed 's,@PREFIX@,$(prefix),g' > $@

ifeq ($(FOUND_SPL),1)
include spl/Makefile.snippet
endif

ifneq ($(detected_OS),Windows)
ifeq ($(FOUND_SWIG)$(FOUND_PERL5),11)
include perl5/Makefile.snippet
endif
endif
ifeq ($(FOUND_SWIG)$(FOUND_PYTHON),11)
include python/Makefile.snippet
endif

ifeq ($(FOUND_SWIG)$(FOUND_RUBY),11)
include ruby/Makefile.snippet
endif

.PHONY: all clean install install_spl

include Makefile.deps

