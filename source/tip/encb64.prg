/*
 * $Id: encb64.prg 9726 2012-10-02 19:56:17Z andijahja $
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
#include "hbapiitm.h"
#define HB_THREAD_OPTIMIZE_STACK
#pragma ENDDUMP

CLASS TIPEncoderBase64 FROM TIPEncoder

// Set this to .T. to enable RFC 2068 (HTTP/1.1) exception to
// RFC 2045 (MIME) base64 format. This exception consists in
// not applying CRLF after each 76 output bytes.
   DATA bHttpExcept

   METHOD New()      Constructor
   METHOD Encode( cData )
   METHOD Decode( cData )

ENDCLASS

METHOD New() CLASS TIPEncoderBase64

   ::cName := "Base64"
   ::bHttpExcept := .F.

   RETURN Self



#pragma BEGINDUMP

HB_FUNC( TIPENCODERBASE64_ENCODE )
{
   HB_THREAD_STUB
   const char *cData = hb_parc(1);
   char *cRet;
   int nLen = ( int ) hb_parclen(1);
   int nPos = 0, nPosRet = 0;
   int nPosBlock = 0, nLineCount = 0;
   ULONG nFinalLen;
   unsigned char cElem, cElem1;
   BOOL bExcept;

   if ( ! cData )
   {
      hb_errRT_BASE( EG_ARG, 3012, NULL,
         "TIPENCODERBASE64_ENCODE", 1, hb_paramError(1) );
      return;
   }

   if ( ! nLen )
   {
      hb_retc( "" );
      return;
   }

   // read the status of bHttpExcept
   if ( hb_pcount() > 1 )
   {
      // this makes this function static!!!!
      bExcept = hb_parl(2);
   }
   else
   {
      bExcept = hb_itemGetL( hb_objSendMsg( hb_stackSelfItem(), "BHTTPEXCEPT",0));
   }
   // we know exactly the renturned length.
   nFinalLen = (ULONG) ((nLen / 3 + 2) * 4);
   // add line termination padding, CRLF each 76 output bytes
   nFinalLen += (nFinalLen / 72 +1) * 2;
   cRet = (char *) hb_xgrab( nFinalLen );

   while ( nPos < nLen )
   {
      cElem = (unsigned char) cData[ nPos ];
      // NOT using trailing 0 here as some buggy 3dparty func
      // will create strings without trailing 0.

      nPosBlock++;

      switch( nPosBlock )
      {
         case 1:
            cElem = cElem >> 2;
            break;
         case 2:
            cElem1 = nPos < nLen -1 ? (unsigned char) cData[ nPos + 1] : 0;
            cElem = ((cElem & 0x3) << 4) | (cElem1 >> 4);
            nPos++;
            break;
         case 3:
            cElem1 = nPos < nLen -1 ? (unsigned char) cData[ nPos + 1] : 0;
            cElem = ((cElem & 0xF) << 2) | (cElem1 >> 6);
            nPos++;
            break;
         case 4:
            cElem = cElem & 0x3f;
            nPos++;
            nPosBlock = 0;
            break;
      }

      if ( cElem < 26 )
      {
         cRet[nPosRet++] = cElem + 'A';
      }
      else if ( cElem < 52 )
      {
         cRet[nPosRet++] = ( cElem - 26 ) + 'a';
      }
      else if ( cElem < 62 )
      {
         cRet[nPosRet++] = ( cElem - 52 ) + '0';
      }
      else if ( cElem == 62 )
      {
         cRet[nPosRet++] = '+';
      }
      else
      {
         cRet[nPosRet++] = '/';
      }

      if ( ! bExcept )
      {
         nLineCount ++ ;
         /* RFC says to add a CRLF each 76 chars, but is pretty unclear about
            the fact of this 76 chars counting CRLF or not. Common
            practice is to limit line size to 72 chars */
         if ( nLineCount == 72 )
         {
            cRet[nPosRet++] = '\r';
            cRet[nPosRet++] = '\n';
            nLineCount = 0;
         }
      }
   }

   switch( nPos % 3 )
   {
      case 1:
         cRet[ nPosRet++ ] = '=';
         /* fallthrough */
      case 2:
         cRet[ nPosRet++ ] = '=';
         /* fallthrough */
   }

   /* RFC is not explicit, but CRLF SHOULD be added at bottom
      during encoding phase */
   if ( ! bExcept )
   {
      cRet[nPosRet++] = '\r';
      cRet[nPosRet++] = '\n';
   }

   /* this function also adds a zero */
   hb_retclenAdopt( cRet, nPosRet );
}

#pragma ENDDUMP


#pragma BEGINDUMP
HB_FUNC( TIPENCODERBASE64_DECODE )
{
   HB_THREAD_STUB
   const char *cData = hb_parc(1);
   unsigned char *cRet;
   int nLen = ( int ) hb_parclen(1);
   int nPos = 0, nPosRet = 0, nPosBlock = 0;
   unsigned char cElem;

   if ( ! cData )
   {
      hb_errRT_BASE( EG_ARG, 3012, NULL,
         "TIPENCODERBASE64_DECODE", 1, hb_paramError(1) );
      return;
   }

   if ( ! nLen )
   {
      hb_retc( "" );
      return;
   }


   // we know exactly the renturned length.
   cRet = (unsigned char *) hb_xgrab( (nLen / 4 + 1) * 3 );

   while ( nPos < nLen )
   {
      cElem = cData[ nPos ];

      if ( cElem >= 'A' && cElem <= 'Z' )
      {
         cElem -= 'A';
      }
      else if ( cElem >= 'a' && cElem <= 'z' )
      {
         cElem = cElem - 'a' + 26;
      }
      else if ( cElem >= '0' && cElem <= '9' )
      {
         cElem = cElem - '0' + 52;
      }
      else if ( cElem == '+' )
      {
         cElem = 62;
      }
      else if ( cElem == '/' )
      {
         cElem = 63;
      }
      // end of stream?
      else if ( cElem == '=' )
      {
         break;
      }
      // RFC 2045 specifies characters not in base64 must be ignored
      else
      {
         nPos++;
         continue;
      }

      switch( nPosBlock )
      {
         case 0:
            cRet[nPosRet]  = cElem << 2;
            nPosBlock++;
            break;
         case 1:
            // higer bits are zeros
            cRet[nPosRet] |= cElem >> 4;
            nPosRet++;
            cRet[nPosRet]  = cElem << 4;
            nPosBlock++;
            break;
         case 2:
            // higer bits are zeros
            cRet[nPosRet] |= cElem >> 2;
            nPosRet++;
            cRet[nPosRet]  = cElem << 6;
            nPosBlock++;
            break;
         case 3:
            cRet[nPosRet] |= cElem;
            nPosRet++;
            nPosBlock = 0;
            break;
      }

      nPos++;
   }

   /* this function also adds a zero */
   /* hopefully reduce the size of cRet */
   cRet = (unsigned char *) hb_xrealloc( cRet, nPosRet + 1 );
   hb_retclenAdopt( (char *)cRet, nPosRet );
}

#pragma ENDDUMP

