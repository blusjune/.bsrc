AC_INIT([FlashIO], [2.2.99.2], [lionel.le_folgoc@inria.fr], [], [http://www.uflip.org/])
AM_INIT_AUTOMAKE([AC_PACKAGE_TARNAME()], [AC_PACKAGE_VERSION()])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# We need GNU extensions on Linux.
AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CC_C99
AC_PROG_RANLIB

# Checks for large file support
AC_SYS_LARGEFILE

# Checks for libraries.
AC_CHECK_LIB([m], [sqrt])

# Checks for header files.
AC_CHECK_HEADERS([ctype.h errno.h inttypes.h limits.h math.h stdint.h stdio.h stdlib.h string.h time.h unistd.h], [], [AC_MSG_FAILURE([Missing header, aborting.])], [])

# Checks for platform
linux_build=no
win32_build=no
AC_CHECK_DECL([_WIN32],
    [win32_build=yes],
    [AC_CHECK_DECL([_WIN64],
        [win32_build=yes], [], [])], [])
AM_CONDITIONAL(WIN32_BUILD, test "x$win32_build" = "xyes")

# Checks for specific headers
if test "x$win32_build" = "xyes"; then
    AC_CHECK_HEADER([windows.h], [], [AC_MSG_FAILURE([Missing header, aborting.])], [])
    AC_CHECK_HEADER([winioctl.h], [], [AC_MSG_FAILURE([Missing header, aborting.)])], [#include <windows.h>])
else
    AC_CHECK_HEADERS([fcntl.h sys/ioctl.h sys/resource.h sys/stat.h sys/times.h sys/types.h],
        [], [AC_MSG_FAILURE([Missing header, aborting.])], [])

    AC_CHECK_HEADERS([linux/fs.h linux/hdreg.h], [linux_build=yes], [linux_build=no], [])
fi
AM_CONDITIONAL(LINUX_BUILD, test "x$linux_build" = "xyes")

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([calloc fopen fclose free fseek fwrite isdigit malloc memset printf sprintf strcat strcpy strdup],
    [], [AC_MSG_FAILURE([Missing function, aborting.])])
# Checks for specific functions
if test "x$win32_build" = "xno"; then
    AC_CHECK_FUNCS([gettimeofday getrusage], [], [AC_MSG_FAILURE([Missing function, aborting.])])

    if test "x$linux_build" = "xyes"; then
        AC_CHECK_FUNCS([posix_memalign], [], [AC_MSG_FAILURE([Missing function, aborting.])])
    fi
fi

# Special handling for strcasecmp()
AC_CHECK_FUNC([strcasecmp], [],
    [AC_CHECK_FUNC([_stricmp],
        [AC_DEFINE([strcasecmp(s1, s2)], [_stricmp(s1, s2)], [Define if compiler doesn't have strcasecmp()])],
        [AC_DEFINE([strcasecmp(s1, s2)], [strcmp(s1, s2)], [Define if compiler doesn't have strcasecmp() nor _stricmp()])])])

# Checks for __func__
AC_CHECK_DECL([__func__], [],
    AC_CHECK_DECL([__FUNCTION__],
        [AC_DEFINE([__func__], [__FUNCTION__], [Define if compiler doesn't have __func__])],
        [AC_DEFINE([__func__], [__FILE__], [Define if compiler doesn't have __func__ nor __FUNCTION__])], []), [])

# Adds specific defines
if test "x$win32_build" = "xyes"; then
    AC_DEFINE([WIN32_LEAN_AND_MEAN], [1], [Lighter Windows binaries])
    AC_DEFINE([DIR_SEPARATOR], ["\\"], [Directory path separator])
    AC_DEFINE([EXEC_PREFIX], [""], [Prefix to execute a binary])
    AC_DEFINE([PAR_PREFIX], ["start"], [Prefix used for parallel tests])
    AC_DEFINE([PAR_SUFFIX], [""], [Suffix used for parallel tests])
    AC_DEFINE([SCRIPT_EXTENSION], [".bat"], [File extension for generated shell scripts])
    AC_DEFINE([SCRIPT_SHEBANGS], ["rem batch script\n"], [First line of generated shell scripts])
    AC_DEFINE([SCRIPT_PAUSE], [" Pause"], [Shell command to wait for user input])
else
    AC_DEFINE([DIR_SEPARATOR], ["/"], [Directory path separator])
    AC_DEFINE([EXEC_PREFIX], ["./"], [Prefix to execute a binary])
    AC_DEFINE([PAR_PREFIX], [""], [Prefix used for parallel tests])
    AC_DEFINE([PAR_SUFFIX], ["&"], [Suffix used for parallel tests])
    AC_DEFINE([SCRIPT_EXTENSION], [".sh"], [File extension for generated shell scripts])
    AC_DEFINE([SCRIPT_SHEBANGS], ["#!/bin/sh\n"], [First line of generated shell scripts])
    AC_DEFINE([SCRIPT_PAUSE], ["echo \"Press Return.\"\nread foo"], [Shell command to wait for user input])
fi

candidate_cflags="$CFLAGS -Wall"
candidate_ldflags="$LDFLAGS"
# Checks for debug
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug], [enable debugging build options (default=no)])],
    [enable_debug=$enableval],
    [enable_debug="no"])
if test "x$enable_debug" = "xyes"; then
    candidate_cflags="$candidate_cflags -O0 -g3 -Wextra -pedantic -std=c99 \
        -Wformat=2 -Wconversion -Wshadow -Wcast-align -Wlogical-op \
        -Waggregate-return -Wswitch-default -Wswitch-enum -Wfloat-equal \
        -Wtraditional-conversion -Wbad-function-cast -Wcast-qual \
        -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes \
        -Wmissing-declarations -Wmissing-format-attribute -Wredundant-decls \
        -Wnested-externs -Wunreachable-code"
fi
AC_MSG_CHECKING(for --enable-debug)
AC_MSG_RESULT($enable_debug)

# Tests all cflags
for flag in $candidate_cflags; do
    old_cflags="$CFLAGS"
    CFLAGS="$flag"
    cflag_ok=no
    AC_COMPILE_IFELSE([/* hello */], [cflag_ok=yes; CFLAGS="$old_cflags $flag"], [CFLAGS="$old_cflags"])
    AC_MSG_CHECKING(whether compiler supports $flag)
    AC_MSG_RESULT($cflag_ok)
done

# Tests all ldflags
for flag in $candidate_ldflags; do
    old_ldflags="$LDFLAGS"
    LDFLAGS="$flag"
    ldflag_ok=no
    AC_LINK_IFELSE([#include <stdlib.h>
        int main(void) { exit(0); }], [ldflag_ok=yes; LDFLAGS="$old_ldflags $flag"], [LDFLAGS="$old_ldflags"])
    AC_MSG_CHECKING(whether linker supports $flag)
    AC_MSG_RESULT($ldflag_ok)
done

AC_OUTPUT([
Doxyfile
Makefile
src/Makefile
src/os/Makefile
])
echo ""
echo "  Win32 build :   ${win32_build}"
echo "  Linux build :   ${linux_build}"
if test "x$win32_build" = "xno" -a "x$linux_build" = "xno"; then
echo "  Dummy build :   yes"
fi
echo "  Debug       :   ${enable_debug}"
echo ""
echo "  CFLAGS      :   $CFLAGS"
echo "  LDFLAGS     :   $LDFLAGS"
