cc = g++
#flags = -g -Wall -fPIC
flags = -Wall -fPIC
inc = -I../frame -I../common
lib = -lpthread -ldl

ss_srcs = ss.cpp trie_impl.cpp lang_proc.cpp StringUtil.cpp
ss_objs = $(patsubst %cpp,%o,$(ss_srcs))
target= ss.so

all: $(target)

ss.so: $(ss_objs)
	$(cc) $(ss_objs) $(lib) -Wl,-E -shared -fPIC -o $@

%o: %cpp
	$(cc) $(flags) -std=gnu++0x -c -o $@ $< $(inc)
# -std=gnu++0x is open for unordered_map structure.

clean:
	rm -f *.o $(target)
