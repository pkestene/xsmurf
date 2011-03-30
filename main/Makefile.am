AM_CFLAGS = -DUSE_READLINE -DX11 @TK_INCLUDES@ @TCL_INCLUDES@ @RDL_INCLUDES@ @XINCLUDES@ @JPEG_INC@

if HAVE_FFTW_THREADS
  AM_CFLAGS += -DFFTW_THREADS
endif

#noinst_LIBRARIES=libgui.a

#libgui_a_SOURCES=smurf_init.c xsm_main.c

pkginclude_HEADERS = smMalloc.h  xsmurf.h

bin_PROGRAMS = xsmurf
xsmurf_SOURCES=smurf_init.c xsm_main.c
xsmurf_LDADD= \
	$(top_builddir)/interface/libinterface.la \
	$(top_builddir)/widgets/libwidgets.la \
	$(top_builddir)/interpreter/libinterpreter.la \
	$(top_builddir)/signal/libsig.la \
	$(top_builddir)/stats/libstats.la \
	$(top_builddir)/image/libimage.la \
	$(top_builddir)/wt2d/libwt2d.la \
	$(top_builddir)/wt3d/libwt3d.la \
	$(top_builddir)/morph2d/libmorph2d.la \
	$(top_builddir)/wt1d/libwt1d.la \
	$(top_builddir)/cv1d/libcv1d.la \
	$(top_builddir)/cv2d/libcv2d.la \
	$(top_builddir)/spline/libspline.la \
	$(top_builddir)/dyadique/libdyadique.la \
	$(top_builddir)/gfft/libFFT.la \
	$(top_builddir)/edge/libedge.la

dist_man_MANS = xsmurf.1

EXTRA_DIST = int_startup.tcl smurfrc xsmurf.spec
