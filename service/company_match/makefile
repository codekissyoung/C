build = release
cc = g++

flags = -O2 -std=c++11 -Wall -fPIC

inc = -I../ -I../frame -I../common -I../hiredis
lib = -lpthread -ldl -lhiredis -L../common -lcommon

srcs = company_match_entry.cpp company_match.cpp StringUtil.cpp ../frame/log.c ../frame/conf.c
objs = $(patsubst %cpp,%o,$(srcs))
target= company_match_entry

all: $(target)

company_match_entry: $(objs)
	$(cc) $(objs) $(lib) -Wl,-E -o $@

%o: %cpp
	$(cc) $(flags) -c -o $@ $< $(inc)

clean:
	rm -f *.o $(target)

