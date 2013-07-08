//
// $Id: say.prg 9279 2011-02-14 18:06:32Z druzus $
//

// Tests @ SAY with and without PICTURE clauses

function Main()
   CLS
   SET CENTURY ON
   @ 2,39 TO 7,39 DOUBLE
   @ 0,0 SAY "Testing @ SAY with and without PICTURE clauses"
   @ 0,60 SAY DATE()
   SET CENTURY OFF
   @ 2,1  SAY -1.25
   @ 2,41 SAY -1.25   PICTURE "@( 99,999.99"
   @ 3,1  SAY  1.25   PICTURE "@( 9,999.99"
   @ 3,41 SAY  1.25   PICTURE "@( $9,999.99"
   @ 5,1  SAY DATE()
   @ 5,41 SAY DATE()  PICTURE "@E"
   @ 7,1  SAY "Hello"
   @ 7,41 SAY "Hello" PICTURE "@!"
return nil
