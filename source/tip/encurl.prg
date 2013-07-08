/*
 * $Id: encurl.prg 9759 2012-10-15 14:02:59Z andijahja $
 */

/*
 * xHarbour Project source code:
 * TIP Class oriented Internet protocol library
 *
 * Copyright 2003 Giancarlo Niccolai <gian@niccolai.ws>
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

#include "hbclass.ch"

#pragma BEGINDUMP
#include "hbapi.h"
#include "hbapierr.h"
#include "hbstack.h"
#pragma ENDDUMP

CLASS TIPEncoderUrl FROM TIPEncoder

   METHOD New()   Constructor
   METHOD Encode()
   METHOD Decode()

ENDCLASS

METHOD New() CLASS TIPEncoderURL

   ::cName := "urlencoded"

   RETURN Self


#pragma BEGINDUMP

HB_FUNC( TIPENCODERURL_ENCODE )
{
   const char *cData = hb_parc(1);
   int nLen = ( int ) hb_parclen(1);
   BOOL bComplete = hb_parl(2);
   char *cRet;
   int nPos = 0, nPosRet = 0, nVal;
   char cElem;
   
   if ( hb_pcount() < 2 )
   {
      bComplete = TRUE;
   }

   if ( ! cData )
   {
      hb_errRT_BASE( EG_ARG, 3012, NULL,
         "TIPENCODERURL_ENCODE", 1, hb_paramError(1) );
      return;
   }

   if ( ! nLen )
   {
      hb_retc( "" );
      return;
   }

   // Giving maximum final length possible
   cRet = (char *) hb_xgrab( nLen * 3 +1);

   while ( nPos < nLen )
   {
      cElem = cData[ nPos ];

      if ( cElem == ' ' )
      {
         cRet[ nPosRet ] = '+';
      }
      else if (
         (cElem >= 'A' && cElem <= 'Z') ||
         (cElem >= 'a' && cElem <= 'z') ||
         (cElem >= '0' && cElem <= '9') ||
         cElem == '.' || cElem == ',' || cElem == '&' ||
         cElem == '/' || cElem == ';' || cElem =='_' )
      {
         cRet[ nPosRet ] = cElem;
      }
      else if ( ! bComplete && ( cElem == ':' || cElem == '?' || cElem == '=' ) )
      {
         cRet[ nPosRet ] = cElem;
      }
      else // encode!
      {
         cRet[ nPosRet++] = '%';
         nVal = ((unsigned char) cElem) >> 4;
         cRet[ nPosRet++] = ( char ) ( nVal < 10 ? '0' + nVal : 'A' + nVal - 10 );
         nVal = ((unsigned char) cElem) & 0x0f;
         cRet[ nPosRet ] = ( char ) ( nVal < 10 ? '0' + nVal : 'A' + nVal - 10 );
      }

      nPosRet++;
      nPos++;
   }

   cRet[ nPosRet ] = 0;
   /* this function also adds a zero */
   hb_retclenAdoptRaw( cRet, nPosRet );
}

#pragma ENDDUMP


#pragma BEGINDUMP
HB_FUNC( TIPENCODERURL_DECODE )
{
   const char *cData = hb_parc(1);
   int nLen = ( int ) hb_parclen(1);
   char *cRet;
   int nPos = 0, nPosRet = 0;
   char cElem;

   if ( ! cData )
   {
      hb_errRT_BASE( EG_ARG, 3012, NULL,
         "TIPENCODERURL_DECODE", 1, hb_paramError(1) );
      return;
   }

   if ( ! nLen )
   {
      hb_retc( "" );
      return;
   }


   // maximum possible lenght
   cRet = (char *) hb_xgrab( nLen );

   while ( nPos < nLen )
   {
      cElem = cData[ nPos ];

      if ( cElem == '+' )
      {
         cRet[ nPosRet ] = ' ';
      }
      else if ( cElem == '%' )
      {
         if ( nPos < nLen - 2 )
         {
            cElem = cData[ ++nPos ];
            cRet[ nPosRet ] = cElem < 'A' ? cElem - '0' : cElem - 'A' + 10;
            cRet[ nPosRet ] *= 16;

            cElem = cData[ ++nPos ];
            cRet[ nPosRet ] |= cElem < 'A' ? cElem - '0' : cElem - 'A' + 10;
         }
         else
         {
            if (nPosRet > 0 )
            {
               break;
            }
         }
      }
      else
      {
         cRet[ nPosRet ] = cElem;
      }

      nPos++;
      nPosRet++;
   }

   /* this function also adds a zero */
   /* hopefully reduce the size of cRet */
   cRet = (char *) hb_xrealloc( cRet, nPosRet + 1 );
   hb_retclenAdopt( cRet, nPosRet );
}

#pragma ENDDUMP

