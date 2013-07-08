/*
 * $Id: hbjson.c 9826 2012-11-16 23:38:09Z andijahja $
 */

/*
 * Harbour Project source code:
 * JavaScript Object Notation (JSON)
 *
 * Copyright 2010 Mindaugas Kavaliauskas <dbtopas / at / dbtopas.lt>
 * www - http://harbour-project.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, if you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

#if defined( __WATCOMC__ )
   #pragma disable_message ( 136 )
#endif

#include "hbjson.h"

/*
   The application/json Media Type for JavaScript Object Notation (JSON)
   http://www.ietf.org/rfc/rfc4627.txt

      C level functions:
        char * hb_jsonEncode( PHB_ITEM pValue, ULONG * pnLen, BOOL fHuman );
           pValue  - value to encode;
           pnLen   - if pnLen is not NULL, length of returned buffer is
                     stored to *pnLen;
           fHuman  - format to be human redable;
           returns pointer to encoded JSON buffer. buffer must be fried
              by the caller.

        ULONG hb_jsonDecode( const char * szSource, PHB_ITEM pValue );
           szSource - JSON source;
           pValue   - item to store decoded value. Item value is
                      undetermined in case of error;
           returns number of bytes decoded from the buffer. This allows
              to use the remaining part of the buffer for some other
              purposes. Returns 0 on error.

      Harbour level functions:
        hb_jsonDecode( cJSON, @xValue ) --> nLengthDecoded
        hb_jsonEncode( xValue [, lHuman = .F. ] ) --> cJSON

      Note:
        - JSON encode functions are safe for recursive arrays and hashes.
          Recursive part of array or hash will be stored as null. JSON
          encoder still allows to use same structure in the leaves, in
          this case content will be duplicate.
          I.e.:
             xI := {1, NIL}
             xI[2] := xI
             ? hb_jsonEncode( xI )  // [1,null]
          but:
             xI := {1, .T.}
             xI := {2, xI, xI}
             ? hb_jsonEncode( xI )  // [2,[1,true],[1,true]]
 */

typedef struct
{
   char * pBuffer;
   char * pHead;
   HB_SIZE nAlloc;
   void ** pId;
   HB_SIZE nAllocId;
   BOOL fHuman;
} HB_JSON_ENCODE_CTX, * PHB_JSON_ENCODE_CTX;


#if defined( HB_OS_UNIX ) && ! defined( HB_EOL_CRLF )
static const char s_szEol[ 2 ] = { HB_CHAR_LF, 0 };
static const int  s_iEolLen      = 1;
#else
static const char s_szEol[ 3 ]   = { HB_CHAR_CR, HB_CHAR_LF, 0 };
static const int  s_iEolLen = 2;
#endif

#define INDENT_SIZE 2

static void _hb_jsonCtxAdd( PHB_JSON_ENCODE_CTX pCtx, const char * szString, HB_SIZE nLen )
{
   if( pCtx->pHead + nLen >= pCtx->pBuffer + pCtx->nAlloc )
   {
      HB_SIZE nSize = pCtx->pHead - pCtx->pBuffer;

      pCtx->nAlloc   += ( pCtx->nAlloc << 1 ) + nLen;
      pCtx->pBuffer  = ( char * ) hb_xrealloc( pCtx->pBuffer, pCtx->nAlloc );
      pCtx->pHead    = pCtx->pBuffer + nSize;
   }
   if( szString )
   {
      hb_xmemcpy( pCtx->pHead, szString, ( size_t ) nLen );
      pCtx->pHead += nLen;
   }
}

static void _hb_jsonCtxAddIndent( PHB_JSON_ENCODE_CTX pCtx, HB_SIZE nCount )
{
   if( nCount <= 0 )
      return;

   if( pCtx->pHead + nCount >= pCtx->pBuffer + pCtx->nAlloc )
   {
      HB_SIZE nSize = pCtx->pHead - pCtx->pBuffer;

      pCtx->nAlloc   += ( pCtx->nAlloc << 1 ) + nCount;
      pCtx->pBuffer  = ( char * ) hb_xrealloc( pCtx->pBuffer, pCtx->nAlloc );
      pCtx->pHead    = pCtx->pBuffer + nSize;
   }
   hb_xmemset( pCtx->pHead, ' ', ( size_t ) nCount );
   pCtx->pHead += nCount;
}

