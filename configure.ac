# ------------------------------------------------------------------------------
# Package setup
# ------------------------------------------------------------------------------
AC_INIT([xsmurf],[2.4.1], [pierre.kestener@cea.fr])

AM_INIT_AUTOMAKE([-Wall -Werror foreign -Wno-portability])
AC_CANONICAL_HOST

#AC_CONFIG_MACRO_DIR([m4])

# ------------------------------------------------------------------------------
# Default flags reinitialization
# ------------------------------------------------------------------------------
if test -z "$CFLAGS"
then
   CFLAGS="-Wall -Wextra"
fi

# ------------------------------------------------------------------------------
# Dummy headline formater
# ------------------------------------------------------------------------------
AC_DEFUN([CONFIGURE_HEADLINE],
[
        echo; echo "+++ $1 +++"
])

# ------------------------------------------------------------------------------
# Checks for tool support
# ------------------------------------------------------------------------------
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_LIBTOOL
AC_PROG_LEX
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_YACC
AC_PROG_INSTALL

AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

#AC_C_CONST
#AC_C_INLINE

# ------------------------------------------------------------------------------
# Checks for C/C++ support
# ------------------------------------------------------------------------------
AC_LANG([C])

# ------------------------------------------------------------------------------
# Enable silent build rules by default
# ------------------------------------------------------------------------------
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],[AC_SUBST([AM_DEFAULT_VERBOSITY], [1])])

# ------------------------------------------------------------------------------
# Turn on double precision
# ------------------------------------------------------------------------------
AC_ARG_ENABLE([double],
	[  --enable-double    Turn on double precision computation],
	[case "${enableval}" in
	      yes) DOUBLE=yes ;;
	      no)  DOUBLE=no ;;
	      *)   AC_MSG_ERROR([bad value ${enableval} for --enable-double]) ;;
	      esac],
   	[DOUBLE=no]
)
AM_CONDITIONAL(USE_DOUBLE_PRECISION, test "$DOUBLE" = "yes")

# ------------------------------------------------------------------------------
#    Check for Tcl/Tk installation
# ------------------------------------------------------------------------------
# the following macros is defined in
# ./m4/tcl.m4
CONFIGURE_HEADLINE([ Tcl/Tk ])
SC_CONFIG_CFLAGS

#AC_PREFIX_DEFAULT(/usr/local)

####################################
#
# set current directory (xsmurf sources)
#
SM_SRC_DIR=`cd .; pwd`
AC_SUBST(SM_SRC_DIR)
AC_DEFINE_UNQUOTED(SM_SRC_DIR_STR,"${SM_SRC_DIR}",[path to the top directory])


################################
# how to generate dependencies ?
################################
dnl AC_PATH_PROG(MKDEP, mkdep)
dnl AC_PATH_PROG(MAKEDEPEND, makedepend)
dnl AC_PATH_PROG(GCCMAKEDEP, gccmakedep)
dnl AC_MSG_CHECKING(how to generate dependency info)

dnl if test "$ac_cv_c_compiler_gnu" = yes; then
dnl 	if test ! -z "$GCCMAKEDEP"; then
dnl 		AC_MSG_RESULT(gccmakedep)
dnl 		MKDEP="$GCCMAKEDEP"	
dnl 	else
dnl 		AC_MSG_RESULT(gcc -MM)
dnl 		MKDEP="$CC -MM"
dnl 	fi
dnl elif test ! -z "$MKDEP"; then
dnl 	AC_MSG_RESULT(mkdep)

dnl 	if test -z "$Tru"; then
dnl 		STDOUT=""
dnl 	else
dnl 		STDOUT=" 2> /dev/null"
dnl 	fi
dnl elif test "$SunWorkShop" = yes; then
dnl 	AC_MSG_RESULT($CC -xM)
dnl 	MKDEP="$CC -xM"
dnl 	STDOUT="> .depend 2> /dev/null"
dnl elif test ! -z "$MAKEDEPEND"; then
dnl 	AC_MSG_RESULT(makedepend)
dnl 	MKDEP="$MAKEDEPEND -f-"
dnl else
dnl 	MKDEP=":"
dnl fi

