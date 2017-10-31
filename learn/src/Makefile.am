# 静态库 自动编译
lib_LIBRARIES = libstatic.a
libstatic_a_SOURCES = static.c
INCLUDES = -l$(top_srcdir)/include/common.h

# 动态库 自动编译
lib_PROGRAMS = share.so
share_so_SOURCES = share.c
share_so_LDFLAGS = -shared -fPIC




