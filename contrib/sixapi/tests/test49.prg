/*
 * $Id: test49.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
  test49.prg
  Tests for Locking Mechanisms ...
*/
#include "sixapi.ch"

#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL cFile1 := "test\test.dbf"
   LOCAL aLockList := {}

   ?
   ? 'Opening TEST.DBF and Copy to NEWTEST.DBF ...'
   USE cFile1 ALIAS MYALIAS

   ? 'sx_Rlock( 10 )  =', sx_Rlock( 10 )
   ? 'sx_Rlock( 20 )  =', sx_Rlock( 20 )
   ? 'sx_Rlock( 30 )  =', sx_Rlock( 30 )

   ? 'sx_Locked( 10 ) =', sx_Locked( 10 )
   ? 'sx_Locked( 20 ) =', sx_Locked( 20 )
   ? 'sx_Locked( 30 ) =', sx_Locked( 30 )

   ? 'sx_LockCount()  =', __trim( sx_LockCount() )
   aLockList := sx_DBRlockList()
   ? 'aLockList := sx_DBRlockList() =>', __trim( len( aLockList ) )
   aeval( aLockList, { |e, i| outstd( "aLockList[" + __trim(i ) + "] = " + __trim(e ), EOL ) } )

   ?
   ? 'sx_Unlock( 10 ) =', sx_Unlock( 10 )
   ? 'sx_Unlock( 20 ) =', sx_Unlock( 20 )
   ? 'sx_Unlock( 30 ) =', sx_Unlock( 30 )

   ? 'sx_Locked( 10 ) =', sx_Locked( 10 )
   ? 'sx_Locked( 20 ) =', sx_Locked( 20 )
   ? 'sx_Locked( 30 ) =', sx_Locked( 30 )

   ? 'sx_LockCount()  =', __trim( sx_LockCount() )
   aLockList := sx_DBRlockList()
   ? 'aLockList := sx_DBRlockList() =>', __trim( len( aLockList ) )
   aeval( aLockList, { |e, i| outstd( "aLockList[" + __trim(i ) + "] = " + __trim(e ), EOL ) } )

   ?
   ? 'sx_Rlock( 10 )  =', sx_Rlock( 10 )
   ? 'sx_Rlock( 20 )  =', sx_Rlock( 20 )
   ? 'sx_Rlock( 30 )  =', sx_Rlock( 30 )

   ? 'sx_UnLockAll( ) =', sx_UnLockAll()

   ? 'sx_Locked( 10 ) =', sx_Locked( 10 )
   ? 'sx_Locked( 20 ) =', sx_Locked( 20 )
   ? 'sx_Locked( 30 ) =', sx_Locked( 30 )

   ? 'sx_LockCount()  =', __trim( sx_LockCount() )
   aLockList := sx_DBRlockList()
   ? 'aLockList := sx_DBRlockList() =>', __trim( len( aLockList ) )
   aeval( aLockList, { |e, i| outstd( "aLockList[" + __trim(i ) + "] = " + __trim(e ), EOL ) } )

   sx_UnLockAll()

   ?
   ? 'sx_DBRlock( { 1, 2 ,3 ,50, 66 } )  =', sx_DBRlock( { 1, 2 , 3 , 50, 66 } )
   ? 'sx_LockCount()  =', __trim( sx_LockCount() )
   aLockList := sx_DBRlockList()
   ? 'aLockList := sx_DBRlockList() =>', __trim( len( aLockList ) )
   aeval( aLockList, { |e, i| outstd( "aLockList[" + __trim(i ) + "] = " + __trim(e ), EOL ) } )

   ? sx_LastRec()

   ? 'sx_UnLockAll( ) =', sx_UnLockAll()
   ?
   ? 'sx_Rlock( {3,6,8,9,23,55,78} )  =', sx_Rlock( { 3, 6, 8, 9, 23, 55, 78 } )
   ? 'sx_LockCount()  =', __trim( sx_LockCount() )
   aLockList := sx_DBRlockList()
   ? 'aLockList := sx_DBRlockList() =>', __trim( len( aLockList ) )
   aeval( aLockList, { |e, i| outstd( "aLockList[" + __trim(i ) + "] = " + __trim(e ), EOL ) } )
   ?
   ? 'sx_DbrUnLock( aLockList ) =', sx_DbrUnLock( aLockList )
   // ? 'sx_UnLock( aLockList ) =', sx_UnLock( aLockList )
   ?
   ? 'sx_LockCount()  =', __trim( sx_LockCount() )
   aLockList := sx_DBRlockList()
   ? 'aLockList := sx_DBRlockList() =>', __trim( len( aLockList ) )
   aeval( aLockList, { |e, i| outstd( "aLockList[" + __trim(i ) + "] = " + __trim(e ), EOL ) } )

   ? 'sx_LastRec()  =', sx_LastRec()
   ? 'sx_RecCount() =', sx_RecCount()

   CLOSE ALL

STATIC FUNCTION __trim( no )

   RETURN alltrim( str( no ) )
