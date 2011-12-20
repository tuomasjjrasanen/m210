AC_INIT(
        [m210],
        [0.5],
        [https://bugs.launchpad.net/m210],
        [m210],
        [http://tjjr.fi/sw/m210/])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_CHECK_HEADERS([libm210/dev.h libm210/err.h libm210/note.h],
		 [],
		 [AC_MSG_ERROR([This package needs libm210 to get compiled.])])
AC_CONFIG_FILES([
	Makefile
        src/Makefile
])
AC_OUTPUT
