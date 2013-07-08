/*
 * $Id: dbfntx1.c 9723 2012-10-02 09:18:08Z andijahja $
 */

/*
 * Harbour Project source code:
 * DBFNTX RDD
 *
 * Copyright 1999 Bruno Cantero <bruno@issnet.net>
 * www - http://www.harbour-project.org
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
/*
 * The following functions are added by
 *       Alexander Kresin <alex@belacy.belgorod.su>
 *
 * commonError()
 * hb_IncString()
 * ntxNumToStr()
 * checkLogicalExpr()
 * hb__ntxTagKeyCount()
 * hb_ntxInTopScope()
 * hb_ntxInBottomScope()
 * hb_ntxTagKeyNo()
 * hb_ntxTagKeyCount()
 * hb_ntxClearScope()
 * hb_ntxGoEof()
 * hb_ntxGetKeyType()
 * hb_ntxTagKeyFind()
 * hb_ntxPageKeySearch()
 * hb_ntxTagFindCurrentKey()
 * hb_ntxIsRecBad()
 * hb_ntxPageFindCurrentKey()
 * hb_ntxGetCurrentKey()
 * hb_ntxTagGoToNextKey()
 * hb_ntxTagGoToPrevKey()
 * hb_ntxTagGoToTopKey()
 * hb_ntxTagGoToBottomKey()
 * hb_ntxTagKeyGoTo()
 * hb_ntxPageRelease()
 * hb_ntxKeysMove()
 * hb_ntxPageSplit()
 * hb_ntxPageJoin()
 * hb_ntxPageBalance()
 * hb_ntxTagBalance()
 * hb_ntxPageKeyDel()
 * hb_ntxTagKeyAdd()
 * hb_ntxSwapPageSave()
 * hb_ntxKeysSort()
 * hb_ntxSortKeyAdd()
 * hb_ntxSortKeyEnd()
 * hb_ntxWritePage()
 * hb_ntxRootPage()
 * hb_ntxGetSortedKey()
 * hb_ntxBufferSave()
 * hb_ntxReadBuf()
 * hb_ntxPageFind()
 * ntxFindIndex()
 * hb_ntxOrdKeyAdd()
 * hb_ntxOrdKeyDel()
 * ntxGoBottom()
 * ntxGoTo()
 * ntxGoTop()
 * ntxSeek()
 * ntxSkipRaw()
 * ntxGoCold()
 * ntxGoHot()
 * ntxSysName()
 * ntxPack()
 * ntxZap()
 * ntxClearScope()
 * ntxScopeInfo()
 * ntxOrderListAdd()
 * ntxOrderListClear()
 * ntxOrderListFocus()
 * ntxOrderListRebuild()
 * ntxSetScope()
 */

/*
 * Copyright 2005 Przemyslaw Czerpak <druzus@priv.onet.pl>
 * in practice most of the code rewritten
 */

/* #define HB_NTX_NOMULTITAG */

/* #define HB_NTX_EXTERNAL_PAGEBUFFER */

#define HB_NTX_STRONG_BALANCE

/*
   #define HB_NTX_DEBUG
   #define HB_NTX_DEBUG_EXT
   #define HB_NTX_DEBUG_DISP
 */

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbinit.h"
#include "hbapierr.h"
#include "hbapilng.h"
#include "hbvm.h"
#include "hbset.h"
#include "hbmath.h"
#include "hbrddntx.h"
#include "rddsys.ch"
#include "hbregex.h"
#include "hbdate.h"
#ifndef HB_CDP_SUPPORT_OFF
   #include "hbapicdp.h"
#endif

#ifdef HB_NTX_DEBUG_DISP
static ULONG      s_rdNO   = 0;
static ULONG      s_wrNO   = 0;
#endif

static RDDFUNCS   ntxSuper;
static USHORT     s_uiRddId;


#define hb_ntxKeyFree( K )             hb_xfree( K )
#define hb_ntxFileOffset( I, B )       ( ( B ) << ( ( I )->LargeFile ? NTXBLOCKBITS : 0 ) )
#define hb_ntxPageBuffer( p )          ( ( p )->buffer )

/*
 * The helper functions (endian dependent) - on big endian machines
 * or RISC with strict alignment it's much better to use functions
 * then macros to inform compiler that can count complex parameters
 * only once.
 * On other machines it should not cause noticeable differences because
 * most of modern C compilers auto inline small functions
 */
#if defined( HB_LITTLE_ENDIAN ) && ! defined( HB_STRICT_ALIGNMENT )

#define hb_ntxGetKeyCount( p )         HB_GET_LE_UINT16( hb_ntxPageBuffer( p ) )
#define hb_ntxSetKeyCount( p, n )      HB_PUT_LE_UINT16( hb_ntxPageBuffer( p ), ( n ) )

#define hb_ntxGetKeyOffset( p, n )     HB_GET_LE_UINT16( hb_ntxPageBuffer( p ) + 2 + ( ( n ) << 1 ) )
#define hb_ntxGetKeyPtr( p, n )        ( hb_ntxPageBuffer( p ) + hb_ntxGetKeyOffset( p, n ) )
#define hb_ntxGetKeyPage( p, n )       HB_GET_LE_UINT32( hb_ntxGetKeyPtr( p, n ) )
#define hb_ntxGetKeyRec( p, n )        HB_GET_LE_UINT32( hb_ntxGetKeyPtr( p, n ) + 4 )
#define hb_ntxGetKeyVal( p, n )        ( hb_ntxGetKeyPtr( p, n ) + 8 )

#define hb_ntxSetKeyOffset( p, n, u )  HB_PUT_LE_UINT16( hb_ntxPageBuffer( p ) + 2 + ( ( n ) << 1 ), u )
#define hb_ntxSetKeyPage( p, n, l )    HB_PUT_LE_UINT32( hb_ntxGetKeyPtr( p, n ), l )
#define hb_ntxSetKeyRec( p, n, l )     HB_PUT_LE_UINT32( hb_ntxGetKeyPtr( p, n ) + 4, l )

#else

static USHORT hb_ntxGetKeyCount( LPPAGEINFO pPage )
{
   char * ptr = hb_ntxPageBuffer( pPage );

   return HB_GET_LE_UINT16( ptr );
}

static void hb_ntxSetKeyCount( LPPAGEINFO pPage, USHORT uiKeys )
{
   char * ptr = hb_ntxPageBuffer( pPage );

   HB_PUT_LE_UINT16( ptr, uiKeys );
}

static USHORT hb_ntxGetKeyOffset( LPPAGEINFO pPage, SHORT iKey )
{
   char * ptr = hb_ntxPageBuffer( pPage ) + 2 + ( iKey << 1 );

   return HB_GET_LE_UINT16( ptr );
}

static void hb_ntxSetKeyOffset( LPPAGEINFO pPage, SHORT iKey, USHORT uiOffset )
{
   char * ptr = hb_ntxPageBuffer( pPage ) + 2 + ( iKey << 1 );

   HB_PUT_LE_UINT16( ptr, uiOffset );
}

static char * hb_ntxGetKeyPtr( LPPAGEINFO pPage, SHORT iKey )
{
   return hb_ntxPageBuffer( pPage ) + hb_ntxGetKeyOffset( pPage, iKey );
}

static ULONG hb_ntxGetKeyPage( LPPAGEINFO pPage, SHORT iKey )
{
   char * ptr = hb_ntxGetKeyPtr( pPage, iKey );

   return HB_GET_LE_UINT32( ptr );
}

static void hb_ntxSetKeyPage( LPPAGEINFO pPage, SHORT iKey, ULONG ulPage )
{
   char * ptr = hb_ntxGetKeyPtr( pPage, iKey );

   HB_PUT_LE_UINT32( ptr, ulPage );
}

static char * hb_ntxGetKeyVal( LPPAGEINFO pPage, SHORT iKey )
{
   return hb_ntxGetKeyPtr( pPage, iKey ) + 8;
}

static void hb_ntxSetKeyRec( LPPAGEINFO pPage, SHORT iKey, ULONG ulRec )
{
   char * ptr = hb_ntxGetKeyPtr( pPage, iKey ) + 4;

   HB_PUT_LE_UINT32( ptr, ulRec );
}

static ULONG hb_ntxGetKeyRec( LPPAGEINFO pPage, SHORT iKey )
{
   char * ptr = hb_ntxGetKeyPtr( pPage, iKey ) + 4;

   return HB_GET_LE_UINT32( ptr );
}

#endif

/*
 * generate Run-Time error
 */
static HB_ERRCODE hb_ntxErrorRT( NTXAREAP pArea, USHORT uiGenCode, USHORT uiSubCode,
                                 const char * szFileName, USHORT uiOsCode,
                                 USHORT uiFlags, PHB_ITEM * pErrorPtr )
{
   PHB_ITEM    pError;
   HB_ERRCODE  iRet = HB_FAILURE;

   if( hb_vmRequestQuery() == 0 )
   {
      if( pErrorPtr )
      {
         if( ! *pErrorPtr )
            *pErrorPtr = hb_errNew();
         pError = *pErrorPtr;
      }
      else
         pError = hb_errNew();
      hb_errPutGenCode( pError, uiGenCode );
      hb_errPutSubCode( pError, uiSubCode );
      hb_errPutOsCode( pError, uiOsCode );
      hb_errPutDescription( pError, hb_langDGetErrorDesc( uiGenCode ) );
      if( szFileName )
         hb_errPutFileName( pError, szFileName );
      if( uiFlags )
         hb_errPutFlags( pError, uiFlags );
      iRet = SELF_ERROR( ( AREAP ) pArea, pError );
      if( ! pErrorPtr )
         hb_errRelease( pError );
   }
   return iRet;
}

/*
 * convert numeric item into NTX key value
 */
static char * hb_ntxNumToStr( PHB_ITEM pItem, char * szBuffer, USHORT length, USHORT dec )
{
   char * ptr = szBuffer;

   hb_itemStrBuf( szBuffer, pItem, length, dec );

   while( *ptr == ' ' )
      *ptr++ = '0';

   if( *ptr == '-' )
   {
      *ptr = '0';
      for( ptr = &szBuffer[ 0 ]; *ptr; ptr++ )
      {
         if( *ptr >= '0' && *ptr <= '9' )
            *ptr = ( char ) ( '0' - ( *ptr - '0' ) - 4 );
         /*
          * I intentionally used the above formula to avoid problems on
          * non ASCII machines though many of other xHarbour codes is
          * hard coded to ASCII values and should be fixed. Druzus.
          */
      }
   }

   return szBuffer;
}

/*
 * convert numeric NTX key value into item
 */
static PHB_ITEM hb_ntxStrToNum( PHB_ITEM pItem, char * szKeyVal, USHORT length, USHORT dec )
{
   char     szBuffer[ NTX_MAX_KEY + 1 ];
   char *   ptr = szKeyVal, * ptr2, c;
   int      iLen, iDec;
   HB_LONG  lValue;
   double   dValue;

   HB_SYMBOL_UNUSED( dec );

   if( *ptr == '0' - 4 ) /* negative number */
   {
      ptr2 = szBuffer;
      while( ( c = *ptr++ ) != 0 )
      {
         if( c != '.' )
            c = '0' - ( c - '0' + 4 );
         *ptr2++ = c;
      }
      szBuffer[ 0 ]  = '-';
      *ptr2          = '\0';
      ptr            = szBuffer;
   }
   if( hb_valStrnToNum( ptr, length, &lValue, &dValue, &iDec, &iLen ) )
      return hb_itemPutNDLen( pItem, dValue, iLen, iDec );
   else
      return hb_itemPutNIntLen( pItem, lValue, length );
}

/*
 * create new index key
 */
static LPKEYINFO hb_ntxKeyNew( LPKEYINFO pKeyFrom, int keylen )
{
   LPKEYINFO pKey;

   pKey = ( LPKEYINFO ) hb_xgrab( sizeof( KEYINFO ) + keylen );
   if( pKeyFrom )
   {
      HB_MEMCPY( pKey->key, pKeyFrom->key, keylen + 1 );
      pKey->Tag   = pKeyFrom->Tag;
      pKey->Xtra  = pKeyFrom->Xtra;
   }
   else
   {
      pKey->key[ keylen ]  = '\0';
      pKey->Tag            = pKey->Xtra = 0;
   }
   return pKey;
}

/*
 * copy index key, if dst is null create new dst key else destroy dst
 */
static LPKEYINFO hb_ntxKeyCopy( LPKEYINFO pKeyDest, LPKEYINFO pKey, int keylen )
{
   if( ! pKeyDest )
      pKeyDest = hb_ntxKeyNew( NULL, keylen );

   HB_MEMCPY( pKeyDest->key, pKey->key, keylen + 1 );
   pKeyDest->Tag  = pKey->Tag;
   pKeyDest->Xtra = pKey->Xtra;

   return pKeyDest;
}

/*
 * get ntx key type for given item
 */
static BYTE hb_ntxItemType( PHB_ITEM pItem )
{
   switch( hb_itemType( pItem ) )
   {
      case HB_IT_STRING:
      case HB_IT_STRING | HB_IT_MEMO:
         return 'C';

      case HB_IT_INTEGER:
      case HB_IT_LONG:
      case HB_IT_DOUBLE:
         return 'N';

      case HB_IT_DATE:
         return 'D';

      case HB_IT_TIMEFLAG:
         return 'T';

      case HB_IT_LOGICAL:
         return 'L';

      default:
         return 'U';
   }
}

/*
 * convert key type to comparable type
 */
static BYTE hb_ntxItemTypeCmp( BYTE bType )
{
   return bType == 'T' ? 'D' : bType;
}

/*
 * store Item in index key
 * TODO: uiType check and generate RT error if necessary
 *       probably not here or we will have to add parameter
 *       for scope key evaluation
 */
static LPKEYINFO hb_ntxKeyPutItem( LPKEYINFO pKey, PHB_ITEM pItem, ULONG ulRecNo,
                                   LPTAGINFO pTag, BOOL fTrans, USHORT * puiLen )
{
   ULONG len;

   if( ! pKey )
      pKey = hb_ntxKeyNew( NULL, pTag->KeyLength );

   if( puiLen )
      *puiLen = pTag->KeyLength;

   switch( hb_ntxItemType( pItem ) )
   {
      case 'C':
#ifndef HB_CDP_SUPPORT_OFF
         if( fTrans )
         {
            len = pTag->KeyLength;
            hb_cdpnDup2( hb_itemGetCPtr( pItem ), hb_itemGetCLen( pItem ),
                         pKey->key, ( HB_SIZE * ) &len,
                         hb_cdppage(), pTag->Owner->Owner->dbfarea.area.cdPage );
         }
         else
#else
         HB_SYMBOL_UNUSED( fTrans );
#endif
         {
            len = ( ULONG ) hb_itemGetCLen( pItem );
            if( len > ( ULONG ) pTag->KeyLength )
               len = pTag->KeyLength;
            HB_MEMCPY( pKey->key, hb_itemGetCPtr( pItem ), ( size_t ) len );
         }
         if( len < ( ULONG ) pTag->KeyLength )
         {
            memset( pKey->key + len, ' ', ( size_t ) ( pTag->KeyLength - len ) );
            if( puiLen )
               *puiLen = ( USHORT ) len;
         }
         pKey->key[ pTag->KeyLength ] = '\0';
         break;
      case 'N':
         hb_ntxNumToStr( pItem, pKey->key, pTag->KeyLength, pTag->KeyDec );
         break;
      case 'D':
         hb_itemGetDS( pItem, pKey->key );
         break;
      case 'L':
         pKey->key[ 0 ]                = ( hb_itemGetL( pItem ) ? 'T' : 'F' );
         if( pTag->KeyLength > 1 )
            memset( pKey->key + 1, '\0', pTag->KeyLength - 1 );
         pKey->key[ pTag->KeyLength ]  = '\0';
         break;
      default:
         memset( pKey->key, '\0', pTag->KeyLength + 1 );
   }
   pKey->Xtra  = ulRecNo;
   pKey->Tag   = 0;

   return pKey;
}

/*
 * get Item from index key
 */
static PHB_ITEM hb_ntxKeyGetItem( PHB_ITEM pItem, LPKEYINFO pKey,
                                  LPTAGINFO pTag, BOOL fTrans )
{
   if( pKey )
   {
      switch( pTag->KeyType )
      {
         case 'C':
#ifndef HB_CDP_SUPPORT_OFF
            if( fTrans )
            {
               ULONG    ulLen    = pTag->KeyLength;
               char *   pszVal   = hb_cdpnDup( pKey->key, ( HB_SIZE * ) &ulLen,
                                               pTag->Owner->Owner->dbfarea.area.cdPage, hb_cdppage() );
               pItem = hb_itemPutCLPtr( pItem, pszVal, ulLen );
            }
            else
#else
            HB_SYMBOL_UNUSED( fTrans );
#endif
            {
               pItem = hb_itemPutCL( pItem, pKey->key, pTag->KeyLength );
            }
            break;
         case 'N':
            pItem = hb_ntxStrToNum( pItem, pKey->key, pTag->KeyLength, pTag->KeyDec );
            break;
         case 'D':
            pItem = hb_itemPutDS( pItem, pKey->key );
            break;
         case 'L':
            pItem = hb_itemPutL( pItem, pKey->key[ 0 ] == 'T' );
            break;
         default:
            if( pItem )
               hb_itemClear( pItem );
            else
               pItem = hb_itemNew( NULL );
      }
   }
   else if( pItem )
      hb_itemClear( pItem );
   else
      pItem = hb_itemNew( NULL );

   return pItem;
}

/*
 * evaluate conditional expression and return the logical result
 */
static BOOL hb_ntxEvalCond( NTXAREAP pArea, PHB_ITEM pCondItem, BOOL fSetWA )
{
   int   iCurrArea = 0;
   BOOL  fRet;

   if( fSetWA )
   {
      iCurrArea = hb_rddGetCurrentWorkAreaNumber();
      if( iCurrArea != pArea->dbfarea.area.uiArea )
         hb_rddSelectWorkAreaNumber( pArea->dbfarea.area.uiArea );
      else
         iCurrArea = 0;
   }

   fRet = hb_itemGetL( hb_vmEvalBlockOrMacro( pCondItem ) );

   if( iCurrArea )
      hb_rddSelectWorkAreaNumber( iCurrArea );

   return fRet;
}

/*
 * evaluate seek/skip block: {|key, rec| ... }
 */
static BOOL hb_ntxEvalSeekCond( LPTAGINFO pTag, PHB_ITEM pCondItem )
{
   BOOL     fRet;
   PHB_ITEM pKeyVal, pKeyRec;

   pKeyVal  = hb_ntxKeyGetItem( NULL, pTag->CurKeyInfo, pTag, TRUE );
   pKeyRec  = hb_itemPutNInt( NULL, pTag->CurKeyInfo->Xtra );

   fRet     = hb_itemGetL( hb_vmEvalBlockV( pCondItem, 2, pKeyVal, pKeyRec ) );

   hb_itemRelease( pKeyVal );
   hb_itemRelease( pKeyRec );

   return fRet;
}

/*
 * get ITEM type of key expression
 */
static BYTE hb_ntxGetKeyType( LPTAGINFO pTag )
{
   BYTE bType;

   if( pTag->nField )
   {
      PHB_ITEM pItem = hb_itemNew( NULL );
      SELF_GETVALUE( ( AREAP ) pTag->Owner->Owner, pTag->nField, pItem );
      bType = hb_ntxItemType( pItem );
      hb_itemRelease( pItem );
   }
   else
   {
      int iCurrArea = hb_rddGetCurrentWorkAreaNumber();

      if( iCurrArea != pTag->Owner->Owner->dbfarea.area.uiArea )
         hb_rddSelectWorkAreaNumber( pTag->Owner->Owner->dbfarea.area.uiArea );
      else
         iCurrArea = 0;

      bType = hb_ntxItemType( hb_vmEvalBlockOrMacro( pTag->pKeyItem ) );

      if( iCurrArea )
         hb_rddSelectWorkAreaNumber( iCurrArea );
   }
   return bType;
}

/*
 * evaluate key expression and create new Key from the result
 */
static LPKEYINFO hb_ntxEvalKey( LPKEYINFO pKey, LPTAGINFO pTag )
{
   NTXAREAP       pArea = pTag->Owner->Owner;
   PHB_ITEM       pItem;

#ifndef HB_CDP_SUPPORT_OFF
   PHB_CODEPAGE   cdpTmp = hb_cdpSelect( pArea->dbfarea.area.cdPage );
#endif

   if( pTag->nField )
   {
      pItem = hb_itemNew( NULL );
      SELF_GETVALUE( ( AREAP ) pArea, pTag->nField, pItem );
      pKey  = hb_ntxKeyPutItem( pKey, pItem, pArea->dbfarea.ulRecNo, pTag, FALSE, NULL );
      hb_itemRelease( pItem );
   }
   else
   {
      int iCurrArea = hb_rddGetCurrentWorkAreaNumber();

      if( iCurrArea != pArea->dbfarea.area.uiArea )
         hb_rddSelectWorkAreaNumber( pArea->dbfarea.area.uiArea );
      else
         iCurrArea = 0;

      pItem = hb_vmEvalBlockOrMacro( pTag->pKeyItem );
      pKey  = hb_ntxKeyPutItem( pKey, pItem, pArea->dbfarea.ulRecNo, pTag, FALSE, NULL );

      if( iCurrArea )
         hb_rddSelectWorkAreaNumber( iCurrArea );
   }

#ifndef HB_CDP_SUPPORT_OFF
   hb_cdpSelect( cdpTmp );
#endif

   return pKey;
}

/*
 * compare two values using Tag conditions (len & type)
 */
static int hb_ntxValCompare( LPTAGINFO pTag, const char * val1, int len1,
                             const char * val2, int len2, BOOL fExact )
{
   int iLimit, iResult = 0;

   iLimit = ( len1 > len2 ) ? len2 : len1;

   if( pTag->KeyType == 'C' )
   {
      if( iLimit > 0 )
      {
#ifndef HB_CDP_SUPPORT_OFF
         if( pTag->Owner->Owner->dbfarea.area.cdPage->lSort )
            iResult = hb_cdpcmp( val1, ( ULONG ) iLimit, val2, ( ULONG ) iLimit, pTag->Owner->Owner->dbfarea.area.cdPage, 0 );
         else
#endif
         iResult = memcmp( val1, val2, iLimit );
      }

      if( iResult == 0 )
      {
         if( len1 > len2 )
            iResult = 1;
         else if( len1 < len2 && fExact )
            iResult = -1;
      }
   }
   else
   {
      if( iLimit <= 0 || ( iResult = memcmp( val1, val2, iLimit ) ) == 0 )
      {
         if( len1 > len2 )
            iResult = 1;
         else if( len1 < len2 && fExact )
            iResult = -1;
      }
   }
   return iResult;
}

/*
 * check if a given key is in top scope
 */
static BOOL hb_ntxInTopScope( LPTAGINFO pTag, char * key )
{
   PHB_NTXSCOPE pScope = pTag->fUsrDescend ? &pTag->bottom : &pTag->top;

   if( pScope->scopeKeyLen )
   {
      int i = hb_ntxValCompare( pTag, pScope->scopeKey->key, pScope->scopeKeyLen,
                                key, pTag->KeyLength, FALSE );
      return pTag->fUsrDescend ? i >= 0 : i <= 0;
   }
   else
      return TRUE;
}

/*
 * check if a given key is in bottom scope
 */
static BOOL hb_ntxInBottomScope( LPTAGINFO pTag, char * key )
{
   PHB_NTXSCOPE pScope = pTag->fUsrDescend ? &pTag->top : &pTag->bottom;

   if( pScope->scopeKeyLen )
   {
      int i = hb_ntxValCompare( pTag, pScope->scopeKey->key, pScope->scopeKeyLen,
                                key, pTag->KeyLength, FALSE );
      return pTag->fUsrDescend ? i <= 0 : i >= 0;
   }
   else
      return TRUE;
}

/*
 * check if a given key is in current scope
 */
static BOOL hb_ntxKeyInScope( LPTAGINFO pTag, LPKEYINFO pKey )
{
   return hb_ntxInTopScope( pTag, pKey->key ) &&
          hb_ntxInBottomScope( pTag, pKey->key );
}

/*
 * clear top or bottom scope
 */
static void hb_ntxTagClearScope( LPTAGINFO pTag, USHORT nScope )
{
   NTXAREAP       pArea = pTag->Owner->Owner;
   PHB_NTXSCOPE   pScope;

   /* resolve any pending scope relations first */
   if( pArea->dbfarea.lpdbPendingRel && pArea->dbfarea.lpdbPendingRel->isScoped )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( pTag->fUsrDescend )
      nScope = ( nScope == 0 ) ? 1 : 0;

   pScope = ( nScope == 0 ) ? &pTag->top : &pTag->bottom;

   if( pScope->scopeKey )
   {
      hb_ntxKeyFree( pScope->scopeKey );
      pScope->scopeKey = NULL;
   }
   if( pScope->scopeItem )
   {
      hb_itemRelease( pScope->scopeItem );
      pScope->scopeItem = NULL;
   }
   pScope->scopeKeyLen  = 0;

   pTag->keyCount       = 0;
}

/*
 * set top or bottom scope
 */
static void hb_ntxTagSetScope( LPTAGINFO pTag, USHORT nScope, PHB_ITEM pItem )
{
   NTXAREAP pArea = pTag->Owner->Owner;
   PHB_ITEM pScopeVal;

   /* resolve any pending scope relations first */
   if( pArea->dbfarea.lpdbPendingRel && pArea->dbfarea.lpdbPendingRel->isScoped )
      SELF_FORCEREL( ( AREAP ) pArea );

   pScopeVal = ( hb_itemType( pItem ) == HB_IT_BLOCK ) ?
               hb_vmEvalBlock( pItem ) : pItem;

   if( hb_ntxItemTypeCmp( pTag->KeyType ) == hb_ntxItemTypeCmp( hb_ntxItemType( pScopeVal ) ) )
   {
      PHB_NTXSCOPE   pScope;
      BOOL           fTop = ( nScope == 0 );

      if( pTag->fUsrDescend )
         fTop = ! fTop;

      pScope            = fTop ? &pTag->top : &pTag->bottom;

      pScope->scopeKey  = hb_ntxKeyPutItem( pScope->scopeKey, pScopeVal,
                                            ( fTop == pTag->AscendKey ) ? NTX_IGNORE_REC_NUM : NTX_MAX_REC_NUM,
                                            pTag, TRUE, &pScope->scopeKeyLen );

      if( pScope->scopeItem == NULL )
         pScope->scopeItem = hb_itemNew( NULL );
      hb_itemCopy( pScope->scopeItem, pItem );

      pTag->keyCount = 0;
   }
   else
   {
      hb_ntxTagClearScope( pTag, nScope );
   }
}

/*
 * get top or bottom scope item
 */
static void hb_ntxTagGetScope( LPTAGINFO pTag, USHORT nScope, PHB_ITEM pItem )
{
   NTXAREAP       pArea = pTag->Owner->Owner;
   PHB_NTXSCOPE   pScope;

   /* resolve any pending scope relations first */
   if( pArea->dbfarea.lpdbPendingRel && pArea->dbfarea.lpdbPendingRel->isScoped )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( pTag->fUsrDescend )
      nScope = ( nScope == 0 ) ? 1 : 0;

   pScope = ( nScope == 0 ) ? &pTag->top : &pTag->bottom;

   if( pScope->scopeItem )
      hb_itemCopy( pItem, pScope->scopeItem );
   else
      hb_itemClear( pItem );
}

/*
 * refresh top and bottom scope value if set as codeblock
 */
static void hb_ntxTagRefreshScope( LPTAGINFO pTag )
{
   PHB_ITEM pItem;

   /* resolve any pending scope relations first */
   if( pTag->Owner->Owner->dbfarea.lpdbPendingRel &&
       pTag->Owner->Owner->dbfarea.lpdbPendingRel->isScoped )
      SELF_FORCEREL( ( AREAP ) pTag->Owner->Owner );

   if( hb_itemType( pTag->top.scopeItem ) == HB_IT_BLOCK )
   {
      pItem                = hb_vmEvalBlock( pTag->top.scopeItem );
      pTag->top.scopeKey   = hb_ntxKeyPutItem( pTag->top.scopeKey, pItem,
                                               pTag->top.scopeKey->Xtra, pTag, TRUE, &pTag->top.scopeKeyLen );
   }
   if( hb_itemType( pTag->bottom.scopeItem ) == HB_IT_BLOCK )
   {
      pItem                   = hb_vmEvalBlock( pTag->bottom.scopeItem );
      pTag->bottom.scopeKey   = hb_ntxKeyPutItem( pTag->bottom.scopeKey, pItem,
                                                  pTag->bottom.scopeKey->Xtra, pTag, TRUE, &pTag->bottom.scopeKeyLen );
   }
}

/*
 * an interface for fast check record number in record filter
 */
static BOOL hb_ntxCheckRecordScope( NTXAREAP pArea, ULONG ulRec )
{
   LONG lRecNo = ( LONG ) ulRec;

   if( SELF_COUNTSCOPE( ( AREAP ) pArea, NULL, &lRecNo ) == HB_SUCCESS && lRecNo == 0 )
   {
      return FALSE;
   }
   return TRUE;
}

#ifdef HB_NTX_DEBUG
static void hb_ntxTagCheckBuffers( LPTAGINFO pTag )
{
   LPPAGEINFO  pPage;
   ULONG       i;

   if( ( pTag->HdrChanged || pTag->Owner->Changed ) && ! pTag->Owner->lockWrite )
      hb_errInternal( 9301, "hb_ntxTagCheckBuffers: tag modified in unlocked index", NULL, NULL );

   for( i = 0; i < pTag->Owner->ulPages; i++ )
   {
      pPage = pTag->Owner->pages[ i ];
      if( pPage->Changed && ! pTag->Owner->lockWrite )
         hb_errInternal( 9302, "hb_ntxTagCheckBuffers: page modified in unlocked index", NULL, NULL );
      if( pPage->iUsed )
         hb_errInternal( 9303, "hb_ntxTagCheckBuffers: page still allocated", NULL, NULL );
   }
}

static void hb_ntxPageCheckKeys( LPPAGEINFO pPage, LPTAGINFO pTag, int iPos, int iType )
{
   USHORT   u;
   int      i;

   for( u = 1; u < pPage->uiKeys; u++ )
   {
      i = hb_ntxValCompare( pTag,
                            hb_ntxGetKeyVal( pPage, u - 1 ), pTag->KeyLength,
                            hb_ntxGetKeyVal( pPage, u ), pTag->KeyLength, TRUE );
      if( ! pTag->AscendKey )
         i = -i;
      if( i > 0 )
      {
         printf( "\r\nuiKeys=%d(%d/%d), (%d)[%.*s]>(%d)[%.*s]", pPage->uiKeys, iPos, iType,
                 u - 1, pTag->KeyLength, hb_ntxGetKeyVal( pPage, u - 1 ),
                 u, pTag->KeyLength, hb_ntxGetKeyVal( pPage, u ) );
         fflush( stdout );
         hb_errInternal( 9304, "hb_ntxPageCheckKeys: keys sorted wrong.", NULL, NULL );
      }
   }
}
#endif

/*
 * read a given block from index file
 */
static BOOL hb_ntxBlockRead( LPNTXINDEX pIndex, ULONG ulBlock, BYTE * buffer, int iSize )
{
   if( ! pIndex->lockRead && ! pIndex->lockWrite )
      hb_errInternal( 9103, "hb_ntxBlockRead on not locked index file.", NULL, NULL );

#ifdef HB_NTX_DEBUG_DISP
   s_rdNO++;
#endif
   if( hb_fileReadAt( pIndex->DiskFile, buffer, iSize,
                      hb_ntxFileOffset( pIndex, ulBlock ) ) != ( ULONG ) iSize )
   {
      hb_ntxErrorRT( pIndex->Owner, EG_READ, EDBF_READ,
                     pIndex->IndexName, hb_fsError(), 0, NULL );
      return FALSE;
   }
   return TRUE;
}

/*
 * write a given block into index file
 */
static BOOL hb_ntxBlockWrite( LPNTXINDEX pIndex, ULONG ulBlock, BYTE * buffer, int iSize )
{
   if( ! pIndex->lockWrite )
      hb_errInternal( 9102, "hb_ntxBlockWrite on not locked index file.", NULL, NULL );

#ifdef HB_NTX_DEBUG_DISP
   s_wrNO++;
#endif
   if( hb_fileWriteAt( pIndex->DiskFile, buffer, iSize,
                       hb_ntxFileOffset( pIndex, ulBlock ) ) != ( ULONG ) iSize )
   {
      hb_ntxErrorRT( pIndex->Owner, EG_WRITE, EDBF_WRITE,
                     pIndex->IndexName, hb_fsError(), 0, NULL );
      return FALSE;
   }
   return TRUE;
}

/*
 * write a given tag page to file
 */
static BOOL hb_ntxPageSave( LPNTXINDEX pIndex, LPPAGEINFO pPage )
{
   hb_ntxSetKeyCount( pPage, pPage->uiKeys );
   if( ! hb_ntxBlockWrite( pIndex, pPage->Page,
                           ( BYTE * ) hb_ntxPageBuffer( pPage ), NTXBLOCKSIZE ) )
      return FALSE;
   pPage->Changed = FALSE;
   pIndex->fFlush = TRUE;
   /* In shared mode we have to update counter in version field of
      NTXHEADER to signal for other stations that their index buffers
      has to be discarded */
   if( pIndex->fShared )
      pIndex->Changed = TRUE;
   return TRUE;
}

/*
 * discard all index buffers due to concurrent access
 */
static void hb_ntxDiscardBuffers( LPNTXINDEX pIndex )
{
   pIndex->ulPages   = pIndex->ulPageLast = 0;
   pIndex->pChanged  = pIndex->pFirst = pIndex->pLast = NULL;
   if( pIndex->Compound )
   {
      int i;

      for( i = 0; i < pIndex->iTags; i++ )
      {
         pIndex->lpTags[ i ]->RootBlock   = 0;
         pIndex->lpTags[ i ]->stackLevel  = 0;
      }
   }
   else
   {
      pIndex->TagBlock = 0;
      if( pIndex->iTags )
         pIndex->lpTags[ 0 ]->stackLevel = 0;
   }
}

/*
 * update tag flags
 */
static void hb_ntxTagUpdateFlags( LPTAGINFO pTag )
{
   USHORT uiSignature = pTag->Signature;

   pTag->Custom   = ( uiSignature & NTX_FLAG_CUSTOM ) != 0;
   pTag->ChgOnly  = ( uiSignature & NTX_FLAG_CHGONLY ) != 0;
   pTag->Partial  = ( uiSignature & NTX_FLAG_PARTIAL ) != 0;
   pTag->Template = ( uiSignature & NTX_FLAG_TEMPLATE ) != 0;
   pTag->MultiKey = ( uiSignature & NTX_FLAG_MULTIKEY ) != 0;
   pTag->fSortRec = ( uiSignature & NTX_FLAG_SORTRECNO ) != 0;
}

/*
 * check tag header in compound index
 */
static BOOL hb_ntxTagHeaderCheck( LPTAGINFO pTag )
{
   if( ! pTag->RootBlock )
   {
      if( pTag->HeadBlock )
      {
         BYTE buffer[ NTX_TAGHEAD_HEADSIZE ];
         if( hb_ntxBlockRead( pTag->Owner, pTag->HeadBlock, buffer, NTX_TAGHEAD_HEADSIZE ) )
         {
            LPNTXHEADER pHeader = ( LPNTXHEADER ) ( void * ) buffer;
            pTag->Signature   = HB_GET_LE_UINT16( pHeader->type );
            pTag->RootBlock   = HB_GET_LE_UINT32( pHeader->root );
            hb_ntxTagUpdateFlags( pTag );
         }
      }
   }
   return pTag->RootBlock != 0;
}

/*
 * free buffers for pages in the tag
 */
static void hb_ntxFreePageBuffer( LPNTXINDEX pIndex )
{
   ULONG          ul, ulMax = pIndex->ulPagesDepth;
   LPPAGEINFO *   pPagePtr = pIndex->pages;

   if( ulMax )
   {
      for( ul = 0; ul < ulMax; ul++, pPagePtr++ )
      {
         if( *pPagePtr )
         {
#ifdef HB_NTX_EXTERNAL_PAGEBUFFER
            if( hb_ntxPageBuffer( *pPagePtr ) )
               hb_xfree( hb_ntxPageBuffer( *pPagePtr ) );
#endif
            hb_xfree( *pPagePtr );
         }
      }
      hb_xfree( pIndex->pages );
      pIndex->pages     = NULL;
      pIndex->ulPages   = pIndex->ulPageLast = pIndex->ulPagesDepth = 0;
      pIndex->pFirst    = pIndex->pLast = pIndex->pChanged = NULL;
   }
}

/*
 * trunc index file, left only space for header
 */
static void hb_ntxIndexTrunc( LPNTXINDEX pIndex )
{
   if( ! pIndex->lockWrite )
      hb_errInternal( 9102, "hb_ntxIndexTrunc on not locked index file.", NULL, NULL );

   hb_ntxFreePageBuffer( pIndex );
   pIndex->Update    = pIndex->Changed = pIndex->fFlush = TRUE;
   pIndex->TagBlock  = pIndex->NextAvail = 0;
   pIndex->Version   = 0;
   hb_fileTruncAt( pIndex->DiskFile, NTXBLOCKSIZE );
}

