#include <stdio.h>

#include "StringUtil.h"

bool StringUtil::IsNumber(char c)
{
    if ( (c >= '0' && c <= '9') || c=='.')
        return true;
    
	return false;
}

bool StringUtil::IsNumberExact(char c)
{
	if ( c >= '0' && c <= '9' )
		return true ;
	
	return false ;
}
bool StringUtil::IsWhite(char c)
{
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        return true;
    else
        return false;
}

bool StringUtil::IsAlpha(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return true;
    else
        return false;
}

bool StringUtil::IsAlphaNumber(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
        return true;
    else
        return false;
}

bool StringUtil::IsSeparator(char c)
{
    std::string strSeparator(" \t\r\n-_[](){}:.,=*&^%$#@!~?<>/|'\"");

    if (std::string::npos == strSeparator.find(c))
        return false;
    
	return true;
}

bool StringUtil::IsNumber(const char* pStr)
{
	if ( NULL == pStr )
		return false ;

	char* p = (char*) pStr;
    while (*p != '\0')
	{
        if( !IsNumber(*p))
            return false;
		p++;	
	}

    return true;  
}

bool StringUtil::IsWhite(char const *pStr)
{
	if ( NULL == pStr )
		return false ;

    while( (*pStr == ' ') || (*pStr == '\t') || (*pStr == '\n') )
        pStr ++;

    if(*pStr == '\0')
        return true;
    else
        return false;
}

bool StringUtil::IsEnglish(const char* pStr, bool bLowLineAsAlpha)
{
	if ( NULL == pStr )
		return false ;

	char* p = (char*) pStr;

    while ( *p != '\0' )
	{
        if (bLowLineAsAlpha)
        {
            if( !ISALPHA(*p) && ('_' != *p) )
                return false;
        }
        else
        {
            if( !IsAlpha(*p) )
                return false;
        }
		p++;	
	}

    return true;  
}

bool StringUtil::IsValidUrl(char* pUrl)
{
	if ( NULL == pUrl )
		return false;

	char* p = pUrl;
	while (*p != '\0')
	{
		if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
			return false;
		else if ((*p == '<' || *p == '>') && (*(p-1) != '\\'))
			return false;
		else if (*p == '*' || *p == '|')
			return false;
		else if (*p == '/' && (STRNCASECMP(p+1, "Javascript:", 11) == 0))
			return false;
		else if ((*p == 'j' || *p == 'J') && (STRNCASECMP(p+1, "avascript:", 10) == 0))
			return false;

		p++;
	}
	return true;
}

bool StringUtil::IsNumericIP(std::string strIp)
{
	std::vector<std::string> strList;
	int dwTmp;
		
	SplitString(strIp, ".", strList);
    if (4 != strList.size())
    	return false;
    
    for(int i = 0; i < 4; i++)
    {
    	if (strList[i].size() > 3 || strList[i].size() == 0 )
    		return false;
    	
		for (size_t j = 0; j < strList[i].size(); j++)
		{
			if (!(strList[i][j] >= '0' && strList[i][j] <= '9'))
				return false;
    	}
    	dwTmp = StrToInt32(strList[i]);
    	if (dwTmp > 255)
    		return false;
    }
	return true;	
}

/// system functions have bugs: tolower(), toupper().

char StringUtil::CharToLower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + 32;
	else
		return c;
}

char StringUtil::CharToUpper(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - 32;
	else
		return c;
}

std::string StringUtil::ToLower(const char *pszValue)
{
    std::string  strRet (pszValue) ;

	size_t len = strRet.size() ;
	for (size_t i=0; i<len; i++ )
	{
		strRet[i] = CharToLower(strRet[i]) ;
	}

	return strRet;
}

/*
 *   It is illegal to assign a string in such a manner as :
 *   
 *         std::string value ;
 *         value[0] = 'a' ;
 *         value[1] = 'b' ;
 *
 *   after assignment, the variable string remains to be 
 *   empty.
 */

