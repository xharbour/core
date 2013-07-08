/*
 * $Id: prog0.prg 9279 2011-02-14 18:06:32Z druzus $
 */
/*
   PROG0.PRG
   Copyright 2004 Budyanto Dj. <budyanto@centrin.net.id>

   This is a simple Clipper program runs simple GET and BROWSE dialogs
   each on a pseudo-window with brief help message on the bottom of
   the screen.

   Note that this Clipper program uses ZNEWWINDOW() and ZREVWINDOW() to
   open and close every pseudo-windows respectively.

   Compiling and linking in Clipper:
     clipper prog0 -m -n -w
     blinker file prog0
*/


#include "common.ch"
#include "inkey.ch"
#include "setcurs.ch"

static s_zwin := {}
static s_cStdColor := "N/W,N/GR*,,,N/W*"

proc main
local i,j
   SET SCOREBOARD OFF
   SetColor( s_cStdColor )
   setcursor(SC_NONE)
   CLS
   @ 0,0 say padc("This is the Main Window", maxcol()+1)

   * screen background
   DISPBEGIN()
   for i := 1 to maxrow()-1
      for j := 0 to maxcol()
         devpos(i,j)
         devout("�")
      next
   next
   DISPEND()

   lboxmessage("Welcome to our test program." + chr(13)+chr(10) +;
               "This program will show typical GET and BROWSE dialogs " +;
               "with brief help on the bottom of the screen.")
   xGet1()
   xBrowse1()
   lboxmessage("That's all folks")

   * restore state
   setcursor(SC_NORMAL)
return //main

procedure xGet1()
local nWin
local cName := padr("Name",20)
local cAddr := padr("Address",25)
local cPhone:= padr("Phone",15)
local cFax  := padr("Fax",15)
local lDone := .f.
local getlist := {}
local oldCurs := setcursor(SC_NORMAL)

   nWin := znewwindow("�Ŀ�����",10,20,22,59,"Some Window")

   //@ 21,21 say "Inside the window" color "R/W"
   //@ 23,0  say "Outside the window" color "R/W"

   do while !lDone
      @ 12,22 say "Name    : " get cName  pict "@!K" when lMessage("Please enter your name")
      @ 14,22 say "Address : " get cAddr  pict "@!K" when lMessage("Please enter your address")
      @ 16,22 say "Phone   : " get cPhone pict "@K"  when lMessage("Please enter your phone number")
      @ 18,22 say "Fax     : " get cFax   pict "@K"  when lMessage("Please enter your fax number")
      read

      lMessage("")
      lDone := lyesno("Done?")
   enddo

   zrevwindow()

   setcursor(oldCurs)
return //xGet1()

/* the following is adapted from WVTGUI.PRG by Pritpal Bedi
   for illustration purposes only */
