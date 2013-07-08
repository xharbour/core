//NOTEST
//
// $Id: testid.prg 9279 2011-02-14 18:06:32Z druzus $
//

// Warning: This sample must be tested using /dTEST compiler flag

//TODO: Check why there is core dump om Linux if this file is compiled
// without /dTEST

#define FIRST
#define SECOND
#define THIRD

function Main()

   QOut( "testing Harbour /d compiler flag" )

   #ifdef TEST
      QOut( "Fine, you have just tested the /d compiler flag" )
   #else
      QOut( "Please change hb32.bat and include /dTEST compiler flag" )
      QOut( "Or run 'SET PRG_USR=/dTEST' if you are using the GNU Make System" )
   #endif

   #ifdef FIRST
      QOut( "FIRST is defined" )

      #ifdef SECOND
         QOut( "FIRST and SECOND are defined" )

         #ifdef THIRD
            QOut( "FIRST, SECOND and THIRD are defined" )
         #else
            QOut( "THIRD is not defined" )
         #endif

      #else
         QOut( "SECOND is not defined" )
      #endif

   #else
      QOut( "FIRST is not defined" )
   #endif

return nil
