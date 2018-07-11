#ifndef _LANG_PROC_H__
#define _LANG_PROC_H__

#include <vector>
#include <string>

#include <unordered_map>
// 打开g++选项 -std=gnu++0x

using namespace std ;

typedef unordered_map<string, vector<string> > hashMap; 

// get the utf8 len of the string p
size_t get_utf8_len(const char *p) ;

// read the chinese2pinyin dictionary.
int read_chn2pinyin(const string &dict_file, hashMap & chn2py_map) ;

// extract all candidate trie key combinations from the words.
void extract_all_trie_keys(const string& words, const string& english, hashMap & chn2py_map, vector<string> & combination) ;

// select any single key from a word, and combine them to generate a trie key string.
void key_combinations(const vector<vector<string> > & word_key_vec, vector<string> & combination) ;

#endif
