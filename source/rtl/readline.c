/*
 * $Id: readline.c 9759 2012-10-15 14:02:59Z andijahja $
 */

/*
 * xHarbour Project source code:
 * Text file reading functions
 *
 * Copyright 2003 Marcelo Lombardo - lombardo@uol.com.br
 * http://www.xharbour.org
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

#include <ctype.h>

#include "hbapi.h"
#include "hbapifs.h"
#include "hb_io.h"
#include "hbset.h"
#include "hbapiitm.h"
#include "hbapierr.h"

#define READING_BLOCK 4096

BYTE * hb_fsReadLine( FHANDLE hFileHandle, HB_SIZE * plBuffLen, char ** Term, int * iTermSizes, USHORT iTerms, BOOL * bFound, BOOL * bEOF  )
{
   USHORT   uiPosTerm   = 0, uiPosition;
   HB_SIZE  iPos;
   USHORT   nTries;
   HB_SIZE  lRead       = 0;
   HB_SIZE  lOffset, lSize;
   BYTE *   pBuff;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsReadLine(%p, %ld, %p, %p, %hu, %i, %i)", hFileHandle, *plBuffLen, Term, iTermSizes, iTerms, *bFound, *bEOF ) );

   *bFound  = 0;
   *bEOF    = 0;
   nTries   = 0;
   lOffset  = 0;
   lSize    = *plBuffLen;

   if( *plBuffLen < 10 )
   {
      *plBuffLen = READING_BLOCK;
   }

   pBuff = ( BYTE * ) hb_xgrab( *plBuffLen );

   do
   {
      if( nTries > 0 )
      {
         /* pBuff can be enlarged to hold the line as needed.. */
         lSize    = ( *plBuffLen * ( nTries + 1 ) ) + 1;
         pBuff    = ( BYTE * ) hb_xrealloc( pBuff, lSize );
         lOffset  += lRead;
      }

      /* read from file */
      lRead = hb_fsReadLarge( hFileHandle, pBuff + lOffset, lSize - lOffset );

      /* scan the read buffer */

      if( lRead > 0 )
      {
         for( iPos = 0; iPos < lRead; iPos++ )
         {
            for( uiPosTerm = 0; uiPosTerm < iTerms; uiPosTerm++ )
            {
               /* Compare with the LAST terminator byte */
               if( pBuff[ lOffset + iPos ] == Term[ uiPosTerm ][ iTermSizes[ uiPosTerm ] - 1 ] && ( iTermSizes[ uiPosTerm ] - 1 ) <= ( USHORT ) ( iPos + lOffset ) )
               {
                  *bFound = 1;

                  for( uiPosition = 0; uiPosition < ( iTermSizes[ uiPosTerm ] - 1 ); uiPosition++ )
                  {
                     if( Term[ uiPosTerm ][ uiPosition ] != pBuff[ lOffset + ( iPos - iTermSizes[ uiPosTerm ] ) + uiPosition + 1 ] )
                     {
                        *bFound = 0;
                        break;
                     }
                  }

                  if( *bFound )
                  {
                     break;
                  }
               }
            }

            if( *bFound )
            {
               break;
            }
         }

         if( *bFound )
         {
            *plBuffLen           = lOffset + iPos - iTermSizes[ uiPosTerm ] + 1;

            pBuff[ *plBuffLen ]  = '\0';

            /* Set handle pointer in the end of the line */
            hb_fsSeek( hFileHandle, ( LONG ) ( ( ( lRead - iPos ) * -1 ) + 1 ), FS_RELATIVE );

            return pBuff;
         }
      }
      else
      {
         if( ! *bFound )
         {
            if( nTries == 0 )
            {
               pBuff[ 0 ]  = '\0';
               *plBuffLen  = 0;
            }
            else
            {
               pBuff[ lOffset + lRead ]   = '\0';
               *plBuffLen                 = lOffset + lRead;
            }

            *bEOF = 1;
         }
      }

      nTries++;
   }
   while( ( ! *bFound ) && lRead > 0 );

   return pBuff;
}

/* PRG level fReadLine( <Handle>, <@buffer>, [<aTerminators | cTerminator>], [<nReadingBlock>] ) */

HB_FUNC( HB_FREADLINE )
{
   PHB_ITEM pTerm1;
   HB_ITEM  Opt;
   FHANDLE  hFileHandle = ( FHANDLE ) hb_parnl( 1 );
   char **  Term;
   BYTE *   pBuffer;
   int *    iTermSizes;
   HB_SIZE  lSize = hb_parnl( 4 );
   USHORT   i, iTerms;
   BOOL     bFound, bEOF;

   if( ( ! ISBYREF( 2 ) ) || ( ! ISNUM( 1 ) ) )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "FREADLINE", 4, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
      return;
   }

   if( ISARRAY( 3 ) || ISCHAR( 3 ) )
   {
      if( ISARRAY( 3 ) )
      {
         pTerm1   = hb_param( 3, HB_IT_ARRAY );
         iTerms   = ( int ) ( USHORT ) pTerm1->item.asArray.value->ulLen;

         if( iTerms <= 0 )
         {
            hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "FREADLINE", 4,
                                  hb_paramError( 1 ), hb_paramError( 2 ),
                                  hb_paramError( 3 ), hb_paramError( 4 ) );
            return;
         }

         Term        = ( char ** ) hb_xgrab( sizeof( char * ) * iTerms );
         iTermSizes  = ( int * ) hb_xgrab( sizeof( int ) * iTerms );
         Opt.type    = HB_IT_NIL;

         for( i = 0; i < iTerms; i++ )
         {
            hb_arrayGet( pTerm1, i + 1, &Opt );
            Term[ i ]         = ( char * ) ( &Opt )->item.asString.value;
            iTermSizes[ i ]   = ( int ) ( &Opt )->item.asString.length;
         }
      }
      else
      {
         pTerm1            = hb_param( 3, HB_IT_STRING );
         Term              = ( char ** ) hb_xgrab( sizeof( char * ) );
         iTermSizes        = ( int * ) hb_xgrab( sizeof( int ) );
         Term[ 0 ]         = ( char * ) pTerm1->item.asString.value;
         iTermSizes[ 0 ]   = ( int ) pTerm1->item.asString.length;
         iTerms            = 1;
      }
   }
   else
   {
      Term              = ( char ** ) hb_xgrab( sizeof( char * ) );
      iTermSizes        = ( int * ) hb_xgrab( sizeof( int ) );
      Term[ 0 ]         = ( char * ) "\r\n"; /* Should be preplaced with the default EOL sequence */
      iTerms            = 1;
      iTermSizes[ 0 ]   = 2;
   }

   if( lSize == 0 )
   {
      lSize = READING_BLOCK;
   }

   pBuffer = hb_fsReadLine( hFileHandle, &lSize, Term, iTermSizes, iTerms, &bFound, &bEOF  );

   hb_storclenAdopt( ( char * ) pBuffer, lSize, 2 );
   hb_retnl( bEOF ? -1 : 0 );
   hb_xfree( Term );
   hb_xfree( iTermSizes );
}
