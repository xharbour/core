/*
 * $Id: browse.prg 9576 2012-07-17 16:41:57Z andijahja $
 */

/*
 * Harbour Project source code:
 * Quick Clipper Browse()
 *
 * Copyright 1999 Antonio Linares <alinares@fivetech.com>
 * www - http://www.harbour-project.org
 *
 * This program is hb_xfree software; you can redistribute it and/or modify
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

#include "inkey.ch"

function sx_Browse( nTop, nLeft, nBottom, nRight )

   local oBrw
   local cOldScreen
   local n, nOldCursor
   local nKey := 0
   local lExit := .f.
   // local lGotKey := .f.
   local bAction

   if !sx_Used()
      return .f.
   end

   sx_GoTop()

   if PCount() < 4
      nTop    := 1
      nLeft   := 0
      nBottom := MaxRow()
      nRight  := MaxCol()
   endif

   nOldCursor := SetCursor( 0 )
   cOldScreen := SaveScreen( nTop, nLeft, nBottom, nRight )

   @ nTop, nLeft TO nBottom, nRight
   @ nTop + 3, nLeft SAY Chr( 198 )
   @ nTop + 3, nRight SAY Chr( 181 )
   @ nTop + 1, nLeft + 1 SAY Space( nRight - nLeft - 1 )

   oBrw := sx_TBrowseDB( nTop + 2, nLeft + 1, nBottom - 1, nRight - 1 )
   oBrw:HeadSep := " " + Chr( 205 )

   for n := 1 to sx_FieldCount()
      oBrw:AddColumn( TBColumnNew( sx_FieldName( n ), sx_FieldBlock( sx_FieldName( n ) ) ) )
   next

   oBrw:ForceStable()

   while ! lExit

      if nKey == 0
         while !oBrw:stabilize() .and. NextKey() == 0
         enddo
      endif

      if NextKey() == 0

         oBrw:forceStable()
         Statline( oBrw )

         nKey := Inkey( 0 )

         if ( bAction := SetKey( nKey ) ) != nil
            Eval( bAction, ProcName( 1 ), ProcLine( 1 ), "" )
            loop
         endif
      else
         nKey := Inkey()
      endif

      switch nKey
         case K_ESC
            lExit := .t.
            exit

         case K_UP
            oBrw:Up()
            exit

         case K_DOWN
            oBrw:Down()
            exit

         case K_END
            oBrw:End()
            exit

         case K_HOME
            oBrw:Home()
            exit

         case K_LEFT
            oBrw:Left()
            exit

         case K_RIGHT
            oBrw:Right()
            exit

         case K_PGUP
            oBrw:PageUp()
            exit

         case K_PGDN
            oBrw:PageDown()
            exit

         case K_CTRL_PGUP
            oBrw:GoTop()
            exit

         case K_CTRL_PGDN
            oBrw:GoBottom()
            exit

         case K_CTRL_LEFT
            oBrw:panLeft()
            exit

         case K_CTRL_RIGHT
            oBrw:panRight()
            exit

         case K_CTRL_HOME
            oBrw:panHome()
            exit

         case K_CTRL_END
            oBrw:panEnd()
            exit

      end
   end

   RestScreen( nTop, nLeft, nBottom, nRight, cOldScreen )
   SetCursor( nOldCursor )

return .t.

static procedure Statline( oBrw )

   local nTop   := oBrw:nTop - 1
   local nRight := oBrw:nRight

   @ nTop, nRight - 27 SAY "Record "

   if sx_LastRec() == 0
      @ nTop, nRight - 20 SAY "<none>               "
   elseif sx_RecNo() == sx_LastRec() + 1
      @ nTop, nRight - 40 SAY "         "
      @ nTop, nRight - 20 SAY "                <new>"
   else
      @ nTop, nRight - 40 SAY iif( sx_Deleted(), "<Deleted>", "         " )
      @ nTop, nRight - 20 SAY PadR( LTrim( Str( sx_RecNo() ) ) + "/" +;
                                    Ltrim( Str( sx_LastRec() ) ), 16 ) +;
                              iif( oBrw:hitTop, "<bof>", "     " )
   endif

return

