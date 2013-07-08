/*
 * $Id: test26.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test26.prg
   creating new index files, seek and found tests
   closing files and reusing existing index file
   SX_DBEVAL ........ ANOTHER TESTS ... RECALL
*/
#include "sixapi.ch"
#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL cFile   := "sixtest.dbf"
   LOCAL aStruct := { ;
      { "MYCHAR" , "C", 15, 0 }, ;
      { "MYDATE" , "D", 8, 0 }, ;
      { "MYNUM"  , "N", 8, 0 }, ;
      { "MYMEMO" , "M", 10, 0 }, ;
      { "MYLOGIC", "L", 1, 0 } }
   LOCAL j, n := seconds(), nArea, cPad
   LOCAL nIndex, cOldColor

   SET DELETED ON
   SET COMMITLEVEL 2
   SET EPOCH 1950
   SET DATE "DD/MM/YYYY"

   CREATE DBF cFile STRUCT aStruct RDD SDENSX

   USE cFile ALIAS MYALIAS RDD SDENSX VAR nArea EXCLUSIVE

   ? "-------------------------------"
   ? "Polupating DBF with 100 Records"
   ? "-------------------------------"
   ? "Area  : ", nArea
   ? "RDD   : " + sx_rddDriver( nArea )
   ? "Start : ", n

   FOR j := 1 TO 100
      APPEND BLANK
      cPad := PADL( j, 5, "0" )
      FieldPut( MYCHAR, "NAME_" + cPad )
      FieldPut( MYDATE,  date() + j )
      FieldPut( MYNUM,   j )
      FieldPut( MYMEMO,  "This is Record Number " + cPad )
      FieldPut( MYLOGIC, j % 2 == 0 )
   NEXT

   COMMIT

   ? "End   : ", seconds()
   ? "Time  : ", seconds() - n
   ?
   ? 'Now will browse DBF file ... Press any key ...'
   PAUSE
   BROWSE

   cls
   ?
   ? 'Now testing sx_DBEval with the following expressions :'
   ?
   ? 'DELETE ALL'
   ?
   ? 'Press any key ...'
   PAUSE
   n := seconds()
   ?
   ? 'Working .....'

   DELETE ALL

   ?
   ? "Start : ", n
   ? "End   : ", seconds()
   ? "Time  : ", seconds() - n
   ?
   ? 'Now will browse DBF file ... Press any key ...'
   PAUSE
   BROWSE

   cls

   ?
   ? 'Now testing sx_DBEval with the following expressions :'
   ?
   ? 'RECALL ALL'
   ?
   ? 'Press any key ...'
   PAUSE
   n := seconds()
   ?
   ? 'Working .....'

   SET DELETED OFF
   RECALL ALL

   ?
   ? "Start : ", n
   ? "End   : ", seconds()
   ? "Time  : ", seconds() - n
   ?
   ? 'Now will browse DBF file ... Press any key ...'

   PAUSE
   BROWSE

   CLOSE ALL
