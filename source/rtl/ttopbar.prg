/*
 * $Id: ttopbar.prg 9935 2013-03-21 08:28:29Z zsaulius $
 */

/*
 * Harbour Project source code:
 * TOPBAR menu class
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

#include "box.ch"
#include "button.ch"
#include "color.ch"
#include "common.ch"
#include "inkey.ch"
#include "setcurs.ch"
#include "getexit.ch"
#include "hbclass.ch"

#ifdef HB_COMPAT_C53

#define HB_DEBUG_MENU_COLORS  "N/BG, W+/N, GR+/BG, GR+/N, N/BG, N/BG"

/* TOFIX: Harbour doesn't check if the colorSpec instance
          var has always six pairs of colors. It should
          do so and throw an error. [jlalin]
*/

/* NOTE: In the Get* methods we are breaking the "only one return" rule. I
         know this isn't a good practice however we are eliminating a variable,
         an exit statement and two assigments which is good for speed critical
         and small functions. [jlalin]
*/

/*
*
*  (according to MenuSys of Clipper 5.3b)
*  Code in C-style comment blocks may be uncommented to test non-CUA
*  menuing behavior which permits returning to the selected menu item.
*
*/

/***
*
*  MenuModal( <oTopMenu>, <nSelection>, <nMsgRow>, <nMsgLeft>, ;
*             <nMsgRight>, cMsgColor, GetList ) --> nReturn
*
*  Standard Menu System Modal handling for Menu Items
*
***/

FUNCTION MenuModal( oTopMenu, nSelection, nMsgRow, nMsgLeft, nMsgRight, ;
                    cMsgColor, GetList )

RETURN oTopMenu:Modal( nSelection, nMsgRow, nMsgLeft, nMsgRight, ;
                       cMsgColor, GetList )

//--------------------------------------------------------------------------//
FUNCTION TopBar( nRow, nLeft, nRight )


   LOCAL oTopBar := nil

   IF ISNUMBER( nRow ) .AND. ISNUMBER( nLeft ) .AND. ISNUMBER( nRight )
      oTopBar := TopBarMenu():New( nRow, nLeft, nRight )
   ENDIF

RETURN oTopBar

//--------------------------------------------------------------------------//
CLASS TopBarMenu

   DATA ClassName       init    "TOPBARMENU"
   DATA aItems          init    {}
   DATA cargo
   DATA colorSpec       init    "N/W,W/N,W+/W,W+/N,N+/W,W/N"
   DATA current         init    0
   DATA itemCount       init    0
   DATA left
   DATA right
   DATA row

   // Additional Data to simulate MenuSys.prg of C53

   DATA oMenu
   DATA aMenuList
   DATA nMenuLevel
   DATA nOldRow         PROTECTED
   DATA nOldCol         PROTECTED
   DATA nOldCursor      PROTECTED
   DATA lOldMsgFlag
   DATA cOldMessage

   /* Hold the value of the last TopBarItem position:
   DATA nTopBar         INIT 1
   Array of PopUp MenuItem positions.
   DATA aPopUp          INIT {} */

   METHOD New( nRow, nLeft, nRight )
   METHOD AddItem( oItem )
   METHOD DelItem( nPos )
   METHOD Display()
   METHOD GetAccel( nKey )
   METHOD GetFirst()
   METHOD GetItem( nPos )
   METHOD GetLast()
   METHOD GetNext()
   METHOD GetPrev()

   /* NOTE: This method exists but it is not
            documented in the manuals nor the NG's [jlalin]
   */
   METHOD GetShortct( nKey )

   METHOD HitTest( nRow, nCol )
   METHOD InsItem( nPos, oItem )
   MESSAGE Select( nPos )       METHOD _Select( nPos )
   METHOD SetItem( nPos, oItem )
   METHOD __AltToKey( nKey )    PROTECTED

   // Additional Data to simulate MenuSys.prg of C53

   METHOD ModalGet( nSelect, oMenuMsg )
   METHOD Modal( nSelection, nMsgRow, nMsgLeft, nMsgRight, ;
                 cMsgColor, GetList )
   METHOD IsShortCut( nKey, nID )
   METHOD IsQuick( nKey, nID )
   METHOD PushMenu( lSelect )
   METHOD PopMenu()
   METHOD PopChild( nNewLevel )
   METHOD PopAll()
   METHOD Execute()
   METHOD MHitTest( oNewMenu, nNewLevel, nNewItem )

ENDCLASS

