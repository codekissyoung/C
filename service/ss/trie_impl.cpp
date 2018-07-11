#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "log.h"
#include "trie_impl.h"

bool priority_compare(const Object &lhs, const Object &rhs)
{
    return lhs.priority > rhs.priority;
}

/*******************************************************************/
/*                                                                 */
/* 对结构化的数据对象, 抽取其可能的Key串, 建立Trie树.              */
/* 由于daTrie无法支持自定义类的对象序列化和反序列化功能, 因此无    */
/* 法将Trie树内容落地至文件中去.                                   */
/*                                                                 */
/*******************************************************************/
int build_trie_index(vector<Object> & obj_vec, hashMap & chn2py_map, Trie_Type & trie)
{
    map<string, Trie_Node_Type> keys_to_trie_nodes ;

    vector<string> multi_keys ;
    vector<Object>::iterator iter = obj_vec.begin() ;

    while ( iter != obj_vec.end() )
    {
        extract_all_trie_keys((*iter).chn_name, (*iter).eng_name, chn2py_map, multi_keys) ;
        size_t keys_size = multi_keys.size() ;
        if ( keys_size > 0 )
        {
            for (int i=0; i<keys_size; i++)
                keys_to_trie_nodes[multi_keys[i]].push_back(*iter) ;
        }

        iter ++ ;
        multi_keys.clear() ;
    }
    
    map<string, Trie_Node_Type>::iterator map_iter = keys_to_trie_nodes.begin() ;
    while ( map_iter != keys_to_trie_nodes.end() )
    {
        trie.KEYInsert((char *)(map_iter->first).c_str(), map_iter->second) ;
        map_iter ++ ;
    }
    return 0 ;
}

int query(Trie_Type & global_trie, const string & query_str, Trie_Node_Type & results, int max_num_query)
{
    vector<string> hitKeys ;
    vector<Trie_Node_Type> hitValues ;
    
    global_trie.prefixLike((char *)query_str.c_str(), hitKeys, & hitValues) ;

    /****************************************************************** */
    /* 一次短query词, 可能由于是多个不同的长Key的前缀, 导致命中不同路径 */
    /* 上叶子节点, 这些叶子节点可能对应同一个值. 排重.                  */
    /* 如: 输入字母b, 沿着2条不同路径: beijing, bjing, 都命中 "北京".   */
    /*                                                                  */
    /********************************************************************/

    vector<Trie_Node_Type>::iterator hitIter = hitValues.begin() ;
    while ( hitIter != hitValues.end() )
    {
        Trie_Node_Type::iterator finalIter = hitIter->begin() ;
        while (finalIter != hitIter->end() )
        {
            /* 如果结果中已经有了相同的目标值, 无需插入.  */
            bool existed = false ;
            for (int i=0; i<results.size(); i++)
            {
                if ( *finalIter == results[i] )
                {
                    existed = true ;
                    break ;
                }
            }
            if ( existed == false )
                results.push_back(*finalIter) ;
            finalIter ++ ;
        }
        hitIter ++ ;
    }

    /* 对符合要求的命中结果, 进行排序. */
    sort(results.begin(), results.end(), priority_compare) ;

    /* 如果命中结果数量太多, 丢弃多余的. */
    if ( results.size() > max_num_query )
    {
        results.erase ( results.begin() + max_num_query, results.end()) ;
    }
    return results.size() ;
}