dnl AC_SUBST(MKDEP)

###########################################
#
# Debug or not debug, that is the question.
#
AC_ARG_ENABLE([debug], 
    [  --enable-debug         Turn on debugging],
   [case "${enableval}" in
      yes) DEBUG=true ;;
      no)  DEBUG=false ;;
      *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
   esac],
   [DEBUG=false]
)

if test x$DEBUG = xtrue
then
	CFLAGS="-g3 -O0 ${CFLAGS}"
else
	CFLAGS="-O2 -DNDEBUG $CFLAGS"
fi

# ------------------------------------------------------------------------------
#    Check for lib dl
# ------------------------------------------------------------------------------
CONFIGURE_HEADLINE([ dl support ])
AC_CHECK_LIB(dl, dlopen)

# ------------------------------------------------------------------------------
#    Check for lib libmatheval
# ------------------------------------------------------------------------------
CONFIGURE_HEADLINE([ libmatheval support ])
#PKG_CHECK_MODULES([MATHEVAL],  [libmatheval >= 1.1])
#AC_SUBST([MATHEVAL_CFLAGS])
#AC_SUBST([MATHEVAL_LIBS])
AC_MATHEVAL_STATUS
CFLAGS="$CFLAGS $MATHEVAL_INC"

# ------------------------------------------------------------------------------
#    Check for lib libreadline
# ------------------------------------------------------------------------------
CONFIGURE_HEADLINE([ libreadline support ])
#VL_LIB_READLINE

AC_ARG_WITH(readline-inc, [  --with-readline-inc=DIR use readline include file from DIR/readline],
	RDL_INC_DIR=$withval)
