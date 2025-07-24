# Process this file with autoconf to produce a configure script.
AC_INIT([golden],3.4.4.post1)
AC_CONFIG_SRCDIR([src/golden.c])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_PROG(POD2MAN, pod2man, pod2man, :)
AC_PATH_PROG([PYTHON2_7], [python2.7], [/bin/false])
# AC_PATH_PROG([PYTHON3_4], [python3.4], [/bin/false])
# AC_PATH_PROG([PYTHON3_5], [python3.5], [/bin/false])
# AC_PATH_PROG([PYTHON3_6], [python3.6], [/bin/false])
# AC_PATH_PROG([PYTHON3_7], [python3.7], [/bin/false])
# AC_PATH_PROG([PYTHON3_8], [python3.8], [/bin/false])
# AC_PATH_PROG([PYTHON3_9], [python3.9], [/bin/false])
# Try do do something more generic so that I don't have to check all existing versions of python.
AC_PATH_PROG([PYTHON3], [python3],  [/bin/false])
if test -z "$PYTHON3"; then
   AC_MSG_ERROR([Could not find a suitable Python 3 interpreter])
fi
# If python3 is not found, try highest versions first
# for ver in 3.13 3.12 3.11 3.10 3.9 3.8 3.7 3.6 3.5 3.4; do
#		        AC_PATH_PROG([PYTHON3], [python$ver])
#			    if test -n "$PYTHON3"; then
#				          break
#					      fi
#					        done
#					fi
#
AC_SUBST([PYTHON3])

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
          
AC_SUBST(dir_for_binding_install)

AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile python/Makefile
                 test/Makefile])
AC_OUTPUT
