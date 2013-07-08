*****************************************************
* Demo of timeout checking with INET/THREAD systems
* This file contains also a skeleton of socket
* service management through an async control thread.
*
* Contributed by Giancarlo Niccolai and Charles Kwon
*
* $Id: inetto.prg 9524 2012-07-03 21:11:10Z andijahja $
*

* This array contain the socket control objects that
* the control system will use to do it's cleanups

GLOBAL aCntSockets

* When holding this mutex, the services are signaling that
* they don't want to be interrupted by the controller
GLOBAL MutexCnt

PROCEDURE Main( cAddress, nPort, nTimeout )
   LOCAL Socket, CtlThreadID
   LOCAL nStatus := 0
   LOCAL nDots := 0

   CLEAR SCREEN


   /* Requesting Inet system startup */
   InetInit()

   IF cAddress == NIL
      cAddress := "intel.com"
   ENDIF

   IF nPort == NIL
      nPort := "80"
   ENDIF

   IF nTimeout == NIL
      nTimeout := "4"
   ENDIF

   @5, 10 SAY "Connecting to " + cAddress +":"+ nPort
   @6, 10 SAY "Timeout set to " + nTimeout + " seconds."

   nPort := Val( nPort )
   nTimeout := Val( nTimeout )

   /* Initializing socket control structure to void */
   aCntSockets := {}
   MutexCnt := HB_MutexCreate()

   /* Now starting the control thread */
   CtlThreadID := StartThread ( @ControlThread(), nTimeout )

   /* Now we can start how many services threads we want */
   StartThread ( @Connect(), cAddress, nPort, nTimeout )

   nDots := 0
   WHILE .T.
      ThreadSleep( 200 ) // always a sleep!
      @7, 10 + nDots SAY "."
      nDots ++
      HB_MutexLock( MutexCnt )
      IF Len( aCntSockets ) > 0 .and. aCntSockets[1][3] != 0
         HB_MutexUnlock( MutexCnt )
         EXIT
      ENDIF
      HB_MutexUnlock( MutexCnt )
   ENDDO

   // -1: connection timed out
   // 1: connection established or error while connecting

   DO CASE
      CASE aCntSockets[1][3] == -2
         @8, 10 say "Server name not resolved before timeout."

      CASE aCntSockets[1][3] == -1
         @8, 10 say "Connection not established before timeout."

      CASE aCntSockets[1][3] == 1
         @8, 10 say "Connection ESTABLISHED. Timeout aborted."

      CASE aCntSockets[1][3] == 2
         @8, 10 say "Connection REJECTED. Timeout aborted."

      CASE aCntSockets[1][3] == 3
         @8, 10 say "Server name not found."

   ENDCASE

   StopThread( CtlThreadID )
   DestroyMutex( MutexCnt )
   @10,10 SAY "Please, press a key to continue."

   INKEY(0)

   InetCleanup()

RETURN


/********************************************
* This function handles async connecton
* It uses another async thread to search the
* server name
*/

FUNCTION Connect( cAddress, nPort, nTimeout )
   LOCAL aServiceData
   LOCAL Socket, aServer
   LOCAL MutexDone, thSearcher
   LOCAL nPos, bSuccess

   /* We create now a vaild service data, without the socket */
   aServiceData := { InetCreate(), ThreadGetCurrent(), 0 , Seconds() }

   /* Locking here to prevent control thread to intervene in the middle
      of our operation */
   HB_MutexLock( MutexCnt )
   AAdd( aCntSockets, aServiceData )
   HB_MutexUnlock( MutexCnt )

   /* request to get the inet address of the server asynchronously */
   MutexDone := HB_MutexCreate()
   thSearcher := StartThread( @SearchForServer(), cAddress, MutexDone )

   /* we'll wait 1/2 of the timeout */

   aServer := Subscribe( MutexDone, 500 * nTimeout, @bSuccess )
   /*And if we have not found an answer, we return failure */
   HB_MutexLock( MutexCnt )

   IF .not. bSuccess
      aServiceData[3] := -2 /* timed out */
      HB_MutexUnlock( MutexCnt )
      /* Kill that thread, but without waiting for it to be done*/
      KillThread( thSearcher )
      RETURN NIL
   ENDIF

   /* But the resolver could also return a failure */
   IF Len( aServer ) == 0
      aServiceData[3] := 3 /* Name not found */
      HB_MutexUnlock( MutexCnt )
      RETURN NIL
   ENDIF

   HB_MutexUnlock( MutexCnt )

   /* now we can be interrupted */

   InetConnectIP( aServer[1], nPort, aServiceData[1] )

   /* now we need to be not interrupted */
   HB_MutexLock( MutexCnt )
   IF InetErrorCode( aServiceData[ 1 ] ) > 0
      aServiceData[ 3 ] := 2  // Connection rejected
   ELSE
      aServiceData[ 3 ] := 1  // Connection done.
   ENDIF

   aServiceData[ 4 ] := Seconds()
   aCntSockets[ 1 ] := aServiceData

   HB_MutexUnlock( MutexCnt )

RETURN NIL


/********************************************
* This is the control thread, that ensures that
* the threads have not reached a dead point
* or a too long blocking condition.
*/

FUNCTION ControlThread( nTimeout )
   LOCAL aTicket

   DO WHILE .T.
      ThreadSleep( 1000 )
      HB_MutexLock( MutexCnt )
      FOR EACH aTicket IN aCntSockets

         /* If status is still connecting ... */
         IF aTicket[ 3 ] == 0 .and. aTicket[ 4 ] + nTimeout < Seconds()
            StopThread( aTicket[ 2 ] )
            aTicket[ 3 ] := -1
            aTicket[ 1 ] := NIL // removes the socket
         ENDIF

         /* A complete app could have more status/timeout relations */

      NEXT
      HB_MutexUnlock( MutexCnt )

      /* In a complete enviroment, this thread should also remove
         unused tickets */

   ENDDO

RETURN NIL


/********************************************
* This function scans asynchronously internet
* for a server name
*/

FUNCTION SearchForServer( cName, MutexDone )
   LOCAL aServer

   aServer := InetGetHosts( cName )
   Notify( MutexDone, aServer )
RETURN NIL
