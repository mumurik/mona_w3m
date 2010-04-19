#
# Makefile
# Makefile.  Generated from Makefile.in by configure.
#

SHELL=/bin/sh
PACKAGE = w3m
VERSION = 0.5.2
DOMAIN = $(PACKAGE)
prefix = /usr/local
exec_prefix = ${prefix}
datarootdir = ${prefix}/share
bindir = ${exec_prefix}/bin
datadir = ${datarootdir}
localedir = $(datadir)/locale
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
infodir = ${datarootdir}/info
libexecdir = ${exec_prefix}/libexec
localstatedir = ${prefix}/var
mandir = ${datarootdir}/man
oldincludedir = /usr/include
sbindir = ${exec_prefix}/sbin
sharedstatedir = ${prefix}/com
srcdir = .
sysconfdir = ${prefix}/etc
top_srcdir = .
top_builddir = .
VPATH = $(top_srcdir):.
DESTDIR = 

CGIBIN_DIR = $(libexecdir)/$(PACKAGE)/cgi-bin
AUXBIN_DIR = $(libexecdir)/$(PACKAGE)
HELP_DIR = ${datarootdir}/w3m
RC_DIR = ~/.w3m
ETC_DIR = $(sysconfdir)
CONF_DIR = $(sysconfdir)/$(PACKAGE)

CFLAGS = $(OPTS) -I. -I$(top_srcdir) -g -O2 -I$(srcdir)/libwc $(CPPFLAGS) $(DEFS)
WCCFLAGS = -DUSE_UNICODE -I$(srcdir) -I$(srcdir)/..
CPPFLAGS = 
DEFS = -DHAVE_CONFIG_H -DAUXBIN_DIR=\"$(AUXBIN_DIR)\" \
	-DCGIBIN_DIR=\"$(CGIBIN_DIR)\" -DHELP_DIR=\"$(HELP_DIR)\" \
	-DETC_DIR=\"$(ETC_DIR)\" -DCONF_DIR=\"$(CONF_DIR)\" \
	-DRC_DIR=\"$(RC_DIR)\" \
        -DLOCALEDIR=\"$(localedir)\"
LDFLAGS =  -L../../mona/core/scheme/BoehmGC/gc-7.0//lib
LIBS = 
EXT_LIBS = -L. -lindep  -L../../mona/core/scheme/BoehmGC/gc-7.0//lib -lgc
W3M_LIBS =  -L./libwc -lwc
WCTARGET = libwc/libwc.a
NLSTARGET = 

MAKE_ARGS = PERL='$(PERL)' MKDIR='$(MKDIR)' \
	BIN_DIR='$(bindir)' AUXBIN_DIR='$(AUXBIN_DIR)' \
	CGIBIN_DIR='$(CGIBIN_DIR)' \
	HELP_DIR='$(HELP_DIR)' MAN_DIR='$(mandir)' ETC_DIR='$(ETC_DIR)' \
	CONF_DIR='$(CONF_DIR)' \
	RC_DIR='$(RC_DIR)' DESTDIR='$(DESTDIR)' KEYBIND_SRC='$(KEYBIND_SRC)'

IMGCFLAGS =  
IMGLDFLAGS =  

CC0 = gcc
CC =  $(CC0)
CPP = gcc -E
RANLIB=ranlib
AWK = gawk
PERL = /usr/local/bin/perl
MKDIR=mkdir -p
MV=mv
RM=rm
AR=ar
INSTALL=/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_SCRIPT=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644
INSTALL_W3MIMGDISPLAY=${INSTALL_PROGRAM}

HELP_FILE = w3mhelp-w3m_en.html
KEYBIND_SRC = keybind.c
KEYBIND_OBJ = keybind.o

VERSION=0.5.2+cvs-1.1001
MODEL=-EN

SRCS=main.c file.c buffer.c display.c etc.c search.c linein.c table.c local.c \
	form.c map.c frame.c rc.c menu.c mailcap.c image.c \
	symbol.c entity.c terms.c url.c ftp.c mimehead.c regex.c news.c \
	func.c cookie.c history.c backend.c $(KEYBIND_SRC)
OBJS=main.o file.o buffer.o display.o etc.o search.o linein.o table.o local.o\
	form.o map.o frame.o rc.o menu.o mailcap.o image.o \
	symbol.o entity.o terms.o url.o ftp.o mimehead.o regex.o news.o \
	func.o cookie.o history.o backend.o $(KEYBIND_OBJ)
LSRCS=anchor.c parsetagx.c tagtable.c istream.c
LOBJS=anchor.o parsetagx.o tagtable.o istream.o
LLOBJS=version.o
ALIBOBJS=Str.o indep.o regex.o textlist.o parsetag.o myctype.o hash.o
ALIB=libindep.a
ALLOBJS=$(OBJS) $(LOBJS) $(LLOBJS)

EXT=.exe