/*
 * try to find given tag page in the buffer
 */
static LPPAGEINFO hb_ntxPageFind( LPTAGINFO pTag, ULONG ulPage )
{
   LPPAGEINFO *   pPagePtr = pTag->Owner->pages;
   ULONG          u;

   for( u = pTag->Owner->ulPages; u; u--, pPagePtr++ )
   {
      if( *pPagePtr && ( *pPagePtr )->Page == ulPage )
         return *pPagePtr;
   }
   return NULL;
}

/*
 * try to find free space in buffer
 */
static LPPAGEINFO hb_ntxPageGetBuffer( LPTAGINFO pTag, ULONG ulPage )
{
   LPNTXINDEX     pIndex = pTag->Owner;
   LPPAGEINFO *   pPagePtr;

   if( pIndex->ulPages < pIndex->ulPagesDepth )
   {
      pPagePtr = &pIndex->pages[ pIndex->ulPages++ ];
   }
   else if( pIndex->pFirst )
   {
      LPPAGEINFO pPage = pIndex->pFirst;

      if( pPage->iUsed )
         hb_errInternal( 9305, "hb_ntxPageGetBuffer: page used.", NULL, NULL );
      if( pPage->Changed )
         hb_errInternal( 9306, "hb_ntxPageGetBuffer: page changed.", NULL, NULL );

      pIndex->pFirst = pPage->pNext;
      if( pIndex->pFirst )
         pIndex->pFirst->pPrev = NULL;
      else
         pIndex->pLast = NULL;
      pPage->pPrev   = NULL;
      pPage->Page    = ulPage;
      pPage->iUsed   = 1;

      return pPage;
   }
   else if( pIndex->ulPagesDepth == 0 )
   {
      pIndex->ulPages      = 1;
      pIndex->ulPageLast   = 0;
      pIndex->ulPagesDepth = NTX_PAGES_PER_TAG;
      pIndex->pages        = ( LPPAGEINFO * ) hb_xgrab( sizeof( LPPAGEINFO ) * NTX_PAGES_PER_TAG );
      memset( pIndex->pages, 0, sizeof( LPPAGEINFO ) * NTX_PAGES_PER_TAG );
      pPagePtr             = &pIndex->pages[ 0 ];
   }
   else
   {
      ULONG ul = pIndex->ulPageLast;
      for(;; )
      {
         if( ++ul >= pIndex->ulPagesDepth )
            ul = 0;
         pPagePtr = &pIndex->pages[ ul ];
         if( ! ( *pPagePtr )->iUsed && ! ( *pPagePtr )->Changed )
         {
            pIndex->ulPageLast = ul;
            break;
         }
         if( ul == pIndex->ulPageLast )
         {
            ul                   = pIndex->ulPagesDepth;
            pIndex->ulPagesDepth += NTX_PAGES_PER_TAG >> 1;
            pIndex->pages        = ( LPPAGEINFO * ) hb_xrealloc( pIndex->pages,
                                                                 sizeof( LPPAGEINFO ) * pIndex->ulPagesDepth );
            memset( pIndex->pages + ul, 0,
                    ( NTX_PAGES_PER_TAG >> 1 ) * sizeof( LPPAGEINFO ) );
            pIndex->ulPages++;
            pPagePtr             = &pIndex->pages[ ul ];
            pIndex->ulPageLast   = 0;
            break;
         }
      }
   }

   if( ! *pPagePtr )
   {
      *pPagePtr = ( LPPAGEINFO ) hb_xgrab( sizeof( HB_PAGEINFO ) );
      memset( *pPagePtr, 0, sizeof( HB_PAGEINFO ) );
   }
#ifdef HB_NTX_EXTERNAL_PAGEBUFFER
   if( ! hb_ntxPageBuffer( *pPagePtr ) )
   {
      hb_ntxPageBuffer( *pPagePtr ) = ( char * ) hb_xgrab( NTXBLOCKSIZE );
      memset( hb_ntxPageBuffer( *pPagePtr ), 0, NTXBLOCKSIZE );
   }
#endif
   ( *pPagePtr )->pPrev = NULL;
   ( *pPagePtr )->Page  = ulPage;
   ( *pPagePtr )->iUsed = 1;
   return *pPagePtr;
}

/*
 * free the index page for future reuse
 */
static void hb_ntxPageFree( LPTAGINFO pTag, LPPAGEINFO pPage )
{
   hb_ntxSetKeyPage( pPage, 0, pTag->Owner->NextAvail );
   pTag->Owner->NextAvail  = pPage->Page;
   pTag->Owner->Changed    = pPage->Changed = TRUE;
}

/*
 * mark used page as free
 */
static void hb_ntxPageRelease( LPTAGINFO pTag, LPPAGEINFO pPage )
{
   LPNTXINDEX pIndex = pTag->Owner;

   if( --pPage->iUsed == 0 )
   {
      if( pPage->Changed )
      {
         if( ! pPage->pPrev )
         {
            pPage->pPrev      = pPage;
            pPage->pNext      = pIndex->pChanged;
            pIndex->pChanged  = pPage;
         }
      }
      else if( pIndex->pLast )
      {
         pIndex->pLast->pNext = pPage;
         pPage->pPrev         = pIndex->pLast;
         pPage->pNext         = NULL;
         pIndex->pLast        = pPage;
      }
      else
      {
         pPage->pNext   = pPage->pPrev = NULL;
         pIndex->pFirst = pIndex->pLast = pPage;
      }
   }
   else if( pPage->iUsed < 0 )
      hb_errInternal( 9307, "hb_ntxPageRelease: unused page freed.", NULL, NULL );
}

/*
 * load page from index file or the buffer
 */
static LPPAGEINFO hb_ntxPageLoad( LPTAGINFO pTag, ULONG ulPage )
{
   LPPAGEINFO pPage;

   if( ! ulPage )
   {
      if( hb_ntxTagHeaderCheck( pTag ) )
         ulPage = pTag->RootBlock;
      if( ! ulPage )
      {
         hb_ntxErrorRT( pTag->Owner->Owner, EG_CORRUPTION, EDBF_CORRUPT,
                        pTag->Owner->IndexName, 0, 0, NULL );
         return NULL;
      }
   }
   pPage = hb_ntxPageFind( pTag, ulPage );
   if( pPage )
   {
      if( ! pPage->Changed && ! pPage->iUsed )
      {
         if( pPage->pNext )
            pPage->pNext->pPrev = pPage->pPrev;
         else
            pTag->Owner->pLast = pPage->pPrev;
         if( pPage->pPrev )
         {
            pPage->pPrev->pNext  = pPage->pNext;
            pPage->pPrev         = NULL;
         }
         else
            pTag->Owner->pFirst = pPage->pNext;
      }
      pPage->iUsed++;
   }
   else
   {
      pPage          = hb_ntxPageGetBuffer( pTag, ulPage );
      pPage->Changed = FALSE;
      if( ! hb_ntxBlockRead( pTag->Owner, ulPage,
                             ( BYTE * ) hb_ntxPageBuffer( pPage ), NTXBLOCKSIZE ) )
      {
         hb_ntxPageRelease( pTag, pPage );
         return NULL;
      }
      pPage->uiKeys = hb_ntxGetKeyCount( pPage );
   }
   return pPage;
}

/*
 * initialize empty page structure
 */
static void hb_ntxPageInit( LPTAGINFO pTag, LPPAGEINFO pPage )
{
   USHORT u, o = ( pTag->MaxKeys + 2 ) << 1;

   for( u = 0; u <= pTag->MaxKeys; u++, o += pTag->KeyLength + 8 )
      hb_ntxSetKeyOffset( pPage, u, o );
   hb_ntxSetKeyPage( pPage, 0, 0 );
   pPage->uiKeys = 0;
}

/*
 * allocate new page address
 */
static ULONG hb_ntxPageAlloc( LPNTXINDEX pIndex )
{
   ULONG ulPage;

   if( ! pIndex->TagBlock )
   {
      HB_FOFFSET fOffset;
      fOffset           = hb_fileSize( pIndex->DiskFile );
      pIndex->TagBlock  = ( ULONG )
                          ( fOffset >> ( pIndex->LargeFile ? NTXBLOCKBITS : 0 ) );
   }
   ulPage            = pIndex->TagBlock;
   pIndex->TagBlock  += pIndex->LargeFile ? 1 : NTXBLOCKSIZE;
   return ulPage;
}

/*
 * allocate new page in index file - reuse freed one or increase file
 */
static LPPAGEINFO hb_ntxPageNew( LPTAGINFO pTag, BOOL fNull )
{
   LPPAGEINFO pPage;

   if( pTag->Owner->NextAvail != 0 )
   {
      /*
         Handling of a pool of empty pages.
         Some sources says that this address is in the first 4 bytes of
         a page ( http://www.e-bachmann.dk/docs/xbase.htm ).
         But as I understood, studying dumps of Clipper ntx'es, address of the
         next available page is in the address field of a first key item
         in the page - it is done here now in such a way.
         = Alexander Kresin =
       */
      pPage = hb_ntxPageLoad( pTag, pTag->Owner->NextAvail );
      if( ! pPage )
         return NULL;
      /*
         Unfortunately Clipper does not left unused index pages clean and
         the key counter can be set to non zero value so to make possible
         concurrent index access from Clipper and xHarbour it's necessary
         to disable the check code below. [druzus]
       */
#if 0
      if( pPage->uiKeys != 0 )
      {
         hb_ntxErrorRT( pTag->Owner->Owner, EG_CORRUPTION, EDBF_CORRUPT,
                        pTag->Owner->IndexName, 0, 0, NULL );
         return NULL;
      }
#endif
      pTag->Owner->NextAvail  = hb_ntxGetKeyPage( pPage, 0 );
#if defined( HB_NTX_NOMULTITAG )
      hb_ntxSetKeyPage( pPage, 0, 0 );
      pPage->uiKeys           = 0;
#else
      hb_ntxPageInit( pTag, pPage );
#endif
   }
   else
   {
      pPage = hb_ntxPageGetBuffer( pTag, fNull ? 0 : hb_ntxPageAlloc( pTag->Owner ) );
      hb_ntxPageInit( pTag, pPage );
   }
   pTag->Owner->Changed = pPage->Changed = TRUE;

   return pPage;
}

/*
 * add given page to list of free pages
 */
static void hb_ntxPageAddFree( LPTAGINFO pTag, ULONG ulPage )
{
   LPPAGEINFO pPage = hb_ntxPageGetBuffer( pTag, ulPage );

   pPage->Changed = TRUE;
   hb_ntxPageInit( pTag, pPage );
   hb_ntxPageFree( pTag, pPage );
   hb_ntxPageSave( pTag->Owner, pPage );
   hb_ntxPageRelease( pTag, pPage );
}

/*
 * get free page in index file
 */
static ULONG hb_ntxPageGetFree( LPTAGINFO pTag )
{
   LPPAGEINFO  pPage    = hb_ntxPageNew( pTag, FALSE );
   ULONG       ulPage   = 0;

   if( pPage )
   {
      ulPage         = pPage->Page;
      pPage->Changed = FALSE;
      hb_ntxPageRelease( pTag, pPage );
   }
   return ulPage;
}

/*
 * create the new tag structure
 */
static LPTAGINFO hb_ntxTagNew( LPNTXINDEX pIndex,
                               const char * szTagName, BOOL fTagName,
                               const char * szKeyExpr, PHB_ITEM pKeyExpr,
                               BYTE bKeyType, USHORT uiKeyLen, USHORT uiKeyDec,
                               const char * szForExpr, PHB_ITEM pForExpr,
                               BOOL fAscendKey, BOOL fUnique, BOOL fCustom,
                               BOOL fSortRec )
{
   LPTAGINFO pTag;

   pTag           = ( LPTAGINFO ) hb_xgrab( sizeof( TAGINFO ) );
   memset( pTag, 0, sizeof( TAGINFO ) );
   pTag->TagName  = hb_strndup( szTagName, NTX_MAX_TAGNAME );
   pTag->fTagName = fTagName;
   pTag->Owner    = pIndex;
   if( szKeyExpr )
   {
      pTag->KeyExpr = hb_strndup( szKeyExpr, NTX_MAX_EXP );
   }
   if( pForExpr && szForExpr )
   {
      pTag->ForExpr = hb_strndup( szForExpr, NTX_MAX_EXP );
   }
   pTag->nField      = hb_rddFieldExpIndex( ( AREAP ) pIndex->Owner, pTag->KeyExpr );
   pTag->pKeyItem    = pKeyExpr;
   pTag->pForItem    = pForExpr;
   pTag->AscendKey   = fAscendKey;
   pTag->fUsrDescend = ! pTag->AscendKey;
   pTag->UniqueKey   = fUnique;
   pTag->Custom      = fCustom;
   pTag->MultiKey    = fCustom && DBFAREA_DATA( &pIndex->Owner->dbfarea )->fMultiKey;
   pTag->KeyType     = bKeyType;
   pTag->KeyLength   = uiKeyLen;
   pTag->KeyDec      = uiKeyDec;
   pTag->fSortRec    = fSortRec;
   /*
    * TODO?: keep during page update the offset to 'MaxKeys' key fixed
    * so we will be able to store 1 key more in the page
    */
   pTag->MaxKeys     = ( NTXBLOCKSIZE - 2 ) / ( uiKeyLen + 10 ) - 1;

   /* TODO?: is it necessary? It should not interact with well implemented
      algorithm */
   if( pTag->MaxKeys & 0x01 && pTag->MaxKeys > 2 )
      pTag->MaxKeys--;

   pTag->CurKeyInfo = hb_ntxKeyNew( NULL, pTag->KeyLength );

   return pTag;
}

/*
 * free from memory tag structure
 */
static void hb_ntxTagFree( LPTAGINFO pTag )
{
   if( pTag == pTag->Owner->Owner->lpCurTag )
      pTag->Owner->Owner->lpCurTag = NULL;
   hb_xfree( pTag->TagName );
   if( pTag->KeyExpr )
      hb_xfree( pTag->KeyExpr );
   if( pTag->ForExpr )
      hb_xfree( pTag->ForExpr );
   if( pTag->pKeyItem )
      hb_vmDestroyBlockOrMacro( pTag->pKeyItem );
   if( pTag->pForItem )
      hb_vmDestroyBlockOrMacro( pTag->pForItem );
   if( pTag->HotKeyInfo )
      hb_ntxKeyFree( pTag->HotKeyInfo );
   hb_ntxKeyFree( pTag->CurKeyInfo );
   hb_ntxTagClearScope( pTag, 0 );
   hb_ntxTagClearScope( pTag, 1 );
   if( pTag->stack )
      hb_xfree( pTag->stack );
   hb_xfree( pTag );
}

/*
 * delete tag from compund index
 */
static void hb_ntxTagDelete( LPTAGINFO pTag )
{
   LPNTXINDEX  pIndex = pTag->Owner;
   int         i;

   for( i = 0; i < pIndex->iTags; i++ )
   {
      if( pTag == pIndex->lpTags[ i ] )
      {
         while( ++i < pIndex->iTags )
            pIndex->lpTags[ i - 1 ] = pIndex->lpTags[ i ];
         if( --pIndex->iTags )
            pIndex->lpTags = ( LPTAGINFO * ) hb_xrealloc( pIndex->lpTags,
                                                          sizeof( LPTAGINFO ) * pIndex->iTags );
         else
            hb_xfree( pIndex->lpTags );
         break;
      }
   }
   hb_ntxTagFree( pTag );
   pIndex->Owner->fSetTagNumbers = TRUE;
}

/*
 * add tag to compund index
 */
static HB_ERRCODE hb_ntxTagAdd( LPNTXINDEX pIndex, LPTAGINFO pTag )
{
   if( pIndex->iTags >= CTX_MAX_TAGS )
      return HB_FAILURE;

   if( pIndex->iTags )
      pIndex->lpTags = ( LPTAGINFO * ) hb_xrealloc( pIndex->lpTags,
                                                    sizeof( LPTAGINFO ) * ( pIndex->iTags + 1 ) );
   else
      pIndex->lpTags = ( LPTAGINFO * ) hb_xgrab( sizeof( LPTAGINFO ) );

   pIndex->lpTags[ pIndex->iTags++ ]   = pTag;
   pIndex->Owner->fSetTagNumbers       = TRUE;
   return HB_SUCCESS;
}

/*
 * create new tag and load it from index file
 */
static LPTAGINFO hb_ntxTagLoad( LPNTXINDEX pIndex, ULONG ulBlock,
                                char * szTagName, BYTE * buffer )
{
   LPNTXHEADER lpNTX = ( LPNTXHEADER ) buffer;
   LPTAGINFO   pTag;
   PHB_ITEM    pKeyExp, pForExp = NULL;
   USHORT      usType;
   BOOL        fName;

   usType = HB_GET_LE_UINT16( lpNTX->type );

   if( ( usType & ~NTX_FLAG_MASK ) ||
       ( ( usType & NTX_FLAG_DEFALUT ) != NTX_FLAG_DEFALUT &&
         usType != NTX_FLAG_OLDDEFALUT ) ||
       lpNTX->key_expr[ 0 ] < 0x20 )
      return NULL;

   if( SELF_COMPILE( ( AREAP ) pIndex->Owner, ( const char * ) lpNTX->key_expr ) == HB_FAILURE )
      return NULL;
   pKeyExp                                = pIndex->Owner->dbfarea.area.valResult;
   pIndex->Owner->dbfarea.area.valResult  = NULL;

   if( usType & NTX_FLAG_FORITEM && lpNTX->for_expr[ 0 ] >= 0x20 )
   {
      if( SELF_COMPILE( ( AREAP ) pIndex->Owner, ( const char * ) lpNTX->for_expr ) == HB_FAILURE )
      {
         hb_vmDestroyBlockOrMacro( pKeyExp );
         return NULL;
      }
      pForExp                                = pIndex->Owner->dbfarea.area.valResult;
      pIndex->Owner->dbfarea.area.valResult  = NULL;
   }
   fName = ! pIndex->Compound && lpNTX->tag_name[ 0 ] >= 0x20;
   pTag  = hb_ntxTagNew( pIndex,
                         fName ? ( char * ) lpNTX->tag_name : szTagName, fName,
                         ( char * ) lpNTX->key_expr, pKeyExp,
                         '\0',
                         HB_GET_LE_UINT16( lpNTX->key_size ),
                         HB_GET_LE_UINT16( lpNTX->key_dec ),
                         ( char * ) lpNTX->for_expr, pForExp,
                         lpNTX->descend[ 0 ] == 0, lpNTX->unique[ 0 ] != 0,
                         ( usType & NTX_FLAG_CUSTOM ) != 0 || lpNTX->custom[ 0 ] != 0,
                         ( usType & NTX_FLAG_SORTRECNO ) != 0 );

   pTag->Signature   = usType;
   hb_ntxTagUpdateFlags( pTag );
   pTag->HeadBlock   = ulBlock;
   pTag->RootBlock   = HB_GET_LE_UINT32( lpNTX->root );
   pTag->MaxKeys     = HB_GET_LE_UINT16( lpNTX->max_item );
   pTag->KeyType     = hb_ntxGetKeyType( pTag );

   pIndex->LargeFile = ( usType & NTX_FLAG_LARGEFILE ) != 0;

   if( ! pIndex->Compound )
   {
      pIndex->Version   = HB_GET_LE_UINT16( lpNTX->version );
      pIndex->NextAvail = HB_GET_LE_UINT32( lpNTX->next_page );
      pIndex->TagBlock  = 0;

      /* TODO: this breaks unlocking !!! */
      if( usType & NTX_FLAG_LARGEFILE )
      {
         pIndex->Owner->dbfarea.bLockType = DB_DBFLOCK_XHB64;
      }
      else if( usType & NTX_FLAG_EXTLOCK )
      {
         pIndex->Owner->dbfarea.bLockType = DB_DBFLOCK_CL53EXT;
      }
      else if( ! pIndex->Owner->dbfarea.bLockType )
      {
         pIndex->Owner->dbfarea.bLockType = usType & NTX_FLAG_EXTLOCK ?
                                            DB_DBFLOCK_CL53EXT : DB_DBFLOCK_CLIP;
      }
   }
   return pTag;
}

/*
 * add tag into CTX header
 */
static void hb_ntxIndexTagAdd( LPNTXINDEX pIndex, LPTAGINFO pTag )
{
   LPCTXHEADER    lpCTX    = ( LPCTXHEADER ) pIndex->HeaderBuff;
   int            iTags    = HB_GET_LE_UINT16( lpCTX->ntags ), iLen, i;
   LPCTXTAGITEM   pTagItem = ( LPCTXTAGITEM ) lpCTX->tags;

   for( i = 0; i < iTags; pTagItem++, i++ )
   {
      if( ! hb_strnicmp( ( char * ) pTagItem->tag_name, pTag->TagName, NTX_MAX_TAGNAME ) )
         break;
   }
   if( i == iTags )
   {
      ++iTags;
      HB_PUT_LE_UINT16( lpCTX->ntags, iTags );
      iLen = ( int ) strlen( pTag->TagName );
      if( iLen > NTX_MAX_TAGNAME )
         iLen = NTX_MAX_TAGNAME;
      HB_MEMCPY( pTagItem->tag_name, pTag->TagName, iLen );
      memset( pTagItem->tag_name + iLen, 0, sizeof( pTagItem->tag_name ) - iLen );
   }
   HB_PUT_LE_UINT32( pTagItem->tag_header, pTag->HeadBlock );
   pIndex->Update = TRUE;
}

/*
 * delete tag from CTX header
 */
static void hb_ntxIndexTagDel( LPNTXINDEX pIndex, char * szTagName )
{
   LPCTXHEADER    lpCTX    = ( LPCTXHEADER ) pIndex->HeaderBuff;
   int            iTags    = HB_GET_LE_UINT16( lpCTX->ntags ), i;
   LPCTXTAGITEM   pTagItem = ( LPCTXTAGITEM ) lpCTX->tags;

   for( i = 0; i < iTags; pTagItem++, i++ )
   {
      if( ! hb_strnicmp( ( char * ) pTagItem->tag_name, szTagName, NTX_MAX_TAGNAME ) )
      {
         memmove( pTagItem, pTagItem + 1, ( iTags - i ) * NTX_TAGITEMSIZE );
         memset( pTagItem + iTags - 1, 0, NTX_TAGITEMSIZE );
         --iTags;
         HB_PUT_LE_UINT16( lpCTX->ntags, iTags );
         pIndex->Update = TRUE;
         break;
      }
   }
}

/*
 * find tag header block in CTX header
 */
static ULONG hb_ntxIndexTagFind( LPCTXHEADER lpCTX, char * szTagName )
{
   int            iTags    = HB_GET_LE_UINT16( lpCTX->ntags ), i;
   LPCTXTAGITEM   pTagItem = ( LPCTXTAGITEM ) lpCTX->tags;

   for( i = 0; i < iTags; pTagItem++, i++ )
   {
      if( ! hb_strnicmp( ( char * ) pTagItem->tag_name, szTagName, NTX_MAX_TAGNAME ) )
         return HB_GET_LE_UINT32( pTagItem->tag_header );
   }
   return NTX_DUMMYNODE;
}

/*
 * Write tag header
 */
static HB_ERRCODE hb_ntxTagHeaderSave( LPTAGINFO pTag )
{
   LPNTXINDEX  pIndex   = pTag->Owner;
   NTXHEADER   Header;
   int         iSize    = NTX_ROOTHEAD_HEADSIZE, type, version = 0, iLen;
   ULONG       next     = 0;

   if( pIndex->Compound )
   {
      if( ! pTag->HeadBlock )
      {
         pTag->HeadBlock = hb_ntxPageGetFree( pTag );
         if( ! pTag->HeadBlock )
            return HB_FAILURE;
         hb_ntxIndexTagAdd( pIndex, pTag );
      }
   }
   else
   {
      if( pTag->HeadBlock )
      {
         hb_ntxPageAddFree( pTag, pTag->HeadBlock );
         pTag->HeadBlock   = 0;
         pIndex->Update    = TRUE;
      }
      pIndex->Version++;
      version  = pIndex->Version &= 0xffff;
      next     = pIndex->NextAvail;
   }

   type = NTX_FLAG_DEFALUT |
          ( pTag->ForExpr ? NTX_FLAG_FORITEM : 0 ) |
          ( pTag->Partial ? NTX_FLAG_PARTIAL | NTX_FLAG_FORITEM : 0 ) |
          ( pIndex->Owner->dbfarea.bLockType == DB_DBFLOCK_CL53EXT ? NTX_FLAG_EXTLOCK : 0 ) |
          ( pTag->Partial  ? NTX_FLAG_PARTIAL | NTX_FLAG_FORITEM : 0 ) |
          /* non CLipper flags */
          ( pTag->Custom   ? NTX_FLAG_CUSTOM : 0 ) |
          ( pTag->ChgOnly  ? NTX_FLAG_CHGONLY : 0 ) |
          ( pTag->Template ? NTX_FLAG_TEMPLATE : 0 ) |
          ( pTag->MultiKey ? NTX_FLAG_MULTIKEY : 0 ) |
          ( pTag->fSortRec ? NTX_FLAG_SORTRECNO : 0 ) |
          ( pIndex->LargeFile ? NTX_FLAG_LARGEFILE : 0 );

   HB_PUT_LE_UINT16( Header.type, type );
   HB_PUT_LE_UINT16( Header.version, version );
   HB_PUT_LE_UINT32( Header.root, pTag->RootBlock );
   HB_PUT_LE_UINT32( Header.next_page, next );

   if( pIndex->Update )
   {
      memset( ( BYTE * ) &Header + NTX_ROOTHEAD_HEADSIZE, 0,
              sizeof( NTXHEADER ) - NTX_ROOTHEAD_HEADSIZE );

      HB_PUT_LE_UINT16( Header.item_size, pTag->KeyLength + 8 );
      HB_PUT_LE_UINT16( Header.key_size, pTag->KeyLength );
      HB_PUT_LE_UINT16( Header.key_dec, pTag->KeyDec );
      HB_PUT_LE_UINT16( Header.max_item, pTag->MaxKeys );
      HB_PUT_LE_UINT16( Header.half_page, pTag->MaxKeys >> 1 );
      Header.unique[ 0 ]   = pTag->UniqueKey ? 1 : 0;
      Header.descend[ 0 ]  = pTag->AscendKey ? 0 : 1;
      Header.custom[ 0 ]   = pTag->Custom    ? 1 : 0;
      iLen                 = ( int ) strlen( pTag->KeyExpr );
      if( iLen > NTX_MAX_EXP )
         iLen = NTX_MAX_EXP;
      HB_MEMCPY( Header.key_expr, pTag->KeyExpr, iLen );
      if( pTag->ForExpr )
      {
         iLen = ( int ) strlen( pTag->ForExpr );
         if( iLen > NTX_MAX_EXP )
            iLen = NTX_MAX_EXP;
         HB_MEMCPY( Header.for_expr, pTag->ForExpr, iLen );
      }
      if( pTag->fTagName )
      {
         iLen = ( int ) strlen( pTag->TagName );
         if( iLen > NTX_MAX_TAGNAME )
            iLen = NTX_MAX_TAGNAME;
         HB_MEMCPY( Header.tag_name, pTag->TagName, iLen );
      }
      iSize = sizeof( NTXHEADER );
   }

   if( ! hb_ntxBlockWrite( pIndex, pTag->HeadBlock, ( BYTE * ) &Header, iSize ) )
      return HB_FAILURE;
   pTag->HdrChanged  = FALSE;
   pIndex->Changed   = pIndex->Compound;
   pIndex->fFlush    = TRUE;
   return HB_SUCCESS;
}

/*
 * create new index structure
 */
static LPNTXINDEX hb_ntxIndexNew( NTXAREAP pArea )
{
   LPNTXINDEX pIndex;

   pIndex            = ( LPNTXINDEX ) hb_xgrab( sizeof( NTXINDEX ) );
   memset( pIndex, 0, sizeof( NTXINDEX ) );

   pIndex->DiskFile  = NULL;
   pIndex->Owner     = pArea;
   return pIndex;
}

/*
 * close the index file and free from memory index and tag structures
 */
static void hb_ntxIndexFree( LPNTXINDEX pIndex )
{
   hb_ntxFreePageBuffer( pIndex );
   if( pIndex->iTags )
   {
      int i;
      for( i = 0; i < pIndex->iTags; i++ )
         hb_ntxTagFree( pIndex->lpTags[ i ] );
      hb_xfree( pIndex->lpTags );
   }
   if( pIndex->HeaderBuff )
      hb_xfree( pIndex->HeaderBuff );
   if( pIndex->DiskFile )
   {
      hb_fileClose( pIndex->DiskFile );
      if( pIndex->fDelete )
      {
         hb_fileDelete( ( const char * ) pIndex->RealName ? pIndex->RealName : pIndex->IndexName, 0 );
      }
   }
   if( pIndex->IndexName )
      hb_xfree( pIndex->IndexName );
   if( pIndex->RealName )
      hb_xfree( pIndex->RealName );
   hb_xfree( pIndex );
}

/*
 * Write tag header
 */
static HB_ERRCODE hb_ntxIndexHeaderSave( LPNTXINDEX pIndex )
{
   if( pIndex->Compound )
   {
      LPCTXHEADER lpCTX = ( LPCTXHEADER ) pIndex->HeaderBuff;
      int         iSize = pIndex->Update ? NTXBLOCKSIZE : 16;
      USHORT      type;

      type = NTX_FLAG_COMPOUND | ( pIndex->LargeFile ? NTX_FLAG_LARGEFILE : 0 );

      pIndex->Version++;
      HB_PUT_LE_UINT16( lpCTX->type, type );
      HB_PUT_LE_UINT16( lpCTX->ntags, pIndex->iTags );
      HB_PUT_LE_UINT32( lpCTX->version, pIndex->Version );
      HB_PUT_LE_UINT32( lpCTX->freepage, pIndex->NextAvail );
      HB_PUT_LE_UINT32( lpCTX->filesize, pIndex->TagBlock );

      if( ! hb_ntxBlockWrite( pIndex, 0, ( BYTE * ) lpCTX, iSize ) )
         return HB_FAILURE;
   }
   pIndex->Changed = pIndex->Update = FALSE;
   return HB_SUCCESS;
}

/*
 * load new tags from index file
 */
static HB_ERRCODE hb_ntxIndexLoad( LPNTXINDEX pIndex, char * szTagName )
{
   LPTAGINFO   pTag;
   USHORT      type;

   if( ! pIndex->fValidHeader )
   {
      if( ! pIndex->HeaderBuff )
         pIndex->HeaderBuff = ( BYTE * ) hb_xgrab( NTXBLOCKSIZE );
      if( ! hb_ntxBlockRead( pIndex, 0, pIndex->HeaderBuff, NTXBLOCKSIZE ) )
         return HB_FAILURE;
      pIndex->fValidHeader = TRUE;
   }

   type              = HB_GET_LE_UINT16( pIndex->HeaderBuff );
#if ! defined( HB_NTX_NOMULTITAG )
   pIndex->Compound  = ( type & NTX_FLAG_COMPOUND ) != 0;
   if( pIndex->Compound )
   {
      BYTE           tagbuffer[ NTXBLOCKSIZE ];
      LPCTXHEADER    lpCTX    = ( LPCTXHEADER ) pIndex->HeaderBuff;
      LPCTXTAGITEM   pTagItem = ( LPCTXTAGITEM ) lpCTX->tags;
      ULONG          ulBlock;
      int            iTags;

      iTags             = HB_GET_LE_UINT16( lpCTX->ntags );
      if( iTags > CTX_MAX_TAGS )
         return HB_FAILURE;
      pIndex->Version   = HB_GET_LE_UINT32( lpCTX->version );
      pIndex->NextAvail = HB_GET_LE_UINT32( lpCTX->freepage );
      pIndex->TagBlock  = HB_GET_LE_UINT32( lpCTX->filesize );
      pIndex->LargeFile = ( type & NTX_FLAG_LARGEFILE ) != 0;

      for( pIndex->iTags = 0; pIndex->iTags < iTags; pTagItem++ )
      {
         ulBlock = HB_GET_LE_UINT32( pTagItem->tag_header );
         if( ulBlock == 0 || pTagItem->tag_name[ 0 ] <= 0x20 )
            return HB_FAILURE;
         if( ! hb_ntxBlockRead( pIndex, ulBlock, tagbuffer, NTXBLOCKSIZE ) )
            return HB_FAILURE;
         pTag = hb_ntxTagLoad( pIndex, ulBlock, ( char * ) pTagItem->tag_name, tagbuffer );
         if( ! pTag )
            return HB_FAILURE;
         hb_ntxTagAdd( pIndex, pTag );
      }
   }
   else
#endif
   {
      pTag = hb_ntxTagLoad( pIndex, 0, szTagName, pIndex->HeaderBuff );
      if( ! pTag )
         return HB_FAILURE;
      hb_ntxTagAdd( pIndex, pTag );
   }

   return HB_SUCCESS;
}

/*
 * read index header and check for concurrent access
 */
static HB_ERRCODE hb_ntxIndexHeaderRead( LPNTXINDEX pIndex )
{
   USHORT type;

   if( ! pIndex->HeaderBuff )
      pIndex->HeaderBuff = ( BYTE * ) hb_xgrab( NTXBLOCKSIZE );

   if( ! hb_ntxBlockRead( pIndex, 0, pIndex->HeaderBuff, NTXBLOCKSIZE ) )
      return HB_FAILURE;

   type = HB_GET_LE_UINT16( pIndex->HeaderBuff );
   if( ( type & NTX_FLAG_COMPOUND ) != 0 )
   {
#if defined( HB_NTX_NOMULTITAG )
      hb_ntxErrorRT( pIndex->Owner, EG_CORRUPTION, EDBF_CORRUPT,
                     pIndex->IndexName, 0, 0, NULL );
      return HB_FAILURE;
#else
      LPCTXHEADER lpCTX = ( LPCTXHEADER ) pIndex->HeaderBuff;
      ULONG       ulVersion, ulNext;
      /* USHORT usTags = HB_GET_LE_UINT16( lpCTX->ntags ); */

      ulVersion         = HB_GET_LE_UINT32( lpCTX->version );
      ulNext            = HB_GET_LE_UINT32( lpCTX->freepage );
      pIndex->TagBlock  = HB_GET_LE_UINT32( lpCTX->filesize );

      if( pIndex->Version != ulVersion || pIndex->NextAvail != ulNext ||
          ! pIndex->Compound )
      {
         int i;
         hb_ntxDiscardBuffers( pIndex );
         pIndex->Version   = ulVersion;
         pIndex->NextAvail = ulNext;
         pIndex->Compound  = TRUE;
         for( i = 1; i < pIndex->iTags; i++ )
         {
            pIndex->lpTags[ i ]->HeadBlock =
               hb_ntxIndexTagFind( lpCTX, pIndex->lpTags[ i ]->TagName );
            if( ! pIndex->lpTags[ i ]->HeadBlock )
               pIndex->lpTags[ i ]->RootBlock = 0;
         }
      }
#endif
   }
   else
   {
      LPNTXHEADER lpNTX = ( LPNTXHEADER ) pIndex->HeaderBuff;
      ULONG       ulRootPage, ulVersion;
      LPTAGINFO   pTag;

      if( pIndex->Compound )
      {
         hb_ntxErrorRT( pIndex->Owner, EG_CORRUPTION, EDBF_CORRUPT,
                        pIndex->IndexName, 0, 0, NULL );
         return HB_FAILURE;
      }
      pTag              = pIndex->iTags ? pIndex->lpTags[ 0 ] : NULL;

      ulVersion         = HB_GET_LE_UINT16( lpNTX->version );
      ulRootPage        = HB_GET_LE_UINT32( lpNTX->root );
      pIndex->NextAvail = HB_GET_LE_UINT32( lpNTX->next_page );
      if( pIndex->Version != ulVersion || ( pTag &&
                                            ( pTag->Signature != type || ulRootPage != pTag->RootBlock ) ) )
      {
         hb_ntxDiscardBuffers( pIndex );
         pIndex->Version = ulVersion;
         if( pTag )
         {
            pTag->RootBlock   = ulRootPage;
            pTag->Signature   = type;
            hb_ntxTagUpdateFlags( pTag );
         }
      }
   }
   return HB_SUCCESS;
}

/*
 * write modified pages to index file
 */
static void hb_ntxIndexFlush( LPNTXINDEX pIndex )
{
   while( pIndex->pChanged )
   {
      LPPAGEINFO pPage = pIndex->pChanged;
      pIndex->pChanged = pPage->pNext;
      if( pPage->Changed )
      {
         hb_ntxPageSave( pIndex, pPage );
         ++pPage->iUsed;
         /* hack */
         hb_ntxPageRelease( pIndex->lpTags[ 0 ], pPage );
      }
      else
         hb_errInternal( 9308, "hb_ntxIndexFlush: unchaged page in the list.", NULL, NULL );
   }

   if( pIndex->Compound )
   {
      int i;

      for( i = 0; i < pIndex->iTags; i++ )
         if( pIndex->lpTags[ i ]->HdrChanged )
            hb_ntxTagHeaderSave( pIndex->lpTags[ i ] );
      if( pIndex->Changed )
         hb_ntxIndexHeaderSave( pIndex );
   }
   else if( pIndex->iTags )
   {
      if( pIndex->Changed || pIndex->lpTags[ 0 ]->HdrChanged )
         hb_ntxTagHeaderSave( pIndex->lpTags[ 0 ] );
   }
}

