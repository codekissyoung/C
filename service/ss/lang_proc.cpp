#include <fstream>
#include <set>

#include "log.h"
#include "StringUtil.h"
#include "lang_proc.h"

size_t get_utf8_len(const char *p)
{
    if (!p || !*p)
    {
        return 0;
    }

    const unsigned char *pTemp = reinterpret_cast<const unsigned char *>(p);
    if (*pTemp < 0x80)
    {
        return 1;
    }
    else if ((*pTemp >> 5) == 0x6)
    {
        return 2;
    }
    else if ((*pTemp >> 4) == 0xe)
    {
        return 3;
    }
    else if ((*pTemp >> 3) == 0x1e)
    {
        return 4;
    }
    else
    {
        return 0;
    }
}

int read_chn2pinyin(const string &dict_file, hashMap & chn2py_map)
{
    ifstream dict_fs(dict_file.c_str());
    if (!dict_fs.is_open())
    {
        fprintf(stderr, "open file failure: %s\n", dict_file.c_str()) ;
        return -1;
    }

    int read_line_cnt = 0;
    string line_content;
    while (getline(dict_fs, line_content))
    {
        vector<string> splitUnit ;
        StringUtil::SplitString(line_content, " ", splitUnit) ;
        if (splitUnit.size() < 2)
        {
            continue;
        }
        chn2py_map[splitUnit[0]] = vector<string>(++(splitUnit.begin()), splitUnit.end());
        ++read_line_cnt;
    }
    fprintf(stderr, "read %d lines from file: %s\n", read_line_cnt, dict_file.c_str()) ;
    if ( read_line_cnt < 6779 )
    {
        fprintf(stderr, "short of line numbers: %d\n", read_line_cnt) ;
        return -2 ;
    }

    return 0;
}

