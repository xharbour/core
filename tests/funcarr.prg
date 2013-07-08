//
// $Id: funcarr.prg 9279 2011-02-14 18:06:32Z druzus $
//

//
// Function Array syntax test
//
// Written by Eddie Runia <eddie@runia.com>
// www - http://www.harbour-project.org
//
// Placed in the public domain
//

Function Main

   local a

   QOut( "Direct reference : ", aFunc()[1] )

   a := aFunc()
   QOut( "Ref via array    : ", a[1] )

   aFunc()[1] := "Something different"
   QOut( "Assign new text  : ", aFunc()[1] )

   aFunc()[1] := 4
   QOut( "Assign 4         : ", aFunc()[1] )

   QOut( "Post increment   : ", aFunc()[1]++ )
   QOut( "After            : ", aFunc()[1] )
   QOut( "Pre decrement    : ", --aFunc()[1] )
   QOut( "After            : ", aFunc()[1] )

   aFunc()[1] += 2
   QOut( "Plus 2           : ", aFunc()[1] )

   aFunc()[1] -= 3
   QOut( "Minus 3          : ", aFunc()[1] )

   aFunc()[1] *= 3
   QOut( "Times 3          : ", aFunc()[1] )

   aFunc()[1] /= 1.5
   QOut( "Divide by 1.5    : ", aFunc()[1] )

   aFunc()[1] %= 4
   QOut( "Modulus 4        : ", aFunc()[1] )

   aFunc()[1] ^= 3
   QOut( "To the power 3   : ", aFunc()[1] )

   QOut( "Global stack" )
   Debug( hb_dbg_vmStkGList() )        // Please note a is a reference to aArray !
   QOut( "Statics")
   Debug( hb_dbg_vmVarSList() )
return NIL

Function aFunc()

   static aArray := { [Test] }

return aArray