/*
 * lock index for reading (shared lock)
 */
static BOOL hb_ntxIndexLockRead( LPNTXINDEX pIndex )
{
   BOOL fOK;

   if( pIndex->lockRead > 0 || pIndex->lockWrite > 0 || ! pIndex->fShared ||
       HB_DIRTYREAD( &pIndex->Owner->dbfarea ) )
   {
      fOK = TRUE;
      pIndex->lockRead++;
   }
   else
   {
      fOK = hb_dbfLockIdxFile( pIndex->DiskFile, pIndex->Owner->dbfarea.bLockType,
                               FL_LOCK | FLX_SHARED | FLX_WAIT, &pIndex->ulLockPos );
      /* if fOK then check VERSION field in NTXHEADER and
       * if it has been changed then discard all page buffers
       */
      if( fOK )
      {
         pIndex->lockRead++;
         if( hb_ntxIndexHeaderRead( pIndex ) != HB_SUCCESS )
         {
            pIndex->lockRead--;
            hb_dbfLockIdxFile( pIndex->DiskFile, pIndex->Owner->dbfarea.bLockType,
                               FL_UNLOCK, &pIndex->ulLockPos );
            return FALSE;
         }
      }
   }
   if( ! fOK )
      hb_ntxErrorRT( pIndex->Owner, EG_LOCK, EDBF_LOCK,
                     pIndex->IndexName, hb_fsError(), 0, NULL );

   return fOK;
}

/*
 * lock index for writing (exclusive lock)
 */
static BOOL hb_ntxIndexLockWrite( LPNTXINDEX pIndex, BOOL fCheck )
{
   BOOL fOK;

   if( pIndex->fReadonly )
      hb_errInternal( 9101, "hb_ntxIndexLockWrite: readonly index.", NULL, NULL );

   if( pIndex->lockRead )
      hb_errInternal( 9105, "hb_ntxIndexLockWrite: writeLock after readLock.", NULL, NULL );

   if( pIndex->lockWrite > 0 || ! pIndex->fShared )
   {
      fOK = TRUE;
      pIndex->lockWrite++;
   }
   else
   {
      fOK = hb_dbfLockIdxFile( pIndex->DiskFile, pIndex->Owner->dbfarea.bLockType,
                               FL_LOCK | FLX_WAIT, &pIndex->ulLockPos );
      /* if fOK then check VERSION field in NTXHEADER and
       * if it has been changed then discard all page buffers
       */
      if( fOK )
      {
         pIndex->lockWrite++;
         if( fCheck && hb_ntxIndexHeaderRead( pIndex ) != HB_SUCCESS )
         {
            pIndex->lockWrite--;
            hb_dbfLockIdxFile( pIndex->DiskFile, pIndex->Owner->dbfarea.bLockType,
                               FL_UNLOCK, &pIndex->ulLockPos );
            return FALSE;
         }
      }
   }
   if( ! fOK )
      hb_ntxErrorRT( pIndex->Owner, EG_LOCK, EDBF_LOCK,
                     pIndex->IndexName, hb_fsError(), 0, NULL );

   return fOK;
}

/*
 * remove index read lock (shared lock)
 */
static BOOL hb_ntxIndexUnLockRead( LPNTXINDEX pIndex )
{
   BOOL  fOK;

#ifdef HB_NTX_DEBUG
   int   i;
   for( i = 0; i < pIndex->iTags; i++ )
      hb_ntxTagCheckBuffers( pIndex->lpTags[ i ] );
#endif

   pIndex->lockRead--;
   if( pIndex->lockRead < 0 )
      hb_errInternal( 9106, "hb_ntxIndexUnLockRead: bad count of locks.", NULL, NULL );

   if( pIndex->lockRead || pIndex->lockWrite || ! pIndex->fShared ||
       HB_DIRTYREAD( &pIndex->Owner->dbfarea ) )
   {
      fOK = TRUE;
   }
   else
   {
      pIndex->fValidHeader = FALSE;
      fOK                  = hb_dbfLockIdxFile( pIndex->DiskFile, pIndex->Owner->dbfarea.bLockType,
                                                FL_UNLOCK, &pIndex->ulLockPos );
   }
   if( ! fOK )
      hb_errInternal( 9108, "hb_ntxIndexUnLockRead: unlock error.", NULL, NULL );

   return fOK;
}

/*
 * remove index write lock (exclusive lock)
 */
static BOOL hb_ntxIndexUnLockWrite( LPNTXINDEX pIndex )
{
   BOOL  fOK;

#ifdef HB_NTX_DEBUG
   int   i;
   for( i = 0; i < pIndex->iTags; i++ )
      hb_ntxTagCheckBuffers( pIndex->lpTags[ i ] );
#endif

   if( pIndex->lockWrite <= 0 )
      hb_errInternal( 9106, "hb_ntxIndexUnLockWrite: bad count of locks.", NULL, NULL );
   if( pIndex->lockRead )
      hb_errInternal( 9105, "hb_ntxIndexUnLockWrite: writeUnLock before readUnLock.", NULL, NULL );

   hb_ntxIndexFlush( pIndex );
   pIndex->lockWrite--;

   if( pIndex->lockWrite || ! pIndex->fShared )
   {
      fOK = TRUE;
   }
   else
   {
      pIndex->fValidHeader = FALSE;
      fOK                  = hb_dbfLockIdxFile( pIndex->DiskFile, pIndex->Owner->dbfarea.bLockType,
                                                FL_UNLOCK, &pIndex->ulLockPos );
   }
   if( ! fOK )
      hb_errInternal( 9108, "hb_ntxIndexUnLockWrite: unlock error.", NULL, NULL );

   return fOK;
}

/*
 * lock tag for reading (shared lock)
 */
static BOOL hb_ntxTagLockRead( LPTAGINFO pTag )
{
   BOOL fOK = FALSE;

   if( hb_ntxIndexLockRead( pTag->Owner ) )
   {
      fOK = hb_ntxTagHeaderCheck( pTag );
      if( ! fOK )
      {
         hb_ntxIndexUnLockRead( pTag->Owner );
         hb_ntxErrorRT( pTag->Owner->Owner, EG_CORRUPTION, EDBF_CORRUPT,
                        pTag->Owner->IndexName, 0, 0, NULL );
      }
   }
   return fOK;
}

/*
 * lock tag for writing (exclusive lock)
 */
static BOOL hb_ntxTagLockWrite( LPTAGINFO pTag )
{
   BOOL fOK = FALSE;

   if( hb_ntxIndexLockWrite( pTag->Owner, TRUE ) )
   {
      fOK = hb_ntxTagHeaderCheck( pTag );
      if( ! fOK )
      {
         hb_ntxIndexUnLockWrite( pTag->Owner );
         hb_ntxErrorRT( pTag->Owner->Owner, EG_CORRUPTION, EDBF_CORRUPT,
                        pTag->Owner->IndexName, 0, 0, NULL );
      }
   }
   return fOK;
}

/*
 * remove tag read lock (shared lock)
 */
static BOOL hb_ntxTagUnLockRead( LPTAGINFO pTag )
{
   return hb_ntxIndexUnLockRead( pTag->Owner );
}

/*
 * remove tag write lock (exclusive lock)
 */
static BOOL hb_ntxTagUnLockWrite( LPTAGINFO pTag )
{
   return hb_ntxIndexUnLockWrite( pTag->Owner );
}

/*
 * retrive key from page
 */
static void hb_ntxPageGetKey( LPPAGEINFO pPage, USHORT uiKey, LPKEYINFO pKey, USHORT uiLen )
{
   if( uiKey < pPage->uiKeys )
   {
      HB_MEMCPY( pKey->key, hb_ntxGetKeyVal( pPage, uiKey ), uiLen );
      pKey->Xtra  = hb_ntxGetKeyRec( pPage, uiKey );
      pKey->Tag   = pPage->Page;
   }
   else
   {
      pKey->Xtra = pKey->Tag = 0;
   }
}

/*
 * set next page and key in page path
 */
static void hb_ntxTagSetPageStack( LPTAGINFO pTag, ULONG ulPage, USHORT uiKey )
{
   if( pTag->stackLevel == pTag->stackSize )
   {
      if( pTag->stackSize == 0 )
      {
         pTag->stackSize   = NTX_STACKSIZE;
         pTag->stack       = ( LPTREESTACK ) hb_xgrab( sizeof( TREE_STACK ) * NTX_STACKSIZE );
      }
      else
      {
         pTag->stackSize   += NTX_STACKSIZE;
         pTag->stack       = ( LPTREESTACK ) hb_xrealloc( pTag->stack,
                                                          sizeof( TREE_STACK ) * pTag->stackSize );
      }
   }
   pTag->stack[ pTag->stackLevel ].page   = ulPage;
   pTag->stack[ pTag->stackLevel++ ].ikey = uiKey;
}

/*
 * go down from the given index page to the first key
 */
static LPPAGEINFO hb_ntxPageTopMove( LPTAGINFO pTag, ULONG ulPage )
{
   LPPAGEINFO pPage = NULL;

   do
   {
      if( pPage )
         hb_ntxPageRelease( pTag, pPage );
      pPage = hb_ntxPageLoad( pTag, ulPage );
      if( ! pPage )
         return NULL;
#ifdef HB_NTX_DEBUG_EXT
      if( pPage->uiKeys == 0 && pTag->stackLevel > 0 )
         hb_errInternal( 9201, "hb_ntxPageTopMove: index corrupted.", NULL, NULL );
#endif
      ulPage = hb_ntxGetKeyPage( pPage, 0 );
      hb_ntxTagSetPageStack( pTag, pPage->Page, 0 );
   }
   while( ulPage );

   return pPage;
}

/*
 * go down from the given index page to the last key
 */
static LPPAGEINFO hb_ntxPageBottomMove( LPTAGINFO pTag, ULONG ulPage )
{
   LPPAGEINFO pPage = NULL;

   do
   {
      if( pPage )
         hb_ntxPageRelease( pTag, pPage );
      pPage = hb_ntxPageLoad( pTag, ulPage );
      if( ! pPage )
         return NULL;
#ifdef HB_NTX_DEBUG_EXT
      if( pPage->uiKeys == 0 && pTag->stackLevel > 0 )
         hb_errInternal( 9201, "hb_ntxPageBottomMove: index corrupted.", NULL, NULL );
#endif
      ulPage = hb_ntxGetKeyPage( pPage, pPage->uiKeys );
      hb_ntxTagSetPageStack( pTag, pPage->Page, pPage->uiKeys -
                             ( ulPage || pPage->uiKeys == 0 ? 0 : 1 ) );
   }
   while( ulPage );

   return pPage;
}

/*
 * set page path to the first key in tag
 */
static BOOL hb_ntxTagTopKey( LPTAGINFO pTag )
{
   LPPAGEINFO  pPage;
   int         iKeys;

   pTag->stackLevel  = 0;
   pPage             = hb_ntxPageTopMove( pTag, 0 );
   if( ! pPage )
      return FALSE;
   hb_ntxPageGetKey( pPage, 0, pTag->CurKeyInfo, pTag->KeyLength );
   iKeys = pPage->uiKeys;
   hb_ntxPageRelease( pTag, pPage );
   return iKeys != 0;
}

/*
 * set page path to the last key in tag
 */
static BOOL hb_ntxTagBottomKey( LPTAGINFO pTag )
{
   LPPAGEINFO  pPage;
   int         iKeys;

   pTag->stackLevel  = 0;
   pPage             = hb_ntxPageBottomMove( pTag, 0 );
   if( ! pPage )
      return FALSE;
   hb_ntxPageGetKey( pPage, pTag->stack[ pTag->stackLevel - 1 ].ikey,
                     pTag->CurKeyInfo, pTag->KeyLength );
   iKeys = pPage->uiKeys;
   hb_ntxPageRelease( pTag, pPage );
   return iKeys != 0;
}

/*
 * update page path to the next key in tag
 */
static BOOL hb_ntxTagNextKey( LPTAGINFO pTag )
{
   int         iLevel   = pTag->stackLevel - 1;
   LPPAGEINFO  pPage;
   ULONG       ulPage   = 0;

   if( iLevel >= 0 )
   {
      pPage = hb_ntxPageLoad( pTag, pTag->stack[ iLevel ].page );
      if( ! pPage )
         return FALSE;
      if( pTag->stack[ iLevel ].ikey < pPage->uiKeys )
         ulPage = hb_ntxGetKeyPage( pPage, pTag->stack[ iLevel ].ikey + 1 );
      if( ulPage || pTag->stack[ iLevel ].ikey + 1 < pPage->uiKeys )
      {
         pTag->stack[ iLevel ].ikey++;
         if( ulPage )
         {
            hb_ntxPageRelease( pTag, pPage );
            pPage = hb_ntxPageTopMove( pTag, ulPage );
            if( ! pPage )
               return FALSE;
         }
      }
      else
      {
         while( --iLevel >= 0 )
         {
            hb_ntxPageRelease( pTag, pPage );
            pPage = hb_ntxPageLoad( pTag, pTag->stack[ iLevel ].page );
            if( ! pPage )
               return FALSE;
            if( pTag->stack[ iLevel ].ikey < pPage->uiKeys )
               break;
         }
         if( iLevel < 0 )
         {
            hb_ntxPageRelease( pTag, pPage );
            return FALSE;
         }
         pTag->stackLevel = iLevel + 1;
      }
      hb_ntxPageGetKey( pPage, pTag->stack[ pTag->stackLevel - 1 ].ikey,
                        pTag->CurKeyInfo, pTag->KeyLength );
      hb_ntxPageRelease( pTag, pPage );
      return TRUE;
   }
   return FALSE;
}

/*
 * update page path to the previous key in tag
 */
static BOOL hb_ntxTagPrevKey( LPTAGINFO pTag )
{
   int         iLevel = pTag->stackLevel - 1;
   LPPAGEINFO  pPage;
   ULONG       ulPage;

   if( iLevel >= 0 )
   {
      pPage    = hb_ntxPageLoad( pTag, pTag->stack[ iLevel ].page );
      if( ! pPage )
         return FALSE;
      ulPage   = hb_ntxGetKeyPage( pPage, pTag->stack[ iLevel ].ikey );
      if( ulPage )
      {
         hb_ntxPageRelease( pTag, pPage );
         pPage = hb_ntxPageBottomMove( pTag, ulPage );
         if( ! pPage )
            return FALSE;
      }
      else if( pTag->stack[ iLevel ].ikey )
      {
         pTag->stack[ iLevel ].ikey--;
      }
      else
      {
         while( --iLevel >= 0 )
         {
            hb_ntxPageRelease( pTag, pPage );
            pPage = hb_ntxPageLoad( pTag, pTag->stack[ iLevel ].page );
            if( ! pPage )
               return FALSE;
            if( pTag->stack[ iLevel ].ikey )
            {
               pTag->stack[ iLevel ].ikey--;
               break;
            }
         }
         if( iLevel < 0 )
         {
            hb_ntxPageRelease( pTag, pPage );
            return FALSE;
         }
         pTag->stackLevel = iLevel + 1;
      }
      hb_ntxPageGetKey( pPage, pTag->stack[ pTag->stackLevel - 1 ].ikey,
                        pTag->CurKeyInfo, pTag->KeyLength );
      hb_ntxPageRelease( pTag, pPage );
      return TRUE;
   }
   return FALSE;
}

/*
 * find a key value in page
 */
static int hb_ntxPageKeyFind( LPTAGINFO pTag, LPPAGEINFO pPage,
                              char * key, SHORT keylen, BOOL fNext,
                              ULONG ulRecNo, BOOL * fStop )
{
   SHORT iLast = -1, iBegin = 0, iEnd = pPage->uiKeys - 1, k, i;

   *fStop = FALSE;
   while( iBegin <= iEnd )
   {
      i  = ( iBegin + iEnd ) >> 1;
      k  = hb_ntxValCompare( pTag, key, keylen, hb_ntxGetKeyVal( pPage, i ),
                             pTag->KeyLength, FALSE );
      if( k == 0 )
      {
         if( ulRecNo != 0 && pTag->fSortRec )
         {
            ULONG ulRec = hb_ntxGetKeyRec( pPage, i );
            if( ulRecNo < ulRec )
               k = -1;
            else if( ulRecNo > ulRec )
               k = 1;
            else
            {
               *fStop = TRUE;
               return i;
            }
         }
      }
      else if( ! pTag->AscendKey )
         k = -k;
      if( fNext ? k >= 0 : k > 0 )
         iBegin = i + 1;
      else
      {
         if( k == 0 && ! ulRecNo )
            *fStop = TRUE;
         iLast = i;
         iEnd  = i - 1;
      }
   }
   return iLast >= 0 ? iLast : pPage->uiKeys;
}

/*
 * find a record in page starting from given key
 */
static BOOL hb_ntxPageFindRecNo( LPPAGEINFO pPage, int * iStart, ULONG ulRecno )
{
   int iKey = *iStart;

   while( iKey < pPage->uiKeys )
   {
      if( hb_ntxGetKeyRec( pPage, iKey ) == ulRecno )
      {
         *iStart = iKey;
         return TRUE;
      }
      iKey++;
   }
   return FALSE;
}

/*
 * set page path to given key in tag
 */
static BOOL hb_ntxTagKeyFind( LPTAGINFO pTag, LPKEYINFO pKey, USHORT uiLen )
{
   LPPAGEINFO  pPage    = NULL;
   ULONG       ulPage   = 0, ulRecNo = 0;
   int         iKey;
   BOOL        fStop    = FALSE, fNext = FALSE, fPrev = FALSE, fOut = FALSE;

   if( pKey->Tag == NTX_MAX_REC_NUM )          /* for key add */
   {
      if( pTag->fSortRec )
         ulRecNo = pKey->Xtra;
      else
         fNext = TRUE;
   }
   else if( pKey->Xtra == NTX_MAX_REC_NUM )     /* for seek last */
      fNext = fPrev = TRUE;
   else if( pKey->Xtra != NTX_IGNORE_REC_NUM )  /* for key del and current key */
      ulRecNo = pKey->Xtra;
   /* else -> normal seek */

   pTag->stackLevel = 0;
   do
   {
      if( pPage )
         hb_ntxPageRelease( pTag, pPage );
      pPage = hb_ntxPageLoad( pTag, ulPage );
      if( ! pPage )
         return FALSE;
      iKey  = hb_ntxPageKeyFind( pTag, pPage, pKey->key, uiLen, fNext, ulRecNo, &fStop );
      hb_ntxTagSetPageStack( pTag, pPage->Page, iKey );
      if( fStop && ulRecNo && pTag->fSortRec )
         break;
      ulPage = hb_ntxGetKeyPage( pPage, iKey );
   }
   while( ulPage != 0 );

   if( ulRecNo && ! pTag->fSortRec ) /* small hack - should speedup in some cases */
   {
      if( hb_ntxPageFindRecNo( pPage, &iKey, ulRecNo ) )
         pTag->stack[ pTag->stackLevel - 1 ].ikey = iKey;
   }

   hb_ntxPageGetKey( pPage, iKey, pTag->CurKeyInfo, pTag->KeyLength );
   hb_ntxPageRelease( pTag, pPage );

   if( ulRecNo )
   {
      if( ! pTag->fSortRec )
      {
         fStop = TRUE;
         while( fStop && ulRecNo != pTag->CurKeyInfo->Xtra )
         {
            if( ! hb_ntxTagNextKey( pTag ) ) /* Tag EOF */
            {
               fOut  = TRUE;
               fStop = FALSE;
            }
            else
            {
               fStop = hb_ntxValCompare( pTag, pKey->key, uiLen,
                                         pTag->CurKeyInfo->key, pTag->KeyLength,
                                         FALSE ) == 0;
            }
         }
      }
   }
   else if( fPrev )
   {
      if( ! hb_ntxTagPrevKey( pTag ) )
      {
         fOut  = TRUE;
         fStop = FALSE;
      }
      else
      {
         fStop = hb_ntxValCompare( pTag, pKey->key, uiLen, pTag->CurKeyInfo->key,
                                   pTag->KeyLength, FALSE ) == 0;
      }
   }
   else if( ! fNext && ! fStop && pTag->CurKeyInfo->Xtra == 0 )
   {
      if( ! hb_ntxTagNextKey( pTag ) ) /* Tag EOF */
      {
         fOut  = TRUE;
         fStop = FALSE;
      }
      else
      {
         fStop = hb_ntxValCompare( pTag, pKey->key, uiLen,
                                   pTag->CurKeyInfo->key, pTag->KeyLength,
                                   FALSE ) == 0;
      }
   }

   pTag->TagBOF = pTag->TagEOF = fOut || pTag->CurKeyInfo->Xtra == 0;

   return fStop;
}

/*
 * set key in the given tag page
 */
static void hb_ntxPageKeySet( LPTAGINFO pTag, LPPAGEINFO pPage, USHORT uiPos,
                              ULONG ulPage, ULONG ulRec, char * keyVal )
{
   hb_ntxSetKeyPage( pPage, uiPos, ulPage );
   hb_ntxSetKeyRec( pPage, uiPos, ulRec );
   HB_MEMCPY( hb_ntxGetKeyVal( pPage, uiPos ), keyVal, pTag->KeyLength );
   pPage->Changed = TRUE;
}

/*
 * add key to tag page
 */
static void hb_ntxPageKeyAdd( LPTAGINFO pTag, LPPAGEINFO pPage, USHORT uiPos,
                              ULONG ulPage, ULONG ulRec, char * keyVal )
{
   USHORT u, ntmp = hb_ntxGetKeyOffset( pPage, pPage->uiKeys + 1 );

   /* TODO?: update to keep last key pointer fixed */
   for( u = pPage->uiKeys + 1; u > uiPos; u-- )
   {
      hb_ntxSetKeyOffset( pPage, u, hb_ntxGetKeyOffset( pPage, u - 1 ) );
   }
   hb_ntxSetKeyOffset( pPage, uiPos, ntmp );
   pPage->uiKeys++;

   hb_ntxPageKeySet( pTag, pPage, uiPos, ulPage, ulRec, keyVal );
#ifdef HB_NTX_DEBUG
   hb_ntxPageCheckKeys( pPage, pTag, uiPos, 41 );
#endif
}

/*
 * del key from the page
 */
static void hb_ntxPageKeyDel( LPPAGEINFO pPage, USHORT uiPos )
{
   USHORT u, ntmp = hb_ntxGetKeyOffset( pPage, uiPos );

   /* TODO?: update to keep last key pointer fixed */
   for( u = uiPos; u < pPage->uiKeys; u++ )
      hb_ntxSetKeyOffset( pPage, u, hb_ntxGetKeyOffset( pPage, u + 1 ) );
   hb_ntxSetKeyOffset( pPage, pPage->uiKeys, ntmp );

   pPage->uiKeys--;
   pPage->Changed = TRUE;
}

/*
 * split single page into two and return key to the new one
 */
static LPKEYINFO hb_ntxPageSplit( LPTAGINFO pTag, LPPAGEINFO pPage,
                                  LPKEYINFO pKey, USHORT uiPos )
{
   LPPAGEINFO  pNewPage = hb_ntxPageNew( pTag, FALSE );
   LPKEYINFO   pKeyNew;
   USHORT      uiKeys   = pPage->uiKeys + 1, uiLen = pTag->KeyLength + 8,
               i, j, u, uiHalf;
   ULONG       ulPage;

   if( ! pNewPage )
      return NULL;
   pKeyNew  = hb_ntxKeyNew( NULL, pTag->KeyLength );

   uiHalf   = uiKeys >> 1;

   j        = 0;
   while( pNewPage->uiKeys < uiHalf )
   {
      if( pNewPage->uiKeys == uiPos )
      {
         hb_ntxSetKeyPage( pNewPage, pNewPage->uiKeys, pKey->Tag );
         hb_ntxSetKeyRec( pNewPage, pNewPage->uiKeys, pKey->Xtra );
         HB_MEMCPY( hb_ntxGetKeyVal( pNewPage, pNewPage->uiKeys ), pKey->key, pTag->KeyLength );
      }
      else
      {
         HB_MEMCPY( hb_ntxGetKeyPtr( pNewPage, pNewPage->uiKeys ),
                    hb_ntxGetKeyPtr( pPage, j ), uiLen );
         j++;
      }
      pNewPage->uiKeys++;
   }

   if( uiHalf == uiPos )
   {
      pKeyNew->Xtra = pKey->Xtra;
      HB_MEMCPY( pKeyNew->key, pKey->key, pTag->KeyLength );
      hb_ntxSetKeyPage( pNewPage, pNewPage->uiKeys, pKey->Tag );
   }
   else
   {
      pKeyNew->Xtra = hb_ntxGetKeyRec( pPage, j );
      HB_MEMCPY( pKeyNew->key, hb_ntxGetKeyVal( pPage, j ), pTag->KeyLength );
      hb_ntxSetKeyPage( pNewPage, pNewPage->uiKeys, hb_ntxGetKeyPage( pPage, j ) );
      j++;
   }
   pKeyNew->Tag   = pNewPage->Page;

   i              = 0;
   while( ++uiHalf < uiKeys )
   {
      if( uiHalf == uiPos )
      {
         hb_ntxSetKeyPage( pPage, i, pKey->Tag );
         hb_ntxSetKeyRec( pPage, i, pKey->Xtra );
         HB_MEMCPY( hb_ntxGetKeyVal( pPage, i ), pKey->key, pTag->KeyLength );
      }
      else
      {
         u = hb_ntxGetKeyOffset( pPage, j );
         hb_ntxSetKeyOffset( pPage, j, hb_ntxGetKeyOffset( pPage, i ) );
         hb_ntxSetKeyOffset( pPage, i, u );
         j++;
      }
      i++;
   }
   ulPage         = hb_ntxGetKeyPage( pPage, pPage->uiKeys );
   hb_ntxSetKeyPage( pPage, pPage->uiKeys, 0 );
   hb_ntxSetKeyPage( pPage, i, ulPage );
   pPage->uiKeys  = i;

   pPage->Changed = pNewPage->Changed = TRUE;
#ifdef HB_NTX_DEBUG
   hb_ntxPageCheckKeys( pNewPage, pTag, uiPos, 1 );
   hb_ntxPageCheckKeys( pPage, pTag, uiPos - pNewPage->uiKeys, 2 );
#endif
   hb_ntxPageRelease( pTag, pNewPage );

   return pKeyNew;
}

/*
 * join two neighbour pages and update the parent page key
 */
static void hb_ntxPageJoin( LPTAGINFO pTag, LPPAGEINFO pBasePage, USHORT uiPos,
                            LPPAGEINFO pFirst, LPPAGEINFO pLast )
{
   USHORT uiLen = pTag->KeyLength + 8, i;

   hb_ntxSetKeyRec( pFirst, pFirst->uiKeys, hb_ntxGetKeyRec( pBasePage, uiPos ) );
   HB_MEMCPY( hb_ntxGetKeyVal( pFirst, pFirst->uiKeys ),
              hb_ntxGetKeyVal( pBasePage, uiPos ), pTag->KeyLength );
   pFirst->uiKeys++;
   hb_ntxPageKeyDel( pBasePage, uiPos );
   hb_ntxSetKeyPage( pBasePage, uiPos, pFirst->Page );
   for( i = 0; i < pLast->uiKeys; i++ )
   {
      HB_MEMCPY( hb_ntxGetKeyPtr( pFirst, pFirst->uiKeys ),
                 hb_ntxGetKeyPtr( pLast, i ), uiLen );
      pFirst->uiKeys++;
   }
   hb_ntxSetKeyPage( pFirst, pFirst->uiKeys, hb_ntxGetKeyPage( pLast, pLast->uiKeys ) );
   pLast->uiKeys     = 0;
   hb_ntxPageFree( pTag, pLast );
   pFirst->Changed   = pLast->Changed = TRUE;
#ifdef HB_NTX_DEBUG
   hb_ntxPageCheckKeys( pBasePage, pTag, uiPos, 11 );
   hb_ntxPageCheckKeys( pFirst, pTag, 0, 12 );
#endif
}

/*
 * balance keys in two neighbour pages and update the parent page key
 */
static void hb_ntxBalancePages( LPTAGINFO pTag, LPPAGEINFO pBasePage, USHORT uiPos,
                                LPPAGEINFO pFirst, LPPAGEINFO pLast )
{
   USHORT   uiLen = pTag->KeyLength + 8, n;
   int      i, j, iMove = ( ( pFirst->uiKeys + pLast->uiKeys + 1 ) >> 1 ) - pFirst->uiKeys;

   /*
    * such situation should not exist even max keys, though it does not cost
    * much and I want to be able to call hb_ntxBalancePages in any case for
    * some advanced balancing
    */
   if( iMove == 0 )
      return;

#ifdef HB_NTX_DEBUG
   hb_ntxPageCheckKeys( pBasePage, pTag, uiPos, 31 );
   hb_ntxPageCheckKeys( pFirst, pTag, iMove, 32 );
   hb_ntxPageCheckKeys( pLast, pTag, iMove, 33 );
#endif

   if( iMove > 0 )
   {
      hb_ntxSetKeyRec( pFirst, pFirst->uiKeys, hb_ntxGetKeyRec( pBasePage, uiPos ) );
      HB_MEMCPY( hb_ntxGetKeyVal( pFirst, pFirst->uiKeys ),
                 hb_ntxGetKeyVal( pBasePage, uiPos ), pTag->KeyLength );
      pFirst->uiKeys++;
      i = 0;
      while( --iMove )
      {
         HB_MEMCPY( hb_ntxGetKeyPtr( pFirst, pFirst->uiKeys ),
                    hb_ntxGetKeyPtr( pLast, i ), uiLen );
         pFirst->uiKeys++;
         i++;
      }
      hb_ntxSetKeyRec( pBasePage, uiPos, hb_ntxGetKeyRec( pLast, i ) );
      HB_MEMCPY( hb_ntxGetKeyVal( pBasePage, uiPos ),
                 hb_ntxGetKeyVal( pLast, i ), pTag->KeyLength );
      hb_ntxSetKeyPage( pFirst, pFirst->uiKeys, hb_ntxGetKeyPage( pLast, i ) );
      i++;
      pLast->uiKeys -= i;
      /* TODO?: update to keep last key pointer fixed */
      for( j = 0; j <= pLast->uiKeys; j++ )
      {
         n = hb_ntxGetKeyOffset( pLast, j );
         hb_ntxSetKeyOffset( pLast, j, hb_ntxGetKeyOffset( pLast, j + i ) );
         hb_ntxSetKeyOffset( pLast, j + i, n );
      }
   }
   else
   {
      /* TODO?: update to keep last key pointer fixed */
      for( j = pLast->uiKeys; j >= 0; j-- )
      {
         n = hb_ntxGetKeyOffset( pLast, j - iMove );
         hb_ntxSetKeyOffset( pLast, j - iMove, hb_ntxGetKeyOffset( pLast, j ) );
         hb_ntxSetKeyOffset( pLast, j, n );
      }
      i = -iMove - 1;
      hb_ntxSetKeyRec( pLast, i, hb_ntxGetKeyRec( pBasePage, uiPos ) );
      HB_MEMCPY( hb_ntxGetKeyVal( pLast, i ),
                 hb_ntxGetKeyVal( pBasePage, uiPos ), pTag->KeyLength );
      hb_ntxSetKeyPage( pLast, i, hb_ntxGetKeyPage( pFirst, pFirst->uiKeys ) );
      while( --i >= 0 )
      {
         pFirst->uiKeys--;
         HB_MEMCPY( hb_ntxGetKeyPtr( pLast, i ),
                    hb_ntxGetKeyPtr( pFirst, pFirst->uiKeys ), uiLen );
      }
      pLast->uiKeys -= iMove;
      pFirst->uiKeys--;
      hb_ntxSetKeyRec( pBasePage, uiPos, hb_ntxGetKeyRec( pFirst, pFirst->uiKeys ) );
      HB_MEMCPY( hb_ntxGetKeyVal( pBasePage, uiPos ),
                 hb_ntxGetKeyVal( pFirst, pFirst->uiKeys ), pTag->KeyLength );
   }
   pFirst->Changed = pLast->Changed = pBasePage->Changed = TRUE;
#ifdef HB_NTX_DEBUG
   hb_ntxPageCheckKeys( pBasePage, pTag, uiPos, 21 );
   hb_ntxPageCheckKeys( pFirst, pTag, iMove, 22 );
   hb_ntxPageCheckKeys( pLast, pTag, iMove, 23 );
#endif
}

/*
 * add key to the index at the curret page path
 */
static BOOL hb_ntxTagKeyAdd( LPTAGINFO pTag, LPKEYINFO pKey )
{
   int         iLevel, iKey;
   LPPAGEINFO  pPage    = NULL;
   LPKEYINFO   pNewKey  = NULL;
   ULONG       ulPage;
   BOOL        fFound, fBottom = FALSE;

   if( pTag->UniqueKey )
   {
      ULONG ulRecNo = pKey->Xtra;

      pKey->Xtra  = NTX_IGNORE_REC_NUM;
      fFound      = hb_ntxTagKeyFind( pTag, pKey, pTag->KeyLength );
      pKey->Xtra  = ulRecNo;
      if( fFound )
         return FALSE;
      fBottom     = TRUE;
   }
   else
   {
      pKey->Tag   = NTX_MAX_REC_NUM;
      fFound      = hb_ntxTagKeyFind( pTag, pKey, pTag->KeyLength );
      pKey->Tag   = 0;
      if( fFound )
      {
         if( pTag->MultiKey )
            fBottom = TRUE;
         else
            return FALSE;
      }
   }

   iLevel = pTag->stackLevel - 1;
   if( fBottom )
   {
      pPage    = hb_ntxPageLoad( pTag, pTag->stack[ iLevel ].page );
      if( ! pPage )
         return FALSE;
      ulPage   = hb_ntxGetKeyPage( pPage, pTag->stack[ iLevel ].ikey );
      if( ulPage )
      {
         hb_ntxPageRelease( pTag, pPage );
         pPage    = hb_ntxPageBottomMove( pTag, ulPage );
         if( ! pPage )
            return FALSE;
         iLevel   = pTag->stackLevel - 1;
         if( pTag->stack[ iLevel ].ikey < pPage->uiKeys )
            pTag->stack[ iLevel ].ikey++;
      }
   }

   pTag->CurKeyInfo = hb_ntxKeyCopy( pTag->CurKeyInfo, pKey, pTag->KeyLength );

   while( iLevel >= 0 && pKey )
   {
      if( pPage )
         hb_ntxPageRelease( pTag, pPage );
      pPage = hb_ntxPageLoad( pTag, pTag->stack[ iLevel ].page );
      if( ! pPage )
      {
         if( pNewKey )
            hb_ntxKeyFree( pNewKey );
         pTag->stackLevel = 0;
         return FALSE;
      }
      iKey = pTag->stack[ iLevel ].ikey;
      if( pPage->uiKeys < pTag->MaxKeys )
      {
         hb_ntxPageKeyAdd( pTag, pPage, iKey, pKey->Tag, pKey->Xtra, pKey->key );
         pKey = NULL;
      }
      else
      {
         pTag->stackLevel = 0;
#if defined( HB_NTX_STRONG_BALANCE )
         if( iLevel > 0 )
         {
            LPPAGEINFO  pBasePage;
            USHORT      uiFirst, uiLast, uiBaseKey;
            pBasePage = hb_ntxPageLoad( pTag, pTag->stack[ iLevel - 1 ].page );
            if( ! pBasePage )
            {
               hb_ntxPageRelease( pTag, pPage );
               if( pNewKey )
                  hb_ntxKeyFree( pNewKey );
               return FALSE;
            }
            uiFirst = uiLast = uiBaseKey = pTag->stack[ iLevel - 1 ].ikey;
            if( uiLast < pBasePage->uiKeys && hb_ntxGetKeyPage( pBasePage, uiLast + 1 ) != 0 )
               uiLast++;
            else if( uiFirst > 0 && hb_ntxGetKeyPage( pBasePage, uiFirst - 1 ) != 0 )
               uiFirst--;
            if( uiFirst != uiLast )
            {
               LPPAGEINFO pFirst, pLast;

               if( uiFirst == uiBaseKey )
               {
                  pFirst   = pPage;
                  pLast    = hb_ntxPageLoad( pTag, hb_ntxGetKeyPage( pBasePage, uiLast ) );
                  if( ! pLast )
                  {
                     hb_ntxPageRelease( pTag, pPage );
                     hb_ntxPageRelease( pTag, pBasePage );
                     if( pNewKey )
                        hb_ntxKeyFree( pNewKey );
                     return FALSE;
                  }
                  uiBaseKey = iKey;
               }
               else
               {
                  pLast    = pPage;
                  pFirst   = hb_ntxPageLoad( pTag, hb_ntxGetKeyPage( pBasePage, uiFirst ) );
                  if( ! pFirst )
                  {
                     hb_ntxPageRelease( pTag, pPage );
                     hb_ntxPageRelease( pTag, pBasePage );
                     if( pNewKey )
                        hb_ntxKeyFree( pNewKey );
                     return FALSE;
                  }
                  uiBaseKey = pFirst->uiKeys + iKey + 1;
               }
               if( ( pFirst->uiKeys + pLast->uiKeys ) <= ( ( pTag->MaxKeys - 1 ) << 1 ) )
               {
                  hb_ntxBalancePages( pTag, pBasePage, uiFirst, pFirst, pLast );
                  if( pFirst->uiKeys >= uiBaseKey )
                     hb_ntxPageKeyAdd( pTag, pFirst, uiBaseKey, pKey->Tag, pKey->Xtra, pKey->key );
                  else
                     hb_ntxPageKeyAdd( pTag, pLast, uiBaseKey - pFirst->uiKeys - 1, pKey->Tag, pKey->Xtra, pKey->key );
                  pKey = NULL;
               }
               if( pFirst != pPage )
                  hb_ntxPageRelease( pTag, pFirst );
               else
                  hb_ntxPageRelease( pTag, pLast );
               hb_ntxPageRelease( pTag, pBasePage );
               if( ! pKey )
                  break;
            }
         }
#endif
         pKey     = hb_ntxPageSplit( pTag, pPage, pKey, iKey );
         if( pNewKey )
            hb_ntxKeyFree( pNewKey );
         pNewKey  = pKey;
      }
      iLevel--;
   }
   hb_ntxPageRelease( pTag, pPage );
   if( pKey )
   {
      pPage = hb_ntxPageNew( pTag, FALSE );
      if( ! pPage )
         return FALSE;
      hb_ntxPageKeyAdd( pTag, pPage, 0, pKey->Tag, pKey->Xtra, pKey->key );
      hb_ntxSetKeyPage( pPage, 1, pTag->RootBlock );
      pTag->RootBlock   = pPage->Page;
      pTag->HdrChanged  = TRUE;
      hb_ntxPageRelease( pTag, pPage );
      pTag->stackLevel  = 0;
   }
   if( pNewKey )
      hb_ntxKeyFree( pNewKey );
   return TRUE;
}