//--------------------------------------------------------------------------//
METHOD New( nRow, nLeft, nRight ) CLASS TopBarMenu

   ::aItems    := {}
   ::colorSpec := "N/W,W/N,W+/W,W+/N,N+/W,W/N"
   ::current   := 0
   ::itemCount := 0
   ::left      := nLeft
   ::right     := nRight
   ::row       := nRow

RETURN Self

//--------------------------------------------------------------------------//
METHOD AddItem( oItem ) CLASS TopBarMenu

   LOCAL oLast

   ::itemCount++
   IF ::itemCount == 1
      oItem:column := ::left
   ENDIF

   IF ::itemCount > 1
      oLast := ATail( ::aItems )
      oItem:column := oLast:column + Len( StrTran( oLast:caption, "&", "" ) ) + 2
   ENDIF

   aAdd( ::aItems, oItem )

RETURN Self

//--------------------------------------------------------------------------//
METHOD DelItem( nPos ) CLASS TopBarMenu

   IF nPos > 0 .AND. nPos <= Len( ::aItems )
      aDel( ::aItems, nPos, .T. )
      ::itemCount--
   ENDIF

RETURN Self

//--------------------------------------------------------------------------//
METHOD GetFirst() CLASS TopBarMenu

   LOCAL aItems

   FOR EACH aItems IN ::aItems
      IF aItems:enabled
         RETURN HB_EnumIndex()
      ENDIF
   next

RETURN 0

//--------------------------------------------------------------------------//
METHOD GetItem( nPos ) CLASS TopBarMenu

   LOCAL oItem

   IF nPos > 0 .AND. nPos <= ::itemCount
      oItem := ::aItems[ nPos ]
   ENDIF

RETURN oItem

//--------------------------------------------------------------------------//
METHOD GetLast() CLASS TopBarMenu

   LOCAL n

   for n := ::itemCount to 1 step -1
      IF ::aItems[ n ]:enabled
         RETURN n
      ENDIF
   next

RETURN 0

//--------------------------------------------------------------------------//
METHOD GetNext() CLASS TopBarMenu

   LOCAL n

   IF ::current < ::itemCount
      for n := ::current + 1 to ::itemCount
         IF ::aItems[ n ]:enabled
            RETURN n
         ENDIF
      next
  ENDIF

RETURN 0

//--------------------------------------------------------------------------//
METHOD GetPrev() CLASS TopBarMenu

   LOCAL n

   IF ::current > 1
      for n := ::current - 1 to 1 step -1
         IF ::aItems[ n ]:enabled
            RETURN n
         ENDIF
      next
   ENDIF

RETURN 0

//--------------------------------------------------------------------------//
/* NOTE: This method corrects two bugs in Cl*pper:
         1) when two menuitems have the same key AND the
            first item is disabled
         2) when a menuitem is disabled it will ignore the key [jlalin]
*/
METHOD GetAccel( nKey ) CLASS TopBarMenu

   LOCAL nAt
   LOCAL cKey  := Upper( ::__AltToKey( nKey ) ) /* By now */
   LOCAL aItems

   FOR EACH aItems IN ::aItems
      nAt := At( "&", aItems:caption )
      IF nAt > 0 .AND. aItems:enabled .AND. ;
            Upper( SubStr( aItems:caption, nAt + 1, 1 ) ) == cKey
         RETURN HB_EnumIndex()
      ENDIF
   next

RETURN 0

//--------------------------------------------------------------------------//
/* NOTE: In my tests I can't get other values than HTNOWHERE or a value
         greather than 0 (selected item), althought the NG's says that
         it RETURNs other HT* values [jlalin]

         This method correct a bug in Cl*pper:
         when click on a disabled menuitem it will ignore it [jlalin]
*/
METHOD HitTest( nRow, nCol ) CLASS TopBarMenu

   LOCAL aItems

   IF ::row == nRow
      FOR EACH aItems IN ::aItems
         IF nCol >= aItems:column .AND. ;
               nCol <= aItems:column + Len( aItems:caption ) .AND. ;
                       aItems:enabled
            RETURN HB_EnumIndex()
         ENDIF
      next
   ENDIF

RETURN HTNOWHERE

//--------------------------------------------------------------------------//
METHOD InsItem( nPos, oItem ) CLASS TopBarMenu
   Local  n
   IF nPos > 0 .AND. nPos <= ::itemCount
