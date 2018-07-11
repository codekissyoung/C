#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <ul_log.h>
//#include "chinese.h"
#include "ul_utf8_to_unicode.h"
#include "ul_dictmatch.h"

//{{{
//global data
typedef u_int(*dm_get_word_func_t)(const char *inbuf, int &pos, const int slen);
static dm_get_word_func_t _g_dm_get_word_funcs[DM_CHARSET_NUM];
// }}}

// {{{ static functions declared here
u_int dm_get_word_gb18030(const char *inbuf, int &pos, const int slen);
u_int dm_get_word_utf8(const char *inbuf, int &pos, const int slen);
u_int dm_get_word(const char *inbuf, int &pos, const int slen, dm_charset_t charset);
int dm_insert_dentry(dm_dict_t* pdict, u_int lastpos, u_int value, u_int &curpos);
int dm_resize_lmlist(dm_dict_t * pdict, u_int newsize);
int dm_resize_strbuf(dm_dict_t* pdict, u_int newsize);
u_int dm_get_hashsize(u_int * sufentry);
u_int dm_get_sufentry(u_int* sufentry, u_int pos);
int dm_adjust_seinfo(dm_dict_t*  pdict);
int dm_resize_seinfo(dm_dict_t * pdict, u_int newsize);
void dm_init_sufentry(u_int* sufentry, u_int hashsize, u_int backsepos);
int dm_resize_dentry(dm_dict_t * pdict, u_int newsize);
void dm_set_sufentry(u_int * sufentry, u_int pos, u_int sepos);
u_int dm_get_backentry(u_int * sufentry);
void dm_set_hashsize(u_int * sufentry, u_int hashsize);
void dm_set_backentry(u_int * sufentry, u_int backentrypos);
u_int dm_seek_entry(dm_dict_t * pdict, u_int lde, u_int value);
int dm_append_lemma(dm_pack_t*ppack, dm_lemma_t* plm, u_int off);
// }}}


int (*dm_prop_str2int_p)(char *) = NULL;
void (*dm_prop_int2str_p)(int, char *) = NULL;


// {{{ dm_dict_create
/*=====================================================================================
 * funtion   : create a dm_dict_t struct
 * return    : NULL, failed; else, pointer to dm_dict_t struct
 *===================================================================================*/
dm_dict_t* dm_dict_create(int lemma_num)
{
    if (lemma_num <= 0)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'lemma_num' should be larger than 0. "
                    "the actual value is %d", lemma_num);
                    */
        return NULL;
    }

    //const char * where = "dm_dict_create";
    dm_dict_t* pdict;
    char* strbuf = NULL;
    u_int* seinfo = NULL;
    dm_entry_t* debuf = NULL;
    dm_lemma_t* lmlist = NULL;

    if ((pdict = (dm_dict_t*)calloc(1, sizeof(dm_dict_t))) == NULL)
    {
        goto failed;
    }

    if ((strbuf = (char*)calloc(20 * lemma_num, sizeof(char))) == NULL)
    {
        goto failed;
    }

    if ((debuf = (dm_entry_t*)calloc(2 * lemma_num, sizeof(dm_entry_t)))
        == NULL)
    {
        goto failed;
    }

    if ((seinfo = (u_int*)calloc(8 * lemma_num, sizeof(u_int)))
        == NULL)
    {
        goto failed;
    }

    if ((lmlist = (dm_lemma_t*)calloc(lemma_num, sizeof(dm_lemma_t)))
        == NULL)
    {
        goto failed;
    }

    pdict->strbuf = strbuf;
    pdict->sbsize = 20 * lemma_num;
    pdict->sbpos  = 0;

    pdict->dentry = debuf;
    pdict->desize = 2 * lemma_num;
    pdict->depos  = 0;

    pdict->seinfo  = seinfo;
    pdict->seisize = 8 * lemma_num;
    pdict->seipos  = 0;

    pdict->lmlist = lmlist;
    pdict->lmsize = lemma_num;
    pdict->lmpos  = 0;

    assert(pdict->seisize > 3);
    pdict->entrance = 0;
    dm_init_sufentry(pdict->seinfo + pdict->entrance, 1, DM_DENTRY_FIRST);
    pdict->seipos = 3;

    return pdict;

failed:
    free(pdict);
    free(strbuf);
    free(debuf);
    free(seinfo);
    free(lmlist);

    //ul_writelog(UL_LOG_FATAL, "error in %s", where);
    return NULL;
}
// }}}

// {{{ dm_dict_create
/*=====================================================================================
 * funtion   : create a dm_dict_t struct
 * return    : NULL, failed; else, pointer to dm_dict_t struct
 *===================================================================================*/
dm_dict_t* dm_dict_create(int lemma_num, dm_charset_t charset)
{
    return dm_dict_create(lemma_num);
}
// }}}

// {{{ dm_dict_load
dm_dict_t* dm_dict_load(const char* fullpath, int lemma_num, dm_charset_t charset)
{
    if (!fullpath)
    {
        /*
        ul_writelog(UL_LOG_FATAL, "argument null exception: 'fullpath' cannot be null");
        */
        return NULL;
    }

    if (charset < 0 || charset >= DM_CHARSET_NUM)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'charset' should be between %d and %d. "
                    "the actual value is %d", 0, DM_CHARSET_NUM - 1, charset);
                    */
        return NULL;
    }

    FILE* fp = NULL;
    char buff[2048];
    char word[256];
    char prop[256];
    //const char* where = "dm_dict_load()";
    dm_dict_t* pdict = NULL;

    pdict = dm_dict_create(lemma_num);
    if (!pdict)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s, create dict failed", where);
        goto failed;
    }

    if ((fp = fopen(fullpath, "rb")) == NULL)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s, file %s open failed", where, fullpath);
        goto failed;
    }

    while (fgets(buff, sizeof(buff), fp))
    {
        if (sscanf(buff, "%s%s", word, prop) != 2)
        {
            //ul_writelog(UL_LOG_NOTICE, "warning in %s, bad format %s", where, buff);
            continue;
        }

        dm_lemma_t lm;
        lm.pstr = word;
        lm.len     = strlen(word);
        lm.prop    = dm_prop_str2int_p(prop);

        if (dm_add_lemma(pdict, &lm, charset) < 0)
        {
            //ul_writelog(UL_LOG_FATAL, "error in %s, add lemma failed", where);
            goto failed;
        }
    }

    fclose(fp);
    fp = NULL;

    for (u_int i = 0; i < pdict->lmpos; i++)
    {
        pdict->lmlist[i].pstr = pdict->strbuf + pdict->lmlist[i].bpos;
    }

    return pdict;