/*
 * del key at the curret page path from the index
 */
static BOOL hb_ntxTagKeyDel( LPTAGINFO pTag, LPKEYINFO pKey )
{
   int         iLevel, iBaseKey, iKey;
   LPPAGEINFO  pBasePage, pPage;
   ULONG       ulPage;

   pKey->Tag = 0;
   if( pTag->stackLevel == 0 || pTag->CurKeyInfo->Xtra != pKey->Xtra ||
       memcmp( pTag->CurKeyInfo->key, pKey->key, pTag->KeyLength ) != 0 )
   {
      if( ! hb_ntxTagKeyFind( pTag, pKey, pTag->KeyLength ) )
         return FALSE;
   }

   iLevel   = pTag->stackLevel - 1;

   pPage    = hb_ntxPageLoad( pTag, pTag->stack[ iLevel ].page );
   if( ! pPage )
      return FALSE;
   iKey     = pTag->stack[ iLevel ].ikey;
   ulPage   = hb_ntxGetKeyPage( pPage, iKey );

   if( ulPage )
   {
      pBasePage   = pPage;
      iBaseKey    = iKey;
      pPage       = hb_ntxPageBottomMove( pTag, ulPage );
      if( ! pPage )
      {
         hb_ntxPageRelease( pTag, pBasePage );
         return FALSE;
      }
      iLevel   = pTag->stackLevel - 1;
      iKey     = pTag->stack[ iLevel ].ikey;

      hb_ntxSetKeyRec( pBasePage, iBaseKey, hb_ntxGetKeyRec( pPage, iKey ) );
      HB_MEMCPY( hb_ntxGetKeyVal( pBasePage, iBaseKey ),
                 hb_ntxGetKeyVal( pPage, iKey ), pTag->KeyLength );
      pBasePage->Changed = TRUE;
#ifdef HB_NTX_DEBUG
      hb_ntxPageCheckKeys( pBasePage, pTag, iBaseKey, 61 );
#endif
      hb_ntxPageRelease( pTag, pBasePage );
   }
   hb_ntxPageKeyDel( pPage, iKey );

   while( iLevel > 0 )
   {
      if( pPage->uiKeys < ( pTag->MaxKeys >> 1 ) )
      {
         USHORT uiFirst, uiLast, uiBaseKey;

         pBasePage = hb_ntxPageLoad( pTag, pTag->stack[ iLevel - 1 ].page );
         if( ! pBasePage )
         {
            hb_ntxPageRelease( pTag, pPage );
            return FALSE;
         }
         uiFirst = uiLast = uiBaseKey = pTag->stack[ iLevel - 1 ].ikey;
         if( uiLast < pBasePage->uiKeys && hb_ntxGetKeyPage( pBasePage, uiLast + 1 ) != 0 )
            uiLast++;
         else if( uiFirst > 0 && hb_ntxGetKeyPage( pBasePage, uiFirst - 1 ) != 0 )
            uiFirst--;

         if( uiFirst == uiLast )
         {
            if( pPage->uiKeys == 0 )
            {
               hb_ntxSetKeyPage( pBasePage, uiBaseKey, 0 );
               hb_ntxPageFree( pTag, pPage );
            }
            hb_ntxPageRelease( pTag, pPage );
         }
         else
         {
            LPPAGEINFO pFirst, pLast;

            if( uiFirst == uiBaseKey )
            {
               pFirst   = pPage;
               pLast    = hb_ntxPageLoad( pTag, hb_ntxGetKeyPage( pBasePage, uiLast ) );
               if( ! pLast )
               {
                  hb_ntxPageRelease( pTag, pPage );
                  hb_ntxPageRelease( pTag, pBasePage );
                  pTag->stackLevel = 0;
                  return FALSE;
               }
            }
            else
            {
               pLast    = pPage;
               pFirst   = hb_ntxPageLoad( pTag, hb_ntxGetKeyPage( pBasePage, uiFirst ) );
               if( ! pFirst )
               {
                  hb_ntxPageRelease( pTag, pPage );
                  hb_ntxPageRelease( pTag, pBasePage );
                  pTag->stackLevel = 0;
                  return FALSE;
               }
            }
            if( pFirst->uiKeys + pLast->uiKeys < pTag->MaxKeys )
               hb_ntxPageJoin( pTag, pBasePage, uiFirst, pFirst, pLast );
            else
               hb_ntxBalancePages( pTag, pBasePage, uiFirst, pFirst, pLast );
            hb_ntxPageRelease( pTag, pFirst );
            hb_ntxPageRelease( pTag, pLast );
         }
         pPage = pBasePage;
      }
      else
         break;
      iLevel--;
   }

   if( pPage->uiKeys == 0 && pPage->Page == pTag->RootBlock )
   {
      ulPage = hb_ntxGetKeyPage( pPage, 0 );
      if( ulPage != 0 )
      {
         pTag->RootBlock   = ulPage;
         pTag->HdrChanged  = TRUE;
         hb_ntxPageFree( pTag, pPage );
      }
   }
   hb_ntxPageRelease( pTag, pPage );
   pTag->stackLevel = 0;
   return TRUE;
}

/*
 * refresh CurKey value and set proper path from RootPage to LeafPage
 */
