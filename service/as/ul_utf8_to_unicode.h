#ifndef __UL_UTF8_TO_UNICODE_H__
#define __UL_UTF8_TO_UNICODE_H__

#include <wchar.h>
#include <stdint.h>
//#include "ul_def.h"

typedef unsigned char u_char;
typedef unsigned int u_int;

namespace{

typedef wchar_t uln_wchar_t;        ///< ������ַ������ͣ��洢Unicode UCS4 

/**
* @brief ����Unicode�ַ�
*  
*/
const uln_wchar_t ULN_UNICODE_MAX_LEGAL = 0x0010FFFF; 

/**
* @brief UTF8�Ĵ�����
*/
enum {
    ULN_UTF8_SUCCESS     =  0,        ///<���سɹ�  
    ULN_UTF8_ERROR_PARAM = -1,        ///<��������
    ULN_UTF8_ERROR_VALUE = -2,        ///<�Ƿ��ַ�
    ULN_UTF8_ERROR_BUFF  = -3,        ///<BUFF̫С  
    ULN_UTF8_LOAD_FAIL = -4,          ///<�������  
    ULN_UTF8_MEMORY_LIMIT = -5,       ///<�ڴ治��
    ULN_UTF8_ERROR_TABLBE = -6        ///<��������  
};

/* Unicode �� UTF8�Ķ�Ӧ��ϵ
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
* @brief UTF8�ַ���ͷλ�ñ�ʾ�ĳ��ȵĶ�Ӧ�� 
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
* @brief UTF8�ַ�ͷ��  
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
* @brief ������utf8_char��ͷ��UTF8�ַ�����
* @param [in] char utf8_char ����һ���ַ�
* @note ����һ��utf8_char��ͷ��UTF8�ַ�����
*/
inline u_int uln_utf8_count_trail_bytes(const char utf8_char)
{
    return ULN_UTF8_HEADBYTES[(u_char)(utf8_char)];
}

/**
* @brief ����exp����ֱ��return val
*/
#define ULN_UTF8_RETURN_VAL_IF(exp, val) do {if (exp) {return val;} }while(0);

/**
* @brief �ж�ch�Ƿ���UTF8��ͷλ��
*
* ����ULN_UTF8_CHAR_CHECK�ĸ�������
* 
* @param [in] ch : ��Ҫ�����ַ�
* @param [in] count : ���ch��UTF8��ͷλ�ã�countΪ0
*  
*/

#define ULN_UTF8_NOHEADER(utf8_char) ((((u_char)(utf8_char))&0xC0) == 0x80)

#define ULN_UTF8_CHAR_CHECK_BREK(ch, count) \
    if (!(ULN_UTF8_NOHEADER(ch))) { \
        count = 0; \
        break; \
    }

//��Ƚ�����һ����,��������Ƿ����ںϷ���unicode��Χ��...
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
* @brief ����UTF8��DOM����
*
* �������DOM��ֱ���ƶ�
* 
* @param [in/out] src : ��Ҫ������ַ���
*  
*/
#define ULN_UTF8_CHECK_DOM(src) \
do { \
  if  (0xEF == (u_char)(src[0]) && 0xBB == (u_char)(src[1]) && 0xBF == (u_char)(src[2])) { \
    src += 3; \
  } \
}while (0)

/**
 * @brief utf8�ַ�������һ��UTF8�ַ�
 *
 * ����������UTF8��ȫ��飬����ƶ���λ��������'\0'��ֹͣ�ƶ�
 *
 * @param [in] utf8_str   : ��'\0'��β��UTF8�ַ���
 * @param [in] n   : ���Ƴ���
 * @return ����NULL��ʾ �ƶ����UTF8�ַ�λ��,�������βλ�û�ͣ�����'\0'λ�ã�
 * ����������NULL��ʾ�����UTF8�ַ����Ƿ�
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
 * @brief ��һ����֪����Ϊcount�ģ�UTF8�ַ���ת��Ϊһ��Unicode �ַ� 
 *
 * @param [in] src : ��Ҫת����UTF8�ַ���
 * @param [in/out] count : ��Ҫת����UTF8�ַ������ȣ����ת��ʧ�ܱ���Ϊ0
 * @param [out] uch :  ת����Unicode
 *
 * @note ���uch����ULN_UNICODE_MAX_LEGAL count�ᱻ��0
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
 * @brief ��һ��UTF8�ַ�ת��Ϊһ��Unicode�ַ�
 *
 * @param [in] utf8_str   : ��Ҫ�����UTF8�ַ�
 * @return ת����Unicode �ַ���������ǷǷ���UTF8�ַ�����0xFFFD 
 *
 * @note ����Ŀǰ�Ϸ���Unicode��Χ��[0, 0x10FFFFF], ���ת�������з���Unicde���������Χ
 *       ��ֱ�ӷ���0xFFFD
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

