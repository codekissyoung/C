//###################################################################
//daTrie.h
//AUTHOR  : DongYi dolo@vip.sina.com, xmFan1983@gmail.com
//TIME    : 2005-08-16, 2015-04-28
//FUNCTION: FAST and COMPACT TRIE for 1B1 letter match.
//        : 双向字典 WORD->ID ID->WORD
//NOTE    : 参考了DA.c,修正了其中的BUGS.
//###################################################################


#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <vector>
#include <map>
#include <string>
#ifndef __DA_TRIE_H_
#define __DA_TRIE_H_

using namespace std;

struct wordVSM;
typedef struct wordVSM wordVSM;
typedef struct wordVSM *pwordVSM;
struct wordVSM {
	int nWID;
	int nWTF;
};
typedef std::map<int,wordVSM> vsmDOC;
typedef std::map<int,wordVSM> *pvsmDOC;

struct wordVEC;
typedef struct wordVEC wordVEC;
typedef struct wordVEC *pwordVEC;
struct wordVEC {
	int nWID;
	int nWTF;
//目前为了简便,使用固定个词频
#define MAX_POS_NUM	8
	short nPOS[MAX_POS_NUM];
};

typedef std::map<int,wordVEC> docMap;
typedef std::map<int,wordVEC> *pdocMap;

//hhj 20090313
struct wordInfo;
typedef struct wordInfo wordInfo;
struct wordInfo {
        int nWID;
        int nOffset;
};

template <class _Tp>
class daTrie 
{
public:
	struct wordProp;
	typedef struct daTrie<_Tp>::wordProp wordProp;
	typedef struct daTrie<_Tp>::wordProp *pwordProp;
	struct wordProp {
		_Tp nProp;
		int       nOFF;
	};

	struct daElm;
	typedef struct daTrie<_Tp>::daElm daElm;
	typedef struct daTrie<_Tp>::daElm *pdaElm;
	struct daElm {
		int nBase;
		int nCheck;
	};

	struct resolveElm;
	typedef struct daTrie<_Tp>::resolveElm resolveElm;
	typedef struct daTrie<_Tp>::resolveElm *presolveElm;
	struct resolveElm {
		int c;
		int nBase;
	};
	
	struct resolveStat;
	typedef struct daTrie<_Tp>::resolveStat resolveStat;
	typedef struct daTrie<_Tp>::resolveStat *presolveStat;
	struct resolveStat{
		int			nElm;
		resolveElm	pElm[256];
	};

	struct daTrieNode;
	typedef struct daTrie<_Tp>::daTrieNode daTrieNode;
	typedef struct daTrie<_Tp>::daTrieNode *pdaTrieNode;
	struct daTrieNode {
#define DATRIE_MAGIC	"DCT.DY.DTRIE"
		char		sMagic[12];
unsigned char		sTranslator[256];
		int			nWord;
		int			nElm;
		pdaElm		pElm;
		int			nProp;
	pwordProp		pProp;
		int			nKEYSIZ;
		int			nKEYOFF;
		char	*	pKEYBUF;
	};

public:

	daTrie( int bCase = 1, int nSpeedLevel = 0 )
			: m_nSpeedLevel(abs(nSpeedLevel) % 129), m_nSrchLEFT(2), m_ptrMap( NULL ), m_sizeMap( 0 )
	{
		__Initialize( bCase );
	}
	~daTrie()
	{
		__clean( ); 
	}

private:
	int		   m_nSpeedLevel;
	int		   m_nSrchLEFT;	
	daTrieNode m_daTrie;
	char*      m_ptrMap;
	int        m_sizeMap;
			
	unsigned char m_digitTAB[256];
	unsigned char m_alphaTAB[256];
	unsigned char m_punctTAB[256];
	unsigned char m_spaceTAB[256];

private:
	void resizeElm( int nIdx )
	{
		if ( nIdx >= m_daTrie.nElm ) {
			int i;
			//INCREASE BY 1.5 
			int nSIZE = ( (nIdx + 511) & ~511 ) * 3 / 2;
			pdaElm p = (pdaElm)realloc( m_daTrie.pElm, nSIZE * sizeof(daElm) );
			if( p == NULL ) {
				nSIZE = (nIdx + 1023) & ~511;
				p = (pdaElm)realloc( m_daTrie.pElm, nSIZE * sizeof(daElm) );
				assert ( p != NULL );
			}
			m_daTrie.pElm = p;
			for( i = m_daTrie.nElm; i < nSIZE; i++ ) {
				m_daTrie.pElm[i].nBase = 0;
				m_daTrie.pElm[i].nCheck= 0;
			}
			m_daTrie.nElm = nSIZE;
		}
	}

