/*
 * $Id: box.c 9759 2012-10-15 14:02:59Z andijahja $
 */

/*
 * Harbour Project source code:
 * DISPBOX() function
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

#include "hbapi.h"
#include "hbapigt.h"
#include "hbapiitm.h"

HB_FUNC( DISPBOX )
{
   PHB_ITEM pTop     = hb_param( 1, HB_IT_NUMERIC );
   PHB_ITEM pLeft    = hb_param( 2, HB_IT_NUMERIC );
   PHB_ITEM pBottom  = hb_param( 3, HB_IT_NUMERIC );
   PHB_ITEM pRight   = hb_param( 4, HB_IT_NUMERIC );

   if( pTop && pLeft && pBottom && pRight )
   {
      const char *   pszColor = ISCHAR( 6 ) ? hb_parcx( 6 ) : NULL;
      char           szOldColor[ CLR_STRLEN ];

      if( pszColor )
      {
         hb_gtGetColorStr( szOldColor );
         hb_gtSetColorStr( pszColor );
      }

      if( ISCHAR( 5 ) )
      {
         hb_gtBox( ( SHORT ) hb_itemGetNI( pTop ),
                   ( SHORT ) hb_itemGetNI( pLeft ),
                   ( SHORT ) hb_itemGetNI( pBottom ),
                   ( SHORT ) hb_itemGetNI( pRight ),
                   ( BYTE * ) ( hb_parclen( 5 ) == 0 ? "         " : hb_parcx( 5 ) ) );
      }
      else if( ISNUM( 5 ) && hb_parni( 5 ) == 2 )
      {
         hb_gtBoxD( ( SHORT ) hb_itemGetNI( pTop ),
                    ( SHORT ) hb_itemGetNI( pLeft ),
                    ( SHORT ) hb_itemGetNI( pBottom ),
                    ( SHORT ) hb_itemGetNI( pRight ) );
      }
      else
      {
         hb_gtBoxS( ( SHORT ) hb_itemGetNI( pTop ),
                    ( SHORT ) hb_itemGetNI( pLeft ),
                    ( SHORT ) hb_itemGetNI( pBottom ),
                    ( SHORT ) hb_itemGetNI( pRight ) );
      }

      if( pszColor )
      {
         hb_gtSetColorStr( szOldColor );
      }
   }
}

HB_FUNC( HB_DISPBOX )
{
   PHB_ITEM pTop     = hb_param( 1, HB_IT_NUMERIC );
   PHB_ITEM pLeft    = hb_param( 2, HB_IT_NUMERIC );
   PHB_ITEM pBottom  = hb_param( 3, HB_IT_NUMERIC );
   PHB_ITEM pRight   = hb_param( 4, HB_IT_NUMERIC );

   if( pTop && pLeft && pBottom && pRight )
   {
      const char *   pszBox   = hb_parcx( 5 );
      const char *   pszColor = hb_parc( 6 );
      int            iColor;

      if( pszColor )
         iColor = hb_gtColorToN( pszColor );
      else if( ISNUM( 6 ) )
         iColor = hb_parni( 6 );
      else
         iColor = -1;

      hb_gtDrawBox( ( SHORT ) hb_itemGetNI( pTop ),
                    ( SHORT ) hb_itemGetNI( pLeft ),
                    ( SHORT ) hb_itemGetNI( pBottom ),
                    ( SHORT ) hb_itemGetNI( pRight ),
                    ( BYTE * ) ( *pszBox ? pszBox : "         " ),
                    iColor );
   }
}
