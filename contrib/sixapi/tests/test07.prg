/*
 * $Id: test07.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test7.prg
   creating new index files, seek and found tests
   closing files and reusing existing index file
*/
#include "sixapi.ch"
#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL cFile   := "sixtest.dbf"
   LOCAL aStruct := { { "MYCHAR","C",10,0 }, { "MYDATE","D",8,0 }, { "MYNUM","N",8,0 }, { "MYLOGIC","L",1,0 } }
   LOCAL j, n := seconds(), nArea
   LOCAL nIndex

   SET EPOCH 1950

   IF FILE( "sixtest.nsx" )
      FERASE( "sixtest.nsx" )
   ENDIF

   CREATE DBF cFile STRUCT aStruct RDD SDENSX

   USE cFile ALIAS MYALIAS RDD SDENSX VAR nArea EXCLUSIVE

   ? "------------------------------------------------------"
   ? "Test Appending 10,000 Blank Records and Creating Index"
   ? "------------------------------------------------------"
   ? "Area  : ", nArea
   ? "RDD   : " + sx_rddDriver( nArea )
   ? "Start : ", n

   FOR j := 1 TO 10000
      APPEND BLANK
      FieldPut( MYCHAR,   "NAME_" + PADL( j, 5, "0" ) )
      FieldPut( MYDATE, date() + j )
      FieldPut( MYNUM,  j )
      FieldPut( MYLOGIC,  j % 2 == 0 )
   NEXT

   COMMIT

   ?
   ? 'INDEX ON UPPER(MYCHAR) TO SIXTEST'
   ?
   INDEX ON UPPER( MYCHAR ) TO SIXTEST
   ? "End   : ", seconds()
   ? "Time  : ", seconds() - n
   ?
   ? "Now CLOSE ALL and ReOpen DBF ... Press any key ..."
   PAUSE

   CLOSE ALL

   // ReOpen Database
   USE cFile ALIAS MYALIAS RDD SDENSX VAR nArea EXCLUSIVE
   // Automatically Open index file if it has same name with DBF
   // Must explisitely set order 1 because we opened one index file
   ?
   ? 'sx_SetOrder( 1 )     =', sx_SetOrder( 1 )
   ? 'sx_IndexName( 1 )    =', sx_IndexName( 1 )
   ?
   ? 'Must explisitely set order 1 because we opened one index file'
   ?
   ? 'Seek( "NAME_07567" ) =', Seek( "NAME_07567" )
   ? 'Found()              =', Found()
   ? 'Eof()                =', Eof()
   ? 'RecNo()              =', RecNo()
   ?
   ? 'Seek( "NOTEXIST" )   =', Seek( "NOTEXIST" )
   ? 'Found()              =', Found()
   ? 'Eof()                =', Eof()
   ? 'RecNo()              =', RecNo()

   CLOSE ALL
