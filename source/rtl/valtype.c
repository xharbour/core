/*
 * $Id: valtype.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * VALTYPE() function
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

/*
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 * Copyright 2002-2003 Walter Negro <anegro@overnet.com.ar>
 *    HB_ISBYREF()
 *    HB_ISNIL()
 *    HB_ISSTRING()
 *    HB_ISMEMO()
 *    HB_ISNUMERIC()
 *    HB_ISLOGICAL()
 *    HB_ISDATE()
 *    HB_ISDATETIME() //toninho@fwi
 *    HB_ISARRAY()
 *    HB_ISOBJECT()
 *    HB_ISBLOCK()
 *    HB_ISPOINTER()
 *    HB_ISNULL()
 *
 *  Copyright 2007-2008 Miguel Angel Marchuet <miguelangel@marchuet.net>
 *    ISNULL()
 *    SETNULL()
 *
 * See doc/license.txt for licensing terms.
 *
 */


#include "hbapi.h"
#include "hbapierr.h"
#include "hbapiitm.h"
#include "hbstack.h"
#include "hashapi.h"

HB_FUNC( VALTYPE )
{
   hb_retc( hb_itemTypeStr( hb_param( 1, HB_IT_ANY ) ) );
}

HB_FUNC( ISNULL )
{
   hb_retl( ISNULL( 1 ) );
}

HB_FUNC( SETNULL )
{
   if( hb_pcount() )
   {
      PHB_ITEM pItem = hb_param( 1, HB_IT_ANY );
      pItem->type |= HB_IT_NULL;
   }
}

HB_FUNC( HB_ISBYREF )
{
   PHB_ITEM pItem;

   if( hb_pcount() )
   {
      pItem = hb_stackItemFromBase( 1 );

      if( HB_IS_BYREF( pItem ) )
      {
         pItem = hb_itemUnRefOnce( pItem );

         if( HB_IS_BYREF( pItem ) )
            hb_retl( TRUE );

         else
            hb_retl( FALSE );
      }
      else
         hb_ret();
   }
}

HB_FUNC( HB_ISNIL )
{
   hb_retl( ISNIL( 1 ) );
}

HB_FUNC( HB_ISNUMERIC )
{
   hb_retl( HB_IS_NUMBER( hb_param( 1, HB_IT_ANY ) ) );
}

HB_FUNC( HB_ISLOGICAL )
{
   hb_retl( ISLOG( 1 ) );
}

HB_FUNC( HB_ISDATE )
{
   hb_retl( ISDATE( 1 ) );
}

HB_FUNC( HB_ISDATETIME )
{
   hb_retl( ISDATETIME( 1 ) );
}

HB_FUNC( HB_ISTIMESTAMP )
{
   hb_retl( ( hb_parinfo( 1 ) & HB_IT_TIMEFLAG ) != 0 );
}

HB_FUNC( HB_ISBLOCK )
{
   hb_retl( ISBLOCK( 1 ) );
}

HB_FUNC( HB_ISPOINTER )
{
   hb_retl( ISPOINTER( 1 ) );
}

HB_FUNC( HB_ISSTRING )
{
   hb_retl( ISCHAR( 1 ) );
}

HB_FUNC( HB_ISCHAR )
{
   hb_retl( ( hb_parinfo( 1 ) & HB_IT_MEMO ) == HB_IT_STRING );
}

HB_FUNC( HB_ISMEMO )
{
   hb_retl( ( hb_parinfo( 1 ) & HB_IT_MEMOFLAG ) != 0 );
}

HB_FUNC( HB_ISARRAY )
{
   hb_retl( ISARRAY( 1 ) && hb_param( 1, HB_IT_ARRAY )->item.asArray.value->uiClass == 0 );
}

HB_FUNC( HB_ISOBJECT )
{
   hb_retl( ISOBJECT( 1 ) );
}

HB_FUNC( HB_ISHASH )
{
   hb_retl( ISHASH( 1 ) );
}

HB_FUNC( HB_ISNULL )
{
   PHB_ITEM pItem = hb_param( 1, HB_IT_ANY );

   if( pItem )
   {
      if( HB_IS_STRING( pItem ) )
      {
         hb_retl( hb_itemGetCLen( pItem ) == 0 );
         return;
      }
      else if( HB_IS_ARRAY( pItem ) )
      {
         hb_retl( hb_arrayLen( pItem ) == 0 );
         return;
      }
      else if( HB_IS_HASH( pItem ) )
      {
         hb_retl( hb_hashLen( pItem ) == 0 );
         return;
      }
   }
   hb_errRT_BASE_SubstR( EG_ARG, 1111, NULL, "HB_ISNULL", 1, hb_paramError( 1 ) );
}