	void resizeProp( int nOFF )
	{
		if( nOFF >= m_daTrie.nProp ) {
			int nSIZE = ( nOFF + 127 ) & ~63;
			m_daTrie.pProp = (wordProp*)realloc( m_daTrie.pProp, nSIZE * sizeof(wordProp) );
			assert( m_daTrie.pProp != NULL );
			memset( m_daTrie.pProp + m_daTrie.nProp, 0, sizeof( wordProp ) * ( nSIZE - m_daTrie.nProp ) );
			m_daTrie.nProp = nSIZE;
		}
	}

	void resizeBUF( int nInc )
	{
		if( nInc + m_daTrie.nKEYOFF >= m_daTrie.nKEYSIZ ) {
			int nSIZE = ( ( m_daTrie.nKEYOFF + nInc + 1023 ) & ~1023) * 3 / 2;
			m_daTrie.pKEYBUF = (char*)realloc( m_daTrie.pKEYBUF, nSIZE * sizeof(char) );
			assert( m_daTrie.pKEYBUF != NULL );
			//			memset( m_daTrie.pKEYBUF + m_daTrie.nKEYSIZ, 0, nSIZE - m_daTrie.nKEYSIZ );
			m_daTrie.nKEYSIZ = nSIZE;
		}
	}

	int getBase( int nIdx )
	{
		return (nIdx >= m_daTrie.nElm) ? 0 : m_daTrie.pElm[nIdx].nBase;
	}

	void setBase( int nIdx, int v )
	{
		resizeElm( nIdx );
		m_daTrie.pElm[nIdx].nBase = v;
	}

	int getCheck( int nIdx )
	{
		return (nIdx >= m_daTrie.nElm) ? 0 : m_daTrie.pElm[nIdx].nCheck;
	}

	void setCheck( int nIdx, int v )
	{
		resizeElm( nIdx );
		m_daTrie.pElm[nIdx].nCheck = v;
	}

	_Tp *getProp( int nIdx )
	{
		int nOFF = getBase( nIdx );
		return (nOFF >= m_daTrie.nProp) ? NULL : &m_daTrie.pProp[nOFF].nProp;
	}

	void setProp( int nIdx, const _Tp &iProp )
	{
		int nOFF = getBase( nIdx );
		resizeProp( nOFF );
		m_daTrie.pProp[nOFF].nProp = iProp;
	}

	void setKEY( int nIdx, char *sKEY )
	{
		int nOFF = getBase( nIdx );
		int nLen = strlen( sKEY ) + 1;
		resizeProp( nOFF );
		resizeBUF ( nLen );
		m_daTrie.pProp[nOFF].nOFF = m_daTrie.nKEYOFF;
		memcpy( m_daTrie.pKEYBUF + m_daTrie.nKEYOFF, sKEY, nLen );
		m_daTrie.nKEYOFF += nLen;
	}

	void pushChild( presolveStat pStat, int c, int orig_base)
	{
		int n = pStat->nElm;
		pStat->nElm++;
		pStat->pElm[n].c = c;
		pStat->pElm[n].nBase = orig_base;
	}

	int collectChild( presolveStat pStat, int parent, unsigned char c)
	{
		int i,idx,nLeft = m_nSrchLEFT;
		int base = getBase( parent );

		pStat->nElm = 0;
		for ( i = 0; i < 256; i++) {
			idx = parent + base + i;
			if ( i == c ) {
				pushChild( pStat, i, ( c == 0 ) ? (m_daTrie.nWord + 1) : 1 );
			}
			else if ( getCheck( idx ) == parent ) {
				pushChild( pStat, i, getBase(idx) );
			}
		}

		i = 0;
		while( i < pStat->nElm ) {
			if ( getCheck( nLeft + pStat->pElm[i].c ) != 0 ) {
				nLeft++; i = 0;
				continue;
			}
			i++;
		}

		int nChr = 128 - m_nSpeedLevel;
		if( pStat->nElm >= nChr ) m_nSrchLEFT = nLeft;

		for( i = 0; i < pStat->nElm; i++ ) {
			if( pStat->pElm[i].c != c ) {
				idx = parent + base + pStat->pElm[i].c;
				setBase ( idx, 0 ); setCheck( idx, 0 );
			}
		}

		return nLeft;
	}

