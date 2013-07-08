/*
 * $Id: test33.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test33.prg
   creating new index files, seek and found tests
   closing files and reusing existing index file
   COPY STRUCTURE TO FIELDS ...
*/
#include "sixapi.ch"
#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL cFile   := "sixtest.dbf"
   LOCAL aStruct := { ;
      { "MYCHAR"    , "C", 15, 0 }, ;
      { "MYDATE"    , "D", 8, 0 }, ;
      { "MYNUMBER"  , "N", 8, 0 }, ;
      { "MYDECIMAL" , "N", 8, 2 }, ;
      { "MYLOGIC"   , "L", 1, 0 }, ;
      { "MYMEMO"    , "M", 10, 0 } }

   ? 'CREATE DBF cFile STRUCT aStruct'
   ? 'USE cFile ALIAS MYALIAS'
   CREATE DBF cFile STRUCT aStruct
   USE cFile ALIAS MYALIAS
   ?
   ? 'Now will copy structure ... Press any key ...'
   PAUSE
   ?
   ? 'COPY STRUCTURE TO TSTSTRU'
   COPY STRUCTURE TO TSTSTRU
   ?
   ? 'Now browse the newly created DBF ... Press any key ...'
   PAUSE
   cls
   USE "TSTSTRU" ALIAS MYSTRUCT
   BROWSE
   sx_Close()

   ?
   ? 'Now will copy structure using FIELDS clause ... Press any key ...'
   PAUSE
   ?
   ? 'COPY STRUCTURE TO TSTSTRU FIELDS MYCHAR, MYDATE, MYDECIMAL, MYLOGIC'
   COPY STRUCTURE TO TSTSTRU FIELDS MYCHAR, MYDATE, MYDECIMAL, MYLOGIC

   ? 'Now browse the newly created DBF ... Press any key ...'
   PAUSE
   cls
   USE "TSTSTRU" ALIAS MYSTRUCT
   BROWSE

   CLOSE ALL
