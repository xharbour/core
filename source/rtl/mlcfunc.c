/*
 * $Id: mlcfunc.c 9764 2012-10-16 14:21:59Z andijahja $
 */

/*
 * Harbour Project source code:
 * memo line functions: MEMOLINE(), MLCOUNT(), MLPOS(), MLCTOPOS(), MPOSTOLC()
 *
 * Copyright 2007 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
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

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbset.h"

typedef struct
{
   const char * szEOL;
   HB_SIZE ulLen;
} HB_EOL_INFO, * PHB_EOL_INFO;

static int hb_mlEol( const char * pszString, HB_SIZE ulLen,
                     PHB_EOL_INFO pEOLs, int iEOLs )
{
   int i;

   for( i = 0; i < iEOLs; ++i )
   {
      if( ulLen >= pEOLs[ i ].ulLen &&
          memcmp( pszString, pEOLs[ i ].szEOL, ( size_t ) pEOLs[ i ].ulLen ) == 0 )
         return i;
   }
   return -1;
}

static HB_SIZE hb_mlGetLine( const char * pszString, HB_SIZE ulLen, HB_SIZE ulOffset,
                             HB_SIZE ulLineLength, HB_SIZE ulTabSize, HB_SIZE ulMaxPos,
                             BOOL fWordWrap, PHB_EOL_INFO pEOLs, int iEOLs,
                             HB_SIZE * pulLen, HB_SIZE * pulEOL )
{
   HB_SIZE  ulCol = 0, ulBlankCol = 0, ulBlankPos = 0;
   int      i;

   if( pulEOL )
      *pulEOL = 0;

   while( ulOffset < ulLen && ( ulMaxPos == 0 || ulOffset < ulMaxPos ) )
   {
      if( ( unsigned char ) pszString[ ulOffset ] == HB_CHAR_SOFT1 &&
          pszString[ ulOffset + 1 ] == HB_CHAR_SOFT2 )
      {
         ulOffset += 2;
         if( ! fWordWrap )
            break;
         continue;
      }

      i = hb_mlEol( pszString + ulOffset, ulLen - ulOffset, pEOLs, iEOLs );
      if( i >= 0 )
      {
         if( ulMaxPos )
            ulCol += pEOLs[ i ].ulLen;
         else
            ulOffset += pEOLs[ i ].ulLen;
         if( pulEOL )
            *pulEOL = pEOLs[ i ].ulLen;
         break;
      }

      if( pszString[ ulOffset ] == ' ' || pszString[ ulOffset ] == HB_CHAR_HT )
      {
         ulBlankCol  = ulCol;
         ulBlankPos  = ulOffset;
      }

      if( ulCol >= ulLineLength )
      {
         if( fWordWrap )
         {
            if( ulBlankCol == 0 || pszString[ ulOffset ] == ' ' ||
                pszString[ ulOffset ] == HB_CHAR_HT )
            {
               ulCol = ulLineLength;
               if( pszString[ ulOffset ] == ' ' )
                  ++ulOffset;
               if( ( unsigned char ) pszString[ ulOffset ] == HB_CHAR_SOFT1 &&
                   pszString[ ulOffset + 1 ] == HB_CHAR_SOFT2 )
                  ulOffset += 2;
            }
            else
            {
               ulCol    = ulBlankCol;
               ulOffset = ulBlankPos + 1;
            }
         }
         else
         {
            if( ulCol > ulLineLength )
               --ulOffset;
            ulCol = ulLineLength;
         }
         break;
      }

      ulCol += pszString[ ulOffset ] == HB_CHAR_HT ?
               ulTabSize - ( ulCol % ulTabSize ) : 1;
      ulOffset++;
   }
   *pulLen = ulCol;

   return ulOffset;
}

static PHB_EOL_INFO hb_mlGetEOLs( int iParam, int * piEOLs )
{
   PHB_EOL_INFO   pEOLs = NULL;
   int            iEOLs = 0;

#ifdef HB_EXTENSION
   const char *   szEOL;
   HB_SIZE        ulLen, ul;

   szEOL = hb_parc( iParam );
   if( szEOL )
   {
      ulLen = hb_parclen( iParam );
      if( ulLen )
      {
         pEOLs          = ( PHB_EOL_INFO ) hb_xgrab( sizeof( HB_EOL_INFO ) );
         pEOLs->szEOL   = szEOL;
         pEOLs->ulLen   = ulLen;
         iEOLs          = 1;
      }
   }
   else if( ISARRAY( iParam ) )
   {
      PHB_ITEM pArray   = hb_param( iParam, HB_IT_ARRAY );
      HB_SIZE  ulSize   = hb_arrayLen( pArray );
      for( ul = 1; ul <= ulSize; ++ul )
      {
         if( hb_arrayGetCLen( pArray, ul ) > 0 )
            ++iEOLs;
      }
      if( iEOLs )
      {
         pEOLs = ( PHB_EOL_INFO ) hb_xgrab( sizeof( HB_EOL_INFO ) * iEOLs );
         iEOLs = 0;
         for( ul = 1; ul <= ulSize; ++ul )
         {
            ulLen = hb_arrayGetCLen( pArray, ul );
            if( ulLen > 0 )
            {
               pEOLs[ iEOLs ].szEOL = hb_arrayGetCPtr( pArray, ul );
               pEOLs[ iEOLs ].ulLen = ulLen;
               ++iEOLs;
            }
         }
      }
   }
#else
   HB_SYMBOL_UNUSED( iParam );
#endif

   if( iEOLs == 0 )
   {
      pEOLs          = ( PHB_EOL_INFO ) hb_xgrab( sizeof( HB_EOL_INFO ) );
      pEOLs->szEOL   = hb_setGetEOL();
      if( ! pEOLs->szEOL || ! pEOLs->szEOL[ 0 ] )
         pEOLs->szEOL = hb_conNewLine();
      pEOLs->ulLen   = strlen( pEOLs->szEOL );
      iEOLs          = pEOLs->ulLen ? 1 : 0;
   }

   *piEOLs = iEOLs;
   return pEOLs;
}

static const char * hb_mlGetParams( int iParAdd, HB_SIZE * pulLen,
                                    HB_SIZE * pulLineLength,
                                    HB_SIZE * pulTabSize, BOOL * pfWordWrap,
                                    PHB_EOL_INFO * pEOLs, int * piEOLs )
{
   const char * pszString = hb_parc( 1 );

   if( pszString )
   {
      if( ISNUM( 2 ) )
      {
         if( hb_parnd( 2 ) <= 0 )
            return NULL;
         *pulLineLength = hb_parnl( 2 );
      }
      else
         *pulLineLength = 79;
      *pulLen     = hb_parclen( 1 );
      *pulTabSize = ISNUM( 3 + iParAdd ) ? hb_parnl( 3 + iParAdd ) : 4;
      *pfWordWrap = ISLOG( 4 + iParAdd ) ? hb_parl( 4 + iParAdd ) : TRUE;
      *pEOLs      = hb_mlGetEOLs( 5 + iParAdd, piEOLs );
#ifdef HB_C52_STRICT
      if( *pulLineLength > 254 )
         *pulLineLength = 79;
#endif
      if( *pulTabSize >= *pulLineLength )
         *pulTabSize = *pulLineLength - 1;
      else if( *pulTabSize == 0 )
         *pulTabSize = 1;
   }
   return pszString;
}

HB_FUNC( MEMOLINE )
{
   HB_SIZE        ulLen, ulLineLength, ulTabSize;
   BOOL           fWordWrap;
   PHB_EOL_INFO   pEOLs;
   int            iEOLs;
   const char *   pszString = hb_mlGetParams( 1, &ulLen, &ulLineLength,
                                              &ulTabSize, &fWordWrap,
                                              &pEOLs, &iEOLs );
   char *         szLine;
   ULONG          ulLine   = hb_parnl( 3 );
   HB_SIZE        ulEnd, ulOffset = ISNUM( 7 ) ? hb_parnl( 7 ) - 1 : 0;
   HB_SIZE        ulCols   = 0;

   if( ! pszString )
   {
      hb_retc( NULL );
      return;
   }

   if( ulLine == 0 )
   {
      ulLine = 1;
   }
   while( --ulLine && ulOffset < ulLen )
   {
      ulOffset = hb_mlGetLine( pszString, ulLen, ulOffset,
                               ulLineLength, ulTabSize, 0, fWordWrap,
                               pEOLs, iEOLs, &ulCols, NULL );
   }

   ulEnd = ulOffset;

   if( ulOffset < ulLen )
   {
      ULONG ulCol = 0;
      ulEnd    = hb_mlGetLine( pszString, ulLen, ulOffset,
                               ulLineLength, ulTabSize, 0, fWordWrap,
                               pEOLs, iEOLs, &ulCols, NULL );
      szLine   = ( char * ) hb_xgrab( ulLineLength + 1 );

      while( ulCol < ulCols )
      {
         if( pszString[ ulOffset ] == HB_CHAR_HT )
         {
            HB_SIZE ul = ulTabSize - ( ulCol % ulTabSize );
            do
            {
               szLine[ ulCol++ ] = ' ';
            }
            while( --ul && ulCol < ulCols );
         }
         else if( ( unsigned char ) pszString[ ulOffset ] == HB_CHAR_SOFT1 &&
                  pszString[ ulOffset + 1 ] == HB_CHAR_SOFT2 )
         {
            ulOffset++;
         }
         else
         {
            szLine[ ulCol++ ] = pszString[ ulOffset ];
         }
         ulOffset++;
      }
      if( ulCols < ulLineLength )
      {
         memset( szLine + ulCols, ' ', ( size_t ) ( ulLineLength - ulCols ) );
      }
      szLine[ ulLineLength ] = 0;

      hb_retclen_buffer( szLine, ulLineLength );
   }
   else
   {
      hb_retc( NULL );
   }
   hb_xfree( pEOLs );

   if( ISBYREF( 7 ) )
   {
      hb_stornl( ( LONG ) ulEnd + 1, 7 );
   }

}

HB_FUNC( MLCOUNT )
{
   HB_SIZE        ulLen, ulLineLength, ulTabSize;
   BOOL           fWordWrap;
   PHB_EOL_INFO   pEOLs;
   int            iEOLs;
   const char *   pszString   = hb_mlGetParams( 0, &ulLen, &ulLineLength,
                                                &ulTabSize, &fWordWrap,
                                                &pEOLs, &iEOLs );
   HB_SIZE        ulLines     = 0;
   HB_SIZE        ulOffset    = 0;
   HB_SIZE        ulCols      = 0;

   if( pszString )
   {
      while( ulOffset < ulLen )
      {
         ++ulLines;
         ulOffset = hb_mlGetLine( pszString, ulLen, ulOffset,
                                  ulLineLength, ulTabSize, 0, fWordWrap,
                                  pEOLs, iEOLs, &ulCols, NULL );
      }
      hb_xfree( pEOLs );
   }
   hb_retnl( ( LONG ) ulLines );
}

HB_FUNC( MLPOS )
{
   HB_SIZE        ulLen, ulLineLength, ulTabSize;
   BOOL           fWordWrap;
   PHB_EOL_INFO   pEOLs;
   int            iEOLs;
   const char *   pszString   = hb_mlGetParams( 1, &ulLen, &ulLineLength,
                                                &ulTabSize, &fWordWrap,
                                                &pEOLs, &iEOLs );
   HB_SIZE        ulLine      = hb_parnl( 3 );
   HB_SIZE        ulOffset    = 0;
   HB_SIZE        ulCols      = 0;

   if( pszString )
   {
      if( ulLine == 0 )
         ulLine = 1;
      while( --ulLine && ulOffset < ulLen )
         ulOffset = hb_mlGetLine( pszString, ulLen, ulOffset,
                                  ulLineLength, ulTabSize, 0, fWordWrap,
                                  pEOLs, iEOLs, &ulCols, NULL );
      if( ulOffset < ulLen )
         ++ulOffset;
      hb_xfree( pEOLs );
   }
   hb_retnl( ( LONG ) ulOffset );
}

HB_FUNC( MLCTOPOS )
{
   HB_SIZE        ulLen, ulLineLength, ulTabSize;
   BOOL           fWordWrap;
   PHB_EOL_INFO   pEOLs;
   int            iEOLs;
   const char *   pszString   = hb_mlGetParams( 2, &ulLen, &ulLineLength,
                                                &ulTabSize, &fWordWrap,
                                                &pEOLs, &iEOLs );
   HB_SIZE        ulLine      = hb_parnl( 3 );
   HB_SIZE        ulCol       = hb_parnl( 4 );
   HB_SIZE        ulOffset    = 0;
   HB_SIZE        ulCols      = 0;

   if( pszString )
   {
      if( ulLineLength > 4 && ulLine && ISNUM( 4 ) )
      {
         while( --ulLine && ulOffset < ulLen )
            ulOffset = hb_mlGetLine( pszString, ulLen, ulOffset,
                                     ulLineLength, ulTabSize, 0, fWordWrap,
                                     pEOLs, iEOLs, &ulCols, NULL );
         if( ulOffset < ulLen && ulCol )
            ulOffset = hb_mlGetLine( pszString, ulLen, ulOffset,
                                     ulCol, ulTabSize, ulLen, FALSE,
                                     pEOLs, iEOLs, &ulCols, NULL );
      }
      hb_xfree( pEOLs );
   }
   ++ulOffset;
   hb_retnl( ( LONG ) ulOffset );
}

HB_FUNC( MPOSTOLC )
{
   HB_SIZE        ulLen, ulLineLength, ulTabSize;
   BOOL           fWordWrap;
   PHB_EOL_INFO   pEOLs;
   int            iEOLs;
   const char *   pszString   = hb_mlGetParams( 1, &ulLen, &ulLineLength,
                                                &ulTabSize, &fWordWrap,
                                                &pEOLs, &iEOLs );
   HB_SIZE        ulPos       = hb_parnl( 3 );
   HB_SIZE        ulOffset    = 0;
   HB_SIZE        ulLine      = 0;
   HB_SIZE        ulCol       = 0;
   HB_SIZE        ulEOL       = 0;

   if( pszString )
   {
      if( ulPos && ulLen )
      {
         if( --ulPos )
         {
            do
            {
               ++ulLine;
               ulOffset = hb_mlGetLine( pszString, ulLen, ulOffset,
                                        ulLineLength, ulTabSize, ulPos, fWordWrap,
                                        pEOLs, iEOLs, &ulCol, &ulEOL );
               if( ulEOL )
               {
                  if( ulOffset + ulEOL == ulPos )
                  {
                     ulCol = 0;
                     ++ulLine;
                     break;
                  }
                  ulOffset += ulEOL;
               }
            }
            while( ulOffset < ulLen && ulOffset < ulPos );

            if( ulLine && ulCol == ulLineLength && ulPos <= ulLen &&
                ( hb_mlEol( pszString + ulPos, ulLen - ulPos, pEOLs, iEOLs ) >= 0 ||
                  ( ( unsigned char ) pszString[ ulPos ] == HB_CHAR_SOFT1 &&
                    pszString[ ulPos + 1 ] == HB_CHAR_SOFT2 ) ||
                  ( ulPos > 0 && ( unsigned char ) pszString[ ulPos - 1 ] == HB_CHAR_SOFT1 &&
                    pszString[ ulPos ] == HB_CHAR_SOFT2 ) ||
                  ( ulPos > 1 && ( unsigned char ) pszString[ ulPos - 2 ] == HB_CHAR_SOFT1 &&
                    pszString[ ulPos - 1 ] == HB_CHAR_SOFT2 ) ) )
            {
               ulCol = 0;
               ++ulLine;
            }
         }
         else
            ++ulLine;
      }
      hb_xfree( pEOLs );
   }
   hb_reta( 2 );
   hb_stornl( ( LONG ) ulLine, -1, 1 );
   hb_stornl( ( LONG ) ulCol, -1, 2 );
}
