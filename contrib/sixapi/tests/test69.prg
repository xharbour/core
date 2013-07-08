/*
 * $Id: test69.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test69.prg
   sx_CreateFrom()
*/
#include "sixapi.ch"
#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   USE "test/test" ALIAS TESTME
   COPY STRUCTURE EXTENDED TO "Foo"
   // Alias "BAR" passed and "newdbf.dbf" will be opened after creation
   ? sx_CreateFrom( "newdbf.dbf", "bar", "Foo", "SDENSX" )
   sx_Close( "TESTME" )
   ? sx_Alias() // should be in "BAR"
   CLOSE ALL
   ?
   USE "test/test" ALIAS TESTME
   COPY STRUCTURE EXTENDED TO "Foo"
   // No alias passed, FOO will not be opened
   ? sx_CreateFrom( "newdbf.dbf", , "Foo", "SDENSX" )
   ? sx_Alias() // should be in "TESTME"
   CLOSE ALL