	void moveChildren( presolveStat pStat, int parent, int left )
	{
		int i,move;
		int orig_base = getBase( parent );

		//move = old_left - new_left = ( parent + base(parent) ) - left;
		move = parent + orig_base - left;
		//new_base = orig_base - move = orig_base - ( parent + orig_base - left )
		setBase( parent, left - parent );

		for (i = 0; i < pStat->nElm; i++) {
			int idx = left + pStat->pElm[i].c;
			setCheck( idx, parent );
			if( pStat->pElm[i].c == 0 )
				setBase ( idx, pStat->pElm[i].nBase );
			else
				setBase ( idx, pStat->pElm[i].nBase + move);
		}

		for (i = 0; i < pStat->nElm; i++) {
			int k;
			int old_left, old_child;

			//old_child = t = s + base(s) + c
			old_child = parent + orig_base + pStat->pElm[i].c; 
			//s = t = old_child ; left = old_child + base(old_child) = old_child + pStat->pElm[i].nBase
			old_left = old_child + pStat->pElm[i].nBase;

			for (k = 0; k < 256; k++) {
				int orig_check = getCheck( old_left + k);
				if ( orig_check == old_child && getCheck( orig_check ) == 0 ) {
					setCheck( old_left + k, orig_check - move );
				}
			}
		}
	}

	int resolveConflict( int parent, unsigned char c )
	{
		int nLeft;
		resolveStat curChild;

		nLeft = collectChild( &curChild,  parent, c);
		moveChildren( &curChild, parent, nLeft);
		return nLeft + c;
	}

	int addChr( int parent, unsigned char c )
	{
		int next = parent + getBase(parent) + c;
		if( getCheck( next ) > 0 ) {
			return resolveConflict( parent , c); 
		}
		setCheck( next, parent );
		if( c == 0 ) 
			setBase ( next, m_daTrie.nWord + 1 );
		else
			setBase ( next, (parent == 1) ? (256 - next) : 1 );
		return next;
	}

	void __Initialize( int bCase )
	{
		int i;
		memset( &m_daTrie, 0, sizeof( daTrieNode ) );
		memcpy( m_daTrie.sMagic, DATRIE_MAGIC, 12 );
		for( i = 0; i < 256; i++ ) {
			m_daTrie.sTranslator[i] = (bCase ? toupper( i ) : i);
		}

		memset( m_digitTAB, 0, 256 );
		for( i = 0; i < 256; i++ ) { if( isdigit( i ) ) m_digitTAB[i] = 1; }
		memset( m_alphaTAB, 0, 256 );
		for( i = 0; i < 256; i++ ) { if( isalpha( i ) ) m_alphaTAB[i] = 1; }
		memset( m_punctTAB, 0, 256 );
		for( i = 0; i < 256; i++ ) { if( ispunct( i ) ) m_punctTAB[i] = 1; }
		memset( m_spaceTAB, 0, 256 );
		for( i = 0; i < 256; i++ ) { if( isspace( i ) ) m_spaceTAB[i] = 1; }

		setBase ( 1, 1 ); setCheck( 1, -1);
	}

	void __clean( )
	{
		if ( m_ptrMap ) {
			munmap( m_ptrMap, m_sizeMap); 
			m_ptrMap = NULL; m_sizeMap = 0;
		}
		else {
			if( m_daTrie.pElm ) { free( m_daTrie.pElm ); }
			if( m_daTrie.pProp) { free( m_daTrie.pProp); }
			if( m_daTrie.pKEYBUF) { free( m_daTrie.pKEYBUF); }
		}
		memset( &m_daTrie, 0, sizeof( daTrieNode ) );
	}

private:

	int __dynac_load( char *sIndexName )
	{
		int i,nFD;
		daTrieNode iNode;

		nFD = open( sIndexName, O_RDONLY, 0666);
		if( nFD < 0 ) return 0;

		//HEADER
		i = read( nFD, &iNode, sizeof( daTrieNode ) );
		if( i != sizeof( daTrieNode ) ) return 0;
		if( memcmp( iNode.sMagic, DATRIE_MAGIC, 12 ) ) {
			close( nFD ); return 0;
		}
		__clean( );
		iNode.pElm = (pdaElm)malloc( sizeof( daElm ) * iNode.nElm );
		assert( iNode.pElm != NULL );
		iNode.pProp= (pwordProp)malloc( sizeof( wordProp ) * iNode.nProp );
		assert( iNode.pProp != NULL );
		iNode.pKEYBUF = (char *)malloc( sizeof( char ) * iNode.nKEYSIZ );
		assert( iNode.pKEYBUF != NULL );
		//daElm
		i = read( nFD, iNode.pElm , sizeof( daElm ) * iNode.nElm );
		if( i != (int)( sizeof( daElm ) * iNode.nElm) ) return 0;
		//wordProp
		i = read( nFD, iNode.pProp, sizeof( wordProp ) * iNode.nProp );
		if( i != (int)(sizeof( wordProp ) * iNode.nProp) ) return 0;
		//KEYBUF
		i = read( nFD, iNode.pKEYBUF, sizeof( int ) * iNode.nKEYSIZ );
		if( i != (int)(sizeof( char ) * iNode.nKEYSIZ ) ) return 0;

		memcpy( &m_daTrie, &iNode, sizeof( daTrieNode ) );

		close(nFD);
		return 1;
	}

