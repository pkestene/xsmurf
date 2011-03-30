#------------------------------------------------------------------------
# AC_MATHEVAL_STATUS --
#
#         Find out if we have libmatheval
#
# Arguments:
#	none
#
# Results:
#
#	Adds the following arguments to configure:
#		--with-libmatheval=...
#
#	Defines the following vars:
#		MATHEVAL_DIR    Full path to the directory containing
#			    the lib/libmatheval.a[so] and include/matheval.h
#------------------------------------------------------------------------

AC_DEFUN([AC_MATHEVAL_STATUS], [

	AC_MSG_RESULT([ ])
	AC_MSG_RESULT([-----------------------------------])
	AC_MSG_RESULT([Checking for lib matheval.])
	# AC_MSG_RESULT([])

	MATHEVAL_LIB=""
	MATHEVAL_INC=""

	# try to parse matheval location from option --with-matheval
	AC_ARG_WITH(matheval, 
	    AC_HELP_STRING([--with-matheval=DIR],
			[use libmatheval, and get include files from DIR/include
			and lib file from DIR/lib]),
	    MATHEVAL_DIR=$withval)
	AC_MSG_CHECKING(for MATHEVAL)

	MATHEVAL_FLAGS=""
	AC_CACHE_VAL(ac_cv_c_mathevalpath,[

		if test "$MATHEVAL_DIR" != ""; then
			if test -f "$MATHEVAL_DIR/include/matheval.h" ; then
				is_matheval_ok="yes"
				AC_MSG_RESULT((cached) yes)
				ac_cv_c_mathevalpath=$MATHEVAL_DIR
				MATHEVAL_INC="-I$MATHEVAL_DIR/include"
				MATHEVAL_LIB="-L$MATHEVAL_DIR/lib -lmatheval"
			else
				is_matheval_ok="no"
				AC_MSG_RESULT(matheval library is not installed in $MATHEVAL_DIR)
			fi
		else
			is_matheval_ok="no"
			# then check for a private matheval installation
			if test x"${ac_cv_c_mathevalpath}" = x ; then
				for i in \
					`ls -d /usr 2>/dev/null` \
					`ls -d /usr/local 2>/dev/null` \
					`ls -d /sw 2>/dev/null` \
					; do
					if test -f "$i/include/matheval.h" ; then
						ac_cv_c_mathevalpath=`(cd $i; pwd)`
						is_matheval_ok="yes"
						MATHEVAL_INC="-I$i/include"
						MATHEVAL_LIB="-L$i/lib -lmatheval"
					fi
				done
			fi
		fi

		if test "$is_matheval_ok" != "yes"; then
			AC_MSG_ERROR(cannot find matheval library)
		fi
	])

	if test x"${ac_cv_c_mathevalpath}" = x ; then
	    AC_MSG_WARN([Can't find matheval library])
	    exit 0
	else
	    AC_MSG_RESULT([matheval found in $ac_cv_c_mathevalpath])
	fi

])