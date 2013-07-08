/*
 * $Id: testbrw.prg 9279 2011-02-14 18:06:32Z druzus $
 */

// Harbour Class TBrowse and TBColumn sample

#include "inkey.ch"

function Main()

   local linit   := .F., i
   local oBrowse := TBrowseNew( 5, 5, 16, 30 )
   local aTest0  := { "This", "is", "a", "browse", "on", "an", "array", "test", "with", "a", "long", "data" }
   local aTest1  := { 1, 2, 3, 4, 5, 6, 7, 8, 10000, -1000, 54, 456342 }
   local aTest2  := { date(), date()+4, date()+56, date()+14, date()+5, date()+6, date()+7, date()+8, date()+10000, date()-1000, date()-54, date()+456342 }
   local aTest3  := { .t., .f., .t., .t., .f., .f., .t., .f., .t., .t., .f., .f. }
   local n       := 1
   local nKey
   local lEnd    := .f.
   local nCursor
   local cColor
   local nRow, nCol
   local nTmpRow, nTmpCol

   oBrowse:colorSpec     = "W+/B, N/BG"
   oBrowse:ColSep        = "�"
   oBrowse:HeadSep        = "��"
   oBrowse:FootSep        = "��"
   oBrowse:GoTopBlock    = { || n := 1 }
   oBrowse:GoBottomBlock = { || n := Len( aTest0 ) }
   oBrowse:SkipBlock     = { | nSkip, nPos | nPos := n,;
                             n := If( nSkip > 0, Min( Len( aTest0 ), n + nSkip ),;
                             Max( 1, n + nSkip )), n - nPos }

   oBrowse:AddColumn( TBColumnNew( "First",  { || n } ) )
   oBrowse:AddColumn( TBColumnNew( "Second", { || aTest0[ n ] } ) )
   oBrowse:AddColumn( TBColumnNew( "Third",  { || aTest1[ n ] } ) )
   oBrowse:AddColumn( TBColumnNew( "Forth",  { || aTest2[ n ] } ) )
   oBrowse:AddColumn( TBColumnNew( "Fifth",  { || aTest3[ n ] } ) )
   oBrowse:GetColumn(1):Footing = 'Number'
   oBrowse:GetColumn(2):Footing = 'Strins'

   oBrowse:GetColumn(2):Picture := '@!'

   oBrowse:GetColumn(3):Footing = 'Number'
   oBrowse:GetColumn(3):Picture := '999,999.99'
   oBrowse:GetColumn(4):Footing = 'Dates'
   oBrowse:GetColumn(5):Footing = 'Logical'
   // needed since I've changed some columns _after_ I've added them to TBrowse object
   oBrowse:Configure()

   Alert( oBrowse:ClassName() )
   Alert( oBrowse:GetColumn( 1 ):ClassName() )

   oBrowse:RowPos = 5
   oBrowse:Freeze = 1
   nCursor := SetCursor( 0 )
   cColor := SetColor( "W+/B" )
   nRow := Row()
   nCol := Col()
   @ 4,4,17,31 BOX "�Ŀ����� "
#ifdef HB_COMPAT_C53
 oBrowse:Setkey(0,{|ob,nkey| Defproc(ob,nKey)})
while .t.
    oBrowse:ForceStable()
 if (oBrowse:applykey(inkey(0))== -1)
   exit
endif
enddo
#else
While !lEnd
      dispbegin()
      oBrowse:ForceStable()
      dispend()
      nKey = InKey( 0 )

      do case
         case nKey == K_ESC
              SetPos( 17, 0 )
              lEnd = .t.

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

         case nKey = K_TAB
              nTmpRow := ROW()
              nTmpCol := COL()
              @ 0, 0 SAY TIME()
              DevPos( nTmpRow, nTmpCol )

      endcase

   end
#endif
   DevPos( nRow, nCol )
   SetColor( cColor )
   SetCursor( nCursor )

return nil
#ifdef HB_COMPAT_C53
function  defproc(ob,nkey)
Local nTmpRow,nTmpCol
        if nKey = K_TAB
              nTmpRow := ROW()
              nTmpCol := COL()
              @ 0, 0 SAY TIME()
              DevPos( nTmpRow, nTmpCol )
            ob:Refreshall()
        endif
        return 1
#endif
