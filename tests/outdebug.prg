************************************************************
* outdebug.prg
* $Id: outdebug.prg 9279 2011-02-14 18:06:32Z druzus $
*
* Test for HB_OutDebug() routine
*
* (C) Giancarlo Niccolai
*


PROCEDURE Main()

   SET EXACT OFF

   SET COLOR TO W+/b
   CLEAR SCREEN
   @1,20 SAY "X H A R B O U R - HB_OutDebug Test "

   @3,10 SAY "Following strings will be repeated into debug window:"
   @4,10 SAY "Hello World!"
   HB_OutDebug( "Hello World!" )
   @5,10 SAY "How are you"
   HB_OutDebug( "How are you" )
   @6,10 SAY "Fine, thanks, and you?"
   HB_OutDebug( "Fine, thanks, and you?" )
   @7,10 SAY "Fine!"
   HB_OutDebug( "Fine!" )

   @10,20 SAY "Press a key to terminate"
   Inkey( 0 )
   @12,0
RETURN
