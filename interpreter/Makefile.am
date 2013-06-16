# HDF5 flags
if HAVE_HDF5
HDF5_CPPFLAGS += -DUSE_HDF5 -DH5_NO_DEPRECATED_SYMBOLS 
endif

AM_CFLAGS =  @CFLAGS@ @MEM_DEBUG_FLAGS@ @LOG_MESSAGES_FLAGS@ @TK_INCLUDES@ @TCL_INCLUDES@ -I. -I$(top_srcdir)/cv1d -I$(top_srcdir)/cv2d -I$(top_srcdir)/gfft -I$(top_srcdir)/spline -I$(top_srcdir)/wt2d -I$(top_srcdir)/dyadique -I$(top_srcdir)/edge @CV1D_FFT_INC@ @JPEG_INC@ @MATHEVAL_INC@ $(HDF5_CPPFLAGS)
AM_LDFLAGS = $(HDF5_LDFLAGS)


noinst_LTLIBRARIES=libinterpreter.la

libinterpreter_la_SOURCES=hash_tables.c arguments.c signal_cmds.c \
	stats_cmds.c image_cmds.c morph_cmds.c wt1d_cmds.c \
	wt2d_cmds.c wt3d_cmds.c files_cmds.c stPkg.c cv1dPkg.c \
	cv2dPkg.c dyadPkg.c sm_interpreter_init.c

pkginclude_HEADERS = arguments.h  hash_tables.h  sm_interpreter.h  smPkgInt.h