static void _hb_jsonEncode( PHB_ITEM pValue, PHB_JSON_ENCODE_CTX pCtx, HB_SIZE nLevel )
{
   if( nLevel >= pCtx->nAllocId )
   {
      pCtx->nAllocId += 8;
      pCtx->pId      = ( void ** ) hb_xrealloc( pCtx->pId, sizeof( void * ) * pCtx->nAllocId );
   }

   /* Protection against recursive structures */
   if( HB_IS_ARRAY( pValue ) || HB_IS_HASH( pValue ) )
   {
      void *   id = HB_IS_HASH( pValue ) ? hb_hashId( pValue ) : hb_arrayId( pValue );
      ULONG    nIndex;

      for( nIndex = 0; nIndex < nLevel; nIndex++ )
      {
         if( pCtx->pId[ nIndex ] == id )
         {
            _hb_jsonCtxAdd( pCtx, "null", 4 );
            return;
         }
      }
      pCtx->pId[ nLevel ] = id;
   }

   if( HB_IS_DATE( pValue ) )
   {
      char szBuffer[ 10 ];
      hb_itemGetDS( pValue, szBuffer + 1 );
      szBuffer[ 0 ]  = '\"';
      szBuffer[ 9 ]  = '\"';
      _hb_jsonCtxAdd( pCtx, szBuffer, 10 );
   }
   else if( HB_IS_TIMEFLAG( pValue ) )
   {
      char szBuffer[ 19 ];
      hb_datetimeDecStr( szBuffer + 1, hb_itemGetDL( pValue ), hb_itemGetT( pValue ) );
      szBuffer[ 0 ]  = '\"';
      szBuffer[ 18 ] = '\"';
      _hb_jsonCtxAdd( pCtx, szBuffer, 19 );
   }
   else if( HB_IS_STRING( pValue ) )
   {
      const char *   szString = hb_itemGetCPtr( pValue );
      HB_SIZE        nPos, nPos2, nLen = hb_itemGetCLen( pValue );

      _hb_jsonCtxAdd( pCtx, "\"", 1 );

      nPos = 0;
      while( nPos < nLen )
      {
         nPos2 = nPos;
         while( *( ( const unsigned char * ) szString + nPos2 ) >= ' ' &&
                szString[ nPos2 ] != '\\' && szString[ nPos2 ] != '\"' )
            nPos2++;
         if( nPos2 > nPos )
         {
            _hb_jsonCtxAdd( pCtx, szString + nPos, nPos2 - nPos );
            nPos = nPos2;
            continue;
         }

         switch( szString[ nPos ] )
         {
            case '\\':
               _hb_jsonCtxAdd( pCtx, "\\\\", 2 );
               break;
            case '\"':
               _hb_jsonCtxAdd( pCtx, "\\\"", 2 );
               break;
            case '\b':
               _hb_jsonCtxAdd( pCtx, "\\b", 2 );
               break;
            case '\f':
               _hb_jsonCtxAdd( pCtx, "\\f", 2 );
               break;
            case '\n':
               _hb_jsonCtxAdd( pCtx, "\\n", 2 );
               break;
            case '\r':
               _hb_jsonCtxAdd( pCtx, "\\r", 2 );
               break;
            case '\t':
               _hb_jsonCtxAdd( pCtx, "\\t", 2 );
               break;
            default:
            {
               char buf[ 8 ];
               hb_snprintf( buf, sizeof( buf ), "\\u00%02X", ( unsigned char ) szString[ nPos ] );
               _hb_jsonCtxAdd( pCtx, buf, 6 );
               break;
            }
         }
         nPos++;
      }
      _hb_jsonCtxAdd( pCtx, "\"", 1 );
   }
   else if( HB_IS_NUMINT( pValue ) )
   {
      char buf[ 32 ];

      hb_snprintf( buf, sizeof( buf ), "%" PFHL "d", hb_itemGetNInt( pValue ) );
      _hb_jsonCtxAdd( pCtx, buf, strlen( buf ) );
   }
   else if( HB_IS_NUMERIC( pValue ) )
   {
      char     buf[ 64 ];
      int      iDec;
      double   dblValue = hb_itemGetNDDec( pValue, &iDec );

      hb_snprintf( buf, sizeof( buf ), "%.*f", iDec, dblValue );
      _hb_jsonCtxAdd( pCtx, buf, strlen( buf ) );
   }
   else if( HB_IS_NIL( pValue ) )
   {
      _hb_jsonCtxAdd( pCtx, "null", 4 );
   }
   else if( HB_IS_LOGICAL( pValue ) )
   {
      if( hb_itemGetL( pValue ) )
         _hb_jsonCtxAdd( pCtx, "true", 4 );
      else
         _hb_jsonCtxAdd( pCtx, "false", 5 );

   }
   else if( HB_IS_ARRAY( pValue ) )
   {
      HB_SIZE nLen = hb_itemSize( pValue );

      if( nLen )
      {
         ULONG nIndex;

         if( pCtx->fHuman )
            _hb_jsonCtxAddIndent( pCtx, nLevel * INDENT_SIZE );

         _hb_jsonCtxAdd( pCtx, "[", 1 );

         for( nIndex = 1; nIndex <= nLen; nIndex++ )
         {
            PHB_ITEM pItem = hb_arrayGetItemPtr( pValue, nIndex );

            if( nIndex > 1 )
               _hb_jsonCtxAdd( pCtx, ",", 1 );

            if( pCtx->fHuman )
               _hb_jsonCtxAdd( pCtx, s_szEol, s_iEolLen );

            if( pCtx->fHuman &&
                ! ( ( HB_IS_ARRAY( pItem ) || HB_IS_HASH( pItem ) ) &&
                    hb_itemSize( pItem ) > 0 ) )
               _hb_jsonCtxAddIndent( pCtx, ( nLevel + 1 ) * INDENT_SIZE );

            _hb_jsonEncode( pItem, pCtx, nLevel + 1 );
         }
         if( pCtx->fHuman )
         {
            _hb_jsonCtxAdd( pCtx, s_szEol, s_iEolLen );
            _hb_jsonCtxAddIndent( pCtx, nLevel * INDENT_SIZE );
         }
         _hb_jsonCtxAdd( pCtx, "]", 1 );
      }
      else
         _hb_jsonCtxAdd( pCtx, "[]", 2 );
   }
   else if( HB_IS_HASH( pValue ) )
   {
      HB_SIZE nLen = hb_hashLen( pValue );

      if( nLen )
      {
         ULONG nIndex;

         if( pCtx->fHuman )
            _hb_jsonCtxAddIndent( pCtx, nLevel * INDENT_SIZE );

         _hb_jsonCtxAdd( pCtx, "{", 1 );

         for( nIndex = 1; nIndex <= nLen; nIndex++ )
         {
            PHB_ITEM pKey = hb_hashGetKeyAt( pValue, nIndex );

            if( HB_IS_STRING( pKey ) )
            {
               PHB_ITEM pItem = hb_hashGetValueAt( pValue, nIndex );
               if( nIndex > 1 )
                  _hb_jsonCtxAdd( pCtx, ",", 1 );

               if( pCtx->fHuman )
               {
                  _hb_jsonCtxAdd( pCtx, s_szEol, s_iEolLen );
                  _hb_jsonCtxAddIndent( pCtx, ( nLevel + 1 ) * INDENT_SIZE );
               }
               _hb_jsonEncode( pKey, pCtx, nLevel + 1 );

               if( pCtx->fHuman )
               {
                  _hb_jsonCtxAdd( pCtx, " : ", 3 );
                  if( ( HB_IS_ARRAY( pItem ) || HB_IS_HASH( pItem ) ) && hb_itemSize( pItem ) > 0 )
                     _hb_jsonCtxAdd( pCtx, s_szEol, s_iEolLen );
               }
               else
                  _hb_jsonCtxAdd( pCtx, ":", 1 );

               _hb_jsonEncode( pItem, pCtx, nLevel + 1 );
            }
         }
         if( pCtx->fHuman )
         {
            _hb_jsonCtxAdd( pCtx, s_szEol, s_iEolLen );
            _hb_jsonCtxAddIndent( pCtx, nLevel * INDENT_SIZE );
         }
         _hb_jsonCtxAdd( pCtx, "}", 1 );
      }
      else
         _hb_jsonCtxAdd( pCtx, "{}", 2 );
   }
   else
   {
      /* All unsupported types are replacd by null */
      _hb_jsonCtxAdd( pCtx, "null", 4 );
   }
}