//      oItem:column := ::aItems[ nPos - 1 ]:column + Len( StrTran( ::aItems[ nPos - 1 ]:caption, "&", "" ) ) + 2
      aIns( ::aItems, nPos, oItem, .T. )
      ::itemCount++

      For n := nPos to ::itemCount
          ::aItems[ n ]:column := ::aItems[ n - 1 ]:column + Len( StrTran( ::aItems[ n -1 ]:caption, "&", "" ) ) + 2
      NEXT

   ENDIF

RETURN Self

//--------------------------------------------------------------------------//
METHOD _Select( nPos ) CLASS TopBarMenu
   IF ( nPos > 0 .AND. nPos <= ::itemCount ) .AND. ;
         nPos != ::current .AND. ::aItems[ nPos ]:enabled

      IF ::current > 0
         IF ::aItems[ ::current ]:isPopUp()
            ::aItems[ ::current ]:data:Close()
         ENDIF
      ENDIF

      ::current := nPos
   else
      ::current := 0
   ENDIF
RETURN Self

//--------------------------------------------------------------------------//
METHOD SetItem( nPos, oItem ) CLASS TopBarMenu

   IF nPos > 0 .AND. nPos <= ::itemCount
      ::aItems[ nPos ] := oItem
   ENDIF

RETURN Self

//--------------------------------------------------------------------------//
METHOD GetShortct( nKey ) CLASS TopBarMenu

   LOCAL aItems

   FOR EACH aItems IN ::aItems
      IF aItems:shortcut == nKey
         RETURN HB_EnumIndex()
      ENDIF
   next

RETURN 0

//--------------------------------------------------------------------------//
METHOD Display() CLASS TopBarMenu

   LOCAL oPopup
   LOCAL nAt
   LOCAL aItems
   LOCAL cPrompt

   LOCAL nOldRow  := Row()
   LOCAL nOldCol  := Col()
   LOCAL lOldCur  := MSetCursor( FALSE )

   DispBegin()

   DispOutAt( ::row, ::left, ;
              Space( ::right - ::left + 1 ), hb_ColorIndex( ::colorSpec, CLR_STANDARD ) )

   FOR EACH aItems IN ::aItems

      nAt := At( "&", aItems:caption )
      cPrompt := " " + StrTran( aItems:caption, "&", "" ) + " "

      DispOutAt( ;
         ::row, aItems:column, ;
         cPrompt, ;
         hb_ColorIndex( ::colorSpec, ;
            iIF( aItems:enabled, ;
               iIF( HB_EnumIndex() == ::current, CLR_ENHANCED, CLR_STANDARD ), ;
               CLR_UNSELECTED ) ) )

      IF nAt > 0
         DispOutAt( ::row, aItems:column + nAt, ;
            SubStr( aItems:caption, nAt + 1, 1 ), ;
            hb_ColorIndex( ::colorSpec, ;
               iIF( HB_EnumIndex() == ::current, CLR_BACKGROUND, CLR_BORDER ) ) )
      ENDIF

      IF aItems:isPopup()
         aItems:data:SetCoors( ::row + 1, aItems:column, .t. )
      ENDIF

   next

   IF ::current > 0 .AND. ::aItems[ ::current ]:isPopup()
      oPopUp  := ::aItems[ ::current ]:data
      IF oPopUp:isOpen()
         oPopUp:display()
      ENDIF
   ENDIF

   DevPos( nOldRow, nOldCol )
   MSetCursor( lOldCur )

   DispEnd()

RETURN Self
//--------------------------------------------------------------------------//

METHOD __AltToKey( nKey ) CLASS TopBarMenu

   LOCAL nIndex := AScan( { K_ALT_A, K_ALT_B, K_ALT_C, K_ALT_D, K_ALT_E, K_ALT_F,;
                            K_ALT_G, K_ALT_H, K_ALT_I, K_ALT_J, K_ALT_K, K_ALT_L,;
                            K_ALT_M, K_ALT_N, K_ALT_O, K_ALT_P, K_ALT_Q, K_ALT_R,;
                            K_ALT_S, K_ALT_T, K_ALT_U, K_ALT_V, K_ALT_W, K_ALT_X,;
                            K_ALT_Y, K_ALT_Z }, nKey )

RETURN iIF( nIndex > 0, SubStr( "ABCDEFGHIJKLMNOPQRSTUVWXYZ", nIndex, 1 ), "" )

//--------------------------------------------------------------------------//

/* The routines below were added by Larry Sevilla <lsevilla@nddc.edu.ph> */
/* based on the MenuSys.prg of Clipper 5.3b                              */

