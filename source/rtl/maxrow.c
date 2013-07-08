/*
 * $Id: maxrow.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * MAXROW(), MAXCOL() functions
 *
 * Copyright 2004 Paul Tucker <ptucker@sympatico.ca>
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

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapigt.h"
#include "hbgtcore.h"

int hb_MaxRow( BOOL bVisible )
{
   /*
    * if called with logical .T. parameter then return real screen high - 1
    * It gives exactly the same result in all standard GT drivers so we
    * are still Clipper compatible. The difference can appear in some extended
    * GT drivers which have additional functionality, f.e. CTW GT which
    * is upper level GT and add CTIII Window support. When it's activated
    * then MaxRow() will return current window max row and MaxRow(.t.) real
    * screen (window 0) max row what is the exact behavior of MaxRow()
    * in CT3, [druzus]
    */
   if( bVisible )
   {
      PHB_GT      pGT = hb_gt_Base();
      HB_GT_INFO  gtInfo;

      gtInfo.pNewVal = gtInfo.pResult = NULL;

      assert( pGT );

      HB_GTSELF_INFO( pGT, HB_GTI_VIEWPORTHEIGHT, &gtInfo );

      if( gtInfo.pResult )
      {
         int iMaxRow = hb_itemGetNI( gtInfo.pResult );

         hb_itemRelease( gtInfo.pResult );
         return iMaxRow;
      }
      else
      {
         return hb_gtMaxRow();
      }
   }
   else
   {
      return hb_gtMaxRow();
   }
}

int hb_MaxCol( BOOL bVisible )
{
   /* See the note about MaxRow(.t.) above */
   if( bVisible )
   {
      PHB_GT      pGT = hb_gt_Base();
      HB_GT_INFO  gtInfo;

      gtInfo.pNewVal = gtInfo.pResult = NULL;

      assert( pGT );

      HB_GTSELF_INFO( pGT, HB_GTI_VIEWPORTWIDTH, &gtInfo );

      if( gtInfo.pResult )
      {
         int iMaxCol = hb_itemGetNI( gtInfo.pResult );

         hb_itemRelease( gtInfo.pResult );
         return iMaxCol;
      }
      else
      {
         return hb_gtMaxCol();
      }
   }
   else
   {
      return hb_gtMaxCol();
   }
}

HB_FUNC( MAXROW ) /* Return the maximum screen row number (zero origin) */
{
   hb_retni( hb_MaxRow( ISLOG( 1 ) && hb_parl( 1 ) ) );
}

HB_FUNC( MAXCOL ) /* Return the maximum screen column number (zero origin) */
{
   hb_retni( hb_MaxCol( ISLOG( 1 ) && hb_parl( 1 ) ) );
}