std::string StringUtil::ToUpper(const char *pszValue)
{
    std::string  strRet (pszValue) ;

	size_t len = strRet.size() ;
	for (size_t i=0; i<len; i++ )
	{
		strRet[i] = CharToUpper(strRet[i]) ;
	}

	/*
    std::string strRet ;
	const char * p = pszValue ;
	int idx = 0 ;
	while ( *p != '\0' )
	{
		strRet[idx] = CharToUpper( *p ) ;
		p ++ ;
		idx ++ ;
	}
	*/
    return strRet;
}

std::string StringUtil::Int32ToStr(int32_t dwValue)
{
    char szBuf[16];
    sprintf(szBuf,"%d",dwValue);
    return std::string(szBuf);
}

std::string StringUtil::Uint32ToStr(uint32_t  dwValue)
{
    char szBuf[16];
    sprintf(szBuf,"%u",dwValue);
    return std::string(szBuf);
}

std::string StringUtil::Int64ToStr(int64_t ddwValue)
{
    char szResult[64];
    szResult[0] = '\0';

#ifdef _WIN32
	/*
    _i64toa(ddwValue, szResult, 10);
	*/
	sprintf(szResult, "%I64d", ddwValue) ;
#else
    sprintf (szResult, "%lld", ddwValue);
#endif
    return std::string(szResult);
}

std::string StringUtil::Uint64ToStr(uint64_t ddwValue)
{
    char szResult[64];
    szResult[0] = '\0';

#ifdef _WIN32
	sprintf(szResult, "%I64u", ddwValue) ;
#else
    sprintf (szResult, "%llu", ddwValue);
#endif
    return std::string(szResult);
}

int32_t StringUtil::StrToInt32(const std::string& _str)
{
	const char* str = _str.c_str();
	bool isNegative = false;
	int32_t x = 0;
	
	while ( *str != '\0' )
	{
		if ( (!IsNumber(*str)) && (*str != '-') )
			str ++ ;
		else
			break ;
	}

	if( *str == '-' )
	{
		isNegative = true;
		++str;
	}
		
	do
	{
		const int32_t c = *str - '0';
		if( c < 0 || 9 < c )
			break ;

		x = 10 * x + c;

	} while (*++str);
		
	if( isNegative )
		x = -x;
	
	return x; 
}

uint64_t StringUtil::StrToUint64(const std::string& str)
{
	uint64_t dwRet = 0;
#ifdef _WIN32
	sscanf(str.c_str(), "%I64u", &dwRet);     
#else
	sscanf(str.c_str(), "%Lu", &dwRet);     
#endif
	return dwRet;
}

double StringUtil::Str2Double(const std::string& str)
{
	/*
	 * atof() performs the same on Windows and Linux.
	 */

	return atof(str.c_str());
}

std::string StringUtil::Double2Str(double lfValue)
{
    char szBuf[80];
    sprintf(szBuf, "%lf", lfValue);

    return std::string(szBuf);
}

/// It works well only when the integer is positive.

int32_t StringUtil::HexStr2Int(const char* pcHex)
{
	if ( pcHex == NULL )
		return 0 ;

	int32_t dwRet = 0;
	int32_t dwTmp;

	const char * p = pcHex ;
	while ( *p != '\0' )
	{
		if ( *p >= 'A' && *p <= 'F' )
			dwTmp = *p - 'A' + 10;
		else if ( *p >= 'a' && *p <= 'f')
			dwTmp = *p - 'a' + 10;
		else if ( IsNumber(*p) )
			dwTmp = *p - '0';
		else
			break ;
		
		dwRet = dwRet*16 + dwTmp;

		p ++ ;
	}
	return dwRet;
}