/***
*
*  IsShortCut( <oMenu>, <nKey>, <nID> ) -> .T. | .F.
*
*  ShortCut processing for initial Get or Menu Item.
*
***/
METHOD IsShortCut( nKey, nID ) CLASS TopBarMenu

      RETURN ( ::IsQuick( nKey, @nID ) )


/***
*
*  IsQuick( <oMenu>, <nKey>, <nID> ) --> .T. | .F.
*
*  IsShortCut() for secondary ShortCut processing.
*  Navigates to the next Get or Menu Item from the
*  Current IF more than one uses the same ShortCut.
*
***/
METHOD IsQuick( nKey, nID ) CLASS TopBarMenu

   LOCAL nItem, nTotal, nShortCut, oItem // , i

   IF ( ( nShortCut := ::GetShortCt( nKey ) ) == 0 )
      nTotal := ::ItemCount

      FOR nItem := 1 TO nTotal
         IF ( !( oItem := ::GetItem( nItem ) ):Enabled )
         ELSEIF ( !( oItem:IsPopUp() ) )
         ELSEIF ( oItem:Data:IsQuick( nKey, @nID ) )
            RETURN ( .T. )
         ENDIF
      NEXT

   ELSEIF ( !( oItem := ::GetItem( nShortCut ) ):IsPopUp() )
      IF oItem:Enabled
         ::Select( nShortCut )
         EVAL( oItem:Data, oItem )
         nID := oItem:ID
         RETURN ( .T. )
      ENDIF

   ENDIF

   RETURN ( .F. )

/***
*  ModalGet
***/
METHOD ModalGet( nSelect, oMsg ) CLASS TopBarMenu

RETURN ::Modal( nSelect, oMsg:Row, oMsg:Left, oMsg:Right, oMsg:Color )