failed:
    if (fp)
    {
        fclose(fp);
    }
    dm_dict_del(pdict);
    return NULL;
}
// }}}

// {{{ dm_dict_load
dm_dict_t* dm_dict_load(const char* fullpath, int lemma_num)
{
    return dm_dict_load(fullpath, lemma_num, DM_CHARSET_GB18030);
}
//  }}}

// {{{ dm_binary_dict_load
dm_dict_t* dm_binarydict_load(const char* fullpath)
{
    if (!fullpath)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'fullpath' cannot be null");
        return NULL;
    }

    //const char *where = "dm_binarydict_load";
    dm_dict_t *pdict = NULL;
    FILE *fp = NULL;
    long nowpos = 0;
    long endpos = 0;

    if ((pdict = (dm_dict_t*)calloc(1, sizeof(dm_dict_t))) == NULL)
    {
        goto failed;
    }

    if ((fp = fopen(fullpath, "rb")) == NULL)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s, file %s open failed", where, fullpath);
        goto failed;
    }

    // read wordbuf
    if (fread(&(pdict->sbpos), sizeof(u_int), 1, fp) != 1)
    {
        goto failed;
    }

    if ((pdict->strbuf = (char*)malloc(pdict->sbpos + 4)) == NULL)
    {
        goto failed;
    }
    pdict->sbsize = pdict->sbpos + 4;

    if (fread(pdict->strbuf, sizeof(char), pdict->sbpos, fp)
        != pdict->sbpos)
    {
        goto failed;
    }

    // read seinfo
    if (fread(&(pdict->seipos), sizeof(u_int), 1, fp) != 1)
    {
        goto failed;
    }

    if ((pdict->seinfo = (u_int*)
                         calloc(pdict->seipos + 1, sizeof(u_int))) == NULL)
    {
        goto failed;
    }
    pdict->seisize = pdict->seipos + 1;

    if (fread(pdict->seinfo, sizeof(u_int), pdict->seipos, fp)
        != pdict->seipos)
    {
        goto failed;
    }

    // read dictentry
    if (fread(&(pdict->depos), sizeof(u_int), 1, fp) != 1)
    {
        goto failed;
    }

    if ((pdict->dentry = (dm_entry_t*)
                         calloc(pdict->depos + 1, sizeof(dm_entry_t))) == NULL)
    {
        goto failed;
    }
    pdict->desize = pdict->depos + 1;

    if (fread(pdict->dentry, sizeof(dm_entry_t), pdict->depos, fp)
        != pdict->depos)
    {
        goto failed;
    }

    // read lemmalist
    if (fread(&(pdict->lmpos), sizeof(u_int), 1, fp) != 1)
    {
        goto failed;
    }
    if ((pdict->lmlist = (dm_lemma_t*)
                         calloc(pdict->lmpos + 1, sizeof(dm_lemma_t))) == NULL)
    {
        goto failed;
    }
    pdict->lmsize = pdict->lmpos + 1;

    for (u_int i = 0; i < pdict->lmpos; i++)
    {
        dm_inlemma_t tmp;
        if (fread(&tmp, sizeof(tmp), 1, fp) != 1)
        {
            goto failed;
        }
        pdict->lmlist[i].len = tmp.len;
        pdict->lmlist[i].prop = tmp.prop;
        pdict->lmlist[i].bpos = tmp.bpos;
        pdict->lmlist[i].pstr = pdict->strbuf + pdict->lmlist[i].bpos;
    }

    // read the entrance
    if (fread(&(pdict->entrance), sizeof(u_int), 1, fp) != 1)
    {
        goto failed;
    }

    nowpos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    endpos = ftell(fp);
    if (endpos > nowpos + 1)
    {
        //ul_writelog(UL_LOG_FATAL, "maybe wrong dict to load");
        //do nothing
    }

    fclose(fp);
    fp = NULL;

    return pdict;

failed:
    if (fp != NULL)
    {
        fclose(fp);
    }
    if (pdict != NULL)
    {
        dm_dict_del(pdict);
    }

    //ul_writelog(UL_LOG_FATAL, "error in %s", where);
    return NULL;
}
// }}}

// {{{ dm_binary_dict_load
dm_dict_t * dm_binarydict_load(const char* fullpath, dm_charset_t charset)
{
    return dm_binarydict_load(fullpath);
}
// }}}

// {{{ dm_binarydict_save
/**
 * -  1  成功
 * - -1  文件创建失败
 * - -2  写数据失败
 * - -3  调整dict失败
 * - -4  无效参数
 */
int dm_binarydict_save(dm_dict_t* pdict, const char* fullname)
{
    if (!pdict)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'pdict' cannot be null");
        return -4;
    }

    if (!fullname)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'fullname' cannot be null");
        return -4;
    }

    //const char* where = "dm_binarydict_save";
    FILE* fp = NULL;
    int err = 1;

    if (dm_adjust_seinfo(pdict) < 0)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s, adjust error", where);
        err = -3;
        goto done;
    }

    if ((fp = fopen(fullname, "wb")) == NULL)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s, file %s open failed", where, fullname);
        err = -1;
        goto done;
    }

    if (fwrite(&(pdict->sbpos), sizeof(u_int), 1, fp) != 1)
    {
        err = -2;
        goto done;
    }

    if (fwrite(pdict->strbuf, sizeof(char), pdict->sbpos, fp)
        != pdict->sbpos)
    {
        err = -2;
        goto done;
    }

    if (fwrite(&(pdict->seipos), sizeof(u_int), 1, fp) != 1)
    {
        err = -2;
        goto done;
    }

    if (fwrite(pdict->seinfo, sizeof(u_int), pdict->seipos, fp)
        != pdict->seipos)
    {
        err = -2;
        goto done;
    }

    if (fwrite(&(pdict->depos), sizeof(u_int), 1, fp) != 1)
    {
        err = -2;
        goto done;
    }

    if (fwrite(pdict->dentry, sizeof(dm_entry_t), pdict->depos, fp)
        != pdict->depos)
    {
        err = -2;
        goto done;
    }

    if (fwrite(&(pdict->lmpos), sizeof(u_int), 1, fp) != 1)
    {
        err = -2;
        goto done;
    }

    for (u_int i = 0; i < pdict->lmpos; ++i)
    {
        dm_inlemma_t tmp;
        tmp.len = pdict->lmlist[i].len;
        tmp.prop = pdict->lmlist[i].prop;
        tmp.bpos = pdict->lmlist[i].bpos;
        if (fwrite(&tmp, sizeof(tmp), 1, fp) != 1)
        {
            err = -2;
            goto done;
        }
    }

    if (fwrite(&(pdict->entrance), sizeof(u_int), 1, fp) != 1)
    {
        err = -2;
        goto done;
    }