static BOOL hb_ntxCurKeyRefresh( LPTAGINFO pTag )
{
   NTXAREAP pArea = pTag->Owner->Owner;

   if( pArea->dbfarea.lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( ! pArea->dbfarea.fPositioned )
   {
      pTag->stackLevel        = 0;
      pTag->TagBOF            = pTag->TagEOF = TRUE;
      pTag->CurKeyInfo->Xtra  = 0;
      return FALSE;
   }
   else if( pTag->stackLevel == 0 || pTag->CurKeyInfo->Xtra != pArea->dbfarea.ulRecNo )
   {
      BYTE        buf[ NTX_MAX_KEY ];
      BOOL        fBuf  = FALSE;
      LPKEYINFO   pKey  = NULL;
      /* Try to find previous if it's key for the same record */
      if( pTag->CurKeyInfo->Xtra == pArea->dbfarea.ulRecNo )
      {
         fBuf  = TRUE;
         HB_MEMCPY( buf, pTag->CurKeyInfo->key, pTag->KeyLength );
         pKey  = hb_ntxKeyCopy( pKey, pTag->CurKeyInfo, pTag->KeyLength );
         hb_ntxTagKeyFind( pTag, pKey, pTag->KeyLength );
      }
      if( pTag->CurKeyInfo->Xtra != pArea->dbfarea.ulRecNo )
      {
         BOOL fValidBuf = pArea->dbfarea.fValidBuffer;
         /* not found, create new key from DBF and if differs seek again */
         pKey = hb_ntxEvalKey( pKey, pTag );
         if( ! fBuf || memcmp( buf, pKey->key, pTag->KeyLength ) != 0 )
         {
            hb_ntxTagKeyFind( pTag, pKey, pTag->KeyLength );
         }
         /* not found, if key was generated from DBF buffer then force to
          * update it, create the new key and if differs seek again */
         if( pTag->CurKeyInfo->Xtra != pArea->dbfarea.ulRecNo && fValidBuf )
         {
            SELF_GOTO( ( AREAP ) pArea, pArea->dbfarea.ulRecNo );
            HB_MEMCPY( buf, pKey->key, pTag->KeyLength );
            pKey = hb_ntxEvalKey( pKey, pTag );
            if( memcmp( buf, pKey->key, pTag->KeyLength ) != 0 )
               hb_ntxTagKeyFind( pTag, pKey, pTag->KeyLength );
         }
      }
      hb_ntxKeyFree( pKey );
      return pTag->CurKeyInfo->Xtra != 0 && pTag->CurKeyInfo->Xtra == pArea->dbfarea.ulRecNo;
   }
   pTag->TagBOF = pTag->TagEOF = FALSE;
   return TRUE;
}

/*
 * Skip in tag respecting record filter only
 */
static void hb_ntxTagSkipFilter( LPTAGINFO pTag, BOOL fForward )
{
   BOOL fBack, fEof = fForward ? pTag->TagEOF : pTag->TagBOF;

   fBack = pTag->fUsrDescend == pTag->AscendKey ? fForward : ! fForward;

   while( ! fEof && ! hb_ntxCheckRecordScope( pTag->Owner->Owner,
                                              pTag->CurKeyInfo->Xtra ) )
   {
      if( fBack )
         fEof = ! hb_ntxTagPrevKey( pTag );
      else
         fEof = ! hb_ntxTagNextKey( pTag );

      if( ! fEof && ! hb_ntxKeyInScope( pTag, pTag->CurKeyInfo ) )
      {
         fEof = TRUE;
      }
   }
   if( fEof )
   {
      if( fForward )
         pTag->TagEOF = TRUE;
      else
         pTag->TagBOF = TRUE;
   }
}

/*
 * go to the first visiable record in Tag
 */
static void hb_ntxTagGoTop( LPTAGINFO pTag )
{
   PHB_NTXSCOPE pScope = pTag->fUsrDescend ? &pTag->bottom : &pTag->top;

   if( pScope->scopeKeyLen )
      hb_ntxTagKeyFind( pTag, pScope->scopeKey, pScope->scopeKeyLen );
   else if( pTag->fUsrDescend == pTag->AscendKey )
      hb_ntxTagBottomKey( pTag );
   else
      hb_ntxTagTopKey( pTag );

   pTag->TagEOF = pTag->CurKeyInfo->Xtra == 0 ||
                  ! hb_ntxKeyInScope( pTag, pTag->CurKeyInfo );

   if( ! pTag->TagEOF && pTag->Owner->Owner->dbfarea.area.dbfi.fFilter )
      hb_ntxTagSkipFilter( pTag, TRUE );

   pTag->TagBOF = pTag->TagEOF;
}

/*
 * go to the last visiable record in Tag
 */
static void hb_ntxTagGoBottom( LPTAGINFO pTag )
{
   PHB_NTXSCOPE pScope = pTag->fUsrDescend ? &pTag->top : &pTag->bottom;

   if( pScope->scopeKeyLen )
      hb_ntxTagKeyFind( pTag, pScope->scopeKey, pScope->scopeKeyLen );
   else if( pTag->fUsrDescend == pTag->AscendKey )
      hb_ntxTagTopKey( pTag );
   else
      hb_ntxTagBottomKey( pTag );

   pTag->TagBOF = pTag->CurKeyInfo->Xtra == 0 ||
                  ! hb_ntxKeyInScope( pTag, pTag->CurKeyInfo );

   if( ! pTag->TagBOF && pTag->Owner->Owner->dbfarea.area.dbfi.fFilter )
      hb_ntxTagSkipFilter( pTag, FALSE );

   pTag->TagEOF = pTag->TagBOF;
}

/*
 * skip to Next Key in the Tag
 */
static void hb_ntxTagSkipNext( LPTAGINFO pTag )
{
   pTag->TagBOF = FALSE;

   if( pTag->stackLevel == 0 )
      pTag->TagEOF = TRUE;
   else if( ! hb_ntxInTopScope( pTag, pTag->CurKeyInfo->key ) )
      hb_ntxTagGoTop( pTag );
   else if( pTag->fUsrDescend == pTag->AscendKey )
      pTag->TagEOF = ! hb_ntxTagPrevKey( pTag );
   else
      pTag->TagEOF = ! hb_ntxTagNextKey( pTag );

   if( ! pTag->TagEOF && ! hb_ntxKeyInScope( pTag, pTag->CurKeyInfo ) )
      pTag->TagEOF = TRUE;

   if( ! pTag->TagEOF && pTag->Owner->Owner->dbfarea.area.dbfi.fFilter )
      hb_ntxTagSkipFilter( pTag, TRUE );
}

/*
 * skip to Previous Key in the Tag
 */
static void hb_ntxTagSkipPrev( LPTAGINFO pTag )
{
   pTag->TagEOF = FALSE;

   if( pTag->stackLevel == 0 )
      /* TODO?: check if this is NTX behavior,
         for sure CDX works in such way */
      hb_ntxTagGoBottom( pTag );
   else if( pTag->fUsrDescend == pTag->AscendKey )
      pTag->TagBOF = ! hb_ntxTagNextKey( pTag );
   else
      pTag->TagBOF = ! hb_ntxTagPrevKey( pTag );

   if( ! pTag->TagBOF && ! hb_ntxKeyInScope( pTag, pTag->CurKeyInfo ) )
      pTag->TagBOF = TRUE;

   if( ! pTag->TagBOF && pTag->Owner->Owner->dbfarea.area.dbfi.fFilter )
      hb_ntxTagSkipFilter( pTag, FALSE );
}

/*
 * count keys in the given page and all subpages
 */
static ULONG hb_ntxPageCountKeys( LPTAGINFO pTag, ULONG ulPage )
{
   LPPAGEINFO  pPage = hb_ntxPageLoad( pTag, ulPage );
   ULONG       ulKeys;
   USHORT      u;

   if( ! pPage )
      return 0;

   ulKeys = pPage->uiKeys;
   for( u = 0; u <= pPage->uiKeys; u++ )
   {
      ulPage = hb_ntxGetKeyPage( pPage, u );
      if( ulPage )
         ulKeys += hb_ntxPageCountKeys( pTag, ulPage );
   }
   hb_ntxPageRelease( pTag, pPage );

   return ulKeys;
}

/*
 * count relative position of current location in page stack
 */
static double hb_ntxTagCountRelKeyPos( LPTAGINFO pTag )
{
   int      iLevel   = pTag->stackLevel, iKeys;
   double   dPos     = 1.0;

   while( --iLevel >= 0 )
   {
      LPPAGEINFO pPage = hb_ntxPageLoad( pTag, pTag->stack[ iLevel ].page );
      if( ! pPage )
         break;
      iKeys = pPage->uiKeys;
      if( hb_ntxGetKeyPage( pPage, pPage->uiKeys ) )
         ++iKeys;
      else if( iLevel == pTag->stackLevel - 1 )
         dPos = 0.5;
      if( iKeys )
         dPos = ( dPos + pTag->stack[ iLevel ].ikey ) / iKeys;
      hb_ntxPageRelease( pTag, pPage );
   }
   if( pTag->fUsrDescend == pTag->AscendKey )
      dPos = 1.0 - dPos;
   return dPos;
}

static void hb_ntxTagGoToRelKeyPos( LPTAGINFO pTag, double dPos )
{
   LPPAGEINFO  pPage    = NULL;
   ULONG       ulPage   = 0;
   int         iKey, iKeys;

   if( pTag->fUsrDescend == pTag->AscendKey )
      dPos = 1.0 - dPos;

   pTag->stackLevel = 0;
   do
   {
      if( pPage )
         hb_ntxPageRelease( pTag, pPage );
      pPage = hb_ntxPageLoad( pTag, ulPage );
      if( ! pPage )
      {
         pTag->stackLevel = 0;
         return;
      }
      if( pPage->uiKeys == 0 )
         iKey = 0;
      else
      {
         iKeys = pPage->uiKeys;
         if( hb_ntxGetKeyPage( pPage, pPage->uiKeys ) )
            ++iKeys;
         iKey  = ( int ) ( dPos * iKeys );
         if( iKey >= iKeys )
            iKey = iKeys - 1;
         dPos  = dPos * iKeys - iKey;
         if( dPos <= 0.0 )
            dPos = 0.0;
         else if( dPos >= 1.0 )
            dPos = 1.0;
      }
      hb_ntxTagSetPageStack( pTag, pPage->Page, iKey );
      ulPage = hb_ntxGetKeyPage( pPage, iKey );
   }
   while( ulPage != 0 );

   hb_ntxPageGetKey( pPage, iKey, pTag->CurKeyInfo, pTag->KeyLength );
   hb_ntxPageRelease( pTag, pPage );

   if( dPos > 0.75 )
      hb_ntxTagNextKey( pTag );
   else if( dPos < 0.25 )
      hb_ntxTagPrevKey( pTag );
}

/*
 * free pages allocated by tag
 */
static BOOL hb_ntxTagPagesFree( LPTAGINFO pTag, ULONG ulPage )
{
   LPPAGEINFO  pPage = hb_ntxPageLoad( pTag, ulPage );
   BOOL        fOK   = pPage != NULL;
   USHORT      u;

   for( u = 0; fOK && u <= pPage->uiKeys; u++ )
   {
      ulPage = hb_ntxGetKeyPage( pPage, u );
      if( ulPage )
         fOK = hb_ntxTagPagesFree( pTag, ulPage );
   }

   if( fOK )
   {
      pPage->uiKeys = 0;
      hb_ntxPageFree( pTag, pPage );
      if( ! pPage->pPrev )
         fOK = hb_ntxPageSave( pTag->Owner, pPage );
   }
   hb_ntxPageRelease( pTag, pPage );

   return fOK;
}

/*
 * free space allocated by tag
 */
static HB_ERRCODE hb_ntxTagSpaceFree( LPTAGINFO pTag )
{
   if( hb_ntxTagHeaderCheck( pTag ) )
   {
      if( pTag->RootBlock )
      {
         if( ! hb_ntxTagPagesFree( pTag, pTag->RootBlock ) )
            return HB_FAILURE;
      }
      hb_ntxPageAddFree( pTag, pTag->HeadBlock );
      hb_ntxIndexTagDel( pTag->Owner, pTag->TagName );
      pTag->Owner->Changed = TRUE;
   }
   hb_ntxTagDelete( pTag );
   return HB_SUCCESS;
}

/*
 * create index file name
 */
static void hb_ntxCreateFName( NTXAREAP pArea, const char * szBagName, BOOL * fProd,
                               char * szFileName, char * szTagName )
{
   PHB_FNAME   pFileName;
   PHB_ITEM    pExt  = NULL;
   BOOL        fName = szBagName && *szBagName;

   pFileName = hb_fsFNameSplit( fName ? szBagName : pArea->dbfarea.szDataFileName );

   if( szTagName )
   {
      if( pFileName->szName )
         hb_strncpyUpperTrim( szTagName, pFileName->szName, NTX_MAX_TAGNAME );
      else
         szTagName[ 0 ] = '\0';
   }

   if( ( hb_setGetDefExtension() && ! pFileName->szExtension ) || ! fName )
   {
      DBORDERINFO pExtInfo;
      memset( &pExtInfo, 0, sizeof( pExtInfo ) );
      pExt = pExtInfo.itmResult = hb_itemPutC( NULL, NULL );
      if( SELF_ORDINFO( ( AREAP ) pArea, DBOI_BAGEXT, &pExtInfo ) == HB_SUCCESS &&
          hb_itemGetCLen( pExt ) > 0 )
      {
         pFileName->szExtension = hb_itemGetCPtr( pExt );
      }
   }
   hb_fsFNameMerge( szFileName, pFileName );

   if( fProd )
   {
      if( ! pFileName->szName )
         *fProd = FALSE;
      else if( ! fName )
         *fProd = TRUE;
      else
      {
         PHB_FNAME pTableFileName = hb_fsFNameSplit( pArea->dbfarea.szDataFileName );

         *fProd = pTableFileName->szName &&
                  hb_stricmp( pTableFileName->szName, pFileName->szName ) == 0;
         if( *fProd && pFileName->szExtension && ! pExt )
         {
            DBORDERINFO pExtInfo;
            memset( &pExtInfo, 0, sizeof( pExtInfo ) );
            pExt = pExtInfo.itmResult = hb_itemPutC( NULL, NULL );
            if( SELF_ORDINFO( ( AREAP ) pArea, DBOI_BAGEXT, &pExtInfo ) == HB_SUCCESS )
            {
               *fProd = hb_stricmp( pFileName->szExtension,
                                    hb_itemGetCPtr( pExt ) ) == 0;
            }
         }
         hb_xfree( pTableFileName );
      }
   }
   hb_xfree( pFileName );
   if( pExt )
      hb_itemRelease( pExt );
}

/*
 * find order bag by its name
 */
static LPNTXINDEX hb_ntxFindBag( NTXAREAP pArea, const char * szBagName )
{
   LPNTXINDEX  pIndex;
   PHB_FNAME   pSeek, pName;
   BOOL        fFound;

   pSeek = hb_fsFNameSplit( szBagName );
   if( ! pSeek->szName )
      pSeek->szName = ( char * ) "";

   pIndex = pArea->lpIndexes;
   while( pIndex )
   {
      pName    = hb_fsFNameSplit( pIndex->IndexName );
      if( ! pName->szName )
         pName->szName = ( char * ) "";
      fFound   = ! hb_stricmp( pName->szName, pSeek->szName ) &&
                 ( ! pSeek->szPath || ( pName->szPath &&
                                        ! hb_stricmp( pName->szPath, pSeek->szPath ) ) ) &&
                 ( ! pSeek->szExtension || ( pName->szExtension &&
                                             ! hb_stricmp( pName->szExtension, pSeek->szExtension ) ) );
      hb_xfree( pName );
      if( fFound )
         break;
      pIndex = pIndex->pNext;
   }
   hb_xfree( pSeek );
   return pIndex;
}

/*
 * Find tag by name in index bag
 */
static int hb_ntxFindTagByName( LPNTXINDEX pIndex, const char * szTag )
{
   int i;

   for( i = 0; i < pIndex->iTags; i++ )
   {
      if( ! hb_strnicmp( pIndex->lpTags[ i ]->TagName, szTag,
                         NTX_MAX_TAGNAME ) )
         return i + 1;
   }
   return 0;
}

/*
 * Find the tag by its name or number
 */
static LPTAGINFO hb_ntxFindTag( NTXAREAP pArea, PHB_ITEM pTagItem,
                                PHB_ITEM pBagItem )
{
   LPNTXINDEX  pIndex;
   BOOL        fBag;

   if( ! pTagItem ||
       ( hb_itemType( pTagItem ) & ( HB_IT_STRING | HB_IT_NUMERIC ) ) == 0 )
      return pArea->lpCurTag;

   fBag = hb_itemGetCLen( pBagItem ) > 0;
   if( fBag )
   {
      if( hb_itemType( pTagItem ) & HB_IT_STRING )
         pIndex = hb_ntxFindBag( pArea, hb_itemGetCPtr( pBagItem ) );
      else
         pIndex = pArea->lpIndexes;
   }
   else
   {
      int iBag = hb_itemGetNI( pBagItem );

      pIndex = pArea->lpIndexes;
      if( iBag > 0 )
      {
         fBag = TRUE;
         while( pIndex )
         {
            if( --iBag == 0 )
               break;
            pIndex = pIndex->pNext;
         }
      }
      else if( iBag < 0 )
      {
         pIndex = NULL;
      }
   }
   if( pIndex )
   {
      if( hb_itemType( pTagItem ) & HB_IT_STRING )
      {
         const char *   szTag = hb_itemGetCPtr( pTagItem );
         int            iTag;

         if( fBag )
            iTag = hb_ntxFindTagByName( pIndex, szTag );
         else
         {
            do
            {
               iTag     = hb_ntxFindTagByName( pIndex, szTag );
               if( iTag )
                  break;
               pIndex   = pIndex->pNext;
            }
            while( pIndex );
         }
         if( iTag )
            return pIndex->lpTags[ iTag - 1 ];
      }
      else
      {
         int i = hb_itemGetNI( pTagItem ) - 1;

         if( i >= 0 )
         {
            if( fBag )
            {
               if( i < pIndex->iTags )
                  return pIndex->lpTags[ i ];
            }
            else
            {
               do
               {
                  if( i < pIndex->iTags )
                     return pIndex->lpTags[ i ];
                  i        -= pIndex->iTags;
                  pIndex   = pIndex->pNext;
               }
               while( pIndex );
            }
         }
      }
   }

   return NULL;
}

/*
 * find the given tag number
 */
static int hb_ntxFindTagNum( NTXAREAP pArea, LPTAGINFO pTag )
{
   if( pArea->fSetTagNumbers )
   {
      LPNTXINDEX  pIndex   = pArea->lpIndexes;
      USHORT      uiNum    = 0, i;

      pTag->uiNumber = 0;
      while( pIndex )
      {
         for( i = 0; i < pIndex->iTags; i++ )
         {
            pIndex->lpTags[ i ]->uiNumber = ++uiNum;
         }
         pIndex = pIndex->pNext;
      }
      pArea->fSetTagNumbers = FALSE;
   }
   return pTag->uiNumber;
}

/*
 * count number of keys in given tag
 */
static ULONG hb_ntxOrdKeyCount( LPTAGINFO pTag )
{
   ULONG ulKeyCount = 0;

   if( ! pTag->Owner->fShared && pTag->keyCount &&
       ! pTag->Owner->Owner->dbfarea.area.dbfi.fFilter )
      return pTag->keyCount;

   if( hb_ntxTagLockRead( pTag ) )
   {
      hb_ntxTagRefreshScope( pTag );

      if( pTag->top.scopeKeyLen || pTag->bottom.scopeKeyLen ||
          pTag->Owner->Owner->dbfarea.area.dbfi.fFilter )
      {
         hb_ntxTagGoTop( pTag );
         while( ! pTag->TagEOF )
         {
            ulKeyCount++;
            hb_ntxTagSkipNext( pTag );
         }
      }
      else
      {
         ulKeyCount = hb_ntxPageCountKeys( pTag, 0 );
      }
      if( ! pTag->Owner->Owner->dbfarea.area.dbfi.fFilter )
         pTag->keyCount = ulKeyCount;
      hb_ntxTagUnLockRead( pTag );
   }
   return ulKeyCount;
}

/*
 * get the logical key position in the given tag
 */
static ULONG hb_ntxOrdKeyNo( LPTAGINFO pTag )
{
   ULONG ulKeyNo = 0;

   if( hb_ntxTagLockRead( pTag ) )
   {
      hb_ntxTagRefreshScope( pTag );
      if( hb_ntxCurKeyRefresh( pTag ) )
      {
         if( pTag->top.scopeKeyLen || pTag->bottom.scopeKeyLen ||
             pTag->Owner->Owner->dbfarea.area.dbfi.fFilter )
         {
            if( hb_ntxKeyInScope( pTag, pTag->CurKeyInfo ) )
            {
               do
               {
                  ulKeyNo++;
                  hb_ntxTagSkipPrev( pTag );
               }
               while( ! pTag->TagBOF );
            }
         }
         else
         {
            int         iLevel   = pTag->stackLevel, iKey, iFirst = 1;
            BOOL        fBack    = pTag->fUsrDescend == pTag->AscendKey;
            LPPAGEINFO  pPage;
            ULONG       ulPage;

            while( --iLevel >= 0 )
            {
               pPage = hb_ntxPageLoad( pTag, pTag->stack[ iLevel ].page );
               if( ! pPage )
                  break;
               if( fBack )
               {
                  iKey     = pTag->stack[ iLevel ].ikey;
                  ulKeyNo  += pPage->uiKeys - iKey;
                  while( ++iKey <= pPage->uiKeys )
                  {
                     ulPage = hb_ntxGetKeyPage( pPage, iKey );
                     if( ulPage )
                        ulKeyNo += hb_ntxPageCountKeys( pTag, ulPage );
                  }
               }
               else
               {
                  ulKeyNo  += iKey = pTag->stack[ iLevel ].ikey + iFirst;
                  iFirst   = 0;
                  while( --iKey >= 0 )
                  {
                     ulPage = hb_ntxGetKeyPage( pPage, iKey );
                     if( ulPage )
                        ulKeyNo += hb_ntxPageCountKeys( pTag, ulPage );
                  }
               }
               hb_ntxPageRelease( pTag, pPage );
            }
         }
      }
      hb_ntxTagUnLockRead( pTag );
   }
   return ulKeyNo;
}

/*
 * set logical key position in given tag
 */
static BOOL hb_ntxOrdKeyGoto( LPTAGINFO pTag, ULONG ulKeyNo )
{
   NTXAREAP pArea = pTag->Owner->Owner;

   if( ! ulKeyNo || ! hb_ntxTagLockRead( pTag ) )
      return FALSE;

   hb_ntxTagRefreshScope( pTag );
   hb_ntxTagGoTop( pTag );
   while( ! pTag->TagEOF && --ulKeyNo )
   {
      hb_ntxTagSkipNext( pTag );
   }

   if( pTag->TagEOF )
   {
      SELF_GOTO( ( AREAP ) pArea, 0 );
   }
   else
   {
      LPTAGINFO pSavedTag = pArea->lpCurTag;
      pArea->lpCurTag   = pTag;
      if( SELF_GOTO( ( AREAP ) pArea, pTag->CurKeyInfo->Xtra ) == HB_SUCCESS )
         SELF_SKIPFILTER( ( AREAP ) pArea, 1 );
      pArea->lpCurTag   = pSavedTag;
   }
   hb_ntxTagUnLockRead( pTag );
   return TRUE;
}

/*
 * get the relative key position (from 0.0 to 1.0) in the given tag
 */
static double hb_ntxOrdGetRelKeyPos( LPTAGINFO pTag )
{
   double   dPos  = 0.0, dStart = 0.0, dStop = 1.0, dFact = 0.0000000000001;
   BOOL     fOK   = TRUE, fFilter = pTag->Owner->Owner->dbfarea.area.dbfi.fFilter;

   if( ! hb_ntxTagLockRead( pTag ) )
      return FALSE;

   hb_ntxTagRefreshScope( pTag );

   pTag->Owner->Owner->dbfarea.area.dbfi.fFilter = FALSE;
   if( pTag->fUsrDescend ? pTag->bottom.scopeKeyLen : pTag->top.scopeKeyLen )
   {
      hb_ntxTagGoTop( pTag );
      if( pTag->TagEOF )
         fOK = FALSE;
      else
         dStart = hb_ntxTagCountRelKeyPos( pTag );
   }
   if( fOK && ( pTag->fUsrDescend ? pTag->top.scopeKeyLen : pTag->bottom.scopeKeyLen ) )
   {
      hb_ntxTagGoBottom( pTag );
      if( pTag->TagBOF )
         fOK = FALSE;
      else
         dStop = hb_ntxTagCountRelKeyPos( pTag );
   }
   pTag->Owner->Owner->dbfarea.area.dbfi.fFilter = fFilter;

   if( fOK )
   {
      if( hb_ntxCurKeyRefresh( pTag ) &&
          hb_ntxKeyInScope( pTag, pTag->CurKeyInfo ) )
      {
         if( dStart >= dStop - dFact )
            dPos = 0.5;
         else
         {
            dPos  = hb_ntxTagCountRelKeyPos( pTag );
            dPos  = ( dPos - dStart ) / ( dStop - dStart );
            /* fix possible differences in FL representation */
            if( dPos <= 0.0 )
               dPos = 0.0;
            else if( dPos >= 1.0 )
               dPos = 1.0;
         }
      }
   }
   hb_ntxTagUnLockRead( pTag );

   return dPos;
}

/*
 * set the relative key position (from 0.0 to 1.0) in the given tag
 */
static void hb_ntxOrdSetRelKeyPos( LPTAGINFO pTag, double dPos )
{
   if( hb_ntxTagLockRead( pTag ) )
   {
      NTXAREAP pArea    = pTag->Owner->Owner;
      double   dStart   = 0.0, dStop = 1.0, dFact = 0.0000000000001;
      BOOL     fOK      = TRUE, fFilter = pArea->dbfarea.area.dbfi.fFilter;
      BOOL     fForward = TRUE, fTop = FALSE;

      hb_ntxTagRefreshScope( pTag );

      if( dPos >= 1.0 )
         fForward = FALSE;
      else if( dPos <= 0.0 )
         fTop = TRUE;
      else
      {
         pArea->dbfarea.area.dbfi.fFilter = FALSE;
         if( pTag->fUsrDescend ? pTag->bottom.scopeKeyLen : pTag->top.scopeKeyLen )
         {
            hb_ntxTagGoTop( pTag );
            if( pTag->TagEOF )
               fOK = FALSE;
            else
               dStart = hb_ntxTagCountRelKeyPos( pTag );
         }
         if( fOK && ( pTag->fUsrDescend ? pTag->top.scopeKeyLen : pTag->bottom.scopeKeyLen ) )
         {
            hb_ntxTagGoBottom( pTag );
            if( pTag->TagBOF )
               fOK = FALSE;
            else
               dStop = hb_ntxTagCountRelKeyPos( pTag );
         }
         pArea->dbfarea.area.dbfi.fFilter = fFilter;

         if( fOK )
         {
            if( dStart >= dStop - dFact )
            {
               fTop = TRUE;
            }
            else
            {
               dPos = dPos * ( dStop - dStart ) + dStart;
               hb_ntxTagGoToRelKeyPos( pTag, dPos );
               if( pTag->CurKeyInfo->Xtra == 0 )
                  fForward = FALSE;
               else if( ! hb_ntxInTopScope( pTag, pTag->CurKeyInfo->key ) )
                  fTop = TRUE;
               else if( ! hb_ntxInBottomScope( pTag, pTag->CurKeyInfo->key ) )
                  fForward = FALSE;
            }
         }
      }
      if( ! fOK )
      {
         SELF_GOTO( ( AREAP ) pArea, 0 );
      }
      else
      {
         LPTAGINFO pSavedTag = pArea->lpCurTag;
         pArea->lpCurTag            = pTag;

         pArea->dbfarea.area.fTop   = pArea->dbfarea.area.fBottom = FALSE;

         if( fForward )
         {
            if( fTop )
               hb_ntxTagGoTop( pTag );
            if( pTag->CurKeyInfo->Xtra != 0 )
            {
               if( SELF_GOTO( ( AREAP ) pArea, pTag->CurKeyInfo->Xtra ) == HB_SUCCESS )
               {
                  SELF_SKIPFILTER( ( AREAP ) pArea, 1 );
                  if( pArea->dbfarea.area.fEof && ! fTop )
                     fForward = FALSE;
               }
            }
            else if( fTop )
               SELF_GOTO( ( AREAP ) pArea, 0 );
            else
               fForward = FALSE;
         }
         if( ! fForward )
         {
            hb_ntxTagGoBottom( pTag );
            if( SELF_GOTO( ( AREAP ) pArea, pTag->CurKeyInfo->Xtra ) == HB_SUCCESS &&
                pTag->CurKeyInfo->Xtra != 0 )
            {
               pArea->dbfarea.area.fBottom = TRUE;
               SELF_SKIPFILTER( ( AREAP ) pArea, -1 );
            }
         }
         pArea->lpCurTag = pSavedTag;
      }
      hb_ntxTagUnLockRead( pTag );
   }
}

/*
 * skip to next/previous unique key
 */
static BOOL hb_ntxOrdSkipUnique( LPTAGINFO pTag, LONG lDir )
{
   NTXAREAP pArea = pTag->Owner->Owner;
   BOOL     fOut  = FALSE, fEof = FALSE, fForward = ( lDir >= 0 );

   if( pArea->dbfarea.lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   pArea->dbfarea.area.fTop = pArea->dbfarea.area.fBottom = FALSE;

   if( hb_ntxTagLockRead( pTag ) )
   {
      LPTAGINFO pSavedTag = pArea->lpCurTag;
      pArea->lpCurTag = pTag;

      hb_ntxTagRefreshScope( pTag );
      if( hb_ntxCurKeyRefresh( pTag ) )
      {
         char keyVal[ NTX_MAX_KEY ];
         HB_MEMCPY( keyVal, pTag->CurKeyInfo->key, pTag->KeyLength );

         do
         {
            if( fForward )
               hb_ntxTagSkipNext( pTag );
            else
               hb_ntxTagSkipPrev( pTag );
            fOut = pTag->TagEOF || pTag->TagBOF;
         }
         while( ! fOut && hb_ntxValCompare( pTag,
                                            pTag->CurKeyInfo->key, pTag->KeyLength,
                                            keyVal, pTag->KeyLength, TRUE ) == 0 );
      }
      else if( ! fForward && ! pArea->dbfarea.fPositioned )
      {
         hb_ntxTagGoBottom( pTag );
         fEof = pTag->TagEOF;
      }
      else
      {
         fOut = TRUE;
      }
      if( fOut )
      {
         if( fForward )
            fEof = TRUE;
         else
         {
            hb_ntxTagGoTop( pTag );
            fEof = pTag->TagEOF;
         }
      }
      hb_ntxTagUnLockRead( pTag );

      if( SELF_GOTO( ( AREAP ) pArea, fEof ? 0 : pTag->CurKeyInfo->Xtra ) == HB_SUCCESS &&
          ! fEof )
      {
         SELF_SKIPFILTER( ( AREAP ) pArea, ( fForward || fOut ) ? 1 : -1 );
         if( ! fForward && fOut )
            pArea->dbfarea.area.fBof = TRUE;
      }

      /* Update Bof and Eof flags */
      if( fForward )
         pArea->dbfarea.area.fBof = FALSE;
      else
         pArea->dbfarea.area.fEof = FALSE;

      pArea->lpCurTag = pSavedTag;
      return TRUE;
   }
   return FALSE;
}

/*
 * skip while code block doesn't return TRUE
 */
static BOOL hb_ntxOrdSkipEval( LPTAGINFO pTag, BOOL fForward, PHB_ITEM pEval )
{
   NTXAREAP pArea    = pTag->Owner->Owner;
   BOOL     fFound   = FALSE;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrdSkipEval(%p, %d, %p)", pTag, fForward, pEval ) );

   if( hb_itemType( pEval ) != HB_IT_BLOCK )
   {
      if( SELF_SKIP( ( AREAP ) pArea, fForward ? 1 : -1 ) != HB_SUCCESS )
         return FALSE;
      return fForward ? ! pArea->dbfarea.area.fEof : ! pArea->dbfarea.area.fBof;
   }

   if( pArea->dbfarea.lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   pArea->dbfarea.area.fTop = pArea->dbfarea.area.fBottom = FALSE;

   if( hb_ntxTagLockRead( pTag ) )
   {
      LPTAGINFO pSavedTag = pArea->lpCurTag;
      pArea->lpCurTag = pTag;

      hb_ntxTagRefreshScope( pTag );
      if( hb_ntxCurKeyRefresh( pTag ) )
      {
         if( fForward )
            hb_ntxTagSkipNext( pTag );
         else
            hb_ntxTagSkipPrev( pTag );

         while( fForward ? ! pTag->TagEOF : ! pTag->TagBOF )
         {
            if( SELF_GOTO( ( AREAP ) pArea, pTag->CurKeyInfo->Xtra ) != HB_SUCCESS )
               break;
            if( hb_ntxEvalSeekCond( pTag, pEval ) )
            {
               ULONG ulRecNo = pArea->dbfarea.ulRecNo;
               if( SELF_SKIPFILTER( ( AREAP ) pArea, fForward ? 1 : -1 ) != HB_SUCCESS ||
                   pArea->dbfarea.ulRecNo == ulRecNo || hb_ntxEvalSeekCond( pTag, pEval ) )
               {
                  fFound = TRUE;
                  break;
               }
            }
            if( fForward )
               hb_ntxTagSkipNext( pTag );
            else
               hb_ntxTagSkipPrev( pTag );
         }
         if( ! fFound )
         {
            if( fForward )
               SELF_GOTO( ( AREAP ) pArea, 0 );
            else
            {
               SELF_GOTOP( ( AREAP ) pArea );
               pArea->dbfarea.area.fBof = TRUE;
            }
         }
      }
      pArea->lpCurTag = pSavedTag;
      hb_ntxTagUnLockRead( pTag );
   }

   /* Update Bof and Eof flags */
   if( fForward )
      pArea->dbfarea.area.fBof = FALSE;
   else
      pArea->dbfarea.area.fEof = FALSE;

   return fFound;
}

/*
 * skip while code block doesn't return TRUE
 */
static BOOL hb_ntxOrdSkipWild( LPTAGINFO pTag, BOOL fForward, PHB_ITEM pWildItm )
{
   NTXAREAP       pArea    = pTag->Owner->Owner;
   const char *   szPattern;
   char *         szFree   = NULL;
   BOOL           fFound   = FALSE;
   int            iFixed   = 0;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrdSkipWild(%p, %d, %p)", pTag, fForward, pWildItm ) );

   szPattern = hb_itemGetCPtr( pWildItm );

   if( pTag->KeyType != 'C' || ! szPattern || ! *szPattern )
   {
      if( SELF_SKIP( ( AREAP ) pArea, fForward ? 1 : -1 ) != HB_SUCCESS )
         return FALSE;
      return fForward ? ! pArea->dbfarea.area.fEof : ! pArea->dbfarea.area.fBof;
   }

#ifndef HB_CDP_SUPPORT_OFF
   if( pArea->dbfarea.area.cdPage != hb_cdppage() )
   {
      szPattern = szFree = hb_cdpDup( szPattern, hb_cdppage(), pArea->dbfarea.area.cdPage );
   }
#endif
   while( iFixed < pTag->KeyLength && szPattern[ iFixed ] &&
          szPattern[ iFixed ] != '*' && szPattern[ iFixed ] != '?' )
   {
      ++iFixed;
   }

   if( pArea->dbfarea.lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   pArea->dbfarea.area.fTop = pArea->dbfarea.area.fBottom = FALSE;

   if( hb_ntxTagLockRead( pTag ) )
   {
      LPTAGINFO pSavedTag = pArea->lpCurTag;
      pArea->lpCurTag = pTag;

      hb_ntxTagRefreshScope( pTag );
      if( hb_ntxCurKeyRefresh( pTag ) )
      {
         int iStop = fForward ? -1 : 1;
         if( pTag->fUsrDescend )
            iStop = -iStop;
         if( iFixed && hb_ntxValCompare( pTag, szPattern, iFixed,
                                         pTag->CurKeyInfo->key, iFixed, FALSE ) == -iStop )
         {
            LPKEYINFO pKey;
            pKey                 = hb_ntxKeyNew( NULL, pTag->KeyLength );
            HB_MEMCPY( pKey->key, szPattern, iFixed );
            pKey->key[ iFixed ]  = '\0';
            pKey->Xtra           = pArea->lpCurTag->fUsrDescend ==
                                   pArea->lpCurTag->AscendKey ? NTX_MAX_REC_NUM :
                                   NTX_IGNORE_REC_NUM;
            if( ! hb_ntxTagKeyFind( pTag, pKey, iFixed ) )
            {
               if( fForward )
                  pTag->TagEOF = TRUE;
               else
                  pTag->TagBOF = TRUE;
            }
            hb_ntxKeyFree( pKey );
         }
         else if( fForward )
            hb_ntxTagSkipNext( pTag );
         else
            hb_ntxTagSkipPrev( pTag );

         while( fForward ? ! pTag->TagEOF : ! pTag->TagBOF )
         {
            if( hb_strMatchWild( pTag->CurKeyInfo->key, szPattern ) )
            {
               ULONG ulRecNo = pTag->CurKeyInfo->Xtra;
               if( SELF_GOTO( ( AREAP ) pArea, ulRecNo ) != HB_SUCCESS )
                  break;
               if( SELF_SKIPFILTER( ( AREAP ) pArea, fForward ? 1 : -1 ) != HB_SUCCESS ||
                   pArea->dbfarea.ulRecNo == ulRecNo ||
                   hb_strMatchWild( pTag->CurKeyInfo->key, szPattern ) )
               {
                  fFound = TRUE;
                  break;
               }
            }
            if( iFixed && hb_ntxValCompare( pTag, szPattern, iFixed,
                                            pTag->CurKeyInfo->key, iFixed, FALSE ) == iStop )
            {
               break;
            }
            if( fForward )
               hb_ntxTagSkipNext( pTag );
            else
               hb_ntxTagSkipPrev( pTag );
         }
         if( ! fFound )
         {
            if( fForward )
               SELF_GOTO( ( AREAP ) pArea, 0 );
            else
            {
               SELF_GOTOP( ( AREAP ) pArea );
               pArea->dbfarea.area.fBof = TRUE;
            }
         }
      }
      pArea->lpCurTag = pSavedTag;
      hb_ntxTagUnLockRead( pTag );
   }

   /* Update Bof and Eof flags */
   if( fForward )
      pArea->dbfarea.area.fBof = FALSE;
   else
      pArea->dbfarea.area.fEof = FALSE;

   if( szFree )
      hb_xfree( szFree );

   return fFound;
}

static BOOL hb_ntxRegexMatch( LPTAGINFO pTag, PHB_REGEX pRegEx, char * szKey )
{
   ULONG ulLen = pTag->KeyLength;

#ifndef HB_CDP_SUPPORT_OFF
   char  szBuff[ NTX_MAX_KEY + 1 ];

   if( pTag->Owner->Owner->dbfarea.area.cdPage != hb_cdppage() )
   {
      ulLen             = sizeof( szBuff ) - 1;
      hb_cdpnDup2( szKey, pTag->KeyLength, szBuff, ( HB_SIZE * ) &ulLen,
                   pTag->Owner->Owner->dbfarea.area.cdPage, hb_cdppage() );
      szBuff[ ulLen ]   = '\0';
      szKey             = szBuff;
   }
#else
   HB_SYMBOL_UNUSED( pTag );
#endif
   return hb_regexMatch( pRegEx, szKey, FALSE );
}

/*
 * skip while regular expression on index key val doesn't return TRUE
 */
static BOOL hb_ntxOrdSkipRegEx( LPTAGINFO pTag, BOOL fForward, PHB_ITEM pRegExItm )
{
   NTXAREAP    pArea    = pTag->Owner->Owner;
   BOOL        fFound   = FALSE;
   PHB_REGEX   pRegEx   = 0;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrdSkipRegEx(%p, %d, %p)", pTag, fForward, pRegExItm ) );

   if( pTag->KeyType != 'C' || ! hb_regexGet( pRegEx, pRegExItm, 0, 0 ) )
   {
      if( SELF_SKIP( ( AREAP ) pArea, fForward ? 1 : -1 ) != HB_SUCCESS )
         return FALSE;
      return fForward ? ! pArea->dbfarea.area.fEof : ! pArea->dbfarea.area.fBof;
   }

   if( pArea->dbfarea.lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   pArea->dbfarea.area.fTop = pArea->dbfarea.area.fBottom = FALSE;

   if( hb_ntxTagLockRead( pTag ) )
   {
      LPTAGINFO pSavedTag = pArea->lpCurTag;
      pArea->lpCurTag = pTag;

      hb_ntxTagRefreshScope( pTag );
      if( hb_ntxCurKeyRefresh( pTag ) )
      {
         if( fForward )
            hb_ntxTagSkipNext( pTag );
         else
            hb_ntxTagSkipPrev( pTag );

         while( fForward ? ! pTag->TagEOF : ! pTag->TagBOF )
         {
            if( SELF_GOTO( ( AREAP ) pArea, pTag->CurKeyInfo->Xtra ) != HB_SUCCESS )
               break;

            if( hb_ntxRegexMatch( pTag, pRegEx, ( char * ) pTag->CurKeyInfo->key ) )
            {
               ULONG ulRecNo = pArea->dbfarea.ulRecNo;
               if( SELF_SKIPFILTER( ( AREAP ) pArea, fForward ? 1 : -1 ) != HB_SUCCESS ||
                   pArea->dbfarea.ulRecNo == ulRecNo ||
                   hb_ntxRegexMatch( pTag, pRegEx, ( char * ) pTag->CurKeyInfo->key ) )
               {
                  fFound = TRUE;
                  break;
               }
            }
            if( fForward )
               hb_ntxTagSkipNext( pTag );
            else
               hb_ntxTagSkipPrev( pTag );
         }
         if( ! fFound )
         {
            if( fForward )
               SELF_GOTO( ( AREAP ) pArea, 0 );
            else
            {
               SELF_GOTOP( ( AREAP ) pArea );
               pArea->dbfarea.area.fBof = TRUE;
            }
         }
      }
      pArea->lpCurTag = pSavedTag;
      hb_ntxTagUnLockRead( pTag );
   }

   /* Update Bof and Eof flags */
   if( fForward )
      pArea->dbfarea.area.fBof = FALSE;
   else
      pArea->dbfarea.area.fEof = FALSE;

   hb_regexFree( pRegEx );

   return fFound;
}

/*
 * add key to custom tag (ordKeyAdd())
 * user key value is not implemented
 */
static BOOL hb_ntxOrdKeyAdd( LPTAGINFO pTag, PHB_ITEM pItem )
{
   NTXAREAP    pArea    = pTag->Owner->Owner;
   BOOL        fResult  = FALSE;
   LPKEYINFO   pKey;

   if( pArea->dbfarea.lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( ! pArea->dbfarea.fPositioned )
      return FALSE;

   if( pTag->pForItem && ! hb_ntxEvalCond( pArea, pTag->pForItem, TRUE ) )
      return FALSE;

   if( pTag->Template && pItem && hb_itemType( pItem ) != HB_IT_NIL )
   {
      pKey = hb_ntxKeyPutItem( NULL, pItem, pArea->dbfarea.ulRecNo, pTag, TRUE, NULL );
   }
   else
   {
      pKey = hb_ntxEvalKey( NULL, pTag );
   }

   if( hb_ntxTagLockWrite( pTag ) )
   {
      if( hb_ntxTagKeyAdd( pTag, pKey ) )
      {
         fResult = TRUE;
         if( ! pTag->Owner->fShared && pTag->keyCount &&
             hb_ntxKeyInScope( pTag, pKey ) )
            pTag->keyCount++;
      }
      hb_ntxTagUnLockWrite( pTag );
   }
   hb_ntxKeyFree( pKey );
   return fResult;
}

/*
 * del key from custom tag (ordKeyDel())
 * user key value is not implemented
 */
static BOOL hb_ntxOrdKeyDel( LPTAGINFO pTag, PHB_ITEM pItem )
{
   NTXAREAP    pArea    = pTag->Owner->Owner;
   BOOL        fResult  = FALSE;
   LPKEYINFO   pKey     = NULL;

   if( pArea->dbfarea.lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( ! pArea->dbfarea.fPositioned )
      return FALSE;

   if( pTag->pForItem && ! hb_ntxEvalCond( pArea, pTag->pForItem, TRUE ) )
      return FALSE;

   if( pTag->Template && pItem && hb_itemType( pItem ) != HB_IT_NIL )
   {
      pKey = hb_ntxKeyPutItem( NULL, pItem, pArea->dbfarea.ulRecNo, pTag, TRUE, NULL );
   }

   if( hb_ntxTagLockWrite( pTag ) )
   {
      if( pKey == NULL )
      {
         if( hb_ntxCurKeyRefresh( pTag ) )
            pKey = hb_ntxKeyCopy( NULL, pTag->CurKeyInfo, pTag->KeyLength );
         else
            pKey = hb_ntxEvalKey( NULL, pTag );
      }
      if( hb_ntxTagKeyDel( pTag, pKey ) )
      {
         fResult = TRUE;
         if( ! pTag->Owner->fShared && pTag->keyCount &&
             hb_ntxKeyInScope( pTag, pKey ) )
            pTag->keyCount--;
      }
      hb_ntxTagUnLockWrite( pTag );
   }
   hb_ntxKeyFree( pKey );
   return fResult;
}

/*
 * DBOI_FINDREC find a specific record in the tag - it's useful for
 * custom indexes when the same record can be stored more then once
 * or when the used index key is unknown
 */
static BOOL hb_ntxOrdFindRec( LPTAGINFO pTag, ULONG ulRecNo, BOOL fCont )
{
   NTXAREAP pArea    = pTag->Owner->Owner;
   BOOL     fFound   = FALSE;

   if( pTag && ulRecNo )
   {
      if( pArea->dbfarea.lpdbPendingRel && pArea->dbfarea.lpdbPendingRel->isScoped )
         SELF_FORCEREL( ( AREAP ) pArea );

      if( hb_ntxTagLockRead( pTag ) )
      {
         hb_ntxTagRefreshScope( pTag );
         if( fCont )
         {
            if( ! hb_ntxCurKeyRefresh( pTag ) )
               ulRecNo = 0;
            else
               hb_ntxTagSkipNext( pTag );
         }
         else
         {
            hb_ntxTagGoTop( pTag );
         }
         if( ulRecNo )
         {
            while( ! pTag->TagEOF )
            {
               if( pTag->CurKeyInfo->Xtra == ulRecNo )
               {
                  fFound = TRUE;
                  break;
               }
               hb_ntxTagSkipNext( pTag );
            }
         }
         hb_ntxTagUnLockRead( pTag );
      }
   }
   SELF_GOTO( ( AREAP ) pArea, fFound ? ulRecNo : 0 );
   return fFound;
}

/*
 * evaluate given C function in given scope
 */
static ULONG hb_ntxOrdScopeEval( LPTAGINFO pTag,
                                 HB_EVALSCOPE_FUNC pFunc, void * pParam,
                                 PHB_ITEM pItemLo, PHB_ITEM pItemHi )
{
   ULONG    ulCount  = 0, ulLen = ( ULONG ) pTag->KeyLength;
   PHB_ITEM pItemTop = hb_itemNew( NULL ), pItemBottom = hb_itemNew( NULL );

   hb_ntxTagGetScope( pTag, 0, pItemTop );
   hb_ntxTagGetScope( pTag, 1, pItemBottom );
   hb_ntxTagSetScope( pTag, 0, pItemLo );
   hb_ntxTagSetScope( pTag, 1, pItemHi );

   if( hb_ntxTagLockRead( pTag ) )
   {
      hb_ntxTagGoTop( pTag );
      while( ! pTag->TagEOF )
      {
         pFunc( pTag->CurKeyInfo->Xtra, ( BYTE * ) pTag->CurKeyInfo->key, ulLen, pParam );
         ulCount++;
         hb_ntxTagSkipNext( pTag );
      }
      hb_ntxTagUnLockRead( pTag );
   }

   hb_ntxTagSetScope( pTag, 0, pItemTop );
   hb_ntxTagSetScope( pTag, 1, pItemBottom );
   hb_itemRelease( pItemTop );
   hb_itemRelease( pItemBottom );

   return ulCount;
}

/* ************************************************************************* */
/* create index: hb_ntxTagCreate() */
/* ************************************************************************* */

static int hb_ntxQuickSortCompare( LPNTXSORTINFO pSort, BYTE * pKey1, BYTE * pKey2 )
{
   int iLen = pSort->keyLen, i;

   i = hb_ntxValCompare( pSort->pTag, ( char * ) pKey1, iLen, ( char * ) pKey2, iLen, TRUE );
   if( i == 0 )
   {
      if( pSort->pTag->fSortRec )
         i = ( HB_GET_LE_UINT32( pKey1 + iLen ) < HB_GET_LE_UINT32( pKey2 + iLen ) ) ? -1 : 1;
   }
   else if( ! pSort->pTag->AscendKey )
   {
      i = -i;
   }

   return i;
}

static BOOL hb_ntxQSort( LPNTXSORTINFO pSort, BYTE * pSrc, BYTE * pBuf, LONG lKeys )
{
   if( lKeys > 1 )
   {
      int      iLen = pSort->keyLen + 4;
      LONG     l1, l2;
      BYTE *   pPtr1, * pPtr2, * pDst;
      BOOL     f1, f2;

      l1    = lKeys >> 1;
      l2    = lKeys - l1;
      pPtr1 = &pSrc[ 0 ];
      pPtr2 = &pSrc[ l1 * iLen ];

      f1    = hb_ntxQSort( pSort, pPtr1, &pBuf[ 0 ], l1 );
      f2    = hb_ntxQSort( pSort, pPtr2, &pBuf[ l1 * iLen ], l2 );
      if( f1 )
      {
         pDst = pBuf;
      }
      else
      {
         pDst  = pSrc;
         pPtr1 = &pBuf[ 0 ];
      }
      if( ! f2 )
      {
         pPtr2 = &pBuf[ l1 * iLen ];
      }
      while( l1 > 0 && l2 > 0 )
      {
         if( hb_ntxQuickSortCompare( pSort, pPtr1, pPtr2 ) <= 0 )
         {
            HB_MEMCPY( pDst, pPtr1, iLen );
            pPtr1 += iLen;
            l1--;
         }
         else
         {
            HB_MEMCPY( pDst, pPtr2, iLen );
            pPtr2 += iLen;
            l2--;
         }
         pDst += iLen;
      }
      if( l1 > 0 )
      {
         HB_MEMCPY( pDst, pPtr1, iLen * l1 );
      }
      else if( l2 > 0 && f1 == f2 )
      {
         HB_MEMCPY( pDst, pPtr2, iLen * l2 );
      }
      return ! f1;
   }
   return TRUE;
}

static void hb_ntxSortSortPage( LPNTXSORTINFO pSort )
{
   ULONG ulSize = pSort->ulKeys * ( pSort->keyLen + 4 );

   if( ! hb_ntxQSort( pSort, pSort->pKeyPool, &pSort->pKeyPool[ ulSize ], pSort->ulKeys ) )
   {
      pSort->pStartKey = &pSort->pKeyPool[ ulSize ];
   }
   else
   {
      pSort->pStartKey = pSort->pKeyPool;
   }
}

static void hb_ntxSortBufferFlush( LPNTXSORTINFO pSort )
{
   ULONG ulSize;

   if( pSort->ulPagesIO )
   {
      LPNTXINDEX pIndex = pSort->pTag->Owner;
      ulSize = pSort->ulPagesIO * NTXBLOCKSIZE;
      if( hb_fileWriteAt( pIndex->DiskFile, pSort->pBuffIO, ulSize,
                          hb_ntxFileOffset( pIndex, pSort->ulFirstIO ) ) != ulSize )
      {
         hb_ntxErrorRT( pIndex->Owner, EG_WRITE, EDBF_WRITE,
                        pIndex->IndexName, hb_fsError(), 0, NULL );
      }
      pSort->ulPagesIO  = 0;
      pIndex->fFlush    = TRUE;
      if( pIndex->fShared )
         pIndex->Changed = TRUE;
   }
}

static void hb_ntxSortStorePage( LPNTXSORTINFO pSort, LPPAGEINFO pPage )
{
   LPNTXINDEX pIndex = pSort->pTag->Owner;

   if( ! pPage->Page )
   {
      pPage->Page = hb_ntxPageAlloc( pIndex );
      if( pSort->ulSizeIO )
      {
         if( pSort->ulPagesIO == pSort->ulSizeIO )
            hb_ntxSortBufferFlush( pSort );
         if( ! pSort->ulPagesIO ||
             hb_ntxFileOffset( pIndex, pSort->ulLastIO ) + NTXBLOCKSIZE ==
             hb_ntxFileOffset( pIndex, pPage->Page ) )
         {
            hb_ntxSetKeyCount( pPage, pPage->uiKeys );
            HB_MEMCPY( pSort->pBuffIO + pSort->ulPagesIO * NTXBLOCKSIZE,
                       hb_ntxPageBuffer( pPage ), NTXBLOCKSIZE );
            pSort->ulLastIO   = pPage->Page;
            if( ! pSort->ulPagesIO++ )
               pSort->ulFirstIO = pPage->Page;
            pPage->Changed    = FALSE;
            return;
         }
      }
   }
   if( ! pPage->pPrev )
      hb_ntxPageSave( pIndex, pPage );
}

static void hb_ntxSortAddNodeKey( LPNTXSORTINFO pSort, BYTE * pKeyVal, ULONG ulRec )
{
   LPPAGEINFO  pPage;
   ULONG       ulPage   = 0;
   int         iLevel   = 0;

   for(;; )
   {
      pPage = pSort->NodeList[ iLevel ];
      if( pPage == NULL )
      {
         pPage = pSort->NodeList[ iLevel ] = hb_ntxPageNew( pSort->pTag, TRUE );
         break;
      }
      else if( pPage->uiKeys >= pSort->pTag->MaxKeys )
      {
         hb_ntxSetKeyPage( pPage, pPage->uiKeys, ulPage );
         hb_ntxSortStorePage( pSort, pPage );
         ulPage                        = pPage->Page;
         hb_ntxPageRelease( pSort->pTag, pPage );
         pSort->NodeList[ iLevel++ ]   = hb_ntxPageNew( pSort->pTag, TRUE );
      }
      else
         break;
   }

   HB_MEMCPY( hb_ntxGetKeyVal( pPage, pPage->uiKeys ), pKeyVal, pSort->pTag->KeyLength );
   hb_ntxSetKeyRec( pPage, pPage->uiKeys, ulRec );
   hb_ntxSetKeyPage( pPage, pPage->uiKeys, ulPage );
   pPage->uiKeys++;
}

static void hb_ntxSortWritePage( LPNTXSORTINFO pSort )
{
   ULONG ulSize = pSort->ulKeys * ( pSort->keyLen + 4 );

   hb_ntxSortSortPage( pSort );

   if( pSort->hTempFile == FS_ERROR )
   {
      char szName[ HB_PATH_MAX ];
      pSort->hTempFile = hb_fsCreateTemp( NULL, NULL, FC_NORMAL, szName );
      if( pSort->hTempFile == FS_ERROR )
         hb_ntxErrorRT( pSort->pTag->Owner->Owner, EG_CREATE, EDBF_CREATE_TEMP,
                        szName, hb_fsError(), 0, NULL );
      else
         pSort->szTempFileName = hb_strdup( szName );
   }

   pSort->pSwapPage[ pSort->ulCurPage ].ulKeys = pSort->ulKeys;
   if( pSort->hTempFile != FS_ERROR )
   {
      pSort->pSwapPage[ pSort->ulCurPage ].nOffset = hb_fsSeekLarge( pSort->hTempFile, 0, FS_END );
      if( hb_fsWriteLarge( pSort->hTempFile, pSort->pStartKey, ulSize ) != ulSize )
         hb_ntxErrorRT( pSort->pTag->Owner->Owner, EG_WRITE, EDBF_WRITE_TEMP,
                        pSort->szTempFileName, hb_fsError(), 0, NULL );
   }
   else
      pSort->pSwapPage[ pSort->ulCurPage ].nOffset = 0;
   pSort->ulKeys = 0;
   pSort->ulCurPage++;
}

static void hb_ntxSortGetPageKey( LPNTXSORTINFO pSort, ULONG ulPage,
                                  BYTE ** pKeyVal, ULONG * pulRec )
{
   int iLen = pSort->keyLen;

   if( pSort->pSwapPage[ ulPage ].ulKeyBuf == 0 )
   {
      ULONG ulKeys   = HB_MIN( pSort->ulPgKeys, pSort->pSwapPage[ ulPage ].ulKeys );
      ULONG ulSize   = ulKeys * ( iLen + 4 );

      if( pSort->hTempFile != FS_ERROR &&
          ( hb_fsSeekLarge( pSort->hTempFile, pSort->pSwapPage[ ulPage ].nOffset, FS_SET ) != pSort->pSwapPage[ ulPage ].nOffset ||
            hb_fsReadLarge( pSort->hTempFile, pSort->pSwapPage[ ulPage ].pKeyPool, ulSize ) != ulSize ) )
      {
         hb_ntxErrorRT( pSort->pTag->Owner->Owner, EG_READ, EDBF_READ_TEMP,
                        pSort->szTempFileName, hb_fsError(), 0, NULL );
      }
      pSort->pSwapPage[ ulPage ].nOffset  += ulSize;
      pSort->pSwapPage[ ulPage ].ulKeyBuf = ulKeys;
      pSort->pSwapPage[ ulPage ].ulCurKey = 0;
   }
   *pKeyVal = &pSort->pSwapPage[ ulPage ].pKeyPool[ pSort->pSwapPage[ ulPage ].ulCurKey * ( iLen + 4 ) ];
   *pulRec  = HB_GET_LE_UINT32( *pKeyVal + iLen );
}

static void hb_ntxSortOrderPages( LPNTXSORTINFO pSort )
{
   int      iLen  = pSort->keyLen, i;
   LONG     l, r, m;
   ULONG    n, ulPage, ulRec;
   BYTE *   pKey  = NULL, * pTmp;

   pSort->ulFirst             = 0;
   pSort->pSortedPages        = ( ULONG * ) hb_xgrab( pSort->ulPages * sizeof( ULONG ) );
   pSort->pSortedPages[ 0 ]   = 0;

   if( pSort->ulTotKeys > 0 )
   {
      for( n = 0; n < pSort->ulPages; n++ )
      {
         hb_ntxSortGetPageKey( pSort, n, &pKey, &ulRec );
         l  = 0;
         r  = n - 1;
         while( l <= r )
         {
            m        = ( l + r ) >> 1;
            ulPage   = pSort->pSortedPages[ m ];
            pTmp     = &pSort->pSwapPage[ ulPage ].pKeyPool[ pSort->pSwapPage[ ulPage ].ulCurKey * ( iLen + 4 ) ];
            i        = hb_ntxValCompare( pSort->pTag, ( char * ) pKey, iLen, ( char * ) pTmp, iLen, TRUE );
            if( i == 0 )
            {
               if( pSort->pTag->fSortRec )
                  i = ( ulRec < HB_GET_LE_UINT32( &pTmp[ iLen ] ) ) ? -1 : 1;
            }
            else if( ! pSort->pTag->AscendKey )
               i = -i;
            if( i >= 0 )
               l = m + 1;
            else
               r = m - 1;
         }
         for( r = n; r > l; r-- )
            pSort->pSortedPages[ r ] = pSort->pSortedPages[ r - 1 ];
         pSort->pSortedPages[ l ] = n;
      }
   }
}

static BOOL hb_ntxSortKeyGet( LPNTXSORTINFO pSort, BYTE ** pKeyVal, ULONG * pulRec )
{
   int   iLen = pSort->keyLen, i;
   LONG  l, r, m;
   ULONG ulPage;

   ulPage = pSort->pSortedPages[ pSort->ulFirst ];

   /* check if first page has some keys yet */
   if( pSort->pSwapPage[ ulPage ].ulKeys > 0 )
   {
      BYTE *   pKey, * pTmp;
      ULONG    ulRec, ulPg;

      /*
       * last key was taken from this page - we have to resort it.
       * This is done intentionally here to be sure that the key
       * value return by this function will not be overwritten by
       * next keys in page read from temporary file in function
       * hb_ntxSortGetPageKey() - please do not move this part down
       * even it seems to be correct
       */
      hb_ntxSortGetPageKey( pSort, ulPage, &pKey, &ulRec );

      l  = pSort->ulFirst + 1;
      r  = pSort->ulPages - 1;
      while( l <= r )
      {
         m     = ( l + r ) >> 1;
         ulPg  = pSort->pSortedPages[ m ];
         pTmp  = &pSort->pSwapPage[ ulPg ].pKeyPool[ pSort->pSwapPage[ ulPg ].ulCurKey * ( iLen + 4 ) ];
         i     = hb_ntxValCompare( pSort->pTag, ( char * ) pKey, iLen, ( char * ) pTmp, iLen, TRUE );
         if( i == 0 )
         {
            if( pSort->pTag->fSortRec )
               i = ( ulRec < HB_GET_LE_UINT32( &pTmp[ iLen ] ) ) ? -1 : 1;
            else
               i = ( ulPage < ulPg ) ? -1 : 1;
         }
         else if( ! pSort->pTag->AscendKey )
            i = -i;
         if( i > 0 )
            l = m + 1;
         else
            r = m - 1;
      }
      if( l > ( LONG ) pSort->ulFirst + 1 )
      {
         ulPage                        = pSort->pSortedPages[ pSort->ulFirst ];
         for( r = pSort->ulFirst + 1; r < l; r++ )
            pSort->pSortedPages[ r - 1 ] = pSort->pSortedPages[ r ];
         pSort->pSortedPages[ l - 1 ]  = ulPage;
      }
   }
   else
   {
      pSort->ulFirst++;
   }
   if( pSort->ulFirst < pSort->ulPages )
   {
      ulPage = pSort->pSortedPages[ pSort->ulFirst ];
      hb_ntxSortGetPageKey( pSort, ulPage, pKeyVal, pulRec );
      pSort->pSwapPage[ ulPage ].ulCurKey++;
      pSort->pSwapPage[ ulPage ].ulKeys--;
      pSort->pSwapPage[ ulPage ].ulKeyBuf--;
      return TRUE;
   }

   *pKeyVal = NULL;
   *pulRec  = 0;

   return FALSE;
}

static void hb_ntxSortKeyAdd( LPNTXSORTINFO pSort, ULONG ulRec, const char * pKeyVal, int iKeyLen )
{
   int      iLen = pSort->keyLen;
   BYTE *   pDst;

   if( pSort->ulKeys >= pSort->ulPgKeys )
   {
      hb_ntxSortWritePage( pSort );
   }
   pDst = &pSort->pKeyPool[ pSort->ulKeys * ( iLen + 4 ) ];

   if( iLen > iKeyLen )
   {
      HB_MEMCPY( pDst, pKeyVal, iKeyLen );
      memset( &pDst[ iKeyLen ], ' ', iLen - iKeyLen );
   }
   else
   {
      HB_MEMCPY( pDst, pKeyVal, iLen );
   }
   HB_PUT_LE_UINT32( &pDst[ iLen ], ulRec );
   pSort->ulKeys++;
   pSort->ulTotKeys++;
}

static LPNTXSORTINFO hb_ntxSortNew( LPTAGINFO pTag, ULONG ulRecCount )
{
   LPNTXSORTINFO  pSort;
   BYTE *         pBuf;
   int            iLen = pTag->KeyLength;
   ULONG          ulSize, ulMax, ulMin;

   if( ulRecCount == 0 )
      ulRecCount = 1;

   pSort    = ( LPNTXSORTINFO ) hb_xgrab( sizeof( NTXSORTINFO ) );
   memset( pSort, 0, sizeof( NTXSORTINFO ) );

   ulMin    = ( ULONG ) ceil( sqrt( ( double ) ulRecCount ) );
   ulMax    = ( ( ULONG ) ceil( sqrt( ( double ) ulRecCount / ( iLen + 4 ) ) ) ) << 7;
   /*
    * this effectively increase allocated memory buffer for very large files
    * moving the maximum to: 270'566'400 for 4'294'967'295 records and 256
    * index key length.
    * if you want to force smaller buffer I wrote below then add here:
    * ulMax = ulMin;
    */
   ulSize   = ( 1L << 20 ) / ( iLen + 4 );
   while( ulMax < ulSize )
      ulMax <<= 1;
   if( ulMax > ulRecCount )
      ulMax = ulRecCount;

   do
   {
      ulSize   = ulMax * ( iLen + 4 );
      pBuf     = ( BYTE * ) hb_xalloc( ulSize << 2 );
      if( pBuf )
      {
         hb_xfree( pBuf );
         pBuf = ( BYTE * ) hb_xalloc( ulSize << 1 );
      }
      else
      {
         ulMax >>= 1;
      }
   }
   while( ! pBuf && ulMax >= ulMin );

   if( ! pBuf )
   {
      /* call hb_xgrab() to force out of memory error,
       * though in multi process environment this call may return
       * with success when other process free some memory
       * (also the size of buf is reduced to absolute minimum).
       * Sorry but I'm to lazy to implement indexing with smaller
       * memory though it's possible - just simply I can even create
       * index on-line by key adding like in normal update process.
       * The memory necessary to index file is now ~
       *    ~ (keySize+4+sizeof(NTXSWAPPAGE)) * sqrt(ulRecCount) * 2
       * so the maximum is for DBF with 2^32 records and keySize 256 ~
       * ~ 2^17 * 284 ~=~ 37 Mb
       * this is not a problem for current computers and I do not see
       * any way to use DBFs with four billions records and indexes with
       * such long (256 bytes) keys on the old ones - they will be simply
       * to slow. IMHO it's also better to signal out of memory here and
       * force some system upgrades then run process which will have to
       * take many hours, Druzus.
       */
      ulMax = ulMin;
      pBuf  = ( BYTE * ) hb_xgrab( ( ulMax << 1 ) * ( iLen + 4 ) );
   }

   pSort->pTag       = pTag;
   pSort->hTempFile  = FS_ERROR;
   pSort->keyLen     = iLen;
   pSort->fUnique    = pTag->UniqueKey;
   pSort->ulMaxKey   = ulMax << 1;
   pSort->ulPgKeys   = ulMax;
   pSort->ulMaxRec   = ulRecCount;
   pSort->pKeyPool   = pBuf;
   pSort->ulPages    = ( ulRecCount + pSort->ulPgKeys - 1 ) / pSort->ulPgKeys;
   /* check for overflow on 32 bit machines when number of records is nearly 2^32 */
   if( ! pSort->ulPages )
      pSort->ulPages = ulRecCount / pSort->ulPgKeys + 1;
   pSort->pSwapPage = ( LPNTXSWAPPAGE ) hb_xgrab( sizeof( NTXSWAPPAGE ) * pSort->ulPages );
   memset( pSort->pSwapPage, 0, sizeof( NTXSWAPPAGE ) * pSort->ulPages );
   return pSort;
}

static void hb_ntxSortFree( LPNTXSORTINFO pSort, BOOL fFull )
{
   if( pSort->hTempFile != FS_ERROR )
   {
      hb_fsClose( pSort->hTempFile );
      pSort->hTempFile = FS_ERROR;
   }
   if( pSort->szTempFileName )
   {
      hb_fsDelete( pSort->szTempFileName );
      hb_xfree( pSort->szTempFileName );
      pSort->szTempFileName = NULL;
   }
   if( pSort->pKeyPool )
   {
      hb_xfree( pSort->pKeyPool );
      pSort->pKeyPool = NULL;
   }
   if( pSort->pSwapPage )
   {
      hb_xfree( pSort->pSwapPage );
      pSort->pSwapPage = NULL;
   }
   if( pSort->pBuffIO )
   {
      hb_xfree( pSort->pBuffIO );
      pSort->pBuffIO = NULL;
   }
   if( pSort->pSortedPages )
   {
      hb_xfree( pSort->pSortedPages );
      pSort->pSortedPages = NULL;
   }
   if( fFull )
   {
      hb_xfree( pSort );
   }
}

static void hb_ntxSortOut( LPNTXSORTINFO pSort )
{
   BOOL        fUnique  = pSort->fUnique, fBalance, fNext;
   LPTAGINFO   pTag     = pSort->pTag;
   ULONG       ulPage, ulRec, ulKey;
   USHORT      uiHalf;
   BYTE *      pKeyVal;
   int         iLen = pSort->keyLen, iLevel;

   pSort->ulPages    = pSort->ulCurPage + 1;
   pSort->ulPgKeys   = pSort->ulMaxKey / pSort->ulPages;
   if( pSort->ulPages > 1 )
   {
      BYTE * pBuf = pSort->pKeyPool;
      hb_ntxSortWritePage( pSort );
      for( ulPage = 0; ulPage < pSort->ulPages; ulPage++ )
      {
         pSort->pSwapPage[ ulPage ].ulKeyBuf = 0;
         pSort->pSwapPage[ ulPage ].ulCurKey = 0;
         pSort->pSwapPage[ ulPage ].pKeyPool = pBuf;
         pBuf                                += pSort->ulPgKeys * ( pSort->keyLen + 4 );
      }
   }
   else
   {
      hb_ntxSortSortPage( pSort );
      pSort->pSwapPage[ 0 ].ulKeys     = pSort->ulKeys;
      pSort->pSwapPage[ 0 ].ulKeyBuf   = pSort->ulKeys;
      pSort->pSwapPage[ 0 ].ulCurKey   = 0;
      pSort->pSwapPage[ 0 ].pKeyPool   = pSort->pStartKey;
   }
   /* printf("pSort->ulPages=%ld, pSort->ulPgKeys=%ld", pSort->ulPages, pSort->ulPgKeys);fflush(stdout); */

   hb_ntxSortOrderPages( pSort );

   if( hb_vmRequestQuery() != 0 )
      return;

   for( ulKey = 0; ulKey < pSort->ulTotKeys; ulKey++ )
   {
      if( ! hb_ntxSortKeyGet( pSort, &pKeyVal, &ulRec ) )
      {
         if( hb_vmRequestQuery() != 0 )
            return;
         hb_errInternal( 9309, "hb_ntxSortOut: memory structure corrupted.", NULL, NULL );
      }
      if( fUnique )
      {
         if( ulKey != 0 && hb_ntxValCompare( pTag, ( char * ) pSort->pLastKey, iLen, ( char * ) pKeyVal, iLen, TRUE ) == 0 )
         {
            continue;
         }
#ifndef HB_NTX_DEBUG_EXT
         else
         {
            HB_MEMCPY( pSort->pLastKey, pKeyVal, iLen );
         }
#endif
      }
#ifdef HB_NTX_DEBUG_EXT
      if( ulKey != 0 )
      {
         int i = hb_ntxValCompare( pTag, ( char * ) pSort->pLastKey, iLen, ( char * ) pKeyVal, iLen, TRUE );
         if( ! pTag->AscendKey )
            i = -i;
         if( i == 0 )
            i = ( pSort->ulLastRec < ulRec ) ? -1 : 1;
         if( i > 0 )
         {
            printf( "\r\nulKey=%ld, pKeyVal=[%s][%ld], pKeyLast=[%s][%ld]\r\n",
                    ulKey, pKeyVal, ulRec, pSort->pLastKey, pSort->ulLastRec ); fflush( stdout );
            if( hb_vmRequestQuery() != 0 )
               return;
            hb_errInternal( 9310, "hb_ntxSortOut: sorting fails.", NULL, NULL );
         }
      }
      HB_MEMCPY( pSort->pLastKey, pKeyVal, iLen );
      pSort->ulLastRec = ulRec;
#endif
      hb_ntxSortAddNodeKey( pSort, pKeyVal, ulRec );
   }

#ifdef HB_NTX_DEBUG
   if( hb_ntxSortKeyGet( pSort, &pKeyVal, &ulRec ) )
   {
      if( hb_vmRequestQuery() != 0 )
         return;
      hb_errInternal( 9311, "hb_ntxSortOut: memory structure corrupted(2).", NULL, NULL );
   }
#endif

   if( pSort->NodeList[ 0 ] == NULL )
   {
      pSort->NodeList[ 0 ] = hb_ntxPageNew( pTag, TRUE );
   }
   hb_ntxSetKeyPage( pSort->NodeList[ 0 ], pSort->NodeList[ 0 ]->uiKeys, 0 );

   iLevel   = 0;
   fNext    = TRUE;
   fBalance = FALSE;
   uiHalf   = pTag->MaxKeys >> 1;
   do
   {
      hb_ntxSortStorePage( pSort, pSort->NodeList[ iLevel ] );
      if( iLevel + 1 == NTX_STACKSIZE || pSort->NodeList[ iLevel + 1 ] == NULL )
      {
         pTag->RootBlock   = pSort->NodeList[ iLevel ]->Page;
         fNext             = FALSE;
      }
      else
      {
         hb_ntxSetKeyPage( pSort->NodeList[ iLevel + 1 ],
                           pSort->NodeList[ iLevel + 1 ]->uiKeys,
                           pSort->NodeList[ iLevel ]->Page );
         if( pSort->NodeList[ iLevel ]->uiKeys < uiHalf )
         {
            fBalance = TRUE;
         }
      }
      hb_ntxPageRelease( pTag, pSort->NodeList[ iLevel ] );
      iLevel++;
   }
   while( fNext );

   hb_ntxSortBufferFlush( pSort );
   hb_ntxSortFree( pSort, FALSE );

   if( fBalance )
   {
      LPPAGEINFO pPage, pFirst, pLast;

      ulPage = pTag->RootBlock;
      while( ulPage )
      {
         pPage    = hb_ntxPageLoad( pTag, ulPage );
         if( ! pPage )
            return;
         ulPage   = hb_ntxGetKeyPage( pPage, pPage->uiKeys );
         if( ulPage && pPage->uiKeys )
         {
            pLast = hb_ntxPageLoad( pTag, ulPage );
            if( ! pLast )
            {
               hb_ntxPageRelease( pTag, pPage );
               return;
            }
            if( pLast->uiKeys < uiHalf )
            {
               pFirst = hb_ntxPageLoad( pTag, hb_ntxGetKeyPage( pPage,
                                                                pPage->uiKeys - 1 ) );
               if( ! pFirst )
               {
                  hb_ntxPageRelease( pTag, pPage );
                  hb_ntxPageRelease( pTag, pLast );
                  return;
               }
               hb_ntxBalancePages( pTag, pPage, pPage->uiKeys - 1, pFirst, pLast );
               hb_ntxPageRelease( pTag, pFirst );
            }
            hb_ntxPageRelease( pTag, pLast );
         }
         hb_ntxPageRelease( pTag, pPage );
      }
   }
}

/*
 * create tag in index file
 */
static HB_ERRCODE hb_ntxTagCreate( LPTAGINFO pTag, BOOL fReindex )
{
   LPNTXAREA      pArea = pTag->Owner->Owner;
   PHB_ITEM       pForItem, pWhileItem = NULL, pEvalItem = NULL, pItem = NULL;
   ULONG          ulRecCount, ulRecNo = pArea->dbfarea.ulRecNo;
   LPNTXSORTINFO  pSort;
   LONG           lStep    = 0;
   HB_ERRCODE     errCode  = HB_SUCCESS;

   if( pArea->dbfarea.area.lpdbOrdCondInfo )
   {
      pWhileItem  = pArea->dbfarea.area.lpdbOrdCondInfo->itmCobWhile;
      lStep       = pArea->dbfarea.area.lpdbOrdCondInfo->lStep;
      pEvalItem   = pArea->dbfarea.area.lpdbOrdCondInfo->itmCobEval;
   }

   if( pTag->Custom )
   {
      ulRecCount = 0;
   }
   else
   {
      errCode = SELF_RECCOUNT( ( AREAP ) pArea, &ulRecCount );
      if( errCode != HB_SUCCESS )
         return errCode;
   }
   pArea->pSort      = pSort = hb_ntxSortNew( pTag, ulRecCount );
   pSort->fReindex   = fReindex;

   if( ulRecCount == 0 )
   {
      LPPAGEINFO pPage = hb_ntxPageNew( pTag, FALSE );

      if( pPage )
      {
         pTag->RootBlock = pPage->Page;
         hb_ntxPageRelease( pTag, pPage );
      }
      else
      {
         errCode = HB_FAILURE;
      }
   }
   else
   {
      LPTAGINFO      pSaveTag       = pArea->lpCurTag;
      ULONG          ulStartRec     = 0, ulNextCount = 0;
      BOOL           fDirectRead, fUseFilter = FALSE;
      BYTE *         pSaveRecBuff   = pArea->dbfarea.pRecord;
      char           szBuffer[ NTX_MAX_KEY ];
      int            iRecBuff       = 0, iRecBufSize, iRec;
#ifndef HB_CDP_SUPPORT_OFF
      PHB_CODEPAGE   cdpTmp         = hb_cdpSelect( pArea->dbfarea.area.cdPage );
#endif

      pForItem = pTag->pForItem;
      if( pTag->nField )
         pItem = hb_itemNew( NULL );

      if( ! pArea->dbfarea.area.lpdbOrdCondInfo || pArea->dbfarea.area.lpdbOrdCondInfo->fAll )
      {
         pArea->lpCurTag = NULL;
      }
      else
      {
         if( pArea->dbfarea.area.lpdbOrdCondInfo->itmRecID )
            ulStartRec = hb_itemGetNL( pArea->dbfarea.area.lpdbOrdCondInfo->itmRecID );
         if( ulStartRec )
         {
            ulNextCount = 1;
         }
         else if( pArea->dbfarea.area.lpdbOrdCondInfo->fRest || pArea->dbfarea.area.lpdbOrdCondInfo->lNextCount > 0 )
         {
            if( pArea->dbfarea.area.lpdbOrdCondInfo->itmStartRecID )
               ulStartRec = hb_itemGetNL( pArea->dbfarea.area.lpdbOrdCondInfo->itmStartRecID );
            if( ! ulStartRec )
               ulStartRec = ulRecNo;
            if( pArea->dbfarea.area.lpdbOrdCondInfo->lNextCount > 0 )
               ulNextCount = pArea->dbfarea.area.lpdbOrdCondInfo->lNextCount;
         }
         else if( pArea->dbfarea.area.lpdbOrdCondInfo->fUseFilter )
         {
            fUseFilter = TRUE;
         }
         else if( ! pArea->dbfarea.area.lpdbOrdCondInfo->fUseCurrent )
         {
            pArea->lpCurTag = NULL;
         }
      }

      fDirectRead = ! hb_setGetStrictRead() && /* !pArea->dbfarea.area.lpdbRelations && */
                    ( ! pArea->dbfarea.area.lpdbOrdCondInfo || pArea->dbfarea.area.lpdbOrdCondInfo->fAll ||
                      ( pArea->lpCurTag == NULL && ! fUseFilter ) );

      pSort->ulSizeIO   = ( 1 << 16 ) / NTXBLOCKSIZE;
      pSort->pBuffIO    = ( BYTE * ) hb_xgrab( pSort->ulSizeIO * NTXBLOCKSIZE );
      iRecBufSize       = ( pSort->ulSizeIO * NTXBLOCKSIZE ) / pArea->dbfarea.uiRecordLen;

      if( ulStartRec == 0 && pArea->lpCurTag == NULL )
         ulStartRec = 1;

      if( ulStartRec == 0 )
      {
         errCode = SELF_GOTOP( ( AREAP ) pArea );
      }
      else
      {
         errCode = SELF_GOTO( ( AREAP ) pArea, ulStartRec );
         if( fUseFilter && errCode == HB_SUCCESS )
            errCode = SELF_SKIPFILTER( ( AREAP ) pArea, 1 );
      }

      ulRecNo = pArea->dbfarea.ulRecNo;

      while( errCode == HB_SUCCESS && ! pArea->dbfarea.area.fEof )
      {
         if( hb_vmRequestQuery() != 0 )
         {
            errCode = HB_FAILURE;
            break;
         }

         if( fDirectRead )
         {
            if( ulRecNo > ulRecCount )
               break;
            if( iRecBuff == 0 || iRecBuff >= iRecBufSize )
            {
               if( ulRecCount - ulRecNo >= ( ULONG ) iRecBufSize )
                  iRec = iRecBufSize;
               else
                  iRec = ulRecCount - ulRecNo + 1;
               if( ulNextCount > 0 && ulNextCount < ( ULONG ) iRec )
                  iRec = ( int ) ulNextCount;
               hb_fileReadAt( pArea->dbfarea.pDataFile, pSort->pBuffIO, pArea->dbfarea.uiRecordLen * iRec,
                              ( HB_FOFFSET ) pArea->dbfarea.uiHeaderLen +
                              ( HB_FOFFSET ) ( ulRecNo - 1 ) *
                              ( HB_FOFFSET ) pArea->dbfarea.uiRecordLen );
               iRecBuff = 0;
            }
            pArea->dbfarea.pRecord        = pSort->pBuffIO + iRecBuff * pArea->dbfarea.uiRecordLen;
            pArea->dbfarea.ulRecNo        = ulRecNo;
            if( SELF_GETREC( ( AREAP ) pArea, NULL ) == HB_FAILURE )
               break;
            pArea->dbfarea.fValidBuffer   = pArea->dbfarea.fPositioned = TRUE;
            pArea->dbfarea.fDeleted       = pArea->dbfarea.pRecord[ 0 ] == '*';
            /* Force relational movement in child WorkAreas */
            if( pArea->dbfarea.area.lpdbRelations )
            {
               errCode = SELF_SYNCCHILDREN( ( AREAP ) pArea );
               if( errCode != HB_SUCCESS )
                  break;
            }
            iRecBuff++;
         }

         if( pWhileItem && ! hb_ntxEvalCond( NULL, pWhileItem, FALSE ) )
            break;

         if( ulRecNo <= ulRecCount &&
             ( pForItem == NULL || hb_ntxEvalCond( pArea, pForItem, FALSE ) ) )
         {
            if( pTag->nField )
               errCode = SELF_GETVALUE( ( AREAP ) pArea, pTag->nField, pItem );
            else
               pItem = hb_vmEvalBlockOrMacro( pTag->pKeyItem );

            switch( hb_itemType( pItem ) )
            {
               case HB_IT_STRING:
               case HB_IT_STRING | HB_IT_MEMO:
                  hb_ntxSortKeyAdd( pSort, pArea->dbfarea.ulRecNo,
                                    hb_itemGetCPtr( pItem ),
                                    ( int ) hb_itemGetCLen( pItem ) );
                  break;

               case HB_IT_INTEGER:
               case HB_IT_LONG:
               case HB_IT_DOUBLE:
                  hb_ntxNumToStr( pItem, szBuffer, pTag->KeyLength, pTag->KeyDec );
                  hb_ntxSortKeyAdd( pSort, pArea->dbfarea.ulRecNo, szBuffer, pTag->KeyLength );
                  break;

               case HB_IT_DATE:
                  hb_itemGetDS( pItem, szBuffer );
                  hb_ntxSortKeyAdd( pSort, pArea->dbfarea.ulRecNo, szBuffer, 8 );
                  break;

               case HB_IT_TIMEFLAG:
               {
                  char szBuffer[ 17 ];
                  hb_datetimeDecStr( szBuffer, hb_itemGetDL( pItem ), hb_itemGetT( pItem ) );
                  hb_ntxSortKeyAdd( pSort, pArea->dbfarea.ulRecNo, szBuffer, 17 );
                  break;
               }

               case HB_IT_LOGICAL:
                  szBuffer[ 0 ] = hb_itemGetL( pItem ) ? 'T' : 'F';
                  hb_ntxSortKeyAdd( pSort, pArea->dbfarea.ulRecNo, szBuffer, 1 );
                  break;

               default:
                  hb_ntxErrorRT( pArea, EG_DATATYPE, EDBF_INVALIDKEY,
                                 pTag->Owner->IndexName, 0, 0, NULL );
                  errCode        = HB_FAILURE;
                  pTag->Partial  = TRUE;
                  pEvalItem      = NULL;
                  ulNextCount    = 1;
                  break;
            }
         }

         if( ulNextCount > 0 )
         {
            if( --ulNextCount == 0 )
               break;
         }

         if( pEvalItem )
         {
            if( lStep >= pArea->dbfarea.area.lpdbOrdCondInfo->lStep )
            {
               lStep = 0;
               if( ! hb_ntxEvalCond( pArea, pEvalItem, FALSE ) )
               {
                  pTag->Partial = TRUE;
                  break;
               }
            }
            ++lStep;
         }

         if( fDirectRead )
            ulRecNo++;
         else if( errCode == HB_SUCCESS )
         {
            errCode  = SELF_SKIPRAW( ( AREAP ) pArea, 1 );
            if( fUseFilter && errCode == HB_SUCCESS )
               errCode = SELF_SKIPFILTER( ( AREAP ) pArea, 1 );
            ulRecNo  = pArea->dbfarea.ulRecNo;
         }
      }

      if( fDirectRead )
      {
         pArea->dbfarea.pRecord        = pSaveRecBuff;
         pArea->dbfarea.fValidBuffer   = FALSE;
         if( errCode == HB_SUCCESS )
            errCode = SELF_GOTO( ( AREAP ) pArea, ulRecNo );
      }

      if( errCode == HB_SUCCESS )
         hb_ntxSortOut( pSort );

      if( pTag->nField )
         hb_itemRelease( pItem );

      pArea->lpCurTag = pSaveTag;
#ifndef HB_CDP_SUPPORT_OFF
      hb_cdpSelect( cdpTmp );
#endif
   }

   hb_ntxSortFree( pSort, TRUE );
   pArea->pSort = NULL;

   return errCode;
}

/*
 * recreate tags in index file
 */
static HB_ERRCODE hb_ntxReIndex( LPNTXINDEX pIndex )
{
   HB_ERRCODE  errCode = HB_FAILURE;
   int         i;

   if( hb_ntxIndexLockWrite( pIndex, FALSE ) )
   {
      errCode = HB_SUCCESS;
      hb_ntxIndexTrunc( pIndex );

      for( i = 0; i < pIndex->iTags; i++ )
      {
         LPTAGINFO pTag = pIndex->lpTags[ i ];
         pTag->HeadBlock   = pTag->RootBlock = pTag->keyCount = 0;
         pTag->HdrChanged  = TRUE;
         errCode           = hb_ntxTagCreate( pTag, TRUE );
         if( errCode != HB_SUCCESS )
            break;
      }
      hb_ntxIndexUnLockWrite( pIndex );
   }
   return errCode;
}


/* ************************************************************************* */

/* Implementation of exported functions */

#define hb_ntxBof    NULL
#define hb_ntxEof    NULL
#define hb_ntxFound  NULL

static HB_ERRCODE hb_ntxGoBottom( NTXAREAP pArea )
{
   HB_ERRCODE retval;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxGoBottom(%p)", pArea ) );

   if( SELF_GOCOLD( ( AREAP ) pArea ) == HB_FAILURE )
      return HB_FAILURE;

   if( ! pArea->lpCurTag )
      return SUPER_GOBOTTOM( ( AREAP ) pArea );

   if( pArea->dbfarea.lpdbPendingRel && pArea->dbfarea.lpdbPendingRel->isScoped )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( ! hb_ntxTagLockRead( pArea->lpCurTag ) )
      return HB_FAILURE;
   hb_ntxTagRefreshScope( pArea->lpCurTag );

   hb_ntxTagGoBottom( pArea->lpCurTag );

   pArea->dbfarea.area.fTop      = FALSE;
   pArea->dbfarea.area.fBottom   = TRUE;

   if( pArea->lpCurTag->TagEOF )
      retval = SELF_GOTO( ( AREAP ) pArea, 0 );
   else
   {
      retval = SELF_GOTO( ( AREAP ) pArea, pArea->lpCurTag->CurKeyInfo->Xtra );
      if( retval != HB_FAILURE && pArea->dbfarea.fPositioned )
         retval = SELF_SKIPFILTER( ( AREAP ) pArea, -1 );
   }
   hb_ntxTagUnLockRead( pArea->lpCurTag );

   return retval;
}

#define hb_ntxGoTo   NULL
#define hb_ntxGoToId NULL

static HB_ERRCODE hb_ntxGoTop( NTXAREAP pArea )
{
   HB_ERRCODE retval;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxGoTop(%p)", pArea ) );

   if( SELF_GOCOLD( ( AREAP ) pArea ) == HB_FAILURE )
      return HB_FAILURE;

   if( ! pArea->lpCurTag )
      return SUPER_GOTOP( ( AREAP ) pArea );

   if( pArea->dbfarea.lpdbPendingRel && pArea->dbfarea.lpdbPendingRel->isScoped )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( ! hb_ntxTagLockRead( pArea->lpCurTag ) )
      return HB_FAILURE;
   hb_ntxTagRefreshScope( pArea->lpCurTag );

   hb_ntxTagGoTop( pArea->lpCurTag );

   pArea->dbfarea.area.fTop      = TRUE;
   pArea->dbfarea.area.fBottom   = FALSE;

   if( pArea->lpCurTag->TagEOF )
      retval = SELF_GOTO( ( AREAP ) pArea, 0 );
   else
   {
      retval = SELF_GOTO( ( AREAP ) pArea, pArea->lpCurTag->CurKeyInfo->Xtra );
      if( retval != HB_FAILURE && pArea->dbfarea.fPositioned )
         retval = SELF_SKIPFILTER( ( AREAP ) pArea, 1 );
   }
   hb_ntxTagUnLockRead( pArea->lpCurTag );

   return retval;
}

static HB_ERRCODE hb_ntxSeek( NTXAREAP pArea, BOOL fSoftSeek, PHB_ITEM pItem, BOOL fFindLast )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxSeek(%p, %d, %p, %d)", pArea, fSoftSeek, pItem, fFindLast ) );

   if( SELF_GOCOLD( ( AREAP ) pArea ) == HB_FAILURE )
      return HB_FAILURE;

   if( ! pArea->lpCurTag )
   {
      hb_ntxErrorRT( pArea, EG_NOORDER, EDBF_NOTINDEXED, NULL, 0, EF_CANDEFAULT, NULL );
      return HB_FAILURE;
   }
   else
   {
      LPKEYINFO   pKey;
      HB_ERRCODE  retval   = HB_SUCCESS;
      BOOL        fEOF     = FALSE, fLast;
      USHORT      uiLen;
      ULONG       ulRec;

      if( pArea->dbfarea.lpdbPendingRel && pArea->dbfarea.lpdbPendingRel->isScoped )
         SELF_FORCEREL( ( AREAP ) pArea );

      pArea->dbfarea.area.fTop   = pArea->dbfarea.area.fBottom = FALSE;
      pArea->dbfarea.area.fEof   = FALSE;

      fLast                      = pArea->lpCurTag->fUsrDescend == pArea->lpCurTag->AscendKey ?
                                   ! fFindLast : fFindLast;

      pKey                       = hb_ntxKeyPutItem( NULL, pItem, fLast ? NTX_MAX_REC_NUM :
                                                     NTX_IGNORE_REC_NUM, pArea->lpCurTag, TRUE, &uiLen );

      if( ! hb_ntxTagLockRead( pArea->lpCurTag ) )
      {
         hb_ntxKeyFree( pKey );
         return HB_FAILURE;
      }
      hb_ntxTagRefreshScope( pArea->lpCurTag );

      if( hb_ntxTagKeyFind( pArea->lpCurTag, pKey, uiLen ) )
         ulRec = pArea->lpCurTag->CurKeyInfo->Xtra;
      else
         ulRec = 0;

      if( ( ulRec == 0 && ! fSoftSeek ) || pArea->lpCurTag->TagEOF )
         fEOF = TRUE;
      else
      {
         if( ! hb_ntxInBottomScope( pArea->lpCurTag, pArea->lpCurTag->CurKeyInfo->key ) )
            fEOF = TRUE;
         else if( ! hb_ntxInTopScope( pArea->lpCurTag, pArea->lpCurTag->CurKeyInfo->key ) )
         {
            hb_ntxTagGoTop( pArea->lpCurTag );
            if( pArea->lpCurTag->CurKeyInfo->Xtra == 0 ||
                pArea->lpCurTag->TagEOF )
               fEOF = TRUE;
         }
      }
      hb_ntxTagUnLockRead( pArea->lpCurTag );
      if( ! fEOF )
      {
         retval = SELF_GOTO( ( AREAP ) pArea, pArea->lpCurTag->CurKeyInfo->Xtra );
         if( retval != HB_FAILURE && pArea->dbfarea.fPositioned )
         {
            retval = SELF_SKIPFILTER( ( AREAP ) pArea, fFindLast ? -1 : 1 );
            if( retval != HB_FAILURE && ulRec && pArea->dbfarea.fPositioned )
            {
               pArea->dbfarea.area.fFound = ( ulRec == pArea->dbfarea.ulRecNo ||
                                              hb_ntxValCompare( pArea->lpCurTag, pKey->key, uiLen,
                                                                pArea->lpCurTag->CurKeyInfo->key,
                                                                pArea->lpCurTag->KeyLength, FALSE ) == 0 );
               if( ! pArea->dbfarea.area.fFound && ! fSoftSeek )
                  fEOF = TRUE;
            }
         }
      }
      if( retval != HB_FAILURE && ( fEOF ||
                                    ! hb_ntxKeyInScope( pArea->lpCurTag, pArea->lpCurTag->CurKeyInfo ) ) )
      {
         retval = SELF_GOTO( ( AREAP ) pArea, 0 );
      }
      if( pArea->dbfarea.fPositioned || pArea->dbfarea.ulRecNo != 1 )
         pArea->dbfarea.area.fBof = FALSE;
      hb_ntxKeyFree( pKey );
      return retval;
   }
}

#define hb_ntxSkip         NULL
#define hb_ntxSkipFilter   NULL

static HB_ERRCODE hb_ntxSkipRaw( NTXAREAP pArea, LONG lToSkip )
{
   HB_ERRCODE  retval;
   BOOL        fOut = FALSE, fForward;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxSkipRaw(%p, %ld)", pArea, lToSkip ) );

   if( SELF_GOCOLD( ( AREAP ) pArea ) == HB_FAILURE )
      return HB_FAILURE;

   if( ! pArea->lpCurTag || lToSkip == 0 )
      return SUPER_SKIPRAW( ( AREAP ) pArea, lToSkip );

   if( ! hb_ntxTagLockRead( pArea->lpCurTag ) )
      return HB_FAILURE;
   hb_ntxTagRefreshScope( pArea->lpCurTag );

   fForward = ( lToSkip > 0 );

   if( ! hb_ntxCurKeyRefresh( pArea->lpCurTag ) )
   {
      if( fForward || pArea->dbfarea.fPositioned )
         fOut = TRUE;
      else
      {
         hb_ntxTagGoBottom( pArea->lpCurTag );
         fOut = pArea->lpCurTag->TagEOF;
         lToSkip++;
      }
   }

   if( fForward )
   {
      while( ! fOut && ! pArea->lpCurTag->TagEOF && lToSkip-- > 0 )
      {
         hb_ntxTagSkipNext( pArea->lpCurTag );
      }
      retval = SELF_GOTO( ( AREAP ) pArea,
                          ( pArea->lpCurTag->TagEOF || fOut ) ? 0 :
                          pArea->lpCurTag->CurKeyInfo->Xtra );
   }
   else /* if( lToSkip < 0 ) */
   {
      while( ! fOut && ! pArea->lpCurTag->TagBOF && lToSkip++ < 0 )
      {
         hb_ntxTagSkipPrev( pArea->lpCurTag );
      }
      if( fOut || pArea->lpCurTag->TagBOF )
      {
         hb_ntxTagGoTop( pArea->lpCurTag );
         fOut = TRUE;
      }
      retval                     = SELF_GOTO( ( AREAP ) pArea, pArea->lpCurTag->TagEOF ? 0 :
                                              pArea->lpCurTag->CurKeyInfo->Xtra );
      pArea->dbfarea.area.fBof   = fOut;
   }

   hb_ntxTagUnLockRead( pArea->lpCurTag );
   /* Update Bof and Eof flags */
   /*
      if( fForward )
      pArea->dbfarea.area.fBof = FALSE;
      else
      pArea->dbfarea.area.fEof = FALSE;
    */
   return retval;
}

#define hb_ntxAddField     NULL
#define hb_ntxAppend       NULL
#define hb_ntxCreateFields NULL
#define hb_ntxDeleteRec    NULL
#define hb_ntxDeleted      NULL
#define hb_ntxFieldCount   NULL
#define hb_ntxFieldDisplay NULL
#define hb_ntxFieldInfo    NULL
#define hb_ntxFieldName    NULL

/*
 * Flush _system_ buffers to disk
 */
static HB_ERRCODE hb_ntxFlush( NTXAREAP pArea )
{
   HB_ERRCODE uiError;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxFlush(%p)", pArea ) );

   uiError = SELF_GOCOLD( ( AREAP ) pArea );
   if( uiError == HB_SUCCESS )
   {
      uiError = SUPER_FLUSH( ( AREAP ) pArea );

      if( hb_setGetHardCommit() )
      {
         LPNTXINDEX pIndex = pArea->lpIndexes;
         while( pIndex )
         {
            if( pIndex->fFlush /* && !pIndex->Temporary */ )
            {
               hb_fileCommit( pIndex->DiskFile );
               pIndex->fFlush = FALSE;
            }
            pIndex = pIndex->pNext;
         }
      }
   }

   return uiError;
}

#define hb_ntxGetRec    NULL
#define hb_ntxGetValue  NULL
#define hb_ntxGetVarLen NULL

/*
 * Perform a write of WorkArea memory to the data store.
 */
static HB_ERRCODE hb_ntxGoCold( NTXAREAP pArea )
{
   BOOL  fRecordChanged = pArea->dbfarea.fRecordChanged;
   BOOL  fAppend        = pArea->dbfarea.fAppend;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxGoCold(%p)", pArea ) );

   if( SUPER_GOCOLD( ( AREAP ) pArea ) == HB_SUCCESS )
   {
      if( fRecordChanged || pArea->fNtxAppend )
      {
         if( fAppend && pArea->dbfarea.fShared )
         {
            if( pArea->fNtxAppend )
               hb_errInternal( 9312, "hb_ntxGoCold: multiple appending without GOCOLD.", NULL, NULL );
            pArea->fNtxAppend = TRUE;
         }
         else
         {
            LPNTXINDEX  pIndex = pArea->lpIndexes;
            LPTAGINFO   pTag;
            LPKEYINFO   pKey;
            BOOL        fAdd, fDel, fLck = FALSE;
            int         i;

            /* The pending relation may move the record pointer so we should
               disable them for KEY/FOR evaluation */
            LPDBRELINFO lpdbPendingRel = pArea->dbfarea.lpdbPendingRel;
            pArea->dbfarea.lpdbPendingRel = NULL;

            if( pArea->dbfarea.fShared )
            {
               fAppend           = pArea->fNtxAppend;
               pArea->fNtxAppend = FALSE;
            }

            while( pIndex )
            {
               for( i = 0; i < pIndex->iTags; i++ )
               {
                  pTag = pIndex->lpTags[ i ];
                  if( pIndex->fReadonly || pTag->Custom ||
                      ( pTag->Owner->Compound && ! pTag->HeadBlock ) ||
                      ( fAppend && pTag->ChgOnly ) )
                     continue;

                  pKey  = hb_ntxEvalKey( NULL, pTag );

                  fAdd  = ( pTag->pForItem == NULL ||
                            hb_ntxEvalCond( pArea, pTag->pForItem, TRUE ) );
                  if( fAppend )
                  {
                     fDel = FALSE;
                  }
                  else
                  {
                     if( hb_ntxValCompare( pTag, pKey->key, pTag->KeyLength,
                                           pTag->HotKeyInfo->key, pTag->KeyLength, TRUE ) == 0 )
                     {
                        if( pTag->HotFor ? fAdd : ! fAdd )
                           fAdd = fDel = FALSE;
                        else
                           fDel = ! fAdd;
                     }
                     else
                     {
                        fDel = pTag->HotFor || pTag->Partial;
                     }
                  }
                  if( fDel || fAdd )
                  {
                     if( ! fLck )
                     {
                        if( ! hb_ntxIndexLockWrite( pIndex, TRUE ) )
                        {
                           hb_ntxKeyFree( pKey );
                           break;
                        }
                        fLck = TRUE;
                        if( ( pTag->Owner->Compound && ! pTag->HeadBlock ) ||
                            ! pTag->RootBlock )
                           fAdd = fDel = FALSE;
                     }
                     if( fDel )
                     {
                        if( hb_ntxTagKeyDel( pTag, pTag->HotKeyInfo ) )
                        {
                           if( ! pIndex->fShared && pTag->keyCount &&
                               hb_ntxKeyInScope( pTag, pTag->HotKeyInfo ) )
                              pTag->keyCount--;
                        }
                        else
                        {
                           if( pTag->ChgOnly )
                              fAdd = FALSE;
                           else if( ! pTag->Partial && ! pTag->UniqueKey )
                              hb_ntxErrorRT( pTag->Owner->Owner,
                                             EG_CORRUPTION, EDBF_CORRUPT,
                                             pTag->Owner->IndexName, 0, 0, NULL );
                        }
                     }
                     if( fAdd )
                     {
                        if( hb_ntxTagKeyAdd( pTag, pKey ) )
                        {
                           if( ! pIndex->fShared && pTag->keyCount &&
                               hb_ntxKeyInScope( pTag, pKey ) )
                              pTag->keyCount++;
                        }
                     }
                  }
                  hb_ntxKeyFree( pKey );
               }
               if( fLck )
               {
                  hb_ntxIndexUnLockWrite( pIndex );
                  fLck = FALSE;
               }
               pIndex = pIndex->pNext;
            }

            /* Restore disabled pending relation */
            pArea->dbfarea.lpdbPendingRel = lpdbPendingRel;
         }
      }
      return HB_SUCCESS;
   }
   return HB_FAILURE;
}

/*
 * Mark the WorkArea data buffer as hot.
 */
static HB_ERRCODE hb_ntxGoHot( NTXAREAP pArea )
{
   HB_ERRCODE errCode;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxGoHot(%p)", pArea ) );

   errCode = SUPER_GOHOT( ( AREAP ) pArea );
   if( errCode == HB_SUCCESS )
   {
      if( ! pArea->fNtxAppend )
      {
         LPNTXINDEX  pIndex = pArea->lpIndexes;
         LPTAGINFO   pTag;
         int         i;

         while( pIndex )
         {
            if( ! pIndex->fReadonly )
            {
               for( i = 0; i < pIndex->iTags; i++ )
               {
                  pTag = pIndex->lpTags[ i ];
                  if( ! pTag->Custom )
                  {
                     pTag->HotKeyInfo  = hb_ntxEvalKey( pTag->HotKeyInfo, pTag );
                     pTag->HotFor      = ( pTag->pForItem == NULL ||
                                           hb_ntxEvalCond( pArea, pTag->pForItem, TRUE ) );
                  }
               }
            }
            pIndex = pIndex->pNext;
         }
      }
      return HB_SUCCESS;
   }
   return errCode;
}

#define hb_ntxPutRec          NULL
#define hb_ntxPutValue        NULL
#define hb_ntxRecall          NULL
#define hb_ntxRecCount        NULL
#define hb_ntxRecInfo         NULL
#define hb_ntxRecNo           NULL
#define hb_ntxRecId           NULL
#define hb_ntxSetFieldsExtent NULL
#define hb_ntxAlias           NULL

/*
 * Close the table in the WorkArea.
 */
static HB_ERRCODE hb_ntxClose( NTXAREAP pArea )
{
   HB_ERRCODE errCode;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxClose(%p)", pArea ) );

   if( SELF_GOCOLD( ( AREAP ) pArea ) == HB_FAILURE )
      return HB_FAILURE;

   errCode = SUPER_CLOSE( ( AREAP ) pArea );

   if( errCode == HB_SUCCESS )
   {
      if( pArea->pSort )
      {
         hb_ntxSortFree( pArea->pSort, TRUE );
         pArea->pSort = NULL;
      }

      SELF_ORDLSTCLEAR( ( AREAP ) pArea );

      /* close also production indexes if any */
      while( pArea->lpIndexes )
      {
         LPNTXINDEX pIndex = pArea->lpIndexes;
         pArea->lpIndexes = pIndex->pNext;
         hb_ntxIndexFree( pIndex );
      }

#ifdef HB_NTX_DEBUG_DISP
      printf( "\r\n#reads=%ld, #writes=%ld\r\n", s_rdNO, s_wrNO ); fflush( stdout );
#endif
   }

   return errCode;
}

#define hb_ntxCreate    NULL
#define hb_ntxInfo      NULL
#define hb_ntxNewArea   NULL

/*
 * Retrieve the size of the WorkArea structure.
 */
static HB_ERRCODE hb_ntxStructSize( NTXAREAP pArea, USHORT * uiSize )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxStructSize(%p, %p)", pArea, uiSize ) );
   HB_SYMBOL_UNUSED( pArea );

   *uiSize = sizeof( NTXAREA );
   return HB_SUCCESS;
}

/*
 * Open a data store in the WorkArea.
 */
static HB_ERRCODE hb_ntxOpen( NTXAREAP pArea, LPDBOPENINFO pOpenInfo )
{
   HB_ERRCODE errCode;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOpen(%p, %p)", pArea, pOpenInfo ) );

   errCode = SUPER_OPEN( ( AREAP ) pArea, pOpenInfo );

   if( errCode == HB_SUCCESS && DBFAREA_DATA( &pArea->dbfarea )->fStruct &&
       ( DBFAREA_DATA( &pArea->dbfarea )->fStrictStruct ?
         pArea->dbfarea.fHasTags : hb_setGetAutOpen() ) )
   {
      char szFileName[ HB_PATH_MAX ];

      hb_ntxCreateFName( pArea, NULL, NULL, szFileName, NULL );
      if( hb_fileExists( szFileName, NULL ) ||
          DBFAREA_DATA( &pArea->dbfarea )->fStrictStruct )
      {
         DBORDERINFO pOrderInfo;

         pOrderInfo.itmResult    = hb_itemPutNI( NULL, 0 );
         pOrderInfo.atomBagName  = hb_itemPutC( NULL, szFileName );
         pOrderInfo.itmNewVal    = NULL;
         pOrderInfo.itmOrder     = NULL;
         errCode                 = SELF_ORDLSTADD( ( AREAP ) pArea, &pOrderInfo );
         if( errCode == HB_SUCCESS )
         {
            pOrderInfo.itmOrder  = hb_itemPutNI( NULL, hb_setGetAutOrder() );
            errCode              = SELF_ORDLSTFOCUS( ( AREAP ) pArea, &pOrderInfo );
            hb_itemRelease( pOrderInfo.itmOrder );
            if( errCode == HB_SUCCESS )
               errCode = SELF_GOTOP( ( AREAP ) pArea );
         }
         hb_itemRelease( pOrderInfo.atomBagName );
         hb_itemRelease( pOrderInfo.itmResult );
      }
   }

   return errCode;
}

#define hb_ntxRelease   NULL
#define hb_ntxSysName   NULL
#define hb_ntxEval      NULL

static HB_ERRCODE hb_ntxPack( NTXAREAP pArea )
{
   HB_ERRCODE errCode;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxPack(%p)", pArea ) );

   errCode = SUPER_PACK( ( AREAP ) pArea );
   if( errCode == HB_SUCCESS )
      return SELF_ORDLSTREBUILD( ( AREAP ) pArea );

   return errCode;
}

#define ntPackRec       NULL
#define hb_ntxSort      NULL
#define hb_ntxTrans     NULL
#define hb_ntxTransRec  NULL

static HB_ERRCODE hb_ntxZap( NTXAREAP pArea )
{
   HB_ERRCODE errCode;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxZap(%p)", pArea ) );

   errCode = SUPER_ZAP( ( AREAP ) pArea );
   if( errCode == HB_SUCCESS )
      return SELF_ORDLSTREBUILD( ( AREAP ) pArea );

   return errCode;
}

#define hb_ntxchildEnd        NULL
#define hb_ntxchildStart      NULL
#define hb_ntxchildSync       NULL
#define hb_ntxsyncChildren    NULL
#define hb_ntxclearRel        NULL
#define hb_ntxforceRel        NULL
#define hb_ntxrelArea         NULL
#define hb_ntxrelEval         NULL
#define hb_ntxrelText         NULL
#define hb_ntxsetRel          NULL

#define hb_ntxOrderCondition  NULL

static HB_ERRCODE hb_ntxOrderCreate( NTXAREAP pArea, LPDBORDERCREATEINFO pOrderInfo )
{
   PHB_ITEM       pResult, pKeyExp, pForExp = NULL;
   int            iLen, iDec, iTag, i;
   char           szFileName[ HB_PATH_MAX ], szSpFile[ HB_PATH_MAX ],
                  szTagName[ NTX_MAX_TAGNAME + 1 ];
   const char *   szKey, * szFor = NULL;
   LPNTXINDEX     pIndex, * pIndexPtr;
   LPTAGINFO      pTag = NULL;
   LPDBFDATA      pData;
   HB_ERRCODE     errCode;
   ULONG          ulRecNo;
   BOOL           fCompound, fTagName, fBagName, fProd, fLocked = FALSE,
                  fAscend = TRUE, fCustom = FALSE, fTemporary = FALSE, fExclusive = FALSE;
   BYTE           bType;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrderCreate(%p, %p)", pArea, pOrderInfo ) );

   errCode = SELF_GOCOLD( ( AREAP ) pArea );
   if( errCode != HB_SUCCESS )
      return errCode;

   if( pArea->dbfarea.lpdbPendingRel )
   {
      errCode = SELF_FORCEREL( ( AREAP ) pArea );
      if( errCode != HB_SUCCESS )
         return errCode;
   }

   szKey = hb_itemGetCPtr( pOrderInfo->abExpr );
   /* If we have a codeblock for the expression, use it */
   if( pOrderInfo->itmCobExpr )
      pKeyExp = hb_itemNew( pOrderInfo->itmCobExpr );
   else /* Otherwise, try compiling the key expression string */
   {
      errCode                       = SELF_COMPILE( ( AREAP ) pArea, szKey );
      if( errCode != HB_SUCCESS )
         return errCode;
      pKeyExp                       = pArea->dbfarea.area.valResult;
      pArea->dbfarea.area.valResult = NULL;
   }

   /* Get a blank record before testing expression */
   ulRecNo  = pArea->dbfarea.ulRecNo;
   errCode  = SELF_GOTO( ( AREAP ) pArea, 0 );
   if( errCode != HB_SUCCESS )
      return errCode;

   errCode = SELF_EVALBLOCK( ( AREAP ) pArea, pKeyExp );
   if( errCode != HB_SUCCESS )
   {
      hb_vmDestroyBlockOrMacro( pKeyExp );
      SELF_GOTO( ( AREAP ) pArea, ulRecNo );
      return errCode;
   }
   pResult                       = pArea->dbfarea.area.valResult;
   pArea->dbfarea.area.valResult = NULL;

   bType                         = hb_ntxItemType( pResult );
   iLen                          = iDec = 0;
   switch( bType )
   {
      case 'N':
         hb_itemGetNLen( pResult, &iLen, &iDec );
         if( iDec )
            iLen += iDec + 1;
         break;
      case 'D':
         iLen  = 8;
         break;
      case 'L':
         iLen  = 1;
         break;
      case 'C':
         iLen  = ( int ) hb_itemGetCLen( pResult );
         if( iLen > NTX_MAX_KEY )
            iLen = NTX_MAX_KEY;
         break;
      default:
         bType = 'U';
   }
   hb_itemRelease( pResult );

   /* Make sure KEY has proper type and iLen is not 0 */
   if( bType == 'U' || iLen == 0 )
   {
      hb_vmDestroyBlockOrMacro( pKeyExp );
      SELF_GOTO( ( AREAP ) pArea, ulRecNo );
      hb_ntxErrorRT( pArea, bType == 'U' ? EG_DATATYPE : EG_DATAWIDTH,
                     EDBF_INVALIDKEY, NULL, 0, 0, NULL );
      return HB_FAILURE;
   }

   if( pArea->dbfarea.area.lpdbOrdCondInfo )
   {
      fAscend     = ! pArea->dbfarea.area.lpdbOrdCondInfo->fDescending;
      fCustom     = pArea->dbfarea.area.lpdbOrdCondInfo->fCustom;
      fTemporary  = pArea->dbfarea.area.lpdbOrdCondInfo->fTemporary;
      fExclusive  = pArea->dbfarea.area.lpdbOrdCondInfo->fExclusive;
      /* Check conditional expression */
      szFor       = ( char * ) pArea->dbfarea.area.lpdbOrdCondInfo->abFor;
      if( pArea->dbfarea.area.lpdbOrdCondInfo->itmCobFor )
         /* If we have a codeblock for the conditional expression, use it */
         pForExp = hb_itemNew( pArea->dbfarea.area.lpdbOrdCondInfo->itmCobFor );
      else if( szFor )
      {
         /* Otherwise, try compiling the conditional expression string */
         errCode = SELF_COMPILE( ( AREAP ) pArea, szFor );
         if( errCode != HB_SUCCESS )
         {
            hb_vmDestroyBlockOrMacro( pKeyExp );
            SELF_GOTO( ( AREAP ) pArea, ulRecNo );
            return errCode;
         }
         pForExp                       = pArea->dbfarea.area.valResult;
         pArea->dbfarea.area.valResult = NULL;
      }
   }

   if( pArea->dbfarea.fTemporary )
      fTemporary = TRUE;

   /* Test conditional expression */
   if( pForExp )
   {
      BOOL fOK;

      errCode = SELF_EVALBLOCK( ( AREAP ) pArea, pForExp );
      if( errCode != HB_SUCCESS )
      {
         hb_vmDestroyBlockOrMacro( pKeyExp );
         hb_vmDestroyBlockOrMacro( pForExp );
         SELF_GOTO( ( AREAP ) pArea, ulRecNo );
         return errCode;
      }
      fOK                           = hb_itemType( pArea->dbfarea.area.valResult ) == HB_IT_LOGICAL;
      hb_itemRelease( pArea->dbfarea.area.valResult );
      pArea->dbfarea.area.valResult = NULL;
      if( ! fOK )
      {
         hb_vmDestroyBlockOrMacro( pKeyExp );
         hb_vmDestroyBlockOrMacro( pForExp );
         SELF_GOTO( ( AREAP ) pArea, ulRecNo );
         hb_ntxErrorRT( pArea, EG_DATATYPE, EDBF_INVALIDFOR, NULL, 0, 0, NULL );
         return HB_FAILURE;
      }
   }

   SELF_GOTO( ( AREAP ) pArea, ulRecNo );

   pData       = DBFAREA_DATA( &pArea->dbfarea );
   /*
    * abBagName -> cBag, atomBagName -> cTag
    * The following scheme implemented:
    * 1. abBagName == NULL   -> add the Tag to the structural index
    *    if no compound index support then create new separate index
    *    with atomBagName
    * 2. atomBagName == NULL -> overwrite any index file of abBagName
    * 3. ads the Tag to index file
    */
   fTagName    = pOrderInfo->atomBagName && pOrderInfo->atomBagName[ 0 ];
   fBagName    = pOrderInfo->abBagName && pOrderInfo->abBagName[ 0 ];
#if defined( HB_NTX_NOMULTITAG )
   fCompound   = FALSE;
#else
   fCompound   = fTagName && pData->fMultiTag;
#endif
   hb_ntxCreateFName( pArea, ( char * ) ( ( fBagName || fCompound ) ?
                                          pOrderInfo->abBagName : pOrderInfo->atomBagName ),
                      &fProd, szFileName, szTagName );
   if( fTagName )
      hb_strncpyUpperTrim( szTagName, ( char * ) pOrderInfo->atomBagName, NTX_MAX_TAGNAME );

   pIndex = hb_ntxFindBag( pArea, szFileName );
   if( pIndex && ! fCompound )
   {
      pIndexPtr = &pArea->lpIndexes;
      while( *pIndexPtr )
      {
         if( pIndex == *pIndexPtr )
         {
            *pIndexPtr = pIndex->pNext;
            hb_ntxIndexFree( pIndex );
            break;
         }
         pIndexPtr = &( *pIndexPtr )->pNext;
      }
      pIndex = NULL;
   }

   if( pIndex )
   {
      if( pIndex->fReadonly )
      {
         hb_vmDestroyBlockOrMacro( pKeyExp );
         if( pForExp != NULL )
            hb_vmDestroyBlockOrMacro( pForExp );
         hb_ntxErrorRT( pArea, EG_READONLY, EDBF_READONLY, pIndex->IndexName, 0, 0, NULL );
         return HB_FAILURE;
      }
#if 0 /* enable this code if you want to forbid tag deleting in shared mode */
      else if( pIndex->fShared )
      {
         hb_vmDestroyBlockOrMacro( pKeyExp );
         if( pForExp != NULL )
            hb_vmDestroyBlockOrMacro( pForExp );
         hb_ntxErrorRT( pArea, EG_SHARED, EDBF_SHARED, pIndex->IndexName, 0, 0, NULL );
         return HB_FAILURE;
      }
#endif
   }
   else
   {
      PHB_FILE pFile;
      BOOL     bRetry, fOld, fShared = pArea->dbfarea.fShared && ! fTemporary && ! fExclusive;
      USHORT   uiFlags  = FO_READWRITE | ( fShared ? FO_DENYNONE : FO_EXCLUSIVE );
      PHB_ITEM pError   = NULL;

      fOld = fCompound;
      do
      {
         if( fTemporary )
         {
            pFile = hb_fileCreateTemp( NULL, NULL, FC_NORMAL, szSpFile );
            fOld  = FALSE;
         }
         else
         {
            pFile = hb_fileExtOpen( szFileName, NULL, uiFlags |
                                    ( fOld ? FXO_APPEND : FXO_TRUNCATE ) |
                                    FXO_DEFAULTS | FXO_SHARELOCK | FXO_COPYNAME,
                                    NULL, pError, TRUE );
         }
         if( ! pFile )
            bRetry = hb_ntxErrorRT( pArea, EG_CREATE, EDBF_CREATE, szFileName,
                                    hb_fsError(), EF_CANRETRY | EF_CANDEFAULT,
                                    &pError ) == E_RETRY;
         else
         {
            bRetry = FALSE;
            if( fOld )
               fOld = hb_fileSize( pFile ) != 0;
         }
      }
      while( bRetry );

      if( pError )
         hb_errRelease( pError );

      if( ! pFile )
      {
         hb_vmDestroyBlockOrMacro( pKeyExp );
         if( pForExp != NULL )
            hb_vmDestroyBlockOrMacro( pForExp );
         /* hb_ntxSetTagNumbers() */
         return HB_FAILURE;
      }

      pIndex            = hb_ntxIndexNew( pArea );
      pIndex->IndexName = hb_strdup( szFileName );
      pIndex->fReadonly = FALSE;
      pIndex->fShared   = fShared;
      pIndex->DiskFile  = pFile;
      pIndex->fDelete   = fTemporary;
      if( fTemporary )
         pIndex->RealName = hb_strdup( szSpFile );
      else
         pIndex->Production = fProd;

      pIndexPtr   = &pArea->lpIndexes;
      while( *pIndexPtr )
         pIndexPtr = &( *pIndexPtr )->pNext;
      *pIndexPtr  = pIndex;
      if( fOld )
      {
         if( ! hb_ntxIndexLockWrite( pIndex, TRUE ) )
            errCode = HB_FAILURE;
         else
         {
            errCode = hb_ntxIndexLoad( pIndex, szTagName );
            if( errCode != HB_SUCCESS )
               hb_ntxIndexUnLockWrite( pIndex );
            else
               fLocked = TRUE;
         }
         if( errCode != HB_SUCCESS )
         {
            *pIndexPtr = pIndex->pNext;
            hb_ntxIndexFree( pIndex );
            hb_vmDestroyBlockOrMacro( pKeyExp );
            if( pForExp != NULL )
               hb_vmDestroyBlockOrMacro( pForExp );
            /* hb_ntxSetTagNumbers() */
            hb_ntxErrorRT( pArea, EG_CORRUPTION, EDBF_CORRUPT, szFileName, 0, 0, NULL );
            return errCode;
         }
      }
      else
      {
         pIndex->LargeFile = ( pIndex->Owner->dbfarea.bLockType == DB_DBFLOCK_XHB64 );
      }
   }

   iTag        = hb_ntxFindTagByName( pIndex, szTagName );
   fCompound   = ( pIndex->iTags > ( iTag ? 1 : 0 ) );

   if( ! iTag && pIndex->iTags == CTX_MAX_TAGS )
   {
      if( fLocked )
         hb_ntxIndexUnLockWrite( pIndex );
      hb_vmDestroyBlockOrMacro( pKeyExp );
      if( pForExp != NULL )
         hb_vmDestroyBlockOrMacro( pForExp );
      /* hb_ntxSetTagNumbers() */
      hb_ntxErrorRT( pArea, EG_LIMIT, EDBF_LIMITEXCEEDED, pIndex->IndexName, 0, 0, NULL );
      return HB_FAILURE;
   }

   if( ! fLocked && ! hb_ntxIndexLockWrite( pIndex, fCompound ) )
   {
      errCode = HB_FAILURE;
   }
   else
   {
      if( pIndex->Compound != fCompound )
      {
         pIndex->Compound = fCompound;
         if( fCompound )
         {
            if( ! pIndex->HeaderBuff )
               pIndex->HeaderBuff = ( BYTE * ) hb_xgrab( NTXBLOCKSIZE );
            memset( pIndex->HeaderBuff, 0, NTXBLOCKSIZE );
            pIndex->fValidHeader = TRUE;
         }
         for( i = 0; i < pIndex->iTags; i++ )
         {
            pIndex->lpTags[ i ]->HdrChanged  = TRUE;
            pIndex->lpTags[ i ]->HeadBlock   = 0;
            if( fCompound )
               hb_ntxIndexTagAdd( pIndex, pIndex->lpTags[ i ] );
         }
      }
      pTag = hb_ntxTagNew( pIndex, szTagName, fTagName,
                           szKey, pKeyExp, bType, ( USHORT ) iLen, ( USHORT ) iDec,
                           szFor, pForExp,
                           fAscend, pOrderInfo->fUnique, fCustom, pData->fSortRecNo );
      pTag->Partial = ( pArea->dbfarea.area.lpdbOrdCondInfo && ! pArea->dbfarea.area.lpdbOrdCondInfo->fAll );

      if( ! pIndex->Compound )
      {
         while( pIndex->iTags )
            hb_ntxTagDelete( pIndex->lpTags[ 0 ] );
         hb_ntxIndexTrunc( pIndex );
         iTag = 0;
      }

      if( iTag )
      {
         pTag->HeadBlock = pIndex->lpTags[ iTag - 1 ]->HeadBlock;
         if( pIndex->lpTags[ iTag - 1 ]->RootBlock &&
             ! hb_ntxTagPagesFree( pIndex->lpTags[ iTag - 1 ],
                                   pIndex->lpTags[ iTag - 1 ]->RootBlock ) )
         {
            errCode = HB_FAILURE;
         }
         else
         {
            hb_ntxTagFree( pIndex->lpTags[ iTag - 1 ] );
            pIndex->lpTags[ iTag - 1 ] = pTag;
         }
      }
      else
      {
         hb_ntxTagAdd( pIndex, pTag );
         if( pIndex->Compound )
            hb_ntxIndexTagAdd( pIndex, pTag );
      }

      if( errCode == HB_SUCCESS )
      {
         pIndex->Update = pIndex->Changed = pTag->HdrChanged = TRUE;
         errCode        = hb_ntxTagCreate( pTag, FALSE );
      }
      hb_ntxIndexUnLockWrite( pIndex );
   }

   pIndexPtr = &pArea->lpIndexes;
   while( *pIndexPtr && *pIndexPtr != pIndex )
      pIndexPtr = &( *pIndexPtr )->pNext;

   /* It should not happen, reintrance? */
   if( ! *pIndexPtr )
      return HB_FAILURE;

   if( errCode != HB_SUCCESS )
   {
      *pIndexPtr = pIndex->pNext;
      hb_ntxIndexFree( pIndex );
      /* hb_ntxSetTagNumbers() */
      return errCode;
   }

   if( ! pArea->dbfarea.area.lpdbOrdCondInfo || ! pArea->dbfarea.area.lpdbOrdCondInfo->fAdditive )
   {
      *pIndexPtr     = pIndex->pNext;
      pIndex->pNext  = NULL;
      SELF_ORDLSTCLEAR( ( AREAP ) pArea );
      pIndexPtr      = &pArea->lpIndexes;
      while( *pIndexPtr )
         pIndexPtr = &( *pIndexPtr )->pNext;
      *pIndexPtr     = pIndex;
   }
   if( pIndex->Production && ! pArea->dbfarea.fHasTags &&
       pData->fStruct && ( pData->fStrictStruct || hb_setGetAutOpen() ) )
   {
      pArea->dbfarea.fHasTags = TRUE;
      if( ! pArea->dbfarea.fReadonly && ( pArea->dbfarea.dbfHeader.bHasTags & 0x01 ) == 0 )
         SELF_WRITEDBHEADER( ( AREAP ) pArea );
   }
   /* hb_ntxSetTagNumbers() */
   pArea->lpCurTag = pTag;
   SELF_ORDSETCOND( ( AREAP ) pArea, NULL );
   return SELF_GOTOP( ( AREAP ) pArea );
}

static HB_ERRCODE hb_ntxOrderDestroy( NTXAREAP pArea, LPDBORDERINFO pOrderInfo )
{
   HB_ERRCODE errCode;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrderDestroy(%p, %p)", pArea, pOrderInfo ) );

   errCode = SELF_GOCOLD( ( AREAP ) pArea );
   if( errCode != HB_SUCCESS )
      return errCode;

   if( pArea->dbfarea.lpdbPendingRel )
   {
      errCode = SELF_FORCEREL( ( AREAP ) pArea );
      if( errCode != HB_SUCCESS )
         return errCode;
   }

   if( pOrderInfo->itmOrder )
   {
      LPTAGINFO pTag = hb_ntxFindTag( pArea, pOrderInfo->itmOrder, pOrderInfo->atomBagName );

      if( pTag )
      {
         LPNTXINDEX pIndex = pTag->Owner, * pIndexPtr;

         if( pIndex->iTags == 1 )
         {
            BOOL fProd = pIndex->Production;
            pIndexPtr         = &pArea->lpIndexes;
            while( *pIndexPtr != pIndex )
               pIndexPtr = &( *pIndexPtr )->pNext;
            *pIndexPtr        = pIndex->pNext;
            pIndex->fDelete   = TRUE;
            hb_ntxIndexFree( pIndex );
            if( fProd && pArea->dbfarea.fHasTags &&
                DBFAREA_DATA( &pArea->dbfarea )->fStruct &&
                ( DBFAREA_DATA( &pArea->dbfarea )->fStrictStruct || hb_setGetAutOpen() ) )
            {
               pArea->dbfarea.fHasTags = FALSE;
               if( ! pArea->dbfarea.fReadonly && ( pArea->dbfarea.dbfHeader.bHasTags & 0x01 ) != 0 )
                  SELF_WRITEDBHEADER( ( AREAP ) pArea );
            }
         }
         else if( pIndex->fReadonly )
         {
            hb_ntxErrorRT( pArea, EG_READONLY, EDBF_READONLY, pIndex->IndexName, 0, 0, NULL );
            return HB_FAILURE;
         }
#if 0    /* enable this code if you want to forbid tag deleting in shared mode */
         else if( pIndex->fShared )
         {
            hb_ntxErrorRT( pArea, EG_SHARED, EDBF_SHARED, pIndex->IndexName, 0, 0, NULL );
            return HB_FAILURE;
         }
#endif
         else if( ! hb_ntxIndexLockWrite( pIndex, TRUE ) )
         {
            return HB_FAILURE;
         }
         else
         {
            errCode = hb_ntxTagSpaceFree( pTag );
            hb_ntxIndexUnLockWrite( pIndex );
         }
         /* hb_ntxSetTagNumbers() */
      }
   }

   return errCode;
}

static HB_ERRCODE hb_ntxOrderInfo( NTXAREAP pArea, USHORT uiIndex, LPDBORDERINFO pInfo )
{
   LPTAGINFO pTag;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrderInfo(%p, %hu, %p)", pArea, uiIndex, pInfo ) );

   switch( uiIndex )
   {
      case DBOI_STRICTREAD:
         hb_itemClear( pInfo->itmResult );
         return SELF_RDDINFO( SELF_RDDNODE( &pArea->dbfarea.area ), RDDI_STRICTREAD, 0, pInfo->itmResult );
      case DBOI_OPTIMIZE:
         hb_itemClear( pInfo->itmResult );
         return SELF_RDDINFO( SELF_RDDNODE( &pArea->dbfarea.area ), RDDI_OPTIMIZE, 0, pInfo->itmResult );
      case DBOI_AUTOOPEN:
         hb_itemClear( pInfo->itmResult );
         return SELF_RDDINFO( SELF_RDDNODE( &pArea->dbfarea.area ), RDDI_AUTOOPEN, 0, pInfo->itmResult );
      case DBOI_AUTOORDER:
         hb_itemClear( pInfo->itmResult );
         return SELF_RDDINFO( SELF_RDDNODE( &pArea->dbfarea.area ), RDDI_AUTOORDER, 0, pInfo->itmResult );
      case DBOI_AUTOSHARE:
         hb_itemClear( pInfo->itmResult );
         return SELF_RDDINFO( SELF_RDDNODE( &pArea->dbfarea.area ), RDDI_AUTOSHARE, 0, pInfo->itmResult );
      case DBOI_BAGEXT:
         hb_itemClear( pInfo->itmResult );
         return SELF_RDDINFO( SELF_RDDNODE( &pArea->dbfarea.area ), RDDI_ORDBAGEXT, 0, pInfo->itmResult );
      case DBOI_EVALSTEP:
         hb_itemPutNL( pInfo->itmResult,
                       pArea->dbfarea.area.lpdbOrdCondInfo ? pArea->dbfarea.area.lpdbOrdCondInfo->lStep : 0 );
         return HB_SUCCESS;
      case DBOI_KEYSINCLUDED:
         hb_itemPutNL( pInfo->itmResult,
                       pArea->pSort ? pArea->pSort->ulTotKeys : 0 );
         return HB_SUCCESS;
      case DBOI_I_TAGNAME:
         hb_itemPutC( pInfo->itmResult,
                      pArea->pSort ? pArea->pSort->pTag->TagName : NULL );
         return HB_SUCCESS;
      case DBOI_I_BAGNAME:
         hb_itemPutC( pInfo->itmResult, pArea->pSort ?
                      pArea->pSort->pTag->Owner->IndexName : NULL );
         return HB_SUCCESS;
      case DBOI_ISREINDEX:
         hb_itemPutL( pInfo->itmResult,
                      pArea->pSort ? pArea->pSort->fReindex : FALSE );
         return HB_SUCCESS;
      case DBOI_LOCKOFFSET:
      case DBOI_HPLOCKING:
      {
         HB_FOFFSET ulPos, ulPool;
         hb_dbfLockIdxGetData( pArea->dbfarea.bLockType, &ulPos, &ulPool );
         if( uiIndex == DBOI_LOCKOFFSET )
            hb_itemPutNInt( pInfo->itmResult, ulPos );
         else
            hb_itemPutL( pInfo->itmResult, ulPool > 0 );
         return HB_SUCCESS;
      }
      case DBOI_ORDERCOUNT:
      {
         int         i        = 0;
         BOOL        fBag     = hb_itemGetCLen( pInfo->atomBagName ) > 0;
         LPNTXINDEX  pIndex   = fBag ?
                                hb_ntxFindBag( pArea, hb_itemGetCPtr( pInfo->atomBagName ) ) :
                                pArea->lpIndexes;
         while( pIndex )
         {
            i        += pIndex->iTags;
            if( fBag )
               break;
            pIndex   = pIndex->pNext;
         }
         hb_itemPutNI( pInfo->itmResult, i );
         return HB_SUCCESS;
      }
      case DBOI_BAGCOUNT:
      {
         int         i        = 0;
         LPNTXINDEX  pIndex   = pArea->lpIndexes;
         while( pIndex )
         {
            ++i;
            pIndex = pIndex->pNext;
         }
         hb_itemPutNI( pInfo->itmResult, i );
         return HB_SUCCESS;
      }
      case DBOI_BAGNUMBER:
      {
         LPNTXINDEX  pIndex   = pArea->lpIndexes, pIndexSeek = NULL;
         int         i        = 0;

         if( hb_itemGetCLen( pInfo->atomBagName ) > 0 )
            pIndexSeek = hb_ntxFindBag( pArea,
                                        hb_itemGetCPtr( pInfo->atomBagName ) );
         else if( pArea->lpCurTag )
            pIndexSeek = pArea->lpCurTag->Owner;

         if( pIndexSeek )
         {
            do
            {
               ++i;
               if( pIndex == pIndexSeek )
                  break;
               pIndex = pIndex->pNext;
            }
            while( pIndex );
         }
         hb_itemPutNI( pInfo->itmResult, pIndex ? i : 0 );
         return HB_SUCCESS;
      }
      case DBOI_BAGORDER:
      {
         LPNTXINDEX  pIndex   = pArea->lpIndexes, pIndexSeek = NULL;
         int         i        = 0;

         if( hb_itemGetCLen( pInfo->atomBagName ) > 0 )
            pIndexSeek = hb_ntxFindBag( pArea,
                                        hb_itemGetCPtr( pInfo->atomBagName ) );
         else if( pArea->lpCurTag )
            pIndexSeek = pArea->lpCurTag->Owner;

         if( pIndexSeek )
         {
            ++i;
            do
            {
               if( pIndex == pIndexSeek )
                  break;
               i        += pIndex->iTags;
               pIndex   = pIndex->pNext;
            }
            while( pIndex );
         }
         hb_itemPutNI( pInfo->itmResult, pIndex ? i : 0 );
         return HB_SUCCESS;
      }
   }

   if( SELF_GOCOLD( ( AREAP ) pArea ) == HB_FAILURE )
      return HB_FAILURE;

   pTag = hb_ntxFindTag( pArea, pInfo->itmOrder, pInfo->atomBagName );

   if( pTag )
   {
      switch( uiIndex )
      {
         case DBOI_CONDITION:
            hb_itemPutC( pInfo->itmResult, pTag->ForExpr ? pTag->ForExpr : NULL );
            if( hb_itemType( pInfo->itmNewVal ) & HB_IT_STRING )
            {
               const char * szForExpr = hb_itemGetCPtr( pInfo->itmNewVal );
               if( pTag->ForExpr ?
                   strncmp( pTag->ForExpr, szForExpr, NTX_MAX_EXP ) != 0 :
                   *szForExpr )
               {
                  PHB_ITEM pForItem = NULL;
                  BOOL     fOK      = *szForExpr == 0;
                  if( ! fOK )
                  {
                     if( SELF_COMPILE( ( AREAP ) pArea, szForExpr ) == HB_SUCCESS )
                     {
                        pForItem                      = pArea->dbfarea.area.valResult;
                        pArea->dbfarea.area.valResult = NULL;
                        if( SELF_EVALBLOCK( ( AREAP ) pArea, pForItem ) == HB_SUCCESS )
                        {
                           fOK                           = hb_itemType( pArea->dbfarea.area.valResult ) == HB_IT_LOGICAL;
                           hb_itemRelease( pArea->dbfarea.area.valResult );
                           pArea->dbfarea.area.valResult = NULL;
                        }
                     }
                  }
                  if( fOK && hb_ntxTagLockWrite( pTag ) )
                  {
                     if( pTag->ForExpr )
                        hb_xfree( pTag->ForExpr );
                     if( pTag->pForItem )
                        hb_vmDestroyBlockOrMacro( pTag->pForItem );
                     if( pForItem )
                     {
                        pTag->ForExpr  = hb_strndup( szForExpr, NTX_MAX_EXP );
                        pTag->pForItem = pForItem;
                        pForItem       = NULL;
                     }
                     else
                     {
                        pTag->ForExpr  = NULL;
                        pTag->pForItem = NULL;
                     }
                     pTag->Partial        = TRUE;
                     pTag->HdrChanged     = TRUE;
                     pTag->Owner->Update  = TRUE;
                     hb_ntxTagUnLockWrite( pTag );
                  }
                  if( pForItem )
                     hb_vmDestroyBlockOrMacro( pForItem );
               }
            }
            break;
         case DBOI_EXPRESSION:
            hb_itemPutC( pInfo->itmResult, pTag->KeyExpr );
            break;
         case DBOI_BAGNAME:
         {
            PHB_FNAME pFileName = hb_fsFNameSplit( pTag->Owner->IndexName );
            pInfo->itmResult = hb_itemPutC( pInfo->itmResult, pFileName->szName );
            hb_xfree( pFileName );
            break;
         }
         case DBOI_NAME:
            hb_itemPutC( pInfo->itmResult, pTag->TagName );
            break;
         case DBOI_NUMBER:
            hb_itemPutNI( pInfo->itmResult, hb_ntxFindTagNum( pArea, pTag ) );
            break;
         case DBOI_FILEHANDLE:
            hb_itemPutNInt( pInfo->itmResult,
                            ( HB_NHANDLE ) hb_fileHandle( pTag->Owner->DiskFile ) );
            break;
         case DBOI_FULLPATH:
            hb_itemPutC( pInfo->itmResult, pTag->Owner->IndexName );
            break;
         case DBOI_KEYCOUNT:
         case DBOI_KEYCOUNTRAW:
            hb_itemPutNL( pInfo->itmResult, hb_ntxOrdKeyCount( pTag ) );
            break;
         case DBOI_POSITION:
         case DBOI_KEYNORAW:
            /* case DBOI_RECNO: */
            if( hb_itemType( pInfo->itmNewVal ) & HB_IT_NUMERIC )
               hb_itemPutL( pInfo->itmResult,
                            hb_ntxOrdKeyGoto( pTag, hb_itemGetNL( pInfo->itmNewVal ) ) );
            else
               hb_itemPutNL( pInfo->itmResult, hb_ntxOrdKeyNo( pTag ) );
            break;
         case DBOI_RELKEYPOS:
            if( hb_itemType( pInfo->itmNewVal ) & HB_IT_NUMERIC )
               hb_ntxOrdSetRelKeyPos( pTag, hb_itemGetND( pInfo->itmNewVal ) );
            else
               hb_itemPutND( pInfo->itmResult, hb_ntxOrdGetRelKeyPos( pTag ) );
            break;
         case DBOI_ISCOND:
            hb_itemPutL( pInfo->itmResult, pTag->ForExpr != NULL );
            break;
         case DBOI_ISDESC:
            hb_itemPutL( pInfo->itmResult, pTag->fUsrDescend );
            if( hb_itemType( pInfo->itmNewVal ) == HB_IT_LOGICAL )
               pTag->fUsrDescend = hb_itemGetL( pInfo->itmNewVal );
            break;
         case DBOI_UNIQUE:
            hb_itemPutL( pInfo->itmResult, pTag->UniqueKey );
            break;
         case DBOI_CUSTOM:
            if( hb_itemType( pInfo->itmNewVal ) == HB_IT_LOGICAL )
            {
               if( hb_ntxTagLockWrite( pTag ) )
               {
                  if( ! pTag->Template )
                  {
                     BOOL fNewVal = hb_itemGetL( pInfo->itmNewVal );
                     if( pTag->Custom ? ! fNewVal : fNewVal )
                     {
                        pTag->Custom      = fNewVal;
                        pTag->Partial     = TRUE;
                        pTag->ChgOnly     = FALSE;
                        pTag->HdrChanged  = TRUE;
                     }
                  }
                  hb_ntxTagUnLockWrite( pTag );
               }
            }
            hb_itemPutL( pInfo->itmResult, pTag->Custom );
            break;
         case DBOI_CHGONLY:
            if( hb_itemType( pInfo->itmNewVal ) == HB_IT_LOGICAL )
            {
               if( hb_ntxTagLockWrite( pTag ) )
               {
                  if( ! pTag->Custom )
                  {
                     BOOL fNewVal = hb_itemGetL( pInfo->itmNewVal );
                     if( pTag->ChgOnly ? ! fNewVal : fNewVal )
                     {
                        pTag->ChgOnly     = fNewVal;
                        pTag->Partial     = TRUE;
                        pTag->HdrChanged  = TRUE;
                     }
                  }
                  hb_ntxTagUnLockWrite( pTag );
               }
            }
            hb_itemPutL( pInfo->itmResult, pTag->ChgOnly );
            break;
         case DBOI_TEMPLATE:
            if( hb_itemType( pInfo->itmNewVal ) == HB_IT_LOGICAL &&
                hb_itemGetL( pInfo->itmNewVal ) )
            {
               if( hb_ntxTagLockWrite( pTag ) )
               {
                  if( pTag->Custom && ! pTag->Template )
                  {
                     pTag->Template    = TRUE;
                     pTag->HdrChanged  = TRUE;
                  }
                  hb_ntxTagUnLockWrite( pTag );
               }
            }
            hb_itemPutL( pInfo->itmResult, pTag->Template );
            break;
         case DBOI_MULTIKEY:
            if( hb_itemGetL( pInfo->itmNewVal ) )
            {
               if( hb_ntxTagLockWrite( pTag ) )
               {
                  if( pTag->Custom && ! pTag->MultiKey )
                  {
                     pTag->MultiKey    = TRUE;
                     pTag->HdrChanged  = TRUE;
                  }
                  hb_ntxTagUnLockWrite( pTag );
               }
            }
            hb_itemPutL( pInfo->itmResult, pTag->MultiKey );
            break;
         case DBOI_PARTIAL:
            hb_itemPutL( pInfo->itmResult, pTag->Partial );
            break;
         case DBOI_SCOPETOP:
            if( pInfo->itmResult )
               hb_ntxTagGetScope( pTag, 0, pInfo->itmResult );
            if( pInfo->itmNewVal )
               hb_ntxTagSetScope( pTag, 0, pInfo->itmNewVal );
            break;
         case DBOI_SCOPEBOTTOM:
            if( pInfo->itmResult )
               hb_ntxTagGetScope( pTag, 1, pInfo->itmResult );
            if( pInfo->itmNewVal )
               hb_ntxTagSetScope( pTag, 1, pInfo->itmNewVal );
            break;
         case DBOI_SCOPESET:
            if( pInfo->itmNewVal )
            {
               hb_ntxTagSetScope( pTag, 0, pInfo->itmNewVal );
               hb_ntxTagSetScope( pTag, 1, pInfo->itmNewVal );
            }
            if( pInfo->itmResult )
               hb_itemClear( pInfo->itmResult );
            break;
         case DBOI_SCOPETOPCLEAR:
            if( pInfo->itmResult )
               hb_ntxTagGetScope( pTag, 0, pInfo->itmResult );
            hb_ntxTagClearScope( pTag, 0 );
            break;
         case DBOI_SCOPEBOTTOMCLEAR:
            if( pInfo->itmResult )
               hb_ntxTagGetScope( pTag, 1, pInfo->itmResult );
            hb_ntxTagClearScope( pTag, 1 );
            break;
         case DBOI_SCOPECLEAR:
            hb_ntxTagClearScope( pTag, 0 );
            hb_ntxTagClearScope( pTag, 1 );
            if( pInfo->itmResult )
               hb_itemClear( pInfo->itmResult );
            break;
         case DBOI_KEYADD:
            if( pTag->Owner->fReadonly )
            {
               hb_ntxErrorRT( pArea, EG_READONLY, EDBF_READONLY,
                              pTag->Owner->IndexName, 0, 0, NULL );
               return HB_FAILURE;
            }
            if( pTag->Custom )
            {
               hb_itemPutL( pInfo->itmResult,
                            hb_ntxOrdKeyAdd( pTag, pInfo->itmNewVal ) );
            }
            else
            {
               hb_ntxErrorRT( pArea, 0, EDBF_NOTCUSTOM, NULL, 0, 0, NULL );
               return HB_FAILURE;
            }
            break;
         case DBOI_KEYDELETE:
            if( pTag->Owner->fReadonly )
            {
               hb_ntxErrorRT( pArea, EG_READONLY, EDBF_READONLY,
                              pTag->Owner->IndexName, 0, 0, NULL );
               return HB_FAILURE;
            }
            if( pTag->Custom )
            {
               hb_itemPutL( pInfo->itmResult,
                            hb_ntxOrdKeyDel( pTag, pInfo->itmNewVal ) );
            }
            else
            {
               hb_ntxErrorRT( pArea, 0, EDBF_NOTCUSTOM, NULL, 0, 0, NULL );
               return HB_FAILURE;
            }
            break;
         case DBOI_KEYTYPE:
         {
            char szType[ 2 ];
            szType[ 0 ] = ( char ) pTag->KeyType;
            szType[ 1 ] = 0;
            hb_itemPutC( pInfo->itmResult, szType );
         }
         break;
         case DBOI_KEYSIZE:
            hb_itemPutNI( pInfo->itmResult, pTag->KeyLength );
            break;
         case DBOI_KEYDEC:
            hb_itemPutNI( pInfo->itmResult, pTag->KeyDec );
            break;
         case DBOI_KEYVAL:
            if( hb_ntxTagLockRead( pTag ) )
            {
               if( hb_ntxCurKeyRefresh( pTag ) )
                  hb_ntxKeyGetItem( pInfo->itmResult, pTag->CurKeyInfo, pTag, TRUE );
               else
                  hb_itemClear( pInfo->itmResult );
               hb_ntxTagUnLockRead( pTag );
            }
            break;
         case DBOI_SKIPUNIQUE:
            hb_itemPutL( pInfo->itmResult, hb_ntxOrdSkipUnique( pTag,
                                                                hb_itemGetNL( pInfo->itmNewVal ) ) );
            break;
         case DBOI_SKIPEVAL:
         case DBOI_SKIPEVALBACK:
            hb_itemPutL( pInfo->itmResult, hb_ntxOrdSkipEval( pTag,
                                                              uiIndex == DBOI_SKIPEVAL, pInfo->itmNewVal ) );
            break;
         case DBOI_SKIPWILD:
         case DBOI_SKIPWILDBACK:
            hb_itemPutL( pInfo->itmResult, hb_ntxOrdSkipWild( pTag,
                                                              uiIndex == DBOI_SKIPWILD, pInfo->itmNewVal ) );
            break;
         case DBOI_SKIPREGEX:
         case DBOI_SKIPREGEXBACK:
            hb_itemPutL( pInfo->itmResult, hb_ntxOrdSkipRegEx( pTag,
                                                               uiIndex == DBOI_SKIPREGEX, pInfo->itmNewVal ) );
            break;
         case DBOI_FINDREC:
         case DBOI_FINDRECCONT:
            hb_itemPutL( pInfo->itmResult, hb_ntxOrdFindRec( pTag,
                                                             hb_itemGetNL( pInfo->itmNewVal ),
                                                             uiIndex == DBOI_FINDRECCONT ) );
            break;
         case DBOI_SCOPEEVAL:
            if( hb_itemType( pInfo->itmNewVal ) == HB_IT_ARRAY &&
                hb_arrayLen( pInfo->itmNewVal ) == DBRMI_SIZE &&
                hb_arrayGetPtr( pInfo->itmNewVal, DBRMI_FUNCTION ) != NULL )
            {
               hb_itemPutNL( pInfo->itmResult, hb_ntxOrdScopeEval( pTag,
                                                                   ( HB_EVALSCOPE_FUNC )
                                                                   hb_arrayGetPtr( pInfo->itmNewVal, DBRMI_FUNCTION ),
                                                                   hb_arrayGetPtr( pInfo->itmNewVal, DBRMI_PARAM ),
                                                                   hb_arrayGetItemPtr( pInfo->itmNewVal, DBRMI_LOVAL ),
                                                                   hb_arrayGetItemPtr( pInfo->itmNewVal, DBRMI_HIVAL ) ) );
            }
            else
            {
               hb_itemPutNI( pInfo->itmResult, 0 );
            }
            break;
         case DBOI_UPDATECOUNTER:
            /* refresh update counter */
            if( hb_ntxIndexLockRead( pTag->Owner ) )
               hb_ntxIndexUnLockRead( pTag->Owner );
            hb_itemPutNInt( pInfo->itmResult, pTag->Owner->Version );
            break;
         case DBOI_READLOCK:
            if( hb_itemType( pInfo->itmNewVal ) == HB_IT_LOGICAL )
            {
               hb_itemPutL( pInfo->itmResult,
                            hb_itemGetL( pInfo->itmNewVal ) ?
                            hb_ntxIndexLockRead( pTag->Owner ) :
                            hb_ntxIndexUnLockRead( pTag->Owner ) );
            }
            else
            {
               hb_itemPutL( pInfo->itmResult, pTag->Owner->lockRead > 0 );
            }
            break;
         case DBOI_WRITELOCK:
            if( hb_itemType( pInfo->itmNewVal ) == HB_IT_LOGICAL )
            {
               hb_itemPutL( pInfo->itmResult,
                            hb_itemGetL( pInfo->itmNewVal ) ?
                            hb_ntxIndexLockWrite( pTag->Owner, TRUE ) :
                            hb_ntxIndexUnLockWrite( pTag->Owner ) );
            }
            else
            {
               hb_itemPutL( pInfo->itmResult, pTag->Owner->lockWrite > 0 );
            }
            break;
         case DBOI_ISSORTRECNO:
            hb_itemPutL( pInfo->itmResult, pTag->fSortRec );
            break;
         case DBOI_ISMULTITAG:
#if defined( HB_NTX_NOMULTITAG )
            hb_itemPutL( pInfo->itmResult, FALSE );
#else
            hb_itemPutL( pInfo->itmResult, pTag->Owner->Compound );
#endif
            break;
         case DBOI_LARGEFILE:
            hb_itemPutL( pInfo->itmResult, pTag->Owner->LargeFile );
            break;
         case DBOI_SHARED:
            hb_itemPutL( pInfo->itmResult, pTag->Owner->fShared );
            if( hb_itemType( pInfo->itmNewVal ) == HB_IT_LOGICAL )
               pTag->Owner->fShared = hb_itemGetL( pInfo->itmNewVal );
            break;
         case DBOI_ISREADONLY:
            hb_itemPutL( pInfo->itmResult, pTag->Owner->fReadonly );
            break;
         case DBOI_INDEXTYPE:
#if defined( HB_NTX_NOMULTITAG )
            hb_itemPutNI( pInfo->itmResult, DBOI_TYPE_NONCOMPACT );
#else
            hb_itemPutNI( pInfo->itmResult, pTag->Owner->Compound ?
                          DBOI_TYPE_COMPOUND : DBOI_TYPE_NONCOMPACT );
#endif
            break;
      }
   }
   else if( pInfo->itmResult )
   {
      switch( uiIndex )
      {
         case DBOI_KEYCOUNT:
         case DBOI_KEYCOUNTRAW:
         {
            ULONG ulRecCount = 0;
            SELF_RECCOUNT( ( AREAP ) pArea, &ulRecCount );
            hb_itemPutNInt( pInfo->itmResult, ulRecCount );
            break;
         }
         case DBOI_POSITION:
         case DBOI_KEYNORAW:
            /* case DBOI_RECNO: */
            if( pInfo->itmNewVal && hb_itemType( pInfo->itmNewVal ) & HB_IT_NUMERIC )
               hb_itemPutL( pInfo->itmResult, SELF_GOTO( ( AREAP ) pArea,
                                                         hb_itemGetNL( pInfo->itmNewVal ) ) == HB_SUCCESS );
            else
               SELF_RECID( ( AREAP ) pArea, pInfo->itmResult );
            break;
         case DBOI_RELKEYPOS:
            if( hb_itemType( pInfo->itmNewVal ) & HB_IT_NUMERIC )
            {
               double      dPos        = hb_itemGetND( pInfo->itmNewVal );
               LPTAGINFO   pSavedTag   = pArea->lpCurTag;
               pArea->lpCurTag = NULL;
               if( dPos >= 1.0 )
               {
                  SELF_GOBOTTOM( ( AREAP ) pArea );
               }
               else if( dPos <= 0.0 )
               {
                  SELF_GOTOP( ( AREAP ) pArea );
               }
               else
               {
                  ULONG ulRecCount, ulRecNo;
                  SELF_RECCOUNT( ( AREAP ) pArea, &ulRecCount );
                  ulRecNo = ( ULONG ) dPos * ulRecCount + 1;
                  if( ulRecNo >= ulRecCount )
                     ulRecNo = ulRecCount;
                  if( SELF_GOTO( ( AREAP ) pArea, ulRecNo ) == HB_SUCCESS &&
                      SELF_SKIPFILTER( ( AREAP ) pArea, 1 ) == HB_SUCCESS &&
                      pArea->dbfarea.area.fEof )
                     SELF_GOTOP( ( AREAP ) pArea );
               }
               pArea->lpCurTag = pSavedTag;
            }
            else
            {
               ULONG    ulRecNo  = 0, ulRecCount = 0;
               double   dPos     = 0.0;
               /* resolve any pending relations */
               if( SELF_RECNO( ( AREAP ) pArea, &ulRecNo ) == HB_SUCCESS )
               {
                  if( ! pArea->dbfarea.fPositioned )
                  {
                     if( ulRecNo > 1 )
                        dPos = 1.0;
                  }
                  else
                  {
                     SELF_RECCOUNT( ( AREAP ) pArea, &ulRecCount );
                     if( ulRecCount != 0 )
                        dPos = ( 0.5 + ulRecNo ) / ulRecCount;
                  }
               }
               hb_itemPutND( pInfo->itmResult, dPos );
            }
            break;
         case DBOI_SKIPUNIQUE:
            hb_itemPutL( pInfo->itmResult, SELF_SKIP( ( AREAP ) pArea,
                                                      hb_itemGetNL( pInfo->itmNewVal ) >= 0 ? 1 : -1 ) == HB_SUCCESS );
            break;
         case DBOI_SKIPEVAL:
         case DBOI_SKIPEVALBACK:
         case DBOI_SKIPWILD:
         case DBOI_SKIPWILDBACK:
         case DBOI_SKIPREGEX:
         case DBOI_SKIPREGEXBACK:
         case DBOI_FINDREC:
         case DBOI_FINDRECCONT:
            SELF_GOTO( ( AREAP ) pArea, 0 );
            hb_itemPutL( pInfo->itmResult, FALSE );
            break;
         case DBOI_ISCOND:
         case DBOI_ISDESC:
         case DBOI_UNIQUE:
         case DBOI_CUSTOM:
         case DBOI_KEYADD:
         case DBOI_KEYDELETE:

         case DBOI_ISSORTRECNO:
         case DBOI_ISMULTITAG:
         case DBOI_LARGEFILE:
         case DBOI_TEMPLATE:
         case DBOI_MULTIKEY:
         case DBOI_PARTIAL:
         case DBOI_CHGONLY:
         case DBOI_SHARED:
         case DBOI_ISREADONLY:
         case DBOI_WRITELOCK:
         case DBOI_READLOCK:
            hb_itemPutL( pInfo->itmResult, FALSE );
            break;
         case DBOI_KEYVAL:
         case DBOI_SCOPETOP:
         case DBOI_SCOPEBOTTOM:
         case DBOI_SCOPESET:
         case DBOI_SCOPETOPCLEAR:
         case DBOI_SCOPEBOTTOMCLEAR:
         case DBOI_SCOPECLEAR:
            hb_itemClear( pInfo->itmResult );
            break;
         case DBOI_KEYSIZE:
         case DBOI_KEYDEC:
         case DBOI_NUMBER:
         case DBOI_ORDERCOUNT:
         case DBOI_SCOPEEVAL:
         case DBOI_UPDATECOUNTER:
            hb_itemPutNI( pInfo->itmResult, 0 );
            break;
         case DBOI_FILEHANDLE:
            hb_itemPutNInt( pInfo->itmResult, ( HB_NHANDLE ) FS_ERROR );
            break;
         case DBOI_INDEXTYPE:
            hb_itemPutNI( pInfo->itmResult, DBOI_TYPE_UNDEF );
            break;
         case DBOI_BAGNAME:
         case DBOI_CONDITION:
         case DBOI_EXPRESSION:
         case DBOI_FULLPATH:
         case DBOI_NAME:
         case DBOI_KEYTYPE:
            hb_itemPutC( pInfo->itmResult, NULL );
            break;
         default:
            hb_itemClear( pInfo->itmResult );
      }
   }
   return HB_SUCCESS;
}

static HB_ERRCODE hb_ntxOrderListAdd( NTXAREAP pArea, LPDBORDERINFO pOrderInfo )
{
   USHORT      uiFlags;
   PHB_FILE    pFile;
   char        szFileName[ HB_PATH_MAX ], szTagName[ NTX_MAX_TAGNAME + 1 ];
   LPNTXINDEX  pIndex, * pIndexPtr;
   HB_ERRCODE  errCode;
   BOOL        fRetry, fReadonly, fShared, fProd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrderListAdd(%p, %p)", pArea, pOrderInfo ) );

   errCode = SELF_GOCOLD( ( AREAP ) pArea );
   if( errCode != HB_SUCCESS )
      return errCode;

   if( hb_itemGetCLen( pOrderInfo->atomBagName ) == 0 )
      return HB_FAILURE;

   hb_ntxCreateFName( pArea, hb_itemGetCPtr( pOrderInfo->atomBagName ),
                      &fProd, szFileName, szTagName );

/*
   if( ! szTagName[0] )
      return HB_FAILURE;
 */

   pIndex = hb_ntxFindBag( pArea, szFileName );

   if( ! pIndex )
   {
      PHB_ITEM pError = NULL;

      fReadonly   = pArea->dbfarea.fReadonly;
      fShared     = pArea->dbfarea.fShared;
      uiFlags     = ( fReadonly ? FO_READ : FO_READWRITE ) |
                    ( fShared ? FO_DENYNONE : FO_EXCLUSIVE );
      do
      {
         fRetry   = FALSE;
         pFile    = hb_fileExtOpen( szFileName, NULL, uiFlags |
                                    FXO_DEFAULTS | FXO_SHARELOCK | FXO_COPYNAME,
                                    NULL, pError, TRUE );
         if( ! pFile )
         {
            fRetry = hb_ntxErrorRT( pArea, EG_OPEN, EDBF_OPEN_INDEX, szFileName,
                                    hb_fsError(), EF_CANRETRY | EF_CANDEFAULT,
                                    &pError ) == E_RETRY;
         }
      }
      while( fRetry );

      if( pError )
         hb_errRelease( pError );

      if( ! pFile )
         return HB_FAILURE;

      pIndex               = hb_ntxIndexNew( pArea );
      pIndex->IndexName    = hb_strdup( szFileName );
      pIndex->fReadonly    = fReadonly;
      pIndex->fShared      = fShared;
      pIndex->DiskFile     = pFile;
      pIndex->Production   = fProd;

      pIndexPtr            = &pArea->lpIndexes;
      while( *pIndexPtr )
         pIndexPtr = &( *pIndexPtr )->pNext;
      *pIndexPtr           = pIndex;

      if( hb_ntxIndexLockRead( pIndex ) )
      {
         errCode = hb_ntxIndexLoad( pIndex, szTagName );
         hb_ntxIndexUnLockRead( pIndex );
      }
      else
         errCode = HB_FAILURE;

      if( errCode != HB_SUCCESS )
      {
         *pIndexPtr = pIndex->pNext;
         hb_ntxIndexFree( pIndex );
         hb_ntxErrorRT( pArea, EG_CORRUPTION, EDBF_CORRUPT, szFileName, 0, 0, NULL );
         return errCode;
      }
      /* hb_ntxSetTagNumbers() */
   }

   if( ! pArea->lpCurTag && pIndex->iTags )
   {
      pArea->lpCurTag   = pIndex->lpTags[ 0 ];
      errCode           = SELF_GOTOP( ( AREAP ) pArea );
   }
   return errCode;
}

static HB_ERRCODE hb_ntxOrderListClear( NTXAREAP pArea )
{
   LPNTXINDEX * pIndexPtr, pIndex;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrderListClear(%p)", pArea ) );

   if( SELF_GOCOLD( ( AREAP ) pArea ) == HB_FAILURE )
      return HB_FAILURE;

   pArea->lpCurTag   = NULL;
   pIndexPtr         = &pArea->lpIndexes;
   while( *pIndexPtr )
   {
      pIndex = *pIndexPtr;
      if( DBFAREA_DATA( &pArea->dbfarea )->fStruct && pIndex->Production &&
          ( DBFAREA_DATA( &pArea->dbfarea )->fStrictStruct ? pArea->dbfarea.fHasTags :
            hb_setGetAutOpen() ) )
      {
         pIndexPtr = &pIndex->pNext;
      }
      else
      {
         *pIndexPtr = pIndex->pNext;
         hb_ntxIndexFree( pIndex );
      }
   }
   /* hb_ntxSetTagNumbers() */
   return HB_SUCCESS;
}

static HB_ERRCODE hb_ntxOrderListDelete( NTXAREAP pArea, LPDBORDERINFO pOrderInfo )
{
   char        szTagName[ NTX_MAX_TAGNAME + 1 ];
   char        szFileName[ HB_PATH_MAX ];
   LPNTXINDEX  pIndex, * pIndexPtr;
   BOOL        fProd;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrderListDelete(%p, %p)", pArea, pOrderInfo ) );

   if( SELF_GOCOLD( ( AREAP ) pArea ) == HB_FAILURE )
      return HB_FAILURE;

   hb_ntxCreateFName( pArea, hb_itemGetCPtr( pOrderInfo->atomBagName ), &fProd,
                      szFileName, szTagName );
   pIndex = hb_ntxFindBag( pArea, szFileName );

   if( pIndex && ! ( pIndex->Production && DBFAREA_DATA( &pArea->dbfarea )->fStruct &&
                     ( DBFAREA_DATA( &pArea->dbfarea )->fStrictStruct ?
                       pArea->dbfarea.fHasTags : hb_setGetAutOpen() ) ) )
   {
      pIndexPtr = &pArea->lpIndexes;
      while( *pIndexPtr )
      {
         if( pIndex == *pIndexPtr )
         {
            *pIndexPtr = pIndex->pNext;
            hb_ntxIndexFree( pIndex );
            /* hb_ntxSetTagNumbers() */
            break;
         }
         pIndexPtr = &( *pIndexPtr )->pNext;
      }
   }
   return HB_SUCCESS;
}

static HB_ERRCODE hb_ntxOrderListFocus( NTXAREAP pArea, LPDBORDERINFO pOrderInfo )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrderListFocus(%p, %p)", pArea, pOrderInfo ) );

   pOrderInfo->itmResult = hb_itemPutC( pOrderInfo->itmResult,
                                        pArea->lpCurTag ? pArea->lpCurTag->TagName : NULL );

   if( pOrderInfo->itmOrder )
   {
      /*
       * In Clipper tag is not changed when bad name is given in DBFNTX
       * but not in DBFCDX. I'd like to keep the same behavior in
       * [x]Harbour RDDs and I chosen DBFCDX one as default. [druzus]
       */
#ifdef HB_C52_STRICT
      LPTAGINFO pTag = hb_ntxFindTag( pArea, pOrderInfo->itmOrder,
                                      pOrderInfo->atomBagName );
      if( pTag )
         pArea->lpCurTag = pTag;
#else
      pArea->lpCurTag = hb_ntxFindTag( pArea, pOrderInfo->itmOrder,
                                       pOrderInfo->atomBagName );
#endif
   }

   return HB_SUCCESS;
}

static HB_ERRCODE hb_ntxOrderListRebuild( NTXAREAP pArea )
{
   LPTAGINFO   pCurrTag;
   LPNTXINDEX  pIndex;
   HB_ERRCODE  errCode;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxOrderListRebuild(%p)", pArea ) );

   errCode = SELF_GOCOLD( ( AREAP ) pArea );
   if( errCode != HB_SUCCESS )
      return errCode;

   if( pArea->dbfarea.fShared )
   {
      hb_ntxErrorRT( pArea, EG_SHARED, EDBF_SHARED, pArea->dbfarea.szDataFileName, 0, 0, NULL );
      return HB_FAILURE;
   }
   if( pArea->dbfarea.fReadonly )
   {
      hb_ntxErrorRT( pArea, EG_READONLY, EDBF_READONLY, pArea->dbfarea.szDataFileName, 0, 0, NULL );
      return HB_FAILURE;
   }

   if( pArea->dbfarea.lpdbPendingRel )
   {
      errCode = SELF_FORCEREL( ( AREAP ) pArea );
      if( errCode != HB_SUCCESS )
         return errCode;
   }
   pCurrTag          = pArea->lpCurTag;
   pArea->lpCurTag   = NULL;
   pIndex            = pArea->lpIndexes;
   while( pIndex && errCode == HB_SUCCESS )
   {
      errCode  = hb_ntxReIndex( pIndex );
      pIndex   = pIndex->pNext;
   }
   if( errCode == HB_SUCCESS )
   {
      pArea->lpCurTag   = pCurrTag;
      errCode           = SELF_GOTOP( ( AREAP ) pArea );
   }
   return errCode;
}

#define hb_ntxClearFilter  NULL
#define hb_ntxClearLocate  NULL
#define hb_ntxClearScope   NULL

static HB_ERRCODE hb_ntxCountScope( NTXAREAP pArea, void * pPtr, LONG * plRecNo )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxCountScope(%p, %p, %p)", pArea, pPtr, plRecNo ) );

   if( pPtr == NULL )
   {
      return HB_SUCCESS;
   }
   return SUPER_COUNTSCOPE( ( AREAP ) pArea, pPtr, plRecNo );
}

#define hb_ntxFilterText      NULL
#define hb_ntxScopeInfo       NULL
#define hb_ntxSetFilter       NULL
#define hb_ntxSetLocate       NULL
#define hb_ntxSetScope        NULL
#define hb_ntxSkipScope       NULL
#define hb_ntxLocate          NULL
#define hb_ntxCompile         NULL
#define hb_ntxError           NULL
#define hb_ntxEvalBlock       NULL
#define hb_ntxRawLock         NULL
#define hb_ntxLock            NULL
#define hb_ntxUnLock          NULL
#define hb_ntxCloseMemFile    NULL
#define hb_ntxCreateMemFile   NULL
#define hb_ntxGetValueFile    NULL
#define hb_ntxOpenMemFile     NULL
#define hb_ntxPutValueFile    NULL
#define hb_ntxReadDBHeader    NULL
#define hb_ntxWriteDBHeader   NULL

static HB_ERRCODE hb_ntxInit( LPRDDNODE pRDD )
{
   HB_ERRCODE errCode;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxInit(%p)", pRDD ) );

   errCode = SUPER_INIT( pRDD );
   if( errCode == HB_SUCCESS )
   {
      PHB_ITEM pItem = hb_itemPutNI( NULL, DB_MEMO_DBT );
      SELF_RDDINFO( pRDD, RDDI_MEMOTYPE, 0, pItem );
      hb_itemRelease( pItem );
#if ! defined( HB_NTX_NOMULTITAG )
      DBFNODE_DATA( pRDD )->fMultiTag = TRUE;
#endif
   }
   return errCode;
}

#define hb_ntxExit   NULL
#define hb_ntxDrop   NULL
#define hb_ntxExists NULL
#define hb_ntxRename NULL

static HB_ERRCODE hb_ntxRddInfo( LPRDDNODE pRDD, USHORT uiIndex, ULONG ulConnect, PHB_ITEM pItem )
{
   LPDBFDATA pData;

   HB_TRACE( HB_TR_DEBUG, ( "hb_ntxRddInfo(%p, %hu, %lu, %p)", pRDD, uiIndex, ulConnect, pItem ) );

   pData = DBFNODE_DATA( pRDD );

   switch( uiIndex )
   {
      case RDDI_ORDBAGEXT:
      case RDDI_ORDEREXT:
      case RDDI_ORDSTRUCTEXT:
      {
         const char *   szNew = hb_itemGetCPtr( pItem );
         char *         szNewVal;

         szNewVal = szNew[ 0 ] == '.' && szNew[ 1 ] ? hb_strdup( szNew ) : NULL;
         hb_itemPutC( pItem, pData->szIndexExt[ 0 ] ? pData->szIndexExt : NTX_INDEXEXT );
         if( szNewVal )
         {
            hb_strncpy( pData->szIndexExt, szNewVal, sizeof( pData->szIndexExt ) - 1 );
            hb_xfree( szNewVal );
         }
         break;
      }

      case RDDI_MULTITAG:
      {
#if defined( HB_NTX_NOMULTITAG )
         hb_itemPutL( pItem, FALSE );
#else
         BOOL fMultiTag = pData->fMultiTag;
         if( hb_itemType( pItem ) == HB_IT_LOGICAL )
            pData->fMultiTag = hb_itemGetL( pItem );
         hb_itemPutL( pItem, fMultiTag );
#endif
         break;
      }

      case RDDI_SORTRECNO:
      {
         BOOL fSortRecNo = pData->fSortRecNo;
         if( hb_itemType( pItem ) == HB_IT_LOGICAL )
            pData->fSortRecNo = hb_itemGetL( pItem );
         hb_itemPutL( pItem, fSortRecNo );
         break;
      }

      case RDDI_STRUCTORD:
      {
         BOOL fStruct = pData->fStruct;
         if( hb_itemType( pItem ) == HB_IT_LOGICAL )
            pData->fStruct = hb_itemGetL( pItem );
         hb_itemPutL( pItem, fStruct );
         break;
      }

      case RDDI_STRICTSTRUCT:
      {
         BOOL fStrictStruct = pData->fStrictStruct;
         if( hb_itemType( pItem ) == HB_IT_LOGICAL )
            pData->fStrictStruct = hb_itemGetL( pItem );
         hb_itemPutL( pItem, fStrictStruct );
         break;
      }

      case RDDI_MULTIKEY:
      {
         BOOL fMultiKey = pData->fMultiKey;
         if( hb_itemType( pItem ) == HB_IT_LOGICAL )
            pData->fMultiKey = hb_itemGetL( pItem );
         hb_itemPutL( pItem, fMultiKey );
         break;
      }


      default:
         return SUPER_RDDINFO( pRDD, uiIndex, ulConnect, pItem );

   }

   return HB_SUCCESS;
}

#define hb_ntxWhoCares NULL

static const RDDFUNCS ntxTable = {
   hb_ntxBof,
   hb_ntxEof,
   hb_ntxFound,
   ( DBENTRYP_V ) hb_ntxGoBottom,
   hb_ntxGoTo,
   hb_ntxGoToId,
   ( DBENTRYP_V ) hb_ntxGoTop,
   ( DBENTRYP_BIB ) hb_ntxSeek,
   hb_ntxSkip,
   hb_ntxSkipFilter,
   ( DBENTRYP_L ) hb_ntxSkipRaw,
   hb_ntxAddField,
   ( DBENTRYP_B ) hb_ntxAppend,
   hb_ntxCreateFields,
   hb_ntxDeleteRec,
   hb_ntxDeleted,
   hb_ntxFieldCount,
   hb_ntxFieldDisplay,
   hb_ntxFieldInfo,
   hb_ntxFieldName,
   ( DBENTRYP_V ) hb_ntxFlush,
   hb_ntxGetRec,
   hb_ntxGetValue,
   hb_ntxGetVarLen,
   ( DBENTRYP_V ) hb_ntxGoCold,
   ( DBENTRYP_V ) hb_ntxGoHot,
   hb_ntxPutRec,
   hb_ntxPutValue,
   hb_ntxRecall,
   hb_ntxRecCount,
   hb_ntxRecInfo,
   hb_ntxRecNo,
   hb_ntxRecId,
   hb_ntxSetFieldsExtent,
   hb_ntxAlias,
   ( DBENTRYP_V ) hb_ntxClose,
   hb_ntxCreate,
   hb_ntxInfo,
   hb_ntxNewArea,
   ( DBENTRYP_VO ) hb_ntxOpen,
   hb_ntxRelease,
   ( DBENTRYP_SP ) hb_ntxStructSize,
   hb_ntxSysName,
   hb_ntxEval,
   ( DBENTRYP_V ) hb_ntxPack,
   ntPackRec,
   hb_ntxSort,
   hb_ntxTrans,
   hb_ntxTransRec,
   ( DBENTRYP_V ) hb_ntxZap,
   hb_ntxchildEnd,
   hb_ntxchildStart,
   hb_ntxchildSync,
   hb_ntxsyncChildren,
   hb_ntxclearRel,
   hb_ntxforceRel,
   hb_ntxrelArea,
   hb_ntxrelEval,
   hb_ntxrelText,
   hb_ntxsetRel,
   ( DBENTRYP_VOI ) hb_ntxOrderListAdd,
   ( DBENTRYP_V ) hb_ntxOrderListClear,
   ( DBENTRYP_VOI ) hb_ntxOrderListDelete,
   ( DBENTRYP_VOI ) hb_ntxOrderListFocus,
   ( DBENTRYP_V ) hb_ntxOrderListRebuild,
   hb_ntxOrderCondition,
   ( DBENTRYP_VOC ) hb_ntxOrderCreate,
   ( DBENTRYP_VOI ) hb_ntxOrderDestroy,
   ( DBENTRYP_SVOI ) hb_ntxOrderInfo,
   hb_ntxClearFilter,
   hb_ntxClearLocate,
   hb_ntxClearScope,
   ( DBENTRYP_VPLP ) hb_ntxCountScope,
   hb_ntxFilterText,
   hb_ntxScopeInfo,
   hb_ntxSetFilter,
   hb_ntxSetLocate,
   hb_ntxSetScope,
   hb_ntxSkipScope,
   hb_ntxLocate,
   hb_ntxCompile,
   hb_ntxError,
   hb_ntxEvalBlock,
   hb_ntxRawLock,
   hb_ntxLock,
   hb_ntxUnLock,
   hb_ntxCloseMemFile,
   hb_ntxCreateMemFile,
   hb_ntxGetValueFile,
   hb_ntxOpenMemFile,
   hb_ntxPutValueFile,
   hb_ntxReadDBHeader,
   hb_ntxWriteDBHeader,
   hb_ntxInit,
   hb_ntxExit,
   hb_ntxDrop,
   hb_ntxExists,
   hb_ntxRename,
   hb_ntxRddInfo,
   hb_ntxWhoCares
};

HB_FUNC( DBFNTX )
{
   ;
}

HB_FUNC( DBFNTX_GETFUNCTABLE )
{
   RDDFUNCS *  pTable;
   USHORT *    uiCount, uiRddId;

   uiCount  = ( USHORT * ) hb_parptr( 1 );
   pTable   = ( RDDFUNCS * ) hb_parptr( 2 );
   uiRddId  = hb_parni( 4 );

   if( pTable )
   {
      HB_ERRCODE errCode;

      if( uiCount )
         *uiCount = RDDFUNCSCOUNT;
      errCode = hb_rddInherit( pTable, &ntxTable, &ntxSuper, ( const char * ) "DBFFPT" );
      if( errCode != HB_SUCCESS )
         errCode = hb_rddInherit( pTable, &ntxTable, &ntxSuper, ( const char * ) "DBFDBT" );
      if( errCode != HB_SUCCESS )
         errCode = hb_rddInherit( pTable, &ntxTable, &ntxSuper, ( const char * ) "DBF" );
      if( errCode == HB_SUCCESS )
      {
         /*
          * we successfully register our RDD so now we can initialize it
          * You may think that this place is RDD init statement, Druzus
          */
         s_uiRddId = uiRddId;
      }
      hb_retni( errCode );
   }
   else
   {
      hb_retni( HB_FAILURE );
   }
}


#define __PRG_SOURCE__     __FILE__

#ifdef HB_PCODE_VER
#  undef HB_PRG_PCODE_VER
#  define HB_PRG_PCODE_VER HB_PCODE_VER
#endif

HB_FUNC_EXTERN( _DBF );

static void hb_dbfntxRddInit( void * cargo )
{
   HB_SYMBOL_UNUSED( cargo );

   if( hb_rddRegister( "DBF", RDT_FULL ) <= 1 )
   {
      hb_rddRegister( "DBFFPT", RDT_FULL );
      if( hb_rddRegister( "DBFNTX", RDT_FULL ) <= 1 )
         return;
   }

   hb_errInternal( HB_EI_RDDINVALID, NULL, NULL, NULL );

   /* not executed, only to force linking DBF RDD */
   HB_FUNC_EXEC( _DBF );
}

HB_INIT_SYMBOLS_BEGIN( dbfntx1__InitSymbols )
{
   "DBFNTX", { HB_FS_PUBLIC | HB_FS_LOCAL }, { HB_FUNCNAME( DBFNTX ) }, NULL
},
{ "DBFNTX_GETFUNCTABLE", { HB_FS_PUBLIC | HB_FS_LOCAL }, { HB_FUNCNAME( DBFNTX_GETFUNCTABLE ) }, NULL }
HB_INIT_SYMBOLS_END( dbfntx1__InitSymbols )

HB_CALL_ON_STARTUP_BEGIN( _hb_dbfntx_rdd_init_ )
hb_vmAtInit( hb_dbfntxRddInit, NULL );
HB_CALL_ON_STARTUP_END( _hb_dbfntx_rdd_init_ )

#if defined( HB_PRAGMA_STARTUP )
   #pragma startup dbfntx1__InitSymbols
   #pragma startup _hb_dbfntx_rdd_init_
#elif defined( HB_DATASEG_STARTUP )
   #define HB_DATASEG_BODY HB_DATASEG_FUNC( dbfntx1__InitSymbols ) \
   HB_DATASEG_FUNC( _hb_dbfntx_rdd_init_ )
   #include "hbiniseg.h"
#endif
