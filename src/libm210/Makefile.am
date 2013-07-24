AM_CPPFLAGS = -Wall -Werror -Wextra -pedantic -std=gnu99
noinst_LTLIBRARIES = libm210.la
libm210_la_SOURCES = dev.c err.c note.c
noinst_HEADERS = dev.h err.h note.h rawnote.h libudev.h
libm210_la_LDFLAGS = -l:libudev.so.0
