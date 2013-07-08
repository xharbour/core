/*
 * $Id: tab.c 9785 2012-10-24 22:21:03Z andijahja $
 */

/*
 * Harbour Project source code:
 *   TABEXPAND(), TABPACK() CT3 string functions
 *
 * Copyright 2002 IntTec GmbH, Neunlindenstr 32, 79106 Freiburg, Germany
 *        Author: Martin Vogel <vogel@inttec.de>
 *
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

#include "ct.h"

HB_FUNC( TABEXPAND )
{
   if( ISCHAR( 1 ) )
   {
      char *   pcString    = ( char * ) hb_parc( 1 );
      HB_SIZE  sStrLen     = hb_parclen( 1 );
      unsigned char *   pcRet;
      HB_SIZE  sRetLen;
      HB_SIZE  sTabWidth   = 0;
      char     cFill, cTab, cCR;
      char *   pcNewLine;
      HB_SIZE  sNewLineLen;
      int      iIgnore141;

      HB_SIZE  sIndex, sLineIndex;
      HB_SIZE  sTabCnt = 0;

      if( ISNUM( 2 ) )
      {
         sTabWidth = hb_parnl( 2 );
      }
      if( ( signed ) sTabWidth <= 0 )
      {
         sTabWidth = 8;
      }

      if( ISNUM( 3 ) ||
          ( ( ISCHAR( 3 ) ) && ( hb_parclen( 3 ) > 0 ) ) )
      {
         if( ISNUM( 3 ) )
         {
            cFill = ( char ) ( ( hb_parnl( 3 ) ) % 256 );
         }
         else
         {
            cFill = ( char ) ( *( hb_parc( 3 ) ) );
         }
      }
      else
      {
         cFill = 0x20;
      }

      if( ( ISCHAR( 4 ) ) && ( hb_parclen( 4 ) > 0 ) )
      {
         pcNewLine   = ( char * ) hb_parc( 4 );
         sNewLineLen = hb_parclen( 4 );
      }
      else
      {
         pcNewLine   = hb_conNewLine();
         sNewLineLen = 0;

         while( *( pcNewLine + sNewLineLen ) != 0x00 )
            sNewLineLen++;
      }
      if( sNewLineLen > 0 )
      {
         cCR = *( pcNewLine );
      }
      else
      {
         cCR = 13;
      }

      if( ISNUM( 5 ) ||
          ( ( ISCHAR( 5 ) ) && ( hb_parclen( 5 ) > 0 ) ) )
      {
         if( ISNUM( 5 ) )
         {
            cTab = ( char ) ( ( hb_parnl( 5 ) ) % 256 );
         }
         else
         {
            cTab = ( char ) ( *( hb_parc( 5 ) ) );
         }
      }
      else
      {
         cTab = 0x09;
      }

      if( ISLOG( 6 ) )
      {
         iIgnore141 = hb_parl( 6 );
      }
      else
      {
         iIgnore141 = 0;
      }

      /* estimate maximum return length by assuming that EVERY tab char
         can be replaced by at most <nTabWidth> characters */
      for( sIndex = 0; sIndex < sStrLen; sIndex++ )
      {
         if( *( pcString + sIndex ) == cTab )
         {
            sTabCnt++;
         }
      }
      if( sTabCnt == 0 )
      {
         hb_retclen( pcString, sStrLen );
         return;
      }
      pcRet       = ( unsigned char * ) hb_xgrab( sStrLen + ( sTabCnt * ( sTabWidth - 1 ) ) );

      /* now copy the string */
      sIndex      = 0;
      sRetLen     = 0;
      sLineIndex  = 0;
      while( sTabCnt > 0 )
      {
         char cChar = ( char ) *( pcString + sIndex );
         if( cChar == cTab )
         {
            /* tab character */
            HB_SIZE sFillIndex;

            for( sFillIndex = sTabWidth - ( sLineIndex % sTabWidth );
                 sFillIndex > 0; sFillIndex-- )
            {
               *( pcRet + sRetLen ) = cFill;
               sRetLen++;
               sLineIndex++;
            }
            sTabCnt--;
            sIndex++;
         }
         else if( ( unsigned char ) cChar == 141 )
         {
            /* soft carriage return */
            *( pcRet + sRetLen ) = ( unsigned char ) 141;
            sRetLen++;
            sIndex++;
            if( iIgnore141 )
            {
               sLineIndex++;
            }
            else
            {
               sLineIndex = 0;
            }
         }
         else if( cChar == cCR )
         {
            /* newline string ? */
            if( ( sNewLineLen > 0 ) &&
                ( ( sIndex + sNewLineLen ) <= sStrLen ) &&
                ( ct_at_exact_forward( pcString + sIndex, sNewLineLen,
                                       pcNewLine, sNewLineLen, NULL ) == pcString + sIndex ) )
            {
               hb_xmemcpy( pcRet + sRetLen, pcString + sIndex, (size_t) sNewLineLen );
               sRetLen     += sNewLineLen;
               sIndex      += sNewLineLen;
               sLineIndex  = 0;
            }
            else
            {
               *( pcRet + sRetLen ) = cCR;
               sRetLen++;
               sIndex++;
               sLineIndex++;
            }
         }
         else
         {
            *( pcRet + sRetLen ) = *( pcString + sIndex );
            sRetLen++;
            sIndex++;
            sLineIndex++;
         }

      }

      /* copy rest */
      hb_xmemcpy( pcRet + sRetLen, pcString + sIndex, (size_t) ( sStrLen - sIndex ) );
      sRetLen += sStrLen - sIndex;
      hb_retclen( ( const char *) pcRet, sRetLen );
      hb_xfree( pcRet );

   }
   else /* ISCHAR (1) */
   {
      PHB_ITEM pSubst         = NULL;
      int      iArgErrorMode  = ct_getargerrormode();

      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         pSubst = ct_error_subst( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_TABEXPAND,
                                  NULL, "TABEXPAND", 0, EF_CANSUBSTITUTE, 6,
                                  hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ),
                                  hb_paramError( 4 ), hb_paramError( 5 ), hb_paramError( 6 ) );
      }

      if( pSubst != NULL )
      {
         hb_itemRelease( hb_itemReturnForward( pSubst ) );
      }
      else
      {
         hb_retc( "" );
      }
   }
}