static const char * _skipws( const char * szSource )
{
   while( *szSource == ' ' || *szSource == '\t' || *szSource == '\n' || *szSource == '\r' )
      szSource++;
   return szSource;
}

static const char * _hb_jsonDecode( const char * szSource, PHB_ITEM pValue )
{
   if( *szSource == '\"' )
   {
      char *   szDest, * szHead;
      ULONG    nAlloc = 16;

      szHead = szDest = ( char * ) hb_xgrab( nAlloc );
      szSource++;
      while( *szSource != '\"' )
      {
         if( szHead + 6 >= szDest + nAlloc )
         {
            HB_SIZE nLen = szHead - szDest;
            nAlloc   += nAlloc << 1;
            szDest   = ( char * ) hb_xrealloc( szDest, nAlloc );
            szHead   = szDest + nLen;
         }
         if( *szSource == '\\' )
         {
            szSource++;
            switch( *szSource )
            {
               case '\"':
                  *szHead++   = '\"';
                  break;
               case '\\':
                  *szHead++   = '\\';
                  break;
               case '/':
                  *szHead++   = '/';
                  break;
               case 'b':
                  *szHead++   = '\b';
                  break;
               case 'f':
                  *szHead++   = '\f';
                  break;
               case 'n':
                  *szHead++   = '\n';
                  break;
               case 'r':
                  *szHead++   = '\r';
                  break;
               case 't':
                  *szHead++   = '\t';
                  break;
               case 'u':
               {
                  int i, val = 0;

                  szSource++;
                  for( i = 0; i < 4 && ( ( *szSource >= '0' && *szSource <= '9' ) ||
                                         ( *szSource >= 'A' && *szSource <= 'F' ) ||
                                         ( *szSource >= 'a' && *szSource <= 'f' ) ); i++ )
                  {
                     if( szSource[ i ] <= '9' )
                        val = ( val << 4 ) + szSource[ i ] - '0';
                     else if( *szSource <= 'F' )
                        val = ( val << 4 ) + szSource[ i ] - 'A' + 10;
                     else if( *szSource <= 'f' )
                        val = ( val << 4 ) + szSource[ i ] - 'a' + 10;
                  }
                  if( i < 4 )
                  {
                     hb_xfree( szDest );
                     return NULL;
                  }
                  *szHead++   = hb_cdpGetChar( hb_cdppage(), TRUE, ( wchar_t ) val );
                  szSource    += 3;
                  break;
               }
               default:
                  hb_xfree( szDest );
                  return NULL;
            }
            szSource++;
         }
         else if( *( const unsigned char * ) szSource >= ' ' )
            *szHead++ = *szSource++;
         else
         {
            hb_xfree( szDest );
            return NULL;
         }
      }
      hb_itemPutCL( pValue, szDest, szHead - szDest );
      hb_xfree( szDest );
      return szSource + 1;
   }
   else if( *szSource == '-' || ( *szSource >= '0' && *szSource <= '9' ) )
   {
      /* NOTE: this function is much less strict to number format than
               JSON syntax definition. This is allowed behaviour [Mindaugas] */
      HB_LONG  nValue   = 0;
      double   dblValue = 0;
      BOOL     fNeg, fDbl = FALSE;
      int iDec = 0;

      fNeg = *szSource == '-';
      if( fNeg )
         szSource++;

      while( *szSource >= '0' && *szSource <= '9' )
      {
         nValue = nValue * 10 + *szSource - '0';
         szSource++;
      }
      if( *szSource == '.' )
      {
         double mult = 1;

         dblValue = ( double ) nValue;
         fDbl     = TRUE;
         szSource++;
         while( *szSource >= '0' && *szSource <= '9' )
         {
            mult     /= 10;
            dblValue += ( ( double ) ( *szSource - '0' ) ) * mult;
            szSource++;
            iDec++;
         }
      }
      if( *szSource == 'e' || *szSource == 'E' )
      {
         BOOL  fNegExp;
         int   iExp = 0;

         szSource++;
         fNegExp = *szSource == '-';
         if( fNegExp )
            szSource++;

         while( *szSource >= '0' && *szSource <= '9' )
         {
            iExp = iExp * 10 + *szSource - '0';
            szSource++;
         }
         if( ! fDbl )
         {
            dblValue = ( double ) nValue;
            fDbl     = TRUE;
         }
         dblValue *= pow( 10.0, ( double ) ( fNegExp ? -iExp : iExp ) );
         if ( fNegExp )
            iDec += iExp;
      }

      if( fDbl )
         hb_itemPutNDDec( pValue, fNeg ? -dblValue : dblValue, iDec );
      else
         hb_itemPutNInt( pValue, fNeg ? -nValue : nValue );
      return szSource;
   }
   else if( ! strncmp( szSource, "null", 4 ) )
   {
      hb_itemClear( pValue );
      return szSource + 4;
   }
   else if( ! strncmp( szSource, "true", 4 ) )
   {
      hb_itemPutL( pValue, TRUE );
      return szSource + 4;
   }
   else if( ! strncmp( szSource, "false", 5 ) )
   {
      hb_itemPutL( pValue, FALSE );
      return szSource + 5;
   }
   else if( *szSource == '[' )
   {
      hb_arrayNew( pValue, 0 );
      szSource = _skipws( szSource + 1 );
      if( *szSource != ']' )
      {
         PHB_ITEM pItem = hb_itemNew( NULL );

         for(;; )
         {
            szSource = _hb_jsonDecode( szSource, pItem );
            if( ! szSource )
            {
               hb_itemRelease( pItem );
               return NULL;
            }
            hb_arrayAddForward( pValue, pItem );

            szSource = _skipws( szSource );
            if( *szSource == ',' )
            {
               szSource = _skipws( szSource + 1 );
               continue;
            }
            else if( *szSource == ']' )
               break;
            else
            {
               hb_itemRelease( pItem );
               return NULL;
            }
         }
         hb_itemRelease( pItem );
      }
      return szSource + 1;
   }
   else if( *szSource == '{' )
   {
      hb_hashNew( pValue );
      szSource = _skipws( szSource + 1 );
      if( *szSource != '}' )
      {
         PHB_ITEM pItemKey    = hb_itemNew( NULL );
         PHB_ITEM pItemValue  = hb_itemNew( NULL );

         for(;; )
         {
            if( ( szSource = _hb_jsonDecode( szSource, pItemKey ) ) == NULL ||
                ! HB_IS_STRING( pItemKey ) ||
                *( szSource = _skipws( szSource ) ) != ':' ||
                ( szSource = _hb_jsonDecode( _skipws( szSource + 1 ), pItemValue ) ) == NULL )
            {
               /* Do we need to check if key does not exist yet? */
               hb_itemRelease( pItemKey );
               hb_itemRelease( pItemValue );
               return NULL;
            }

            hb_hashAdd( pValue, ULONG_MAX, pItemKey, pItemValue );
            szSource = _skipws( szSource );
            if( *szSource == ',' )
            {
               szSource = _skipws( szSource + 1 );
               continue;
            }
            else if( *szSource == '}' )
               break;
            else
            {
               hb_itemRelease( pItemKey );
               hb_itemRelease( pItemValue );
               return NULL;
            }
         }
         hb_itemRelease( pItemKey );
         hb_itemRelease( pItemValue );
      }
      return szSource + 1;
   }
   return NULL;
}

