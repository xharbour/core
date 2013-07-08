/*
 * $Id: test73.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test73.prg
   sx_Empty()/sx_IsNull()
*/
#include "sixapi.ch"
#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL aStruct1 := { { "PART_NO","C",10,0 }, { "PRICE","N",10,2 }, { "NOTES","M",10,0 } }

   SET RDD SDEFOX
   SET DATE "dd/mm/yyyy"
   CREATE TABLE "TEST1" STRUCT aStruct1
   USE "test1"
   APPEND BLANK
   REPLACE PRICE WITH 20
   ? sx_Empty( "PART_NO" )
   ? sx_IsNull( "PART_NO" )
   ? sx_Empty( "PRICE" )
   ? sx_IsNull( "PRICE" )
