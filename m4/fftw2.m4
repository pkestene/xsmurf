# -*- mode: autoconf -*-
#------------------------------------------------------------------------
# AC_FFTW2_STATUS --
#
#         Find out if we have an appropriate version
#         of fftw2 (single precision)
#
# Arguments:
#	none
#
# Results:
#
#	Adds the following arguments to configure:
#		--with-fftw=...
#
#	Defines the following vars:
#		FFTW_DIR	Full path to the directory containing
#			the lib/libfftw.a[so] and include/fftw.h
#------------------------------------------------------------------------

AC_DEFUN([AC_FFTW2_STATUS], [

	AC_MSG_RESULT([ ])
	AC_MSG_RESULT([-----------------------------------])
	AC_MSG_RESULT([Checking for FFTW2 single-precision.])
	# AC_MSG_RESULT([])

	# try to parse FFTW location from option --with-fftw
	AC_ARG_WITH(fftw, 
	    AC_HELP_STRING([--with-fftw=DIR],
			[use fftw, and get include files from DIR/include
			and lib files from DIR/lib]),
	    FFTW_DIR=$withval)
	AC_MSG_CHECKING(for fftw)

	FFTW_FLAGS=""
	AC_CACHE_VAL(ac_cv_c_fftwpath,[

		if test "$FFTW_DIR" != ""; then
		   if (eval grep \"FFTW_ENABLE_FLOAT 1\" $FFTW_DIR/include/fftw.h > /dev/null); then
			is_fftw_ok="yes"
			AC_MSG_RESULT((cached) yes)
			FFTW_FLAGS="-DUSE_FFTW"
			FFTW_INC="-I$FFTW_DIR/include"
			FFTW_LIB="-L$FFTW_DIR/lib -lfftw -L$FFTW_DIR/lib -lrfftw"
			ac_cv_c_fftwpath=$FFTW_DIR
		   else
			is_fftw_ok="no"
			AC_MSG_RESULT(fftw library is not compiled for single precision)
		   fi
		else
		   is_fftw_ok="no"
		   # then check for a private fftw installation
		   if test x"${ac_cv_c_fftwpath}" = x ; then
		      for i in \
			`ls -d /usr 2>/dev/null` \
			`ls -d /usr/local 2>/dev/null` \
			; do
			  if test -f "$i/include/fftw.h" ; then
			     if (eval grep \"FFTW_ENABLE_FLOAT 1\" $i/include/fftw.h > /dev/null); then
				ac_cv_c_fftwpath=`(cd $i; pwd)`
				is_fftw_ok="yes"
				FFTW_DIR="$i"
				FFTW_FLAGS="-DUSE_FFTW"
				FFTW_INC="-I$i/include"
				FFTW_LIB="-L$i/lib -lfftw -L$i/lib -lrfftw"
				break
			     fi
			  fi
		      done
		   fi
		fi

		if test "$is_fftw_ok" != "yes"; then
		   AC_MSG_ERROR(cannot find library fftw compiled with float enabled)
		fi
	])

	if test x"${ac_cv_c_fftwpath}" = x ; then
	    FFTW_FLAGS="# no fftw found"
	    AC_MSG_WARN([Can't find library fftw with float enabled])
	    exit 0
	else
	    FFTW_FLAGS="-DUSE_FFTW"
	    AC_MSG_RESULT([fftw found in $ac_cv_c_fftwpath])
	fi


])