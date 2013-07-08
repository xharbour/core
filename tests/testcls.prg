/*
 * $Id: testcls.prg 9279 2011-02-14 18:06:32Z druzus $
 */

// Testing Harbour classes ON ERROR feature

#include "hbclass.ch"

function Main()

   local o := Test()

   o:Another( "Hello" ) // "Another" message is not defined for Class Test, but
                        // it will invoke ON ERROR MyErrorManager() method

   o:Another = 5  // Notice how __GetMessage() shows a underscored message
                  // as we are setting a DATA value.
return nil

CLASS Test

   ON ERROR MyErrorManager( uParam1 )

ENDCLASS

METHOD MyErrorManager( uParam1 ) CLASS Test

   if PCount() > 0
      Alert( uParam1 )
   endif

   Alert( __GetMessage() )  // Shows the message that was sent to the object

return nil