	int __static_load( char *sIndexName )
	{
		int i,nFD;
		daTrieNode iNode;

		nFD = open( sIndexName, O_RDONLY, 0666);
		if( nFD < 0 ) return 0;

		//HEADER
		i = read( nFD, &iNode, sizeof( daTrieNode ) );
		if( i != sizeof( daTrieNode ) ) { close(nFD); return 0; }
		if( memcmp( iNode.sMagic, DATRIE_MAGIC, 12 ) ) {
			close( nFD ); return 0;
		}

		__clean( );

		m_sizeMap = lseek( nFD, 0L, SEEK_END );
		lseek ( nFD, 0L, SEEK_SET);

		m_ptrMap = (char *)::mmap( NULL, m_sizeMap, PROT_READ, MAP_FILE|MAP_SHARED, nFD, 0);
		if( m_ptrMap == MAP_FAILED ) {
			close( nFD ); return 0;
		}
		int iStep = 64 * 1024;
		char c[iStep];
		char *p = (char *)m_ptrMap;
		for (int i = 0; i < (m_sizeMap/iStep); i++) memcpy(c, p + i * iStep, iStep);
		iNode.pElm    = (pdaElm)    ( m_ptrMap + sizeof( daTrieNode ) );
		iNode.pProp   = (pwordProp) ( m_ptrMap + sizeof( daTrieNode ) + 
				sizeof( daElm ) * iNode.nElm );
		iNode.pKEYBUF = (char * )   ( m_ptrMap + sizeof( daTrieNode ) + 
				sizeof( daElm ) * iNode.nElm + 
				sizeof( wordProp ) * iNode.nProp );

		memcpy( &m_daTrie, &iNode, sizeof( daTrieNode ) );
		close( nFD );
		return 1;
	}

public:

	void clean( )
	{
		__clean();
	}

	void clear( )
	{
		m_daTrie.nWord = 0;
		m_daTrie.nKEYOFF = 0;
		memset( m_daTrie.pElm , 0, sizeof( daElm )  * m_daTrie.nElm );
		memset( m_daTrie.pProp, 0, sizeof( wordProp) * m_daTrie.nProp );
		setBase ( 1, 1 ); setCheck( 1, -1);
	}

	int getWordNum( )
	{
		return m_daTrie.nWord;
	}	

	int prefixSrch( char *sKEY , _Tp **ppProp = NULL, int *nWordID = NULL )
	{
		int left, next,parent = 1;
		int	bGBK = 0;
		unsigned char *p;

		left = parent + getBase( parent );	
		for ( p = (unsigned char *)sKEY; *p; p++ ) {
			if( bGBK ) { next = left + *p; bGBK = 0; }
			else {
				if( *p > 0x80 && *(p+1) >= 0x3F ) { next = left + *p; bGBK = 1; }
				else { next = left + m_daTrie.sTranslator[*p]; }
			}
			if ( getCheck( next ) != parent ) return 0;
			parent = next;
			left = parent + getBase( parent );
		}
		if( getCheck( left ) == parent ) {
			if( nWordID != NULL ) *nWordID = getBase( left );
			if( ppProp  != NULL ) *ppProp  = &m_daTrie.pProp[getBase( left )].nProp;
			return 1;
		}
		return 0;
	}

	int prefixMatch( char *sTEXT, _Tp ** ppProp = NULL, int *nWordID = NULL)
	{
		int left,next,parent = 1;
		int acceptPOINT = 0;
		int	bGBK = 0;
		unsigned char *p, *pOK;

		left = parent + getBase( parent );	
		for ( pOK = p = (unsigned char *)sTEXT; *p; p++ ) {
			if( bGBK ) { next = left + *p; bGBK = 0; }
			else {
				if( *p > 0x80 && *(p+1) >= 0x3F ) { next = left + *p; bGBK = 1; }
				else { next = left + m_daTrie.sTranslator[*p]; }
			}
			if ( getCheck( next ) != parent ) {
				if ( acceptPOINT ) {
					if( nWordID != NULL ) *nWordID = getBase( acceptPOINT ) ;
					if( ppProp  != NULL ) *ppProp  = &m_daTrie.pProp[getBase( acceptPOINT )].nProp;
					return (int)( pOK - (unsigned char *)sTEXT );
				}
				return 0;
			}
			parent = next;
			left = parent + getBase( parent );
			if ( getCheck( left ) == parent ) {
				pOK = p + 1; acceptPOINT = left;
			}
		}

		if ( acceptPOINT ) {
			if( nWordID!= NULL ) *nWordID = getBase( acceptPOINT );
			if( ppProp != NULL ) *ppProp  = &m_daTrie.pProp[getBase( acceptPOINT )].nProp;
			return (int)( pOK - (unsigned char *)sTEXT );
		}
		return 0;
	}


