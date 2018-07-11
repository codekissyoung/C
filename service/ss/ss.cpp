#include <vector>
#include <fstream>

#include "common.h"
#include "interface.h"
#include "log.h"
#include "conf.h"
#include "buffer.h"
#include "daTrie.h"
#include "trie_impl.h"
#include "StringUtil.h"

typedef struct {
    char _chn2pinyin_conf_file[MAX_FILE_PATH_LEN];
    char scenic_trie_dict_file[MAX_FILE_PATH_LEN];
    char user_trie_dict_file[MAX_FILE_PATH_LEN];
    float min_priority;
} ss_conf_t;

static ss_conf_t g_setting;
static Trie_Type *trie_scenic, *trie_user, *this_trie ;

static int _build_failed_pack(struct io_buff *buff)
{
    req_pack_t *req = (req_pack_t *)buff->rbuff;
    rsp_pack_t *rsp = (rsp_pack_t *)buff->wbuff;

    // pack response
    rsp->_header.len = sizeof(rsp_pack_t);
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = -1;

    return 0;
}

/* suggestion_type:
 * 0: scenic
 * 1: user
*/
static int _proc_suggestion(struct io_buff *buff, int suggestion_type)
{
    if ( suggestion_type ==0 )
        this_trie = trie_scenic ;
    else if ( suggestion_type == 1 )
        this_trie = trie_user ;
    else
        return -1 ;

    if ( this_trie == NULL )
        return -2 ;

    is2ss_query_t *req = (is2ss_query_t *)buff->rbuff;
    ss2is_suggestion_t *rsp = (ss2is_suggestion_t *)buff->wbuff;

    rsp->_suggestion_num = 0 ;
    for (int i=0; i<MAX_SUGGESTION_NUM; i++)
    {
        rsp->_id[i] = 0 ;
        memset( rsp->_chn_name[i], 0x00, MAX_SUGGESTION_CHN_LEN) ;
        memset( rsp->_en_name[i], 0x00, MAX_SUGGESTION_EN_LEN) ;
    }
    
    if ( req->_query[0] != '\0' )
    {
        std::vector<Object> suggestions ;
        rsp->_suggestion_num = query(*this_trie, req->_query, suggestions, MAX_SUGGESTION_NUM) ;
        for ( int i=0; i<rsp->_suggestion_num; i++ )
        {
            rsp->_id[i] = suggestions[i].id ;
            strcpy(rsp->_chn_name[i], suggestions[i].chn_name.c_str()) ;
            strcpy(rsp->_en_name[i], suggestions[i].eng_name.c_str()) ;
        }
    }

    /* 以下两种写法都可, 后者传输字节数更少. */

    /*
    rsp->_header.len = sizeof( ss2is_suggestion_t );
    */
    if (suggestion_type == 0 )
    {
        rsp->_header.len = sizeof(ss2is_suggestion_t)-sizeof(rsp->_en_name)+rsp->_suggestion_num * sizeof(rsp->_en_name[0]);
    }
    else if (suggestion_type == 1)
    {
        rsp->_header.len = sizeof(ss2is_suggestion_t)-sizeof(rsp->_chn_name)-sizeof(rsp->_en_name)+rsp->_suggestion_num * sizeof(rsp->_chn_name[0]);
    }
    rsp->_header.magic = req->_header.magic;
    rsp->_header.cmd = req->_header.cmd;
    rsp->_header.sequence = req->_header.sequence+1;
    rsp->_header.state = 0;

    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

// 初始化ss, 只会在主线程中调用一次
int ss_init(const char *conf_file)
{
    int ret = 0;
    ret = conf_init(conf_file);
    if (ret < 0) {
        return -1;
    }

    g_setting.scenic_trie_dict_file[0] = 0x00;
    read_conf_str("SS", "SCENIC_TRIE_DICT_FILE", g_setting.scenic_trie_dict_file, 
                  sizeof(g_setting.scenic_trie_dict_file), "");
    if (g_setting.scenic_trie_dict_file[0] == 0x00)
    {
        conf_uninit() ;
        return -2;
    }

    g_setting.user_trie_dict_file[0] = 0x00;
    read_conf_str("SS", "USER_TRIE_DICT_FILE", g_setting.user_trie_dict_file, 
                  sizeof(g_setting.user_trie_dict_file), "");
    if (g_setting.user_trie_dict_file[0] == 0x00)
    {
        conf_uninit() ;
        return -3;
    }

    g_setting._chn2pinyin_conf_file[0] = 0x00;
    read_conf_str("SS", "CHINESE_PINYIN_FILE", g_setting._chn2pinyin_conf_file, 
                  sizeof(g_setting._chn2pinyin_conf_file), "") ;
    if ( g_setting._chn2pinyin_conf_file[0] == 0x00 )
    {
        conf_uninit() ;
        return -4 ;
    }

    read_conf_float("SS", "MIN_SHOW_PRIORITY", &g_setting.min_priority, 0.0);

    conf_uninit();
    
    // read the chinese2pinyin dictionary.
    unordered_map<string, vector<string> > chn2pinyin_map(100000) ;
    if ( read_chn2pinyin(g_setting._chn2pinyin_conf_file, chn2pinyin_map) < 0 )
        return -5 ;

    trie_scenic = new Trie_Type(0, 0) ;
    trie_user = new Trie_Type(0, 0) ;
    if ( trie_scenic == NULL || trie_user == NULL)
        return -6 ;
    
    std::ifstream fs_scenic, fs_user;
    fs_scenic.open( g_setting.scenic_trie_dict_file, std::ios_base::in );
    fs_user.open( g_setting.user_trie_dict_file, std::ios_base::in );
    if ( fs_scenic.fail() || fs_user.fail() )
        return -7 ;
  
    /*********************   建立景点数据的Trie树   ******************** */

    /* 景点数据词典中, 预留了第4列作为保留字段: priority.                */
    /* 当候选词出现多个时, 根据 priority 字段进行排序, 值越大排名越靠前. */

    std::string line ;
    vector<Object> obj_vec_scenic ;

    while(getline(fs_scenic, line, '\n'))
    {
        std::string _line = StringUtil::TrimBothSides(line, " \r\n\t");
        std::vector<std::string> line_scene_cols;
        StringUtil::SplitString(_line, "\t", line_scene_cols) ;
    
        if (line_scene_cols.size() < 3)
            continue ;
        
        Object obj ;
        obj.chn_name = line_scene_cols[0] ;
        obj.eng_name = line_scene_cols[1] ;
        obj.id = StringUtil::StrToUint64(line_scene_cols[2]) ;
        if ( line_scene_cols.size() == 4 )
            obj.priority = StringUtil::Str2Double(line_scene_cols[3]) ;
        else
            obj.priority = 0.0 ;
        if (obj.priority >= g_setting.min_priority) {
            obj_vec_scenic.push_back(obj) ;
        }
    }
    fs_scenic.close() ;
   
    if ( build_trie_index(obj_vec_scenic, chn2pinyin_map, *trie_scenic) < 0 )
        return -8 ;
    
    /*********************   建立用户数据的Trie树   ******************** */

    vector<Object> obj_vec_user ;
    while(getline(fs_user, line, '\n'))
    {
        std::string _line = StringUtil::TrimBothSides(line, " \r\n\t");
        std::vector<std::string> line_user_cols;

        StringUtil::SplitString(_line, "\t", line_user_cols) ;
        if (line_user_cols.size() < 2)
        {
            // log_txt_err("short of columns, %s", line_user_cols[0].c_str()) ;
            continue ;
        }

        Object obj ;
        obj.chn_name = line_user_cols[0] ;
        obj.id = StringUtil::StrToUint64(line_user_cols[1]) ;
        if (line_user_cols.size() == 3 )
            obj.priority = StringUtil::Str2Double(line_user_cols[2]) ;
        else
            obj.priority = 0.0 ;

        if (obj.priority >= g_setting.min_priority) {
            obj_vec_user.push_back(obj) ;
        }
    }
    fs_user.close() ;
    
    if ( build_trie_index(obj_vec_user, chn2pinyin_map, *trie_user) < 0 )
        return -9 ;

    return 0;
}

int ss_proc(struct io_buff *buff)
{
    req_pack_t *req = (req_pack_t *)buff->rbuff;

    int ret = 0;
    switch (req->_header.cmd) {
        case CMD_SUGGESTION_SCENIC:
            ret = _proc_suggestion(buff, 0);
            break;
        case CMD_SUGGESTION_USER:
            ret = _proc_suggestion(buff, 1);
            break;
        default:
            log_txt_err("unkown command number:[%d]", req->_header.cmd);
            ret = -1;
            break;
    }

    if (ret < 0) {
        _build_failed_pack(buff);
    }

    return ret;
}

// 释放相关资源，只会在主线程中调用一次
int ss_uninit()
{
    if ( trie_scenic != NULL )
    {
        delete trie_scenic ;
        trie_scenic = NULL ;
    }
    if ( trie_user != NULL )
    {
        delete trie_user ;
        trie_user = NULL ;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
