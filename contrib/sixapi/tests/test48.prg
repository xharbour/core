/*
 * $Id: test48.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
  test48.prg
  How to DBFEncrypt/DBFDecrypt EXISTING DBF
*/
#include "sixapi.ch"

#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL cFile1 := "test\test.dbf"

   ?
   ? 'Opening TEST.DBF and Copy to NEWTEST.DBF ...'
   USE cFile1 ALIAS MYALIAS EXCLUSIVE
   sx_CopyFile( "NEWTEST.DBF" )
   USE "NEWTEST.DBF" ALIAS NEWTEST EXCLUSIVE
   sx_Close( "MYALIAS" )
   ? 'Now BROWSE NEWTEST .. Press any key ...'
   PAUSE
   CLS
   BROWSE
   CLOSE ALL
   CLS

   ?
   ? 'Now ENCRYPTING NEWTEST.DBF'
   ? 'And BROWSE with CORRECT PASSWORD ... Press any key ...'
   PAUSE
   USE "NEWTEST" ALIAS NEWTEST EXCLUSIVE
   SX_SETPASSWORD( "HARBOUR" ) // Password
   ? 'Working ...'
   ? SX_DBFEncrypt()
   CLS
   BROWSE
   CLOSE ALL

   CLS
   ?
   ? 'BROWSE with WRONG PASSWORD .. Press any key ...'
   PAUSE
   USE "NEWTEST" ALIAS MYALIAS EXCLUSIVE
   SX_SETPASSWORD( "XHARBOUR" ) // Wrong Password
   CLS
   BROWSE
   CLOSE ALL

   CLS
   ?
   ? 'BROWSE with CORRECT PASSWORD .. Press any key ...'
   PAUSE
   USE "NEWTEST" ALIAS MYALIAS EXCLUSIVE
   SX_SETPASSWORD( "HARBOUR" ) // Wrong Password
   CLS
   BROWSE
   CLOSE ALL

   CLS
   ?
   ? 'Now PHYSICALLY DECRYPTING NEWTEST.DBF'
   ? 'Reset PASSWORD and BROWSE with NO PASSWORD.. Press any key ...'
   PAUSE
   USE "NEWTEST" ALIAS MYALIAS EXCLUSIVE
   SX_SETPASSWORD( "HARBOUR" ) // Wrong Password
   ? sx_DBFDecrypt()
   SX_SETPASSWORD( "" )        // Reset Password
   // No longer required because DBF is normal now
   CLS
   BROWSE
   CLOSE ALL
