//
// $Id: objarr.prg 9279 2011-02-14 18:06:32Z druzus $
//

//
// Object Array syntax test
//
// Written by Eddie Runia <eddie@runia.com>
// www - http://www.harbour-project.org
//
// Placed in the public domain
//

Function Main

   local o := TNumber():New()

   QOut( "Direct reference : ", ToChar( o:x ) )

   o:x[1]     := "I am a data"
   o:Get()[2] := "I am a method"
   QOut( "Assign text      : ", ToChar( o:x ) )

   o:x[1]     := 4
   o:Get()[2] := 4
   QOut( "Assign 4         : ", ToChar( o:x ) )

   QOut( "Post increment   : ", o:x[1]++ , o:Get()[2]++ )
   QOut( "After            : ", o:x[1]   , o:Get()[2]   )
   QOut( "Pre decrement    : ", --o:x[1] , --o:Get()[2] )
   QOut( "After            : ", o:x[1]   , o:Get()[2]   )

   o:x[1]     += 2
   o:Get()[2] += 2
   QOut( "Plus 2           : ", ToChar( o:x ) )

   o:x[1]     -= 3
   o:Get()[2] -= 3
   QOut( "Minus 3          : ", ToChar( o:x ) )

   o:x[1]     *= 3
   o:Get()[2] *= 3
   QOut( "Times 3          : ", ToChar( o:x ) )

   o:x[1]     /= 1.5
   o:Get()[2] /= 1.5
   QOut( "Divide by 1.5    : ", ToChar( o:x ) )

   o:x[1]     %= 4
   o:Get()[2] %= 4
   QOut( "Modulus 4        : ", ToChar( o:x ) )

   o:x[1]     ^= 3
   o:Get()[2] ^= 3
   QOut( "To the power 3   : ", ToChar( o:x ) )

   QOut( "Global stack" )
   Debug( hb_dbg_vmStkGList() )
   QOut( "Statics")
   Debug( hb_dbg_vmVarSList() )
return NIL

Function TNumber()                              // Very simple class

   static oNumber

   if oNumber == NIL
      oNumber := HBClass():New( "TNumber" )

      oNumber:AddData  ( "x"   )
      oNumber:AddMethod( "Get", @Get() )
      oNumber:AddMethod( "New", @New() )
      oNumber:Create()
   endif
return oNumber:Instance()


static function New()

   local self := QSelf()

   ::x := {1,1}
return self


static function Get()

   local self := QSelf()

return ::x
