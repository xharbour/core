/*
* New enhanced DBEdit() test
*
* See how aCols[1] is an array of 2 items
* The first is, as usual, the column data
* The second is the codeblock that returns a pair of {std,enh} colors
* just like in tbrowse :-)
*
* NOTE: - Color codeblock takes a parameter (actual data)
*         I�ve ignored (used SALARY instead) it here to demostrate how flexible a TBRowse is.
*       - Try moving columns with K_CTRL_UP & K_CTRL_DOWN
*
* Is DBEdit() deprecated? NO!
* Enjoy
* Mauricio <maurifull@datafull.com>
* Comments, suggestions, bugs report welcome
*
*/
#include "inkey.ch"
#include "dbedit.ch"

Function Main()
   Local aCols := {{"PadR(AllTrim(FIRST)+' '+AllTrim(LAST),30)", {|x| IIf(FIELD->SALARY<10000, {3,2}, IIf(FIELD->SALARY<100000,{1,2},{4,5}))}},;
                "CITY",;
                "SALARY"}

  set scoreboard off

  Use test
  @maxrow(),0 say "<Ctrl-Down/Up>-Toggle columns  <Space bar>-Edit cell" 
  DBEdit(0,0,MaxRow()-1,MaxCol(), aCols, "TstFnc",,{"Name", "City", "Salary"})
  Close
  Cls
  ? "Have a nice day ;)"
  ?
Return Nil

Function TstFnc(nMode, nCol, oTBR)
LOCAL GetList := {}
Local nRet := DE_CONT

  Do Case
    Case nMode == DE_INIT // EXTENSION: Initialization mode
      oTBR:colorSpec := "n/bg,w/n,r/bg,w+/bg,w+/gr"
    Case nMode == DE_HITTOP
      Keyboard Chr(K_CTRL_PGDN)
    Case nMode == DE_HITBOTTOM
      Keyboard Chr(K_CTRL_PGUP)
    Case LastKey() == K_ESC
      nRet := DE_ABORT
    Case LastKey() == K_SPACE .And. oTBR:getColumn(nCol):heading != "Name"
      SetCursor(1)
      @ Row(), Col() Get &(oTBR:getColumn(nCol):heading)
      Read
      SetCursor(0)
      Clear TypeAhead
  End

Return nRet