done:
    if (fp != NULL)
    {
        fclose(fp);
    }
    fp = NULL;

    if (err == -2)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s, file write error", where);
    }

    return err;
}
// }}}

// {{{ dm_binarydict_save
int dm_binarydict_save(dm_dict_t* pdict, const char* fullname, dm_charset_t charset)
{
    return dm_binarydict_save(pdict, fullname);
}
// }}}

// {{{ dm_dict_del
void dm_dict_del(dm_dict_t* pdict)
{
    if (pdict == NULL)
    {
        return;
    }
    free(pdict->strbuf);
    free(pdict->dentry);
    free(pdict->seinfo);
    free(pdict->lmlist);
    free(pdict);
    pdict = NULL;
    return;
}
// }}}

// {{{ dm_dict_del
void dm_dict_del(dm_dict_t* pdict, dm_charset_t charset)
{
    return dm_dict_del(pdict);
}
//  }}}

// {{{ dm_pack_create
dm_pack_t* dm_pack_create(int max_wnum)
{
    dm_pack_t* ppack = NULL;

    ppack = (dm_pack_t*) calloc(1, sizeof(dm_pack_t));
    if (!ppack)
    {
        goto failed;
    }

    ppack->ppseg = (dm_lemma_t**)calloc(max_wnum, sizeof(dm_lemma_t*));
    if (!ppack->ppseg)
    {
        goto failed;
    }
    ppack->poff     = (u_int*)calloc(max_wnum, sizeof(u_int));
    if (!ppack->poff)
    {
        goto failed;
    }

    ppack->maxterm = max_wnum;
    ppack->ppseg_cnt = 0;
    return ppack;

failed:
    if (ppack)
    {
        free(ppack->ppseg);
        free(ppack->poff);
        free(ppack);
        ppack = NULL;
    }

    return NULL;
}
// }}}

// {{{ dm_pack_create
dm_pack_t* dm_pack_create(int max_wnum, dm_charset_t charset)
{
    return dm_pack_create(max_wnum);
}
//  }}}

// {{{ dm_pack_del
void dm_pack_del(dm_pack_t* ppack)
{
    if (ppack)
    {
        free(ppack->ppseg);
        free(ppack->poff);
        free(ppack);
        ppack = NULL;
    }
}
// }}}

// {{{ dm_pack_del
void dm_pack_del(dm_pack_t* ppack, dm_charset_t charset)
{
    return dm_pack_del(ppack);
}
//  }}}

// {{{ dm_add_lemma
int dm_add_lemma(dm_dict_t* pdict, dm_lemma_t* plm)
{
    return dm_add_lemma(pdict, plm, DM_CHARSET_GB18030);
}
// }}}

// {{{ dm_add_lemma
/**
 * -  1    成功
 * -  0    子字符串已存在
 * - -1    失败
 */
int dm_add_lemma(dm_dict_t* pdict, dm_lemma_t* plm, dm_charset_t charset)
{
    if (!pdict)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'pdict' cannot be null");
        return -1;
    }

    if (!plm)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'plm' cannot be null");
        return -1;
    }

    if (charset < 0 || charset >= DM_CHARSET_NUM)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'charset' should be between %d and %d. "
                    "the actual value is %d", 0, DM_CHARSET_NUM - 1, charset);
                    */
        return -1;
    }

    //const char* where = "dm_add_lemma";
    int curpos = 0;
    int len = plm->len;
    u_int last_depos = DM_DENTRY_FIRST;
    u_int cur_depos = DM_COMMON_NULL;
    u_int value = 0;
    u_int lmpos = DM_COMMON_NULL;

    // check to see if the lemma str already in dict
    if ((lmpos = dm_seek_lemma(pdict, plm->pstr, len, charset)) != DM_LEMMA_NULL)
    {
        pdict->lmlist[lmpos].len = plm->len;
        pdict->lmlist[lmpos].prop = plm->prop;
        return 0;
    }

    // insert dict entry
    while (curpos < len)
    {
        value = dm_get_word(plm->pstr, curpos, len, charset);
        if (value == 0)
        {
            return -1;
        }
        if (dm_insert_dentry(pdict, last_depos, value, cur_depos) < 0)
        {
            //ul_writelog(UL_LOG_FATAL, "error in %s, insert fail", where);
            return -1;
        }
        last_depos = cur_depos;
    }
    assert(pdict->dentry[cur_depos].value == value);

    if (pdict->lmpos == pdict->lmsize)
    {
        if (dm_resize_lmlist(pdict, pdict->lmsize * 2) != 1)
        {
            //ul_writelog(UL_LOG_FATAL, "error in %s, resize fail", where);
            return -1;
        }
    }
    assert(pdict->lmpos < pdict->lmsize);
    lmpos = pdict->lmpos;
    pdict->lmpos++;
    pdict->dentry[cur_depos].lemma_pos = lmpos;

    // set lemma properties
    pdict->lmlist[lmpos].len = len;
    pdict->lmlist[lmpos].prop = plm->prop;

    // add the lemma str to strbuf
    while (pdict->sbpos + len + 1 > pdict->sbsize)
    {
        if (dm_resize_strbuf(pdict, pdict->sbsize * 2) != 1)
        {
            //ul_writelog(UL_LOG_FATAL, "error in %s, resize fail", where);
            return -1;
        }
    }
    assert(pdict->sbpos + len + 1 <= pdict->sbsize);

    memcpy(pdict->strbuf + pdict->sbpos, plm->pstr, len);
    pdict->strbuf[pdict->sbpos + len] = 0;
    pdict->lmlist[lmpos].pstr = pdict->strbuf + pdict->sbpos;
    pdict->lmlist[lmpos].bpos = pdict->sbpos;
    pdict->sbpos += len + 1;

    return 1;

}
// }}}

// {{{ dm_search
int dm_search(dm_dict_t* pdict, dm_pack_t* ppack, const char* inbuf, int inlen, int opertype)
{
    return dm_search(pdict, ppack, inbuf, inlen, opertype, DM_CHARSET_GB18030);
}
//  }}}

