/*
 * $Id: empty.c 9859 2012-12-07 00:08:53Z andijahja $
 */

/*
 * Harbour Project source code:
 * EMPTY() function
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
 * Copyright 2007 Walter Negro <anegro@overnet.com.ar>
 *    Support DateTime
 *
 */

#include "hbapi.h"
#include "hbapiitm.h"
#include "hashapi.h"

HB_FUNC( EMPTY )
{
   PHB_ITEM pItem = hb_param( 1, HB_IT_ANY );

   /* switch( hb_itemType( pItem ) )
    */
   switch( pItem->type )
   {
      case HB_IT_ARRAY:
         hb_retl( pItem->item.asArray.value->ulLen == 0 );
         break;

      case HB_IT_STRING:
      case HB_IT_MEMO:
         hb_retl( hb_strEmpty( pItem->item.asString.value, pItem->item.asString.length ) );
         break;

      case HB_IT_INTEGER:
         hb_retl( pItem->item.asInteger.value == 0 );
         break;

      case HB_IT_LONG:
         hb_retl( pItem->item.asLong.value == 0l );
         break;

      case HB_IT_DOUBLE:
         hb_retl( pItem->item.asDouble.value == 0.0 );
         break;

      case HB_IT_DATE:
         hb_retl( pItem->item.asDate.value == 0 );
         break;

      case HB_IT_TIMEFLAG:
         hb_retl( pItem->item.asDate.value == 0 && pItem->item.asDate.time == 0 );
         break;

      case HB_IT_POINTER:
         hb_retl( pItem->item.asPointer.value == NULL );
         break;

      case HB_IT_LOGICAL:
         hb_retl( ! pItem->item.asLogical.value );
         break;

      case HB_IT_BLOCK:
         hb_retl( FALSE );
         break;

      case HB_IT_HASH:
         hb_retl( pItem->item.asHash.value->ulTotalLen == 0 );
         break;

      default:
         hb_retl( TRUE );
         break;
   }
}