	void KEYEnumeric( int nIdx, int nLen, vector<string> &wordEnum, vector<_Tp > *propEnum = NULL)
	{
		int i;
		int base = getBase(nIdx);
		static char sBUF[1024];

		if( nLen >= 1024 ) return;
		for (i = 0; i < 256; i++) {
			if ( getCheck( nIdx + base + i) == nIdx ) {
				sBUF[nLen] = i;
				if (i == 0) {
					wordEnum.push_back( string(sBUF) ); 
					if( propEnum != NULL ) propEnum->push_back( *getProp( nIdx + base + i ) ); 
				}
				else { KEYEnumeric( nIdx + base + i, nLen + 1, wordEnum, propEnum ); }
			}
		}
	}

	void prefixEnum( int nIdx, int nLen, char *sBUF, int nLimit, vector<string> &wordEnum, vector<_Tp > *propEnum )
	{
		int i;
		int base = getBase(nIdx);

		if( nLen >= nLimit )
            return;
		for (i = 0; i < 256; i++) 
        {
			if ( getCheck( nIdx + base + i) == nIdx )
            {
				sBUF[nLen] = i;
				if (i == 0) 
                {
					wordEnum.push_back( string(sBUF) ); 
					if( propEnum != NULL )
                        propEnum->push_back( *getProp( nIdx + base + i ) ); 
				}
				else
                {
                    prefixEnum( nIdx + base + i, nLen + 1, sBUF, nLimit, wordEnum, propEnum );
                }
			}
		}
	}

	int prefixLike( char *sKEY, vector<string> &wordEnum, vector<_Tp> *propEnum = NULL )
	{
		int left, next,parent = 1;
		int	bGBK = 0;
		unsigned char *p;

		wordEnum.clear(); 
		if( propEnum != NULL )
            propEnum->clear();

		left = parent + getBase( parent );	
		for ( p = (unsigned char *)sKEY; *p; p++ )
        {
			if( bGBK ) 
            { 
                next = left + *p; 
                bGBK = 0; 
            }
			else 
            {
				if( *p > 0x80 && *(p+1) >= 0x3F )
                {
                    next = left + *p;
                    bGBK = 1; 
                }
				else 
                {
                    next = left + m_daTrie.sTranslator[*p];
                }
			}
			if ( getCheck( next ) != parent ) 
                return 0;
			parent = next;
			left = parent + getBase( parent );
		}

		{
			char sBUF[1024];
			int  nLen = strlen( sKEY );
			strcpy( sBUF, sKEY );
			prefixEnum( parent, nLen, sBUF, sizeof(sBUF) - 1, wordEnum, propEnum );
		}
		return wordEnum.size();
	}

	//RETURN VAL: 
	// -1:	ERROR
	//0-N:	WORDID
	int KEYInsert( char *sKEY, const _Tp &iProp = 1)
	{
		int i, next, parent = 1;
		int bInsert = 0;
		int bGBK = 0;
		unsigned char c;
		int nLen = strlen( sKEY );

		if( nLen == 0 ) return -1;

		for (i = 0; i <= nLen; i++) {
			if( bGBK ) { c = (unsigned char)sKEY[i]; bGBK = 0; }
			else {
				if( (unsigned char)sKEY[i] > 0x80 && (unsigned char)sKEY[i+1] >= 0x3F ) {
					c = (unsigned char)sKEY[i]; bGBK = 1;
				}
				else { c = m_daTrie.sTranslator[(unsigned char)sKEY[i]]; }
			}
			next = parent + getBase( parent ) + c;
			if ( getCheck( next ) != parent ) {
				bInsert = 1;
				next = addChr( parent, c );
			}
			parent = next;
		}
		if( bInsert ) {
			m_daTrie.nWord++; setKEY( parent, sKEY );
		}
		setProp( parent, iProp );

		//RETURN WORDID;
		return getBase( parent );
	}

	int KEYDelete( char *sKEY )
	{
		int i,next,parent = 1;
		int bGBK = 0;
		unsigned char c;
		int nLen = strlen( sKEY );

		if( nLen == 0 ) return 0;
		for (i = 0; i <= nLen; i++) {
			if( bGBK ) { c = (unsigned char)sKEY[i]; bGBK = 0; }
			else {
				if( (unsigned char)sKEY[i] > 0x80 && (unsigned char)sKEY[i+1] >= 0x3F ) {
					c = (unsigned char)sKEY[i]; bGBK = 1;
				}
				else { 
					c = m_daTrie.sTranslator[(unsigned char)sKEY[i]];
				}
			}
			next = parent + getBase( parent ) + c;
			if ( getCheck( next ) != parent ) return 0;
			parent = next;
		}
		setBase( parent, 0 ); setCheck( parent, 0 );
		return 1;
	}

