/*
 * $Id: round.c 9859 2012-12-07 00:08:53Z andijahja $
 */

/*
 * Harbour Project source code:
 * ROUND(), INT() functions
 *
 * Copyright 1999 David G. Holm <dholm@jsd-llc.com>
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

/*
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 * Copyright 1999 Matthew Hamilton <mhamilton@bunge.com.au>
 *    INT()
 *
 * See doc/license.txt for licensing terms.
 *
 */

#include <math.h>

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"

HB_FUNC( INT )
{
   PHB_ITEM pNumber = hb_param( 1, HB_IT_NUMERIC );

   if( pNumber )
   {
      if( HB_IS_NUMBER_INT( pNumber ) )
         hb_itemReturn( pNumber );
      else
      {
         int iWidth;

         hb_itemGetNLen( pNumber, &iWidth, NULL );
         hb_retnlen( hb_numInt( hb_itemGetND( pNumber ) ), iWidth, 0 );
      }
      return;
   }

   hb_errRT_BASE_SubstR( EG_ARG, 1090, NULL, "INT", 1, hb_paramError( 1 ) );
}

HB_FUNC( ROUND )
{
   PHB_ITEM pNumber = hb_param( 1, HB_IT_NUMERIC );

   if( pNumber )
   {
      PHB_ITEM pDec = hb_param( 2, HB_IT_NUMERIC );

      if( pDec )
      {
         int iDec = hb_itemGetNI( pDec );

         if( HB_IS_NUMBER_INT( pNumber ) && iDec == 0 )
            hb_itemReturn( pNumber );
         else
            hb_retnlen( hb_numRound( hb_itemGetND( pNumber ), iDec ), 0, HB_MAX( iDec, 0 ) );

         return;
      }
   }

   hb_errRT_BASE_SubstR( EG_ARG, 1094, NULL, "ROUND", 2, hb_paramError( 1 ), hb_paramError( 2 ) );
}
