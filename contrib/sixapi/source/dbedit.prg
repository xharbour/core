/*
 * $Id: dbedit.prg 9576 2012-07-17 16:41:57Z andijahja $
 */

/*
 * Harbour Project source code:
 * DBEDIT() function
 *
 * Copyright 1999 {list of individual authors and e-mail addresses}
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

#include "common.ch"
#include "dbedit.ch"
#include "inkey.ch"
#include "setcurs.ch"

#define HB_DBEMPTY() ( sx_LastRec() == 0 .OR. ( ( sx_Eof() .OR. sx_RecNo() == sx_LastRec() + 1 ) .AND. sx_Bof() ) )

/* NOTE: Extension: Harbour supports codeblocks as the xUserFunc parameter
         [vszakats] */
/* NOTE: Clipper is buggy and will throw an error if the number of
         columns is zero. (Check: dbEdit(0,0,20,20,{})) [vszakats] */
/* NOTE: Clipper will throw an error if there's no database open [vszakats] */
/* NOTE: The NG says that the return value is NIL, but it's not. [vszakats] */
/* NOTE: There's an undocumented result code in Clipper (3), which is not
         supported in Harbour. [vszakats] */
/* NOTE: Harbour is multithreading ready/reentrant, Clipper is not.
         [vszakats] */