/***
*
*  MenuModal( <oTopMenu>, <nSelection>, <nMsgRow>, <nMsgLeft>, ;
*             <nMsgRight>, cMsgColor, GetList ) --> nRETURN
*
*  StANDard Menu System Modal hANDling for Menu Items
*
***/
METHOD Modal( nSelection, nMsgRow, nMsgLeft, nMsgRight, ;
                    cMsgColor, GetList ) CLASS TopBarMenu

   LOCAL nKey, nNewItem, lLeftDown, oNewMenu, nNewLevel, ;
         nEvent, oMenuItem, nMenuItem, nRETURN, oMenuMsg, ;
         nTemp, bKeyBlock, lSubMenu, cColor, lWhile,nSave

   nRETURN     := 0

   ::nOldRow    := ROW()
   ::nOldCol    := COL()
   ::nOldCursor := SetCursor( SC_NONE )

   oMenuMsg := MenuMssgLine():new( nMsgRow, nMsgLeft, nMsgRight, cMsgColor )

   IF oMenuMsg:Flag
      cColor := setColor( oMenuMsg:Color )
      @ oMenuMsg:Row, oMenuMsg:Left CLEAR TO oMenuMsg:Row, oMenuMsg:Right
      setColor(cColor)
      oMenuMsg:SaveScreen()
   ENDIF

   //::Select( nSelection )

      ::Display()

   IF nSelection <= 0
      WHILE ( nSelection <= 0 )

         nEvent := SET( _SET_EVENTMASK, INKEY_KEYBOARD + INKEY_LDOWN )
         nKey   :=  Inkey( 0 )
         SET( _SET_EVENTMASK, nEvent )

         IF ( nKey == K_LBUTTONDOWN .OR. nKey == K_LDBLCLK )
            nSelection := ::HitTest( MRow(), MCol() )

         ELSEIF ( ( nSelection := ::GetAccel( nKey ) ) != 0 )

         ELSEIF ( ::IsShortCut( nKey, @nRETURN ) )
            // Restore system entry state settings from file-wide array:
            // saStatics := ACLONE( aSavMenuSys )
            RETURN ( nRETURN )

         ELSE
            nSelection := 1

         ENDIF

      ENDDO


   ENDIF
   ::Select( nSelection )
   ::Display()

   IF ( !::GetItem( nSelection ):Enabled )
      // Restore system entry state settings from file-wide array:
      // saStatics := ACLONE( aSavMenuSys )
      RETURN ( 0 )
   ENDIF

   ::oMenu        := Self
   ::aMenuList    := array( 16 )
   ::nMenuLevel   := 1
   ::aMenuList[1] := ::oMenu

   lLeftDown := mLeftDown()

   /* Continue pushing MenuItems until there are no more to push:
   DO WHILE ( ::PushMenu( .T. ) )
   ENDDO */

   oMenuMsg:Show( Self, .T. )

   lWhile := .T.
   WHILE lWhile
      /* Save Current TopBar MenuItem:
      nTopBar := ::Current
      // Save Current PopUp MenuItem:
      ::aPopUp[::nMenuLevel] := ::oMenu:Current */
      nKey := INKEY( 0 )
      // Check for SET KEY first
      IF !( ( bKeyBlock := SETKEY( nKey ) ) == NIL )
         EVAL( bKeyBlock, PROCNAME(1), PROCLINE(1), "" )
         LOOP
      ENDIF

      Switch nKey
      case K_MOUSEMOVE
         //====== mouse movement.

         IF ( lLeftDown )
            // Changed '!HitTest' to '!MHitTest':
            IF ( ! ::MHitTest( @oNewMenu, @nNewLevel, @nNewItem ) )
               //------------ hit nowhere.

            ELSEIF ( nNewLevel != ::nMenuLevel )
               //------------ menu level change.

               IF ( nNewItem == oNewMenu:Current )
               ELSEIF ( oNewMenu:GetItem( nNewItem ):Enabled )
                  ::oMenu := oNewMenu
                  ::PopChild( nNewLevel )
                  ::oMenu:Select( nNewItem )
                  ::oMenu:Display()
                  ::PushMenu( .T. )
                  oMenuMsg:Show( Self, .T. )
               ENDIF

            ELSEIF ( nNewItem != oNewMenu:Current() )
               //------------ menu item change.

               ::PopChild( ::nMenuLevel )

               IF ( ::oMenu:GetItem( nNewItem ):Enabled )
                  ::oMenu:Select( nNewItem )
                  ::oMenu:Display()
                  ::PushMenu( .T. )
                  oMenuMsg:Show( Self, .T. )
               ENDIF

            ENDIF

         ENDIF
         EXIT

      case K_DOWN
         //====== down arrow key.

         IF ::oMenu:ClassName() == "TOPBARMENU"
            IF ( ::PushMenu( .T. ) )
               oMenuMsg:Show( Self, .T. )
            ENDIF
         ELSE  // IF ( saStatics[ SNMENULEVEL ] > 1 )
            nTemp := ::oMenu:GetNext()
            IF ( nTemp == 0 )
               nTemp := ::oMenu:GetFirst()
            ENDIF
            ::oMenu:Select( nTemp )
            ::oMenu:Display()
            oMenuMsg:Show( Self, .T. )

         ENDIF
         EXIT

      case K_UP
         //====== up arrow key.

         IF ::oMenu:ClassName() != "TOPBARMENU"
            nTemp := ::oMenu:GetPrev()
            IF ( nTemp == 0 )
               nTemp := ::oMenu:GetLast()
            ENDIF
            ::oMenu:Select( nTemp )
            ::oMenu:Display()
            oMenuMsg:Show( Self, .T. )

         ENDIF
         EXIT

      case K_LEFT
         //====== left arrow key.
         /** Reset PopUp MenuItem to 1:
         ::aPopUp[::nMenuLevel] := 1 **/
         IF ( lSubMenu := ::nMenuLevel > 1 )
            ::PopMenu()
         ENDIF
         IF ::oMenu:ClassName() == "TOPBARMENU"
            nTemp := ::oMenu:GetPrev()
            IF ( nTemp == 0 )
              nTemp := ::oMenu:GetLast()
            ENDIF
            ::oMenu:Select( nTemp )
            ::oMenu:Display()
            IF lSubMenu
               ::PushMenu( .T. )
            ENDIF
         ENDIF
         oMenuMsg:Show( Self, .T. )
         EXIT

      case K_RIGHT
         //====== right arrow key.
         /* Reset PopUp MenuItem to 1:
         ::aPopUp[ ::nMenuLevel ] := 1 */
         IF ( lSubMenu := ::nMenuLevel > 1 )
            /* Test !PopMenu() to determine calling PopAll():
            IF !( ::PopMenu() )
               ::PopAll()
            ENDIF */
            ::PopMenu()
         ENDIF

         IF ::oMenu:ClassName() == "TOPBARMENU"
            nTemp := ::oMenu:GetNext()
            IF ( nTemp == 0 )
               nTemp := ::oMenu:GetFirst()
            ENDIF
            ::oMenu:Select( nTemp )
            ::oMenu:Display()
            IF lSubMenu
               ::PushMenu( .T. )
            ENDIF
         ENDIF
         oMenuMsg:Show( Self, .T. )
         EXIT

      case K_ENTER
         //====== enter key .

         IF ( ::PushMenu( .T. ) )
            oMenuMsg:Show( Self, .T. )

         ELSE
            nSave:=::Current
            oMenuMsg:Show( Self, .F. )
            nRETURN := ::Execute()
            IF nSave=0
              nSave:=1
              ::Current:=0
            ENDIF
            ::Select(nSave)
