/*
 * $Id: padc.c 9859 2012-12-07 00:08:53Z andijahja $
 */

/*
 * Harbour Project source code:
 * PADC() function
 *
 * Copyright 1999 Matthew Hamilton <mhamilton@bunge.com.au>
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
#include "hbfast.h"
#include "hbapierr.h"

/* centre-pads a date, number, or string with spaces or supplied character */
HB_FUNC( PADC )
{
   HB_SIZE  ulSize   = 0;
   BOOL     bFreeReq = FALSE;
   PHB_ITEM pPad     = hb_param( 2, HB_IT_NUMERIC );
   char * szText     = pPad ? hb_itemPadConv( hb_param( 1, HB_IT_ANY ), &ulSize, &bFreeReq ) : NULL;

   if( szText )
   {
      LONG lLen = hb_itemGetNL( pPad );

      if( lLen > ( LONG ) ulSize )
      {
         PHB_ITEM pszPad   = hb_param( 3, HB_IT_STRING );
         char *   szResult = ( char * ) hb_xgrab( lLen + 1 );
         char     cPad;
         LONG     w, lPos  = ( lLen - ( LONG ) ulSize ) / 2;

         hb_xmemcpy( szResult + lPos, szText, ( LONG ) ulSize + 1 );

         cPad = pszPad ? *pszPad->item.asString.value : ' ' ;

         for( w = 0; w < lPos; w++ )
            szResult[ w ] = cPad;

         for( w = ( LONG ) ulSize + lPos; w < lLen; w++ )
            szResult[ w ] = cPad;

         szResult[ lLen ] = '\0';

         hb_retclenAdopt( szResult, lLen );
      }
      else
      {
         if( lLen < 0 )
            lLen = 0;

         hb_retclen( szText, lLen );
      }

      if( bFreeReq )
         hb_xfree( szText );
   }
   else
      hb_retc( "" );
}
