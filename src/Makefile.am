AM_CPPFLAGS = @VMOD_INCLUDES@ -Wall -Werror

vmoddir = @VMOD_DIR@
vmod_LTLIBRARIES = libvmod_gelf.la

libvmod_gelf_la_CFLAGS = @VMOD_INCLUDES@
libvmod_gelf_la_LDFLAGS = -module -export-dynamic -avoid-version -shared

libvmod_gelf_la_SOURCES = \
	vmod_gelf.c
nodist_libvmod_gelf_la_SOURCES = \
	vcc_if.c \
	vcc_if.h

$(libvmod_gelf_la_OBJECTS): vcc_if.h

vcc_if.c vcc_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_gelf.vcc
	@VMODTOOL@ $(top_srcdir)/src/vmod_gelf.vcc

VMOD_TESTS = $(top_srcdir)/src/tests/*.vtc
.PHONY: $(VMOD_TESTS)

$(top_srcdir)/src/tests/*.vtc:
	@VARNISHTEST@ -Dvarnishd=@VARNISHD@ -Dvmod_topbuild=$(abs_top_builddir) $@

check: $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_gelf.vcc \
	$(VMOD_TESTS)

CLEANFILES = $(builddir)/vcc_if.c \
	$(builddir)/vcc_if.h \
	$(builddir)/vmod_gelf.man.rst \
	$(builddir)/vmod_gelf.rst
