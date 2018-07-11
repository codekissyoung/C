/*
 * @desc: 
 *      string process utilities.
 *
 * @author: Xiaoming Fan  <xmFan1983@gmail.com>
 */

#ifndef     __STRING_UTIL_H__
#define     __STRING_UTIL_H__

#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "inttypes.h"

/// non-case sensitive comparation.
#ifdef WIN32
    #ifndef STRNCASECMP
    #define STRNCASECMP			_strnicmp
    #endif
    #ifndef STRCASECMP
    #define STRCASECMP			_stricmp
    #endif
	#define snprintf            _snprintf 
#else 
    #ifndef STRNCASECMP
    #define STRNCASECMP			strncasecmp
    #endif
    #ifndef STRCASECMP
    #define STRCASECMP			strcasecmp
    #endif
#endif

#ifdef WIN32
    #define END_LINE "\r\n"
#else
    #define END_LINE "\n"
#endif

#ifndef ISALPHA_DIGIT_WHITE

#define ISALPHA_DIGIT_WHITE
#define ISALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define ISDIGIT(c) (c >= '0' && c <= '9')
#define ISWHITE(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')
#endif

class StringUtil
{
private:
	StringUtil() ;
	/// set the constructor to be private, and prohibit the instantiation.
	
public:

	/// Part 1: character content recognition.
    
    static bool IsNumber(char c);

	static bool IsNumberExact(char c) ;

    static bool IsWhite(char c);
    
	static bool IsAlpha(char c);

    static bool IsAlphaNumber(char c);

    static bool IsSeparator(char c);

    static bool IsNumber(const char* pStr);

    static bool IsWhite(char const *pStr);

    static bool IsEnglish(const char* pStr, bool bLowLineAsAlpha = false);
    
	static bool IsValidUrl(char* pUrl);
    
	static bool IsNumericIP(std::string strIp);

	/// Part 2: convertions.

    static char CharToLower(char c);

    static char CharToUpper(char c);

    static std::string ToLower(const char *pszValue);

    static std::string ToUpper(const char *pszValue);

    static std::string Int32ToStr(int32_t dwValue);

    static std::string Uint32ToStr(uint32_t  dwValue);
	
    static std::string Int64ToStr(int64_t ddwValue);

	static std::string Uint64ToStr(uint64_t ddwValue) ;

    static int32_t StrToInt32(const std::string& str);

	static uint64_t StrToUint64(const std::string& str);

    static double Str2Double(const std::string& str);

    static std::string Double2Str(double);

    static int32_t HexStr2Int(const char* pcHex);

	static bool IsHexDigit(const char* pStr);
	
	/// Part 3: Analysis.
	
	static bool CmpNoCase(char c1, char c2);

    static void StatStr(std::string& strWord, int32_t & dwAlpha, int32_t & dwDigit);

    static int32_t GetCharCount(std::string& strWord, char c);

    static bool MatchWildcard(const std::string& strWild, const std::string& strMatch);

	// Part 4: Manipulations.

    static std::string TrimBothSides(const std::string& strValue, 
									 const std::string& delimiters = " \t\r\n");

    static std::string TrimLeft(const std::string& strValue, 
								const std::string& delimiters = " \t\r\n");

    static std::string TrimRight(const std::string& strValue, 
								 const std::string& delimiters = " \t\r\n");

    static std::string TrimChineseSpace(const std::string& strValue);

    static std::string TrimBothChEnSpace(const std::string& strValue);


    static void SplitString(const std::string& strMain, const std::string strSpliter, 
                            std::vector<std::string>& strList );

    static void SplitString(const std::string& strMain, std::vector<std::string>& strList);

    static std::string ReplaceSubStr(const std::string&, const std::string&, const std::string&);

	static std::string ReplacePairChar(const std::string& strOld, std::string strFromSet, std::string strToSet);

    static std::string RemoveSubStr(std::string& strRawData, std::string strSubStr);

	static std::string RemoveRange(const std::string& strRawData, std::string strBegin, std::string strEnd);

	static std::string EscapeWild(const char* strRaw);
   
    static std::string SingleWhite(const std::string& strRaw); 

	static std::string StripComments(const std::string& strRawFile);

	// Part 4: Chinese characters.

	static bool IsNotSingleChar(const char * pC) ;

	static bool IsGBK(const char *pC) ;

	static bool IsChinese(const char *pC) ;

	static bool IsReservedGBK(const char *pC) ;

	static bool IsValidGBK(const char *pC) ;

    static int32_t GetGB2312Index(const char *pC);

    static bool IsGB2312(const char *pC);

    static bool IsPure_GB2312_Str(const char *pC);

    static char* LocateInChinese(const char* pcMain, const char* pcWChar);

    static int32_t GetCharacterCnt(const char *pC);

    static int32_t GetChineseCharCnt(const char *pC);

private:
	/// determine the character lies in the pattern or not. For instance, 
	/// strPattern = "0-9", and the strMatch character denotes "8".
    static bool _MatchSet(const std::string& strPattern, char strMatch);

	/// it works in a recursive way.
    static bool _MatchRemainder(const std::string& strWild, 
							    std::string::const_iterator itWild, 
							    const std::string& strMatch, 
							    std::string::const_iterator itMatch);

};
/*
 *    A StrTokenizer object facilitates the process of parsing a structured string 
 *    if it consists of segmentations that split from each other by a delimiter.
 *    Example:
 *
 *    StrTokenizer tokens(line, ",") ;
 *    if ( tokens.count_tokens() != 5)
 *        std::cout << "format error." << endl ;
 *    for (int i=0; i<5; i++)
 *        std::cout << tokens.token(i) << endl ;
 */

class StrTokenizer
{
protected:
	std::vector<std::string> tokens ;
	int32_t idx ;
public:
	StrTokenizer(std::string, std::string sep = " ") ;
	void parse(std::string, std::string sep) ;
	int32_t count_tokens() ;
	std::string next_token() ;
	std::string token(int32_t i) ;
};

#endif

/******************  test case.  ******************/
/*
	std::string longStr = "\t \nabc\td\t" ;

	std::cout << StringUtil::TrimBothSides(longStr) << "\t" << StringUtil::TrimBothSides(longStr).size() << std::endl ;
	std::cout << StringUtil::TrimLeft(longStr) << "\t" << StringUtil::TrimLeft(longStr).size() << std::endl ;
	std::cout << StringUtil::TrimRight(longStr) << "\t" << StringUtil::TrimRight(longStr).size() << std::endl ;

	std::cout << StringUtil::MatchWildcard("[a-z][0-9]","h7") << std::endl ;
	std::cout << StringUtil::MatchWildcard("hell*","hexllo world.") << std::endl ;
*/
