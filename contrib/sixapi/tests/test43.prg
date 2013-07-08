/*
 * $Id: test43.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test43.prg
   test sx_CopyFile()
*/
#include "sixapi.ch"

#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL cFile1  := "test_1.dbf"
   LOCAL cFile2  := "test_2.dbf"
   LOCAL cFile3  := "test_3.dbf"

   LOCAL aStruct := { ;
      { "MYCHAR"    , "C", 15, 0 }, ;
      { "MYDATE"    , "D", 8, 0 }, ;
      { "MYNUMBER1" , "N", 8, 0 }, ;
      { "MYNUMBER2" , "N", 8, 0 } }

   SX_RDDSETDEFAULT( "SDEFOX" )
   SET EPOCH 1950
   SET DATE "DD/MM/YYYY"

   CREATE DBF cFile1 STRUCT aStruct
   USE cFile1 ALIAS MYALIAS_1 EXCLUSIVE

   CREATE DBF cFile2 STRUCT aStruct
   USE cFile2 ALIAS MYALIAS_2 EXCLUSIVE

   CREATE DBF cFile3 STRUCT aStruct
   USE cFile3 ALIAS MYALIAS_3 EXCLUSIVE

   ? 'APPEND BLANK TO MYALIAS_1 100'
   APPEND BLANK TO MYALIAS_1 100
   ? 'APPEND BLANK TO MYALIAS_2 200'
   APPEND BLANK TO MYALIAS_2 200
   ? 'APPEND BLANK TO MYALIAS_3 300'
   APPEND BLANK TO MYALIAS_3 300

   ? 'sx_CopyFile( "NEWFILE1", "MYALIAS_1" ) =', sx_CopyFile( "NEWFILE1", "MYALIAS_1" )
   ? 'sx_CopyFile( "NEWFILE2", "MYALIAS_2" ) =', sx_CopyFile( "NEWFILE2", "MYALIAS_2" )
   ? 'sx_CopyFile( "NEWFILE3", "MYALIAS_3" ) =', sx_CopyFile( "NEWFILE3", "MYALIAS_3" )

   CLOSE ALL

