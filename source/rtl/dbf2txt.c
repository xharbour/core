/*
 * $Id: dbf2txt.c 9759 2012-10-15 14:02:59Z andijahja $
 */

/*
 * xHarbour Project source code:
 * DBF2TXT() function
 *
 * Copyright 1999 Andi Jahja <andij@aonlippo.co.id>
 * www - http://www.xharbour.org
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

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapirdd.h"
#include "hbapierr.h"
#include "hbapicdp.h"
#include "hbapifs.h"
#include "hbvm.h"

/* Escaping delimited strings. Need to be cleaned/optimized/improved */
static char * hb_strescape( const char * szInput, int lLen, const char * cDelim )
{
   int            lCnt = 0;
   const char *   szChr;
   char *         szEscape;
   char *         szReturn;

   szReturn = szEscape = ( char * ) hb_xgrab( lLen * 2 + 4 );

   while( lLen && HB_ISSPACE( szInput[ lLen - 1 ] ) )
   {
      lLen--;
   }

   szChr = szInput;

   while( *szChr && lCnt++ < lLen )
   {
      if( *szChr == *cDelim )
      {
         *szEscape++ = '\\';
      }
      *szEscape++ = *szChr++;
   }
   *szEscape = '\0';

   return szReturn;
}

/* Export field values to text file */
#ifndef HB_CDP_SUPPORT_OFF
static BOOL hb_ExportVar( HB_FHANDLE handle, PHB_ITEM pValue, const char * cDelim, PHB_CODEPAGE cdp )
#else
static BOOL hb_ExportVar( HB_FHANDLE handle, PHB_ITEM pValue, const char * cDelim )
#endif
{
   switch( hb_itemType( pValue ) )
   {
      /* a "C" field */
      case HB_IT_STRING:
      {
         char *   szStrEsc;
         char *   szString;

         szStrEsc = hb_strescape( hb_itemGetCPtr( pValue ),
                                  ( int ) hb_itemGetCLen( pValue ), cDelim );
#ifndef HB_CDP_SUPPORT_OFF
         if( cdp )
         {
            hb_cdpnTranslate( szStrEsc, hb_cdppage(), cdp, strlen( szStrEsc ) );
         }
#endif
         szString = hb_xstrcpy( NULL, cDelim, szStrEsc, cDelim, NULL );

         /* FWrite( handle, szString ) */
         hb_fsWriteLarge( handle, ( BYTE * ) szString, strlen( szString ) );

         /* Orphaned, get rif off it */
         hb_xfree( szStrEsc );
         hb_xfree( szString );
         break;
      }
      /* a "D" field */
      case HB_IT_DATE:
      {
         char * szDate = ( char * ) hb_xgrab( 9 );

         hb_itemGetDS( pValue, szDate );
         hb_fsWriteLarge( handle, ( BYTE * ) szDate, strlen( szDate ) );
         hb_xfree( szDate );
         break;
      }
      /* an "L" field */
      case HB_IT_LOGICAL:
         hb_fsWriteLarge( handle, ( BYTE * ) ( hb_itemGetL( pValue ) ? "T" : "F" ), 1 );
         break;
      /* an "N" field */
      case HB_IT_INTEGER:
      case HB_IT_LONG:
      case HB_IT_DOUBLE:
      {
         char * szResult = hb_itemStr( pValue, NULL, NULL );

         if( szResult )
         {
            HB_SIZE        ulLen       = strlen( szResult );
            const char *   szTrimmed   = hb_strLTrim( szResult, &ulLen );

            hb_fsWriteLarge( handle, ( BYTE * ) szTrimmed, strlen( szTrimmed ) );
            hb_xfree( szResult );
         }
         break;
      }
      /* an "M" field or the other, might be a "V" in SixDriver */
      default:
         /* We do not want MEMO contents */
         return FALSE;
   }
   return TRUE;
}

