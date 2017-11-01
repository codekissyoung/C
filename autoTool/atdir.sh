#!/bin/bash
echo "Autotools for autotools multi directories (stands for atdirs)"

echo_exit()
{
    echo "ERROR: $@"
    exit 1
}

# 1. prepare directories and dummy files
[ ! -e "atdirs" ] || echo_exit "atdirs exists, please remove or rename it"
mkdir atdirs && cd atdirs || echo_exit "mkdir -p atdirs or cd atdirs failed"

mkdir -p doc build-aux m4 man src/{include/atdirs,lib{1,2},tests} || \
    echo_exit "create subdirectory failed"
touch AUTHORS BUGS ChangeLog COPYING NEWS README || \
    echo_exit "touch dummy files failed"

# 2. generates source, header files
cat > src/include/atdirs/lib1.h <<EOF
#ifndef __ATDIRS_LIB1_H_
#define __ATDIRS_LIB1_H_

void lib1_print();

#endif
EOF
cat > src/include/atdirs/lib2.h <<EOF
#ifndef __ATDIRS_LIB2_H_
#define __ATDIRS_LIB2_H_

void lib2_print();

#endif
EOF

cat > src/lib1/print.c <<EOF
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <atdirs/lib1.h>

void lib1_print()
{
    printf("lib1-print: " PACKAGE_STRING " <lib-1>\n");
}
EOF

cat > src/lib2/print.c <<EOF
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <atdirs/lib2.h>

void lib2_print()
{
    printf("lib2-print: " PACKAGE_STRING " <lib-2>\n");
}
EOF

cat > src/tests/test.c <<EOF
#include <atdirs/lib1.h>
#include <atdirs/lib2.h>

int main(void)
{
    lib1_print();
    lib2_print();
    return 0;
}
EOF

# 2. generates Makefil.am files
cat > Makefile.am << EOF
SUBDIRS = doc man src

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = ChangeLog BUGS COPYING
EOF
# doc, man needs an empty Makefile.am
touch {doc,man}/Makefile.am
# src needs a valid Makefile.am
cat > src/Makefile.am << EOF
SUBDIRS = lib1 lib2 . tests

lib_LTLIBRARIES = libatdirs.la
libatdirs_la_SOURCES =
libatdirs_la_LIBADD = lib1/libatdirs_lib1.la lib2/libatdirs_lib2.la
libatdirs_la_LDFLAGS = -release \$(PACKAGE_VERSION) -version-info 1

include_HEADERS = \$(top_srcdir)/\$(atdirs_includedir)/atdirs/lib1.h \
          \$(top_srcdir)/\$(atdirs_includedir)/atdirs/lib2.h
EOF

cat > src/lib1/Makefile.am << EOF
include \$(top_srcdir)/Makefile.rules

noinst_LTLIBRARIES = libatdirs_lib1.la
libatdirs_lib1_la_SOURCES = print.c
EOF

cat > src/lib2/Makefile.am << EOF
include \$(top_srcdir)/Makefile.rules

noinst_LTLIBRARIES = libatdirs_lib2.la
libatdirs_lib2_la_SOURCES = print.c
EOF

cat > src/tests/Makefile.am << EOF
include \$(top_srcdir)/Makefile.rules

bin_PROGRAMS = test
test_SOURCES = test.c
test_LDADD = ../libatdirs.la
EOF

# Makefile.rules:
cat > Makefile.rules << EOF || echo_exit "generates Makefile.rules failed"
AM_CPPFLAGS = -I\$(top_srcdir)/\$(atdirs_includedir)

install-header:
    \$(MKDIR_P) "\$(DESTDIR)\$(includedir)" || exit $?
    cp -av \$(top_srcdir)/\$(atdirs_includedir) "\$(DESTDIR)\$(includedir)" \
        || exit $?
EOF

# 3. generates configure.ac
cat > configure.ac << EOF
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([atdirs], [1.0.0], [cppgp@qq.com], [ycc], [http://blog.csdn.net/cppgp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([-Wall -Werror gnu])
AM_PROG_AR
LT_INIT
AC_COPYRIGHT([GPLv2])
AC_REVISION([\$Revision: 1.0 \$])
AC_CONFIG_SRCDIR([ChangeLog])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for library functions.
AC_SUBST([atdirs_includedir], [src/include])
AC_CONFIG_FILES([Makefile
         doc/Makefile
         man/Makefile
         src/Makefile
         src/lib1/Makefile
         src/lib2/Makefile
         src/tests/Makefile
         ])
AC_OUTPUT
EOF

# 4. generates configure
echo "now try to generates configure, it may take some seconds, please wait ..."
autoreconf --install -Wall || echo_exit "autoreconf failed"

# 5. builds it
mkdir -p build && cd build || echo_exit "mkdir -p build && cd build failed"
../configure || echo_exit "configure failed"
make || echo_exit "make failed"
make DESTDIR=$PWD/install install || echo_exit "install failed"
echo "now try to make distcheck, it may take some seconds, please wait ..."
make dist && make distcheck || echo_exit "make dist && make distcheck failed"

# 6. run test
echo "Run test:"
./src/tests/test || echo_exit "run test failed"
echo "If you want run test positioned at install, you should set LD_LIBRARY_PATH, or else you need install it in standard directory with root privilege"
echo "Autoconf for multi directories tests success over !"