// A menu should always exit after an execute call (Clipper compatibility)
//            IF ( nRETURN != 0 )
               lWhile := .F.
               ::Current:=0
               EXIT
//            ENDIF

         ENDIF
         EXIT

      case K_ESC
         //====== escape key - go to previous menu

         IF ::PopMenu()
            ::oMenu:Display()
            oMenuMsg:Show( Self, .T. )
         ELSE

            IF ::oMenu:ClassName() == "POPUPMENU"
               ::oMenu:Close()
            ENDIF
            // Bail out IF at the top menu item:
            nRETURN := -1
            lWhile  := .F.
            EXIT

         ENDIF
         EXIT

      case K_LBUTTONDOWN
         //====== mouse left button press.

         // Changed '!HitTest' to '!MHitTest':
         IF ( ! ::MHitTest( @oNewMenu, @nNewLevel, @nNewItem ) )
            // Added the following to test Get System HitTest():
            IF ( !( GetList == NIL ) .AND. ;
               HitTest( GetList, MRow(), MCol(), oMenuMsg:aMsg ) != 0 )
               GETACTIVE():ExitState := GE_MOUSEHIT
               // Reset Get System values:
               // ReadStats( SNLASTEXIT, GE_MOUSEHIT )
               __GetListActive():nLastExitState := GE_MOUSEHIT
               // IF ReadStats( SOMENU ):ClassName() == "POPUPMENU"
               IF ::oMenu:ClassName() == "POPUPMENU"
                  ::PopMenu()
               ENDIF
               // ENDIF
               nRETURN := -1
               lWhile  := .F.
               EXIT
            ENDIF

            //// Warning: New test for hitting NOWHERE on the Canvas area!!!
            IF ::oMenu:ClassName() == "POPUPMENU"
               ::PopMenu()
            ENDIF

         ELSEIF ( nNewLevel == ::nMenuLevel )
            ::oMenu:Select( nNewItem )
            ::oMenu:Display()
            ::PushMenu(.T.)
            oMenuMsg:Show( Self, .T. )

         ELSE
            ::nMenuLevel := nNewLevel
            ::oMenu      := ::aMenuList[ ::nMenuLevel ]

            nMenuItem := ::oMenu:Current
            //oMenuItem := ::oMenu:GetItem( nMenuItem )
            IF ( ( oMenuItem := ::oMenu:GetItem( ::oMenu:Current ) ):IsPopUp() )
               oMenuItem:Data:Close()
            ENDIF

            IF !( nMenuItem == nNewItem )
               //nMenuItem := nNewItem
               ::oMenu:Select( nNewItem )
               ::oMenu:Display()
               ::PushMenu( .T. )
            ENDIF

            oMenuMsg:Show( Self, .T. )
         ENDIF

         lLeftDown := .T.
         EXIT

      case K_LBUTTONUP
         //====== mouse left button release.

         lLeftDown := .F.

         // Changed '!HitTest' to '!MHitTest':
         IF ( ! ::MHitTest( @oNewMenu, @nNewLevel, @nNewItem ) )
         ELSEIF ( nNewLevel == ::nMenuLevel )
            IF ( nNewItem == ::oMenu:Current )
               oMenuMsg:Show( Self, .F. )
               nRETURN := ::Execute()
// A menu should always exit after an execute call (Clipper compatibility)
//               IF ( nRETURN != 0 )
                  lWhile := .F.
                  EXIT