HB_FUNC( TABPACK )
{
   if( ISCHAR( 1 ) )
   {
      char *   pcString    = ( char * ) hb_parc( 1 );
      HB_SIZE  sStrLen     = hb_parclen( 1 );
      unsigned char *   pcRet;
      HB_SIZE  sRetLen;
      HB_SIZE  sTabWidth   = 0;
      char     cFill, cTab, cCR;
      char *   pcNewLine;
      HB_SIZE  sNewLineLen;
      int      iIgnore141;
      HB_SIZE  sIndex, sTabIndex, sFillCount;

      if( ISNUM( 2 ) )
      {
         sTabWidth = hb_parnl( 2 );
      }
      if( ( signed ) sTabWidth <= 0 )
      {
         sTabWidth = 8;
      }

      if( ISNUM( 3 ) ||
          ( ( ISCHAR( 3 ) ) && ( hb_parclen( 3 ) > 0 ) ) )
      {
         if( ISNUM( 3 ) )
         {
            cFill = ( char ) ( ( hb_parnl( 3 ) ) % 256 );
         }
         else
         {
            cFill = ( char ) ( *( hb_parc( 3 ) ) );
         }
      }
      else
      {
         cFill = 0x20;
      }

      if( ( ISCHAR( 4 ) ) && ( hb_parclen( 4 ) > 0 ) )
      {
         pcNewLine   = ( char * ) hb_parc( 4 );
         sNewLineLen = hb_parclen( 4 );
      }
      else
      {
         pcNewLine   = hb_conNewLine();
         sNewLineLen = 0;

         while( *( pcNewLine + sNewLineLen ) != 0x00 )
            sNewLineLen++;
      }
      if( sNewLineLen > 0 )
      {
         cCR = *( pcNewLine );
      }
      else
      {
         cCR = 13;
      }

      if( ISNUM( 5 ) ||
          ( ( ISCHAR( 5 ) ) && ( hb_parclen( 5 ) > 0 ) ) )
      {
         if( ISNUM( 5 ) )
         {
            cTab = ( char ) ( ( hb_parnl( 5 ) ) % 256 );
         }
         else
         {
            cTab = ( char ) ( *( hb_parc( 5 ) ) );
         }
      }
      else
      {
         cTab = 0x09;
      }

      if( ISLOG( 6 ) )
      {
         iIgnore141 = hb_parl( 6 );
      }
      else
      {
         iIgnore141 = 0;
      }

      if( sStrLen == 0 )
      {
         hb_retc( "" );
         return;
      }
      /* estimate maximum return length by assuming that there's
         nothing to pack */
      pcRet       = ( unsigned char * ) hb_xgrab( sStrLen );

      /* now copy the string */
      sIndex      = 0;
      sRetLen     = 0;
      sTabIndex   = 0;
      sFillCount  = 0;

      while( sIndex < sStrLen )
      {
         char cChar = ( char ) *( pcString + sIndex );

         if( cChar == cFill )
         {
            if( sTabIndex == sTabWidth - 1 )
            {
               /* we have just found the last character of a tabstopp */
               *( pcRet + sRetLen ) = cTab;
               sRetLen++;
               sFillCount           = 0;
               sTabIndex            = 0;
               sIndex++;
            }
            else
            {
               sFillCount++;
               sTabIndex++;
               sIndex++;
            }
         }
         else if( cChar == cTab )
         {
            *( pcRet + sRetLen ) = cTab;
            sRetLen++;
            /* discard any fill characters before the tabstopp */
            sFillCount           = 0;
            sTabIndex            = 0;
            sIndex++;
         }
         else if( ( ( unsigned char ) cChar == 141 ) && ( ! iIgnore141 ) )
         {
            /* soft carriage return */

            /* eventually not enough fill chars to fill a tab,
               so copy them verbatim */
            for(; sFillCount > 0; sFillCount-- )
            {
               *( pcRet + sRetLen ) = cFill;
               sRetLen++;
            }
            *( pcRet + sRetLen ) = ( unsigned char ) 141;
            sRetLen++;
            sTabIndex            = 0;
            sIndex++;
         }
         else if( cChar == cCR )
         {
            /* newline string ? */
            if( ( sNewLineLen > 0 ) &&
                ( ( sIndex + sNewLineLen ) <= sStrLen ) &&
                ( ct_at_exact_forward( pcString + sIndex, sNewLineLen,
                                       pcNewLine, sNewLineLen, NULL ) == pcString + sIndex ) )
            {
               /* eventually not enough fill chars to fill a tab,
                  so copy them verbatim */
               for(; sFillCount > 0; sFillCount-- )
               {
                  *( pcRet + sRetLen ) = cFill;
                  sRetLen++;
               }
               hb_xmemcpy( pcRet + sRetLen, pcString + sIndex, (size_t) sNewLineLen );
               sRetLen     += sNewLineLen;
               sIndex      += sNewLineLen;
               sTabIndex   = 0;
            }
            else
            {
               *( pcRet + sRetLen ) = cCR;
               sRetLen++;
               sIndex++;
               sTabIndex            = 0;
            }
         }
         else
         {
            /* eventually not enough fill chars to fill a tab,
               so copy them verbatim */
            for(; sFillCount > 0; sFillCount-- )
            {
               *( pcRet + sRetLen ) = cFill;
               sRetLen++;
               sTabIndex++;

               if( sTabIndex == sTabWidth - 1 )
               {
                  sTabIndex = 0;
               }
            }

            *( pcRet + sRetLen ) = *( pcString + sIndex );
            sRetLen++;
            sIndex++;
            sTabIndex++;

            if( sTabIndex == sTabWidth - 1 )
            {
               sTabIndex = 0;
            }
         }

      }

      /* copy rest */
      for(; sFillCount > 0; sFillCount-- )
      {
         *( pcRet + sRetLen ) = cFill;
         sRetLen++;
      }

      hb_retclen( ( const char *) pcRet, sRetLen );
      hb_xfree( pcRet );

   }
   else /* ISCHAR (1) */
   {
      PHB_ITEM pSubst         = NULL;
      int      iArgErrorMode  = ct_getargerrormode();

      if( iArgErrorMode != CT_ARGERR_IGNORE )
      {
         pSubst = ct_error_subst( ( USHORT ) iArgErrorMode, EG_ARG, CT_ERROR_TABPACK,
                                  NULL, "TABPACK", 0, EF_CANSUBSTITUTE, 6,
                                  hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ),
                                  hb_paramError( 4 ), hb_paramError( 5 ), hb_paramError( 6 ) );
      }

      if( pSubst != NULL )
      {
         hb_itemRelease( hb_itemReturnForward( pSubst ) );
      }
      else
      {
         hb_retc( "" );
      }
   }
}
