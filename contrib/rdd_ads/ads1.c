/*
 * $Id: ads1.c 9752 2012-10-14 03:33:16Z andijahja $
 */

/*
 * Harbour Project source code:
 * Advantage Database Server RDD
 *
 * Copyright 1999 Alexander Kresin <alex@belacy.belgorod.su>
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

#define HB_NEEDS_ACE

#define HB_OS_WIN_USED

#define HB_SET_IMPORT
#define SUPERTABLE ( &adsSuper )
#define MAX_STR_LEN 255

#include "hbvm.h"
#include "hbinit.h"
#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"
#include "hbdbferr.h"
#include "hbapilng.h"
#include "hbdate.h"
#include "hbset.h"

#include "rddsys.ch"
#include "rddads.h"

#include <ctype.h>

static int s_iSetListenerHandle = 0;

static USHORT s_uiRddCount = 0;
static USHORT s_uiRddIdADS = ( USHORT ) -1;
static USHORT s_uiRddIdADT = ( USHORT ) -1;
static USHORT s_uiRddIdADSNTX = ( USHORT ) -1;
static USHORT s_uiRddIdADSCDX = ( USHORT ) -1;
#if ADS_LIB_VERSION >= 900
static USHORT s_uiRddIdADSVFP = ( USHORT ) -1;
#endif
static BOOL s_bRefreshCount = TRUE;

static RDDFUNCS adsSuper;


/*
 * -- HELPER FUNCTIONS --
 */

static void adsSetListener_callback( HB_set_enum setting, HB_set_listener_enum when )
{
   HB_TRACE(HB_TR_DEBUG, ("adsSetListener_callback (%d, %d)", setting, when));
   if( when == HB_SET_LISTENER_AFTER )  /* we don't do anything with BEFORE calls */
   {
      switch( setting )
      {
         case HB_SET_DATEFORMAT :
            AdsSetDateFormat( ( UNSIGNED8 * ) hb_setGetCPtr( HB_SET_DATEFORMAT ) );
            break;
         case HB_SET_DEFAULT    :
            AdsSetDefault( ( UNSIGNED8 * ) hb_setGetCPtr( HB_SET_DEFAULT ) );
            break;
         case HB_SET_DELETED    :
            AdsShowDeleted( ( UNSIGNED16 ) ! hb_setGetL( HB_SET_DELETED ) );
            break;
         case HB_SET_EPOCH      :
            AdsSetEpoch( ( UNSIGNED16 ) hb_setGetNI( HB_SET_EPOCH ) );
            break;
         case HB_SET_EXACT      :
            AdsSetExact( ( UNSIGNED16 ) hb_setGetL( HB_SET_EXACT ) );
            break;
         case HB_SET_PATH       :
            AdsSetSearchPath( ( UNSIGNED8 * ) hb_setGetCPtr( HB_SET_PATH ) );
            break;

         case HB_SET_DECIMALS   :
            AdsSetDecimals( ( UNSIGNED16 ) hb_setGetNI( HB_SET_DECIMALS ) );
            break;

/* Possible TODO?
         case HB_SET_MFILEEXT   :
            if( hb_setGetCPtr( HB_SET_MFILEEXT ) )
            {
               hb_retc( hb_setGetCPtr( HB_SET_MFILEEXT ) );
            }
            break;
         case HB_SET_STRICTREAD :
            hb_retl( hb_setGetL( HB_SET_STRICTREAD ) );
            break;
*/
         default:
            break;
      }
   }
}

static void adsSetSend( void )
{
   HB_TRACE(HB_TR_DEBUG, ("adsSetSend()"));

   AdsSetDateFormat( ( UNSIGNED8 * ) hb_setGetCPtr( HB_SET_DATEFORMAT ) );
   AdsSetDefault( ( UNSIGNED8 * ) hb_setGetCPtr( HB_SET_DEFAULT ) );
   AdsShowDeleted( ( UNSIGNED16 ) ! hb_setGetL( HB_SET_DELETED ) );
   AdsSetEpoch( ( UNSIGNED16 ) hb_setGetNI( HB_SET_EPOCH ) );
   AdsSetExact( ( UNSIGNED16 ) hb_setGetL( HB_SET_EXACT ) );
   AdsSetSearchPath( ( UNSIGNED8 * ) hb_setGetCPtr( HB_SET_PATH ) );
   AdsSetDecimals( ( UNSIGNED16 ) hb_setGetNI( HB_SET_DECIMALS ) );
}

static HB_ERRCODE commonError( ADSAREAP pArea, USHORT uiGenCode, USHORT uiSubCode,
                               USHORT uiOsCode, const char * szFileName,
                               USHORT uiFlags, PHB_ITEM * pErrorPtr )
{
   PHB_ITEM pError;
   HB_ERRCODE errCode = HB_FAILURE;

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
      hb_errPutDescription( pError, hb_langDGetErrorDesc( uiGenCode ) );
      if( uiOsCode )
         hb_errPutOsCode( pError, uiOsCode );
      if( szFileName )
         hb_errPutFileName( pError, szFileName );
      if( uiFlags )
         hb_errPutFlags( pError, uiFlags );
      errCode = SUPER_ERROR( ( AREAP ) pArea, pError );
      if( !pErrorPtr )
         hb_itemRelease( pError );
   }
   return errCode;
}

/*
 * it's not used - temporary disabled to avoid warnings,
 * enable it if necessary
 */
#if 0
static void DumpArea( ADSAREAP pArea )  /* For debugging: call this to dump ads server settings to HB_TRACE. Currently in a quick-and-dirty state... */
{
   UNSIGNED8  pucTemp[ 1025 ];
   UNSIGNED16 pusLen = 1024;
   UNSIGNED32 ulRetVal, ulRetAOF, ulRetFilt;
   UNSIGNED8  pucFormat[ 16 ];
   UNSIGNED8  pucFilter[ 1025 ];
/* UNSIGNED8  aucBuffer[ MAX_STR_LEN + 1 ]; */
   UNSIGNED8  pucIndexName[ MAX_STR_LEN + 1 ];
   UNSIGNED8  pucIndexExpr[ MAX_STR_LEN + 1 ];
   UNSIGNED8  pucIndexCond[ MAX_STR_LEN + 1 ];

   if( pArea )
   {
      pusLen = 15;
      AdsGetDateFormat( pucFormat, &pusLen );
      pusLen = 1024;
      ulRetVal = AdsGetTableAlias( pArea->hTable, pucTemp, &pusLen );
      AdsGetEpoch( &pusLen );
      HB_TRACE(HB_TR_ALWAYS, ("DumpArea: \n    pArea: %p  hTable: %lu  Alias: %s (RetVal %lu)\n      Eof: %d  DateFormat: %s  Epoch: %d",
         pArea, pArea->hTable, pucTemp, ulRetVal, pArea->area.fEof, pucFormat, pusLen));

      pusLen = 1024;
      ulRetAOF = AdsGetAOF( pArea->hTable, pucTemp, &pusLen );
      pusLen = 1024;
      ulRetFilt = AdsGetFilter( pArea->hTable, pucFilter, &pusLen );
      HB_TRACE(HB_TR_ALWAYS, ("DumpArea AOF: (RetVal %lu) %s \n     Filter: (RetVal %lu) %s", ulRetAOF, pucTemp, ulRetFilt, pucFilter));

      if( pArea->hOrdCurrent )
      {
         pusLen = MAX_STR_LEN;
         AdsGetIndexName( pArea->hOrdCurrent, pucIndexName, &pusLen );
         pusLen = MAX_STR_LEN;
         AdsGetIndexCondition( pArea->hOrdCurrent, pucIndexCond, &pusLen );
         pusLen = MAX_STR_LEN;
         AdsGetIndexExpr( pArea->hOrdCurrent, pucIndexExpr, &pusLen );

         pusLen = 1024;   /*ADS top/bottom are 1,2 instead of 0,1*/
         ulRetVal  = AdsGetScope( pArea->hOrdCurrent, ADS_TOP, pucTemp, &pusLen );
         pusLen = 1024;
         ulRetFilt = AdsGetScope( pArea->hOrdCurrent, ADS_BOTTOM, pucFilter, &pusLen );

         HB_TRACE(HB_TR_ALWAYS, ("DumpArea Index: %s   Expr: %s  Cond: %s\n        Scope: (RetVal %lu) %s  Bottom: (RetVal %lu) %s",
               pucIndexName, pucIndexExpr, pucIndexCond, ulRetVal, pucTemp, ulRetFilt, pucFilter));
      }
   }
}
#endif

static BOOL adsIndexKeyCmp( ADSHANDLE hIndex, UNSIGNED8 * pszKey, UNSIGNED16 u16KeyLen )
{
   UNSIGNED32 u32RetVal;
   UNSIGNED16 u16Found = FALSE;
   UNSIGNED8  pucCurKey[ ADS_MAX_KEY_LENGTH + 1 ];
   UNSIGNED16 u16CurKeyLen = ADS_MAX_KEY_LENGTH;

   /*
    * test if current record has fields that match the given key expression.
    * This is used to evaluate if a seek expression continues to eval to .t.
    * when skipping through filtered records
    */

   u32RetVal = AdsExtractKey( hIndex, pucCurKey, &u16CurKeyLen );
   if( u32RetVal == AE_SUCCESS )
   {
      if( u16CurKeyLen )
      {
         if( u16CurKeyLen >= u16KeyLen &&
             memcmp( ( UNSIGNED8 * ) pucCurKey, ( UNSIGNED8 * ) pszKey, u16KeyLen ) == 0 )
         {
            u16Found = TRUE;
         }
      }
   }

   return  u16Found;
}


static int adsGetFileType( USHORT uiRddID )
{
   if(      uiRddID == s_uiRddIdADSCDX )
      return ADS_CDX;
   else if( uiRddID == s_uiRddIdADSNTX )
      return ADS_NTX;
   else if( uiRddID == s_uiRddIdADT )
      return ADS_ADT;
#if ADS_LIB_VERSION >= 900
   else if( uiRddID == s_uiRddIdADSVFP )
      return ADS_VFP;
#endif

   return hb_ads_iFileType;
}

static const char * adsTableExt( int iFileType )
{
   return iFileType == ADS_ADT ? ".adt" : ".dbf";
}

static const char * adsMemoExt( int iFileType )
{
   switch( iFileType )
   {
   case ADS_ADT: return ".adm";
   case ADS_CDX: return ".fpt";
#if ADS_LIB_VERSION >= 900
   case ADS_VFP: return ".fpt";
#endif
   }

   return ".dbt";
}

static const char * adsIndexExt( int iFileType )
{
   switch( iFileType )
   {
   case ADS_ADT: return ".adi";
   case ADS_CDX: return ".cdx";
#if ADS_LIB_VERSION >= 900
   case ADS_VFP: return ".cdx";
#endif
   }

   return ".ntx";
}

static ADSHANDLE hb_adsFindBag( ADSAREAP pArea, const char * szBagName )
{
   /* This method seems to be most easy one though I'm doubt
      it's really the best one */
#if 1
   ADSHANDLE ahIndex[ 1 ];
   UNSIGNED16 u16Count = 1;
   UNSIGNED32 u32Result;

   u32Result = AdsOpenIndex( pArea->hTable,
                             ( UNSIGNED8* ) szBagName, ahIndex, &u16Count );
   if( u32Result == AE_INDEX_ALREADY_OPEN )
      return ahIndex[ 0 ];

   if( u32Result == AE_SUCCESS )
      AdsCloseIndex( ahIndex[ 0 ] );

   return 0;
#else
   UNSIGNED8 pucName[ MAX_STR_LEN + 1 ];
   UNSIGNED16 u16Option, u16Count, u16len, u;
   ADSHANDLE hIndex = 0, hOrder;
   PHB_FNAME pFileName;

   pFileName = hb_fsFNameSplit( szBagName );
   if( pFileName->szPath )
      u16Option = ADS_FULLPATHNAME;
   else if( pFileName->szExtension )
      u16Option = ADS_BASENAMEANDEXT;
   else
      u16Option = ADS_BASENAME;
   hb_xfree( pFileName );

   if( AdsGetNumIndexes( pArea->hTable, &u16Count ) == AE_SUCCESS )
   {
      for( u = 1; u <= u16Count; ++u )
      {
         if( AdsGetIndexHandleByOrder( pArea->hTable, u, &hOrder ) != AE_SUCCESS )
            break;
         u16len = MAX_STR_LEN;
         if( AdsGetIndexFilename( hOrder, u16Option, pucName, &u16len ) != AE_SUCCESS )
            break;
         if( !hb_stricmp( szBagName, ( char * ) pucName ) )
         {
            hIndex = hOrder;
            break;
         }
      }
   }
   return hIndex;
#endif
}

static HB_ERRCODE hb_adsUpdateAreaFlags( ADSAREAP pArea )
{
   UNSIGNED16 u16Bof, u16Eof, u16Found;

   AdsAtBOF( pArea->hTable, &u16Bof );
   AdsAtEOF( pArea->hTable, &u16Eof );
   AdsIsFound( pArea->hTable, &u16Found );

   pArea->area.fBof = u16Bof != 0;
   pArea->area.fEof = u16Eof != 0;
   pArea->area.fFound = u16Found != 0;

   pArea->fPositioned = !pArea->area.fBof && !pArea->area.fEof;

   return HB_SUCCESS;
}

static HB_ERRCODE hb_adsCheckLock( ADSAREAP pArea )
{
   if( hb_ads_bTestRecLocks && pArea->fShared && !pArea->fFLocked )
   {
      UNSIGNED16 u16Locked = FALSE;
      UNSIGNED32 u32RetVal;

      u32RetVal = AdsIsRecordLocked( pArea->hTable, 0, &u16Locked );
      if( u32RetVal != AE_SUCCESS )
      {
         hb_errRT_DBCMD( EG_UNLOCKED, (HB_ERRCODE) u32RetVal, "Lock Required by TestRecLocks", "ADSISRECORDLOCKED" );
         return HB_FAILURE;
      }
      if( !u16Locked )
      {
         commonError( pArea, EG_UNLOCKED, EDBF_UNLOCKED, 0, NULL, 0, NULL );
         return HB_FAILURE;
      }
   }
   return HB_SUCCESS;
}

static void adsGetKeyItem( ADSAREAP pArea, PHB_ITEM pItem, int iKeyType,
                           char * pKeyBuf, int iKeyLen )
{
   double dValue;

   switch( iKeyType )
   {
   /*
      TODO: ADS_RAW only partially supported.  Presumed string.
            ADT files can use ";" concatentation operator, which returns index key types as Raw
   */
      case ADS_RAW:
         /* hack for timestamp values, we need sth better yo detect timestamp indexes */
         if( pArea->iFileType == ADS_ADT && pKeyBuf[ 0 ] == 0 && ( iKeyLen == 8 || iKeyLen == 4 ) )
         {
            LONG lDate, lTime;
            lDate = HB_GET_BE_UINT32( pKeyBuf );
            if( iKeyLen == 8 )
            {
               lTime = HB_GET_BE_UINT32( &pKeyBuf[ 4 ] );
               /* ADS stores milliseconds in raw ADT form increased by one */
               if( lTime )
                  --lTime;
               hb_itemPutDTL( pItem, lDate, lTime );
            }
            else
               hb_itemPutDL( pItem, lDate );
            break;
         }
#if ADS_LIB_VERSION >= 900
         else if( pArea->iFileType == ADS_VFP && iKeyLen == 8 )
         {
            HB_ORD2DBL( pKeyBuf, &dValue );
            hb_itemPutDTD( pItem, dValue );
            break;
         }
#endif
      case ADS_STRING:
         hb_itemPutCL( pItem, pKeyBuf, iKeyLen );
         break;

      case ADS_NUMERIC:
         if( pArea->iFileType == ADS_NTX )
         {
            int iLen = iKeyLen, iDec;
            HB_LONG lValue;

            if( *pKeyBuf == '0' - 4 ) /* negative number */
            {
               char *ptr = pKeyBuf;

               while( iLen-- )
               {
                  if( *ptr != '.' )
                     *ptr = '0' - ( *ptr - '0' + 4 );
                  ++ptr;
               }
               *ptr = '\0';
               pKeyBuf[ 0 ] = '-';
            }
            if( hb_valStrnToNum( pKeyBuf, iKeyLen, &lValue, &dValue, &iDec, &iLen ) )
               hb_itemPutNDLen( pItem, dValue, iLen, iDec );
            else
               hb_itemPutNIntLen( pItem, lValue, iKeyLen );
         }
         else     /* ADS_CDX, ADS_ADT */
         {
            HB_ORD2DBL( pKeyBuf, &dValue );
            hb_itemPutND( pItem, dValue );
         }
         break;

      case ADS_DATE:
         if( pArea->iFileType == ADS_NTX )
         {
            hb_itemPutDS( pItem, pKeyBuf );
         }
         else /* ADS_CDX, ADS_ADT, ADS_VFP */
         {
            HB_ORD2DBL( pKeyBuf, &dValue );
            hb_itemPutDL( pItem, ( LONG ) dValue );
         }
         break;

      case ADS_LOGICAL:
         hb_itemPutL( pItem, *pKeyBuf == 'T' );
         break;

      default:
         hb_itemClear( pItem );

   }
}

static void adsScopeGet( ADSAREAP pArea, ADSHANDLE hOrder, USHORT nScope, PHB_ITEM pItem )
{
   UNSIGNED8  pucScope[ ADS_MAX_KEY_LENGTH + 1 ];
   UNSIGNED16 u16Len = ADS_MAX_KEY_LENGTH;
   UNSIGNED32 u32RetVal;
   UNSIGNED16 u16KeyType = 0;

   HB_TRACE(HB_TR_DEBUG, ("adsScopeGet(%p, %lu, %hu, %p)", pArea, hOrder, nScope, pItem));

   if( hOrder )
   {
      /*ADS top/bottom are 1,2 instead of 0,1*/
      nScope = ( nScope == 0 ) ? ADS_TOP : ADS_BOTTOM;

      u32RetVal = AdsGetScope( hOrder, ( UNSIGNED16 ) nScope, pucScope, &u16Len );

      if( u32RetVal == AE_SUCCESS )
      {
         AdsGetKeyType( hOrder, &u16KeyType );
         adsGetKeyItem( pArea, pItem, u16KeyType, ( char * ) pucScope, u16Len );
      }
      else
         hb_itemClear( pItem );
   }
}

static HB_ERRCODE adsScopeSet( ADSAREAP pArea, ADSHANDLE hOrder, USHORT nScope, PHB_ITEM pItem )
{
   HB_TRACE(HB_TR_DEBUG, ("adsScopeSet(%p, %lu, %hu, %p)", pArea, hOrder, nScope, pItem));

   HB_SYMBOL_UNUSED( pArea );

   if( hOrder )
   {
      nScope = ( nScope == 0 ) ? ADS_TOP : ADS_BOTTOM;
      if( pItem )
      {
         UNSIGNED16 u16KeyType = 0;
         AdsGetKeyType( hOrder, &u16KeyType );

         /* make sure passed item has same type as index */
         switch( u16KeyType )
         {
            case ADS_RAW:               /* adt files need the ";" concatenation operator (instead of "+") to be optimized */
               /* ADS timestamp values */
               if( HB_IS_DATETIME( pItem ) )
               {
                  if( pArea->iFileType == ADS_ADT )
                  {
                     UNSIGNED8 pKeyBuf[ 8 ];
                     long lDate, lTime;
                     hb_itemGetDTL( pItem, &lDate, &lTime );
                     /* ADS stores milliseconds in raw ADT form increased by one */
                     ++lTime;
                     HB_PUT_BE_UINT32( pKeyBuf, lDate );
                     HB_PUT_BE_UINT32( &pKeyBuf[ 4 ], lTime );
                     AdsSetScope( hOrder, nScope, pKeyBuf, HB_IS_DATETIME( pItem ) ? 8 : 4, ADS_RAWKEY );
                     break;
                  }
#if ADS_LIB_VERSION >= 900
                  else if( pArea->iFileType == ADS_VFP )
                  {
                     double dTemp;
                     dTemp = hb_itemGetDTD( pItem );
                     AdsSetScope( hOrder, nScope,
                                  ( UNSIGNED8 * ) &dTemp,
                                  ( UNSIGNED16 ) sizeof( dTemp ), ADS_DOUBLEKEY );
                     break;
                  }
#endif
               }
            case ADS_STRING:
               if( HB_IS_STRING( pItem ) )
               {
                  UNSIGNED16 u16DataType = ADS_STRINGKEY ;
                  UNSIGNED16 ucLen = ( UNSIGNED16 ) hb_itemGetCLen( pItem );
                  UNSIGNED8 *pucScope = ( UNSIGNED8 * ) hb_itemGetCPtr( pItem );
#if defined( ADS_USE_OEM_TRANSLATION ) && ADS_LIB_VERSION < 600
                  UNSIGNED8 * pszKeyFree = NULL;
#endif
#ifdef ADS_USE_OEM_TRANSLATION
                  if( hb_ads_bOEM )
                  {
#if ADS_LIB_VERSION >= 600
                     u16DataType = ADS_RAWKEY;
#else
                     pucScope = pszKeyFree = ( UNSIGNED8 * ) hb_adsOemToAnsi( ( char * ) pucScope, ucLen );
#endif
                  }
#endif
                  AdsSetScope( hOrder, nScope, pucScope, ucLen, u16DataType );
#if defined( ADS_USE_OEM_TRANSLATION ) && ADS_LIB_VERSION < 600
                  if( pszKeyFree )
                     hb_adsOemAnsiFree( ( char * ) pszKeyFree );
#endif
               }
               break;

            case ADS_NUMERIC:
               if( HB_IS_NUMERIC( pItem ) )
               {
                  double dTemp;
                  dTemp = hb_itemGetND( pItem );
                  AdsSetScope( hOrder, nScope,
                               ( UNSIGNED8 * ) &dTemp,
                               ( UNSIGNED16 ) sizeof( dTemp ), ADS_DOUBLEKEY );
               }
               break;

            case ADS_DATE:
               if( HB_IS_DATE( pItem ) )
               {
                  double dTemp;
                  dTemp = hb_itemGetDL( pItem );
                  AdsSetScope( hOrder, nScope,
                               ( UNSIGNED8 * ) &dTemp,
                               ( UNSIGNED16 ) sizeof( dTemp ), ADS_DOUBLEKEY );
               }
               break;

            case ADS_LOGICAL:
               AdsSetScope( hOrder, nScope,
                            ( UNSIGNED8 * ) ( hb_itemGetL( pItem ) ? "T" : "F" ),
                            1, ADS_STRINGKEY );
               break;
         }
      }
      else
         AdsClearScope( hOrder, nScope );

      return HB_SUCCESS;
   }
   else
   {
      return HB_FAILURE;
   }
}

static double adsGetRelPos( ADSAREAP pArea, ADSHANDLE hOrder )
{
   ULONG ulRecNo, ulRecCount;

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( !pArea->fPositioned )
   {
      SELF_RECNO( ( AREAP ) pArea, &ulRecNo );
      return ulRecNo > 1 ? 1.0 : 0.0;
   }
   else if( hOrder )
   {
      DOUBLE dPos = 0.0;
      AdsGetRelKeyPos( hOrder, &dPos );
      return dPos;
   }
   else
   {
      SELF_RECNO( ( AREAP ) pArea, &ulRecNo );
      SELF_RECCOUNT( ( AREAP ) pArea, &ulRecCount );
      if( ulRecNo == 0 || ulRecCount == 0 )
      {
         return 0.0;
      }
      else
      {
         /* ADS counts relative record position in this way */
         return ( 0.5 + ulRecNo ) / ulRecCount;
      }
   }
}

