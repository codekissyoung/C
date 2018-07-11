#ifndef __UL_UTF8_TO_UNICODE_H__
#define __UL_UTF8_TO_UNICODE_H__

#include <wchar.h>
#include <stdint.h>
//#include "ul_def.h"

typedef unsigned char u_char;
typedef unsigned int u_int;

namespace{

typedef wchar_t uln_wchar_t;        ///< 定义宽字符串类型，存储Unicode UCS4 

/**
* @brief 最大的Unicode字符
*  
*/
const uln_wchar_t ULN_UNICODE_MAX_LEGAL = 0x0010FFFF; 

/**
* @brief UTF8的错误码
*/
enum {
    ULN_UTF8_SUCCESS     =  0,        ///<返回成功  
    ULN_UTF8_ERROR_PARAM = -1,        ///<参数错误
    ULN_UTF8_ERROR_VALUE = -2,        ///<非法字符
    ULN_UTF8_ERROR_BUFF  = -3,        ///<BUFF太小  
    ULN_UTF8_LOAD_FAIL = -4,          ///<载入错误  
    ULN_UTF8_MEMORY_LIMIT = -5,       ///<内存不足
    ULN_UTF8_ERROR_TABLBE = -6        ///<错误的码表  
};

/* Unicode 和 UTF8的对应关系
 *
 * U-00000000 U-0000007F:    0xxxxxxx
 * U-00000080 U-000007FF:    110xxxxx 10xxxxxx
 * U-00000800 U-0000FFFF:    1110xxxx 10xxxxxx 10xxxxxx
 * U-00010000 U-001FFFFF:    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * U-00200000 U-03FFFFFF:    111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * U-04000000 U-7FFFFFFF:    1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 
 */

/**
* @brief UTF8字符开头位置表示的长度的对应表 
*  
*/
const u_char ULN_UTF8_HEADBYTES[256] = { 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x00
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x10
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x20
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x30
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x40
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x50
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x60
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0x70
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0x80
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0x90
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0xA0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0xB0
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, //0xC0
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, //0xD0
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //0xE0
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0  //0xF0
};  

/**
* @brief UTF8字符头部  
*  
*/
const u_char ULN_UTF8_HEADER[] = {
    0x00,           /* unused */
    0x00,           /* 1 byte */
    0xC0,           /* 2 bytes */
    0xE0,           /* 3 bytes */
    0xF0,           /* 4 bytes */
    0xF8,           /* 5 bytes */
    0xFC,           /* 6 bytes */
};

/**
* @brief 计算以utf8_char开头的UTF8字符长度
* @param [in] char utf8_char 传入一个字符
* @note 返回一个utf8_char开头的UTF8字符长度
*/
inline u_int uln_utf8_count_trail_bytes(const char utf8_char)
{
    return ULN_UTF8_HEADBYTES[(u_char)(utf8_char)];
}

/**
* @brief 满足exp条件直接return val
*/
#define ULN_UTF8_RETURN_VAL_IF(exp, val) do {if (exp) {return val;} }while(0);

/**
* @brief 判断ch是否不是UTF8的头位置
*
* 这是ULN_UTF8_CHAR_CHECK的辅助函数
* 
* @param [in] ch : 需要检测的字符
* @param [in] count : 如果ch是UTF8的头位置，count为0
*  
*/

#define ULN_UTF8_NOHEADER(utf8_char) ((((u_char)(utf8_char))&0xC0) == 0x80)

#define ULN_UTF8_CHAR_CHECK_BREK(ch, count) \
    if (!(ULN_UTF8_NOHEADER(ch))) { \
        count = 0; \
        break; \
    }

//相比较上面一个宏,还检查了是否落在合法的unicode范围内...
#define ULN_UTF8_CHAR_CHECK_STRICT(src, count) \
do {\
    count = uln_utf8_count_trail_bytes(*src); \
    switch (count) { \
        case 6: ULN_UTF8_CHAR_CHECK_BREK(src[count-5], count); \
        case 5: ULN_UTF8_CHAR_CHECK_BREK(src[count-4], count); \
        case 4: ULN_UTF8_CHAR_CHECK_BREK(src[count-3], count); \
        case 3: ULN_UTF8_CHAR_CHECK_BREK(src[count-2], count); \
        case 2: ULN_UTF8_CHAR_CHECK_BREK(src[count-1], count); \
        case 1: break; \
        default : count = 0; \
    } \
    if(count!=0){\
        __typeof__(src) save=src;\
        uln_wchar_t uch = (uln_wchar_t)((u_char)(*src++) & ~ULN_UTF8_HEADER[count]);\
        switch (count) {\
        case 6: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
            uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
        case 5: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
            uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
	    case 4: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
            uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
	    case 3: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
            uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
	    case 2: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
            uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
        }\
        if (uch > ULN_UNICODE_MAX_LEGAL) {\
            count = 0;\
        }\
        src=save;\
    }\
} while (0)


/**
* @brief 处理UTF8的DOM问题
*
* 如果存在DOM则直接移动
* 
* @param [in/out] src : 需要处理的字符串
*  
*/
#define ULN_UTF8_CHECK_DOM(src) \
do { \
  if  (0xEF == (u_char)(src[0]) && 0xBB == (u_char)(src[1]) && 0xBF == (u_char)(src[2])) { \
    src += 3; \
  } \
}while (0)

/**
 * @brief utf8字符串后移一个UTF8字符
 *
 * 本函数会做UTF8安全检查，如果移动的位置正好是'\0'则停止移动
 *
 * @param [in] utf8_str   : 以'\0'结尾的UTF8字符串
 * @param [in] n   : 后移长度
 * @return 　非NULL表示 移动后的UTF8字符位置,如果到结尾位置会停在最后'\0'位置，
 * 　　　　　NULL表示输入的UTF8字符串非法
**/
char *uln_utf8_next_strict(const char* utf8_str)
{
    ULN_UTF8_RETURN_VAL_IF(NULL == utf8_str, NULL);
    ULN_UTF8_RETURN_VAL_IF('\0' == *utf8_str, (char*)utf8_str);
    char* src = (char *)utf8_str;
    int count = 0;
    ULN_UTF8_CHECK_DOM(src);
    ULN_UTF8_CHAR_CHECK_STRICT(src, count);
    ULN_UTF8_RETURN_VAL_IF(0 == count, NULL);
    src += count;
    return src;
}

/**
 * @brief 将一个已知长度为count的，UTF8字符串转化为一个Unicode 字符 
 *
 * @param [in] src : 需要转换的UTF8字符串
 * @param [in/out] count : 需要转换的UTF8字符串长度，如果转换失败被置为0
 * @param [out] uch :  转化的Unicode
 *
 * @note 如果uch超过ULN_UNICODE_MAX_LEGAL count会被置0
**/

#define ULN_UTF8_COUNT_CHAR_TO_UNICODE(src, count, uch) \
 do {\
    uch = (uln_wchar_t)((u_char)(*src++) & ~ULN_UTF8_HEADER[count]);\
    switch (count) {\
        case 6: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
                uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
	    case 5: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
                uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
	    case 4: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
                uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
	    case 3: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
                uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
	    case 2: ULN_UTF8_CHAR_CHECK_BREK(*src, count);\
                uch = (uch << 6)|(((u_char)(*src++))&0x3F);\
    }\
    if (uch > ULN_UNICODE_MAX_LEGAL) {\
        count = 0;\
    }\
} while(0)


/**
 * @brief 把一个UTF8字符转化为一个Unicode字符
 *
 * @param [in] utf8_str   : 需要处理的UTF8字符
 * @return 转化的Unicode 字符，　如果是非法的UTF8字符返回0xFFFD 
 *
 * @note 由于目前合法的Unicode范围在[0, 0x10FFFFF], 如果转化过程中发现Unicde超过这个范围
 *       会直接返回0xFFFD
**/

uln_wchar_t uln_utf8_char_to_unicode(const char *utf8_str)
{
    ULN_UTF8_RETURN_VAL_IF(NULL == utf8_str, ULN_UTF8_ERROR_PARAM);
    uln_wchar_t uch;
    int count = uln_utf8_count_trail_bytes(*utf8_str);
    ULN_UTF8_COUNT_CHAR_TO_UNICODE(utf8_str, count, uch);
    ULN_UTF8_RETURN_VAL_IF(0==count, ULN_UTF8_ERROR_VALUE);
    return uch;
}

}//namespace

#endif //__UL_UTF8_TO_UNICODE_H__

