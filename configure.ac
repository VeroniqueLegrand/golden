# Process this file with autoconf to produce a configure script.
AC_INIT([golden],3.3.1)
AC_CONFIG_SRCDIR([src/golden.c])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_PROG(POD2MAN, pod2man, pod2man, :)
AC_PATH_PROG([PYTHON], [python python2.7], [/bin/false])

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FSEEKO
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset strchr strdup strerror strrchr])

# Add 64-bits file support on some hosts
AC_DEFINE([_FILE_OFFSET_BITS], [64],
          [Define to 64 for large file support on some hosts])

AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile python/Makefile
                 test/Makefile])
AC_OUTPUT
