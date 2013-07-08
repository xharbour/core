/*
 * $Id: test01.prg 9578 2012-07-17 17:51:51Z andijahja $
 */
/*
  test1.prg
  Demo Creating DBF and Append Blank Records
  Using SDENSX
*/
#include "sixapi.ch"
#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)

PROCEDURE MAIN()

   LOCAL cFile   := "sixtest.dbf"
   LOCAL aStruct := { { "TEST","C",10,0 }, { "MYDATE","C",8,0 }, { "MYNUM","N",8,0 }, { "MYLOGIC","L",1,0 } }
   LOCAL j, n := seconds(), nArea

   ? SX_VERSION()

   CREATE DBF cFile STRUCT aStruct VIA SDENSX

   USE cFile ALIAS MYALIAS VIA SDENSX VAR nArea EXCLUSIVE

   ? "-----------------------------------"
   ? "Test Appending 10,000 Blank Records"
   ? "-----------------------------------"
   ? "Area  : ", nArea
   ? "RDD   : " + sx_rddDriver( nArea )
   ? "Start : ", n

   for j := 1 TO 10000
      APPEND BLANK
   next

   CLOSE ALL

   ? "End   : ", seconds()
   ? "Time  : ", seconds() - n
