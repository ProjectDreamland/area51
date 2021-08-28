## $Id: Makefile.am,v 1.16 2001/06/27 14:53:35 aleidinger Exp $

AUTOMAKE_OPTIONS = foreign $(top_srcdir)/ansi2knr

DEFS = @DEFS@ @CONFIG_DEFS@

nasm_sources = \
	choose_table.nas \
	cpu_feat.nas \
	fft3dn.nas \
	scalar.nas

if HAVE_NASM
noinst_LTLIBRARIES = liblameasmroutines.la
liblameasmroutines_la_SOURCES = $(nasm_sources)
am_liblameasmroutines_la_OBJECTS = \
	choose_table$U.lo \
	cpu_feat$U.lo \
	fft3dn$U.lo \
	scalar$U.lo
endif

noinst_HEADERS = nasm.h

INCLUDES = @INCLUDES@ -I$(top_srcdir)/libmp3lame/@CPUTYPE@

SUFFIXES = .nas .lo

EXTRA_liblameasmroutines_la_SOURCES = $(nasm_sources)

CLEANFILES = \
	choose_table.o.lst \
	choose_table.lo.lst \
	cpu_feat.o.lst \
	cpu_feat.lo.lst \
	fft3dn.o.lst \
	fft3dn.lo.lst \
	scalar.o.lst \
	scalar.lo.lst

EXTRA_DIST = \
	fft.nas \
	fftfpu.nas \
	fftsse.nas \
	ffttbl.nas

NASM = @NASM@
NASMFLAGS=@NASM_FORMAT@ -i $(top_srcdir)/libmp3lame/@CPUTYPE@/

.nas.o: $< nasm.h
	$(NASM) $(NASMFLAGS) $< -o $@ -l $@.lst

.nas.lo: $< nasm.h
	$(NASM) $(NASMFLAGS) $< -o $@ -l $@.lst

#$(OBJECTS): libtool
#libtool: $(LIBTOOL_DEPS)
#	$(SHELL) $(top_builddir)/config.status --recheck
