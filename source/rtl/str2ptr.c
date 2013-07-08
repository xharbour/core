/*
 * $Id: str2ptr.c 9734 2012-10-09 19:54:17Z andijahja $
 */

/*
 * Harbour Project source code:
 * HB_POINTER2STRING(), HB_STRING2POINTER() functions
 *
 * Copyright 2004 Ron Pinkas <ronpinkas@profit-master.com>
 * www - http://www.xharbour.org
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
#include "hbfast.h"

HB_FUNC( HB_POINTER2STRING )
{
   PHB_ITEM pPointer = hb_param( 1, HB_IT_ANY );
   PHB_ITEM pLen     = hb_param( 2, HB_IT_NUMERIC );

   if( HB_IS_POINTER( pPointer ) && pLen )
   {
      hb_retclenStatic( ( char * ) hb_itemGetPtr( pPointer ), ( ULONG ) hb_itemGetNL( pLen ) );
   }
   else if( HB_IS_INTEGER( pPointer ) && pLen )
   {
#if defined ( HB_OS_WIN_64 )
      hb_retclenStatic( ( char * ) ( HB_LONG ) hb_itemGetNI( pPointer ), ( ULONG ) hb_itemGetNL( pLen ) );
#else
      hb_retclenStatic( ( char * ) hb_itemGetNI( pPointer ), ( ULONG ) hb_itemGetNL( pLen ) );
#endif
   }
   else if( HB_IS_LONG( pPointer ) && pLen )
   {
#if defined ( HB_OS_WIN_64 )
      hb_retclenStatic( ( char * ) ( HB_LONG ) hb_itemGetNL( pPointer ), ( ULONG ) hb_itemGetNL( pLen ) );
#else
      hb_retclenStatic( ( char * ) hb_itemGetNL( pPointer ), ( ULONG ) hb_itemGetNL( pLen ) );
#endif
   }
   else
   {
      hb_errRT_BASE_SubstR( EG_ARG, 1099, NULL, "HB_Pointer2String", 2, hb_paramError( 1 ), hb_paramError( 2 ) );
   }
}

HB_FUNC( HB_STRING2POINTER )
{
   PHB_ITEM pString = hb_param( 1, HB_IT_STRING );

   if( pString )
   {
      hb_retptr( ( void * ) pString->item.asString.value );
   }
   else
   {
      hb_errRT_BASE_SubstR( EG_ARG, 1099, NULL, "HB_String2Pointer", 1, hb_paramError( 1 ) );
   }
}

