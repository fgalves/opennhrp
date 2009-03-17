# Makefile - one file to rule them all, one file to bind them
#
# Copyright (C) 2007 Timo Teräs <timo.teras@iki.fi>
# All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 or later as
# published by the Free Software Foundation.
#
# See http://www.gnu.org/ for details.

VERSION := 0.9.3

GIT_REV := $(shell git-describe || echo exported)
ifneq ($(GIT_REV), exported)
ifneq ($(filter v$(VERSION)%, $(GIT_REV)),)
FULL_VERSION := $(patsubst v%,%,$(GIT_REV))
else
FULL_VERSION := $(GIT_REV)
endif
else
FULL_VERSION := $(VERSION)
endif

CC=gcc
INSTALL=install
INSTALLDIR=$(INSTALL) -d

CFLAGS+=-Werror -Wall -Wstrict-prototypes -std=gnu99 -O2 \
       -DOPENNHRP_VERSION=\"$(FULL_VERSION)\" \
       $(shell pkg-config --cflags libcares)
LDFLAGS+=$(shell pkg-config --libs libcares)

ifneq ($(DEBUG),)
CFLAGS+=-g
LDFLAGS+=-g
endif

DESTDIR=
SBINDIR=/usr/sbin
CONFDIR=/etc/opennhrp
MANDIR=/usr/share/man
DOCDIR=/usr/share/doc/opennhrp
STATEDIR=/var/run

SUBDIRS=nhrp etc man

.PHONY: compile install clean all

all: compile

compile install clean::
	@for i in $(SUBDIRS); do $(MAKE) $(MFLAGS) -C $$i $(MAKECMDGOALS); done

install::
	$(INSTALLDIR) $(DESTDIR)$(DOCDIR)
	$(INSTALL) README $(DESTDIR)$(DOCDIR)

dist:
	(TOP=`pwd` && cd .. && ln -s $$TOP opennhrp-$(VERSION) && \
	 tar --exclude '*/.git*' -cjvf opennhrp-$(VERSION).tar.bz2 opennhrp-$(VERSION)/* && \
	 rm opennhrp-$(VERSION))

.EXPORT_ALL_VARIABLES:
