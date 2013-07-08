/*
 * $Id: substr.c 9859 2012-12-07 00:08:53Z andijahja $
 */

/*
 * Harbour Project source code:
 * SUBSTR() function
 *
 * Copyright 1999 Antonio Linares <alinares@fivetech.com>
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
#include "hbapierr.h"

/* returns l characters from n characters into string */

HB_FUNC( SUBSTR )
{
   PHB_ITEM pText = hb_param( 1, HB_IT_STRING );
   PHB_ITEM pNum  = hb_param( 2, HB_IT_NUMERIC );

   if( pText && pNum )
   {
      LONG lPos = hb_itemGetNL( pNum );

      if( lPos < 0 )
      {
         lPos += ( LONG ) pText->item.asString.length;

         if( lPos < 0 )
            lPos = 0;
      }
      else if( lPos )
         lPos--;

      if( lPos < ( LONG ) pText->item.asString.length )
      {
         LONG lLen;

         if( hb_pcount() >= 3 )
         {
            PHB_ITEM pNum1 = hb_param( 3, HB_IT_NUMERIC );

            if( pNum1 )
            {
               lLen = hb_itemGetNL( pNum1 );

               if( lLen > ( LONG ) pText->item.asString.length - lPos )
                  lLen = ( LONG ) pText->item.asString.length - lPos;
            }
            else
            {
               hb_errRT_BASE_SubstR( EG_ARG, 1110, NULL, "SUBSTR", 3, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
               /* NOTE: Exit from inside [vszakats] */
               return;
            }
         }
         else
            lLen = ( LONG ) pText->item.asString.length - lPos;

         if( lLen > 0 )
            hb_retclen( pText->item.asString.value + lPos, lLen );
         else
            hb_retc( "" );
      }
      else
         hb_retc( "" );

      return;
   }

   hb_errRT_BASE_SubstR( EG_ARG, 1110, NULL, "SUBSTR", 3, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
}
