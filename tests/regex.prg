* regex.prg
* $Id: regex.prg 9279 2011-02-14 18:06:32Z druzus $
* Test for regular expression functions
*
* Giancarlo Niccolai
*

PROCEDURE Main()
   LOCAL regex
   LOCAL aMatch
   LOCAL cStr, nRow := 2, nCol
   LOCAL aSource := { ;
      "First date to match: 2001-3-21", ;
      "2002-12/2", ;
      "Another can be 1999/5/12, and suceeds", ;
      "Could be 1999/534/12, but this will fail" ;
   }

   CLEAR SCREEN

   @0,15 SAY "X H A R B O U R - Regular expression scan tests"
   /*
   * Standard regex to get the ISO date format:
   * ([0-9]{4}): exactly fuor digits (year); it is in brackets,
   *    this means that we want it back as a group
   * [-/]: one bar or a minus
   * ([0-9]{1,2}): one or two digits
   */
   regex := HB_RegexComp( "([0-9]{4})[-/]([0-9]{1,2})[-/]([0-9]{1,2})" )

   FOR EACH cStr IN aSource
      @nRow, 5 SAY "String is '" + cStr + "'"
      nRow ++
      aMatch := HB_Regex( regex, cStr )
      IF .not. Empty( aMatch )
         @nRow, 10 SAY "Matched: " + aMatch[1] + " ( Year: " + aMatch[2] + ", Month: " +;
            aMatch[3] + ", Day: " + aMatch[4] + ")"
      ELSE
         @nRow, 10 SAY "Match FAILED!"
      ENDIF
      nRow += 2
   NEXT

   cStr := "searching 'regex' here:"
   @nRow, 5 SAY "A test of a regex compiled on the fly; " + cStr
   aMatch := HB_Regex( "(.*)regex(.*)", cStr )
   nRow++
   IF Empty( aMatch )
      @ nRow, 10 SAY "NOT FOUND!"
   ELSE
      @ nRow, 10 SAY "Found (Before: <<" + aMatch[2] + ">>, After: <<" + aMatch[3] + ">>)"
   ENDIF

   nRow+=2

   cStr := "A str; with: separators :; here "
   @nRow, 5 SAY "Split test; splitting '" + cStr + "' by ':|;'"
   nRow++
   aMatch := HB_RegexSplit( ":|;", cStr )
   IF Empty( aMatch )
      @nRow++, 10 SAY "Test failed"
   ELSE
      nCol := 10
      FOR EACH cStr in aMatch
         @nRow, nCol SAY cStr + "/"
         nCol += Len( cStr ) +1
      NEXT
      nRow++
   ENDIF

   cStr := "A string without separators"
   @nRow, 5 SAY "Split test; splitting '" + cStr + "' by ':|;'"
   nRow++
   aMatch := HB_RegexSplit( ":|;", cStr )
   IF Empty( aMatch )
      @nRow++, 10 SAY "Test failed"
   ELSE
      nCol := 10
      FOR EACH cStr in aMatch
         @nRow, nCol SAY cStr + "/"
         nCol += Len( cStr ) +1
      NEXT
      nRow++
   ENDIF

   cStr := "Test for RegexAtx"
   @nRow, 5 SAY "RegexAtx test; scanning '" + cStr + "' by 'Reg(.x)'"
   nRow++
   aMatch := HB_RegexAtx( "Reg(.x)", cStr )
   IF Empty( aMatch )
      @nRow++, 10 SAY "Test failed"
   ELSE
      nCol := 15
      FOR EACH cStr in aMatch
         @nRow, nCol SAY "FOUND: '"+cStr[1] + "' Start: " + AllTrim( Str(cStr[2]) )+;
            " End: " + Alltrim( Str( cStr[3] ) )
         nRow++
      NEXT
      nRow++
   ENDIF

   @nRow, 1
   @24, 25 SAY "Press a key to continue"
   Inkey( 0 )
RETURN
