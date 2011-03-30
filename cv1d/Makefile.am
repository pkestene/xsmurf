AM_CFLAGS =  @CFLAGS@ @MEM_DEBUG_FLAGS@ @FFTW_FLAGS@ @LOG_MESSAGES_FLAGS@ -I$(top_srcdir)/log @CV1D_FFT_INC@ -I. @MATHEVAL_INC@

noinst_LTLIBRARIES=libcv1d.la

libcv1d_la_SOURCES=cv1d_misc.c cv1d_a.c cv1d_n.c cv1d.c myFftw.c

pkginclude_HEADERS = cv1d_fft.h  cv1d.h  cv1d_int.h  cv1d_limits.h  myFftw.h
