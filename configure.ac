AC_INIT(
        [m210],
        [0.8],
        [https://bugs.launchpad.net/m210],
        [m210],
        [http://tjjr.fi/sw/m210/])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign -Wno-extra-portability])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_PROG_CC
AC_CHECK_HEADER([libudev.h])
if test "$ac_cv_header_libudev_h" == no
then
  AC_MSG_ERROR([This package needs libudev.h to get compiled.])
fi
AC_CONFIG_FILES([
	Makefile
        src/Makefile
	src/libm210/Makefile
])
AC_OUTPUT
