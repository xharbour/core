/*
 * $Id: test19.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
  test18.prg
  Demo Creating DBF and Append Blank Records
  Using SDENSXDBT .....
*/
#include "sixapi.ch"
#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL cFile   := "sixtest.dbf"
   LOCAL aStruct := { { "TEST","C",10,0 }, { "MYDATE","D",8,0 }, { "MYNUM","N",8,0 }, { "MYMEMO","M",10,0 }, { "MYLOGIC","L",1,0 } }
   LOCAL j, n := seconds(), nArea

   sx_RDDSetDefault( "SDENSXDBT" )

   CREATE DBF cFile STRUCT aStruct

   USE cFile ALIAS MYALIAS VAR nArea SHARED // RDD SDEFOX

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