// {{{ dm_search
/**
 * -  0    成功
 * - -1    失败
 * -  2    匹配结果数量超过maxterm
 */
int dm_search(
    dm_dict_t* pdict,
    dm_pack_t* ppack,
    const char* inbuf,
    int inlen,
    int opertype,
    dm_charset_t charset)
{
    if (!pdict)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'pdict' cannot be null");
        return -1;
    }

    if (!ppack)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'ppack' cannot be null");
        return -1;
    }

    if (!inbuf)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'inbuf' cannot be null");
        return -1;
    }

    if (opertype != DM_OUT_ALL && opertype != DM_OUT_FMM)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'opertype' should be "
                    "DM_OUT_ALL(%d) or DM_OUT_FMM(%d). the actual value is %d",
                    DM_OUT_ALL, DM_OUT_FMM, opertype);
                    */
        return -1;
    }

    if (charset < 0 || charset >= DM_CHARSET_NUM)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'charset' should be between %d and %d. "
                    "the actual value is %d", 0, DM_CHARSET_NUM - 1, charset);
                    */
        return -1;
    }

    //const char* where = "dm_search";

    int bpos = 0;
    int pos = 0;
    int nextpos = pos;
    u_int value = 0;
    u_int nde = 0;
    u_int lde = DM_DENTRY_FIRST;
    u_int lmpos = DM_LEMMA_NULL;
    u_int slemma = DM_LEMMA_NULL;

    ppack->ppseg_cnt = 0;

    if (opertype == DM_OUT_FMM)
    {
        while (pos < inlen)
        {
            bpos = pos;
            value = dm_get_word(inbuf, pos, inlen, charset);
            if (value == 0)
            {
                break;
            }

            nextpos = pos;

            while (value != 0 && (nde = dm_seek_entry(pdict, lde, value)) != DM_DENTRY_NULL)
            {
                if ((lmpos = pdict->dentry[nde].lemma_pos) != DM_LEMMA_NULL)
                {
                    slemma = lmpos;
                    nextpos = pos;
                    lde = nde;
                }

                if (pos < inlen)
                {
                    value = dm_get_word(inbuf, pos, inlen, charset);
                }
                else
                {
                    value = 0;
                }
                lde = nde;
            }

            if (slemma != DM_LEMMA_NULL)
            {
                dm_lemma_t* p = pdict->lmlist + slemma;
                p->pstr = pdict->strbuf + p->bpos;
                if (dm_append_lemma(ppack, p, bpos) < 0)
                {
                    //ul_writelog(UL_LOG_FATAL, "error in %s, append failed", where);
                    return 2;
                }
            }

            lde = DM_DENTRY_FIRST;
            slemma = DM_LEMMA_NULL;
            pos = nextpos;
        }
    }
    else if (opertype == DM_OUT_ALL)
    {
        while (pos < inlen)
        {
            bpos = pos;
            value = dm_get_word(inbuf, pos, inlen, charset);
            if (value == 0)
            {
                break;
            }

            nextpos = pos;

            while (value != 0 && (nde = dm_seek_entry(pdict, lde, value)) != DM_DENTRY_NULL)
            {
                if ((lmpos = pdict->dentry[nde].lemma_pos) != DM_LEMMA_NULL)
                {
                    dm_lemma_t* p = pdict->lmlist + lmpos;
                    p->pstr = pdict->strbuf + p->bpos;
                    if (dm_append_lemma(ppack, p, bpos) < 0)
                    {
                        //ul_writelog(UL_LOG_FATAL, "error in %s, append failed", where);
                        return 2;
                    }
                }

                if (pos < inlen)
                {
                    value = dm_get_word(inbuf, pos, inlen, charset);
                }
                else
                {
                    value = 0;
                }
                lde = nde;
            }

            lde = DM_DENTRY_FIRST;
            pos = nextpos;
        }
    }

    return 0;
}
// }}}

// {{{ dm_seek_lemma
u_int dm_seek_lemma(dm_dict_t* pdict, const char* term, int len)
{
    return dm_seek_lemma(pdict, term, len, DM_CHARSET_GB18030);
}
//  }}}

// {{{ dm_seek_lemma
/**
 * - > 0               找到，返回位置
 * - DM_LEMMA_NULL     未找到
 */
u_int dm_seek_lemma(dm_dict_t * pdict, const char* term, int len, dm_charset_t charset)
{
    if (!pdict)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'pdict' cannot be null");
        return DM_LEMMA_NULL;
    }

    if (!term)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'term' cannot be null");
        return DM_LEMMA_NULL;
    }

    if (charset < 0 || charset >= DM_CHARSET_NUM)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'charset' should be between %d and %d. "
                    "the actual value is %d", 0, DM_CHARSET_NUM - 1, charset);
                    */
        return DM_LEMMA_NULL;
    }

    u_int value = 0;
    int curpos = 0;
    u_int sufpos = 0;
    u_int hsize = 0;
    u_int hpos = 0;
    u_int nde = 0;

    sufpos = pdict->entrance;

    while (curpos < len)
    {
        if (sufpos == DM_SUFENTRY_NULL)
        {
            return DM_LEMMA_NULL;
        }

        value = dm_get_word(term, curpos, len, charset);
        if (value == 0)
        {
            return DM_LEMMA_NULL;
        }
        hsize = dm_get_hashsize(pdict->seinfo + sufpos);
        hpos = value % hsize;
        nde = dm_get_sufentry(pdict->seinfo + sufpos, hpos);
        if ((nde == DM_DENTRY_NULL) || (pdict->dentry[nde].value != value))
        {
            return DM_LEMMA_NULL;
        }
        sufpos = pdict->dentry[nde].suffix_pos;
    }
    assert(pdict->dentry[nde].value == value);

    return pdict->dentry[nde].lemma_pos;
}
// }}}