static void adsSetRelPos( ADSAREAP pArea, ADSHANDLE hOrder, double dPos )
{
   ADSHANDLE hCurrOrder = pArea->hOrdCurrent;

   pArea->hOrdCurrent = hOrder;
   if( dPos >= 1.0 )
      SELF_GOBOTTOM( ( AREAP ) pArea );
   else if( dPos <= 0.0 )
      SELF_GOTOP( ( AREAP ) pArea );
   else if( hOrder )
   {
      /* reset any pending relations */
      SELF_RESETREL( pArea );

      AdsSetRelKeyPos( hOrder, dPos );
      hb_adsUpdateAreaFlags( pArea );
      /* Force relational movement in child WorkAreas */
      if( pArea->area.lpdbRelations )
         SELF_SYNCCHILDREN( ( AREAP ) pArea );

      SELF_SKIPFILTER( ( AREAP ) pArea, 1 );
      if( pArea->area.fEof )
         SELF_GOTOP( ( AREAP ) pArea );
   }
   else
   {
      ULONG ulRecCount, ulRecNo;

      SELF_RECCOUNT( ( AREAP ) pArea, &ulRecCount );
      ulRecNo = ( ULONG ) dPos * ulRecCount + 1;
      if( ulRecNo >= ulRecCount )
         ulRecNo = ulRecCount;
      SELF_GOTO( ( AREAP ) pArea, ulRecNo );

      SELF_SKIPFILTER( ( AREAP ) pArea, 1 );
      if( pArea->area.fEof )
         SELF_GOTOP( ( AREAP ) pArea );
   }
   pArea->hOrdCurrent = hCurrOrder;
}

HB_ERRCODE hb_adsCloseCursor( ADSAREAP pArea )
{
   HB_ERRCODE uiError;

   HB_TRACE(HB_TR_DEBUG, ("hb_adsCloseCursor(%p)", pArea));

   pArea->hOrdCurrent = 0;
   if( pArea->hTable )
   {
      UNSIGNED32 u32RetVal = AdsCloseTable( pArea->hTable );

      if( u32RetVal != AE_SUCCESS )
      {
         HB_TRACE(HB_TR_DEBUG, ("adsCloseTable(%lu, %s) failed", ( ULONG ) u32RetVal, pArea->szDataFileName));
      }
      pArea->hTable = 0;
   }
   if( pArea->hStatement )
   {
      AdsCloseSQLStatement( pArea->hStatement );
      pArea->hStatement = 0;
   }

   uiError = SUPER_CLOSE( ( AREAP ) pArea );

   /* Free buffer */
   if( pArea->pRecord )
   {
      hb_xfree( pArea->pRecord );
      pArea->pRecord = NULL;
   }

   /* Free all filenames */
   if( pArea->szDataFileName )
   {
      hb_xfree( pArea->szDataFileName );
      pArea->szDataFileName = NULL;
   }
   return uiError;
}




/*
 * -- ADS METHODS --
 */

static HB_ERRCODE adsBof( ADSAREAP pArea, BOOL * pBof )
{
   HB_TRACE(HB_TR_DEBUG, ("adsBof(%p, %p)", pArea, pBof));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   *pBof = pArea->area.fBof;

   return HB_SUCCESS;
}

static HB_ERRCODE adsEof( ADSAREAP pArea, BOOL * pEof )
{
   HB_TRACE(HB_TR_DEBUG, ("adsEof(%p, %p)", pArea, pEof));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   *pEof = pArea->area.fEof;

   return HB_SUCCESS;
}

static HB_ERRCODE adsFound( ADSAREAP pArea, BOOL * pFound )
{
   HB_TRACE(HB_TR_DEBUG, ("adsFound(%p, %p)", pArea, pFound));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   *pFound = pArea->area.fFound;

   return HB_SUCCESS;
}

static HB_ERRCODE adsGoBottom( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsGoBottom(%p)", pArea));

   /* reset any pending relations */
   SELF_RESETREL( pArea );

   pArea->area.fTop = FALSE;
   pArea->area.fBottom = TRUE;
   AdsGotoBottom( (pArea->hOrdCurrent) ? pArea->hOrdCurrent : pArea->hTable );

   hb_adsUpdateAreaFlags( pArea );

   /* Force relational movement in child WorkAreas */
   if( pArea->area.lpdbRelations )
      SELF_SYNCCHILDREN( ( AREAP ) pArea );

   return SELF_SKIPFILTER( ( AREAP ) pArea, -1 );
}

static HB_ERRCODE adsGoTo( ADSAREAP pArea, ULONG ulRecNo )
{
/*
 * TODO: bh: Note  Explicitly moving to a deleted record when using the
 * Advantage proprietary table format (ADT) is an illegal operation and
 * will return the error 5022 (AE_INVALID_RECORD_NUMBER), invalid record
 * number.
 *
 * Sut 24 Sep 2005 16:24:18 CEST, Druzus
 * IMHO such GOTO operation should reposition our RDD to phantom record
 */
   UNSIGNED32 ulRetVal, u32RecNo;
   ULONG ulRecCount;

   HB_TRACE(HB_TR_DEBUG, ("adsGoTo(%p, %lu)", pArea, ulRecNo));

   /* -----------------7/19/2001 3:04PM-----------------
     The following call is a necessary workaround for ace32.dll
     prior to 6.1.  There were bugs where
     AdsGotoRecord() can FAIL to move the record pointer
     after some sequences of setting/clearing relations.
     A call to AdsGetRecordNum() before it clears the problem.  -BH
    --------------------------------------------------*/
    /*
     * Thu 25 Aug 2005 11:56:20 CEST, Druzus
     * This trick force to resolving pending relations. It means
     * that this ADS clients does not reset pending relation in
     * AdsGotoRecord() and it is resolved later. Similar situation
     * can appear also in APPEND() where I can see call to AdsAtEOF()
     * which will have exactly the same side effect as AdsGetRecordNum()
     * - both reset pending relations by resolving them
     */

   /* reset our pending relations */
   SELF_RESETREL( pArea );

   /* force to reset ACE pending relations by resolving them */
   AdsGetRecordNum( pArea->hTable, ADS_IGNOREFILTERS, &u32RecNo );

   /*
    * always make GOTO - it's necessary to sync children inside ACE
    * internals
    */
   ulRetVal = AdsGotoRecord( pArea->hTable, ( UNSIGNED32 ) ulRecNo );

   if( ulRetVal == AE_INVALID_RECORD_NUMBER )
   {
      /*
       * We are going to ADT deleted record - see above. GO to phantom
       * record in such case
       */
      ulRecNo = 0;
      ulRetVal = AdsGotoRecord( pArea->hTable, 0 );
   }

   /* update area flag */
   hb_adsUpdateAreaFlags( pArea );
   pArea->ulRecNo = ulRecNo;

   if( !pArea->fPositioned )
   {
      /* set our record number value */
      SELF_RECCOUNT( ( AREAP ) pArea, &ulRecCount );
      /* eliminate posible race condition in this operation */
      if( ulRecNo != 0 && ulRecNo <= ulRecCount )
         pArea->ulRecNo = ulRecNo;
      else
         pArea->ulRecNo = ulRecCount + 1;
   }
   else if( ulRecNo == u32RecNo )
   {
      /* is it really necessary? doesn't AdsGotoRecord() refresh buffers? */
      AdsRefreshRecord( pArea->hTable );
   }

   /* Force relational movement in child WorkAreas */
   if( pArea->area.lpdbRelations )
      SELF_SYNCCHILDREN( ( AREAP ) pArea );

   return ( ulRetVal == AE_SUCCESS ? HB_SUCCESS : HB_FAILURE );
}