//               ENDIF
            ENDIF

         ELSE

            /* This may not be necessary:
            nNewItem := ::oMenu:GetFirst()
            IF ( nNewItem == 0 )
            ELSE
               ::oMenu:Select(  nNewItem )
               ::oMenu:Display()
               oMenuMsg:Show( Self, .T. )
            ENDIF */

         ENDIF
         EXIT

      Default

         IF ( ( nNewItem := ::oMenu:GetAccel( nKey ) ) != 0 )
            //=== check for menu item accelerator key.

            IF ::oMenu:GetItem( nNewItem ):Enabled
               ::oMenu:Select( nNewItem )
               ::oMenu:Display()

               /* Code insert to Debug selected menu item in graphics mode:
               @ 20, 70 SAY STR( nNewItem, 3 )
               INKEY(0)
               @ 20, 70 SAY SPACE( 3 ) */

               IF ( ! ::PushMenu( .T. ) )
                  oMenuMsg:Show( Self, .F. )
                  nRETURN := ::Execute()
// A menu should always exit after an execute call (Clipper compatibility)
//                  IF ( nRETURN != 0 )
                     lWhile := .F.
                     EXIT
//                  ENDIF
               ENDIF
               oMenuMsg:Show( Self, .T. )

            ENDIF

         ELSEIF ::IsShortCut( nKey, @nRETURN )

            IF ( nRETURN != 0 )
               lWhile := .F.
               EXIT
            ENDIF

         // Added the following to test Get System HitTest():
         ELSEIF !( GetList == NIL ) .AND. ;
            ( ( nNewItem := Accelerator( GetList, nKey, oMenuMsg:aMsg ) ) != 0 )
            GETACTIVE():ExitState := GE_SHORTCUT

            // Reset Get System values:
            // ReadStats( SNNEXTGET, nNewItem )
            __GetListActive():nNextGet := nNewItem
            // IF ReadStats( SOMENU ):ClassName() == "POPUPMENU"
            IF ::oMenu:ClassName() == "POPUPMENU"
               ::PopMenu()
            ENDIF
            // ENDIF
            nRETURN := -1
            lWhile  := .F.
            EXIT

         ELSEIF ( nNewItem := ::GetAccel( nKey ) ) != 0
            //=== check for the top menu item accelerator key

            IF ::GetItem( nNewItem ):Enabled
               ::PopAll()
               ::oMenu:Select( nNewItem )
               ::Display()
               IF ( ::GetItem( nNewItem ):IsPopUp() )
                  ::PushMenu( .T. )
               ELSE
                  oMenuMsg:Show( Self, .F. )
                  nRETURN := ::Execute()
// A menu should always exit after an execute call (Clipper compatibility)
//                  IF ( nRETURN != 0 )
                     lWhile := .F.
                     EXIT
//                  ENDIF
               ENDIF
               oMenuMsg:Show( Self, .T. )
            ENDIF
         ENDIF

      END

   ENDDO

   IF oMenuMsg:Flag
      oMenuMsg:RestScreen()
   ENDIF

   ::PopAll()
   SetPos( ::nOldRow, ::nOldCol )
   SetCursor( ::nOldCursor )
   // Restore system entry state settings from file-wide array:
   // saStatics := ACLONE( aSavMenuSys )

   RETURN ( nRETURN )

/***
*
*  PushMenu( <lSelect>, <oTopMenu> ) --> .T. | .F.
*
*  Increment saStatics[ SNMENULEVEL ] AND optionally select first item.
*  IF selected MenuItem IsPopUp, assign saStatics[ SOMENU ].
*
***/
METHOD PushMenu( lSelect ) CLASS TopBarMenu
   LOCAL oNewMenu

   oNewMenu := ::oMenu:GetItem( ::oMenu:Current )

   IF ( !( ValType( oNewMenu ) == "O" ) )
   ELSEIF ( oNewMenu:IsPopUp )

      IF ( !( ValType( lSelect ) == "L" ) )
         lSelect := .F.
      ENDIF

      ::oMenu := oNewMenu:Data
      ::aMenuList[ ++::nMenuLevel ] := ::oMenu

      /* IF entering a new level:
      IF LEN( ::aPopUp ) < ::nMenuLevel
         // Resize the array:
         ASIZE( ::aPopUp, ::nMenuLevel )
         // Reset MenuItem to 1:
         ::aPopUp[ ::nMenuLevel ] := 1
      ENDIF */

      IF ( lSelect )
         ::oMenu:Select( ::oMenu:GetFirst() )
         /* Select previous item:
         ::oMenu:Select( ::aPopUp[ ::nMenuLevel ) */
      ELSE
         ::oMenu:Select( 0 )

      ENDIF

      IF !( ::oMenu:IsOpen )
         ::oMenu:Open()
      ENDIF

      RETURN ( .T. )

   ENDIF

   RETURN ( .F. )