FUNCTION sx_dbEdit(;
      nTop,;
      nLeft,;
      nBottom,;
      nRight,;
      acColumns,;
      xUserFunc,;
      xColumnSayPictures,;
      xColumnHeaders,;
      xHeadingSeparators,;
      xColumnSeparators,;
      xFootingSeparators,;
      xColumnFootings )

   LOCAL oBrowse
   LOCAL nKey
   LOCAL bAction
   LOCAL lException

   LOCAL nOldCursor

   LOCAL nPos
   LOCAL nColCount
   LOCAL oColumn
   LOCAL nAliasPos
   LOCAL cAlias
   LOCAL cFieldName
   LOCAL cHeading
   // LOCAL cBlock
   LOCAL bBlock
   LOCAL aSubArray
   LOCAL lWhile

   IF !sx_Used()
      RETURN .F.
   ENDIF

   sx_GoTop()

   /* ------------------------------------------------------ */
   /* Set up the environment, evaluate the passed parameters */
   /* ------------------------------------------------------ */

   IF !ISNUMBER( nTop ) .OR. nTop < 0
      nTop := 0
   ENDIF
   IF !ISNUMBER( nLeft ) .OR. nLeft < 0
      nLeft := 0
   ENDIF
   IF !ISNUMBER( nBottom ) .OR. nBottom > MaxRow() .OR. nBottom < nTop
      nBottom := MaxRow()
   ENDIF
   IF !ISNUMBER( nRight ) .OR. nRight > MaxCol() .OR. nRight < nLeft
      nRight := MaxCol()
   ENDIF

   oBrowse := sx_TBrowseDB( nTop, nLeft, nBottom, nRight )

   oBrowse:SkipBlock := {| nRecs | sx_dbSkipper( nRecs ) }
   oBrowse:HeadSep   := iif( ISCHARACTER( xHeadingSeparators ), xHeadingSeparators, Chr( 205 ) + Chr( 209 ) + Chr( 205 ) )
   oBrowse:ColSep    := iif( ISCHARACTER( xColumnSeparators ), xColumnSeparators, " " + Chr( 179 ) + " " )
   oBrowse:FootSep   := iif( ISCHARACTER( xFootingSeparators ), xFootingSeparators, "" )
   oBrowse:AutoLite  := .F. /* Set to .F. just like in CA-Cl*pper. [vszakats] */

   // Calculate the number of columns

   IF ISARRAY( acColumns )
      nColCount := Len( acColumns )
      aSubArray:=acColumns[nColCount] // See if is an Array of Array

     IF ISARRAY( aSubArray )
        nColCount := Len( aSubArray )
      nPos := 1
      DO WHILE nPos <= nColCount .AND. ISCHARACTER( aSubArray[ nPos ] ) .AND. !Empty( aSubArray[ nPos ] )
         nPos++
      ENDDO
      nColCount := nPos - 1

      IF nColCount == 0
         RETURN .F.
      ENDIF
      acColumns:=aSubArray
      else
      nPos := 1
      DO WHILE nPos <= nColCount .AND. ISCHARACTER( acColumns[ nPos ] ) .AND. !Empty( acColumns[ nPos ] )
         nPos++
      ENDDO
      nColCount := nPos - 1

      IF nColCount == 0
         RETURN .F.
      ENDIF

      endif
   ELSE
      nColCount := sx_FieldCount()
   ENDIF

   // Generate the TBrowse columns
   FOR nPos := 1 TO nColCount

      /* 09/02/2002 <maurilio.longo@libero.it>
         NOTE: I've removed all code which was here trying to guess content of acColumns[nPos], it
               was not needed and it was not working.
               Clipper dbEdit() requires fully qualified field names if there are columns from more than
               one file or throws an error
               Clipper dbEdit() is not able to change field values

         EXAMPLE: a.dbf has a single field named a
                  b.dbf has a single field named b

                  use a alias "filea" new
                  use b alias "fileb" new

                  aF := { "field->a", "field->b" }
                  dbEdit(,,,, aF)

                  throws an error with Clipper 5.2
      */

      // Column Header
      IF ISARRAY( acColumns )
         IF ( nAliasPos := At( "->", acColumns[ nPos ] ) ) > 0
            cAlias := SubStr( acColumns[ nPos ], 1, nAliasPos - 1 )
            cFieldName := SubStr( acColumns[ nPos ], nAliasPos + 2 )
            cHeading := cAlias + "->;" + cFieldName
         ELSE
            cHeading := acColumns[ nPos ]
         ENDIF
      ELSE
         cHeading := sx_FieldName( nPos )
      ENDIF

      IF ISARRAY( acColumns )
         bBlock := &( "{||" + acColumns[ nPos ] + "}" )
      ELSE
         bBlock := sx_FieldBlock( sx_FieldName( nPos ) )
      ENDIF

      IF ValType(Eval(bBlock)) == "M"
         bBlock := {|| "  <Memo>  " }
      ENDIF

      IF ISARRAY( xColumnHeaders ) .AND. Len( xColumnHeaders ) >= nPos .AND. ISCHARACTER( xColumnHeaders[ nPos ] )
         cHeading := xColumnHeaders[ nPos ]
      ELSEIF ISCHARACTER( xColumnHeaders )
         cHeading := xColumnHeaders
      ENDIF

      oColumn := TBColumnNew( cHeading, bBlock )

      IF ISARRAY( xColumnSayPictures ) .AND. nPos <= Len( xColumnSayPictures ) .AND. ISCHARACTER( xColumnSayPictures[ nPos ] ) .AND. !Empty( xColumnSayPictures[ nPos ] )
         oColumn:Picture := xColumnSayPictures[ nPos ]
      ELSEIF ISCHARACTER( xColumnSayPictures ) .AND. !Empty( xColumnSayPictures )
         oColumn:Picture := xColumnSayPictures
      ENDIF

      IF ISARRAY( xColumnFootings ) .AND. nPos <= Len( xColumnFootings ) .AND. ISCHARACTER( xColumnFootings[ nPos ] )
         oColumn:Footing := xColumnFootings[ nPos ]
      ELSEIF ISCHARACTER( xColumnFootings )
         oColumn:Footing := xColumnFootings
      ENDIF

      IF ISARRAY( xHeadingSeparators ) .AND. nPos <= Len( xHeadingSeparators ) .AND. ISCHARACTER( xHeadingSeparators[ nPos ] )
         oColumn:HeadSep := xHeadingSeparators[ nPos ]
      ENDIF

      IF ISARRAY( xColumnSeparators ) .AND. nPos <= Len( xColumnSeparators ) .AND. ISCHARACTER( xColumnSeparators[ nPos ] )
         oColumn:ColSep := xColumnSeparators[ nPos ]
      ENDIF

      IF ISARRAY( xFootingSeparators ) .AND. nPos <= Len( xFootingSeparators ) .AND. ISCHARACTER( xFootingSeparators[ nPos ] )
         oColumn:FootSep := xFootingSeparators[ nPos ]
      ENDIF

      oBrowse:AddColumn( oColumn )

   NEXT

   /* --------------------------- */
   /* Go into the processing loop */
   /* --------------------------- */

   IF sx_Eof()
      sx_GoBottom()
   ENDIF

   nOldCursor := SetCursor( SC_NONE )
   lException := .F.

   lWhile := .T.
   DO WHILE lWhile

      DO WHILE !oBrowse:Stabilize() .AND. NextKey() == 0
      ENDDO

      IF ( nKey := InKey() ) == 0

         IF !lException
            IF !dbEditCallUser( oBrowse, xUserFunc, 0 )
               oBrowse:forceStable()
               EXIT
            ENDIF
            oBrowse:forceStable()
         ENDIF

         oBrowse:Hilite()
         nKey := InKey( 0 )
         oBrowse:DeHilite()

         IF ( bAction := SetKey( nKey ) ) != NIL
            Eval( bAction, ProcName( 1 ), ProcLine( 1 ), "" )
            LOOP
         ENDIF

      ENDIF

      lException := .F.
      SWITCH nKey
         CASE K_DOWN       ; oBrowse:Down() ; exit
         CASE K_UP         ; oBrowse:Up() ; exit
         CASE K_PGDN       ; oBrowse:PageDown() ; exit
         CASE K_PGUP       ; oBrowse:PageUp() ; exit
         CASE K_CTRL_PGUP  ; oBrowse:GoTop() ; exit
         CASE K_CTRL_PGDN  ; oBrowse:GoBottom() ; exit
         CASE K_RIGHT      ; oBrowse:Right() ; exit
         CASE K_LEFT       ; oBrowse:Left() ; exit
         CASE K_HOME       ; oBrowse:Home() ; exit
         CASE K_END        ; oBrowse:End() ; exit
         CASE K_CTRL_LEFT  ; oBrowse:PanLeft() ; exit
         CASE K_CTRL_RIGHT ; oBrowse:PanRight() ; exit
         CASE K_CTRL_HOME  ; oBrowse:PanHome() ; exit
         CASE K_CTRL_END   ; oBrowse:PanEnd() ; exit
         DEFAULT
            IF !dbEditCallUser( oBrowse, xUserFunc, nKey )
               lWhile := .F.
            ENDIF
            lException := .T.
      END

   ENDDO

   SetCursor( nOldCursor )

   RETURN .T.

