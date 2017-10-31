# 编译生成可执行文件名称
bin_PROGRAMS = cky

# 编译标准
AUTOMAKE_OPTIONS = foreign subdir-objects

# 先扫描子目录
SUBDIRS = src

LTLIBRARIES = libpthread.a

include_HEADERS = include/common.h  include/struct.h  include/thread.h

# 当前目录源文件
cky_SOURCES = main.c

# 添加动态链接库
LIBS = -l pthread -l event