TARGET=$(PACKAGE)$(EXT)
BOOKMARKER=w3mbookmark$(EXT)
HELPER=w3mhelperpanel$(EXT)
INFLATE=inflate$(EXT)
IMGDISPLAY=w3mimgdisplay$(EXT)
MAN1_TARGET=$(PACKAGE).1
MAN1=$(top_srcdir)/doc/w3m.1
MAN1_JA=$(top_srcdir)/doc-jp/w3m.1

LIB_TARGETS=$(BOOKMARKER) $(HELPER)
AUXBIN_TARGETS= w3mimgdisplay$(EXT)
TARGETS=$(TARGET) $(LIB_TARGETS) $(AUXBIN_TARGETS)
HELP_TARGET=w3mhelp.html
HELP_ALLFILES=w3mhelp-w3m_en.html w3mhelp-w3m_ja.html \
	w3mhelp-lynx_en.html w3mhelp-lynx_ja.html

DEFUNS=$(top_srcdir)/main.c $(top_srcdir)/menu.c
SCRIPTSUBDIRS= scripts
SUBDIRS = $(SCRIPTSUBDIRS) w3mimg libwc po
.PHONY: $(SUBDIRS)

all: $(TARGETS) all-scripts $(NLSTARGET)

$(TARGET): $(ALLOBJS) $(ALIB) $(WCTARGET)
	$(CC) $(CFLAGS) -o $(TARGET) $(ALLOBJS) $(LDFLAGS) $(LIBS) $(EXT_LIBS) $(W3M_LIBS)

$(ALIB): $(ALIBOBJS)
	$(AR) rv $(ALIB) $(ALIBOBJS)
	$(RANLIB) $(ALIB)

$(OBJS) $(LOBJS): fm.h funcname1.h

tagtable.c: tagtable.tab mktable$(EXT) html.h 
	./mktable$(EXT) 100 $(srcdir)/tagtable.tab > $@

indep.o: indep.c fm.h funcname1.h
func.o: funcname.c functable.c funcname1.h
keybind.o: funcname2.h
keybind_lynx.o: funcname2.h
parsetagx.o: html.c

funcname.tab: $(DEFUNS)
	(echo '#define DEFUN(x,y,z) x y';\
	 sed -ne '/^DEFUN/{p;n;/^[ 	]/p;}' $(DEFUNS)) | $(CPP) - | \
	 awk '$$1 ~ /^[_A-Za-z]/ { \
	       for (i=2;i<=NF;i++) { print $$i, $$1} \
	 }' > $@.tmp
	@if cmp -s $@.tmp $@; then \
		echo "$@ unchanged"; \
		rm -f $@.tmp; \
	else \
		echo "$@ updated"; \
		mv $@.tmp $@; \
	fi

funcname.c: funcname.tab
	sort funcname.tab | $(AWK) -f $(top_srcdir)/funcname0.awk > $@

funcname1.h: funcname.tab
	sort funcname.tab | $(AWK) -f $(top_srcdir)/funcname1.awk > $@

funcname2.h: funcname.tab
	sort funcname.tab | $(AWK) -f $(top_srcdir)/funcname2.awk > $@

functable.c: funcname.tab mktable$(EXT)
	sort funcname.tab | $(AWK) -f $(top_srcdir)/functable.awk > functable.tab
	./mktable$(EXT) 100 functable.tab > $@
	-rm -f functable.tab

mktable$(EXT): mktable.o dummy.o $(ALIB)
	$(CC) $(CFLAGS) -o mktable mktable.o dummy.o $(LDFLAGS) $(LIBS) $(EXT_LIBS)

$(BOOKMARKER): w3mbookmark.o dummy.o $(ALIB)
	$(CC) $(CFLAGS) -o $(BOOKMARKER) w3mbookmark.o dummy.o $(LDFLAGS) $(LIBS) $(EXT_LIBS)

$(HELPER): w3mhelperpanel.o dummy.o $(ALIB)
	$(CC) $(CFLAGS) -o $(HELPER) w3mhelperpanel.o dummy.o $(LDFLAGS) $(LIBS) $(EXT_LIBS)

$(INFLATE): inflate.o $(ALIB)
	$(CC) $(CFLAGS) -o $(INFLATE) inflate.o $(LDFLAGS) $(LIBS) -lz

inflate.o: inflate.c
	$(CC) $(CFLAGS) -o $@ -c $(srcdir)/inflate.c

libwc/libwc.a: libwc
libwc:
	(cd libwc && $(MAKE) CC='$(CC0)' OPTS='$(OPTS)')
 
dummy.o: entity.c
	$(CC) $(CFLAGS) -DDUMMY -c -o $@ $?

$(IMGDISPLAY): w3mimgdisplay.o $(ALIB) w3mimg/w3mimg.a
	$(CC) $(CFLAGS) -o $(IMGDISPLAY) w3mimgdisplay.o w3mimg/w3mimg.a $(LDFLAGS) $(LIBS) $(IMGLDFLAGS)

w3mimgdisplay.o: w3mimgdisplay.c w3mimg/w3mimg.h
	$(CC) $(CFLAGS) $(IMGCFLAGS) -o $@ -c $(srcdir)/w3mimgdisplay.c