// {{{ dm_get_word_gb18030
u_int dm_get_word_gb18030(const char *inbuf, int &pos, const int slen)
{
    //assert(inbuf);

    u_int value = 0;
    unsigned char *tmpbuf = (unsigned char*)(inbuf + pos);
    int off = 0;

    if (pos >= slen)
    {
        return 0;
    }

    value = tmpbuf[0];
    off = 1;

    if ((pos + 1 < slen) && (tmpbuf[0] >= 0x81 && tmpbuf[0] <= 0xfe))
    {
        if (tmpbuf[1] >= 0x40 && tmpbuf[1] <= 0xfe)
        {
            off = 2;
            value = tmpbuf[0] * 256 + tmpbuf[1];
        }
        else if ((tmpbuf[1] >= 0x30 && tmpbuf[1] <= 0x39))
        {
            if (pos + 3 < slen
                && (tmpbuf[2] >= 0x81 && tmpbuf[2] <= 0xfe)
                && (tmpbuf[3] >= 0x30 && tmpbuf[3] <= 0x39))
            {
                value = tmpbuf[0] * 16777216 + tmpbuf[1] * 65536 + tmpbuf[2] * 256 + tmpbuf[3];
                off = 4;
            }
        }
    }

    pos += off;

    return value;
}
// }}}


// {{{ dm_get_word_utf8
u_int dm_get_word_utf8(const char *inbuf, int &pos, const int slen)
{
    //assert(inbuf);

    const char* tmpbuf = inbuf + pos;
    int off = 0;
    u_int value = 0;

    if (pos >= slen)
    {
        return 0;
    }

    const char* next = uln_utf8_next_strict(tmpbuf);
    if (NULL == next)
    {
        return 0;
    }
    off = next - tmpbuf;
    pos += off;

    value = uln_utf8_char_to_unicode(tmpbuf);
    return value;
}
// }}}

// {{{ dm_get_word
u_int dm_get_word(const char *inbuf, int &pos, const int slen, dm_charset_t charset)
{
    //assert(inbuf);
    //assert(charset >= 0 && charset < DM_CHARSET_NUM);

    //int old_pos = pos;
    u_int ret = _g_dm_get_word_funcs[charset](inbuf, pos, slen);
    if (ret == 0)
    {
        //ul_writelog(UL_LOG_TRACE, "dm_get_word failed at offset %d in \"%s\"", old_pos, inbuf);
    }
    return ret;
}
// }}}

// {{{ dm_insert_dentry
/*========================================================================================
 * function  : insert a new dentry to the worddict
 * argu      : pdict, the worddict
 *           : lastpos, the prefix dictentry id
 *           : value, the new value
 *           : curpos, the output argu, store the current dictentry
 * return    : 1  if success
 *           : 0  if existed
 *           : <0 if failed
 ========================================================================================*/
int dm_insert_dentry(dm_dict_t* pdict, u_int lastpos, u_int value, u_int &curpos)
{
    assert(pdict);
    assert((lastpos == DM_DENTRY_FIRST) || (lastpos < pdict->depos));

    //const char* where = "dm_insert_dentry";
    u_int tmpdepos;
    u_int curdepos;
    u_int sufpos;
    u_int hsize;
    u_int hpos;

    if (lastpos != DM_DENTRY_FIRST)
    {
        sufpos = pdict->dentry[lastpos].suffix_pos;
    }
    else
    {
        sufpos = pdict->entrance;
    }

    if (sufpos == DM_SUFENTRY_NULL)
    {
        // the last dict entry has no suffix entry
        if (pdict->seipos + 3 > pdict->seisize)
        {
            // the seinfo list is not enough, adjust it
            if (dm_adjust_seinfo(pdict) < 0)
            {
                //ul_writelog(UL_LOG_FATAL, "error in %s", where);
                return -1;
            }
        }

        while (pdict->seipos + 3 > pdict->seisize)
        {
            // the seinfo list have no enough space again, resize it
            if (dm_resize_seinfo(pdict, pdict->seisize * 2) != 1)
            {
                //ul_writelog(UL_LOG_FATAL, "error in %s", where);
                return -1;
            }
        }
        assert(pdict->seipos +  3 <= pdict->seisize);
        assert(lastpos != DM_DENTRY_FIRST);
        pdict->dentry[lastpos].suffix_pos = pdict->seipos;
        dm_init_sufentry(pdict->seinfo + pdict->seipos, 1, lastpos);
        pdict->seipos += 3;
        sufpos = pdict->dentry[lastpos].suffix_pos;
    }

    assert(sufpos != DM_SUFENTRY_NULL);
    hsize = dm_get_hashsize(pdict->seinfo + sufpos);
    assert(hsize > 0);
    hpos = value % hsize;
    tmpdepos = dm_get_sufentry(pdict->seinfo + sufpos, hpos);
    if ((tmpdepos != DM_DENTRY_NULL) && (pdict->dentry[tmpdepos].value == value))
    {
        // the dentry has exist
        curpos = tmpdepos;
        return 0;
    }
    else
    {
        // no such dictentry in worddict
        // create it
        if (pdict->depos == pdict->desize)
        {
            if (dm_resize_dentry(pdict, pdict->desize * 2) != 1)
            {
                //ul_writelog(UL_LOG_FATAL, "error in %s", where);
                return -1;
            }
        }

        assert(pdict->depos < pdict->desize);
        curdepos = pdict->depos;
        pdict->depos++;
        pdict->dentry[curdepos].value = value;
        pdict->dentry[curdepos].lemma_pos    = DM_LEMMA_NULL;
        pdict->dentry[curdepos].suffix_pos    = DM_SUFENTRY_NULL;

        // insert into the suffix list of the lastpos dictentry
        if (tmpdepos == DM_DENTRY_NULL)
        {
            // the position in hash table is not used.
            dm_set_sufentry(pdict->seinfo + sufpos, hpos, curdepos);
            curpos = curdepos;
            return 1;
        }
        else
        {
            int newhash;
            for (newhash = hsize + 1;; newhash++)
            {
                int conflict = 0;

                if (pdict->seipos + newhash + 2 > pdict->seisize)
                {
                    // the seinfo list is not enough, adjust it
                    if (dm_adjust_seinfo(pdict) < 0)
                    {
                        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
                        return -1;
                    }
                }

                while (pdict->seipos + newhash + 2 > pdict->seisize)
                {
                    // the seinfo list have no enough space again, resize it
                    if (dm_resize_seinfo(pdict, pdict->seisize * 2) != 1)
                    {
                        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
                        return -1;
                    }
                }
                assert(pdict->seipos +  newhash + 2  <= pdict->seisize);

                if (lastpos != DM_DENTRY_FIRST)
                {
                    sufpos = pdict->dentry[lastpos].suffix_pos;
                }
                else
                {
                    sufpos = pdict->entrance;
                }

                dm_init_sufentry(pdict->seinfo + pdict->seipos, newhash, lastpos);
                for (u_int i = 0; i < hsize; i++)
                {
                    u_int others;
                    if ((others = dm_get_sufentry(pdict->seinfo + sufpos, i)) != DM_DENTRY_NULL)
                    {
                        u_int tmpvalue;
                        u_int tmphpos;

                        tmpvalue = pdict->dentry[others].value;
                        tmphpos = tmpvalue % newhash;
                        if (dm_get_sufentry(pdict->seinfo + pdict->seipos, tmphpos)
                            == DM_DENTRY_NULL)
                        {
                            dm_set_sufentry(pdict->seinfo + pdict->seipos, tmphpos, others);
                        }
                        else
                        {
                            conflict = 1;
                            break;
                        }
                    }
                }
                if (conflict == 0)
                {
                    u_int tmpvalue;
                    u_int tmphpos;

                    tmpvalue = pdict->dentry[curdepos].value;
                    tmphpos = tmpvalue % newhash;
                    if (dm_get_sufentry(pdict->seinfo + pdict->seipos, tmphpos) == DM_DENTRY_NULL)
                    {
                        dm_set_sufentry(pdict->seinfo + pdict->seipos, tmphpos, curdepos);
                    }
                    else
                    {
                        conflict = 1;
                    }
                }
                if (conflict == 0)
                {
                    // have find the minimal hash size
                    if (lastpos != DM_DENTRY_FIRST)
                    {
                        pdict->dentry[lastpos].suffix_pos = pdict->seipos;
                    }
                    else
                    {
                        pdict->entrance = pdict->seipos;
                    }
                    pdict->seipos += newhash + 2;
                    curpos = curdepos;
                    return 1;
                }
            }
        }
    }
}
// }}}

