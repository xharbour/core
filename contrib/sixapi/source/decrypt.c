/*
 * $Id: decrypt.c 9753 2012-10-14 10:51:37Z andijahja $
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

HB_FUNC( SX_DBFDECRYPT )
{
   WORD  iWorkArea = SX_DUMMY_NUMBER;
   PBYTE cpPassword;

   if( ! _sx_Used() )
      hb_errRT_DBCMD( EG_NOTABLE, EDBCMD_NOTABLE, NULL, "SX_DBFDECRYPT" );

   if( ! ISNIL( 2 ) )
      iWorkArea = _sx_select( hb_param( 2, HB_IT_ANY ) );

   if( ISCHAR( 1 ) )
   {
      cpPassword = ( PBYTE ) hb_parc( 1 );
      sx_SetPassword( cpPassword );
   }

   hb_retl( sx_DbfDecrypt() );

   if( ! ( iWorkArea == SX_DUMMY_NUMBER ) )
      sx_Select( iWorkArea );
}

//HB_FUNC( SX_DECRYPT )

//Renamed, because of name conflict with Harbour RTL
HB_FUNC( _SX_DECRYPT )
{
   if( ISCHAR( 1 ) )
   {
      if( ISCHAR( 2 ) )
      {
         hb_retc( ( char * ) SX_CONVFUNC( sx_Decrypt( ( PBYTE ) hb_parc( 1 ),  /* pbBuffer */
                                         ( PBYTE ) hb_parc( 2 ),  /* cpPassword */
                                         ( int ) hb_parclen( 1 ) ) ) );
      }
      else
      {
         hb_retc( ( char * ) SX_CONVFUNC( sx_Decrypt( ( PBYTE ) hb_parc( 1 ),  /* pbBuffer */
                                         ( PBYTE ) NULL,          /* cpPassword */
                                         ( int ) hb_parclen( 1 ) ) ) );
      }
   }
   else
      hb_retc( "" );
}
