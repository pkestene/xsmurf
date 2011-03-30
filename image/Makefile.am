AM_CFLAGS =  @CFLAGS@ @MEM_DEBUG_FLAGS@ -DCONV_DEV -I$(top_srcdir)/gfft  @TCL_INCLUDES@ @FFTW_INC@ @MATHEVAL_INC@

if HAVE_FFTW_THREADS
  AM_CFLAGS += -DFFTW_THREADS
endif

noinst_LTLIBRARIES=libimage.la

libimage_la_SOURCES=image.c image3D.c op.c generator.c nuage.c wt2d.c fft.c wt3d.c

pkginclude_HEADERS = image3D.h  image.h  image_int.h
