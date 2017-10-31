#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cky], [1], [1162097842@qq.com])
AM_INIT_AUTOMAKE([cky], [1])
AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h langinfo.h limits.h locale.h netdb.h netinet/in.h nl_types.h stddef.h stdint.h stdlib.h string.h strings.h sys/statvfs.h sys/time.h syslog.h termios.h unistd.h utmpx.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_WAIT3

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