//    {{{ dm_resize_lmlist
/*========================================================================================
 * function : resize the lemma list
 * argu     : pdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 ========================================================================================*/
int dm_resize_lmlist(dm_dict_t* pdict, u_int newsize)
{
    assert(pdict);

    //const char* where = "dm_resize_lmlist";
    dm_lemma_t * plemmalist;

    if (newsize < pdict->lmpos)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
        return -1;
    }
    if (newsize == pdict->lmsize)
    {
        return 0;
    }
    if ((plemmalist = (dm_lemma_t*)realloc(
                          pdict->lmlist, newsize * sizeof(dm_lemma_t))) != NULL)
    {
        //success
        pdict->lmlist    = plemmalist;
        pdict->lmsize    = newsize;
        return 1;
    }
    else
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
        // failed
        return -2;
    }
}
// }}}

// {{{ dm_resize_strbuf
/*========================================================================================
 * function : resize the word buf
 * argu     : pdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 ========================================================================================*/
int dm_resize_strbuf(dm_dict_t* pdict, u_int newsize)
{
    assert(pdict);

    //const char* where = "dm_resize_wordbuf";
    char * newwordbuf;

    if (newsize < pdict->sbpos)
    {

        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
        return -1;
    }
    if (newsize == pdict->sbsize)
    {
        return 0;
    }
    if ((newwordbuf = (char*)realloc(pdict->strbuf, newsize)) != NULL)
    {
        //success
        pdict->strbuf = newwordbuf;
        pdict->sbsize = newsize;
        return 1;
    }
    else
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
        // failed
        return -2;
    }
}
// }}}

//    {{{ dm_get_hashsize
u_int dm_get_hashsize(u_int* sufentry)
{
    assert(sufentry);
    return sufentry[0];
}
// }}}

u_int dm_get_sufentry(u_int* sufentry, u_int pos)
{
    assert(sufentry);
    assert(pos < sufentry[0]);
    return sufentry[pos + 2];
}

//    {{{ dm_adjust_seinfo
/*========================================================================================
 * function : Adjust the sufentry info list. Because when add item, the
 *          : hash size of the suffix entry list will be changed, so a new
 *          : suffix entry buffer will be used, and the last one will drop
 *          : to unused but cannot be used again. we must adjust the suffix
 *          : to convert the unused hole to useful.
 * argu     : pdict : the worddict
 * return   : 1 if success
 *          : -1 if the worddict is inconsistent
 ========================================================================================*/
int dm_adjust_seinfo(dm_dict_t * pdict)
{
    assert(pdict);

    //const char* where = "dm_adjuest_seinfo";
    u_int nextentry = 0;
    u_int newpos = 0;
    u_int curde;
    u_int hsize;

    while (nextentry < pdict->seipos)
    {
        curde = dm_get_backentry(pdict->seinfo + nextentry);
        hsize = dm_get_hashsize(pdict->seinfo + nextentry);

        if ((curde != DM_DENTRY_FIRST) && (curde >= pdict->depos))
        {
            // inconsistent
            //ul_writelog(UL_LOG_FATAL, "error in %s", where);
            return -1;
        }
        if (((curde == DM_DENTRY_FIRST) && (pdict->entrance != nextentry))
            || ((curde != DM_DENTRY_FIRST) && (pdict->dentry[curde].suffix_pos != nextentry)))
        {
            // not used, move the next
            nextentry += hsize + 2;
        }
        else
        {
            // used
            if (nextentry != newpos)
            {
                // need move
                memmove(pdict->seinfo + newpos,
                        pdict->seinfo + nextentry,
                        (hsize + 2) * sizeof(u_int));
                if (curde != DM_DENTRY_FIRST)
                {
                    pdict->dentry[curde].suffix_pos = newpos;
                }
                else
                {
                    pdict->entrance = newpos;
                }
            }
            // meve the two point to next;
            nextentry += hsize + 2;
            newpos += hsize + 2;
        }
    }
    assert(pdict->seipos == nextentry);
    pdict->seipos = newpos;

    return 1;
}
// }}}

//    {{{ dm_resize_seinfo
/*========================================================================================
 * function : resize the sufentry info list
 * argu     : pdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 *=======================================================================================*/
int dm_resize_seinfo(dm_dict_t * pdict, u_int newsize)
{
    assert(pdict);

    //const char* where = "dm_resize_seinfo";
    u_int* newseinfo;

    if (newsize < pdict->seipos)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
        return -1;
    }
    if (newsize == pdict->seisize)
    {
        return 0;
    }
    if ((newseinfo = (u_int*)realloc(pdict->seinfo, newsize * sizeof(u_int))) != NULL)
    {
        //success
        pdict->seinfo = newseinfo;
        pdict->seisize = newsize;
        return 1;
    }
    else
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
        // failed
        return -2;
    }
}
// }}}