HB_FUNC( DBF2TEXT )
{
   PHB_ITEM       pWhile   = hb_param( 1, HB_IT_BLOCK );
   PHB_ITEM       pFor     = hb_param( 2, HB_IT_BLOCK );
   PHB_ITEM       pFields  = hb_param( 3, HB_IT_ARRAY );

   const char *   cDelim   = hb_parc( 4 );
   HB_FHANDLE     handle   = ( HB_FHANDLE ) hb_parnint( 5 );
   const char *   cSep     = hb_parc( 6 );
   int            nCount   = hb_parni( 7 );

#ifndef HB_CDP_SUPPORT_OFF
   PHB_CODEPAGE   cdp      = hb_cdpFind( ( char * ) hb_parc( 8 ) );
#endif

   AREAP          pArea    = ( AREAP ) hb_rddGetCurrentWorkAreaPointer();

   /* Export DBF content to text file */

   int      iSepLen;
   USHORT   uiFields       = 0;
   USHORT   ui;
   PHB_ITEM pTmp;
   BOOL     bWriteSep      = FALSE;

   BOOL     bEof           = TRUE;
   BOOL     bBof           = TRUE;

   BOOL     bNoFieldPassed = ( pFields == NULL || hb_arrayLen( pFields ) == 0 );

   if( ! pArea )
   {
      hb_errRT_DBCMD( EG_NOTABLE, EDBCMD_NOTABLE, NULL, "COPY TO" );
      return;
   }

   if( ! handle )
   {
      hb_errRT_DBCMD( EG_ARG, EDBCMD_EVAL_BADPARAMETER, NULL, "DBF2TEXT" );
      return;
   }

   if( cdp && cdp == hb_cdppage() )
   {
      cdp = NULL;
   }

   pTmp = hb_itemNew( NULL );

   if( ! cDelim )
   {
      cDelim = "\"";
   }

   if( cSep )
      iSepLen = ( int ) hb_parclen( 6 );
   else
   {
      cSep     = ",";
      iSepLen  = 1;
   }

   SELF_FIELDCOUNT( pArea, &uiFields );

   while( ( nCount == -1 || nCount > 0 ) &&
          ( ! pWhile || hb_itemGetL( hb_vmEvalBlock( pWhile ) ) ) )
   {
      /* While !BOF() .AND. !EOF() */
      SELF_EOF( pArea, &bEof );
      SELF_BOF( pArea, &bBof );

      if( bEof || bBof )
      {
         break;
      }

      /* For condition is met */
      /* if For is NULL, hb__Eval returns TRUE */
      if( ! pFor || hb_itemGetL( hb_vmEvalBlock( pFor ) ) )
      {
         /* User does not request fields, copy all fields */
         if( bNoFieldPassed )
         {
            for( ui = 1; ui <= uiFields; ui++ )
            {
               if( bWriteSep )
               {
                  hb_fsWriteLarge( handle, ( BYTE * ) cSep, iSepLen );
               }

               SELF_GETVALUE( pArea, ui, pTmp );
#ifndef HB_CDP_SUPPORT_OFF
               bWriteSep   = hb_ExportVar( handle, pTmp, cDelim, cdp );
#else
               bWriteSep   = hb_ExportVar( handle, pTmp, cDelim );
#endif
               hb_itemClear( pTmp );
            }
         }
         /* Only requested fields are exported here */
         else
         {
            USHORT   uiFieldCopy = ( USHORT ) hb_arrayLen( pFields );
            USHORT   uiItter;

            for( uiItter = 1; uiItter <= uiFieldCopy; uiItter++ )
            {
               const char * szFieldName = hb_arrayGetCPtr( pFields, uiItter );
               if( szFieldName )
               {
                  int iPos = hb_rddFieldIndex( pArea, szFieldName );

                  if( iPos )
                  {
                     if( bWriteSep )
                     {
                        hb_fsWriteLarge( handle, ( BYTE * ) cSep, iSepLen );
                     }
                     SELF_GETVALUE( pArea, ( USHORT ) iPos, pTmp );
#ifndef HB_CDP_SUPPORT_OFF
                     bWriteSep   = hb_ExportVar( handle, pTmp, cDelim, cdp );
#else
                     bWriteSep   = hb_ExportVar( handle, pTmp, cDelim );
#endif
                     hb_itemClear( pTmp );
                  }
               }
            }
         }
         hb_fsWriteLarge( handle, ( BYTE * ) "\r\n", 2 );
         bWriteSep = FALSE;
      }

      if( nCount != -1 )
      {
         nCount--;
      }

      /* DBSKIP() */
      SELF_SKIP( pArea, 1 );
   }

   /* Writing EOF */
   hb_fsWriteLarge( handle, ( BYTE * ) "\x1A", 1 );
   hb_itemRelease( pTmp );
}
