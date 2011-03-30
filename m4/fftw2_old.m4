dnl
dnl Find out if we have an appropriate version of fftw2 (single precision)
dnl

AC_DEFUN([ACX_FFTW2_STATUS],
[

AC_MSG_RESULT([])
AC_MSG_RESULT([    Try to find if FFTW2 is installed and where.])
AC_MSG_RESULT([])

ok="no"
fftw="no"

# Do we have fftw.h ?
AC_CHECK_HEADERS([fftw.h],
                [fftw="yes"],
                [fftw="no"])
# Yes, check that we can compile against it
if test "$fftw" = "yes" ; then
   ok="yes"
   AC_CHECK_LIB(fftw,fftw_create_plan,,
        [
           echo "";
           echo "   The fftw library does not seem to work";
           echo "(I might be in trouble now, not sure yet)";
           echo "";
	   ok="no"
	  ],[-lm]) 
fi

if test "$fftw" = "yes" ; then
# Found fftw, see if it is single precision
        ok="yes"
        AC_RUN_IFELSE(
        AC_LANG_PROGRAM(
        [[#include <fftw.h>]],
        [[#ifndef FFTW_ENABLE_FLOAT
        exit(-1);
        #endif]]),
	[echo "Checking for single-precision fftw...ok"],
	[
        echo "";
        echo "Found a double-precision fftw, but I need single-precision";
        echo "(I might be in trouble now, not sure yet)";
        echo "";
 	fftw="no"
        ok="no"
        ],
        [
        echo "";
        echo "We appear to be cross-compiling, check compiled code";
        echo "(I might be in trouble now)";
        echo "";
	fftw="no"
        ok="no"
        ])
fi
if test "$fftw" = "yes" ; then
# Test a little more carefully
        ok="yes"
        AC_CHECK_LIB(fftw,fftw_create_plan,,
        [
         echo "";
         echo "Alas, fftw library does not seem to work";
         echo "I need to have this library to go further";
         echo "(I might be in trouble now, not sure yet)";
         echo "";
         ok="no"
        ],[-lm])
fi

if test "$fftw" = "yes" ; then
	AC_DEFINE(HAVE_FFTW_H)
fi

if test "$ok" = "no" ; then
        echo "";
        echo "Cannot find a working fftw library or header";
        echo "This is not a problem, but I need to have one";
        echo "to compile the xsmurf software";
        echo "";
        echo "If you want to seperately compile the code,";
        echo "Answer no below (which will stop configure)";
	AC_PROMPT_USER([fftw],
       ["Do you want me to compile fftw"],
       ["yes"])	
fi
if test "$fftw" != "yes" ; then
        echo "";
	echo "Not much I can do, so I'm giving up, sorry";
	exit -1
fi

dnl If we get here and $ok is no, we should compile fftw here
dnl Use fftw for this purpose
if test "$ok" = "yes" ; then
	fftw="no"
fi
]
