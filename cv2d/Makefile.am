AM_CFLAGS =  @CFLAGS@ @MEM_DEBUG_FLAGS@ @FFTW_FLAGS@ @LOG_MESSAGES_FLAGS@ -I$(top_srcdir)/log @CV1D_FFT_INC@ -I. @MATHEVAL_INC@

noinst_LTLIBRARIES=libcv2d.la

libcv2d_la_SOURCES=cv2d_misc.c cv2d_a.c cv2d_n.c cv2d.c myFftw2.c

pkginclude_HEADERS = cv2d_fft.h  cv2d.h  cv2d_int.h  cv2d_limits.h  myFftw2.h  myFftw.h
