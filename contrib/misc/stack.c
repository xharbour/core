/*
 * $Id: stack.c 9722 2012-10-02 06:13:24Z andijahja $
 */

/*
 * Harbour Project source code:
 * Stack structure
 *
 * Copyright 2000 Jose Lalin <dezac@corevia.com>
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

/* StackNew() --> <aStack>
*/
HB_FUNC( STACKNEW )
{
   HB_ITEM Return;

   Return.type = HB_IT_NIL;

   hb_arrayNew( &Return, 0 );   /* Create array */

   hb_itemReturnForward( &Return );
}

/*  StackPush( <aStack>, <xValue> ) --> <aStack>
*/
HB_FUNC( STACKPUSH )
{
   PHB_ITEM pArray = hb_param( 1, HB_IT_ARRAY );
   PHB_ITEM pAny = hb_param( 2, HB_IT_ANY );

   hb_arrayAdd( pArray, pAny );
}

/* StackPop( <aStack> ) --> <xValue>
   Returns NIL if the stack is empty
*/
HB_FUNC( STACKPOP )
{
   PHB_ITEM pArray = hb_param( 1, HB_IT_ARRAY );
   HB_SIZE ulLen = hb_arrayLen( pArray );
   HB_ITEM Last;

   Last.type = HB_IT_NIL;

   if( ulLen )
   {
      hb_arrayLast( pArray, &Last );
      hb_arrayDel( pArray, ulLen );
      --ulLen;
      hb_arraySize( pArray, HB_MAX( ulLen, 0 ) );
   }

   hb_itemReturnForward( &Last );
}

/* StackIsEmpty( <aStack> ) --> <lEmpty>
*/
HB_FUNC( STACKISEMPTY )
{
   PHB_ITEM pArray = hb_param( 1, HB_IT_ARRAY );

   hb_retl( hb_arrayLen( pArray ) == 0 );
}

/* StackTop( <aStack> ) --> <xValue>
   Returns the top item
*/
HB_FUNC( STACKTOP )
{
   PHB_ITEM pArray = hb_param( 1, HB_IT_ARRAY );
   HB_ITEM Last;

   Last.type = HB_IT_NIL;

   hb_arrayLast( pArray, &Last );

   hb_itemReturnForward( &Last );
}
