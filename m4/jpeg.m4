#------------------------------------------------------------------------
# AC_JPEG_STATUS --
#
#         Find out if we have libjpeg
#
# Arguments:
#	none
#
# Results:
#
#	Adds the following arguments to configure:
#		--with-jpeg=...
#
#	Defines the following vars:
#		JPEG_DIR    Full path to the directory containing
#			    the lib/libjpeg.a[so] and include/jpeglib.h
#------------------------------------------------------------------------

AC_DEFUN([AC_JPEG_STATUS], [

	AC_MSG_RESULT([ ])
	AC_MSG_RESULT([-----------------------------------])
	AC_MSG_RESULT([Checking for lib jpeg.])
	# AC_MSG_RESULT([])

	JPEG_LIB=""
	JPEG_INC=""

	# try to parse jpeg location from option --with-jpeg
	AC_ARG_WITH(jpeg, 
	    AC_HELP_STRING([--with-jpeg=DIR],
			[use libjpeg, and get include files from DIR/include
			and lib file from DIR/lib]),
	    JPEG_DIR=$withval)
	AC_MSG_CHECKING(for JPEG)

	JPEG_FLAGS=""
	AC_CACHE_VAL(ac_cv_c_jpegpath,[

		if test "$JPEG_DIR" != ""; then
			if test -f "$JPEG_DIR/include/jpeglib.h" ; then
				is_jpeg_ok="yes"
				AC_MSG_RESULT((cached) yes)
				ac_cv_c_jpegpath=$JPEG_DIR
				JPEG_INC="-I$JPEG_DIR/include"
				JPEG_LIB="-L$JPEG_DIR/lib -ljpeg"
			else
				is_jpeg_ok="no"
				AC_MSG_RESULT(jpeg library is not installed in $JPEG_DIR)
			fi
		else
			is_jpeg_ok="no"
			# then check for a private jpeg installation
			if test x"${ac_cv_c_jpegpath}" = x ; then
				for i in \
					`ls -d /usr 2>/dev/null` \
					`ls -d /usr/local 2>/dev/null` \
					`ls -d /sw 2>/dev/null` \
					; do
					if test -f "$i/include/jpeglib.h" ; then
						ac_cv_c_jpegpath=`(cd $i; pwd)`
						is_jpeg_ok="yes"
						JPEG_INC="-I$i/include"
						JPEG_LIB="-L$i/lib -ljpeg"
					fi
				done
			fi
		fi

		if test "$is_jpeg_ok" != "yes"; then
			AC_MSG_ERROR(cannot find jpeg library)
		fi
	])

	if test x"${ac_cv_c_jpegpath}" = x ; then
	    AC_MSG_WARN([Can't find jpeg library])
	    exit 0
	else
	    AC_MSG_RESULT([jpeg found in $ac_cv_c_jpegpath])
	fi

])