FUNCTION xBrowse1()
   LOCAL nKey, bBlock, oBrowse , i
   LOCAL lEnd    := .f.
   LOCAL info_   := {}
   LOCAL nTop    :=  6
   LOCAL nLeft   :=  3
   LOCAL nBottom := maxrow() - 2
   LOCAL nRight  := maxcol() - 3
   LOCAL nCursor := setCursor( 0 )
   LOCAL nWin

   USE 'TEST' NEW
   if NetErr()
      return nil
   endif
   info_:= DbStruct()

   SetColor( 'N/W*,N/GR*,,,N/W* ' )
   oBrowse := TBrowseNew( nTop + 1, nLeft + 1, nBottom - 1, nRight - 1 )

   oBrowse:ColSep        = chr(179) //'|'
   oBrowse:HeadSep       = chr(205) //'_'
   oBrowse:GoTopBlock    = { || dbGoTop() }
   oBrowse:GoBottomBlock = { || dbGoBottom() }
   oBrowse:SkipBlock     = { | nSkip | dbSkipBlock( nSkip,oBrowse ) }

   for i := 1 to len( info_ )
      bBlock := VouBlockField( i )
      oBrowse:AddColumn( TBColumnNew( info_[ i,1 ], bBlock ) )
   next

   oBrowse:configure()

   nWin := znewwindow("�Ŀ�����",nTop,nLeft,nBottom,nRight, "Test.dbf")

   While !lEnd
      oBrowse:ForceStable()

      lMessage("Record #" + alltrim(str(recno())) )

      nKey = InKey( 0 )

      do case
      case nKey == K_ESC .or. nKey == K_ENTER
         lEnd := lYesNo("Done?")
      case nKey == K_DOWN
         oBrowse:Down()
      case nKey == K_UP
         oBrowse:Up()
      case nKey == K_LEFT
         oBrowse:Left()
      case nKey == K_RIGHT
         oBrowse:Right()
      case nKey = K_PGDN
         oBrowse:pageDown()
      case nKey = K_PGUP
         oBrowse:pageUp()
      case nKey = K_CTRL_PGUP
         oBrowse:goTop()
      case nKey = K_CTRL_PGDN
         oBrowse:goBottom()
      case nKey = K_HOME
         oBrowse:home()
      case nKey = K_END
         oBrowse:end()
      case nKey = K_CTRL_LEFT
         oBrowse:panLeft()
      case nKey = K_CTRL_RIGHT
         oBrowse:panRight()
      case nKey = K_CTRL_HOME
         oBrowse:panHome()
      case nKey = K_CTRL_END
         oBrowse:panEnd()
      endcase
   end

   lMessage("")

   zrevwindow()

   * restore state
   SetCursor( nCursor )

   DBCloseArea()
RETURN nil
//-------------------------------------------------------------------//
STATIC FUNCTION DbSkipBlock( n, oTbr )
   LOCAL nSkipped := 0
   if n = 0
      DBSkip( 0 )
   elseif n > 0
      do while nSkipped != n .and. TBNext( oTbr )
         nSkipped++
      enddo
   else
      do while nSkipped != n .and. TBPrev( oTbr )
         nSkipped--
      enddo
   endif
RETURN  nSkipped

//-------------------------------------------------------------------//
STATIC FUNCTION TBNext( oTbr )
   LOCAL nSaveRecNum := recno()
   LOCAL lMoved := .T.
   if Eof()
      lMoved := .F.
   else
      DBSkip( 1 )
      if Eof()
         lMoved := .F.
         DBGoTo( nSaveRecNum )
      endif
   endif
RETURN lMoved
//-------------------------------------------------------------------//
STATIC FUNCTION TBPrev( oTbr )
   LOCAL nSaveRecNum := Recno()
   LOCAL lMoved := .T.
   DBSkip( -1 )
   if Bof()
      DBGoTo( nSaveRecNum )
      lMoved := .F.
   endif
RETURN lMoved
//-------------------------------------------------------------------//
STATIC FUNCTION VouBlockField( i )
RETURN  {|| fieldget( i ) }


// supporting functions ***************************

function lMessage(cMsg)
* displays a message on maxrow() and returns .t.
local cOldColor := setcolor(s_cStdColor)
  @ maxrow(), 0 say padc(cMsg, maxcol()+1)
  setcolor(cOldColor)
return .t.

function lYesNo(cMsg)
* display cmsg with Yes/No option, returns .t. if Yes selected
local nTopLine, ;
      nLeft := 5, ;
      nBotLine := maxrow()-2,;
      nRight := maxcol()-5
local nChoice, nWidth, nWinNum
local oldCurs := setcursor(SC_NONE)
local oldColor := setcolor(s_cStdColor)

   default cMsg to "Please Confirm"

   cmsg := " " + alltrim(cmsg) + " "
   nWidth := max(len(cmsg), len("Yes"))
   nTopLine := nBotLine-2-1

   nLeft := max(nLeft, ((nRight+nLeft)*.5)-(nWidth*.5)-1)
   nRight := nLeft + nWidth + 1

   * open window
   nWinNum := znewwindow("�Ŀ�����", nTopLine, nLeft, nBotLine, nRight, cMsg)

   @ nTopLine+1, nLeft+1 PROMPT padr("Yes", nWidth)
   @ nTopLine+2, nLeft+1 PROMPT padr("No", nWidth)
   MENU TO nChoice

   * close window
   zrevwindow()

   setcursor(oldCurs)
   setcolor(oldColor)