bool StringUtil::IsHexDigit(const char* pStr)
{
	const char* p = pStr;
	while (*p != '\0')
	{
		if (!((*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F') || (*p >= '0' && *p <= '9')))
		{
			return false;
		}
		p++;	
	}
	return true;  
}

bool StringUtil::CmpNoCase(char c1, char c2)
{
	return CharToUpper(c1) == CharToUpper(c2) ;
}

void StringUtil::StatStr(std::string& strWord, int32_t & dwAlpha, int32_t & dwDigit)
{
    dwAlpha = 0;
    dwDigit = 0;

    for (size_t i = 0; i < strWord.size(); ++i)
    { 
        if ((strWord[i] >= 'a' && strWord[i] <= 'z') || (strWord[i] >= 'A' && strWord[i] <= 'Z'))
            dwAlpha++;
        else if (strWord[i] >= '0' && strWord[i] <= '9')
            dwDigit++;
    }
}
int32_t StringUtil::GetCharCount(std::string& strWord, char c)
{
	int32_t dwCharCount = 0;
	for (size_t i = 0; i < strWord.size(); ++i)
	{
		if (strWord[i] > 0 && strWord[i] == c) 
		{
            ++dwCharCount;
		}
	}
	return dwCharCount;
}


std::string StringUtil::TrimBothSides(const std::string& strValue, const std::string& delimiters)
{
	if (strValue.size() <= 0)
		return "";

	std::string::size_type startPos = strValue.find_first_not_of(delimiters);
	if ( startPos == std::string::npos )
		return "" ;

	std::string::size_type endPos = strValue.find_last_not_of(delimiters);

	return std::string( strValue, startPos, endPos - startPos + 1 );
}

std::string StringUtil::TrimLeft(const std::string& strValue, const std::string& delimiters)
{
	if (strValue.size() <= 0)
		return "";
	
	std::string::size_type startPos = strValue.find_first_not_of(delimiters);
	if ( startPos == std::string::npos )
		return "" ;

	return strValue.substr(startPos) ;
}

std::string StringUtil::TrimRight(const std::string& strValue, const std::string& delimiters)
{
	if (strValue.size() <= 0)
		return "";

	std::string::size_type endPos = strValue.find_last_not_of(delimiters);

	if ( endPos == std::string::npos )
		return "" ;
	return strValue.substr(0, endPos+1) ;
}

/// 功能: 删除在中文字符串两头的中文空格.

std::string StringUtil::TrimChineseSpace(const std::string& strValue)
{
    if (strValue.size() < 2)
        return strValue;

    int dwHead = 0;
    int dwTail = (int)strValue.size() - 2;

    /// 确定字符串前面的中文空格的个数.
	
    while ((dwHead <= (int)strValue.size() - 2) && 
           0xA1 == (unsigned char)strValue[dwHead] && 
           0xA1 == (unsigned char)strValue[dwHead+1])

        dwHead += 2;

    /// 确定字符串前面的中文空格的个数.
	
    while (dwTail >= 0 && 
           0xA1 == (unsigned char)strValue[dwTail] &&
           0xA1 == (unsigned char)strValue[dwTail+1])

        dwTail -= 2;

	if (dwTail < dwHead)
		return "";
	else
	    return strValue.substr(dwHead, dwTail - dwHead + 2);
}

/// 功能: 删除在中英文字符串两头的中英文空格.

std::string StringUtil::TrimBothChEnSpace(const std::string& strValue)
{
	if (strValue.size() < 2)
		return strValue;

	size_t dwHead = 0 ;
	size_t dwTail = strValue.size()-1 ;

	///  确定字符串前面的中文空格的个数.
	while (dwHead <= strValue.size()-2)
	{
		if (strValue[dwHead] == ' ')
		{
			dwHead ++;
		}
		else if (strValue[dwHead] == (char)0xa1 && strValue[dwHead+1] == (char)0xa1)
		{
			dwHead += 2;
		}
		else
		{
			break;
		}
	}
	///  确定字符串后面的中英文空格的个数.
	while (dwTail > 0)
	{
		if (strValue[dwTail] == ' ')
		{
			dwTail --;
		}
		else if (strValue[dwTail-1] == (char)0xa1 && strValue[dwTail] == (char)0xa1)
		{
			dwTail -= 2;
		}
		else
		{
			break;
		}        
	}
	if (dwTail < dwHead)
		return "";

	return strValue.substr(dwHead, dwTail - dwHead + 2);
}

void StringUtil::SplitString(const std::string& strMain, const std::string strSpliter, 
                             std::vector<std::string>& strList )
{
	strList.clear();

	if (strMain.empty() || strSpliter.empty())
		return;
	
	std::string strMainTemp = TrimBothSides(strMain, " \r\n\t");
	
	if (strMainTemp.empty() || strSpliter.empty())
		return;
	
	std::string::size_type nPos = strMainTemp.find(strSpliter);
	std::string strTemp;

    while (std::string::npos != nPos)
	{
        strTemp = strMainTemp.substr(0,nPos);
        if (!strTemp.empty())
            strList.push_back(strTemp);

		strMainTemp = strMainTemp.substr(nPos+strSpliter.size());
		nPos = strMainTemp.find(strSpliter);
	}
	
	strMainTemp = TrimBothSides(strMainTemp, " \r\n\t");

	if(!strMainTemp.empty())
		strList.push_back(strMainTemp);
}

void StringUtil::SplitString(const std::string& strMain, std::vector<std::string>& strList)
{
	strList.clear();
	if (strMain.empty())
		return;

	std::string strTemp = strMain;
	std::string::size_type dwBeginPos;
	std::string::size_type dwEndPos;
	do
	{
        dwBeginPos = 0;
        while(dwBeginPos < strTemp.size() && IsSeparator(strTemp[dwBeginPos]))
            dwBeginPos++;

        dwEndPos = dwBeginPos;
        while(dwEndPos < strTemp.size() && !IsSeparator(strTemp[dwEndPos]))
            dwEndPos++;

        if (dwEndPos > dwBeginPos)
        {
            strList.push_back(strTemp.substr(dwBeginPos, dwEndPos - dwBeginPos));
            strTemp = strTemp.substr(dwEndPos);
        }
        else
        {
            break;
        }
	}
    while ( strTemp.size() > 0);
}

std::string StringUtil::ReplaceSubStr(const std::string& strValue,
                                      const std::string& oldStr, 
                                      const std::string& newStr)
{
	std::string strRet = strValue;

	std::string::size_type dwPos = strRet.find(oldStr);  
    while (std::string::npos != dwPos)
	{
		strRet.replace( dwPos, oldStr.size(), newStr );
		dwPos += newStr.size() ;
		dwPos = strRet.find(oldStr, dwPos) ;
	}
	return strRet;
}

/*
 * strNew = ReplacePairChar(strOld, "<>", "[]");
 */
std::string StringUtil::ReplacePairChar(const std::string& strOld, 
                                        std::string strFromSet, 
                                        std::string strToSet)
{
	std::string strResult;
	for (std::string::size_type i = 0; i < strOld.size(); i++)
	{
		char ch = strOld[i];

		std::string::size_type dwPos = strFromSet.find(ch);
		if (dwPos == std::string::npos)
		{
			strResult += ch;
		}
		else if (dwPos < strToSet.size())
		{
			strResult += strToSet[dwPos];
		}
	}

	return strResult;
}

std::string StringUtil::RemoveSubStr(std::string& strRawData, std::string strSubStr)
{
	std::string::size_type dwPos;
	std::string strTemp = strRawData;
	std::string strRet;
	
	do
	{
		dwPos = strTemp.find(strSubStr);
        if (dwPos == std::string::npos)
		{
			strRet += strTemp;
			break;
		}
        else
		{
			 strRet += strTemp.substr(0, dwPos);
			 strTemp = strTemp.substr(dwPos+strSubStr.size());
		}
	} while (!strTemp.empty());

	return strRet;
}

/// remove the string ranging from strBegin to strEnd. Note that 
/// the delimiter strings, i.e, strBegin and strEnd, would also 
/// be removed. For instance, strBegin denotes "/*" and strEnd 
/// denotes "*/".

std::string StringUtil::RemoveRange(const std::string& strRawData, 
                                    std::string strBegin,
                                    std::string strEnd)
{
	std::string::size_type dwPosBegin;
	std::string::size_type dwPosEnd;
	std::string strTemp = strRawData;
	std::string strRet;
	
	do
	{
		dwPosBegin = strTemp.find(strBegin);
		dwPosEnd = strTemp.find(strEnd);

        if ((dwPosBegin != std::string::npos) && (dwPosEnd != std::string::npos))
		{
            if (dwPosEnd > dwPosBegin)
            {
                if (dwPosBegin > 0)
                    strRet += strTemp.substr(0, dwPosBegin);
                if (dwPosEnd+strEnd.size() < strTemp.size())
                {
                    size_t dwNewLen = strTemp.size()-dwPosEnd-strEnd.size();
                    strTemp = strTemp.substr(dwPosEnd+strEnd.size(), dwNewLen);
                }
                else
                    strTemp = "";
                continue;
            }
            else
            {
                strRet += strTemp.substr(0, dwPosBegin);
                strTemp = strTemp.substr(dwPosBegin);
            }
		}
        else
        {
            strRet += strTemp;
            break;
        }
	}	while (!strTemp.empty());


	return strRet;
}

std::string StringUtil::EscapeWild(const char* strRaw)
{
	if (NULL == strRaw)
		return "";

	const char * p = strRaw ;
	int32_t strLen = 0 ;
	while ( *p != '\0' )
	{
		strLen ++ ;
		p ++ ;
	}

	char* pcDest = new char[ strLen * 2 + 1 ];

	p = strRaw ;
	int j = 0;

	while ( *p != '\0' ) 
	{
		switch( *p ) 
		{	case '\n':
				pcDest[j++] = '\\';
				pcDest[j++] = 'n';
				break;
			case '\t':
				pcDest[j++] = '\\';
				pcDest[j++] = 't';
				break;
			case '\r':
				pcDest[j++] = '\\';
				pcDest[j++] = 'r';
				break;
			case '\b':
				pcDest[j++] = '\\';
				pcDest[j++] = 'b';
				break;
			case '\'':
				pcDest[j++] = '\\';
				pcDest[j++] = '\'';
				break;
			case '\"':
				pcDest[j++] = '\\';
				pcDest[j++] = '\"';
				break;
			case '\\':
				pcDest[j++] = '\\';
				pcDest[j++] = '\\';
				break;
			default:
				if ((unsigned char) (*p) >= 32) 
					pcDest[j++] = *p ;
				else 
					pcDest[j++] = 32 ;  /* 小于32是控制字符，都转换为空格(32). */
		}

		p ++;
	}
	pcDest[j] = '\0';

	std::string strRet;
	strRet.assign(pcDest);
	delete [] pcDest;
	
	return strRet;
}

/// convert several continuous white characters to only one.

std::string StringUtil::SingleWhite(const std::string & strRaw) 
{
	unsigned int dwCount = 0;
	bool bIsFirstSpace = true;
	const char *ptr = strRaw.c_str();

	std::string strRet(strRaw.length(), ' ');

	/// skip the heading white characters.
	
	while ((*ptr > 0) && ISWHITE(*ptr)) 
        ++ptr;
	
	while (*ptr)
	{
		if ((*ptr > 0) && ISWHITE(*ptr))
		{
			/// the first of a chain of white characters.
			if (bIsFirstSpace)
			{
				bIsFirstSpace = false;
				strRet[dwCount++] = ' ';
			}
		}
		else
		{
			bIsFirstSpace = true;
			strRet[dwCount++] = *ptr;
		}
		
		++ptr;
	}

	// remove the ending space characters.

	std::string::size_type dwPos;
	dwPos = strRet.find_last_not_of(' ', dwCount);
	if (dwPos != std::string::npos)
		strRet.erase(dwPos+1);
	else
	{
		dwPos = 0;
		strRet.erase(dwPos);
	}

	return strRet;
}

/// remove the contents between "<!--" and "-->".

std::string StringUtil::StripComments(const std::string& strRawFile) 
{
	std::string strNewFile;
	strNewFile.reserve(strRawFile.size());

	const char *ptr = strRawFile.c_str();
	const char *end = ptr + strRawFile.length();

	bool bIsInsideComment = false;
	while(1) 
	{
		if(!bIsInsideComment) 
		{
			if(ptr  + 3 < end) 
			{
				if(*ptr == '<' && *(ptr+1) == '!' && *(ptr+2) =='-' && *(ptr + 3) == '-' ) 
				{
					bIsInsideComment = true;
				}
			}
		} 
		else 
		{
			if(ptr + 2 < end) 
			{
				if(*ptr == '-' && *(ptr+1) == '-' && *(ptr+2) == '>' ) 
				{
					bIsInsideComment = false;
					ptr += 3;
				}
			}
		}
		if(ptr == end) 
			break;
		if(!bIsInsideComment) 
			strNewFile += *ptr;
		ptr++;
	}

	strNewFile.resize(strNewFile.size());

	return strNewFile;
}

bool StringUtil::IsNotSingleChar(const char * pC)
{
	if ( pC == NULL )
		return false ;

	char first = *pC ;
	char second = *(pC+1) ;
	
	return ( first < 0 && second != 0 ) ;
}

/****************************************************/
/*        GBK is a super set of GB2312.             */
/*                                                  */
/*  http://www.knowsky.com/resource/gb2312tbl.htm   */
/****************************************************/

/// the codes of any GBK Character lie in set [8140-FEFE].
bool StringUtil::IsGBK(const char *pC)
{
	unsigned char first = (unsigned char) (*pC) ;
	unsigned char second = (unsigned char) (*(pC+1)) ;

	return ((first >= 0x81) && (second >= 0x40) && (first <= 0xFE) && (second <= 0xFE));
}

/// Any GBK Chinese Character lies in ( [B0A1-F7FE], [8140-A0FE], [AA40-FEA0] ).

bool StringUtil::IsChinese(const char *pC)
{
	unsigned char first = (unsigned char) (*pC) ;
	unsigned char second = (unsigned char) (*(pC+1)) ;

	/// 1. GB2312 Chinese
	/// 2. CJK Chinese
	/// 3. other extented Chinese.
	
	return (((first >= 0xB0) && (second >= 0xA1) && (first <= 0xF7) && (second <= 0xFE)) ||
            ((first >= 0x81) && (second >= 0x40) && (first <= 0xA0) && (second <= 0xFE)) ||
			((first >= 0xAA) && (second >= 0x40) && (first <= 0xFE) && (second <= 0xA0)) );
}


/// [A1A0-A1FF]: 全角中文标点符号的支持位图.

static const unsigned char A1char[100] =
{
		0,0,0,0,1,1,1,0,0,1,
		1,0,0,1,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,0,0,
		1,1,0,0,1,1,1,0,1,1,
		1,1,1,0,0,1,0,1,1,0,
		1,1,0,1,1,1,1,1,1,1,
		0,0,1,1,1,0,0,0,1,0,
		1,0,0,1,1,1,0,1,0,0,
		0,0,0,0,0,0,0,1,0,0,
		0,0,0,0,0,0,0,0,0,0
};

/// GB2312/GBK 汉字保留字符.

bool StringUtil::IsReservedGBK(const char *pC)
{
	unsigned char first = (unsigned char)(*pC);
	unsigned char second = (unsigned char)(*(pC+1));
		
	/// [A1A0-A1FF] 全角中文标点符号, 支持一部分.
	if (first == 0xA1)
	{
		if (A1char[second-160] == 1)
			return true;
	}
	
	/// [A2A0-A2FF] 罗马数字, 全部支持.
	if (first == 0xA2 && second >= 0xA0)
		return true;
	
	/// [A3A0-A3FF] 全角英文, 数字, 符号, 支持￥.
	if ((first == 0xA3) && (second == 0xA4))	/// ￥
		return true;

	/// [A4A0-A5FF] 日本片假名, 全部支持.
	if ((first == 0xA4 || first == 0xA5) && (second >= 0xA0))
		return true;

	/// 中文零〇
	if ((first == 0xA9) && (second == 0x96))
		return true;
		
	return false;
}

bool StringUtil::IsValidGBK (const char * pC)
{
	return ( IsChinese(pC) || IsReservedGBK(pC) ) ;
}

/// 计算 GB2312 汉字的序号, 返回值是0到6767之间的数.

int32_t StringUtil::GetGB2312Index(const char *pC)
{
	unsigned char first = (unsigned char) (*pC) ;
	unsigned char second = (unsigned char) (*(pC)+1) ;
	return ( first - 176) * 94 + second - 161;
}

/// 判断字符串的前两个字节是否是GB2312字符.

bool StringUtil::IsGB2312(const char *pC)
{
	if ( pC == NULL )
		return false ;
	if ( *pC == '\0' || *(pC+1) == '\0' )
		return false ;

	int32_t dwIndex = GetGB2312Index(pC);

	return ( dwIndex >= 0 && dwIndex < 6768) ;
}

/// 判断一个字符串是否完全由 GB2312 的字符组成.

bool StringUtil::IsPure_GB2312_Str(const char *pC)
{
	if ( pC == NULL )
		return false ;

	const char * pivot = pC ;
	unsigned char first, second ;

	while ( *pivot != '\0' )
	{
		if ( IsGB2312(pivot) == false )
			return false ;

		pivot += 2 ;
	}

	return true ;
}

/// 在汉字串中定位寻找某个指定的汉字.

char* StringUtil::LocateInChinese(const char* pcMain, const char* pcWChar)
{
    if( NULL == pcMain || NULL == pcWChar || pcWChar[0] >= 0 )
        return NULL;

    char* pcTempMain = (char *)pcMain;
    char  c1, c2;
    do
    {
        c1 = *pcTempMain;
        if ( c1 < 0 )
        {
            c2 = *(pcTempMain+1);
            if ( '\0' == c2 )
                return NULL ;


            if (c1 != pcWChar[0] || c2 != pcWChar[1])
            {
                pcTempMain += 2;
                continue;
            }
            else
                break;
        }
        else
        {
            pcTempMain++;
            if ( *pcTempMain != '\0' )
                continue;
            else
                return NULL;
        }
    } while( '\0' != *pcTempMain );

    return pcTempMain;
}

/// 获得一个字符串的字符个数, 中文等双字节字符只算1个.

int32_t StringUtil::GetCharacterCnt(const char *pC)
{
	if ( pC == NULL )
		return 0 ;

	int32_t dwCharCount = 0 ;

	const char * pivot = pC ;
	while ( *pivot != '\0' )
	{
		if ( *pivot < 0 )
		{
			pivot ++ ;
			if ( *pivot == '\0' )
				break ;
		}

		dwCharCount ++ ;
		pivot ++ ;
	}

	return dwCharCount;
}


/// 获得一个字符串中中文字符的个数.
int32_t StringUtil::GetChineseCharCnt(const char *pC)
{
	if ( pC == NULL )
		return 0 ;

	int32_t dwCharCount = 0 ;

	const char * pivot = pC ;
	while ( *pivot != '\0' )
	{
		if ( *pivot < 0 )
		{
			pivot ++ ;
			if ( *pivot == '\0' )
				break ;
			
			dwCharCount ++ ;
		}

		pivot ++ ;
	}

	return dwCharCount;
}

/*
 *  simplified regular expressions. 
 *
 *  MatchWildcard("he*o","hello"): true
 *  MatchWildcard("he*p","hello"): false
 *  MatchWildcard("he??o","hello"): true
 *  MatchWildcard("he?o","hello"): false
 *  MatchWildcard("[a-z][0-9]","h7"): true
 *  MatchWildcard("172.16.*","172.16.68.29"): true
 */
bool StringUtil::MatchWildcard(const std::string& strWild, const std::string& strMatch)
{
	return _MatchRemainder(strWild, strWild.begin(), strMatch, strMatch.begin());
}

/*
 * Suppose strPattern is [0-9], the returned value is "true" if strMatch denotes
 * "8", while "false" would be returned if strMatch is "a".
 */

bool StringUtil::_MatchSet(const std::string& strPattern, char strMatch)
{
	/// strPattern: a-z, strRealCharSet: abcd...z

	std::string strRealCharSet; 
	std::string::const_iterator i;
	for (i = strPattern.begin(); i != strPattern.end(); ++i)
	{
		switch(*i)
		{
		case '-':
		{
			if (i == strPattern.begin())
				strRealCharSet += *i;
			else if (i+1 == strPattern.end())
		  		return false;
		    else
			{
				/// the starting character is already in the set, thus we 
				/// erase it.
				strRealCharSet.erase(strRealCharSet.end()-1);
				char last = *++i;
				for (char ch = *(i-2); ch <= last; ch++)
				{
					strRealCharSet += ch;
				}
			}
			break;
		}
		case '\\':
			if (i+1 == strPattern.end()) 
				return false;
			strRealCharSet += *++i;
			break;
		default:
			strRealCharSet += *i;
			break;
		}
	}
	std::string::size_type dwResult = strRealCharSet.find(strMatch);
	return dwResult != std::string::npos ;
}

/// try to match the remainder wildcard string recursively.
bool StringUtil::_MatchRemainder(const std::string& strWild, std::string::const_iterator itWild, 
                                 const std::string& strMatch, std::string::const_iterator itMatch)
{
	while (itWild != strWild.end() && itMatch != strMatch.end())
	{
		switch(*itWild)
		{
		case '*':
		{
			++itWild;
			for (std::string::const_iterator i = itMatch; i != strMatch.end(); ++i)
			{
				/// if character * is the last one of the wildcard string
				
				if (itWild == strWild.end())
				{
					/*
					if (i == strMatch.end()-1)
						return true;
					*/

					return true ;
				}
				else if ( _MatchRemainder(strWild, itWild, strMatch, i) )
				{
					return true;
				}
			}

			return false;
		}
		case '[':
		{
			/// find the character "]"
			bool bFound = false;
			std::string::const_iterator it = itWild + 1;
			for (; !bFound && it != strWild.end(); ++it)
			{
				switch(*it)
				{
				case ']':
				{
					if (! _MatchSet(strWild.substr(itWild - strWild.begin() + 1, it - itWild - 1), *itMatch) )
						return false;
					bFound = true;
					break;
				}
				case '\\':
					// 转义字符不能在结尾.
					if (it == strWild.end()-1)
						return false;
					++it;
					break;
				default:
					break;
				}
			}
			if (!bFound)
				return false;
			++itMatch;
			itWild = it;
			break;
		}
		case '?':
			++itWild;
			++itMatch;
			break;
		case '\\':
			if (itWild == strWild.end()-1)
				return false;
			++itWild;
			if (*itWild != *itMatch)
				return false;
			++itWild;
			++itMatch;
			break;
		default:
			if (*itWild != *itMatch)
				return false;
			++itWild;
			++itMatch;
			break;
		}
	}

	return (itWild == strWild.end()) && (itMatch == strMatch.end());
}


/*
 * Implementation of the class StrTokenizer.
*/

StrTokenizer::StrTokenizer(std::string str, std::string sep)
{
	parse(str, sep);
}

void StrTokenizer::parse(std::string str, std::string sep)
{
	int32_t n = (int32_t) str.length();
	int32_t start, stop;
	start = (int32_t) str.find_first_not_of(sep);
	while (start >= 0 && start < n)
	{
		stop = (int32_t) str.find_first_of(sep, start);
		if (stop < 0 || stop > n)
		{
			stop = n;
		}
		
		tokens.push_back(str.substr(start, stop - start));
		start = (int32_t) str.find_first_not_of(sep, stop + 1);
	}
	
	idx = 0;
}

int32_t StrTokenizer::count_tokens()
{
	return (int32_t) tokens.size();
}

std::string StrTokenizer::next_token()
{
	if (idx >= 0 && idx < (int32_t) tokens.size())
	{
		return tokens[idx++];
	}
	else
	{
		return "";
	}
}

std::string StrTokenizer::token(int32_t i)
{
	if (i >=0 && i < (int32_t) tokens.size())
	{
		return tokens[i];
	}
	else
	{
		return "";
	}
}
