/*
 * $Id: binnumx.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * BIN2U(), W2BIN(), U2BIN() functions
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
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

/* NOTE: Xbase++ compatible functions */

#include "hbapi.h"
#include "hbapiitm.h"

#ifdef HB_COMPAT_XPP

HB_FUNC( BIN2U )
{
   PHB_ITEM pItem = hb_param( 1, HB_IT_STRING );

   if( pItem )
   {
      char *   pszString   = pItem->item.asString.value;
      HB_SIZE  ulLen       = pItem->item.asString.length;
      ULONG    value       = HB_MKULONG( ( ulLen >= 1 ) ? ( BYTE ) pszString[ 0 ] : 0,
                                         ( ulLen >= 2 ) ? ( BYTE ) pszString[ 1 ] : 0,
                                         ( ulLen >= 3 ) ? ( BYTE ) pszString[ 2 ] : 0,
                                         ( ulLen >= 4 ) ? ( BYTE ) pszString[ 3 ] : 0 );

#ifndef HB_LONG_LONG_OFF
      hb_retnll( value );
#else
      hb_retnl( value );
#endif
   }
   else
      hb_retnl( 0 );
}

HB_FUNC( W2BIN )
{
   char szString[ 2 ];

   if( ISNUM( 1 ) )
   {
      USHORT uiValue = ( USHORT ) hb_parni( 1 );

      szString[ 0 ]  = ( uiValue & 0x00FF );
      szString[ 1 ]  = ( uiValue & 0xFF00 ) >> 8;
   }
   else
   {
      szString[ 0 ]     =
         szString[ 1 ]  = '\0';
   }

   hb_retclen( szString, 2 );
}

HB_FUNC( U2BIN )
{
   char szString[ 4 ];

   if( ISNUM( 1 ) )
   {
      ULONG ulValue = ( ULONG ) hb_parnl( 1 );

      szString[ 0 ]  = ( char ) ( ulValue & 0x000000FF );
      szString[ 1 ]  = ( char ) ( ( ulValue & 0x0000FF00 ) >> 8 );
      szString[ 2 ]  = ( char ) ( ( ulValue & 0x00FF0000 ) >> 16 );
      szString[ 3 ]  = ( char ) ( ( ulValue & 0xFF000000 ) >> 24 );
   }
   else
   {
      szString[ 0 ]           =
         szString[ 1 ]        =
            szString[ 2 ]     =
               szString[ 3 ]  = '\0';
   }

   hb_retclen( szString, 4 );
}

#endif