return (nChoice==1)

function lBoxMessage(cMsg, cTitle)
local nTopLine, ;
      nLeft := 5, ;
      nBotLine := maxrow()-2,;
      nRight := maxcol()-5
local nwidth, nmaxwidth, i, nNumLines, cAline, nWinNum
local oldCurs := setcursor(SC_NONE)
local oldColor := setcolor(s_cStdColor)

   default cTitle to "Info"

   cmsg := alltrim(cmsg)
   nNumLines := MLCOUNT(cmsg,(nright-nleft)-1)
   nWidth := iif(nNumLines<2, len(cmsg), nRight-nLeft-1)
   nTopLine := nBotLine-nNumLines-1
   if nTopLine < 0            //too many lines to display
      nNumLines += nTopLine
      nTopLine := 0
   endif

   nMaxWidth := 0
   for i := 1 to nNumLines
      nMaxWidth := MAX(nMaxWidth, len(trim(memoline(cmsg,nwidth,i))))
   next

   nLeft := max(nLeft, INT( ((nRight+nLeft)/2)-(nMaxWidth/2)-1 ) )
   nRight := nLeft + nMaxWidth + 1

   * open window
   nWinNum := znewwindow("�Ŀ�����", nTopLine, nLeft, nBotLine, nRight, cTitle)
   DISPBEGIN()
   for i := 1 to nNumLines
     cAline := MEMOLINE(cMsg, nWidth, i)
     devpos(nTopLine+i, nLeft+1)
     devout(padc(alltrim(cAline), nMaxWidth))
   next
   DISPEND()

   inkey(0)

   * close window
   zrevwindow()

   setcursor(oldCurs)
   setcolor(oldColor)
return .t.

FUNCTION ZNEWWINDOW(wtype,r1,c1,r2,c2,ctitle, ccolor)
* Draw a new window on screen and register it in window list
* wtype       : Window border type, eg. "�Ŀ�����"
* r1,c1,r2,c2 : coordinates
* Return      : Numeric id of the new window
  local i:=len(s_zwin)
  local cScreen := savescreen(r1,c1,r2,c2)
  local cOldColor := SETCOLOR()
  local nrow := row(), ncol := col()

  default ctitle to ""
  default ccolor to s_cStdColor
  setcolor(ccolor)

  AADD(s_zwin,{i+1, r1, c1, r2, c2, cScreen, ctitle, nrow, ncol, coldcolor})

  SETCOLOR(ccolor)

  scroll(r1, c1, r2, c2)

  * GTWVW doesn't need box or textual title
  DISPBOX(r1,c1,r2,c2,wtype)
  if !empty(ctitle)
     cTitle := " " + alltrim(ctitle) + " "
     DevPos( r1, nCeiling( (c2+c1-len(cTitle))/2 ) )
     DevOut( cTitle )
  endif

  SETCOLOR(cOldColor)
RETURN(i+1)

FUNCTION ZREVWINDOW()
* Closes the last window and remove it from window list
  local i := len(s_zwin)

  if i == 0
     * no window to close
     return NIL
  endif

  * restore states
  restscreen(s_zwin[i][2], s_zwin[i][3], s_zwin[i][4], s_zwin[i][5], s_zwin[i][6])
  setpos(s_zwin[i][8], s_zwin[i][9])
  setcolor(s_zwin[i][10])

  * remove window from list
  adel(s_zwin, i)
  asize(s_zwin, len(s_zwin)-1)
RETURN(NIL)

function nCeiling(nNumber)
local nTemp
   nTemp := nNumber - INT(nNumber)  //right of dec point
   if nTemp>0
      nNumber := INT(nNumber) + 1
   else
      nNumber := INT(nNumber)
   endif
return nNumber