void dm_init_sufentry(u_int* sufentry, u_int hashsize, u_int backsepos)
{
    assert(sufentry);
    dm_set_hashsize(sufentry, hashsize);
    dm_set_backentry(sufentry, backsepos);
    for (int i = 0; i < (int)hashsize; i++)
    {
        sufentry[2 + i] = DM_DENTRY_NULL;
    }
    return;
}

//    {{{ dm_resize_dentry
/*========================================================================================
 * function : resize the dictentry list
 * argu     : pdict : the worddict
 *          : u_int :newsize , the new size
 * return   : 1 if successed.
 *          : 0 if the new size is same the old
 *          : <0 failed.
 *          : -1 new size is small than used
 *          : -2 memory allocation failed
 =======================================================================================*/
int dm_resize_dentry(dm_dict_t * pdict, u_int newsize)
{
    assert(pdict);

    //const char* where = "dm_resize_dentry";
    dm_entry_t* pde;

    if (newsize < pdict->depos)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
        return -1;
    }
    if (newsize == pdict->desize)
    {
        return 0;
    }
    if ((pde = (dm_entry_t*)realloc(pdict->dentry, newsize * sizeof(dm_entry_t))) != NULL)
    {
        //success
        pdict->dentry = pde;
        pdict->desize = newsize;
        return 1;
    }
    else
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
        // failed
        return -2;
    }
}
// }}}

void dm_set_sufentry(u_int * sufentry, u_int pos, u_int sepos)
{
    assert(sufentry);
    assert(pos < sufentry[0]);
    sufentry[2 + pos] = sepos;
}

u_int dm_get_backentry(u_int * sufentry)
{
    assert(sufentry);
    return sufentry[1];
}

void dm_set_hashsize(u_int * sufentry, u_int hashsize)
{
    assert(sufentry);
    sufentry[0] = hashsize;
    return ;
}

void dm_set_backentry(u_int * sufentry, u_int backentrypos)
{
    assert(sufentry);
    sufentry[1] = backentrypos;
    return;
}

// {{{ scw_seek_entry
/*========================================================================================
 * function : get the dict entry by value and prefix dict entry
 * argu     : pwdict, the worddict
 *          : lde, the last dict entry
 *          : vale, the value
 * return   : DENTRY_NULL if not in
 *          : the dict entry position if IN
 ========================================================================================*/
u_int dm_seek_entry(dm_dict_t * pdict, u_int lde, u_int value)
{
    //assert(pdict);

    u_int sufpos;
    u_int nde;
    u_int hsize;
    u_int hpos;

    if (lde == DM_DENTRY_FIRST)
    {
        sufpos = pdict->entrance;
    }
    else
    {
        sufpos = pdict->dentry[lde].suffix_pos;
    }
    if (sufpos == DM_SUFENTRY_NULL)
    {
        return DM_DENTRY_NULL;
    }
    hsize = dm_get_hashsize(pdict->seinfo + sufpos);
    hpos = value % hsize;
    if (((nde = dm_get_sufentry(pdict->seinfo + sufpos, hpos)) == DM_DENTRY_NULL)
        || (pdict->dentry[nde].value != value))
    {
        return DM_DENTRY_NULL;
    }
    else
    {
        return nde;
    }
}
// }}}

// {{{ append_lemma
/*========================================================================================
 * function : append a lemma to internal segged result
 * argu     : pout, the out struct
 *          : pwdict, the worddict
 *          : plemma, the lemma
 * return   : 1 if success
 *          : <0 if failed
 ========================================================================================*/
int dm_append_lemma(dm_pack_t* ppack, dm_lemma_t* plm, u_int off)
{
    //assert(ppack);
    //assert(plm);

    //const char* where = "dm_append_lemma()";

    if (ppack->ppseg_cnt + 1 > ppack->maxterm)
    {
        //ul_writelog(UL_LOG_FATAL, "error in %s", where);
        return -1;
    }

    ppack->ppseg[ppack->ppseg_cnt] = plm;
    ppack->poff[ppack->ppseg_cnt] = off;
    ppack->ppseg_cnt++;

    return 0;
}
// }}}

// {{{ append_lemma
/*========================================================================================
 * function : append a lemma to internal segged result
 * argu     : pout, the out struct
 *          : pwdict, the worddict
 *          : plemma, the lemma
 * return   : 1 if success
 *          : <0 if failed
 ========================================================================================*/
int dm_append_lemma(dm_pack_t* ppack, dm_lemma_t* plm, u_int off, dm_charset_t charset)
{
    return dm_append_lemma(ppack, plm, off);
}
// }}}

// {{{ dm_has_word
int dm_has_word(
    dm_dict_t* pdict,
    int type,
    const char* inbuf,
    int inlen,
    int opertype)
{
    return dm_has_word(pdict, type, inbuf, inlen, opertype, DM_CHARSET_GB18030);
}
// }}}