/***
*
*  PopMenu( <oTopMenu> ) --> .T. | .F.
*
*  Close SubMenuItem AND RETURN to the upper MenuItem level.
*
***/
METHOD PopMenu() CLASS TopBarMenu
   IF ( ::nMenuLevel > 1 )
      ::oMenu:Select(0)
      ::oMenu:Close(.T.)
      // Decrement MenuItem level AND assign:
      ::oMenu := ::aMenuList[ --::nMenuLevel ]
      RETURN ( .T. )

   ENDIF

   RETURN ( .F. )


/***
*
*  PopChild( <nNewLevel>, <oTopMenu> ) --> .T. | .F.
*
*  Close PopUp Child MenuItem AND RETURN to the upper MenuItem level.
*
***/
METHOD PopChild( nNewLevel ) CLASS TopBarMenu
   LOCAL oOldMenuItem, nCurrent

   IF ( ( nCurrent := ::oMenu:Current ) != 0 )
      oOldMenuItem := ::oMenu:GetItem( nCurrent )
      IF ( oOldMenuItem:IsPopUp )
         oOldMenuItem:Data:Close()
         ::nMenuLevel := nNewLevel
         RETURN ( .T. )
      ENDIF

   ENDIF

   RETURN ( .F. )


/***
*
*  PopAll( <oTopMenu> ) --> .T.
*
*  Close all Menus below Top Menu AND RETURN to upper MenuItem level.
*
***/
METHOD PopAll( ) CLASS TopBarMenu
   IF ::aMenuList[2] != NIL
      ::aMenuList[2]:Close()
   ENDIF
   // Set the menu level AND position relative to the top menu item:
   ::nMenuLevel := 1
   ::oMenu      := ::aMenuList[ 1 ]

   RETURN ( .T. )

/***
*
*  Execute( <oTopMenu> ) --> oNewMenu:Id | 0
*
*  EVAL()uate the Data block IF selected MenuItem is !IsPopUp.
*
***/
METHOD Execute() CLASS TopBarMenu

   LOCAL oNewMenu, nCurrent := 1, lPas := .T.
   oNewMenu := ::oMenu:GetItem( ::oMenu:Current )

   IF ( !( ValType( oNewMenu ) == "O" ) )
   // Execute the Data block IF selected MenuItem is !IsPopUp:
   ELSEIF ( !oNewMenu:IsPopUp )
      IF ( ::oMenu:ClassName() $ "TOPBARMENU_POPUPMENU" )
         SETPOS( ::nOldRow, ::nOldCol )
         SETCURSOR( ::nOldCursor )
         EVAL( oNewMenu:Data, oNewMenu )
         SETCURSOR( SC_NONE )
         lPas := .F.
      ENDIF

      // Pop the Menu:
      IF ::PopMenu()
         nCurrent := ::oMenu:Current
      ENDIF

      ::oMenu:Select( nCurrent )
      // Display newly selected current menu item:
      IF ( ::oMenu:ClassName() == "POPUPMENU" .AND. ;
         ::nMenuLevel == 1 .AND. ! ::oMenu:IsOpen )
         ::oMenu:Open()
      ENDIF

      IF lPas
         ::oMenu:Close()
         SETPOS( ::nOldRow, ::nOldCol )
         SETCURSOR( ::nOldCursor )
         EVAL( oNewMenu:Data, oNewMenu )
         SETCURSOR( SC_NONE )

      ENDIF

      RETURN ( oNewMenu:Id )

   ENDIF

   RETURN ( 0 )

/***
*
*  MHitTest( <oNewMenu>, <nNewLevel>, <nNewItem>, <oTopMenu> ) --> .T. | .F.
*
*  Test to find the Mouse LOCALtion.
*  Note: Formal parameters received here were passed by reference.
*
***/
METHOD MHitTest( oNewMenu, nNewLevel, nNewItem ) CLASS TopBarMenu

   FOR nNewLevel := ::nMenuLevel TO 1 STEP -1
      oNewMenu   := ::aMenuList[ nNewLevel ]
      nNewItem   := oNewMenu:HitTest( MRow(), MCol() )
      IF ( nNewItem < 0 )
         // Test for the mouse on Menu separator or border
         RETURN ( .F. )

      ELSEIF   ( nNewItem > 0 ) .AND. oNewMenu:GetItem( nNewItem ):Enabled
         // Test for the mouse on an enabled item in the menu
         RETURN ( .T. )

      ENDIF

   NEXT

   RETURN ( .F. )

#ENDIF