/* C level API functions */

char * hb_jsonEncode( PHB_ITEM pValue, HB_SIZE * pnLen, BOOL fHuman )
{
   PHB_JSON_ENCODE_CTX  pCtx;
   char *               szRet;
   HB_SIZE              nLen;

   pCtx           = ( PHB_JSON_ENCODE_CTX ) hb_xgrab( sizeof( HB_JSON_ENCODE_CTX ) );
   pCtx->nAlloc   = 16;
   pCtx->pHead    = pCtx->pBuffer = ( char * ) hb_xgrab( pCtx->nAlloc );
   pCtx->nAllocId = 8;
   pCtx->pId      = ( void ** ) hb_xgrab( sizeof( void * ) * pCtx->nAllocId );
   pCtx->fHuman   = fHuman;

   _hb_jsonEncode( pValue, pCtx, 0 );

   nLen           = pCtx->pHead - pCtx->pBuffer;
   szRet          = ( char * ) hb_xrealloc( pCtx->pBuffer, nLen + 1 );
   szRet[ nLen ]  = '\0';
   hb_xfree( pCtx->pId );
   hb_xfree( pCtx );
   if( pnLen )
      *pnLen = nLen;
   return szRet;
}

HB_SIZE hb_jsonDecode( const char * szSource, PHB_ITEM pValue )
{
   PHB_ITEM       pItem = pValue ? pValue : hb_itemNew( NULL );
   const char *   sz;

   sz = szSource ? _hb_jsonDecode( _skipws( szSource ), pItem ) : NULL;
   if( ! pValue )
      hb_itemRelease( pItem );
   if( sz )
      return sz - szSource;
   return 0;
}


/* Harbour level API functions */

HB_FUNC( HB_JSONENCODE )
{
   PHB_ITEM pItem = hb_param( 1, HB_IT_ANY );

   if( pItem )
   {
      HB_SIZE  nLen;
      char *   szRet = hb_jsonEncode( pItem, &nLen, hb_parl( 2 ) );

      hb_retclen_buffer( szRet, nLen );
   }
}

HB_FUNC( HB_JSONDECODE )
{
   PHB_ITEM pItem = hb_itemNew( NULL );
   PHB_ITEM pRef  = hb_param( 2, HB_IT_BYREF );

   hb_retnl( ( LONG ) hb_jsonDecode( hb_parc( 1 ), pItem ) );

   if( pRef )
      hb_itemCopy( pRef, pItem );

   hb_itemRelease( pItem );
}