// {{{ dm_has_word
int dm_has_word(
    dm_dict_t* pdict,
//    int type,
    uint64_t type,
	const char* inbuf,
    int inlen,
    int opertype,
    dm_charset_t charset)
{
    if (!pdict)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'pdict' cannot be null");
        return 0;
    }

    if (!inbuf)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'inbuf' cannot be null");
        return 0;
    }

    if (opertype != DM_OUT_ALL && opertype != DM_OUT_FMM)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'opertype' should be "
                    "DM_OUT_ALL(%d) or DM_OUT_FMM(%d). the actual value is %d",
                    DM_OUT_ALL, DM_OUT_FMM, opertype);
                    */
        return 0;
    }

    if (charset < 0 || charset >= DM_CHARSET_NUM)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'charset' should be between %d and %d. "
                    "the actual value is %d", 0, DM_CHARSET_NUM - 1, charset);
                    */
        return 0;
    }

    int pos = 0;
    int nextpos = pos;
    u_int value = 0;
    u_int nde = 0;
    u_int lde = DM_DENTRY_FIRST;
    u_int lmpos = DM_LEMMA_NULL;
    u_int slemma = DM_LEMMA_NULL;
    dm_lemma_t * plm = NULL;

    if (opertype == DM_OUT_FMM)
    {
        while (pos < inlen)
        {
            value = dm_get_word(inbuf, pos, inlen, charset);
            if (value == 0)
            {
                break;
            }
            nextpos = pos;

            while (value != 0 && (nde = dm_seek_entry(pdict, lde, value)) != DM_DENTRY_NULL)
            {
                if ((lmpos = pdict->dentry[nde].lemma_pos) != DM_LEMMA_NULL)
                {
                    slemma = lmpos;
                    nextpos = pos;
                }

                if (pos < inlen)
                {
                    value = dm_get_word(inbuf, pos, inlen, charset);
                }
                else
                {
                    value = 0;
                }
                lde = nde;
            }

            if (slemma != DM_LEMMA_NULL)
            {
                plm = pdict->lmlist + slemma;
                // if ((plm->prop & type) == (u_int) type)
                if ((plm->prop & type) == type)
                {
                    return 1;
                }
            }

            lde    = DM_DENTRY_FIRST;
            slemma = DM_LEMMA_NULL;
            pos    = nextpos;
        }
    }
    else if (opertype == DM_OUT_ALL)
    {
        while (pos < inlen)
        {
            value = dm_get_word(inbuf, pos, inlen, charset);
            if (value == 0)
            {
                break;
            }
            nextpos = pos;

            while (value != 0 && (nde = dm_seek_entry(pdict, lde, value)) != DM_DENTRY_NULL)
            {
                if ((lmpos = pdict->dentry[nde].lemma_pos) != DM_LEMMA_NULL)
                {
                    plm = pdict->lmlist + lmpos;
                    // if ((plm->prop & type) == (u_int)type)
                    if ((plm->prop & type) == type)
                    {
                        return 1;
                    }
                }

                if (pos < inlen)
                {
                    value = dm_get_word(inbuf, pos, inlen, charset);
                }
                else
                {
                    value = 0;
                }
                lde = nde;
            }

            lde = DM_DENTRY_FIRST;
            pos = nextpos;
        }
    }

    return 0;
}
// }}}

// {{{
u_int dm_search_prop(
    dm_dict_t* pdict,
    const char* inbuf,
    int inlen,
    int opertype)
{
    return dm_search_prop(pdict, inbuf, inlen, opertype, DM_CHARSET_GB18030);
}
// }}

//{{
u_int dm_search_prop(
    dm_dict_t* pdict,
    const char* inbuf,
    int inlen,
    int opertype,
    dm_charset_t charset)
{
    if (!pdict)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'pdict' cannot be null");
        return 0;
    }

    if (!inbuf)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'inbuf' cannot be null");
        return 0;
    }

    if (opertype != DM_OUT_ALL && opertype != DM_OUT_FMM)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'opertype' should be "
                    "DM_OUT_ALL(%d) or DM_OUT_FMM(%d). the actual value is %d",
                    DM_OUT_ALL, DM_OUT_FMM, opertype);
                    */
        return 0;
    }

    if (charset < 0 || charset >= DM_CHARSET_NUM)
    {
        /*
        ul_writelog(UL_LOG_FATAL,
                    "argument out of range exception: 'charset' should be between %d and %d. "
                    "the actual value is %d", 0, DM_CHARSET_NUM - 1, charset);
                    */
        return 0;
    }

    int pos = 0;
    int nextpos = pos;
    u_int value = 0;
    u_int nde = 0;
    u_int lde = DM_DENTRY_FIRST;
    u_int lmpos = DM_LEMMA_NULL;
    u_int slemma = DM_LEMMA_NULL;
    dm_lemma_t * plm = NULL;
//    u_int prop = 0;
    uint64_t prop = 0;

    if (opertype == DM_OUT_FMM)
    {
        while (pos < inlen)
        {
            value = dm_get_word(inbuf, pos, inlen, charset);
            if (value == 0)
            {
                break;
            }
            nextpos = pos;

            while (value != 0 && (nde = dm_seek_entry(pdict, lde, value)) != DM_DENTRY_NULL)
            {
                if ((lmpos = pdict->dentry[nde].lemma_pos) != DM_LEMMA_NULL)
                {
                    slemma = lmpos;
                    nextpos = pos;
                }

                if (pos < inlen)
                {
                    value = dm_get_word(inbuf, pos, inlen, charset);
                }
                else
                {
                    value = 0;
                }
                lde = nde;
            }

            if (slemma != DM_LEMMA_NULL)
            {
                plm = pdict->lmlist + slemma;
                prop |= plm->prop;
            }

            lde    = DM_DENTRY_FIRST;
            slemma = DM_LEMMA_NULL;
            pos = nextpos;
        }
    }
    else if (opertype == DM_OUT_ALL)
    {
        while (pos < inlen)
        {
            value = dm_get_word(inbuf, pos, inlen, charset);
            if (value == 0)
            {
                break;
            }
            nextpos = pos;

            while (value != 0 && (nde = dm_seek_entry(pdict, lde, value)) != DM_DENTRY_NULL)
            {
                if ((lmpos = pdict->dentry[nde].lemma_pos) != DM_LEMMA_NULL)
                {
                    plm = pdict->lmlist + lmpos;
                    prop |= plm->prop;
                }

                if (pos < inlen)
                {
                    value = dm_get_word(inbuf, pos, inlen, charset);
                }
                else
                {
                    value = 0;
                }
                lde = nde;
            }

            lde = DM_DENTRY_FIRST;
            pos = nextpos;
        }
    }

    return prop;
}
// }}}

// {{{
//int dmpack_has_type(dm_pack_t* ppack, int type)
int dmpack_has_type(dm_pack_t* ppack, uint64_t type)
{
    dm_lemma_t**    plmlist = NULL;
    dm_lemma_t*     plm = NULL;
    int lmcnt = 0;

    if (!ppack)
    {
        //ul_writelog(UL_LOG_FATAL, "argument null exception: 'ppack' cannot be null");
        return 0;
    }

    plmlist = ppack->ppseg;
    lmcnt    = ppack->ppseg_cnt;

    for (int i = 0; i < lmcnt; i++)
    {
        plm = plmlist[i];
        if ((plm->prop & type) == (uint64_t)type)
        {
            return 1;
        }
    }

    return 0;
}
// }}}

// {{{
// int dmpack_has_type(dm_pack_t* ppack, int type, dm_charset_t charset)
int dmpack_has_type(dm_pack_t* ppack, uint64_t type, dm_charset_t charset)
{
    return dmpack_has_type(ppack, type);
}
// }}}

//--globals initer--
static bool _init()
{
    _g_dm_get_word_funcs[DM_CHARSET_GB18030] = dm_get_word_gb18030;
    _g_dm_get_word_funcs[DM_CHARSET_UTF8] = dm_get_word_utf8;
    return true;
}

static bool _g_initer = _init();
