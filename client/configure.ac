#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cky-client], [1.0], [1162097842@qq.com])
AM_INIT_AUTOMAKE(cky-client,1.0)
AC_CONFIG_SRCDIR([client.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netdb.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname memset socket])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
