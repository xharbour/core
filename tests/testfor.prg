//
// $Id: testfor.prg 9279 2011-02-14 18:06:32Z druzus $
//

FUNCTION MAIN

   LOCAL i

   FOR i := 1 TO 10

      qout( i )

      IF i = 4 .AND. .T.
         __Accept("")
         qout(i)
         i := 9
         qout(i)
         __Accept("")
      ENDIF

   NEXT

   RETURN( NIL )
