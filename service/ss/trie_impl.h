#ifndef _TRIE_IMPL_H__
#define _TRIE_IMPL_H__

#include <string>
#include <vector>
#include <set>
#include "daTrie.h"
#include "lang_proc.h"

using namespace std ;

class Object
{
public:
    bool operator < (const Object &rhs) const 
    {
        if ( id == rhs.id )
            return (chn_name < rhs.chn_name) ;
        else
            return id < rhs.id ;
    }
    
    bool operator == (const Object &rhs) const 
    {
        return ( id==rhs.id || chn_name==rhs.chn_name) ;
    }
    friend bool priority_compare(const Object &lhs, const Object &rhs);

public:
    string chn_name ;
    string eng_name ;
    uint64_t id ;

    float priority;
    /*  当命中多个选项时, 通过该字段来排序. 数值越小, 优先级越高. */
};

/* Trie树的每个节点可对应多个值, 建树前将多个值存入vector. */
typedef vector<Object> Trie_Node_Type ;
typedef daTrie<Trie_Node_Type> Trie_Type ;

// build index
int build_trie_index(vector<Object> & obj_vec, hashMap & chn2py_map, Trie_Type & trie) ;

// query
int query(Trie_Type &, const string & query_str, Trie_Node_Type & results, int nMaxNumToGet) ;

#endif
