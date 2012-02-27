AC_INIT([libm210],
	[0.2],
        [https://bugs.launchpad.net/libm210/],
        [libm210],
        [http://tjjr.fi/sw/libm210/])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_LIBTOOL
AC_PROG_CC
AC_CONFIG_MACRO_DIR([m4])
AC_CHECK_HEADER([libudev.h])
if test "$ac_cv_header_libudev_h" == no
then
  AC_MSG_ERROR([This package needs libudev.h to get compiled.])
fi
AC_CONFIG_FILES([
        Makefile
        src/Makefile
])
AC_OUTPUT
