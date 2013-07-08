/*
 * $Id: sysprop.c 9829 2012-11-19 17:15:34Z andijahja $
 */

/*
 * SixAPI Project source code:
 *
 * Copyright 2010 Andi Jahja <xharbour@telkom.net.id>
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
 */
#include "sxapi.h"

HB_FUNC( SX_SYSPROP )
{
   int i = hb_parni( 2 );

   if( ISNIL( 2 ) )
      sx_SysProp( ( WORD ) hb_parni( 1 ), ( PVOID ) NULL );
   else
#if defined( __MINGW32__ ) && defined( HB_OS_WIN_64 )
      hb_retni( sx_SysProp(
                   ( WORD ) hb_parni( 1 ), /* One of the predefined constant values. */
                   ( void * ) ( HB_LONG ) i ) );
#else
      hb_retni( sx_SysProp(
                   ( WORD ) hb_parni( 1 ), /* One of the predefined constant values. */
                   ( void * ) i ) );
#endif
}

HB_FUNC( SX_RDDDRIVER )
{
   int      iDriverName;
   char *   cRDD[] = { "SDENTX", "SDEFOX", "SDENSX", "SDENSX_DBT" };

   if( hb_pcount() == 0 )
      iDriverName = sx_SysProp( SDE_SP_GETDRIVER, ( PVOID ) NULL );
   else
   {
      int iWorkArea = hb_parni( 1 );
#if defined( __MINGW32__ ) && defined( HB_OS_WIN_64 )
      iDriverName = ( int ) sx_SysProp( SDE_SP_GETDRIVER, ( void * ) ( HB_LONG ) iWorkArea );
#else
      iDriverName = ( int ) sx_SysProp( SDE_SP_GETDRIVER, ( void * ) iWorkArea );
#endif
   }

   hb_retc( cRDD[ iDriverName - 1 ] );
}