	int updatePROP( char *sKEY, const _Tp &iProp )
	{
		int left,next,parent = 1;
		int bGBK = 0;
		unsigned char c;
		unsigned char *p;

		left = parent + getBase( parent );	
		for ( p = (unsigned char *)sKEY; *p; p++ ) {
			if( bGBK ) { next = left + *p; bGBK = 0; }
			else {
				if( *p > 0x80 && *(p+1) >= 0x3F ) { next = left + *p; bGBK = 1; }
				else { next = left + m_daTrie.sTranslator[*p]; }
			}
			if ( getCheck( next ) != parent ) return 0;
			parent = next;
			left = parent + getBase( parent );
		}
		if( getCheck( left ) == parent ) {
			int nID = getBase( parent );
			m_daTrie.pProp[nID].nProp = iProp;
			return 1;
		}
		return 0;
	}

	int updatePROP( int nWordID, const _Tp &iProp)
	{
		if( nWordID < 0 || nWordID >= m_daTrie.nWord ) return 0;
		m_daTrie.pProp[nWordID].nProp = iProp;
		return 1;
	}

	_Tp getPROP( int nWordID )
	{
		if( nWordID < 0 || nWordID >= m_daTrie.nWord ) return 1;
		return m_daTrie.pProp[nWordID].nProp;
	}

	char *ID2KEY( int nWordID )
	{
		if( nWordID <= 0 || nWordID > m_daTrie.nWord ) return "";
		return m_daTrie.pKEYBUF + m_daTrie.pProp[nWordID].nOFF;
	}

	int KEY2ID( char *sText )
	{
		int i,nID = -1;
		i = prefixSrch( sText, NULL, &nID );
		if( i == 0 ) return -1;
		return nID;
	}

	void DUMP( )
	{
		int i, nEmpty = 0;

		printf( "\n%-7.7s | %-7.7s | %-7.7s\n", "INDEX", "BASE", "CHECK" );
		for(i=1; i <= m_daTrie.nElm; i++) {
			if( getCheck(i) == 0) { nEmpty++; continue; }
			printf("%7d | %7d | %7d\n", i, getBase(i), getCheck(i)) ;
		}
		printf( "=====================================\n");

		printf("TOTAL WRD=[%d]\n", m_daTrie.nWord);
		printf("TOTAL ELM=[%d]\n", m_daTrie.nElm );
		printf("EMPTY ELM=[%d]\n", nEmpty );
	} 

	void INFO( )
	{
		int i, nEmpty = 0;

		for(i=1; i <= m_daTrie.nElm; i++) {
			if( getCheck(i) == 0) { nEmpty++; }
		}
		printf( "=====================================\n");

		printf("TOTAL WRD=[%d]\n", m_daTrie.nWord);
		printf("TOTAL ELM=[%d]\n", m_daTrie.nElm );
		printf("EMPTY ELM=[%d]\n", nEmpty );
	} 