/********************************************************************/
/* 从输入字符(可能为中文, 英文或者混杂)中抽取所有可能的 Key 串.     */
/* 生成的 Key 串结果包含以下5种情况:                                */
/*                                                                  */
/* 1. 中文全拼音,         2. 拼音首字母,       3. 文本内容(转小写)  */
/* 4. 英文全单词,         5. 英文单词首字母.                        */
/*                                                                  */
/********************************************************************/
void extract_all_trie_keys(const string& _words, const string& _english, 
                           hashMap & chn2py_map, vector<string> & combination)
{
    combination.clear();

    std::string words = StringUtil::TrimBothSides(_words, " \r\n\t");
    
    /*  保存每个字的全拼音         */
    vector< vector<string> > pinyin ;
    vector<string> pinyin_comb ;

    /* 保存每个字的全拼音的首字母  */
    vector< vector<string> > pinyin_fc ;
    vector<string> pinyin_fc_comb ;
    string lower_case_words;
    const char *head = words.c_str();
    const char *tail = head + words.size();
    while (head < tail)
    {
        /* 英文字母 word_len=1; 汉字, word_len=3 ; */
        size_t word_len = get_utf8_len(head);
        
        /*  由于在中文和拼音的对照字典中, 英文字母对照成为(小写) */
        /*  字母本身, 以下逻辑考虑到了输入为英文单词的情况.      */

        string single_word(head, word_len);
        
        /* 通常情况下文本内容为纯中文, 但是也可能是纯英文, 或者  */
        /* 有大写英文字母的混合内容. 因此将纯小写的文本内容存入. */

        if(1 == word_len) {
            if('A' <= *head && *head < 'Z') {
                lower_case_words += (*head + 'a' - 'A');
            } else {
                lower_case_words += *head;
            }
        } else {
            lower_case_words += single_word;
        }
        

        hashMap::iterator iter = chn2py_map.find(single_word);
        if (iter != chn2py_map.end())
        {
            /*  1. 中文全拼音.  */
            pinyin.push_back(iter->second);

            /*  2. 拼音首字母(声母).  */
            vector<string> first_chars_of_chn;
            for (size_t i=0 ; i<(iter->second).size() ; ++i)
            {
                string first_char = (iter->second)[i].substr(0, 1) ;

                /*  一个汉字可能是多音字. 若多音字首字母相同, 去重. */
                bool existed = false ;
                for (int j=0; j<first_chars_of_chn.size(); j++)
                {
                    if ( first_char == first_chars_of_chn[j] )
                    {
                        existed = true ;
                        break ;
                    }
                }

                /* 全拼可能只有单字母, 也必须去重一次. */
                /*
                for (int k=0; k<(iter->second).size(); k++)
                {
                    if ( first_char == (iter->second)[k] )
                    {
                        existed = true ;
                        break ;
                    }
                }
                */
                if ( existed == false )
                    first_chars_of_chn.push_back(first_char);
            }
            pinyin_fc.push_back(first_chars_of_chn) ;
        }
        head += word_len;
    }
    
    /*  3. 文本内容(纯小写)     */
    combination.push_back(lower_case_words) ;
    key_combinations(pinyin, pinyin_comb);

    // debug begins...
    /*
    vector<string>::iterator iter = pinyin_comb.begin() ;
    while ( iter != pinyin_comb.end() )
    {
        fprintf(stderr, "pinyin_comb: %s\n", (*iter).c_str()) ;
        iter ++ ;
    }
    // debug ends...
    */

    /* 当输入为全英文的时候, 字面内容和"拼音"重复了, 去重. */
    for (int i=0; i<(int)pinyin_comb.size(); i++)
    {
        bool existed = false ;
        for (int j=0; j<(int)combination.size(); j++)
        {
            if ( pinyin_comb[i] == combination[j] )
            {
                existed = true ;
                break ;
            }
        }
        if ( existed == false )
            combination.push_back(pinyin_comb[i]) ;
    }

    key_combinations(pinyin_fc, pinyin_fc_comb);
    
    // debug begins...
    /*
    vector<vector<string> >::iterator _idx = pinyin_fc.begin() ;
    while ( _idx != pinyin_fc.end() )
    {
        for ( iter=(*_idx).begin(); iter!= (*_idx).end(); iter++)
        {
            fprintf(stderr, "pinyin_fc: %s\t", (*iter).c_str()) ;
        }
        
        fprintf(stderr, "\n") ;

        _idx ++ ;
    }
    iter = pinyin_fc_comb.begin() ;
    while ( iter != pinyin_fc_comb.end() )
    {
        fprintf(stderr, "pinyin_fc_comb: %s\n", (*iter).c_str()) ;
        iter ++ ;
    }
    // debug ends...
    */


    for (int i=0; i<(int)pinyin_fc_comb.size(); i++)
        combination.push_back(pinyin_fc_comb[i]) ;
    
    if ( _english.empty() == false )
    {
        /* 4. 若英文单词和拼音没有重复, 也作为一种可能的Key.    */
        string english_key = _english ;
        for (int i=0; i<english_key.size(); i++)
        {
            if ( english_key[i] >= 'A' && english_key[i] <= 'Z')
                english_key[i] -= 'A'-'a' ;
        }
        bool existed = false ;
        for (int i=0; i<combination.size(); i++)
        {
            if (english_key == combination[i])
            {
                existed = true ;
                break ;
            }
        }
        if ( existed == false )
            combination.push_back(english_key) ;
        
        /*  5. 英文单词首字母可能构成一个 Key串. 取小写形式.     */
        std::string english = StringUtil::TrimBothSides(_english, " \r\n\t");
        vector<string> english_words;
        string first_chars_english_words;
        StringUtil::SplitString(english, " ", english_words) ;
        for (int i=0; i<english_words.size(); i++)
        {
            first_chars_english_words.append(english_words[i].substr(0,1)) ;
        }
        for (int i=0; i<first_chars_english_words.size(); i++)
        {
            if ( first_chars_english_words[i] >= 'A' && 
                 first_chars_english_words[i] <= 'Z')

                first_chars_english_words[i] -= 'A'-'a' ;
        }
        combination.push_back( first_chars_english_words ) ;
    }

    /* 各种组合中可能存在前缀子串, 建Trie树时没有必要的节点. 例如:  */
    /* 北京: beijing, beij. 后者无必要.                             */
    /* 注意, 此处判断子串的方法, 假如面临2个子串完全相同的情况, 同  */
    /* 样需要去重.                                                  */

    vector<string> comb_backup = combination ;
    combination.clear() ;
    size_t comb_size = comb_backup.size() ;
    
    vector<bool> be_prefix(comb_size, false) ;

    for (int i=0; i<comb_size; i++)
    {
        for (int j=0; j<comb_size; j++)
        {
            if ( j==i )
                continue ;

            // do removement operation only when comb_backup[j] starts with comb_backup[i] 
            std::size_t pos = comb_backup[j].find(comb_backup[i]);
            if( (pos != string::npos) && (0 == pos) ) {
                if ( comb_backup[j] != comb_backup[i] )
                {
                    be_prefix[i] = true ;
                    break ;
                }
            }
        }
    }
    
    set<string> combination_set ;
    for (int i=0; i<comb_size; i++)
    {
        if ( be_prefix[i] == false ) {
            combination_set.insert(comb_backup[i]) ;
        }
    }
    set<string>::iterator setIter = combination_set.begin() ;
    while ( setIter != combination_set.end() )
    {
        combination.push_back(*setIter) ;
        setIter ++ ;
    }
    return ;
}

// select any single key from a word, and combine them to generate a trie key string.
void key_combinations(const vector<vector<string> > & word_key_vec, vector<string> & combination)
{
    if (word_key_vec.size() == 0)
        return;

    combination = word_key_vec[0];

    int word_cnt = word_key_vec.size();
    for (int i = 1 ; i < word_cnt ; ++i)
    {
        int comb_size = combination.size();
        int key_type_cnt = word_key_vec[i].size();
        for (int j = 0 ; j < key_type_cnt - 1 ; ++j)
        {
            for (int k = 0 ; k < comb_size ; ++k)
            {
                combination.push_back(combination[k]);
            }
        }

        for (int j=0 ; j<key_type_cnt ; ++j)
        {
            for (int k=j*comb_size; k<(j+1)*comb_size ; ++k)
            {
                combination[k].append(word_key_vec[i][j]);
            }
        }
    }
    return ;
}
