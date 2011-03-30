AM_CFLAGS =  @CFLAGS@ @MEM_DEBUG_FLAGS@ @LOG_MESSAGES_FLAGS@ -I$(top_srcdir)/gfft -I$(top_srcdir)/cv1d -I$(top_srcdir)/log @MATHEVAL_INC@

noinst_LTLIBRARIES=libwt1d.la

libwt1d_la_SOURCES = wt1d.c wt1d_collection.c wt1d_wavelets.c

pkginclude_HEADERS = wt1d.h wt1d_int.h