w3mimg/w3mimg.a: w3mimg
w3mimg:
	(cd w3mimg && $(MAKE) CC="$(CC0)" OPTS="$(OPTS)")

po:
	(cd $@ && $(MAKE))

install: install-core install-scripts install-po

install-core: $(TARGETS)
	-$(MKDIR) $(DESTDIR)$(bindir)
	-$(MKDIR) $(DESTDIR)$(AUXBIN_DIR)
	-$(MKDIR) $(DESTDIR)$(CGIBIN_DIR)
	-$(MKDIR) $(DESTDIR)$(HELP_DIR)
	-$(MKDIR) $(DESTDIR)$(mandir)/man1
	-$(MKDIR) $(DESTDIR)$(mandir)/ja/man1
	$(INSTALL_PROGRAM) $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)
	$(INSTALL_DATA) $(HELP_FILE) $(DESTDIR)$(HELP_DIR)/$(HELP_TARGET)
	$(INSTALL_DATA) $(MAN1) $(DESTDIR)$(mandir)/man1/$(MAN1_TARGET)
	$(INSTALL_DATA) $(MAN1_JA) $(DESTDIR)$(mandir)/ja/man1/$(MAN1_TARGET)
	targets="$(AUXBIN_TARGETS)"; for file in $$targets; \
	do \
		case $$file in \
		$(IMGDISPLAY)) $(INSTALL_W3MIMGDISPLAY) $$file \
			$(DESTDIR)$(AUXBIN_DIR)/$$file;; \
		*) $(INSTALL_PROGRAM) $$file $(DESTDIR)$(AUXBIN_DIR)/$$file;; \
		esac; \
	done
	for file in $(LIB_TARGETS); \
	do \
		$(INSTALL_PROGRAM) $$file $(DESTDIR)$(CGIBIN_DIR)/$$file; \
	done

install-helpfile:
	-$(MKDIR) $(DESTDIR)$(HELP_DIR)
	for file in $(HELP_ALLFILES); \
	do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(HELP_DIR)/$$file; \
	done

install-po:
	NLSTARGET="$(NLSTARGET)"; for subdir in $$NLSTARGET; \
	do \
	 (cd $$subdir && $(MAKE) install); \
	done

all-scripts:
	for dir in $(SCRIPTSUBDIRS);	\
	do	\
		(cd $$dir && $(MAKE) $(MAKE_ARGS)); \
	done

install-scripts: all-scripts
	topdir=`pwd`; \
	for dir in $(SCRIPTSUBDIRS);	\
	do	\
		(cd $$dir && $(MAKE) $(MAKE_ARGS) install); \
	done

uninstall:
	-$(RM) $(bindir)/$(TARGET)
	-for file in $(AUXBIN_TARGETS); \
	do \
		$(RM) -f $(AUXBIN_DIR)/$$file; \
	done
	-for file in $(LIB_TARGETS); \
	do \
		$(RM) -f $(CGIBIN_DIR)/$$file; \
	done
	-for file in $(HELP_ALLFILES); \
	do \
		$(RM) -f $(HELP_DIR)/$$file; \
	done
	-$(RM) -f $(HELP_DIR)/$(HELP_TARGET)
	-$(RM) -f $(mandir)/man1/$(MAN1_TARGET)
	-$(RM) -f $(mandir)/ja/man1/$(MAN1_TARGET)
	-for dir in $(SCRIPTSUBDIRS);	\
	do	\
		(cd $$dir && $(MAKE) $(MAKE_ARGS) uninstall); \
	done

clean: sweep
	-$(RM) -f *.o *.a $(TARGETS) mktable$(EXT)
	-$(RM) -f funcname.tab
	-$(RM) -f funcname.c funcname1.h funcname2.h tagtable.c functable.c
	-for dir in w3mimg libwc; \
	do \
		(cd $$dir && $(MAKE) clean RM="$(RM)"); \
	done
	-for dir in $(SCRIPTSUBDIRS);	\
	do	\
		(cd $$dir && $(MAKE) clean); \
	done

distclean: clean
	for subdir in po scripts w3mimg libwc; \
	do \
		(cd $$subdir && $(MAKE) distclean); \
	done
	-$(RM) -f config.cache config.log config.status
	-$(RM) -f w3mhelp-w3m_en.html w3mhelp-w3m_ja.html
	-$(RM) -f w3mhelp-lynx_en.html w3mhelp-lynx_ja.html
	-$(RM) -f version.c
	-$(RM) -f posubst
	-$(RM) -f config.h Makefile
	-$(RM) -f *core

sweep:
	-$(RM) -f core *~ *.bak *.orig *.rej

depend: 
	makedepend $(CFLAGS) *.c

dist:
	cd ..; tar cvfz w3m-$(VERSION).tar.gz w3m

bindist:
	cd ..; tar cvfz w3m-$(VERSION)-$(MODEL).tar.gz w3m/w3m* w3m/doc* w3m/Bonus* w3m/README w3m/scripts 