STATIC FUNCTION dbEditCallUser( oBrowse, xUserFunc, nKey )
   LOCAL nMode
   LOCAL nResult
   LOCAL nPrevRecNo

   DO CASE
   CASE nKey != 0           ; nMode := DE_EXCEPT
   CASE HB_DBEMPTY()        ; nMode := DE_EMPTY
   CASE oBrowse:hitBottom() ; nMode := DE_HITBOTTOM
   CASE oBrowse:hitTop()    ; nMode := DE_HITTOP
   OTHERWISE                ; nMode := DE_IDLE
   ENDCASE

   oBrowse:forceStable()

   nPrevRecNo := sx_RecNo()

   IF ( ISCHARACTER( xUserFunc ) .AND. !Empty( xUserFunc ) ) .OR. ISBLOCK( xUserFunc )
      nResult := Do( xUserFunc, nMode, oBrowse:ColPos() )
   ELSE
      nResult := iif( nKey == K_ENTER .OR. nKey == K_ESC, DE_ABORT, DE_CONT )
   ENDIF

   IF sx_Eof() .AND. !HB_DBEMPTY()
      sx_Skip( -1 )
   ENDIF

   IF nResult == DE_REFRESH .OR. nPrevRecNo != sx_RecNo()

      IF nResult != DE_ABORT

         IF Set( _SET_DELETED ) .AND. sx_Deleted() .OR. !Empty( dbFilter() ) .AND. ! &( dbFilter() )
            sx_Skip( 1 )
         ENDIF

         IF sx_Eof()
            sx_GoBottom()
         ENDIF

         nPrevRecNo := sx_RecNo()

         oBrowse:RefreshAll():forceStable()
         DO WHILE nPrevRecNo != sx_RecNo()
            oBrowse:Up():forceStable()
         ENDDO

      ENDIF

   ELSE
      oBrowse:Refreshcurrent()
   ENDIF

   RETURN nResult != DE_ABORT
