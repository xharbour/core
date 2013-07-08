//
// $Id: dates3.prg 9279 2011-02-14 18:06:32Z druzus $
//

// Testing Harbour dates management.

#include "set.ch"

function main()

   LOCAL dDate, i, cNewLine := HB_OSNewLine()

   set( _SET_DATEFORMAT, "dd/mm/yyyy" )
   dDate := cToD( "25/05/1999" )

   OutStd( dDate, dow( dDate ), cNewLine )

   OutStd( LastMonday( dDate ), cNewLine )

   dDate += 3
   OutStd( dDate, dow( dDate ), cNewLine )

   dDate += 4
   OutStd( dDate, dow( dDate ), cNewLine )

   set( _SET_DATEFORMAT, "mm/dd/yyyy" )
   dDate := cToD( "05/25/1999" )

   OutStd( dDate, dow( dDate ), cNewLine )

   OutStd( LastMonday( dDate ), cNewLine )

   dDate += 3
   OutStd( dDate, dow( dDate ), cNewLine )

   dDate += 4
   OutStd( dDate, dow( dDate ), cNewLine )

   OutStd( cNewLine )
   dDate := DATE ()
   FOR i := 1 TO 7
      OutStd( dDate, dow( dDate ), cNewLine )
      dDate++
   NEXT
   OutStd( ctod( "" ), dow( ctod( "" ) ), cNewLine )

return nil

// Like NG's sample
function LastMonday( dDate )
return dDate - dow( dDate ) + 2
