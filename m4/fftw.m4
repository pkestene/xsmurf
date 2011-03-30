###
### Stuff for detecting required librairies (FFTW, ...)
###

##
## the following macros comes from olena autoconf[iguration] files ...
## see : http://www.lrde.epita.fr/cgi-bin/twiki/view/Olena/Olena
##

# AC_WITH_CXX_FFTW

# Checks for availability of fftw from C++ programs.

# This macro sets FFTW_CXXFLAGS and FFTW_LDFLAGS if the library is
# found and its functions available from C++.

AC_DEFUN([AC_WITH_CXX_FFTW],
[dnl
 AC_REQUIRE([AC_PROG_CXX])
 AC_LANG_PUSH([C++])

 AC_ARG_WITH([fftw],
      [AC_HELP_STRING([--with-fftw@<:@=DIR@:>@],
                      [using fftw (DIR = prefix for fftw installation)])])
 FFTW_CXXFLAGS=''
 FFTW_LDFLAGS=''
 if test "x$with_fftw" != xno; then
   if test -n "$with_fftw"; then
     FFTW_CXXFLAGS="-I${with_fftw}/include"
     FFTW_LDFLAGS="-L${with_fftw}/lib"
   fi
   save_CXXFLAGS=$CXXFLAGS
   save_LDFLAGS=$LDFLAGS
   CXXFLAGS="$CXXFLAGS $FFTW_CXXFLAGS"
   LDFLAGS="$LDFLAGS $FFTW_LDFLAGS"
   have_fftw=no
   AC_CHECK_HEADER([fftw.h],
                 [AC_CHECK_LIB([fftw],
                               [fftw2d_create_plan],
                               [have_fftw=yes
                                FFTW_LDFLAGS="$FFTW_LDFLAGS -lrfftw -lfftw"
                                AC_DEFINE([HAVE_FFTW], 1,
                                          [Define to 1 if we can use fftw])])])
   CXXFLAGS=$save_CXXFLAGS
   LDFLAGS=$save_LDFLAGS
 
 fi
 AC_SUBST([FFTW_CXXFLAGS])
 AC_SUBST([FFTW_LDFLAGS])

 AC_LANG_POP([C++])
])

