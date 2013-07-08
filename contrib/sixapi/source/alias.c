/*
 * $Id: alias.c 9752 2012-10-14 03:33:16Z andijahja $
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

HB_FUNC( SX_AUTOALIAS )
{
   hb_retcAdopt( _sx_AutoAlias( ( char * ) hb_parc( 1 ) ) );
}

HB_FUNC( SX_ALIAS )
{
   WORD iWorkArea = 0;

   if( ! _sx_Used() )
   {
      hb_retc( "" );
      return;
   }

   if( ISNUM( 1 ) )
   {
      iWorkArea = ( WORD ) hb_parnl( 1 );
   }

   hb_retc( ( char * ) sx_Alias( iWorkArea ) );
}

/* Making new alias name based on cpFileName
   User must hb_xfree szAlias when calling this function
 */
char * _sx_AutoAlias( const char * cpFileName )
{
   if( cpFileName )
   {
      int         uiLenAlias;
      char *      szAlias;
      PHB_FNAME   pFileName = hb_fsFNameSplit( cpFileName );

      uiLenAlias = ( int ) strlen( ( char * ) pFileName->szName ) + 1;
      szAlias    = ( char * ) hb_xgrab( uiLenAlias );
      hb_snprintf( szAlias, uiLenAlias, "%s", pFileName->szName );
      hb_xfree( pFileName );
      return szAlias;
   }

   return NULL;
}

HB_FUNC( SX_WORKAREA )
{
   PBYTE szAlias = 0;

   if( ISCHAR( 1 ) )
   {
      szAlias = ( PBYTE ) hb_parc( 1 );
   }

   hb_retni( sx_WorkArea( szAlias ) );
}