static HB_ERRCODE adsGoToId( ADSAREAP pArea, PHB_ITEM pItem )
{
   ULONG ulRecNo;

   HB_TRACE(HB_TR_DEBUG, ("adsGoToId(%p, %p)", pArea, pItem));

   if( HB_IS_NUMERIC( pItem ) )
   {
      ulRecNo = hb_itemGetNL( pItem );
      return SELF_GOTO( ( AREAP ) pArea, ulRecNo );
   }
   else
   {
      commonError( pArea, EG_DATATYPE, 1020, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }
}

static HB_ERRCODE adsGoTop( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsGoTop(%p)", pArea));

   /* reset any pending relations */
   SELF_RESETREL( pArea );

   pArea->area.fTop = TRUE;
   pArea->area.fBottom = FALSE;
   AdsGotoTop( (pArea->hOrdCurrent) ? pArea->hOrdCurrent : pArea->hTable );

   hb_adsUpdateAreaFlags( pArea );

   /* Force relational movement in child WorkAreas */
   if( pArea->area.lpdbRelations )
      SELF_SYNCCHILDREN( ( AREAP ) pArea );

   return SELF_SKIPFILTER( ( AREAP ) pArea, 1 );
}

static HB_ERRCODE adsSeek( ADSAREAP pArea, BOOL bSoftSeek, PHB_ITEM pKey, BOOL bFindLast )
{
   UNSIGNED32 u32RecNo = 0, u32NewRec;
   UNSIGNED16 u16SeekType = ( bSoftSeek ) ? ADS_SOFTSEEK : ADS_HARDSEEK,
              u16KeyType, u16Found, u16KeyLen;
   UNSIGNED8 * pszKey;
   double dValue;
   UNSIGNED8  *pucSavedKey = NULL;
   UNSIGNED16 u16SavedKeyLen = ADS_MAX_KEY_LENGTH;  /* this may be longer than the actual seek expression, so we don't pass it along */
#if defined( ADS_USE_OEM_TRANSLATION ) && ADS_LIB_VERSION < 600
   UNSIGNED8 * pszKeyFree = NULL;
#endif

   HB_TRACE(HB_TR_DEBUG, ("adsSeek(%p, %d, %p, %d)", pArea, bSoftSeek, pKey, bFindLast));

   if( ! pArea->hOrdCurrent )
   {
      commonError( pArea, EG_NOORDER, 1201, 0, NULL, EF_CANDEFAULT, NULL );
      return HB_FAILURE;
   }

   /* build a seek key */
   if( HB_IS_STRING( pKey ) )
   {
      pszKey = ( UNSIGNED8* ) hb_itemGetCPtr( pKey );
      u16KeyLen = ( UNSIGNED16 ) hb_itemGetCLen( pKey );
#ifdef ADS_USE_OEM_TRANSLATION
#if ADS_LIB_VERSION >= 600
      u16KeyType = hb_ads_bOEM ? ADS_RAWKEY : ADS_STRINGKEY;
#else
      u16KeyType = ADS_STRINGKEY;
      if( hb_ads_bOEM )
      {
         pszKey = pszKeyFree = ( UNSIGNED8 * ) hb_adsOemToAnsi( ( char * ) pszKey, u16KeyLen );
      }
#endif
#else
      u16KeyType = ADS_STRINGKEY;
#endif
   }
   else if( HB_IS_DATE( pKey ) )
   {
      dValue = ( double ) hb_itemGetDL( pKey );
      pszKey = ( UNSIGNED8* ) &dValue;
      u16KeyLen = ( UNSIGNED16 ) sizeof( double );
      u16KeyType = ADS_DOUBLEKEY;
   }
   else if( HB_IS_NUMERIC( pKey ) )
   {
      dValue = hb_itemGetND( pKey );
      pszKey = ( UNSIGNED8 * ) &dValue;
      u16KeyLen = ( UNSIGNED16 ) sizeof( double );
      u16KeyType = ADS_DOUBLEKEY;
   }
   else if( HB_IS_LOGICAL( pKey ) )
   {
      pszKey = ( UNSIGNED8 * ) ( hb_itemGetL( pKey ) ? "1" : "0" );
      u16KeyLen = 1;
      u16KeyType = ADS_STRINGKEY;
   }
   else
   {
      commonError( pArea, EG_DATATYPE, 1020, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }

   /* reset any pending relations - I hope ACE make the same and the problem
      reported in GOTO() does not exist here */
   SELF_RESETREL( pArea );

   /*
    * This flag should be cleaned inside seek
    * They could be used by SKIP_FILTER and gives differ result
    * when seek is called just after GOTOP or GOBOTTOM, Druzus.
    */
   pArea->area.fTop = pArea->area.fBottom = FALSE;

   if( bFindLast )
   {
      AdsSeekLast( pArea->hOrdCurrent,
                   pszKey, u16KeyLen, u16KeyType, &u16Found );
      if( bSoftSeek && ! u16Found )
      {
         UNSIGNED16 u16Eof;

         /* in such case ADS set record at EOF position so we
            should make normal soft seek and then skip -1 to emulate
            Clipper behavior, Druzus */
         AdsSeek( pArea->hOrdCurrent, pszKey, u16KeyLen,
                  u16KeyType, u16SeekType, &u16Found );

         AdsAtEOF( pArea->hTable, &u16Eof );
         if( !u16Eof )
            AdsSkip( pArea->hOrdCurrent, -1 );
      }
   }
   else
      AdsSeek( pArea->hOrdCurrent, pszKey, u16KeyLen,
               u16KeyType, u16SeekType, &u16Found );

   hb_adsUpdateAreaFlags( pArea );

   /* check if we are at ADS BOF phantom record. It's possible only after
    * AdsSkip( -1 ) above */
   if( pArea->area.fBof && !pArea->area.fEof )
   {
      HB_ERRCODE errCode = SELF_GOTO( ( AREAP ) pArea, 0 );
      /* HB_ERRCODE errCode = SELF_GOTOP( ( AREAP ) pArea ); */
      pArea->area.fBof = FALSE;
#if defined( ADS_USE_OEM_TRANSLATION ) && ADS_LIB_VERSION < 600
      if( pszKeyFree )
         hb_adsOemAnsiFree( ( char * ) pszKeyFree );
#endif
      return errCode;
   }

   /* Force relational movement in child WorkAreas */
   if( pArea->area.lpdbRelations )
      SELF_SYNCCHILDREN( ( AREAP ) pArea );

   /* ----------------- BH ------------------
      If a filter is set that is not valid for ADS, we need to skip
      off of any invalid records (IOW, filter at the Harbour level if ADS can't
      because the filter has UDFs or PUBLICVAR references).
      To make sure the skipped-to record still matches the seeked key, we need to
      be able to construct a comparable key for the subsequent record.
      This is annoyingly complex with the various ads key types for various table types.
      AdsExtractKey would seem to be the api of choice, but here on the starting end the
      key we seek on does NOT match the format of what we get back from AdsExtractKey.
      So I'm saving off the first found record's key, and passing that to our
      adsIndexKeyCmp() to compare to the new record's key.
      We're relying on testing to verify that partial key searches and binary
      raw keys all end up working right.
    --------------------------------------------------*/
   if( pArea->area.dbfi.itmCobExpr && !pArea->area.dbfi.fOptimized && !pArea->area.fEof )
   {
      /* Remember FOUND flag for updating after SKIPFILTER() */
      u16Found = ( UNSIGNED16 ) pArea->area.fFound;

      if( u16Found && u16KeyLen > 0 )
      {
         /*
          * remember the record number for faster checking if we should update
          * fFound after SKIPFILTER. Also get its extracted key to simplify
          * that comparison
          */
         UNSIGNED32 u32RetVal;
         pucSavedKey = ( UNSIGNED8 * ) hb_xgrab( ADS_MAX_KEY_LENGTH + 1 );

         AdsGetRecordNum( pArea->hTable, ADS_IGNOREFILTERS, &u32RecNo );
         u32RetVal = AdsExtractKey( pArea->hOrdCurrent, pucSavedKey, &u16SavedKeyLen );

         if( u32RetVal != AE_SUCCESS )
         {
            u16SavedKeyLen = 0;
         }
         else if( u16SavedKeyLen > u16KeyLen )
         {
            /* Initial found key from index is longer than Seek key:
               Did a partial search */
            if( AdsGetKeyType( pArea->hOrdCurrent, &u16KeyType ) == AE_SUCCESS &&
                ( u16KeyType == ADS_STRING || u16KeyType == ADS_RAW ) )
            {
               /*
                * do partial comparison below on String and Raw indexes only.
                * Note that we can search a different type index with a string
                * expression, but ads does internal conversions and the length
                * of our string may be drastically different than the real key
                */
               u16SavedKeyLen = u16KeyLen;
            }
         }
      }

      /*
       * TODO: Possible optimization: if !softseek, skipfilter should abort
       * skipping once keys no longer match.
       * Perhaps use temp replacement of scope for this -- but remember ads
       * does scopes on client and if last good scoped record fails the filter,
       * the server will skip to the end anyway
       */
      if( SELF_SKIPFILTER( ( AREAP ) pArea, bFindLast ? -1 : 1 ) != HB_SUCCESS )
      {
         if( pucSavedKey )
            hb_xfree( pucSavedKey );
#if defined( ADS_USE_OEM_TRANSLATION ) && ADS_LIB_VERSION < 600
         if( pszKeyFree )
            hb_adsOemAnsiFree( ( char * ) pszKeyFree );
#endif
         return HB_FAILURE;
      }

      if( u16Found )
      {
         if( pArea->area.fEof )
         {
            u16Found = FALSE;
         }
         /* seek empty string is synonymous with GoTop */
         else if( u16KeyLen > 0 )
         {
            AdsGetRecordNum( pArea->hTable, ADS_IGNOREFILTERS, &u32NewRec );
            /* SkipFilter moved us?  see if index key is still a match. */
            if( u32RecNo != u32NewRec )
            {
               if( u16SavedKeyLen == 0 )
                  u16Found = FALSE;
               else
                  u16Found = ( UNSIGNED16 ) adsIndexKeyCmp( pArea->hOrdCurrent, pucSavedKey, u16SavedKeyLen );
            }
         }
      }
      pArea->area.fFound = u16Found != 0;
   }

   /*
    * Clipper clears BOF after seek - I found only one strange situation
    * when it doesn't but only sometimes. I've not been able to detect
    * the rule yet. Any how it's much safer to clear it, Druzus.
    */
   pArea->area.fBof = FALSE;

   if( pucSavedKey )
      hb_xfree( pucSavedKey );
#if defined( ADS_USE_OEM_TRANSLATION ) && ADS_LIB_VERSION < 600
   if( pszKeyFree )
      hb_adsOemAnsiFree( ( char * ) pszKeyFree );
#endif

   return HB_SUCCESS;
}

static HB_ERRCODE adsSkip( ADSAREAP pArea, LONG lToSkip )
{
   HB_TRACE(HB_TR_DEBUG, ("adsSkip(%p, %ld)", pArea, lToSkip));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

/* ----------------- Brian Hays ------------------

   In ADS, if you GO 0 (as opposed to skipping past lastrec),
   it considers the record pointer "unpositioned".
   If you then try to skip -1 you end up at Top with BOF True.
   (If you skip past lastrec, then skip -1 it works right.)
   To fix this we need to trap for a (negative lToSkip .AND. EOF)
   and do a GoBottom--but only after letting ads try first and
   testing for BOF.  We need to avoid our GoBottom hack as much as
   possible because with a filter set it could be quite slow.

   In addition, if lToSkip > 1 we need to iterate calls to AdsSkip(1) and
   test the filter each time since, even if the server has a filter set,
   AdsSkip(5) may only test the filter after 5 raw skips (according to the
   Extended Systems developers.)

 --------------------------------------------------*/


   if( lToSkip == 0 )
   {
      /* Clipper does not update record at EOF position in SKIP(0) */
      if( pArea->fPositioned )
      {
         BOOL fBof, fEof;

         /* Save flags */
         fBof = pArea->area.fBof;
         fEof = pArea->area.fEof;

         AdsSkip( (pArea->hOrdCurrent) ? pArea->hOrdCurrent : pArea->hTable, 0 );
         /* TODO: is this really necessary, doesn't AdsSkip(0) do that? */
         AdsRefreshRecord( pArea->hTable );
         hb_adsUpdateAreaFlags( pArea );

         /* Restore flags */
         pArea->area.fBof = fBof;
         pArea->area.fEof = fEof;

         /* Force relational movement in child WorkAreas */
         if( pArea->area.lpdbRelations )
            SELF_SYNCCHILDREN( ( AREAP ) pArea );
      }
      return HB_SUCCESS;
   }
   else
   {
      HB_ERRCODE errCode = HB_SUCCESS;
      LONG lSkipper;

      if( !pArea->fPositioned && lToSkip < 0 )
      {
         errCode = SELF_GOBOTTOM( ( AREAP ) pArea );
         ++lToSkip;
      }

      if( !pArea->fPositioned )
      {
         if( lToSkip > 0 )
            pArea->area.fEof = TRUE;
         else
            pArea->area.fBof = TRUE;
      }
      else if( lToSkip )
      {
         pArea->area.fTop = pArea->area.fBottom = FALSE;

         /*
          * This causes that skip is done on the server side but it
          * may cause bad side effect - in multiple skip ADS respects
          * only index keys and bitmap filter so it may skip differ
          * number of records than we asked, [druzus]
          */
         if( pArea->area.dbfi.itmCobExpr == NULL || pArea->area.dbfi.fOptimized )
            lSkipper = lToSkip;
         else
            lSkipper = lToSkip < 0 ? -1 : 1;

         if( lToSkip > 0 )
         {
            while( errCode == HB_SUCCESS && !pArea->area.fEof && lToSkip )
            {
               AdsSkip( ( pArea->hOrdCurrent ) ? pArea->hOrdCurrent : pArea->hTable, lSkipper );
               hb_adsUpdateAreaFlags( pArea );
               /* Force relational movement in child WorkAreas */
               if( pArea->area.lpdbRelations )
                  SELF_SYNCCHILDREN( ( AREAP ) pArea );
               errCode = SELF_SKIPFILTER( ( AREAP ) pArea, lSkipper );
               lToSkip -= lSkipper;
            }
            pArea->area.fBof = FALSE;
         }
         else
         {
            while( errCode == HB_SUCCESS && !pArea->area.fBof && lToSkip )
            {
               AdsSkip( (pArea->hOrdCurrent) ? pArea->hOrdCurrent : pArea->hTable, lSkipper );
               hb_adsUpdateAreaFlags( pArea );
               /* Force relational movement in child WorkAreas */
               if( pArea->area.lpdbRelations )
                  SELF_SYNCCHILDREN( ( AREAP ) pArea );
               errCode = SELF_SKIPFILTER( ( AREAP ) pArea, lSkipper );
               lToSkip -= lSkipper;
            }
            pArea->area.fEof = FALSE;
         }
      }

      return errCode;
   }
}

static HB_ERRCODE adsSkipFilter( ADSAREAP pArea, LONG lUpDown )
{
   BOOL fBottom;
   HB_ERRCODE uiError;

   HB_TRACE(HB_TR_DEBUG, ("adsSkipFilter(%p, %ld)", pArea, lUpDown));

   lUpDown = ( lUpDown < 0  ? -1 : 1 );

   /* remember if we are here after SLEF_GOTOP() */
   fBottom = pArea->area.fBottom;

   /*
    * because ADS respect DELETED flag and SET_DELETED is synchronized
    * with ADS we only have to check for filter expressions which cannot
    * be optimized by ADS server, Druzus.
    */
   if( pArea->area.dbfi.itmCobExpr != NULL && !pArea->area.dbfi.fOptimized )
   {
      while( !pArea->area.fBof && !pArea->area.fEof )
      {
         PHB_ITEM pResult = hb_vmEvalBlock( pArea->area.dbfi.itmCobExpr );
         if( ! HB_IS_LOGICAL( pResult ) || hb_itemGetL( pResult ) )
            break;

         AdsSkip( ( pArea->hOrdCurrent ) ? pArea->hOrdCurrent : pArea->hTable, lUpDown );
         hb_adsUpdateAreaFlags( pArea );

         /* Force relational movement in child WorkAreas, the child fields
            can be a part of evaluated block so ti's necessary, here */
         if( pArea->area.lpdbRelations )
            SELF_SYNCCHILDREN( ( AREAP ) pArea );
      }
   }

   /*
    * The only one situation when we should repos is backward skipping
    * if we are at BOTTOM position (it's SKIPFILTER called from GOBOTTOM)
    * then GOEOF() if not then GOTOP()
    */
   if( pArea->area.fBof && lUpDown < 0 )
   {
      if( fBottom )
      {
         /* we were passing from bottom to this place (BOF) and we do not
            find any valid record so just simply make GOEOF() */
         uiError = SELF_GOTO( ( AREAP ) pArea, 0 );
      }
      else
      {
         uiError = SELF_GOTOP( ( AREAP ) pArea );
         pArea->area.fBof = TRUE;
      }
   }
   else if( !pArea->fPositioned )
   {
      ULONG ulRecCount;
      /* set our record number value */
      uiError = SELF_RECCOUNT( ( AREAP ) pArea, &ulRecCount );
      pArea->ulRecNo = ulRecCount + 1;
   }
   else
      uiError = HB_SUCCESS;

   return uiError;
}

#define  adsSkipRaw               NULL
#define  adsAddField              NULL

static HB_ERRCODE adsAppend( ADSAREAP pArea, BOOL fUnLockAll )
{
   UNSIGNED32 u32RetVal;

   HB_TRACE(HB_TR_DEBUG, ("adsAppend(%p, %d)", pArea, (int) fUnLockAll));

   /* reset any pending relations */
   SELF_RESETREL( pArea );

   /* ADS does not do the same so force to reset its relations by resolving them */
   if( pArea->area.uiParents )
   {
      UNSIGNED16 u16Eof;
      AdsAtEOF( pArea->hTable, &u16Eof );
   }

   if( fUnLockAll && pArea->fShared && !pArea->fFLocked )
      SELF_RAWLOCK( ( AREAP ) pArea, FILE_UNLOCK, 0 );

   u32RetVal = AdsAppendRecord( pArea->hTable );
   if( u32RetVal == AE_SUCCESS )
   {
      if( pArea->fShared && !pArea->fFLocked )
      {
         ULONG ulRecNo;

         if( SELF_RECNO( ( AREAP ) pArea, &ulRecNo ) == HB_SUCCESS )
         {
            /* to avoid unnecessary record refreshing after locking */
            pArea->fPositioned = TRUE;
            SELF_RAWLOCK( ( AREAP ) pArea, REC_LOCK, ulRecNo );
         }
      }
      pArea->area.fBof = FALSE;
      pArea->area.fEof = FALSE;
      pArea->area.fFound = FALSE;
      pArea->fPositioned = TRUE;
      return HB_SUCCESS;
   }
   else if( u32RetVal == AE_TABLE_READONLY )
   {
      commonError( pArea, EG_READONLY, EDBF_READONLY, 0, NULL, 0, NULL );
   }
   else
   {
      /* 1001 and 7008 are standard ADS Open Errors that will usually be sharing issues */
      USHORT uiOsCOde = u32RetVal == 1001 || u32RetVal == 7008 ? 32 : 0;
      commonError( pArea, EG_APPENDLOCK, ( USHORT ) u32RetVal, uiOsCOde, NULL, EF_CANDEFAULT, NULL );
   }

   return HB_FAILURE;
}

static HB_ERRCODE adsCreateFields( ADSAREAP pArea, PHB_ITEM pStruct )
{
   USHORT uiItems, uiCount, uiLen, uiDec;
   DBFIELDINFO dbFieldInfo;
   PHB_ITEM pFieldDesc;
   const char *szFieldType;
   int iData, iNameLen;

   HB_TRACE(HB_TR_DEBUG, ("adsCreateFields(%p, %p)", pArea, pStruct));

   uiItems = ( USHORT ) hb_arrayLen( pStruct );
   SELF_SETFIELDEXTENT( ( AREAP ) pArea, uiItems );

   memset( &dbFieldInfo, 0, sizeof( dbFieldInfo ) );

   for( uiCount = 0; uiCount < uiItems; uiCount++ )
   {
      dbFieldInfo.uiTypeExtended = 0;
      dbFieldInfo.uiFlags = 0;
      pFieldDesc = hb_arrayGetItemPtr( pStruct, uiCount + 1 );
      dbFieldInfo.atomName = hb_arrayGetCPtr( pFieldDesc, 1 );
      iData = hb_arrayGetNI( pFieldDesc, 3 );
      if( iData < 0 )
         iData = 0;
      uiLen = dbFieldInfo.uiLen = ( USHORT ) iData;
      iData = hb_arrayGetNI( pFieldDesc, 4 );
      if( iData < 0 )
         iData = 0;
      uiDec = ( USHORT ) iData;
      dbFieldInfo.uiDec = 0;
      szFieldType = hb_arrayGetCPtr( pFieldDesc, 2 );
      iNameLen = ( int ) strlen( szFieldType );
      iData = HB_TOUPPER( szFieldType[ 0 ] );
      switch( iData )
      {
         case 'C':
            if( iNameLen == 1 || ! hb_stricmp( szFieldType, "char" ) )
            {
               dbFieldInfo.uiType = HB_FT_STRING;
               dbFieldInfo.uiLen = uiLen + uiDec * 256;
            }
            else if( pArea->iFileType == ADS_ADT && iNameLen >= 4 &&
                     hb_strnicmp( szFieldType, "curdouble", iNameLen ) == 0 )
            {
               dbFieldInfo.uiType = HB_FT_CURDOUBLE;
               dbFieldInfo.uiTypeExtended = ADS_CURDOUBLE;
               dbFieldInfo.uiLen = 8;
               dbFieldInfo.uiDec = uiDec;
            }
            else
               return HB_FAILURE;
            break;

         case 'L':
            dbFieldInfo.uiType = HB_FT_LOGICAL;
            dbFieldInfo.uiLen = 1;
            break;

         case 'M':
            if( iNameLen == 1 || ! hb_stricmp( szFieldType, "memo" ) )
            {
               dbFieldInfo.uiType = HB_FT_MEMO;
               dbFieldInfo.uiLen = ( pArea->iFileType == ADS_ADT ) ? 9 : 10;
            }
            else if( pArea->iFileType == ADS_ADT && iNameLen >= 4 &&
                     hb_strnicmp( szFieldType, "modtime", iNameLen ) == 0 )
            {
               dbFieldInfo.uiType = HB_FT_MODTIME;
               dbFieldInfo.uiTypeExtended = ADS_MODTIME;
               dbFieldInfo.uiLen = 8;
            }
            break;

         case 'D':
            if( iNameLen == 1 || hb_stricmp( szFieldType, "date" ) == 0 )
            {
               dbFieldInfo.uiType = HB_FT_DATE;
               dbFieldInfo.uiLen = ( pArea->iFileType == ADS_ADT || uiLen == 4 ) ? 4 : 8;
            }
            else if( iNameLen >= 4 && hb_strnicmp( szFieldType, "double", iNameLen ) == 0 )
            {
               dbFieldInfo.uiType = HB_FT_DOUBLE;
               dbFieldInfo.uiTypeExtended = ADS_DOUBLE;
               dbFieldInfo.uiLen = 8;
               dbFieldInfo.uiDec = uiDec;
            }
            else
               return HB_FAILURE;
            break;

         case 'N':
         case 'F':
            dbFieldInfo.uiType = HB_FT_LONG;
            dbFieldInfo.uiDec = uiDec;
            if( uiLen > 32 )
               return HB_FAILURE;
            break;

         case 'A':
         case '+':
#if ADS_LIB_VERSION >= 900
            if( pArea->iFileType == ADS_ADT ||
                pArea->iFileType == ADS_VFP )
#else
            if( pArea->iFileType == ADS_ADT )
#endif
            {
               dbFieldInfo.uiType = HB_FT_AUTOINC;
               dbFieldInfo.uiTypeExtended = ADS_AUTOINC;
            }
            else
               return HB_FAILURE;
            break;

         case 'B':
            if( iNameLen == 1 )
            {
               dbFieldInfo.uiType = HB_FT_DOUBLE;
               dbFieldInfo.uiTypeExtended = ADS_DOUBLE;
               dbFieldInfo.uiLen = 8;
               dbFieldInfo.uiDec = uiDec;
            }
            else if( iNameLen >= 4 && hb_strnicmp( szFieldType, "binary", iNameLen ) == 0 )
            {
               dbFieldInfo.uiType = HB_FT_BLOB;
               dbFieldInfo.uiTypeExtended = ADS_BINARY;
#if ADS_LIB_VERSION >= 900
               dbFieldInfo.uiLen = ( pArea->iFileType == ADS_ADT ) ? 9 :
                                   ( pArea->iFileType == ADS_VFP ) ? 4 : 10;
#else
               dbFieldInfo.uiLen = ( pArea->iFileType == ADS_ADT ) ? 9 : 10;
#endif
               dbFieldInfo.uiFlags = HB_FF_BINARY;
            }
            else
               return HB_FAILURE;
            break;

         case 'V':
#if ADS_LIB_VERSION >= 900
            if( pArea->iFileType == ADS_VFP )
            {
               dbFieldInfo.uiType = HB_FT_VARLENGTH;
               dbFieldInfo.uiTypeExtended = ADS_VARCHAR_FOX;
            }
            else
#endif
            {
               dbFieldInfo.uiType = HB_FT_MEMO;
               dbFieldInfo.uiTypeExtended = ADS_VARCHAR;
            }
            break;

#if ADS_LIB_VERSION >= 900
         case 'Q':
            if( pArea->iFileType == ADS_VFP )
            {
               dbFieldInfo.uiType = HB_FT_VARLENGTH;
               dbFieldInfo.uiTypeExtended = ADS_VARBINARY_FOX;
               break;
            }
            else
               return HB_FAILURE;
#endif

         case 'R':
            if( pArea->iFileType == ADS_ADT && iNameLen >= 4 &&
                !hb_strnicmp( szFieldType, "rowversion", iNameLen ) )
            {
               dbFieldInfo.uiType = HB_FT_ROWVER;
               dbFieldInfo.uiTypeExtended = ADS_ROWVERSION;
               dbFieldInfo.uiLen = 8;
            }
            else if( pArea->iFileType == ADS_ADT && iNameLen == 3 &&
                     !hb_stricmp( szFieldType, "raw" ) )
            {
               dbFieldInfo.uiType = HB_FT_STRING;
               dbFieldInfo.uiTypeExtended = ADS_RAW;
               dbFieldInfo.uiFlags = HB_FF_BINARY;
            }
            else
               return HB_FAILURE;
            break;

         case 'S':
            if( !hb_stricmp( szFieldType, "shortdate" ) )
            {
               dbFieldInfo.uiType = HB_FT_DATE;
               dbFieldInfo.uiTypeExtended = ADS_COMPACTDATE;
               dbFieldInfo.uiLen = 4;
            }
            else if( !hb_stricmp( szFieldType, "shortint" ) && pArea->iFileType == ADS_ADT )
            {
               dbFieldInfo.uiType = HB_FT_INTEGER;
               dbFieldInfo.uiTypeExtended = ADS_SHORTINT;
               dbFieldInfo.uiLen = 2;
            }
            else
               return HB_FAILURE;
            break;

         case '@':
#if ADS_LIB_VERSION >= 900
            if( pArea->iFileType == ADS_ADT ||
                pArea->iFileType == ADS_VFP )
#else
            if( pArea->iFileType == ADS_ADT )
#endif
            {
               dbFieldInfo.uiType = HB_FT_TIMESTAMP;
               dbFieldInfo.uiTypeExtended = ADS_TIMESTAMP;
               dbFieldInfo.uiLen = 8;
            }
            else
               return HB_FAILURE;
            break;

         case '=':
            if( pArea->iFileType == ADS_ADT )
            {
               dbFieldInfo.uiType = HB_FT_MODTIME;
               dbFieldInfo.uiTypeExtended = ADS_MODTIME;
               dbFieldInfo.uiLen = 8;
            }
            else
               return HB_FAILURE;
            break;

         case '^':
            if( pArea->iFileType == ADS_ADT )
            {
               dbFieldInfo.uiType = HB_FT_ROWVER;
               dbFieldInfo.uiTypeExtended = ADS_ROWVERSION;
               dbFieldInfo.uiLen = 8;
            }
            else
               return HB_FAILURE;
            break;

         case 'T':
#if ADS_LIB_VERSION >= 900
            if( ( pArea->iFileType == ADS_ADT || pArea->iFileType == ADS_VFP ) &&
#else
            if( pArea->iFileType == ADS_ADT &&
#endif
                ( iNameLen == 1 || ( iNameLen >= 4 &&
                  hb_strnicmp( szFieldType, "timestamp", iNameLen ) == 0 ) ) )
            {
               if( ( iNameLen == 1 && uiLen == 8 ) || iNameLen > 4 )
               {
                  dbFieldInfo.uiType = HB_FT_TIMESTAMP;
                  dbFieldInfo.uiTypeExtended = ADS_TIMESTAMP;
                  dbFieldInfo.uiLen = 8;
               }
               else
               {
                  dbFieldInfo.uiType = HB_FT_TIME;
                  dbFieldInfo.uiTypeExtended = ADS_TIME;
                  dbFieldInfo.uiLen = 4;
               }
            }
            else
               return HB_FAILURE;
            break;

         case 'I':
            if( iNameLen == 1 && ( uiLen == 2 || uiLen == 4 ) )
            {
               dbFieldInfo.uiType = HB_FT_INTEGER;
               dbFieldInfo.uiTypeExtended = uiLen == 2 ? ADS_SHORTINT : ADS_INTEGER;
               dbFieldInfo.uiLen = uiLen;
            }
            else if( !hb_stricmp( szFieldType, "integer" ) )
            {
               dbFieldInfo.uiType = HB_FT_INTEGER;
               dbFieldInfo.uiTypeExtended = ADS_INTEGER;
               dbFieldInfo.uiLen = 4;
            }
            else if( !hb_stricmp( szFieldType, "image" ) )
            {
               dbFieldInfo.uiType = HB_FT_PICTURE;
               dbFieldInfo.uiTypeExtended = ADS_IMAGE;
               dbFieldInfo.uiLen = ( pArea->iFileType == ADS_ADT ) ? 9 : 10;
               dbFieldInfo.uiFlags = HB_FF_BINARY;
            }
            else
               return HB_FAILURE;
            break;

#if ADS_LIB_VERSION >= 900
         case 'Y':
            if( pArea->iFileType == ADS_VFP )
            {
               dbFieldInfo.uiType = HB_FT_CURRENCY;
               dbFieldInfo.uiTypeExtended = ADS_MONEY;
               dbFieldInfo.uiLen = 8;
               dbFieldInfo.uiDec = 4;
            }
            else
               return HB_FAILURE;
            break;
#endif

         case 'P':
            dbFieldInfo.uiType = HB_FT_PICTURE;
            dbFieldInfo.uiTypeExtended = ADS_IMAGE;
            dbFieldInfo.uiLen = ( pArea->iFileType == ADS_ADT ) ? 9 : 10;
            dbFieldInfo.uiFlags = HB_FF_BINARY;
            break;

         default:
            return HB_FAILURE;
      }
      /* Add field */
      if( SELF_ADDFIELD( ( AREAP ) pArea, &dbFieldInfo ) == HB_FAILURE )
      {
         return HB_FAILURE;
      }
   }

   return HB_SUCCESS;
}

static HB_ERRCODE adsDeleteRec( ADSAREAP pArea )
{
   UNSIGNED32 u32RetVal;

   HB_TRACE(HB_TR_DEBUG, ("adsDeleteRec(%p)", pArea));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( !pArea->fPositioned )
      return HB_SUCCESS;

   if( hb_ads_bTestRecLocks )
   {
      if( ! hb_adsCheckLock( pArea ) == HB_SUCCESS )
         return HB_FAILURE;
   }

   u32RetVal = AdsDeleteRecord( pArea->hTable );

   return u32RetVal == AE_SUCCESS ? HB_SUCCESS : HB_FAILURE;
}

static HB_ERRCODE adsDeleted( ADSAREAP pArea, BOOL * pDeleted )
{
   HB_TRACE(HB_TR_DEBUG, ("adsDeleted(%p, %p)", pArea, pDeleted));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( !pArea->fPositioned )
   {
      /* AdsIsRecordDeleted has error AE_NO_CURRENT_RECORD at eof; avoid server call */
      *pDeleted = FALSE;
   }
   else
   {
      UNSIGNED16 u16Deleted = 0;
      UNSIGNED32 u32RetVal;

      u32RetVal = AdsIsRecordDeleted( pArea->hTable, &u16Deleted );
      *pDeleted = u16Deleted != 0;

      return u32RetVal == AE_SUCCESS ? HB_SUCCESS : HB_FAILURE;
   }
   return HB_SUCCESS;
}

static HB_ERRCODE adsFieldCount( ADSAREAP pArea, USHORT * uiFields )
{
   UNSIGNED16 u16Fields;

   HB_TRACE(HB_TR_DEBUG, ("adsFieldCount(%p, %p)", pArea, uiFields));

   AdsGetNumFields( pArea->hTable, &u16Fields );
   *uiFields = u16Fields;

   return HB_SUCCESS;
}

#define  adsFieldDisplay          NULL

static HB_ERRCODE adsFieldInfo( AREAP pArea, USHORT uiIndex, USHORT uiType, PHB_ITEM pItem )
{
   LPFIELD pField;

   HB_TRACE(HB_TR_DEBUG, ("adsFieldInfo(%p, %hu, %hu, %p)", pArea, uiIndex, uiType, pItem));

   if( uiIndex > pArea->uiFieldCount )
      return HB_FAILURE;

   if( uiType != DBS_TYPE )
      return SUPER_FIELDINFO( ( AREAP ) pArea, uiIndex, uiType, pItem );

   pField = pArea->lpFields + uiIndex - 1;
   switch( pField->uiType )
   {
      case HB_FT_STRING:
         if( pField->uiTypeExtended == 0 )
            hb_itemPutC( pItem, "C" );
         else if( pField->uiTypeExtended == ADS_RAW )
            hb_itemPutC( pItem, "RAW" );
         break;

      case HB_FT_VARLENGTH:
#if ADS_LIB_VERSION >= 900
         if( pField->uiTypeExtended == ADS_VARCHAR_FOX )
            hb_itemPutC( pItem, "V" );
         else
#endif
            hb_itemPutC( pItem, "Q" );
         break;

      case HB_FT_BLOB:
         hb_itemPutC( pItem, "BINARY" );
         break;

      case HB_FT_PICTURE:
         hb_itemPutC( pItem, "IMAGE" );
         break;

      case HB_FT_MEMO:
         if( pField->uiTypeExtended == ADS_VARCHAR )
            hb_itemPutC( pItem, "VARCHAR" );
         else
            hb_itemPutC( pItem, "M" );
         break;

      case HB_FT_LOGICAL:
         hb_itemPutC( pItem, "L" );
         break;

      case HB_FT_DATE:
         hb_itemPutC( pItem, "D" );
         break;

      case HB_FT_LONG:
         hb_itemPutC( pItem, "N" );
         break;

      case HB_FT_INTEGER:
         hb_itemPutC( pItem, "I" );
         break;

      case HB_FT_DOUBLE:
         hb_itemPutC( pItem, "DOUBLE" );
         break;

      case HB_FT_TIME:
         hb_itemPutC( pItem, "TIME" );
         break;

      case HB_FT_TIMESTAMP:
         hb_itemPutC( pItem, "TIMESTAMP" );
         break;

      case HB_FT_MODTIME:
         hb_itemPutC( pItem, "MODTIME" );
         break;

      case HB_FT_AUTOINC:
         hb_itemPutC( pItem, "AUTOINC" );
         break;

      case HB_FT_ROWVER:
         hb_itemPutC( pItem, "ROWVERSION" );
         break;

      case HB_FT_CURDOUBLE:
         hb_itemPutC( pItem, "CURDOUBLE" );
         break;

      case HB_FT_CURRENCY:
         hb_itemPutC( pItem, "Y" );
         break;

      default:
         hb_itemPutC( pItem, "U" );
         break;
   }
   return HB_SUCCESS;
}

static HB_ERRCODE adsFieldName( ADSAREAP pArea, USHORT uiIndex, void * szName )
{
   UNSIGNED16 u16Len = pArea->area.uiMaxFieldNameLength + 1;

   HB_TRACE(HB_TR_DEBUG, ("adsFieldName(%p, %hu, %p)", pArea, uiIndex, szName));

   if( uiIndex > pArea->area.uiFieldCount )
      return HB_FAILURE;

   AdsGetFieldName( pArea->hTable, uiIndex, ( UNSIGNED8 * ) szName, &u16Len );

   return HB_SUCCESS;
}

static HB_ERRCODE adsFlush( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsFlush(%p)", pArea ));

   /* This function should flush current record buffer if hot and
      send to OS request to flush its file buffers to disk, so as well as
      of AdsWriteRecord(), AdsFlushFileBuffers() should be used FOR LOCAL
      TABLES (it's ignored by Remote Server).
      AdsWriteRecord() "flushes to the Advantage server",
      AdsFlushFileBuffers() tells the local server to flush to disk.
      Without it, we are dependent on the adslocal.cfg Flush Frequency setting.
   */

   if( !pArea->fReadonly )
   {
      AdsWriteRecord( pArea->hTable );
#if ADS_LIB_VERSION > 600
      if( hb_setGetL( HB_SET_HARDCOMMIT ) )
         AdsFlushFileBuffers( pArea->hTable );
#endif
   }

   return HB_SUCCESS;
}

static HB_ERRCODE adsGetRec( ADSAREAP pArea, BYTE ** pBuffer )
{
   UNSIGNED32 u32Len = ( UNSIGNED32 ) pArea->ulRecordLen, u32Result;

   HB_TRACE(HB_TR_DEBUG, ("adsGetRec(%p, %p)", pArea, pBuffer));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   *pBuffer = pArea->pRecord;
   if( !pArea->fPositioned )
   {
      memset( pArea->pRecord, ' ', u32Len );
      return HB_FAILURE;
   }
   else
      u32Result = AdsGetRecord( pArea->hTable, pArea->pRecord, &u32Len );

   return u32Result == AE_SUCCESS ? HB_SUCCESS : HB_FAILURE;
}

static HB_ERRCODE adsGetValue( ADSAREAP pArea, USHORT uiIndex, PHB_ITEM pItem )
{
   LPFIELD    pField;
   BYTE *     pBuffer = pArea->pRecord;
   UNSIGNED32 u32Length;
   UNSIGNED32 ulRetVal;

   HB_TRACE(HB_TR_DEBUG, ("adsGetValue(%p, %hu, %p)", pArea, uiIndex, pItem));

   if( !uiIndex || uiIndex > pArea->area.uiFieldCount )
      return HB_FAILURE;

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   pField = pArea->area.lpFields + uiIndex - 1;

   /* This code was optimized for use ADSFIELD() macro instead */
   /* AdsGetFieldName() function for speed. ToninhoFwi, 22/07/2003 */

   switch( pField->uiType )
   {
      case HB_FT_STRING:
      case HB_FT_VARLENGTH:
         u32Length = pArea->maxFieldLen + 1;
         if( !pArea->fPositioned )
         {
            u32Length = pField->uiType == HB_FT_STRING ? pField->uiLen : 0;
            memset( pBuffer, ' ', u32Length );
            ulRetVal = AE_SUCCESS;
         }
#ifdef ADS_USE_OEM_TRANSLATION
#if ADS_LIB_VERSION >= 900
         else if( hb_ads_bOEM && pField->uiTypeExtended != ADS_VARBINARY_FOX )
#else
         else if( hb_ads_bOEM )
#endif
         {
#if ADS_LIB_VERSION >= 600
            ulRetVal = AdsGetFieldRaw( pArea->hTable, ADSFIELD( uiIndex ), pBuffer, &u32Length );
            if( ulRetVal != AE_SUCCESS )
            {
               u32Length = pField->uiType == HB_FT_STRING ? pField->uiLen : 0;
               memset( pBuffer, ' ', u32Length );
            }
#else
            ulRetVal = AdsGetField( pArea->hTable, ADSFIELD( uiIndex ), pBuffer, &u32Length, ADS_NONE );
            if( ulRetVal != AE_SUCCESS )
            {
               u32Length = pField->uiType == HB_FT_STRING ? pField->uiLen : 0;
               memset( pBuffer, ' ', u32Length );
            }
            else
            {
               char * pBufOem = hb_adsAnsiToOem( ( char * ) pBuffer, u32Length );
               memcpy( pBuffer, pBufOem, u32Length );
               hb_adsOemAnsiFree( pBufOem );
            }
#endif
         }
#endif
         else
         {
            ulRetVal = AdsGetField( pArea->hTable, ADSFIELD( uiIndex ), pBuffer, &u32Length, ADS_NONE );
            if( ulRetVal != AE_SUCCESS )
            {
               u32Length = pField->uiType == HB_FT_STRING ? pField->uiLen : 0;
               memset( pBuffer, ' ', u32Length );
            }
         }
         hb_itemPutCL( pItem, ( char * ) pBuffer, u32Length );
         break;

      case HB_FT_TIMESTAMP:
      case HB_FT_MODTIME:
      case HB_FT_TIME:
      {
         SIGNED32 lTime = 0, lDate = 0;
         ulRetVal = AdsGetMilliseconds( pArea->hTable, ADSFIELD( uiIndex ), &lTime );
         if( ulRetVal != AE_SUCCESS )
         {
            lTime = 0;
            pArea->area.fEof = TRUE;
         }
         else if( pField->uiType != HB_FT_TIME )
         {
            ulRetVal = AdsGetJulian( pArea->hTable, ADSFIELD( uiIndex ), &lDate );
            if( ulRetVal != AE_SUCCESS )
            {
               pArea->area.fEof = TRUE;
               lDate = 0;
            }
         }
         hb_itemPutDTL( pItem, lDate, lTime );
         break;
      }
      case HB_FT_INTEGER:
      {
         SIGNED32 lVal = 0;
         ulRetVal = AdsGetLong( pArea->hTable, ADSFIELD( uiIndex ), &lVal );
         if( ulRetVal != AE_SUCCESS )
         {
            lVal = 0;
            pArea->area.fEof = TRUE;
         }
         if( pField->uiTypeExtended == ADS_SHORTINT )
            hb_itemPutNILen( pItem, ( int ) lVal, 6 );
         else
            hb_itemPutNLLen( pItem, ( LONG ) lVal, 11 );
         break;
      }
#if ADS_LIB_VERSION >= 700 && !defined(HB_LONG_LONG_OFF)

      case HB_FT_AUTOINC:
      {
         SIGNED64 qVal = 0;
         ulRetVal = AdsGetLongLong( pArea->hTable, ADSFIELD( uiIndex ), &qVal );
         if( ulRetVal != AE_SUCCESS )
         {
            qVal = 0;
            pArea->area.fEof = TRUE;
         }
         hb_itemPutNIntLen( pItem, ( HB_LONG ) qVal, 10 );
         break;
      }
      case HB_FT_ROWVER:
      {
         SIGNED64 qVal = 0;
         ulRetVal = AdsGetLongLong( pArea->hTable, ADSFIELD( uiIndex ), &qVal );
         if( ulRetVal != AE_SUCCESS )
         {
            qVal = 0;
            pArea->area.fEof = TRUE;
         }
         hb_itemPutNIntLen( pItem, ( HB_LONG ) qVal, 20 );
         break;
      }
#else
      case HB_FT_AUTOINC:
      case HB_FT_ROWVER:
      {
         DOUBLE   dVal = 0;

         ulRetVal = AdsGetDouble( pArea->hTable, ADSFIELD( uiIndex ), &dVal );
         if( ulRetVal != AE_SUCCESS )
         {
            dVal = 0.0;
            pArea->area.fEof = TRUE;
         }
         hb_itemPutNLen( pItem, dVal,
                         pField->uiTypeExtended == HB_FT_AUTOINC ? 10 : 20, 0 );
      }
#endif
      case HB_FT_LONG:
      case HB_FT_DOUBLE:
      case HB_FT_CURDOUBLE:
      case HB_FT_CURRENCY:
      {
         DOUBLE   dVal = 0;

         ulRetVal = AdsGetDouble( pArea->hTable, ADSFIELD( uiIndex ), &dVal );
         if( ulRetVal != AE_SUCCESS )
         {
            dVal = 0.0;
            pArea->area.fEof = TRUE;
         }
#ifdef ADS_MONEY /* Not defined below 7.00 */
         if( pField->uiTypeExtended == ADS_CURDOUBLE ||
             pField->uiTypeExtended == ADS_DOUBLE ||
             pField->uiTypeExtended == ADS_MONEY )
#else
         if( pField->uiTypeExtended == ADS_CURDOUBLE ||
             pField->uiTypeExtended == ADS_DOUBLE )
#endif
         {
            hb_itemPutNDLen( pItem, dVal,
                             20 - ( pField->uiDec > 0 ? ( pField->uiDec + 1 ) : 0 ),
                             ( int ) pField->uiDec );
         }
         else if( pField->uiDec )
         {
            hb_itemPutNDLen( pItem, dVal,
                             ( int ) pField->uiLen - ( pField->uiDec + 1 ),
                             ( int ) pField->uiDec );
         }
         else
         {
            hb_itemPutNLen( pItem, dVal, ( int ) pField->uiLen, 0 );
         }
         break;
      }
      case HB_FT_DATE:
      {
         SIGNED32 lDate;

         ulRetVal = AdsGetJulian( pArea->hTable, ADSFIELD( uiIndex ), &lDate );
         if( ulRetVal != AE_SUCCESS )
         {
            pArea->area.fEof = TRUE;
            lDate = 0;
         }
         hb_itemPutDL( pItem, lDate );
         break;
      }

      case HB_FT_LOGICAL:
      {
         UNSIGNED16 pbValue = FALSE;
         ulRetVal = AdsGetLogical( pArea->hTable, ADSFIELD( uiIndex ), &pbValue );
         if( ulRetVal != AE_SUCCESS )
         {
            pbValue = FALSE;
            pArea->area.fEof = TRUE;
         }
         hb_itemPutL( pItem, pbValue != 0 );
         break;
      }

      case HB_FT_MEMO:
      case HB_FT_BLOB:
      case HB_FT_PICTURE:
      {
         UNSIGNED8 *pucBuf;
         UNSIGNED32 u32Len;
         UNSIGNED16 u16Type;

         ulRetVal = AdsGetMemoDataType( pArea->hTable, ADSFIELD( uiIndex ), &u16Type );
         if( ulRetVal != AE_SUCCESS )
            hb_itemPutC( pItem, "" );
         else if( u16Type != ADS_BINARY && u16Type != ADS_IMAGE )
         {
            ulRetVal = AdsGetMemoLength( pArea->hTable, ADSFIELD( uiIndex ), &u32Len );
            if( ulRetVal != AE_SUCCESS )
               hb_itemPutC( pItem, "" );
            else
            {
               if( u32Len > 0 )
               {
                  u32Len++;                 /* make room for NULL */
                  pucBuf = ( UNSIGNED8 * ) hb_xgrab( u32Len );
                  ulRetVal = AdsGetString( pArea->hTable, ADSFIELD( uiIndex ), pucBuf, &u32Len, ADS_NONE );
                  if( ulRetVal != AE_SUCCESS )
                     hb_itemPutC( pItem, "" );
                  else
                  {
                     char * szRet = hb_adsAnsiToOem( ( char * ) pucBuf, u32Len );
                     hb_itemPutCL( pItem, szRet, u32Len );
                     hb_adsOemAnsiFree( szRet );
                  }
                  hb_xfree( pucBuf );
               }
               else
                  hb_itemPutC( pItem, "" );
            }
         }
         else
         {
            ulRetVal = AdsGetBinaryLength( pArea->hTable, ADSFIELD( uiIndex ), &u32Len );
            if( ulRetVal != AE_SUCCESS )
               hb_itemPutC( pItem, "" );
            else
            {
               u32Len++;                  /* make room for NULL */
               pucBuf = ( UNSIGNED8 * ) hb_xgrab( u32Len );
               AdsGetBinary( pArea->hTable, ADSFIELD( uiIndex ), 0, pucBuf, &u32Len );
               hb_itemPutCLPtr( pItem, ( char * ) pucBuf, u32Len );
            }
         }
         hb_itemSetCMemo( pItem );
         break;
      }
      default:
         commonError( pArea, EG_DATATYPE, 1020, 0, NULL, 0, NULL );
         return HB_FAILURE;
   }

   if( ulRetVal == AE_NO_CURRENT_RECORD )
   {
      if( pArea->fPositioned )
      {
         /* It should not happen - sth desynchronize WA with ADS,
            update area flags, Druzus */
         hb_adsUpdateAreaFlags( pArea );
      }
   }
   else if( ulRetVal != AE_SUCCESS )
   {
      if( ulRetVal == AE_DATA_TOO_LONG )
         commonError( pArea, EG_DATAWIDTH, EDBF_DATAWIDTH, 0, NULL, 0, NULL );
      else
         commonError( pArea, EG_READ, ( USHORT ) ulRetVal, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }

   return HB_SUCCESS;
}

static HB_ERRCODE adsGetVarLen( ADSAREAP pArea, USHORT uiIndex, ULONG * ulLen )
{
   LPFIELD pField;

   HB_TRACE(HB_TR_DEBUG, ("adsGetVarLen(%p, %hu, %p)", pArea, uiIndex, ulLen));

   if( uiIndex > pArea->area.uiFieldCount )
      return HB_FAILURE;

   pField = pArea->area.lpFields + uiIndex - 1;
   if( pField->uiType == 'M' )
   {
      UNSIGNED32 u32Len;

      /* resolve any pending relations */
      if( pArea->lpdbPendingRel )
         SELF_FORCEREL( ( AREAP ) pArea );

      if( !pArea->fPositioned )
         *ulLen = 0;
      else if( AdsGetMemoLength( pArea->hTable, ADSFIELD( uiIndex ), &u32Len ) == AE_NO_CURRENT_RECORD )
      {
         /* It should not happen - sth desynchronize WA with ADS,
            update area flags, Druzus */
         hb_adsUpdateAreaFlags( pArea );
         *ulLen = 0;
      }
      else
         *ulLen = u32Len;
   }
   else
      *ulLen = pField->uiLen;

/*
   if( pField->uiType == 'M' )
      * ulLen = ( ( LPDBFMEMO ) pField->memo )->uiLen;
   else
      * ulLen = pField->uiLen;
*/
   return HB_SUCCESS;
}

#define  adsGoCold                NULL
#define  adsGoHot                 NULL

static HB_ERRCODE adsPutRec( ADSAREAP pArea, const BYTE * pBuffer )
{
   UNSIGNED32 u32Len = ( UNSIGNED32 ) pArea->ulRecordLen, u32Result;

   HB_TRACE(HB_TR_DEBUG, ("adsGetRec(%p, %p)", pArea, pBuffer));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( !pArea->fPositioned )
      return HB_SUCCESS;

   if( hb_ads_bTestRecLocks )
   {
      if( ! hb_adsCheckLock( pArea ) == HB_SUCCESS )
         return HB_FAILURE;
   }

   u32Result = AdsSetRecord( pArea->hTable, ( UNSIGNED8 * ) pBuffer, u32Len );

   return u32Result == AE_SUCCESS ? HB_SUCCESS : HB_FAILURE;
}


static HB_ERRCODE adsPutValue( ADSAREAP pArea, USHORT uiIndex, PHB_ITEM pItem )
{
   LPFIELD pField;
   USHORT uiCount;
   BYTE * szText;
   BOOL bTypeError = TRUE;
   UNSIGNED32 ulRetVal = 0;

   HB_TRACE(HB_TR_DEBUG, ("adsPutValue(%p, %hu, %p)", pArea, uiIndex, pItem));

   if( !uiIndex || uiIndex > pArea->area.uiFieldCount )
      return HB_FAILURE;

   /* -----------------10/30/2003 3:54PM----------------

      ADS has Implicit Record locking that can mask programming errors.
      Implicit locking can occur the first time a value is written to a
      field with no lock in effect. The lock can potentially remain in
      effect indefinitely if the record pointer is not moved.

      The hb_ads_bTestRecLocks flag can be set using
          AdsTestRecLocks( lOnOff ) in adsfunc.c.
      If ON, we see if the file is open exclusively or locked, and whether
      the record has been explicitly locked already. If not, we throw
      an error so the developer can catch the missing lock condition.
      For performance reasons, Release code should leave this OFF.
         Although the call to AdsIsRecordLocked is documented as a client
         call, not a server request, and should be fast, it will be
         called for EACH FIELD as it is assigned a value.

    --------------------------------------------------*/
   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( !pArea->fPositioned )
   {
      /* TODO: it should return SUCCESS for Clipper compatibility */
      return HB_FAILURE;
   }

   if( hb_ads_bTestRecLocks )
   {
      if( ! hb_adsCheckLock( pArea ) == HB_SUCCESS )
         return HB_FAILURE;
   }

   pField = pArea->area.lpFields + uiIndex - 1;
   szText = pArea->pRecord;

   /* This code was optimized for use ADSFIELD() macro instead */
   /* AdsGetFieldName() function for speed. ToninhoFwi, 22/07/2003 */

   switch( pField->uiType )
   {
      case HB_FT_STRING:
      case HB_FT_VARLENGTH:
         if( HB_IS_STRING( pItem ) )
         {
            bTypeError = FALSE;
            uiCount = ( USHORT ) hb_itemGetCLen( pItem );
            if( uiCount > pField->uiLen )
               uiCount = pField->uiLen;

#ifdef ADS_USE_OEM_TRANSLATION
            if( hb_ads_bOEM )
            {
#if ADS_LIB_VERSION >= 600
               ulRetVal = AdsSetFieldRaw( pArea->hTable, ADSFIELD( uiIndex ), ( UNSIGNED8 * ) hb_itemGetCPtr( pItem ), uiCount );
#else
               char * pBuffer = hb_adsOemToAnsi( hb_itemGetCPtr( pItem ), uiCount );
               ulRetVal = AdsSetString( pArea->hTable, ADSFIELD( uiIndex ), ( UNSIGNED8 * ) pBuffer, uiCount );
               hb_adsOemAnsiFree( pBuffer );
#endif
            }
            else
#endif
            {
               ulRetVal = AdsSetString( pArea->hTable, ADSFIELD( uiIndex ), ( UNSIGNED8 * ) hb_itemGetCPtr( pItem ), uiCount );
            }

         }
         break;

      case HB_FT_LONG:
      case HB_FT_INTEGER:
      case HB_FT_DOUBLE:
      case HB_FT_AUTOINC:
      case HB_FT_CURDOUBLE:
      case HB_FT_CURRENCY:
         if( HB_IS_NUMERIC( pItem ) )
         {
            bTypeError = FALSE;
            ulRetVal = AdsSetDouble( pArea->hTable, ADSFIELD( uiIndex ), hb_itemGetND( pItem ) );
            /* write to autoincrement field will gen error 5066
               #if HB_TR_LEVEL >= HB_TR_DEBUG
                  if( pField->uiTypeExtended == ADS_AUTOINC )
                     HB_TRACE(HB_TR_INFO, ("adsPutValue() error"));
               #endif
             */
         }
         break;

      case HB_FT_DATETIME:
      case HB_FT_TIME:
      case HB_FT_TIMESTAMP:
      case HB_FT_MODTIME:
         if( HB_IS_DATETIME( pItem ) )
         {
            long lDate, lTime;
            bTypeError = FALSE;
            hb_itemGetDTL( pItem, &lDate, &lTime );
            ulRetVal = AdsSetMilliseconds( pArea->hTable, ADSFIELD( uiIndex ), lTime );
            if( ulRetVal == AE_SUCCESS && pField->uiType != HB_FT_TIME )
               ulRetVal = AdsSetJulian( pArea->hTable, ADSFIELD( uiIndex ), lDate );
         }
         break;

      case HB_FT_DATE:
         if( HB_IS_DATE( pItem ) )
         {
            bTypeError = FALSE;
            ulRetVal = AdsSetJulian( pArea->hTable, ADSFIELD( uiIndex ), hb_itemGetDL( pItem ) );
         }
         break;

      case HB_FT_LOGICAL:
         if( HB_IS_LOGICAL( pItem ) )
         {
            bTypeError = FALSE;
            *szText = hb_itemGetL( pItem ) ? 'T' : 'F';
            ulRetVal = AdsSetLogical( pArea->hTable, ADSFIELD( uiIndex ), ( UNSIGNED16 ) hb_itemGetL( pItem ) );
         }
         break;

      case HB_FT_MEMO:
      case HB_FT_BLOB:
      case HB_FT_PICTURE:
         if( HB_IS_STRING( pItem ) )
         {
            ULONG ulLen;

            bTypeError = FALSE;
            ulLen = ( ULONG ) hb_itemGetCLen( pItem );

            /* ToninhoFwi - 09/12/2006 - In the previous code ulLen was limited to 0xFFFF
               so, I comment it, because ADS support up to 4Gb in memo/binary/image fields.
               Advantage documentations says that we need use AdsSetBinary in binary/image
               fields. I tested these special fields with AdsSetString() and it works, but
               is a little bit slower to save big image file in the fields, so I keep
               AdsSetString() only for commom memo fields and AdsSetBinary() for the others.
            */
            if( pField->uiTypeExtended != ADS_BINARY && pField->uiTypeExtended != ADS_IMAGE )
            {
               char * szRet = hb_adsOemToAnsi( hb_itemGetCPtr( pItem ), ulLen );
               ulRetVal = AdsSetString( pArea->hTable, ADSFIELD( uiIndex ),
                  ( UNSIGNED8 * ) szRet, ulLen );
               hb_adsOemAnsiFree( szRet );
            }
            else
            {
               ulRetVal = AdsSetBinary( pArea->hTable, ADSFIELD( uiIndex ),
                  pField->uiTypeExtended, ulLen, 0,
                  ( UNSIGNED8 * ) hb_itemGetCPtr( pItem ), ulLen );
            }
         }
         break;
   }

   if( bTypeError )
   {
      commonError( pArea, EG_DATATYPE, 1020, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }

   if( ulRetVal != AE_SUCCESS )
   {
      if( ulRetVal == AE_LOCK_FAILED || ulRetVal == AE_RECORD_NOT_LOCKED )
         commonError( pArea, EG_UNLOCKED, EDBF_UNLOCKED, 0, NULL, 0, NULL );
      else if( ulRetVal == AE_TABLE_READONLY )
         commonError( pArea, EG_READONLY, EDBF_READONLY, 0, NULL, 0, NULL );
      else if( ulRetVal == AE_DATA_TOO_LONG )
         return commonError( pArea, EG_DATAWIDTH, EDBF_DATAWIDTH, 0, NULL, EF_CANDEFAULT, NULL );
      else
         commonError( pArea, EG_WRITE, ( USHORT ) ulRetVal, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }

   return HB_SUCCESS;
}

static HB_ERRCODE adsRecall( ADSAREAP pArea )
{
   UNSIGNED32 u32RetVal;

   HB_TRACE(HB_TR_DEBUG, ("adsRecall(%p)", pArea));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( !pArea->fPositioned )
      return HB_SUCCESS;

   if( hb_ads_bTestRecLocks )
   {
      if( ! hb_adsCheckLock( pArea ) == HB_SUCCESS )
         return HB_FAILURE;
   }

   u32RetVal = AdsRecallRecord( pArea->hTable );

   return u32RetVal == AE_SUCCESS ? HB_SUCCESS : HB_FAILURE;
}

static HB_ERRCODE adsRecCount( ADSAREAP pArea, ULONG * pRecCount )
{
   UNSIGNED32 u32RecCount = 0, u32Result;

   HB_TRACE(HB_TR_DEBUG, ("adsRecCount(%p, %p)", pArea, pRecCount));

   u32Result = AdsGetRecordCount( pArea->hTable, ADS_IGNOREFILTERS | ( s_bRefreshCount ? ADS_REFRESHCOUNT : 0 ), &u32RecCount );
   *pRecCount = ( ULONG ) u32RecCount;

   return u32Result == AE_SUCCESS ? HB_SUCCESS : HB_FAILURE;
}

static HB_ERRCODE adsRecInfo( ADSAREAP pArea, PHB_ITEM pRecID, USHORT uiInfoType, PHB_ITEM pInfo )
{
   ULONG ulRecNo = hb_itemGetNL( pRecID );
   HB_ERRCODE uiRetVal = HB_SUCCESS;

   HB_TRACE(HB_TR_DEBUG, ("adsRecInfo(%p, %p, %hu, %p)", pArea, pRecID, uiInfoType, pInfo));

   switch( uiInfoType )
   {
      case DBRI_DELETED:
      {
         BOOL fDeleted = FALSE;
         ULONG ulCurrRec = 0;

         if( ulRecNo != 0 )
         {
            SELF_RECNO( ( AREAP ) pArea, &ulCurrRec );
            if( ulCurrRec == ulRecNo )
               ulCurrRec = 0;
            else
               SELF_GOTO( ( AREAP ) pArea, ulRecNo );
         }
         uiRetVal = SELF_DELETED( ( AREAP ) pArea, &fDeleted );
         if( ulCurrRec != 0 )
            SELF_GOTO( ( AREAP ) pArea, ulCurrRec );
         hb_itemPutL( pInfo, fDeleted );
         break;
      }
      case DBRI_LOCKED:
      {
         UNSIGNED16 u16Locked = 0;

         if( ulRecNo == 0 )
            uiRetVal = SELF_RECNO( ( AREAP ) pArea, &ulRecNo );

         if( AdsIsRecordLocked( pArea->hTable, ulRecNo, &u16Locked ) != AE_SUCCESS )
         {
            uiRetVal = HB_FAILURE;
         }
         hb_itemPutL( pInfo, u16Locked != 0 );
         break;
      }
      case DBRI_RECSIZE:
         hb_itemPutNL( pInfo, pArea->ulRecordLen );
         break;

      case DBRI_RECNO:
         if( ulRecNo == 0 )
            uiRetVal = SELF_RECNO( ( AREAP ) pArea, &ulRecNo );
         hb_itemPutNL( pInfo, ulRecNo );
         break;

      case DBRI_UPDATED:
         /* TODO: this will not work properly with current ADS RDD */
         hb_itemPutL( pInfo, FALSE );
         break;

      default:
         return SUPER_RECINFO( ( AREAP ) pArea, pRecID, uiInfoType, pInfo );
   }
   return uiRetVal;
}


static HB_ERRCODE adsRecNo( ADSAREAP pArea, ULONG * ulRecNo )
{
   UNSIGNED32 u32RecNo, u32Result;

   HB_TRACE(HB_TR_DEBUG, ("adsRecNo(%p, %p)", pArea, ulRecNo));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   u32Result = AdsGetRecordNum( pArea->hTable, ADS_IGNOREFILTERS, &u32RecNo );
   if( u32RecNo != 0 && u32Result == AE_SUCCESS )
      pArea->ulRecNo = u32RecNo;

   *ulRecNo = pArea->ulRecNo;

   return u32Result == AE_SUCCESS ? HB_SUCCESS : HB_FAILURE;
}

static HB_ERRCODE adsRecId( ADSAREAP pArea, PHB_ITEM pRecNo )
{
   HB_ERRCODE errCode;
   ULONG ulRecNo;

   HB_TRACE(HB_TR_DEBUG, ("adsRecId(%p, %p)", pArea, pRecNo));

   errCode = SELF_RECNO( ( AREAP ) pArea, &ulRecNo );
   hb_itemPutNL( pRecNo, ulRecNo );
   return errCode;
}

#define  adsSetFieldExtent        NULL
#define  adsAlias                 NULL

static HB_ERRCODE adsClose( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsClose(%p)", pArea));

   return hb_adsCloseCursor( pArea );
}

static HB_ERRCODE adsCreate( ADSAREAP pArea, LPDBOPENINFO pCreateInfo )
{
   ADSHANDLE hTable, hConnection;
   UNSIGNED32 uRetVal, u32Length, uiFldLen, uiLen;
   UNSIGNED8 *ucfieldDefs, *ucfieldPtr;
   UNSIGNED8 ucBuffer[MAX_STR_LEN + 1];
   USHORT uiCount;
   LPFIELD pField;
   const char * cType;

   HB_TRACE(HB_TR_DEBUG, ("adsCreate(%p, %p)", pArea, pCreateInfo));

   hConnection = HB_ADS_DEFCONNECTION( pCreateInfo->ulConnection );

   pArea->szDataFileName = hb_strdup( ( char * ) pCreateInfo->abName );

   pArea->maxFieldLen = 0;

   /* uiLen = length of buffer for all field definition info times number of fields.
      For extended types cType may be up to 6 chars, and
      there are up to 4 punctuation symbols ( ,; ),
      field length (8) and number of decimals (2).
      So, per field it should be  ( 6 + 4 + 8 + 2 = 20 ):
         uiMaxFieldNameLength + 20

    * 9/19/2006 BH: this is an oversized buffer since most fields don't have
    * 128-byte names. But the overhead in counting up the bytes is worse than
    * allocating a bigger buffer. We need to make sure it's not too big, though.
    * ADS docs say max # of fields is fnameLen + 10
    *     65135 / ( 10 + AverageFieldNameLength )
    */

   uiLen = ( UNSIGNED32 ) pArea->area.uiFieldCount * (pArea->area.uiMaxFieldNameLength + 20) + 1;
   if( uiLen > 65135 )
      uiLen = 65135;

   ucfieldDefs = ( UNSIGNED8 * ) hb_xgrab( uiLen );
   ucfieldDefs[ 0 ] = '\0';
   ucfieldPtr = ucfieldDefs;

   pField = pArea->area.lpFields;
   for( uiCount = 0; uiCount < pArea->area.uiFieldCount; uiCount++ )
   {
      if( ( ULONG ) pField->uiLen > pArea->maxFieldLen )
        pArea->maxFieldLen = pField->uiLen;

      cType = NULL;
      switch( pField->uiType )
      {
         case HB_FT_DATE:
            if( pField->uiTypeExtended == ADS_COMPACTDATE || pField->uiLen <= 4 )
               cType = "ShortD";
            else
               cType = "D";
            break;
         case HB_FT_STRING:
            if( pField->uiTypeExtended == ADS_RAW ||
                ( pField->uiFlags & HB_FF_BINARY ) )
               cType = "Raw";
            else
               cType = "C";
            break;
         case HB_FT_VARLENGTH:
#if ADS_LIB_VERSION >= 900
            if( pField->uiTypeExtended == ADS_VARBINARY_FOX )
               cType = "VarB";
            else
#endif
               cType = "VarC";
            break;
         case HB_FT_MEMO:
            cType = pField->uiTypeExtended == ADS_VARCHAR ? "VarC" : "M";
            break;
         case HB_FT_BLOB:
            cType = "Binary"; /* "W" */
            break;
         case HB_FT_PICTURE:
            cType = "Image"; /* "P" */
            break;
         case HB_FT_LOGICAL:
            cType = "L";
            break;
         case HB_FT_LONG:
            cType = "N";
            break;
         case HB_FT_INTEGER:
            cType = pField->uiLen == 2 ? "ShortI" : "Int"; /* "I" */
            break;
         case HB_FT_DOUBLE:
            cType = "Double"; /* "B" */
            break;
         case HB_FT_TIME:
            cType = "Time"; /* "T" */
            break;
         case HB_FT_TIMESTAMP: /* "@" */
            cType = "TimeSt";
            break;
         case HB_FT_MODTIME: /* "=" */
            cType = "ModTime";
            break;
         case HB_FT_AUTOINC: /* "+" */
            cType = "Auto";
            break;
         case HB_FT_ROWVER:  /* "^" */
            cType = "RowVer";
            break;
         case HB_FT_CURDOUBLE: /* "Z" */
            cType = "CurD";
            break;
         case HB_FT_CURRENCY: /* "Z" */
            cType = "money";
            break;
      }

      if( cType == NULL )
         return HB_FAILURE; /* RT_ERROR */

      switch( pField->uiType )
      {
         case HB_FT_DATE:
         case HB_FT_LOGICAL:
         case HB_FT_MEMO:
         case HB_FT_BLOB:
         case HB_FT_PICTURE:
            if( pField->uiTypeExtended != ADS_VARCHAR )
            {
               uiFldLen = hb_snprintf( ( char * ) ucBuffer, MAX_STR_LEN, "%.*s,%s;",
                                    ( int ) pArea->area.uiMaxFieldNameLength,
                                    hb_dynsymName( ( PHB_DYNS ) pField->sym ),
                                    cType );
               break;
            }
         default:
            uiFldLen = hb_snprintf( ( char * ) ucBuffer, MAX_STR_LEN, "%.*s,%s,%d,%d;",
                                 ( int ) pArea->area.uiMaxFieldNameLength,
                                 hb_dynsymName( ( PHB_DYNS ) pField->sym ),
                                 cType, pField->uiLen, pField->uiDec );
            break;
      }

      if( uiFldLen == 0 )
      {
         uiFldLen = (UNSIGNED32) strlen( ( char * ) ucBuffer );  /* should have been set by hb_snprintf above. */
      }
      if( uiFldLen >= uiLen )
      {
         /* RT_ERROR; probably too many fields */
         return HB_FAILURE;
      }
      memcpy( ucfieldPtr, ucBuffer, uiFldLen );
      uiLen -= uiFldLen;
      ucfieldPtr += uiFldLen;

      pField++;
   }
   *ucfieldPtr = '\0';

   if( pArea->maxFieldLen < 24 )
      pArea->maxFieldLen = 24;

   uRetVal = AdsCreateTable( hConnection,
                             ( UNSIGNED8* ) pCreateInfo->abName,
                             ( UNSIGNED8* ) pCreateInfo->atomAlias,
                             ( UNSIGNED16 ) pArea->iFileType,
                             ( UNSIGNED16 ) hb_ads_iCharType,
                             ( UNSIGNED16 ) hb_ads_iLockType,
                             ( UNSIGNED16 ) hb_ads_iCheckRights,
                             ( UNSIGNED16 ) hb_setGetNI( HB_SET_MBLOCKSIZE ),
                             ucfieldDefs,
                             &hTable );
   hb_xfree( ucfieldDefs );

   if( uRetVal != AE_SUCCESS )
   {
      HB_TRACE(HB_TR_INFO, ("adsCreate() error"));
      return HB_FAILURE;
   }
   /*
    * In Clipper CREATE() keeps database open on success [druzus]
    */
   pArea->hTable    = hTable;
   pArea->fShared   = FALSE;  /* pCreateInfo->fShared; */
   pArea->fReadonly = FALSE;  /* pCreateInfo->fReadonly */

   /* If successful call SUPER_CREATE to finish system jobs */
   if( SUPER_CREATE( ( AREAP ) pArea, pCreateInfo ) != HB_SUCCESS )
   {
      SELF_CLOSE( ( AREAP ) pArea );
      return HB_FAILURE;
   }

   AdsGetRecordLength( pArea->hTable, &u32Length );
   pArea->ulRecordLen = u32Length;
   /* Alloc record buffer - because it's also used for some extended types
      conversion it has to be at least 25 bytes size */
   pArea->pRecord = ( BYTE * ) hb_xgrab( HB_MAX( pArea->ulRecordLen, pArea->maxFieldLen ) + 1 );

   return SELF_GOTOP( ( AREAP ) pArea );
}

static HB_ERRCODE adsInfo( ADSAREAP pArea, USHORT uiIndex, PHB_ITEM pItem )
{
   UNSIGNED32 uRetVal;

   HB_TRACE(HB_TR_DEBUG, ("adsInfo(%p, %hu, %p)", pArea, uiIndex, pItem));

   switch( uiIndex )
   {
      case DBI_ISDBF:
      case DBI_CANPUTREC:
         hb_itemPutL( pItem, TRUE );
         break;

      case DBI_GETHEADERSIZE:
         hb_itemPutNL( pItem, pArea->iFileType == ADS_ADT ? 0 :
                       32 + pArea->area.uiFieldCount * 32 + 2 );
         break;

      case DBI_LASTUPDATE:
      {
         UNSIGNED8 pucFormat[ 11 ];
         UNSIGNED16 pusLen = 11;
         UNSIGNED8 pucDate[ 11 ];

         AdsGetDateFormat( pucFormat, &pusLen );
         AdsSetDateFormat( ( UNSIGNED8 * ) "YYYYMMDD" );
         AdsGetLastTableUpdate( pArea->hTable, ( UNSIGNED8 * )&pucDate, &pusLen );
         *( pucDate + 8 ) = '\0';
         hb_itemPutDS( pItem, ( char * ) pucDate );
         AdsSetDateFormat( pucFormat );
         break;
      }
      case DBI_GETRECSIZE:
         hb_itemPutNL( pItem, pArea->ulRecordLen );
         break;

      case DBI_GETLOCKARRAY:
      {
         UNSIGNED16 u16Count, u16;
         uRetVal = AdsGetNumLocks( pArea->hTable, &u16Count );
         if( uRetVal != AE_SUCCESS )
            return HB_FAILURE;

         if( u16Count )
         {
            UNSIGNED32 *puLocks;
            puLocks = ( UNSIGNED32 * ) hb_xgrab( ( u16Count + 1 ) * sizeof( UNSIGNED32 ) );
            AdsGetAllLocks( pArea->hTable, puLocks, &u16Count );

            if( u16Count )
            {
               hb_arrayNew( pItem, u16Count );
               for( u16 = 0; u16 < u16Count; ++u16 )
               {
                  hb_itemPutNL( hb_arrayGetItemPtr( pItem, ( ULONG ) u16 + 1 ),
                                puLocks[ u16 ] );
               }
            }
            hb_xfree( puLocks );

         }
         if( !u16Count )
            hb_arrayNew( pItem, 0 );    /* don't return nil */

         break;
      }

      case DBI_TABLEEXT:
         hb_itemPutC( pItem, adsTableExt( pArea->iFileType ) );
         break;

      case DBI_FULLPATH:
         {
            UNSIGNED8  aucBuffer[ MAX_STR_LEN + 1 ];
            UNSIGNED16 pusLen = MAX_STR_LEN;
            AdsGetTableFilename( pArea->hTable, ADS_FULLPATHNAME, aucBuffer, &pusLen );
            hb_itemPutCL( pItem, ( char * ) aucBuffer, pusLen );
            break;
         }

      case DBI_ISFLOCK:
         hb_itemPutL( pItem, pArea->fFLocked );
         break;

      case DBI_ISREADONLY:
         hb_itemPutL( pItem, pArea->fReadonly );
         break;

      case DBI_POSITIONED:
         hb_itemPutL( pItem, pArea->fPositioned );
         break;

      case DBI_LOCKCOUNT:
      {
         UNSIGNED16 u16Count;
         uRetVal = AdsGetNumLocks( pArea->hTable, &u16Count );
         if( uRetVal != AE_SUCCESS )
            return HB_FAILURE;

         hb_itemPutNL( pItem, ( LONG ) u16Count );
         break;
      }

      case DBI_SHARED:
         hb_itemPutL( pItem, pArea->fShared );
         break;

      case DBI_MEMOEXT:
         hb_itemPutC( pItem, adsMemoExt( pArea->iFileType ) );
         break;

      case DBI_DB_VERSION:        /* HOST driver Version */
      {
         UNSIGNED32 ulMajor;
         UNSIGNED32 ulMinor;
         UNSIGNED8  ucLetter;
         UNSIGNED8  ucDesc[ 128 ];
         UNSIGNED16 usDescLen = sizeof( ucDesc ) - 1;
         UNSIGNED8  ucVersion[ 256 ];

         AdsGetVersion( &ulMajor, &ulMinor, &ucLetter, ucDesc, &usDescLen);

         hb_snprintf( ( char * ) ucVersion, sizeof( ucVersion ), "%s, v%lu.%lu%c",
                   ( char * ) ucDesc, ( ULONG ) ulMajor, ( ULONG ) ulMinor, ucLetter );
         hb_itemPutC( pItem, ( char * ) ucVersion );
         break;
      }

      case DBI_RDD_VERSION:       /* RDD version (current RDD) */
         hb_itemPutC( pItem, HB_RDD_ADS_VERSION_STRING );
         break;

      /* unsupported options */
      case DBI_FILEHANDLE:        /* Handle of opened file */
      case DBI_VALIDBUFFER:       /* Is the current buffer valid */
      case DBI_GETSCOPE:          /* Locate codeblock */
      case DBI_LOCKOFFSET:        /* New locking offset */
      case DBI_MEMOHANDLE:        /* Dos handle for memo file */
      case DBI_MEMOBLOCKSIZE:     /* Blocksize in memo files */
         break;

      /* use workarea.c implmentation */
      default:
         return SUPER_INFO( ( AREAP ) pArea, uiIndex, pItem );
   }
   return HB_SUCCESS;
}

static HB_ERRCODE adsNewArea( ADSAREAP pArea )
{
   HB_ERRCODE errCode;

   HB_TRACE(HB_TR_DEBUG, ("adsNewArea(%p)", pArea));

   errCode = SUPER_NEW( ( AREAP ) pArea );
   if( errCode == HB_SUCCESS )
   {
      if( pArea->area.rddID == s_uiRddIdADT )
      {
         pArea->iFileType = ADS_ADT;
         pArea->area.uiMaxFieldNameLength = ADS_MAX_FIELD_NAME;
      }
      else if( pArea->area.rddID == s_uiRddIdADSNTX )
      {
         pArea->iFileType = ADS_NTX;
         pArea->area.uiMaxFieldNameLength = ADS_MAX_DBF_FIELD_NAME;
      }
      else if( pArea->area.rddID == s_uiRddIdADSCDX )
      {
         pArea->iFileType = ADS_CDX;
         pArea->area.uiMaxFieldNameLength = ADS_MAX_DBF_FIELD_NAME;
      }
#if ADS_LIB_VERSION >= 900
      else if( pArea->area.rddID == s_uiRddIdADSVFP )
      {
         pArea->iFileType = ADS_VFP;
         pArea->area.uiMaxFieldNameLength = ADS_MAX_DBF_FIELD_NAME;
      }
#endif
      else /* if( pArea->rddID == s_uiRddIdADS ) */
      {
         pArea->iFileType = hb_ads_iFileType;
         pArea->area.uiMaxFieldNameLength = ( pArea->iFileType == ADS_ADT ) ?
                                    ADS_MAX_FIELD_NAME : ADS_MAX_DBF_FIELD_NAME;
      }
   }
   return errCode;
}

static HB_ERRCODE adsOpen( ADSAREAP pArea, LPDBOPENINFO pOpenInfo )
{
   ADSHANDLE hTable = 0, hStatement = 0, hConnection;
   UNSIGNED32 u32RetVal, u32Length;
   USHORT uiFields = 0, uiCount;
   UNSIGNED8 szName[ ADS_MAX_FIELD_NAME + 1 ];
   /* See adsGettValue() for why we don't use pArea->area.uiMaxFieldNameLength here */
   UNSIGNED16 pusBufLen, pusType, pusDecimals;
   DBFIELDINFO dbFieldInfo;
   char szAlias[ HB_RDD_MAX_ALIAS_LEN + 1 ], * szFile;
   BOOL fDictionary = FALSE;

   HB_TRACE(HB_TR_DEBUG, ("adsOpen(%p)", pArea));

   hConnection = HB_ADS_DEFCONNECTION( pOpenInfo->ulConnection );
   u32RetVal = AdsGetHandleType( hConnection, &pusType );
   if( u32RetVal == AE_SUCCESS )
   {
#if ADS_LIB_VERSION >= 600 /* ADS_*_CONNECTION was added in >= 6.00 */
#if ADS_LIB_VERSION < 900 /* ADS_SYS_ADMIN_CONNECTION was removed in >= 9.00 */
      fDictionary = ( pusType == ADS_DATABASE_CONNECTION
                   || pusType == ADS_SYS_ADMIN_CONNECTION );
#else
      fDictionary = ( pusType == ADS_DATABASE_CONNECTION );
#endif
#endif
   }
   szFile = ( char * ) pOpenInfo->abName;

   if( pArea->hTable != 0 )
   {
      /*
       * table was open by ADSEXECUTESQL[DIRECT]() function
       * I do not like the way it was implemented but I also
       * do not have time to change it so I simply restored this
       * functionality, Druzus.
       */
      hTable = pArea->hTable;
      hStatement = pArea->hStatement;
   }
   else if( szFile && hb_strnicmp( szFile, "SELECT ", 7 ) == 0 )
   {
      u32RetVal = AdsCreateSQLStatement( hConnection, &hStatement );
      if( u32RetVal == AE_SUCCESS )
      {
         char * szSQL = hb_adsOemToAnsi( szFile, ( ULONG ) strlen( szFile ) );

#if ADS_LIB_VERSION >= 900
         if( pArea->iFileType == ADS_CDX ||
             pArea->iFileType == ADS_VFP )
#else
         if( pArea->iFileType == ADS_CDX )
#endif
         {
            AdsStmtSetTableType( hStatement, ( UNSIGNED16 ) pArea->iFileType );
         }

         u32RetVal = AdsExecuteSQLDirect( hStatement, ( UNSIGNED8 * ) szSQL, &hTable );

         hb_adsOemAnsiFree( szSQL );

         if( u32RetVal != AE_SUCCESS )
            AdsCloseSQLStatement( hStatement );
      }
      else
      {
         commonError( pArea, EG_OPEN, ( USHORT ) u32RetVal, 0, ( char * ) pOpenInfo->abName, 0, NULL );
         return HB_FAILURE;
      }
   }
   else
   {
      PHB_ITEM pError = NULL;
      BOOL fRetry;

      /* Use an  Advantage Data Dictionary
       * if fDictionary was set for this connection
       */
      do
      {
         u32RetVal = AdsOpenTable( hConnection,
                        ( UNSIGNED8* ) pOpenInfo->abName,
                        ( UNSIGNED8* ) pOpenInfo->atomAlias,
                        ( fDictionary ? ADS_DEFAULT : ( UNSIGNED16 ) pArea->iFileType),
                        ( UNSIGNED16 ) hb_ads_iCharType,
                        ( UNSIGNED16 ) hb_ads_iLockType,
                        ( UNSIGNED16 ) hb_ads_iCheckRights,
                        ( pOpenInfo->fShared ? ADS_SHARED : ADS_EXCLUSIVE ) |
                        ( pOpenInfo->fReadonly ? ADS_READONLY : ADS_DEFAULT ),
                        &hTable );
         if( u32RetVal != AE_SUCCESS )
         {
            /* 1001 and 7008 are standard ADS Open Errors that will usually be sharing issues */
            USHORT uiOsCOde = u32RetVal == 1001 || u32RetVal == 7008 ? 32 : 0;
            fRetry = commonError( pArea, EG_OPEN, ( USHORT ) u32RetVal, uiOsCOde,
                                  ( const char * ) pOpenInfo->abName,
                                  EF_CANRETRY | EF_CANDEFAULT, &pError ) == E_RETRY;
         }
         else
            fRetry = FALSE;
      }
      while( fRetry );

      if( pError )
         hb_errRelease( pError );

      if( u32RetVal != AE_SUCCESS )
         return HB_FAILURE;
   }

   /* Set default alias if necessary */
   if( !pOpenInfo->atomAlias )
   {
      UNSIGNED16 uiAliasLen = HB_RDD_MAX_ALIAS_LEN;
      if( AdsGetTableAlias( hTable, ( UNSIGNED8 * ) szAlias, &uiAliasLen ) == AE_SUCCESS )
         pOpenInfo->atomAlias = szAlias;
      else
         pOpenInfo->atomAlias = "";
   }

   pArea->szDataFileName = hb_strdup( ( char * ) ( pOpenInfo->abName ) );
   pArea->hTable         = hTable;
   pArea->hStatement     = hStatement;
   pArea->hOrdCurrent    = 0;
   pArea->fShared        = pOpenInfo->fShared;
   pArea->fReadonly      = pOpenInfo->fReadonly;

   SELF_FIELDCOUNT( ( AREAP ) pArea, &uiFields );

   SELF_SETFIELDEXTENT( ( AREAP ) pArea, uiFields );

   memset( &dbFieldInfo, 0, sizeof( dbFieldInfo ) );
   pArea->maxFieldLen = 0;

   for( uiCount = 1; uiCount <= uiFields; uiCount++ )
   {
      pusBufLen = ADS_MAX_FIELD_NAME;
      AdsGetFieldName( pArea->hTable, uiCount, szName, &pusBufLen );
      szName[ pusBufLen ] = '\0';
      dbFieldInfo.atomName = ( char * ) szName;
      AdsGetFieldType( pArea->hTable, szName, &pusType );
      AdsGetFieldLength( pArea->hTable, szName, &u32Length );
      dbFieldInfo.uiLen = ( USHORT ) u32Length;
      dbFieldInfo.uiDec = 0;
      dbFieldInfo.uiFlags = 0;
      if( u32Length > pArea->maxFieldLen )
      {
         pArea->maxFieldLen = u32Length;
      }

      dbFieldInfo.uiTypeExtended = pusType;
      switch( pusType )
      {
         case ADS_STRING:
            dbFieldInfo.uiTypeExtended = 0;
            dbFieldInfo.uiType = HB_FT_STRING;
            break;

         case ADS_RAW:
            dbFieldInfo.uiType = HB_FT_STRING;
            dbFieldInfo.uiFlags = HB_FF_BINARY;
            break;

#ifdef ADS_CISTRING /* Not defined below 7.10 */
         case ADS_CISTRING:
            dbFieldInfo.uiType = HB_FT_STRING;
            break;
#endif

         case ADS_NUMERIC:
            dbFieldInfo.uiTypeExtended = 0;
            dbFieldInfo.uiType = HB_FT_LONG;
            AdsGetFieldDecimals( pArea->hTable, szName, &pusDecimals );
            dbFieldInfo.uiDec = ( USHORT ) pusDecimals;
            break;

         case ADS_DOUBLE:               /* uiLen of extended types is set in following switch */
            dbFieldInfo.uiType = HB_FT_DOUBLE;
            AdsGetFieldDecimals( pArea->hTable, szName, &pusDecimals );
            dbFieldInfo.uiDec = ( USHORT ) pusDecimals;
            break;

         case ADS_CURDOUBLE:
            dbFieldInfo.uiType = HB_FT_CURDOUBLE;
            AdsGetFieldDecimals( pArea->hTable, szName, &pusDecimals );
            dbFieldInfo.uiDec = ( USHORT ) pusDecimals;
            break;

#ifdef ADS_MONEY /* Not defined below 7.00 */
         case ADS_MONEY:
            dbFieldInfo.uiType = HB_FT_CURRENCY;
            AdsGetFieldDecimals( pArea->hTable, szName, &pusDecimals );
            dbFieldInfo.uiDec = ( USHORT ) pusDecimals;
            break;
#endif

         case ADS_INTEGER:
         case ADS_SHORTINT:
            dbFieldInfo.uiType = HB_FT_INTEGER;
            break;

         case ADS_TIME:
            dbFieldInfo.uiType = HB_FT_TIME;
            break;

         case ADS_TIMESTAMP:
            dbFieldInfo.uiType = HB_FT_TIMESTAMP;
            break;

         case ADS_MODTIME:
            dbFieldInfo.uiType = HB_FT_MODTIME;
            break;

         case ADS_AUTOINC:
            dbFieldInfo.uiType = HB_FT_AUTOINC;
            break;

         case ADS_ROWVERSION:
            dbFieldInfo.uiType = HB_FT_ROWVER;
            break;

         case ADS_LOGICAL:
            dbFieldInfo.uiTypeExtended = 0;
            dbFieldInfo.uiType = HB_FT_LOGICAL;
            break;

         case ADS_DATE:
            dbFieldInfo.uiTypeExtended = 0;
         case ADS_COMPACTDATE:
            dbFieldInfo.uiType = HB_FT_DATE;
            break;

#if ADS_LIB_VERSION >= 900
         case ADS_VARCHAR_FOX:
         case ADS_VARBINARY_FOX:
            dbFieldInfo.uiType = HB_FT_VARLENGTH;
            break;
#endif

         case ADS_MEMO:
            dbFieldInfo.uiTypeExtended = 0;
         case ADS_VARCHAR:
            dbFieldInfo.uiType = HB_FT_MEMO;
            break;
         case ADS_BINARY:
            dbFieldInfo.uiType = HB_FT_BLOB;
            dbFieldInfo.uiFlags = HB_FF_BINARY;
            break;
         case ADS_IMAGE:
            dbFieldInfo.uiType = HB_FT_PICTURE;
            dbFieldInfo.uiFlags = HB_FF_BINARY;
            break;
      }

      if( SELF_ADDFIELD( ( AREAP ) pArea, &dbFieldInfo ) == HB_FAILURE )
      {
         return HB_FAILURE;
      }
   }

   if( pArea->maxFieldLen < 24 )
      pArea->maxFieldLen = 24;

   AdsGetRecordLength( pArea->hTable, &u32Length );
   pArea->ulRecordLen = u32Length;
   /* Alloc record buffer - because it's also used for some extended types
      conversion it has to be at least 25 bytes size */
   pArea->pRecord = ( BYTE * ) hb_xgrab( HB_MAX( pArea->ulRecordLen, pArea->maxFieldLen ) + 1 );

   /* If successful call SUPER_OPEN to finish system jobs */
   if( SUPER_OPEN( ( AREAP ) pArea, pOpenInfo ) == HB_FAILURE )
   {
      SELF_CLOSE( ( AREAP ) pArea );
      return HB_FAILURE;
   }

   if( hb_setGetNI( HB_SET_AUTORDER ) )
   {
      DBORDERINFO pOrderInfo;
      pOrderInfo.itmResult = hb_itemPutNI( NULL, 0 );
      pOrderInfo.itmNewVal = NULL;
      pOrderInfo.itmOrder  = hb_itemPutNI( NULL, hb_setGetNI( HB_SET_AUTORDER ) );
      pOrderInfo.atomBagName = NULL;
      SELF_ORDLSTFOCUS( ( AREAP ) pArea, &pOrderInfo );
      hb_itemRelease( pOrderInfo.itmOrder );
      hb_itemRelease( pOrderInfo.itmResult );
   }

   return SELF_GOTOP( ( AREAP ) pArea );
}

#define  adsRelease               NULL

static HB_ERRCODE adsStructSize( ADSAREAP pArea, USHORT * StructSize )
{
   HB_SYMBOL_UNUSED( pArea );

   * StructSize = sizeof( ADSAREA );

   return HB_SUCCESS;
}

static HB_ERRCODE adsSysName( ADSAREAP pArea, BYTE * pBuffer )
{
   UNSIGNED16 u16TableType;
   UNSIGNED32 u32RetVal;

   HB_TRACE(HB_TR_DEBUG, ("adsSysName(%p, %p)", pArea, pBuffer));

   if( pArea->hTable )
   {
      u32RetVal = AdsGetTableType( pArea->hTable, &u16TableType );
      if( u32RetVal != AE_SUCCESS )
      {
         HB_TRACE(HB_TR_DEBUG, ("Error in adsSysName: %lu  pArea->hTable %p\n", ( ULONG ) u32RetVal, ( void * ) ( HB_PTRDIFF ) pArea->hTable));
         u16TableType = ( UNSIGNED16 ) pArea->iFileType;
      }
   }
   else
      u16TableType = ( UNSIGNED16 ) pArea->iFileType;

   switch( u16TableType )
   {
      case ADS_NTX:
         hb_strncpy( ( char * ) pBuffer, "ADSNTX", HB_RDD_MAX_DRIVERNAME_LEN );
         break;
      case ADS_CDX:
         hb_strncpy( ( char * ) pBuffer, "ADSCDX", HB_RDD_MAX_DRIVERNAME_LEN );
         break;
#if ADS_LIB_VERSION >= 900
      case ADS_VFP:
         hb_strncpy( ( char * ) pBuffer, "ADSVFP", HB_RDD_MAX_DRIVERNAME_LEN );
         break;
#endif
      case ADS_ADT:
         hb_strncpy( ( char * ) pBuffer, "ADSADT", HB_RDD_MAX_DRIVERNAME_LEN );
         break;
   }

   return HB_SUCCESS;
}

#define  adsEval                  NULL

static HB_ERRCODE adsPack( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsPack(%p)", pArea));

   if( pArea->fReadonly )
   {
      commonError( pArea, EG_READONLY, EDBF_READONLY, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }
   if( pArea->fShared )
   {
      commonError( pArea, EG_SHARED, EDBF_SHARED, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }

   AdsPackTable( pArea->hTable );

   return SELF_GOTOP( ( AREAP ) pArea );
}

#define  adsPackRec               NULL
#define  adsSort                  NULL
#define  adsTrans                 NULL
#define  adsTransRec              NULL

static HB_ERRCODE adsZap( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsZap(%p)", pArea));

   if( pArea->fReadonly )
   {
      commonError( pArea, EG_READONLY, EDBF_READONLY, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }
   if( pArea->fShared )
   {
      commonError( pArea, EG_SHARED, EDBF_SHARED, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }

   AdsZapTable( pArea->hTable );

   return SELF_GOTOP( ( AREAP ) pArea );
}

static HB_ERRCODE adsChildEnd( ADSAREAP pArea, LPDBRELINFO pRelInfo )
{
   HB_ERRCODE uiError;

   HB_TRACE(HB_TR_DEBUG, ("adsChildEnd(%p, %p)", pArea, pRelInfo));

   if( pArea->lpdbPendingRel == pRelInfo )
      uiError = SELF_FORCEREL( ( AREAP ) pArea );
   else
      uiError = HB_SUCCESS;

   SUPER_CHILDEND( ( AREAP ) pArea, pRelInfo );

   return uiError;
}

static HB_ERRCODE adsChildStart( ADSAREAP pArea, LPDBRELINFO pRelInfo )
{
   HB_TRACE(HB_TR_DEBUG, ("adsChildStart(%p, %p)", pArea, pRelInfo));

   SELF_CHILDSYNC( ( AREAP ) pArea, pRelInfo );

   return SUPER_CHILDSTART( ( AREAP ) pArea, pRelInfo );
}

static HB_ERRCODE adsChildSync( ADSAREAP pArea, LPDBRELINFO pRelInfo )
{
   HB_TRACE(HB_TR_DEBUG, ("adsChildSync(%p, %p)", pArea, pRelInfo));

   pArea->lpdbPendingRel = pRelInfo;

   if( pArea->area.lpdbRelations )
      SELF_SYNCCHILDREN( ( AREAP ) pArea );

   return HB_SUCCESS;
}

#define adsSyncChildren                         NULL

static HB_ERRCODE adsClearRel( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsClearRel(%p)", pArea ));

   SUPER_CLEARREL( ( AREAP ) pArea );
   AdsClearRelation( pArea->hTable );

   return HB_SUCCESS;
}

static HB_ERRCODE adsForceRel( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsForceRel(%p)", pArea));

   if( pArea->lpdbPendingRel )
   {
      LPDBRELINFO lpdbPendingRel;

      lpdbPendingRel = pArea->lpdbPendingRel;
      pArea->lpdbPendingRel = NULL;

      if( ! lpdbPendingRel->isOptimized )
         SELF_RELEVAL( ( AREAP ) pArea, lpdbPendingRel );

      hb_adsUpdateAreaFlags( pArea );
   }
   fflush(stdout);
   return HB_SUCCESS;
}

#define adsRelArea                              NULL
#define adsRelEval                              NULL
#define adsRelText                              NULL

static HB_ERRCODE adsSetRel( ADSAREAP pArea, LPDBRELINFO  lpdbRelations )
{
   UNSIGNED32 ulRetVal = ( UNSIGNED32 ) ~AE_SUCCESS;
   UNSIGNED8 *szExp;
   USHORT rddID;

   HB_TRACE(HB_TR_DEBUG, ("adsSetRel(%p, %p)", pArea, lpdbRelations));

   szExp = ( UNSIGNED8 * ) hb_itemGetCPtr( lpdbRelations->abKey );
   rddID = lpdbRelations->lpaChild->rddID;
   if( *szExp && ( rddID == s_uiRddIdADS ||
                   rddID == s_uiRddIdADT ||
#if ADS_LIB_VERSION >= 900
                   rddID == s_uiRddIdADSVFP ||
#endif
                   rddID == s_uiRddIdADSNTX ||
                   rddID == s_uiRddIdADSCDX ) )
   {
      ADSHANDLE hIndex = ( ( ADSAREAP ) lpdbRelations->lpaChild )->hOrdCurrent;

      if( hIndex )
      {
         if( lpdbRelations->isScoped )
            ulRetVal = AdsSetScopedRelation( pArea->hTable, hIndex, szExp );
         else
            ulRetVal = AdsSetRelation( pArea->hTable, hIndex, szExp );
      }
   }
   lpdbRelations->isOptimized = ( ulRetVal == AE_SUCCESS );

   return SUPER_SETREL( ( AREAP ) pArea, lpdbRelations );
}

static HB_ERRCODE adsOrderListAdd( ADSAREAP pArea, LPDBORDERINFO pOrderInfo )
{
   ADSHANDLE ahIndex[ 50 ];
   UNSIGNED16 u16ArrayLen = 50;
   UNSIGNED32 u32RetVal;

   HB_TRACE(HB_TR_DEBUG, ("adsOrderListAdd(%p, %p)", pArea, pOrderInfo));

   u32RetVal = AdsOpenIndex( pArea->hTable,
                        ( UNSIGNED8 * ) hb_itemGetCPtr( pOrderInfo->atomBagName ),
                        ahIndex, &u16ArrayLen );
   if( u32RetVal != AE_SUCCESS && u32RetVal != AE_INDEX_ALREADY_OPEN )
   {
      /* 1001 and 7008 are standard ADS Open Errors that will usually be sharing issues */
      USHORT uiOsCOde = u32RetVal == 1001 || u32RetVal == 7008 ? 32 : 0;
      commonError( pArea, EG_OPEN, ( USHORT ) u32RetVal, uiOsCOde,
                   hb_itemGetCPtr( pOrderInfo->atomBagName ),
                   EF_CANDEFAULT, NULL );
      return HB_FAILURE;
   }
   if( !pArea->hOrdCurrent && u16ArrayLen > 0 )
   {
      pArea->hOrdCurrent = ahIndex[ 0 ];
      return SELF_GOTOP( ( AREAP ) pArea );
   }

   return HB_SUCCESS;
}

static HB_ERRCODE adsOrderListClear( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsOrderListClear(%p)", pArea));

#if ADS_LIB_VERSION >= 600
   if( !pArea->fReadonly )
      AdsFlushFileBuffers( pArea->hTable );  /* meaningful with local server; ignored by remote server */
#endif

   AdsCloseAllIndexes( pArea->hTable );
   pArea->hOrdCurrent = 0;

   return HB_SUCCESS;
}

static HB_ERRCODE adsOrderListDelete( ADSAREAP pArea, LPDBORDERINFO pOrderInfo )
{
   HB_TRACE(HB_TR_DEBUG, ("adsOrderListDelete(%p, %p)", pArea, pOrderInfo));

   if( hb_itemGetCLen( pOrderInfo->atomBagName ) > 0 )
   {
      ADSHANDLE hIndex;

      hIndex = hb_adsFindBag( pArea, hb_itemGetCPtr( pOrderInfo->atomBagName ) );
      if( hIndex )
      {
         if( AdsCloseIndex( hIndex ) == AE_SUCCESS )
         {
            if( pArea->hOrdCurrent )
            {
               UNSIGNED16 u16Order;
               if( AdsGetIndexOrderByHandle( pArea->hOrdCurrent, &u16Order ) != AE_SUCCESS )
                  pArea->hOrdCurrent = 0;
            }
            return HB_SUCCESS;
         }
      }
   }
   return HB_FAILURE;
}

static HB_ERRCODE adsOrderListFocus( ADSAREAP pArea, LPDBORDERINFO pOrderInfo )
{
   ADSHANDLE hIndex;
   UNSIGNED8 pucTagName[ ADS_MAX_TAG_NAME + 1 ];
   UNSIGNED16 u16Len = ADS_MAX_TAG_NAME + 1, u16Order;
   UNSIGNED32 u32RetVal = AE_SUCCESS;

   HB_TRACE(HB_TR_DEBUG, ("adsOrderListFocus(%p, %p)", pArea, pOrderInfo));

   if( !pArea->hOrdCurrent )
   {
      *pucTagName = '\0';
      u16Len = 0;
   }
   else
      AdsGetIndexName( pArea->hOrdCurrent, pucTagName, &u16Len );

   pOrderInfo->itmResult = hb_itemPutCL( pOrderInfo->itmResult,
                                         ( char * ) pucTagName, u16Len );

   if( pOrderInfo->itmOrder )
   {
      if( HB_IS_STRING( pOrderInfo->itmOrder ) )
      {
         /* ADS can't handle a space-padded string--we have to trim it */
         hb_strncpyUpperTrim( ( char * ) pucTagName,
                              hb_itemGetCPtr( pOrderInfo->itmOrder ),
                              sizeof( pucTagName ) - 1 );
         if( !pucTagName[ 0 ] )
         {
            pArea->hOrdCurrent = 0;
            return HB_SUCCESS;
         }
         u32RetVal = AdsGetIndexHandle( pArea->hTable, pucTagName, &hIndex );
      }
      else if( HB_IS_NUMERIC( pOrderInfo->itmOrder ) )
      {
         u16Order = ( UNSIGNED16 ) hb_itemGetNI( pOrderInfo->itmOrder );
         if( !u16Order )
         {
            pArea->hOrdCurrent = 0;
            return HB_SUCCESS;
         }
         u32RetVal = AdsGetIndexHandleByOrder( pArea->hTable, u16Order, &hIndex );
      }
      else
         hIndex = pArea->hOrdCurrent;

      if( u32RetVal != AE_SUCCESS )
      {
         /* ntx compatibilty: keep current order if failed */
         if( pArea->iFileType == ADS_NTX )
            return HB_SUCCESS;

         pArea->hOrdCurrent = 0;
         return HB_FAILURE;
      }
      pArea->hOrdCurrent = hIndex;
   }
   return HB_SUCCESS;
}

static HB_ERRCODE adsOrderListRebuild( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsOrderListRebuild(%p)", pArea));

   AdsReindex( pArea->hTable );

   return SELF_GOTOP( ( AREAP ) pArea );
}

#define  adsOrderCondition        NULL

static HB_ERRCODE adsOrderCreate( ADSAREAP pArea, LPDBORDERCREATEINFO pOrderInfo )
{
   ADSHANDLE  hIndex;
   ADSHANDLE  hTableOrIndex ;
   UNSIGNED32 u32RetVal;
   UNSIGNED32 u32Options = ADS_DEFAULT;
   PHB_ITEM   pExprItem = pOrderInfo->abExpr;
   UNSIGNED16 u16 = 0;
   UNSIGNED8  pucWhile[ ( ADS_MAX_KEY_LENGTH << 1 ) + 3 ];
   UNSIGNED16 u16Len = ADS_MAX_KEY_LENGTH;
   BOOL fClose = TRUE;

   HB_TRACE(HB_TR_DEBUG, ("adsOrderCreate(%p, %p)", pArea, pOrderInfo));

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( !pArea->area.lpdbOrdCondInfo || ( pArea->area.lpdbOrdCondInfo->fAll &&
                                    !pArea->area.lpdbOrdCondInfo->fAdditive ) )
   {
      SELF_ORDLSTCLEAR( ( AREAP ) pArea );
      fClose = FALSE;
   }
   else if( pArea->area.lpdbOrdCondInfo->fAdditive )
      fClose = FALSE;

   if( !pOrderInfo->abBagName || *(pOrderInfo->abBagName) == '\0' )
      u32Options = ADS_COMPOUND;
   else if( pOrderInfo->atomBagName && *(pOrderInfo->atomBagName) != '\0' )
      u32Options = ADS_COMPOUND;

   pucWhile[ 0 ] = 0;
   if( pArea->area.lpdbOrdCondInfo && pArea->area.lpdbOrdCondInfo->fUseCurrent && pArea->hOrdCurrent )
   {
      UNSIGNED8 pucScope[ ADS_MAX_KEY_LENGTH + 1 ];
      UNSIGNED16 u16BufLen = ADS_MAX_KEY_LENGTH;
      /*
         ADS subIndex does not obey scope, so create a While expression
         from the index key and scope expression if there is one.
      */
      u32RetVal = AdsGetScope( pArea->hOrdCurrent, ADS_BOTTOM, pucScope, &u16BufLen );

      if( u32RetVal == AE_SUCCESS && u16BufLen )
      {
         /* TODO:
            if tag/file exists AND a bagname specifies a non-structural bag, it does not subindex!
            Have to see if it's there already and delete it!  For now, warn users to delete
            secondary bags before creating temp indexes with USECURRENT
         */
         AdsGetKeyType(pArea->hOrdCurrent, &u16);
         AdsGetIndexExpr( pArea->hOrdCurrent, pucWhile, &u16Len);
         pucWhile[ u16Len ] = 0;
         if( u16 == ADS_STRING )     /* add quotation marks around the key */
         {
            hb_strncat( ( char * ) pucWhile, "<=\"", sizeof( pucWhile ) - 1 );
            hb_strncat( ( char * ) pucWhile, ( char * ) pucScope, sizeof( pucWhile ) - 1 );
            hb_strncat( ( char * ) pucWhile, "\"", sizeof( pucWhile ) - 1 );
         }
         else
         {
            hb_strncat( ( char * ) pucWhile, "<=", sizeof( pucWhile ) - 1 );
            hb_strncat( ( char * ) pucWhile, ( char * ) pucScope, sizeof( pucWhile ) - 1 );
         }
      }
      hTableOrIndex = pArea->hOrdCurrent;
   }
   else
      hTableOrIndex = pArea->hTable;

   if( pArea->area.lpdbOrdCondInfo && pArea->area.lpdbOrdCondInfo->abWhile )
   {
      if( pucWhile[ 0 ] )
      {
         hb_strncat( ( char * ) pucWhile, ".AND.(", sizeof( pucWhile ) - 1 );
         hb_strncat( ( char * ) pucWhile, ( char * ) pArea->area.lpdbOrdCondInfo->abWhile, sizeof( pucWhile ) - 1 );
         hb_strncat( ( char * ) pucWhile, ")", sizeof( pucWhile ) - 1 );
      }
      else
         hb_strncat( ( char * ) pucWhile, ( char * ) pArea->area.lpdbOrdCondInfo->abWhile, sizeof( pucWhile ) - 1 );

      if( pArea->hOrdCurrent )
         hTableOrIndex = pArea->hOrdCurrent;
   }

   if( pArea->area.lpdbOrdCondInfo )
   {
      if( pArea->area.lpdbOrdCondInfo->fCustom )
         u32Options |= ADS_CUSTOM;

      if( pArea->area.lpdbOrdCondInfo->fDescending )
         u32Options |= ADS_DESCENDING;
   }

   if( pOrderInfo->fUnique )
      u32Options |= ADS_UNIQUE;

#if ADS_LIB_VERSION >= 610
   u32RetVal = AdsCreateIndex61( hTableOrIndex,
           ( UNSIGNED8 * ) pOrderInfo->abBagName,
           ( UNSIGNED8 * ) pOrderInfo->atomBagName,
           ( UNSIGNED8 * ) hb_itemGetCPtr( pExprItem ),
           ( pArea->area.lpdbOrdCondInfo && pArea->area.lpdbOrdCondInfo->abFor ) ?
           ( UNSIGNED8 * ) pArea->area.lpdbOrdCondInfo->abFor : ( UNSIGNED8 * ) "",
           pucWhile, u32Options, ADS_DEFAULT, &hIndex);
#else
   u32RetVal = AdsCreateIndex( hTableOrIndex, pOrderInfo->abBagName,
           pOrderInfo->atomBagName, ( UNSIGNED8 * ) hb_itemGetCPtr( pExprItem ),
           ( pArea->area.lpdbOrdCondInfo && pArea->area.lpdbOrdCondInfo->abFor ) ?
           ( UNSIGNED8 * ) pArea->area.lpdbOrdCondInfo->abFor : ( UNSIGNED8 * ) "",
           pucWhile, u32Options, &hIndex);
#endif

   SELF_ORDSETCOND( ( AREAP ) pArea, NULL );

   if( u32RetVal != AE_SUCCESS )
   {
      commonError( pArea, EG_CREATE, ( USHORT ) u32RetVal, 0, ( char * ) pOrderInfo->abBagName, 0, NULL );
      return HB_FAILURE;
   }
   else
      pArea->hOrdCurrent = hIndex;

   if( fClose )
   {
      ADSHANDLE ahIndex[ 50 ];
      UNSIGNED16 usArrayLen = 50;

      u32RetVal = AdsOpenIndex( pArea->hTable,
                     ( UNSIGNED8 * ) pOrderInfo->abBagName, ahIndex, &usArrayLen );
      if( u32RetVal != AE_SUCCESS  && u32RetVal != AE_INDEX_ALREADY_OPEN )
      {
         SELF_ORDSETCOND( ( AREAP ) pArea, NULL );
         return HB_FAILURE;
      }
      pArea->hOrdCurrent = usArrayLen ? ahIndex[ 0 ] : 0;
   }

   return SELF_GOTOP( ( AREAP ) pArea );
}

static HB_ERRCODE adsOrderDestroy( ADSAREAP pArea, LPDBORDERINFO pOrderInfo )
{
   ADSHANDLE hIndex;
   UNSIGNED32 u32RetVal;
   HB_TRACE(HB_TR_DEBUG, ("adsOrderDestroy(%p, %p)", pArea, pOrderInfo));

   if( HB_IS_STRING( pOrderInfo->itmOrder ) )
   {
      UNSIGNED8 pucTagName[ ADS_MAX_TAG_NAME + 1 ];

      hb_strncpyUpperTrim( ( char * ) pucTagName,
                           hb_itemGetCPtr( pOrderInfo->itmOrder ),
                           sizeof( pucTagName ) - 1 );
      u32RetVal = AdsGetIndexHandle( pArea->hTable, pucTagName, &hIndex );

      if( u32RetVal != AE_SUCCESS )
         return HB_FAILURE;

      u32RetVal = AdsDeleteIndex( hIndex );

      if( u32RetVal != AE_SUCCESS )
         return HB_FAILURE;

      if( hIndex == pArea->hOrdCurrent )
         pArea->hOrdCurrent = 0;
   }
   else
      return HB_FAILURE;

   return HB_SUCCESS;
}

static HB_ERRCODE adsOrderInfo( ADSAREAP pArea, USHORT uiIndex, LPDBORDERINFO pOrderInfo )
{
   ADSHANDLE  hIndex  = 0;
   UNSIGNED8  aucBuffer[ MAX_STR_LEN + 1 ];
   UNSIGNED16 u16len  = MAX_STR_LEN;
   UNSIGNED16 u16     = 0;
   UNSIGNED32 u32     = 0;

   HB_TRACE(HB_TR_DEBUG, ("adsOrderInfo(%p, %hu, %p)", pArea, uiIndex, pOrderInfo));

   aucBuffer[ 0 ] = 0;

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   /* all others need an index handle */
   if( uiIndex != DBOI_ORDERCOUNT && pOrderInfo->itmOrder && !HB_IS_NIL( pOrderInfo->itmOrder ) )
   {
      UNSIGNED32 u32RetVal = AE_SUCCESS;

      if( HB_IS_STRING( pOrderInfo->itmOrder ) )
      {
         UNSIGNED8 pucTagName[ ADS_MAX_TAG_NAME + 1 ];

         hb_strncpyUpperTrim( ( char * ) pucTagName,
                              hb_itemGetCPtr( pOrderInfo->itmOrder ),
                              sizeof( pucTagName ) - 1 );
         u32RetVal = AdsGetIndexHandle( pArea->hTable, pucTagName, &hIndex );
      }
      else if( HB_IS_NUMERIC( pOrderInfo->itmOrder ) )
      {
         u32RetVal = AdsGetIndexHandleByOrder( pArea->hTable,
               ( UNSIGNED16 ) hb_itemGetNI( pOrderInfo->itmOrder ), &hIndex );
      }

      if( u32RetVal != AE_SUCCESS )
         hIndex = 0;
   }
   else
      hIndex = pArea->hOrdCurrent;

   switch( uiIndex )
   {
      case DBOI_CONDITION:
         if( hIndex && AdsGetIndexCondition( hIndex, aucBuffer, &u16len ) == AE_SUCCESS )
            hb_itemPutCL( pOrderInfo->itmResult, ( char * ) aucBuffer, u16len );
         else
            hb_itemPutC( pOrderInfo->itmResult, NULL );
         break;

      case DBOI_EXPRESSION:
         if( hIndex && AdsGetIndexExpr( hIndex, aucBuffer, &u16len) == AE_SUCCESS )
            hb_itemPutCL( pOrderInfo->itmResult, ( char* ) aucBuffer, u16len );
         else
            hb_itemPutC( pOrderInfo->itmResult, NULL );

         break;

      case DBOI_ISCOND:
         if( hIndex )
            AdsGetIndexCondition( hIndex, aucBuffer, &u16 );
         else
            u16 = 0;
         hb_itemPutL( pOrderInfo->itmResult, u16 != 0 );
         break;

      case DBOI_ISDESC:
         if( hIndex )
         {
            AdsIsIndexDescending( hIndex, &u16 );

            if( pOrderInfo->itmNewVal && HB_IS_LOGICAL( pOrderInfo->itmNewVal ) )
            {
#if ADS_LIB_VERSION >= 900
/*
 ordDescend's 3rd arg (itmNewVal) uses .t. for descending, .f. for ascending.
 BUT AdsSetIndexDirection takes an unusual flag that EITHER takes 0 to return
 to the original state, or TRUE to "reverse the direction from the current state"
 so we have to translate NewVal by comparing to "isDescending".
 TODO: It would be nice to have a value to "return to native state" in xHarbour's syntax
*/
               BOOL bRequestedDescend = hb_itemGetL( pOrderInfo->itmNewVal ) ;
               if( (BOOL) u16 != bRequestedDescend )  // if current direction is not the same as requested direction, flip it
               {
                  AdsSetIndexDirection( hIndex, TRUE );
               }
#else
// TODO: if itmNewVal is passed in unsupported versions, should probably throw an error--but may anger users who have been failing
//  silently for years and still have ads8 or earlier.  Do we have a "Unsupported Option in this version" error?
               hb_errRT_DBCMD( EG_ARG, EDBCMD_DBINFOBADPARAMETER, "Dynamic descend not supported in this version", HB_ERR_FUNCNAME );
#endif
            }
         }
         else
         {
            u16 = 0;
         }
         hb_itemPutL( pOrderInfo->itmResult, u16 != 0 );
         break;

      case DBOI_UNIQUE:
         if( hIndex )
            AdsIsIndexUnique( hIndex, &u16 );
         else
            u16 = 0;
         hb_itemPutL( pOrderInfo->itmResult, u16 != 0 );
         break;

      case DBOI_KEYTYPE:
         if( hIndex )
         {
            AdsGetKeyType( hIndex, &u16 );
            switch( u16 )
            {
               case ADS_STRING:
                  hb_itemPutC( pOrderInfo->itmResult, "C" );
                  break;
               case ADS_NUMERIC:
                  hb_itemPutC( pOrderInfo->itmResult, "N" );
                  break;
               case ADS_DATE:
                  hb_itemPutC( pOrderInfo->itmResult, "D" );
                  break;
               case ADS_LOGICAL:
                  hb_itemPutC( pOrderInfo->itmResult, "L" );
                  break;
               case ADS_RAW:
               default:
                  hb_itemPutC( pOrderInfo->itmResult, "" );
            }
         }
         else
            hb_itemPutC( pOrderInfo->itmResult, "" );

         break;

      case DBOI_KEYSIZE:
         if( hIndex )
            AdsGetKeyLength( hIndex, &u16 );
         else
            u16 = 0;
         hb_itemPutNI( pOrderInfo->itmResult, u16 );
         break;

      case DBOI_KEYVAL:
         if( !pArea->area.fEof && hIndex )
         {
            /* ----------------------------------
             From ads docs: It is important to note that the key generated
             by this function is built on the client, and the key may not
             exist in the index.
             -----------------------------------*/
            AdsExtractKey( hIndex, aucBuffer, &u16len );
            AdsGetKeyType( hIndex, &u16);

            adsGetKeyItem( pArea, pOrderInfo->itmResult, u16,
                           ( char * ) aucBuffer, u16len );
         }
         else
            hb_itemClear( pOrderInfo->itmResult );

         break;

      case DBOI_POSITION :
         if( pOrderInfo->itmNewVal && HB_IS_NUMERIC( pOrderInfo->itmNewVal ) )
         {
            /* TODO: results will be wrong if filter is not valid for ADS server */
            AdsGotoTop( hIndex );
            AdsSkip( hIndex, hb_itemGetNL( pOrderInfo->itmNewVal ) - 1 );
            hb_adsUpdateAreaFlags( pArea );
            /* Force relational movement in child WorkAreas */
            if( pArea->area.lpdbRelations )
               SELF_SYNCCHILDREN( ( AREAP ) pArea );

            pOrderInfo->itmResult = hb_itemPutL( pOrderInfo->itmResult, !pArea->area.fEof );
         }
         else
         {
            if( hIndex )
            {
               UNSIGNED16 usFilterOption = ( pArea->area.dbfi.itmCobExpr ? ADS_RESPECTFILTERS : ADS_RESPECTSCOPES );
               AdsGetKeyNum( hIndex, usFilterOption, &u32);
            }
            else
            {
               UNSIGNED16 usFilterOption = ( pArea->area.dbfi.itmCobExpr ? ADS_RESPECTFILTERS : ADS_IGNOREFILTERS );
               AdsGetRecordNum( pArea->hTable, usFilterOption, &u32);
            }
            /*
               TODO: This count will be wrong if server doesn't know full filter!
            */
            hb_itemPutNL( pOrderInfo->itmResult, u32 );
         }
         break;

      case DBOI_RECNO :                 /* TODO: OR IS THIS JUST RECNO?? */
      case DBOI_KEYNORAW :
         if( hIndex )
         {
            AdsGetKeyNum( hIndex, ADS_RESPECTSCOPES, &u32 );
            hb_itemPutNL( pOrderInfo->itmResult, u32 );
         }
         else
         {
            ULONG ulRecNo;
            SELF_RECNO( ( AREAP ) pArea, &ulRecNo );
            hb_itemPutNL( pOrderInfo->itmResult, ulRecNo );
         }
         break;

      case DBOI_RELKEYPOS:
         if( pOrderInfo->itmNewVal && HB_IS_NUMERIC( pOrderInfo->itmNewVal ) )
            adsSetRelPos( pArea, hIndex, hb_itemGetND( pOrderInfo->itmNewVal ) );
         else
            pOrderInfo->itmResult = hb_itemPutND( pOrderInfo->itmResult,
                                             adsGetRelPos( pArea, hIndex ) );
         break;

      case DBOI_NAME:
         if( hIndex )
            AdsGetIndexName( hIndex, aucBuffer, &u16len );
         else
            u16len = 0;
         hb_itemPutCL( pOrderInfo->itmResult, ( char * ) aucBuffer, u16len );
         break;

      case DBOI_NUMBER :
      {
         UNSIGNED16 usOrder = 0;
         if( hIndex )
            AdsGetIndexOrderByHandle( hIndex, &usOrder );
         else
            usOrder = 0;
         hb_itemPutNI( pOrderInfo->itmResult, usOrder );
         break;
      }

      case DBOI_BAGNAME:
         if( hIndex )
            AdsGetIndexFilename( hIndex, ADS_BASENAME, aucBuffer, &u16len );
         else
            u16len = 0;
         hb_itemPutCL( pOrderInfo->itmResult, ( char * ) aucBuffer, u16len );
         break;

      case DBOI_FULLPATH :
         if( hIndex )
            AdsGetIndexFilename( hIndex, ADS_FULLPATHNAME, aucBuffer, &u16len );
         else
            u16len = 0;
         hb_itemPutCL( pOrderInfo->itmResult, ( char * ) aucBuffer, u16len );
         break;

      case DBOI_BAGEXT:
         hb_itemPutC( pOrderInfo->itmResult, adsIndexExt( pArea->iFileType ) );
         break;

      case DBOI_ORDERCOUNT:
         if( hb_itemGetCLen( pOrderInfo->atomBagName ) > 0 )
         {
            /* if already open, ads fills other info OK.
               TODO: verify it is already open, or be sure to close it!
               (AE_INDEX_ALREADY_OPEN)
            */
            u32 = AdsOpenIndex( pArea->hTable,
                       ( UNSIGNED8 * ) hb_itemGetCPtr( pOrderInfo->atomBagName ),
                       NULL, &u16 );

            if( u32 != AE_INDEX_ALREADY_OPEN )
            {
               /* Close the index if we open new one */
               if( u32 == AE_SUCCESS )
               {
                  ADSHANDLE ahIndex[ 1 ];
                  u16 = 1;
                  if( AdsOpenIndex( pArea->hTable,
                       ( UNSIGNED8 * ) hb_itemGetCPtr( pOrderInfo->atomBagName ),
                       ahIndex, &u16 ) == AE_INDEX_ALREADY_OPEN )
                  {
                     AdsCloseIndex( ahIndex[ 0 ] );
                  }
               }
               u16 = 0;
            }
         }
         else    /* no specific bag requested; get all current indexes */
            AdsGetNumIndexes( pArea->hTable, &u16 );

         hb_itemPutNI( pOrderInfo->itmResult, u16 );
         break;

      case DBOI_KEYCOUNT :
         /*
            TODO: This count will be wrong if server doesn't know full filter!
            TODO: If there are child areas that are not at the top of scope,
                  Skip movement may move them to first related record
         */
         if( hIndex )
         {
            if( pArea->area.dbfi.itmCobExpr )
            {
               AdsGetScope( hIndex, ADS_BOTTOM, aucBuffer, &u16len );

               if( u16len )
               {
                  ULONG ulRecNo;
                  /* Have a scope, so walk it. Need Skips to obey filters with a scope.
                     With ADS_RESPECTFILTERS, ADS will respect both the
                     scope and the filter BUT it will walk all keys in the index
                     to do it!!  This is apparently because Scopes in ADS are implemented in the
                     client -- the server itself is NOT aware of them -- so a combined
                     scope and filter count on the server itself cannot be first limited
                     to the scope and then filtered.
                  */
                  SELF_RECNO( ( AREAP ) pArea, &ulRecNo );
                  AdsGotoTop( hIndex );
                  for( ;; )
                  {
                     AdsAtEOF( pArea->hTable, &u16 );
                     if( u16 )
                        break;
                     u32++;
                     if( AdsSkip( hIndex, 1 ) == AE_NO_CURRENT_RECORD )
                        break;
                  }
                  SELF_GOTO( ( AREAP ) pArea, ulRecNo );
               }
               else  /* no scope set */
                  AdsGetRecordCount( hIndex, ADS_RESPECTFILTERS, &u32 );
            }
            else                        /* no filter set */
               AdsGetRecordCount( hIndex, ADS_RESPECTSCOPES, &u32 );
         }
         else
            AdsGetRecordCount( pArea->hTable, ADS_RESPECTFILTERS, &u32 );

         hb_itemPutNL( pOrderInfo->itmResult, u32 );
         break;

      case DBOI_KEYCOUNTRAW :           /* ignore filter but RESPECT SCOPE */
         AdsGetRecordCount( (hIndex ? hIndex : pArea->hTable), ADS_RESPECTSCOPES, &u32 );
         hb_itemPutNL( pOrderInfo->itmResult, u32 );
         break;

      case DBOI_SCOPETOP:
         if( hIndex )
         {
            if( pOrderInfo->itmResult )
               adsScopeGet( pArea, hIndex, 0, pOrderInfo->itmResult );
            if( pOrderInfo->itmNewVal )
               adsScopeSet( pArea, hIndex, 0, pOrderInfo->itmNewVal );
         }
         else if( pOrderInfo->itmResult )
            hb_itemClear( pOrderInfo->itmResult );
         break;

      case DBOI_SCOPEBOTTOM :
         if( hIndex )
         {
            if( pOrderInfo->itmResult )
               adsScopeGet( pArea, hIndex, 1, pOrderInfo->itmResult );
            if( pOrderInfo->itmNewVal )
               adsScopeSet( pArea, hIndex, 1, pOrderInfo->itmNewVal );
         }
         else if( pOrderInfo->itmResult )
            hb_itemClear( pOrderInfo->itmResult );
         break;

      case DBOI_SCOPESET :
         if( hIndex )
         {
            if( pOrderInfo->itmNewVal )
            {
               adsScopeSet( pArea, hIndex, 0, pOrderInfo->itmNewVal );
               adsScopeSet( pArea, hIndex, 1, pOrderInfo->itmNewVal );
            }
         }
         if( pOrderInfo->itmResult )
            hb_itemClear( pOrderInfo->itmResult );
         break;

      case DBOI_SCOPETOPCLEAR :
         if( hIndex )
         {
            if( pOrderInfo->itmResult )
               adsScopeGet( pArea, hIndex, 0, pOrderInfo->itmResult );
            AdsClearScope( hIndex, ADS_TOP );  /* ADS scopes are 1/2 instead of 0/1 */
         }
         else if( pOrderInfo->itmResult )
            hb_itemClear( pOrderInfo->itmResult );
         break;

      case DBOI_SCOPEBOTTOMCLEAR :
         if( hIndex )
         {
            if( pOrderInfo->itmResult )
               adsScopeGet( pArea, hIndex, 1, pOrderInfo->itmResult );
            AdsClearScope( hIndex, ADS_BOTTOM );
         }
         else if( pOrderInfo->itmResult )
            hb_itemClear( pOrderInfo->itmResult );
         break;

      case DBOI_SCOPECLEAR :
         if( hIndex )
         {
            AdsClearScope( hIndex, ADS_TOP );  /* ADS scopes are 1/2 instead of 0/1 */
            AdsClearScope( hIndex, ADS_BOTTOM );
         }
         if( pOrderInfo->itmResult )
            hb_itemClear( pOrderInfo->itmResult );
         break;

      case DBOI_CUSTOM :
         if( hIndex )
            AdsIsIndexCustom( hIndex, &u16 );
         hb_itemPutL( pOrderInfo->itmResult, u16 != 0 );
         break;

#if ADS_LIB_VERSION >= 900

      case DBOI_SKIPUNIQUE:
         if( hIndex &&
             AdsSkipUnique( hIndex, ( SIGNED32 ) ( pOrderInfo && HB_IS_NUMBER( pOrderInfo->itmNewVal ) ? hb_itemGetNL( pOrderInfo->itmNewVal ) : 1 ) ) == AE_SUCCESS )
            hb_itemPutL( pOrderInfo->itmResult, TRUE );
         break;

#endif

      case DBOI_OPTLEVEL :
         AdsGetAOFOptLevel( pArea->hTable, &u16, NULL, NULL );
         switch( u16 )
         {
            case ADS_OPTIMIZED_FULL:    /* ADS values are different from Harbour */
               hb_itemPutNI( pOrderInfo->itmResult, DBOI_OPTIMIZED_FULL );
               break;
            case ADS_OPTIMIZED_PART:
               hb_itemPutNI( pOrderInfo->itmResult, DBOI_OPTIMIZED_PART );
               break;
            default:
               hb_itemPutNI( pOrderInfo->itmResult, DBOI_OPTIMIZED_NONE );
         }
         break;

      case DBOI_KEYADD :
         hb_itemPutL( pOrderInfo->itmResult,
                      hIndex && AdsAddCustomKey( hIndex ) == AE_SUCCESS );
         break;

      case DBOI_KEYDELETE :
         hb_itemPutL( pOrderInfo->itmResult,
                      hIndex && AdsDeleteCustomKey( hIndex ) == AE_SUCCESS );
         break;


/*
Unsupported TODO:

   DBOI_FILEHANDLE
   DBOI_SETCODEBLOCK
   DBOI_KEYDEC
   DBOI_HPLOCKING
   DBOI_LOCKOFFSET
   DBOI_KEYSINCLUDED
these are really global settings:
   DBOI_STRICTREAD
   DBOI_OPTIMIZE
   DBOI_AUTOORDER
   DBOI_AUTOSHARE
*/

      case DBOI_AUTOOPEN :
         hb_itemPutL( pOrderInfo->itmResult, TRUE );
         /* TODO: Since ADS always opens structural indexes throw some kind of error if caller tries to set to False
            OR be prepared to close indexes (if ADS will allow it) if autoopen is False
         */
         break;

      default:
         return SUPER_ORDINFO( ( AREAP ) pArea, uiIndex, pOrderInfo );
   }
   return HB_SUCCESS;
}

static HB_ERRCODE adsClearFilter( ADSAREAP pArea )
{
   HB_TRACE(HB_TR_DEBUG, ("adsClearFilter(%p)", pArea));
   /*
    We don't know if an AOF was used.
    Since a call to the server would need to be made to see if there's an AOF
    anyway, just always attempt to clear it.
   */
   AdsClearAOF( pArea->hTable );
   AdsClearFilter( pArea->hTable );

   return SUPER_CLEARFILTER( ( AREAP ) pArea );
}

#define  adsClearLocate           NULL
#define  adsClearScope            NULL
#define  adsCountScope            NULL
#define  adsFilterText            NULL
#define  adsScopeInfo             NULL

static HB_ERRCODE adsSetFilter( ADSAREAP pArea, LPDBFILTERINFO pFilterInfo )
{
   HB_TRACE(HB_TR_DEBUG, ("adsSetFilter(%p, %p)", pArea, pFilterInfo));

   /* ----------------- NOTE: ------------------
      See if the server can evaluate the filter.
      If not, don't pass it to the server; let the super level
      filter the records locally.
    --------------------------------------------------*/

   /* must do this first as it calls clearFilter */
   if( SUPER_SETFILTER( ( AREAP ) pArea, pFilterInfo ) == HB_SUCCESS )
   {
      UNSIGNED16 bValidExpr = FALSE;
      UNSIGNED16 usResolve = ADS_RESOLVE_DYNAMIC ;  /*ADS_RESOLVE_IMMEDIATE ;get this from a SETting*/
      UNSIGNED32 u32RetVal = AE_INVALID_EXPRESSION;
      const char * pucFilter = hb_itemGetCPtr( pFilterInfo->abFilterText );

      AdsIsExprValid( pArea->hTable, ( UNSIGNED8 * ) pucFilter, &bValidExpr );

      if( bValidExpr )
      {
         char * szFilter = hb_adsOemToAnsi( pucFilter,
                                 ( ULONG ) hb_itemGetCLen( pFilterInfo->abFilterText ) );

         if( hb_setGetL( HB_SET_OPTIMIZE ) )
            u32RetVal = AdsSetAOF( pArea->hTable, ( UNSIGNED8 * ) szFilter, usResolve );
         else
            u32RetVal = AdsSetFilter( pArea->hTable, ( UNSIGNED8 * ) szFilter );

         hb_adsOemAnsiFree( szFilter );
      }     /* else let SUPER handle filtering */
      pArea->area.dbfi.fOptimized = u32RetVal == AE_SUCCESS;
      return HB_SUCCESS;
   }

   return HB_FAILURE;
}

#define  adsSetLocate             NULL
#define  adsSetScope              NULL
#define  adsSkipScope             NULL
#define  adsLocate                NULL
#define  adsCompile               NULL
#define  adsError                 NULL
#define  adsEvalBlock             NULL

static HB_ERRCODE adsRawLock( ADSAREAP pArea, USHORT uiAction, ULONG ulRecNo )
{
   UNSIGNED32 u32RetVal;
   HB_TRACE(HB_TR_DEBUG, ("adsRawLock(%p, %hu, %lu)", pArea, uiAction, ulRecNo));

   switch( uiAction )
   {
      case REC_LOCK:
         if( !pArea->fShared || pArea->fFLocked )
            return HB_SUCCESS;

         u32RetVal = AdsLockRecord( pArea->hTable, ulRecNo );
         if( u32RetVal != AE_SUCCESS )
            return HB_FAILURE;

         /* Update phantom record status after locking */
         if( !pArea->fPositioned )
         {
            ULONG ulCurRec;
            SELF_RECNO( ( AREAP ) pArea, &ulCurRec );
            SELF_GOTO( ( AREAP ) pArea, ulCurRec );
         }
         break;

      case REC_UNLOCK:
         if( !pArea->fShared || pArea->fFLocked )
            return HB_SUCCESS;

         u32RetVal = AdsUnlockRecord( pArea->hTable, ulRecNo );
         if( u32RetVal != AE_SUCCESS )
            return HB_FAILURE;
         break;

      case FILE_LOCK:
         if( !pArea->fShared || pArea->fFLocked )
            return HB_SUCCESS;

         u32RetVal = AdsLockTable( pArea->hTable );
         if( u32RetVal != AE_SUCCESS )
            return HB_FAILURE;

         pArea->fFLocked = TRUE;
         /* Update phantom record status after locking */
         if( !pArea->fPositioned )
         {
            ULONG ulCurRec;
            SELF_RECNO( ( AREAP ) pArea, &ulCurRec );
            SELF_GOTO( ( AREAP ) pArea, ulCurRec );
         }
         break;

      case FILE_UNLOCK:
         if( !pArea->fShared )
            return TRUE;

         u32RetVal = AdsUnlockTable( pArea->hTable );
         if( u32RetVal == AE_SUCCESS || u32RetVal == AE_TABLE_NOT_LOCKED ||
             u32RetVal == AE_TABLE_NOT_SHARED )
            pArea->fFLocked = FALSE;
         else
            return HB_FAILURE;
         break;
   }
   return HB_SUCCESS;
}

static HB_ERRCODE adsLock( ADSAREAP pArea, LPDBLOCKINFO pLockInfo )
{
   USHORT   uiAction;
   ULONG    ulRecNo;

   HB_TRACE(HB_TR_DEBUG, ("adsLock(%p, %p)", pArea, pLockInfo));

   ulRecNo = ( ULONG ) hb_itemGetNL( pLockInfo->itmRecID );

   switch( pLockInfo->uiMethod )
   {
      case DBLM_EXCLUSIVE :
         if( pArea->fShared && !pArea->fFLocked )
            AdsUnlockTable( pArea->hTable );

         if( !ulRecNo )
            SELF_RECNO( ( AREAP ) pArea, &ulRecNo );

         uiAction = REC_LOCK;
         break;

      case DBLM_MULTIPLE :
         if( !ulRecNo )
            SELF_RECNO( ( AREAP ) pArea, &ulRecNo );

         uiAction = REC_LOCK;
         break;

      case DBLM_FILE :
         uiAction = FILE_LOCK;
         break;

      default  :
         /* This should probably throw a real error... */
         HB_TRACE(HB_TR_INFO, ("adsLock() error in pLockInfo->uiMethod"));
         pLockInfo->fResult = FALSE;
         return HB_FAILURE;
   }

   pLockInfo->fResult = SELF_RAWLOCK( ( AREAP ) pArea, uiAction,
                                      ulRecNo ) == HB_SUCCESS;
   return HB_SUCCESS;
}

static HB_ERRCODE adsUnLock( ADSAREAP pArea, PHB_ITEM pRecNo )
{
   ULONG ulRecNo;

   HB_TRACE(HB_TR_DEBUG, ("adsUnLock(%p, %p)", pArea, pRecNo));

   ulRecNo = hb_itemGetNL( pRecNo );

   return SELF_RAWLOCK( ( AREAP ) pArea,
                        ulRecNo ? REC_UNLOCK : FILE_UNLOCK, ulRecNo );
}

#define  adsCloseMemFile          NULL
#define  adsCreateMemFile         NULL

static HB_ERRCODE adsGetValueFile( ADSAREAP pArea, USHORT uiIndex, const char * szFile, USHORT uiMode )
{
   UNSIGNED32 u32RetVal;

   HB_TRACE(HB_TR_DEBUG, ("adsGetValueFile(%p, %hu, %s, %hu)", pArea, uiIndex, szFile, uiMode));

   HB_SYMBOL_UNUSED( uiMode );

   if( !uiIndex || uiIndex > pArea->area.uiFieldCount )
      return HB_FAILURE;

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( !pArea->fPositioned )
      return HB_SUCCESS;

   u32RetVal = AdsBinaryToFile( pArea->hTable, ADSFIELD( uiIndex ),
                                ( UNSIGNED8* ) szFile );
   if( u32RetVal != AE_SUCCESS )
   {
      /* commonError( pArea, EG_READ, ( USHORT ) u32RetVal, 0, NULL, 0, NULL ); */
      return HB_FAILURE;
   }
   return HB_SUCCESS;
}

#define  adsOpenMemFile           NULL

static HB_ERRCODE adsPutValueFile( ADSAREAP pArea, USHORT uiIndex, const char * szFile, USHORT uiMode )
{
   UNSIGNED32 u32RetVal;

   HB_TRACE(HB_TR_DEBUG, ("adsPutValueFile(%p, %hu, %s, %hu)", pArea, uiIndex, szFile, uiMode));

   if( !uiIndex || uiIndex > pArea->area.uiFieldCount )
      return HB_FAILURE;

   /* resolve any pending relations */
   if( pArea->lpdbPendingRel )
      SELF_FORCEREL( ( AREAP ) pArea );

   if( !pArea->fPositioned )
      return HB_SUCCESS;

   if( hb_ads_bTestRecLocks )
   {
      if( ! hb_adsCheckLock( pArea ) == HB_SUCCESS )
         return HB_FAILURE;
   }

   if( uiMode != ADS_BINARY && uiMode != ADS_IMAGE )
      uiMode = ADS_BINARY;

   u32RetVal = AdsFileToBinary( pArea->hTable, ADSFIELD( uiIndex ), uiMode,
                                ( UNSIGNED8* ) szFile );
   if( u32RetVal != AE_SUCCESS )
   {
      commonError( pArea, EG_WRITE, ( USHORT ) u32RetVal, 0, NULL, 0, NULL );
      return HB_FAILURE;
   }
   return HB_SUCCESS;
}

#define  adsReadDBHeader          NULL
#define  adsWriteDBHeader         NULL


/* TODO: Use AdsDeleteFile() */
static HB_ERRCODE adsDrop( LPRDDNODE pRDD, PHB_ITEM pItemTable, PHB_ITEM pItemIndex, ULONG ulConnect )
{
   char szFileName[ HB_PATH_MAX ];
   const char * szFile;
   const char * szExt;
   PHB_ITEM pFileExt = NULL;
   PHB_FNAME pFileName;
   BOOL fTable = FALSE, fResult = FALSE;

   HB_TRACE(HB_TR_DEBUG, ("adsDrop(%p, %p, %p, %lu)", pRDD, pItemTable, pItemIndex, ulConnect));

   szFile = hb_itemGetCPtr( pItemIndex );
   if( !szFile[ 0 ] )
   {
      /* Try to delete index file */
      szFile = hb_itemGetCPtr( pItemTable );
      if( !szFile[ 0 ] )
         return FALSE;
      fTable = TRUE;
   }

   pFileName = hb_fsFNameSplit( szFile );

   if( !pFileName->szExtension )
   {
      /* Add default extension if missing */
      pFileExt = hb_itemPutC( NULL, "" );
      if( SELF_RDDINFO( pRDD, fTable ? RDDI_TABLEEXT : RDDI_ORDBAGEXT, ulConnect, pFileExt ) == HB_SUCCESS )
         pFileName->szExtension = hb_itemGetCPtr( pFileExt );
   }
   hb_fsFNameMerge( szFileName, pFileName );
   hb_xfree( pFileName );

   /* Use hb_spFile first to locate table which can be in differ path */
   if( hb_spFile( szFileName, szFileName ) )
   {
      fResult = hb_fsDelete( szFileName );
      if( fResult && fTable )
      {
         /*
          * Database table file has been deleted, now check if memo is
          * supported and if yes then try to delete memo file if it exists
          * in the same directory as table file
          * hb_fsFNameSplit() repeated intentionally to respect
          * the path set by hb_spFile()
          */
         pFileName = hb_fsFNameSplit( szFileName );
         pFileExt = hb_itemPutC( pFileExt, "" );
         if( SELF_RDDINFO( pRDD, RDDI_MEMOEXT, ulConnect, pFileExt ) == HB_SUCCESS )
         {
            szExt = hb_itemGetCPtr( pFileExt );
            if( szExt[ 0 ] )
            {
               pFileName->szExtension = szExt;
               hb_fsFNameMerge( szFileName, pFileName );
               hb_fsDelete( szFileName );
            }
         }
         /*
          * and try to delete production index also if it exists
          * in the same directory as table file
          */
         pFileExt = hb_itemPutC( pFileExt, "" );
         if( SELF_RDDINFO( pRDD, RDDI_ORDSTRUCTEXT, ulConnect, pFileExt ) == HB_SUCCESS )
         {
            szExt = hb_itemGetCPtr( pFileExt );
            if( szExt[ 0 ] )
            {
               pFileName->szExtension = szExt;
               hb_fsFNameMerge( szFileName, pFileName );
               hb_fsDelete( szFileName );
            }
         }
         hb_xfree( pFileName );
      }
   }

   if( pFileExt )
      hb_itemRelease( pFileExt );

   return fResult ? HB_SUCCESS : HB_FAILURE;
}

/* TODO: Use AdsCheckExistence()
         UNSIGNED32 ENTRYPOINT AdsCheckExistence( ADSHANDLE    hConnect,
                                                  UNSIGNED8    *pucFileName,
                                                  UNSIGNED16   *pusOnDisk );
*/
static HB_ERRCODE adsExists( LPRDDNODE pRDD, PHB_ITEM pItemTable, PHB_ITEM pItemIndex, ULONG ulConnect )
{
   char szFileName[ HB_PATH_MAX ];
   const char * szFile;
   PHB_ITEM pFileExt = NULL;
   PHB_FNAME pFileName;
   BOOL fTable = FALSE;

   HB_TRACE(HB_TR_DEBUG, ("adsExists(%p, %p, %p, %lu)", pRDD, pItemTable, pItemIndex, ulConnect));

   szFile = hb_itemGetCPtr( pItemIndex );
   if( !szFile[ 0 ] )
   {
      szFile = hb_itemGetCPtr( pItemTable );
      if( !szFile[ 0 ] )
         return FALSE;
      fTable = TRUE;
   }

   pFileName = hb_fsFNameSplit( szFile );

   if( !pFileName->szExtension )
   {
      pFileExt = hb_itemPutC( NULL, "" );
      if( SELF_RDDINFO( pRDD, fTable ? RDDI_TABLEEXT : RDDI_ORDBAGEXT, ulConnect, pFileExt ) == HB_SUCCESS )
         pFileName->szExtension = hb_itemGetCPtr( pFileExt );
   }
   hb_fsFNameMerge( szFileName, pFileName );
   hb_xfree( pFileName );

   if( pFileExt )
      hb_itemRelease( pFileExt );

   return hb_spFile( szFileName, NULL ) ? HB_SUCCESS : HB_FAILURE;
}

static HB_ERRCODE adsRename( LPRDDNODE pRDD, PHB_ITEM pItemTable, PHB_ITEM pItemIndex, PHB_ITEM pNewName, ULONG ulConnect )
{
   HB_SYMBOL_UNUSED( pRDD );
   HB_SYMBOL_UNUSED( pItemTable );
   HB_SYMBOL_UNUSED( pItemIndex );
   HB_SYMBOL_UNUSED( pNewName );
   HB_SYMBOL_UNUSED( ulConnect );

   return HB_FAILURE;
}

#define  adsInit                  NULL

static HB_ERRCODE adsExit( LPRDDNODE pRDD )
{
   HB_SYMBOL_UNUSED( pRDD );

   if( s_uiRddCount )
   {
      if( ! --s_uiRddCount )
      {
         if( s_iSetListenerHandle )
         {
            hb_setListenerRemove( s_iSetListenerHandle );
            s_iSetListenerHandle = 0;
         }
#ifdef __BORLANDC__
   #pragma option push -w-pro
#endif
         AdsApplicationExit();
#ifdef __BORLANDC__
   #pragma option pop
#endif
      }
   }

   /* free pRDD->lpvCargo if necessary */

   return HB_SUCCESS;
}


static HB_ERRCODE adsRddInfo( LPRDDNODE pRDD, USHORT uiIndex, ULONG ulConnect, PHB_ITEM pItem )
{
   HB_TRACE(HB_TR_DEBUG, ("adsRddInfo(%p, %hu, %lu, %p)", pRDD, uiIndex, ulConnect, pItem));

   switch( uiIndex )
   {
      case RDDI_REMOTE:
         hb_itemPutL( pItem, TRUE );
         break;

      case RDDI_CONNECTION:
      {
         ADSHANDLE hOldConnection = hb_ads_hConnect;

         hb_ads_hConnect = HB_ADS_GETCONNECTION( pItem );
         HB_ADS_PUTCONNECTION( pItem, hOldConnection );
         break;
      }
      case RDDI_ISDBF:
         hb_itemPutL( pItem, adsGetFileType( pRDD->rddID ) != ADS_ADT );
         break;

      case RDDI_CANPUTREC:
         hb_itemPutL( pItem, TRUE );
         break;

      case RDDI_TABLEEXT:
         hb_itemPutC( pItem, adsTableExt( adsGetFileType( pRDD->rddID ) ) );
         break;

      case RDDI_MEMOEXT:
         hb_itemPutC( pItem, adsMemoExt( adsGetFileType( pRDD->rddID ) ) );
         break;

      case RDDI_ORDEREXT:
      case RDDI_ORDBAGEXT:
      case RDDI_ORDSTRUCTEXT:
         hb_itemPutC( pItem, adsIndexExt( adsGetFileType( pRDD->rddID ) ) );
         break;

      case RDDI_REFRESHCOUNT:
      {
         BOOL bSet = HB_IS_LOGICAL( pItem );
         BOOL bRefresh = hb_itemGetL( pItem );
         hb_itemPutL( pItem, s_bRefreshCount );
         if( bSet )
            s_bRefreshCount = bRefresh;
         break;
      }

      default:
         return SUPER_RDDINFO( pRDD, uiIndex, ulConnect, pItem );
   }

   return HB_SUCCESS;
}


#define  adsWhoCares              NULL

static const RDDFUNCS adsTable = { ( DBENTRYP_BP ) adsBof,
                                   ( DBENTRYP_BP ) adsEof,
                                   ( DBENTRYP_BP ) adsFound,
                                   ( DBENTRYP_V ) adsGoBottom,
                                   ( DBENTRYP_UL ) adsGoTo,
                                   ( DBENTRYP_I ) adsGoToId,
                                   ( DBENTRYP_V ) adsGoTop,
                                   ( DBENTRYP_BIB ) adsSeek,
                                   ( DBENTRYP_L ) adsSkip,
                                   ( DBENTRYP_L ) adsSkipFilter,
                                   ( DBENTRYP_L ) adsSkipRaw,
                                   ( DBENTRYP_VF ) adsAddField,
                                   ( DBENTRYP_B ) adsAppend,
                                   ( DBENTRYP_I ) adsCreateFields,
                                   ( DBENTRYP_V ) adsDeleteRec,
                                   ( DBENTRYP_BP ) adsDeleted,
                                   ( DBENTRYP_SP ) adsFieldCount,
                                   ( DBENTRYP_VF ) adsFieldDisplay,
                                   ( DBENTRYP_SSI ) adsFieldInfo,
                                   ( DBENTRYP_SCP ) adsFieldName,
                                   ( DBENTRYP_V ) adsFlush,
                                   ( DBENTRYP_PP ) adsGetRec,
                                   ( DBENTRYP_SI ) adsGetValue,
                                   ( DBENTRYP_SVL ) adsGetVarLen,
                                   ( DBENTRYP_V ) adsGoCold,
                                   ( DBENTRYP_V ) adsGoHot,
                                   ( DBENTRYP_P ) adsPutRec,
                                   ( DBENTRYP_SI ) adsPutValue,
                                   ( DBENTRYP_V ) adsRecall,
                                   ( DBENTRYP_ULP ) adsRecCount,
                                   ( DBENTRYP_ISI ) adsRecInfo,
                                   ( DBENTRYP_ULP ) adsRecNo,
                                   ( DBENTRYP_I ) adsRecId,
                                   ( DBENTRYP_S ) adsSetFieldExtent,
                                   ( DBENTRYP_CP ) adsAlias,
                                   ( DBENTRYP_V ) adsClose,
                                   ( DBENTRYP_VO ) adsCreate,
                                   ( DBENTRYP_SI ) adsInfo,
                                   ( DBENTRYP_V ) adsNewArea,
                                   ( DBENTRYP_VO ) adsOpen,
                                   ( DBENTRYP_V ) adsRelease,
                                   ( DBENTRYP_SP ) adsStructSize,
                                   ( DBENTRYP_CP ) adsSysName,
                                   ( DBENTRYP_VEI ) adsEval,
                                   ( DBENTRYP_V ) adsPack,
                                   ( DBENTRYP_LSP ) adsPackRec,
                                   ( DBENTRYP_VS ) adsSort,
                                   ( DBENTRYP_VT ) adsTrans,
                                   ( DBENTRYP_VT ) adsTransRec,
                                   ( DBENTRYP_V ) adsZap,
                                   ( DBENTRYP_VR ) adsChildEnd,
                                   ( DBENTRYP_VR ) adsChildStart,
                                   ( DBENTRYP_VR ) adsChildSync,
                                   ( DBENTRYP_V ) adsSyncChildren,
                                   ( DBENTRYP_V ) adsClearRel,
                                   ( DBENTRYP_V ) adsForceRel,
                                   ( DBENTRYP_SSP ) adsRelArea,
                                   ( DBENTRYP_VR ) adsRelEval,
                                   ( DBENTRYP_SI ) adsRelText,
                                   ( DBENTRYP_VR ) adsSetRel,
                                   ( DBENTRYP_VOI ) adsOrderListAdd,
                                   ( DBENTRYP_V ) adsOrderListClear,
                                   ( DBENTRYP_VOI ) adsOrderListDelete,
                                   ( DBENTRYP_VOI ) adsOrderListFocus,
                                   ( DBENTRYP_V ) adsOrderListRebuild,
                                   ( DBENTRYP_VOO ) adsOrderCondition,
                                   ( DBENTRYP_VOC ) adsOrderCreate,
                                   ( DBENTRYP_VOI ) adsOrderDestroy,
                                   ( DBENTRYP_SVOI ) adsOrderInfo,
                                   ( DBENTRYP_V ) adsClearFilter,
                                   ( DBENTRYP_V ) adsClearLocate,
                                   ( DBENTRYP_V ) adsClearScope,
                                   ( DBENTRYP_VPLP ) adsCountScope,
                                   ( DBENTRYP_I ) adsFilterText,
                                   ( DBENTRYP_SI ) adsScopeInfo,
                                   ( DBENTRYP_VFI ) adsSetFilter,
                                   ( DBENTRYP_VLO ) adsSetLocate,
                                   ( DBENTRYP_VOS ) adsSetScope,
                                   ( DBENTRYP_VPL ) adsSkipScope,
                                   ( DBENTRYP_B ) adsLocate,
                                   ( DBENTRYP_CC ) adsCompile,
                                   ( DBENTRYP_I ) adsError,
                                   ( DBENTRYP_I ) adsEvalBlock,
                                   ( DBENTRYP_VSP ) adsRawLock,
                                   ( DBENTRYP_VL ) adsLock,
                                   ( DBENTRYP_I ) adsUnLock,
                                   ( DBENTRYP_V ) adsCloseMemFile,
                                   ( DBENTRYP_VO ) adsCreateMemFile,
                                   ( DBENTRYP_SCCS ) adsGetValueFile,
                                   ( DBENTRYP_VO ) adsOpenMemFile,
                                   ( DBENTRYP_SCCS ) adsPutValueFile,
                                   ( DBENTRYP_V ) adsReadDBHeader,
                                   ( DBENTRYP_V ) adsWriteDBHeader,
                                   ( DBENTRYP_R ) adsInit,
                                   ( DBENTRYP_R ) adsExit,
                                   ( DBENTRYP_RVVL ) adsDrop,
                                   ( DBENTRYP_RVVL ) adsExists,
                                   ( DBENTRYP_RVVVL ) adsRename,
                                   ( DBENTRYP_RSLV ) adsRddInfo,
                                   ( DBENTRYP_SVP ) adsWhoCares
                                 };

static void adsRegisterRDD( USHORT * pusRddId )
{
   RDDFUNCS * pTable;
   USHORT * uiCount, uiRddId;

   uiCount = ( USHORT * ) hb_parptr( 1 );
   pTable = ( RDDFUNCS * ) hb_parptr( 2 );
   uiRddId = ( USHORT ) hb_parni( 4 );

   if( pTable )
   {
      HB_ERRCODE errCode;

      if( uiCount )
         * uiCount = RDDFUNCSCOUNT;
      errCode = hb_rddInherit( pTable, &adsTable, &adsSuper, NULL );
      if( errCode == HB_SUCCESS )
      {
         /*
          * we successfully register our RDD so now we can initialize it
          * You may think that this place is RDD init statement, Druzus
          */
         *pusRddId = uiRddId;
         ++s_uiRddCount;
         if( !s_iSetListenerHandle )
         {
            adsSetSend();
            s_iSetListenerHandle = hb_setListenerAdd( adsSetListener_callback );
         }
      }
      hb_retni( errCode );
   }
   else
      hb_retni( HB_FAILURE );
}

HB_FUNC_STATIC( ADS_GETFUNCTABLE )
{
   HB_TRACE(HB_TR_DEBUG, ("ADS_GETFUNCTABLE()"));

   adsRegisterRDD( &s_uiRddIdADS );
}

HB_FUNC_STATIC( ADT_GETFUNCTABLE )
{
   HB_TRACE(HB_TR_DEBUG, ("ADT_GETFUNCTABLE()"));

   adsRegisterRDD( &s_uiRddIdADT );
}

HB_FUNC_STATIC( ADSNTX_GETFUNCTABLE )
{
   HB_TRACE(HB_TR_DEBUG, ("ADSNTX_GETFUNCTABLE()"));

   adsRegisterRDD( &s_uiRddIdADSNTX );
}

HB_FUNC_STATIC( ADSCDX_GETFUNCTABLE )
{
   HB_TRACE(HB_TR_DEBUG, ("ADSCDX_GETFUNCTABLE()"));

   adsRegisterRDD( &s_uiRddIdADSCDX );
}

#if ADS_LIB_VERSION >= 900

HB_FUNC_STATIC( ADSVFP_GETFUNCTABLE )
{
   HB_TRACE(HB_TR_DEBUG, ("ADSVFP_GETFUNCTABLE()"));

   adsRegisterRDD( &s_uiRddIdADSVFP );
}

#endif

HB_FUNC( ADS ) { ; }

#define __PRG_SOURCE__ __FILE__

#ifdef HB_PCODE_VER
   #undef HB_PRG_PCODE_VER
   #define HB_PRG_PCODE_VER HB_PCODE_VER
#endif

static void hb_adsRddInit( void * cargo )
{
   HB_SYMBOL_UNUSED( cargo );

   if( hb_rddRegister( "ADS", RDT_FULL ) > 1 ||
       hb_rddRegister( "ADT", RDT_FULL ) > 1 ||
#if ADS_LIB_VERSION >= 900
       hb_rddRegister( "ADSVFP", RDT_FULL ) > 1 ||
#endif
       hb_rddRegister( "ADSCDX", RDT_FULL ) > 1 ||
       hb_rddRegister( "ADSNTX", RDT_FULL ) > 1 )
   {
      hb_errInternal( HB_EI_RDDINVALID, NULL, NULL, NULL );
   }
}

HB_INIT_SYMBOLS_BEGIN( ads1__InitSymbols )
{ "ADS",                 {HB_FS_PUBLIC|HB_FS_LOCAL}, {HB_FUNCNAME( ADS )}, NULL },
{ "ADS_GETFUNCTABLE",    {HB_FS_PUBLIC|HB_FS_LOCAL}, {HB_FUNCNAME( ADS_GETFUNCTABLE )}, NULL },
{ "ADT_GETFUNCTABLE",    {HB_FS_PUBLIC|HB_FS_LOCAL}, {HB_FUNCNAME( ADT_GETFUNCTABLE )}, NULL },
#if ADS_LIB_VERSION >= 900
{ "ADSVFP_GETFUNCTABLE", {HB_FS_PUBLIC|HB_FS_LOCAL}, {HB_FUNCNAME( ADSVFP_GETFUNCTABLE )}, NULL },
#endif
{ "ADSNTX_GETFUNCTABLE", {HB_FS_PUBLIC|HB_FS_LOCAL}, {HB_FUNCNAME( ADSNTX_GETFUNCTABLE )}, NULL },
{ "ADSCDX_GETFUNCTABLE", {HB_FS_PUBLIC|HB_FS_LOCAL}, {HB_FUNCNAME( ADSCDX_GETFUNCTABLE )}, NULL }
HB_INIT_SYMBOLS_END( ads1__InitSymbols )

HB_CALL_ON_STARTUP_BEGIN( _hb_ads_rdd_init_ )
   hb_vmAtInit( hb_adsRddInit, NULL );
HB_CALL_ON_STARTUP_END( _hb_ads_rdd_init_ )

#if defined( HB_PRAGMA_STARTUP )
   #pragma startup ads1__InitSymbols
   #pragma startup _hb_ads_rdd_init_
#elif defined( HB_DATASEG_STARTUP )
   #define HB_DATASEG_BODY    HB_DATASEG_FUNC( ads1__InitSymbols ) \
                              HB_DATASEG_FUNC( _hb_ads_rdd_init_ )
   #include "hbiniseg.h"
#endif

ADSAREAP hb_adsGetWorkAreaPointer( void )
{
   ADSAREAP pArea = ( ADSAREAP ) hb_rddGetCurrentWorkAreaPointer();

   if( pArea )
   {
      USHORT rddID = pArea->area.rddID;

      if( rddID == s_uiRddIdADS ||
          rddID == s_uiRddIdADT ||
#if ADS_LIB_VERSION >= 900
          rddID == s_uiRddIdADSVFP ||
#endif
          rddID == s_uiRddIdADSNTX ||
          rddID == s_uiRddIdADSCDX )
         return pArea;
   }
   return NULL;
}

HB_FUNC( ADSGETRELKEYPOS )
{
   ADSAREAP pArea = hb_adsGetWorkAreaPointer();

   if( pArea )
      hb_retnd( adsGetRelPos( pArea, pArea->hOrdCurrent ) );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, HB_ERR_FUNCNAME );
}

HB_FUNC( ADSSETRELKEYPOS )
{
   ADSAREAP pArea = hb_adsGetWorkAreaPointer();

   if( pArea )
      adsSetRelPos( pArea, pArea->hOrdCurrent, hb_parnd( 1 ) );
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, HB_ERR_FUNCNAME );
}

HB_FUNC( ADSCUSTOMIZEAOF )
{
   ADSAREAP   pArea;
   ULONG      ulRecord = 0;
   UNSIGNED32 u32NumRecs = 0;
   UNSIGNED32 u32RetVal = ( UNSIGNED32 ) ~AE_SUCCESS;   /* initialize to something other than success */
   UNSIGNED16 u16Option = ADS_AOF_ADD_RECORD;
   UNSIGNED32 * pu32Records;

   pArea = hb_adsGetWorkAreaPointer();
   if( pArea )
   {
      if( ISNUM( 2 ) )                  /* add, delete or toggle */
         u16Option = ( UNSIGNED16 ) hb_parni( 2 );

      if( ISNIL( 1 ) )                  /* default to current record */
      {
         u32NumRecs = 1;
         SELF_RECNO( ( AREAP ) pArea, &ulRecord );
      }
      else if( ISNUM( 1 ) )             /* Passed a single recno */
      {
         u32NumRecs = 1;
         ulRecord = hb_parnl( 1 );
      }
      else if( ISARRAY( 1 ) )           /* convert array of recnos to C array */
         u32NumRecs = (UNSIGNED32) hb_parinfa( 1, 0 );

      if( u32NumRecs )
      {
         pu32Records = ( UNSIGNED32 * ) hb_xgrab( u32NumRecs * sizeof( UNSIGNED32 ) );
         if( ISARRAY( 1 ) )           /* convert array of recnos to C array */
         {
            for( ulRecord = 0; ulRecord < u32NumRecs; ulRecord++ )
               pu32Records[ ulRecord ] = hb_parnl( 1, ulRecord + 1 );
         }
         else
            pu32Records[ 0 ] = ulRecord;

         u32RetVal = AdsCustomizeAOF( pArea->hTable, u32NumRecs, pu32Records, u16Option );

         /* I cannot understand what this code should do, Druzus */
#if 0
         /* if server has Customized AOF, clear the super filter so bits won't get flipped off! */
         if( u32RetVal == AE_SUCCESS )
            SUPER_CLEARFILTER( ( AREAP ) pArea );
#endif
         hb_xfree( pu32Records );
      }

      hb_retnl( u32RetVal );
   }
   else
      hb_errRT_DBCMD( EG_NOTABLE, 2001, NULL, HB_ERR_FUNCNAME );
}
