# Makefile - one file to rule them all, one file to bind them
#
# Copyright (C) 2007 Timo Teräs <timo.teras@iki.fi>
# All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License version 3 as published
# by the Free Software Foundation. See http://www.gnu.org/ for details.

VERSION := 0.6.2

SVN_REV := $(shell svn info | grep ^Revision | cut -d ' ' -f 2)
ifneq ($(SVN_REV),)
FULL_VERSION := $(VERSION)-r$(SVN_REV)
else
FULL_VERSION := $(VERSION)
endif

CC=gcc
INSTALL=install
INSTALLDIR=$(INSTALL) -d

CFLAGS=-g -Werror -Wall -Wstrict-prototypes -std=gnu99 -O -DOPENNHRP_VERSION=\"$(FULL_VERSION)\"
LDFLAGS=-g

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
	svn-clean
	(TOP=`pwd` && cd .. && ln -s $$TOP opennhrp-$(VERSION) && \
	 tar --exclude '*/.svn*' -cjvf opennhrp-$(VERSION).tar.bz2 opennhrp-$(VERSION)/* && \
	 rm opennhrp-$(VERSION))

.EXPORT_ALL_VARIABLES:
