/*
 * $Id: throw.c 9829 2012-11-19 17:15:34Z andijahja $
 */

/*
 * Harbour Project source code:
 * Regular Expressions Interface functions
 *
 * www - http://www.xharbour.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version, with one exception:
 *
 * The exception is that if you link the Harbour Runtime Library (HRL)
 * and/or the Harbour Virtual Machine (HVM) with other files to produce
 * an executable, this does not by itself cause the resulting executable
 * to be covered by the GNU General Public License. Your use of that
 * executable is in no way restricted on account of linking the HRL
 * and/or HVM code into it.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA (or visit
 * their web site at http://www.gnu.org/).
 *
 */

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"

HB_FUNC( THROW )
{
   PHB_ITEM pError = hb_param( 1, HB_IT_ANY ), pResult;

   if( pError && HB_IS_OBJECT( pError ) )
   {
      pError   = hb_itemNew( pError );
      pResult  = hb_errLaunchSubst( pError );

      hb_itemRelease( pError );

      if( pResult )
         hb_itemRelease( hb_itemReturnForward( pResult ) );
   }
   else
      hb_errRT_BASE( EG_ARG, 9101, NULL, "THROW", 1, hb_paramError( 1 ) );
}