	int Save( char *sIndexName )
	{
		int i,nFD;
		daTrieNode iNode;

		if( m_daTrie.nWord == 0 ) return 0;	

		nFD = open( sIndexName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if( nFD < 0 ) return 0;

		//Write HEADER INFO
		memcpy( &iNode, &m_daTrie, sizeof( daTrieNode ) );
		//Write daElm
		for( i = m_daTrie.nElm - 1; i >= 0; i-- ) {
			if( getCheck(i) > 0 ) break;
		}
		iNode.nElm    = i + 1;
		iNode.nProp   = (iNode.nWord > iNode.nProp ) ? iNode.nProp : iNode.nWord + 1;
		iNode.nKEYSIZ = iNode.nKEYOFF + 1;

		//HEADER
		i = write( nFD, &iNode, sizeof( daTrieNode ) );
		assert( i == sizeof( daTrieNode ) );
		//daElm
		i = write( nFD, iNode.pElm , sizeof( daElm ) * iNode.nElm );
		assert( (size_t)i == sizeof( daElm ) * iNode.nElm );
		//wordProp
		i = write( nFD, iNode.pProp, sizeof( wordProp ) * iNode.nProp );
		assert( (size_t)i == sizeof( wordProp ) * iNode.nProp );
		//_KEYBUF
		i = write( nFD, iNode.pKEYBUF, sizeof( char ) * iNode.nKEYSIZ );
		assert( (size_t)i == sizeof( char ) * iNode.nKEYSIZ );

		close( nFD );

		return 1;
	}

	int Load( char *sIndexName, bool bRdOnly = false )
	{
		if( bRdOnly ) 
			return __static_load( sIndexName );
		else
			return __dynac_load( sIndexName );
	}
	/****************************************************************************************

	  propSize 									
	  获得匹配字符和匹配字符的长度							
	  作者： liuqi
	  日期： 2006-07-14

	 ******************************************************************************************/
	int propSIZE( char * sText, int nLen, map<_Tp,int> &propMap)
	{

		int i;  
		_Tp *pProp; 
		unsigned char *p = (unsigned char *)sText;
		unsigned char *pLimit = p + nLen; 

		propMap.clear();
		while( p < pLimit ) {
			i = prefixMatch( (char *) p, &pProp );
			if( i ) {
				propMap[*pProp] += i;

			}       
			else {  
				if( *p > 0x80 && *(p+1) >= 0x3F ) i = 2;
				else if( m_digitTAB[*p] ) { while( m_digitTAB[ *(p+i) ] ) i++; }
				else if( m_alphaTAB[*p] ) { while( m_alphaTAB[ *(p+i) ] ) i++; }
				else { i = 1; }
			}       
			p += i; 
			while( m_punctTAB[*p] || m_spaceTAB[*p] ) p++;
		}
		return propMap.size();
	}

	int propCOUNT( char *sText, int nLen, map<_Tp,int> &propMap )
	{
		int i;
		_Tp *pProp;
		unsigned char *p = (unsigned char *)sText;
		unsigned char *pLimit = p + nLen;

		propMap.clear();
		while( p < pLimit ) {
			i = prefixMatch( (char *) p, &pProp );
			if( i ) {
				if( m_digitTAB[ *( p + i ) ] ) { while( m_digitTAB[ *( p + i) ] ) i++; }
				else if( m_alphaTAB[ *( p + i ) ] ) { while( m_alphaTAB[ *( p + i) ] ) i++; }
				else {
					propMap[*pProp] = propMap[*pProp] + 1;
				}
			}
			else {
				if( *p > 0x80 && *(p+1) >= 0x3F ) i = 2;
				else if( m_digitTAB[*p] ) { while( m_digitTAB[ *(p+i) ] ) i++; }
				else if( m_alphaTAB[*p] ) { while( m_alphaTAB[ *(p+i) ] ) i++; }
				else { i = 1; }
			}
			p += i;
			while( m_punctTAB[*p] || m_spaceTAB[*p] ) p++;
		}
		return propMap.size();
	}

	int calcRANK( char *sText, int nLen, docMap &docVEC )
	{
		int i,nID;
		int nSequeue = 1;
		unsigned char *p = (unsigned char *)sText;
		unsigned char *pLimit = p + nLen;
		unsigned char *q;

		while( p < pLimit ) {
			i = prefixMatch( (char *)p, NULL, &nID );
			if( i ) {
				q = p + i - 1;
				if( m_digitTAB[q[0]] && m_digitTAB[q[1]] ) { while( m_digitTAB[*(p+i)] ) i++; }
				else if( m_alphaTAB[q[0]] && m_alphaTAB[q[1]] ) { while( m_alphaTAB[*(p+i)] ) i++; }
				else {
					docMap::iterator it = docVEC.find( nID );
					if( it != docVEC.end() ) {
						if( it->second.nWTF < MAX_POS_NUM ) {
							it->second.nPOS[it->second.nWTF] = nSequeue;
						}
						it->second.nWTF++;
					}
					else {
						docVEC[nID].nWID = nID;
						docVEC[nID].nWTF = 1;
						docVEC[nID].nPOS[0] = nSequeue;
					}
				}
				p += i;
			}
			else {
				if( *p > 0x80 && *(p+1) > 0x3F ) i = 2;
				else if( m_digitTAB[*p] ) { while( m_digitTAB[ *(p+i) ] ) i++; }
				else if( m_alphaTAB[*p] ) { while( m_alphaTAB[ *(p+i) ] ) i++; }
				else { i = 1; }
				p += i;
			}
			while( (m_punctTAB[*p] || m_spaceTAB[*p]) && (p < pLimit) ) p++;
			nSequeue++;
		}
		return docVEC.size();
	}

	int calcVSM( char *sText, int nLen, vsmDOC &docVEC )
	{
		int i,nID;
		unsigned char *p = (unsigned char *)sText;
		unsigned char *pLimit = p + nLen;
		unsigned char *q;

		while( p < pLimit ) {
			i = prefixMatch( (char *)p, NULL, &nID );
			if( i ) {
				q = p + i - 1;
				if( m_digitTAB[q[0]] && m_digitTAB[ q[1] ] ) { while( m_digitTAB[ *( p + i) ] ) i++; }
				else if( m_alphaTAB[q[0]] && m_alphaTAB[q[1]] ) { while( m_alphaTAB[ *( p + i) ] ) i++; }
				else {
					docVEC[nID].nWID = nID;
					docVEC[nID].nWTF++;
				}
				p += i;
			}
			else {
				if( *p > 0x80 && *(p+1) > 0x3F ) i = 2;
				else { i = 1; }
				p += i;
			}
		}
		return docVEC.size();
	}

	int wordFLT( char *sText, int nLen, char cRep, int bSpecial = 0 )
	{
		int i,bHead;
		unsigned char *p = (unsigned char *)sText;
		unsigned char *pLimit = p + nLen;

		nLen = 0;
		bHead= 1;
		while( p < pLimit ) {
			if( *p <= 0x80 && !isprint( *p ) ) {
				bHead = 1; sText[nLen++] = cRep;
				++p; continue; 
			}
			//SPECIAL DEALED.
			if( ( *p == '-' || isspace( *p ) || *p == '.' || *p == '#' || *p == '$' || *p == '\'') 
					&& bHead && bSpecial ) { ++p; continue; }
			i = prefixMatch( (char *)p );
			if( i ) {
				//ERROR MATCH
				if( p[0] >= 0x81 && p[1] >= 0x40 && i < 2 ) {
					i = 2;
					for( int k = 0; k < i; k++ ) sText[nLen++] = *p++;
				}
				else {
					bHead = 1;
					sText[nLen++] = cRep; p += i;
				}
			}
			else {
				bHead = 0;
				i = 1;
				//汉字
				if( p[0] > 0x80 ) {

					//剔除单个ASCII开头的
					if( nLen == 1 && bSpecial ) nLen = 0;

					//GB2312:B0A1-F7FE
					if( p[0] >= 0xB0 && p[0] <= 0xF7 &&
							p[1] >= 0xA1 && p[1] <= 0xFE ) {
						i = 2;
					}
					//GBK/3: 8140-A0FE
					else if( p[0] >= 0x81 && p[0] <= 0xA0 &&
							p[1] >= 0x40 && p[1] <= 0xFE ) {
						i = 2;
					}
					//GBK/4: AA40-FEA0
					else if( p[0] >= 0xAA && p[0] <= 0xFE &&
							p[1] >= 0x40 && p[1] <= 0xA0 ) {
						i = 2;
					}
					else //GB 2312 非汉字符号区 GBK/1: A1A1-A9FE
						if( p[0] >= 0xA1 && p[0] <= 0xA9 &&
								p[1] >= 0xA1 && p[1] <= 0xFE ) {
							i = 2;
						}
					//GB 13000.1 扩充非汉字区 GBK/5: A840-A9A0
						else if( p[0] >= 0xA8 && p[0] <= 0xA9 &&
								p[1] >= 0x40 && p[1] <= 0xFE ) {
							i = 2;
						}
						//不认识的
						else if( p[0] >= 0x81 && p[1] >= 0x40 ) {
							bHead = 1; i = 0;
							sText[nLen++] = cRep; 
							p += 2;
						}
					//错误:错位
						else {
							bHead = 1; i = 0;
							sText[nLen++] = cRep; 
							p += 1;
						}
				}
				for( int k = 0; k < i; k++ ) sText[nLen++] = *p++;
			}
		}

		//SPECIAL DEALED.
		if( bSpecial && nLen ) {	
			--nLen;
			while( nLen >= 0 ) {
				if( sText[nLen] == cRep || 
						sText[nLen] == '-'	||
						sText[nLen] == '.'  ||
						isdigit( sText[nLen] ) ) {
					sText[nLen] = cRep;
					nLen--;
				}
				else { break; }
			}
			++nLen;
		}
		sText[nLen] = 0;
		return nLen;
	}
	
	//hhj 20090313
	//功能：对buffer进行切词并记录词语出现位置
	int calcVSM2( char *sText, int nLen, vector<wordInfo> & vecWord)
	{
		vecWord.clear();
		
		int i,nID;
		unsigned char *p = (unsigned char *)sText;
		unsigned char *pLimit = p + nLen;
		unsigned char *q;
		wordInfo wordItem;
		
		while( p < pLimit ) {
			i = prefixMatch( (char *)p, NULL, &nID );
			if( i ) {
				q = p + i - 1;
				if( m_digitTAB[q[0]] && m_digitTAB[ q[1] ] ) { while( m_digitTAB[ *( p + i) ] ) i++; }
				else if( m_alphaTAB[q[0]] && m_alphaTAB[q[1]] ) { while( m_alphaTAB[ *( p + i) ] ) i++; }
				else {
					wordItem.nWID = nID;
					wordItem.nOffset = (p+i-(unsigned char *)sText);
					vecWord.push_back(wordItem);
				}
				p += i;
			}
			else {
				if( *p > 0x80 && *(p+1) > 0x3F ) i = 2;
				else { i = 1; }
				p += i;
			}
		}
		return vecWord.size();
	}
};

#endif