if test "$RDL_INC_DIR" = ""; then
	AC_CHECK_HEADER(readline/readline.h, ,AC_MSG_ERROR(Couldn't find readline headers))
	AC_CHECK_HEADER(readline/history.h, , AC_MSG_ERROR(Couldn't find readline headers))
else
	AC_MSG_CHECKING(for readline.h)
	if test ! -d $RDL_INC_DIR; then
		AC_MSG_ERROR(Directory $RDL_INC_DIR doesn't exist)
	fi
	if test ! -f $RDL_INC_DIR/readline/readline.h; then
		AC_MSG_ERROR(Readline includes are not found in $RDL_INC_DIR/readline)
	fi
	if test ! -f $RDL_INC_DIR/readline/history.h; then
		AC_MSG_ERROR(Readline includes are not found in $RDl_INC_DIR/readline)
	fi
	RDL_INCLUDES="-I$RDL_INC_DIR"
	AC_MSG_RESULT((cached) yes)
fi

###################################
#
# force location of libreadline.a .
#
AC_ARG_WITH(readline-lib, [  --with-readline-lib=DIR use readline library from DIR],
	RDL_LIB_DIR=$withval)
if test "$RDL_LIB_DIR" = ""; then
	AC_CHECK_LIB(readline, rl_delete, , AC_MSG_ERROR(Couldn't find libreadline.a))
	RDL_LIBRARIES="-lreadline"
else
	AC_MSG_CHECKING(for libreadline.a)
	if test ! -d $RDL_LIB_DIR; then
		AC_MSG_ERROR(Directory $RDL_LIB_DIR doesn't exist)
	fi
	if test ! -f $RDL_LIB_DIR/libreadline.a; then
		AC_MSG_ERROR(Readline library is not found in $RDL_LIB_DIR)
	fi
	RDL_LIBRARIES="-L$RDL_LIB_DIR -lreadline"
	LIBS="-L$RDL_LIB_DIR -lreadline $LIBS"
	AC_MSG_RESULT((cached) yes)
fi

##################################################
#
# set TCL/TK variables and provide options
# --with-tcl and --with-tk
#
SC_PATH_TCLCONFIG
SC_LOAD_TCLCONFIG
AC_SUBST(TCL_LIBS)
AC_SUBST(TCL_XLIBSW)
AC_SUBST(TCL_XINCLUDES)
AC_SUBST(TCL_LIB_SPEC)
AC_SUBST(TCL_INCLUDE_SPEC)
TCL_INCLUDES=$TCL_INCLUDE_SPEC
TCL_LIBRARIES=$TCL_LIN_SPEC

SC_PATH_TKCONFIG
SC_LOAD_TKCONFIG
AC_SUBST(TK_LIBS)
AC_SUBST(TK_XLIBSW)
AC_SUBST(TK_XINCLUDES)
AC_SUBST(TK_LIB_SPEC)
AC_SUBST(TK_INCLUDE_SPEC)
TK_INCLUDES=$TK_INCLUDE_SPEC
TK_LIBRARIES=$TK_LIN_SPEC

# ------------------------------------------------------------------------------
#    Use of tcl memory debug facility.
# ------------------------------------------------------------------------------
CONFIGURE_HEADLINE([ Use of tcl memory debug facility. ])
AC_ARG_ENABLE(mem-debug, [  --enable-mem-debug      allow compilation debug flags],
    [mem_debug_ok=$enableval], [mem_debug_ok=no])
if test "$mem_debug_ok" = "yes"; then
	MEM_DEBUG_FLAGS="-DTCL_MEM_DEBUG -DMEM_DEBUG_FLAG -I$SM_SRC_DIR/main $TCL_INCLUDES"
fi
AC_SUBST(MEM_DEBUG_FLAGS)


# ------------------------------------------------------------------------------
#    Use of message log facility.
# ------------------------------------------------------------------------------
CONFIGURE_HEADLINE([ Use of message log facility. ])
AC_ARG_ENABLE(logmsg, [  --enable-logmsg         add log message flag to compilation],
    [logmsg_ok=$enableval], [logmsg_ok=no])
if test "$logmsg_ok" = "yes"; then
	LOG_MESSAGES_FLAGS="$CFLAGS -DLOG_MESSAGES -I$SM_SRC_DIR/log"
fi
AC_SUBST(LOG_MESSAGES_FLAGS)

# ------------------------------------------------------------------------------
#    Check for FFTW3 lib (single precision)
# ------------------------------------------------------------------------------
CONFIGURE_HEADLINE([ FFTW3 single precision support ])
PKG_CHECK_EXISTS(fftw3f >= 3.0, FFTW3F_FOUND="yes"; [PKG_CHECK_MODULES(FFTW3F, fftw3f >= 3.0)], FFTW3F_FOUND="no")
AC_SUBST([FFTW3F_CFLAGS])
AC_SUBST([FFTW3F_LIBS])
AC_SUBST([FFTW3F_FOUND])
AM_CONDITIONAL(HAVE_FFTW3F, test "$FFTW3F_FOUND" != "no")

FFTW_INC="$FFTW3F_CFLAGS"
FFTW_LIB="$FFTW3F_LIBS"
if test "$FFTW3F_FOUND" = "yes"; then
        is_fftw_ok="yes"
else
	is_fftw_ok="no"
fi


AC_ARG_ENABLE(fftw-threads, [  --enable-fftw-threads   switch used to enable threaded version of fftw (usefull for systems with smp architectures)], [fftw_threads_ok=$enableval], [fftw_threads_ok=no])
if test "$fftw_threads_ok" = "yes"; then
        FFTW_LIB="-lfftw3f_threads $FFTW_LIB -lpthread"                                   
        AC_MSG_RESULT([multithreads version of fftw3f will be used ])
fi

AM_CONDITIONAL(HAVE_FFTW_THREADS, test "x$fftw_threads_ok" = "xyes")
 

if test "$is_fftw_ok" = "yes"; then
   	FFTW_FLAGS="-DUSE_FFTW"
	CV1D_FFT_INC="$FFTW_INC"

	# Name of the include file where the functions prototypes are defined
	CV1D_FFT_INC_NAME="myFftw.h"

	# Name of the real fft function.
	CV1D_FFT_FCT_NAME_R="myFftw_real"

	# Name of the real reverse fft function.
	CV1D_FFT_FCT_NAME_RI="myFftwI_real"

	# Name of the complex fft function.
	CV1D_FFT_FCT_NAME_C="myFftw_cplx"

	# Name of the complex reverse fft function.
	CV1D_FFT_FCT_NAME_CI="myFftwI_cplx"
else
	CV1D_FFT_INC="-I../gfft"

	# Name of the include file where the functions prototypes are defined
	CV1D_FFT_INC_NAME="gfft.h"

	# Name of the real fft function.
	CV1D_FFT_FCT_NAME_R="gfft_real"

	# Name of the real reverse fft function.
	CV1D_FFT_FCT_NAME_RI="gifft_real"

	# Name of the complex fft function.
	CV1D_FFT_FCT_NAME_C="gfft_cplx"

	# Name of the complex reverse fft function.
	CV1D_FFT_FCT_NAME_CI="gifft_cplx"
fi

#--------------------------------------------------------------------
#	LOCATE the X11 header files and the X11 library archive.  Try
#	the ac_path_x macro first, but if it doesn't find the X stuff
#	(e.g. because there's no xmkmf program) then check through
#	a list of possible directories.  Under some conditions the
#	autoconf macro will return an include directory that contains
#	no include files, so double-check its result just to be safe.
#--------------------------------------------------------------------
#AC_MSG_RESULT([-----------------------------------])
AC_MSG_RESULT([ ])
AC_MSG_RESULT([-----------------------------------])
AC_PATH_X
not_really_there=""
if test "$no_x" = ""; then
    if test "$x_includes" = ""; then
	AC_TRY_CPP([#include <X11/XIntrinsic.h>], , not_really_there="yes")
    else
	if test ! -r $x_includes/X11/Intrinsic.h; then
	    not_really_there="yes"
	fi
    fi
fi
if test "$no_x" = "yes" -o "$not_really_there" = "yes"; then
    AC_MSG_CHECKING(for X11 header files)
    XINCLUDES=""
    AC_TRY_CPP([#include <X11/Intrinsic.h>], , XINCLUDES="nope")
    if test "$XINCLUDES" = nope; then
        dirs="/usr/unsupported/include /usr/local/include /usr/X386/include /usr/X11R6/include /usr/X11R5/include /usr/include/X11R5 /usr/include/X11R4 /usr/openwin/include /usr/X11/include /usr/sww/include"
        for i in $dirs ; do
	    if test -r $i/X11/Intrinsic.h; then
		AC_MSG_RESULT($i)
	        XINCLUDES=" -I$i"
		break
	    fi
        done
    fi
else
    if test "$x_includes" != ""; then
	XINCLUDES=-I$x_includes
    else
	XINCLUDES=""
    fi
fi
if test "$XINCLUDES" = nope; then
    AC_MSG_RESULT(couldn't find any!)
    XINCLUDES="# no include files found"
fi

if test "$no_x" = yes; then
    AC_MSG_CHECKING(for X11 libraries)
    XLIBSW=nope
    dirs="/usr/unsupported/lib /usr/local/lib /usr/X386/lib /usrX11R6/lib64 /usr/X11R6/lib /usr/X11R5/lib /usr/lib/X11R5 /usr/lib/X11R4 /usr/openwin/lib /usr/X11/lib /usr/sww/X11/lib"
    for i in $dirs ; do
	if test -r $i/libX11.a -o -r $i/libX11.so -o -r $i/libX11.sl; then
	    AC_MSG_RESULT($i)
	    XLIBSW="-L$i -lX11"
	    x_libraries="$i"
	    break
	fi
    done
else
    if test "$x_libraries" = ""; then
	XLIBSW=-lX11
    else
	XLIBSW="-L$x_libraries -lX11"
    fi
fi
if test "$XLIBSW" = nope ; then
    AC_CHECK_LIB(Xwindow, XCreateWindow, XLIBSW=-lXwindow)
fi
if test "$XLIBSW" = nope ; then
    AC_MSG_RESULT(couldn't find any!  Using -lX11.)
    XLIBSW=-lX11
fi
# End of X11 check --------------------

#########################################
# try to find the location of JPEG lib
#
# ------------------------------------------------------------------------------
#    Check for JPEG library
# ------------------------------------------------------------------------------
CONFIGURE_HEADLINE([ JPEG lib support ])
AC_JPEG_STATUS

# ------------------------------------------------------------------------------
#    Check for lib Hdf5
# ------------------------------------------------------------------------------
CONFIGURE_HEADLINE([ HDF5 support ])
AX_LIB_HDF5()
AM_CONDITIONAL(HAVE_HDF5, test "$with_hdf5" = "yes")
AM_CONDITIONAL(HAVE_HDF5_PARALLEL, test "$HDF5_PARALLEL_ENABLED" = "yes")

#######################################################
#######################################################
#######################################################

AC_PROG_INSTALL

AC_SUBST(XINCLUDES)
AC_SUBST(XLIBSW)
AC_SUBST(TCL_INCLUDES)
AC_SUBST(TCL_LIBRARIES)
AC_SUBST(TK_INCLUDES)
AC_SUBST(TK_LIBRARIES)
AC_SUBST(RDL_LIBRARIES)
AC_SUBST(RDL_INCLUDES)
AC_SUBST(MEM_DEBUG_FLAGS)
AC_SUBST(CC)
AC_SUBST(LDFLAGS)
AC_SUBST(FFTW_INC)
AC_SUBST(FFTW_FLAGS)
AC_SUBST(JPEG_INC)
AC_SUBST(JPEG_LIB)
AC_SUBST(MATHEVAL_INC)
AC_SUBST(MATHEVAL_LIB)

LIBS="$TCL_LIB_SPEC $TK_LIB_SPEC $TK_LIBS $FFTW_LIB $JPEG_LIB $LIBS $MATHEVAL_LIB"

AC_SUBST(CV1D_FFT_INC)
AC_SUBST(CV1D_FFT_INC_NAME)
AC_SUBST(CV1D_FFT_FCT_NAME_R)
AC_SUBST(CV1D_FFT_FCT_NAME_RI)
AC_SUBST(CV1D_FFT_FCT_NAME_C)
AC_SUBST(CV1D_FFT_FCT_NAME_CI)

AC_CONFIG_FILES([
	Makefile 
	cv1d/Makefile 
	cv2d/Makefile 
	cv1d/cv1d_fft.h 
	dyadique/Makefile 
	image/Makefile 
	morph2d/Makefile 
	wt1d/Makefile 
	signal/Makefile 
	stats/Makefile 
	widgets/Makefile 
	wt2d/Makefile 
	wt3d/Makefile 
	interpreter/Makefile 
	interface/Makefile 
	main/Makefile 
	gfft/Makefile 
	edge/Makefile
	main/int_startup.tcl 
	spline/Makefile
	doc/templates/testXsmurf_scalar2d/parameters_gaussian.tcl
	doc/templates/testXsmurf_scalar2d/parameters_mexican.tcl
	doc/templates/testXsmurf_vector2d/parameters_gaussian.tcl
	doc/templates/testXsmurf_vector2d/parameters_mexican.tcl
	doc/templates/testXsmurf_vector2d_LT/parameters_gaussian.tcl
	doc/templates/testXsmurf_vector2d_LT/parameters_mexican.tcl
	doc/templates/testXsmurf_scalar3d/parameters_gaussian.tcl
	doc/templates/testXsmurf_scalar3d/parameters_mexican.tcl
	doc/templates/testXsmurf_vector3d/parameters_gaussian.tcl
	doc/templates/testXsmurf_vector3d/parameters_mexican.tcl
])
AC_CONFIG_COMMANDS([time-stamp], [date >time-stamp])

AC_OUTPUT

AC_MSG_RESULT([
        --------------------------------------------------
        Configuration summary for $PACKAGE_NAME ($VERSION)
        --------------------------------------------------

        * Installation prefix    : $prefix
        * Build debug            : $DEBUG
	
	* TCL version            : $TCL_VERSION
	* TK  version		 : $TK_VERSION
	* FFTW3 single precision : $is_fftw_ok
	* FFTW3 threads		 : $fftw_threads_ok
	* HDF5 enabled           : $with_hdf5 ($HDF5_VERSION)
])
