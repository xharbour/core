/*
 * $Id: trpc.prg 9614 2012-08-02 01:30:48Z andijahja $
 */

/*
 * xHarbour Project source code:
 * Remote Procedure Call code
 * xHarbour part
 *
 * Copyright 2003 Giancarlo Niccolai <giancarlo@niccolai.ws>
 * www - http://www.xharbour.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, if you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

/*
   XHB Remote procedure call protocol

   NOTES:
   All packets begin with the string "XHBR??" where ?? are 2 readable characters
   containing the packet type. In the protocol, a field in "" means a serialized
   string. A field in {} means a serialized array. A field in '' means a literal
   set of bytes (characters).

   Serialized strings: 4 chars lenght in network order, and then the string.
   Function serial numbers: "AAAAMMDD.C", where c is a developer defined character.

   UDP requests:
   00 - Server scan
      + "Server Name" ( valid regex or nothing )

   01 - Function scan
     + "Function Name" (valid regex)
     + "Minimum Serial" (00000000.0 for all)

   UDP replies:
   10 - Server available
      + "server name"
   11 - Function available
      +"server name"
      +"function description" as returned by tRPCFunction::Describe()

   12 - Too many functions in function request



   TCP requests:

   As UDP +

   20 - Function call
     <LEN8> - raw data length
     "Function name" + { Param1, ... Param N }

   21 - Compressed function call
     <LEN8> - Original data length
     <LEN8> - compressed data length
     * follows compressed data containing serialized name + params

   22 - Loop Function Call
     <LEN8> - Raw data length
     "A" or "C" or "E": send all results/ send compressed result/
         send confirmation at end
     Numeric BEGIN
     Numeric END
     Numeric STEP
     "Function name" + { Param1, ... Param N }
     Note: the parameter called $1 is the loop indicator

   23 - Loop Function Call / Compressed
     <LEN8> - Original data length
     <LEN8> - compressed data length
     "A" or "C" or "E": send all results/ send compressed result/
         send confirmation at end
     * follows compressed data containing:
     Numeric BEGIN
     Numeric END
     Numeric STEP
     "Function name" + { Param1, ... Param N }
     Note: the parameter called $1 is the loop indicator

   24 - Foreach function call
     <LEN8> - raw data length
     "A" or "C" or "E": send all results/ send compressed result/
         send confirmation at end
     "Function name" + { Param1, ... Param N }
     +Array containing the elements
     Note: the parameter called $1 is substitued with the foreach

   25 - Foreach function call / Compressed
     <LEN8> - Original data length
     <LEN8> - compressed data length
     "A" or "C" or "E": send all results/ send compressed result/
         send confirmation at end
     * follows compressed data containing:
     "Function name" + { Param1, ... Param N }
     +Array containing the elements
     Note: the parameter called $1 is substitued with the foreach

   29 - Cancelation request

   TCP REPLIES:

   30 - Function result
      + <LEN8> - raw data len
      + Serialized result

   31 - Compressed result
     + <LEN8> - Original data length
     + <LEN8> -compressed data len
     + Compressed data containing serialized result

   33 - Progress
     + Serialized progress number (0 to 100 float)

   34 - Progress with raw data
     + Serialized progress number (0 to 100 float) (10 chars)
     + <LEN8> - raw data len
     + Uncompressed progress data

   35 - Progress with compressed data
     + Serialized progress number (0 to 100 float) (10 chars)
     + <LEN8> - Original data length
     + <LEN8> - compressed data lenght
     + Compressed progress data


   40 - Function call error
      00 - Function not present
      01 - Not enough level
      02 - wrong parameters

      10 - Internal function error
         <LEN8> (function specific error code)+ cErrorDesc:45
      11 - busy, retry later

      20 - Protocol error

   90 - LOGIN
      <LEN8> + USERID:PASSWORD
   91 - LOGIN STATUS
      'OK'
      'NO'
   92 - GOODBYE

   93 - Encripted login
      <LEN8> Total length
      'USERID:ENCRYPTED( Random data + PASSWORD:pwd: + Random data)'

   94 - Challenge
      <LEN8> Total length
      'ENCRYPT(CHALLENGE DATA)'

   95 - Challenge reply
      <NUM8> - the CRC32 checksum of challenge.


*/

/* #ifdef HB_THREAD_SUPPORT */

#include "hbclass.ch"
#include "hbrpc.ch"


/************************************
* RPC FUNCTION
*************************************/

CLASS tRPCFunction
   DATA cName
   DATA aParameters
   DATA cReturn
   DATA cSerial
   DATA nAuthLevel

   DATA oExecutable
   DATA oMethod

   DATA aCall

   CLASSDATA cPattern INIT HB_RegexComp( "^C:[0-9]{1,6}$|^A$|^O$|^D$|^N$|^NI$")

   METHOD New( cFname, cSerial, cFret, aParams, nAuthLevel, oExec, oMethod ) CONSTRUCTOR
   METHOD SetCallable( oExecSymbol, oMethod )
   METHOD CheckTypes( aParams )
   METHOD CheckParam( cParam )
   METHOD Describe()
   METHOD Run( aParams, oClient )
ENDCLASS


METHOD New( cFname, cSerial, nAuthLevel, oExec, oMeth ) CLASS tRPCFunction
   LOCAL cParam
   LOCAL aParams, aFuncDef

   // Analyze the function definition
   aFuncDef := HB_Regex( "^([a-zA-Z0-9_-]+)\(([^)]*)\) *(-->)? *(.*)$", cFname )
   IF Empty( aFuncDef )
      Alert( "Invalid function defintion" )
      ErrorLevel( 1 )
      QUIT
   ENDIF

   ::cName := aFuncDef[2]
   cParam := aFuncDef[3]
   ::cReturn := IIF( Len( aFuncDef ) == 4, aFuncDef[4], aFuncDef[5] )

   // analyze parameter list
   IF Len( Trim( cParam ) ) > 0
      aParams := HB_RegexSplit( ",", cParam )
      ::aParameters := {}
      FOR EACH cParam IN aParams
         cParam := AllTrim( Upper(cParam) )
         ::CheckParam( cParam )
         AAdd( ::aParameters, cParam )
      NEXT
   ELSE
      ::aParameters := {}
   ENDIF

   // Analyze function definition return
   ::CheckParam( ::cReturn )

   // Analyze function serial number
   IF .not. HB_RegexMatch( "[0-9]{8}\..", cSerial )
      Alert( "Serial value not valid" )
      ErrorLevel( 1 )
      QUIT
   ENDIF

   // analyze function authorization level
   IF nAuthLevel < 1
      Alert( "Authorization level must be at least 1" )
      ErrorLevel( 1 )
      QUIT
   ENDIF

   ::cSerial := cSerial
   ::nAuthLevel := nAuthLevel

   // Set now Executable object if given
   IF oExec != NIL
      ::SetCallable( oExec, oMeth )
   ENDIF

RETURN Self


METHOD SetCallable( oExec, oMeth ) CLASS tRPCFunction
   // If the callable is an object, we need to store the method
   IF ValType( oExec ) == "O"
      ::aCall := Array( Len( ::aParameters ) + 3 )
      ::aCall[2] := oMeth
   ELSE
      ::aCall := Array( Len( ::aParameters ) + 2 )
   ENDIF

   ::aCall[1] := oExec

RETURN .T.

METHOD Run( aParams, oClient ) CLASS tRPCFunction
   LOCAL nStart, nCount, xRet

   IF .not. ::CheckTypes( aParams )
      RETURN NIL
   ENDIF

   nStart := IIF( ValType( ::aCall[1] ) == "O", 3, 2 )

   FOR nCount := 1 TO Len( aParams )
      ::aCall[ nStart ] := aParams[ nCount ]
      nStart ++
   NEXT

   ::aCall[ nStart ] := oClient

   xRet := HB_ExecFromArray( ::aCall )
RETURN xRet


METHOD CheckParam( cParam ) CLASS tRPCFunction
   IF .not. HB_RegexMatch( ::cPattern, cParam )
      Alert("tRPCFunction:CheckParam() wrong parameter specification: " + cParam )
      QUIT
   ENDIF
RETURN .T.


METHOD CheckTypes( aParams ) CLASS tRPCFunction
   LOCAL oElem, i := 0

   IF ValType( aParams ) != 'A'
      RETURN .F.
   ENDIF

   IF Len( aParams ) != Len( ::aParameters )
      RETURN .F.
   ENDIF

   FOR EACH oElem in ::aParameters
      i++
      IF ValType( aParams[i] ) != oElem[1]
         RETURN .F.
      ENDIF
   NEXT
RETURN .T.


METHOD Describe() CLASS tRPCFunction
   LOCAL cRet := ::cName + "("
   LOCAL nCount

   IF Len( ::aParameters ) > 0
      FOR nCount := 1 TO Len( ::aParameters ) -1
         cRet += ::aParameters[nCount] + ","
      NEXT
      cRet += ::aParameters[ -1 ]
   ENDIF

   cRet += ")-->" + ::cReturn

RETURN cRet+"/" + ::cSerial


/***********************************************************
* Connection manager class; this manages a single connection
************************************************************/

CLASS tRPCServeCon
   /* back reference to the parent to get callback blocks */
   DATA oServer

   /* Socket, mutex and thread */
   DATA skRemote
   DATA mtxBusy
   DATA thSelf INIT NIL

   /* Assigned authorization level */
   DATA nAuthLevel

   /* User ID */
   DATA cUserId

   /* Allow progress ?*/
   DATA lAllowProgress

   METHOD New( oCaller, skRemote ) CONSTRUCTOR
   METHOD Destroy()

   /* Managing async */
   METHOD Start()
   METHOD Stop()
   METHOD Run()

   /* Utilty */
   METHOD SendProgress( nProgress, aData )
   METHOD IsCanceled()        INLINE ::lCanceled

   METHOD GetStatus()         INLINE ::nStatus
   METHOD FunctionRunner( cFuncName, oFunc, nMode, aParams, aItems )
HIDDEN:
   /* Current status */
   DATA  nStatus              INIT RPCS_STATUS_NONE
   /* Is this connection encrypted? */
   DATA bEncrypted
   /* crc for challenge handshake */
   DATA nChallengeCRC
   /* Temporary supposed user in challenge */
   DATA cChallengeUserid
   DATA cCryptKey

   /* Function execution data */
   DATA thFunction   INIT NIL
   DATA lCanceled    INIT  .F.

   METHOD RecvAuth( lEncrypt )
   METHOD RecvChallenge()
   METHOD RecvFunction( bComp, bMode )
   METHOD FuncCall( cData )
   METHOD FuncLoopCall( cData, cMode )
   METHOD FuncForeachCall( cData, cMode )
   METHOD LaunchChallenge( cUserid, cPassword )
   METHOD LaunchFunction( cFuncName, aParms, nMode, aItems )
   /* METHOD FunctionRunner( cFuncName, oFunc, nMode, aParams, aItems ) */
   METHOD SendResult( oRet )

   METHOD Encrypt(cDataIn)
   METHOD Decrypt(cDataIn)

ENDCLASS


METHOD New( oParent, skIn ) CLASS tRPCServeCon
   ::oServer := oParent
   ::skRemote := skIn
   ::mtxBusy := HB_MutexCreate()
   ::bEncrypted := .F.
   ::nAuthLevel := 0
   ::nChallengeCRC := -1
RETURN Self


METHOD Destroy() CLASS tRPCServeCon
   HB_MutexLock( ::mtxBusy )
   // Eventually wait for the function to terminate
   IF ::thFunction != NIL
      ::lCanceled := .T.
      HB_MutexUnlock( ::mtxBusy )
      JoinThread( ::thFunction )
      HB_MutexLock( ::mtxBusy )
   ENDIF

   ::skRemote := NIL
   HB_MutexUnlock( ::mtxBusy )
RETURN .T.


METHOD Start() CLASS tRPCServeCon
   LOCAL lRet := .F.

   HB_MutexLock( ::mtxBusy )
   IF ::thSelf == NIL
      ::thSelf := StartThread( Self, "RUN" )
      lRet := .T.
   ENDIF
   HB_MutexUnlock( ::mtxBusy )

RETURN lRet


METHOD Stop() CLASS tRPCServeCon
   LOCAL lRet := .F.

   HB_MutexLock( ::mtxBusy )
   IF IsValidThread( ::thSelf )
      KillThread( ::thSelf )
      lRet := .T.
      HB_MutexUnlock( ::mtxBusy )
      JoinThread( ::thSelf )
      ::thSelf := NIL
   ELSE
      HB_MutexUnlock( ::mtxBusy )
   ENDIF

RETURN lRet


METHOD Run() CLASS tRPCServeCon
   LOCAL cCode := Space( 6 )
   LOCAL lBreak := .F.
   LOCAL aData
   LOCAL nSafeStatus

   DO WHILE InetErrorCode( ::skRemote ) == 0 .and. .not. lBreak

      /* Get the request code */
      InetRecvAll( ::skRemote, @cCode, 6 )
      IF InetErrorCode( ::skRemote ) != 0
         EXIT
      ENDIF

      HB_MutexLock( ::mtxBusy )
      nSafeStatus := ::nStatus
      HB_MutexUnlock( ::mtxBusy )

      DO CASE

         /* Check for TCP server scan */
         CASE cCode == "XHBR00"
            InetSendAll( ::skRemote, ;
               "XHBR10"+ HB_Serialize( ::oServer:cServerName ) )

         /* Read autorization request */
         CASE cCode == "XHBR90"
            IF nSafeStatus == RPCS_STATUS_NONE
               lBreak := .not. ::RecvAuth( .F. )
               IF .not. lBreak
                  nSafeStatus := RPCS_STATUS_LOGGED
               ENDIF
            ELSE
               nSafeStatus := RPCS_STATUS_ERROR
            ENDIF

         /* Read encrypted autorization request */
         CASE cCode == "XHBR93"
            IF nSafeStatus == RPCS_STATUS_NONE
               lBreak := .not. ::RecvAuth( .T. )
               IF .not. lBreak
                  nSafeStatus := RPCS_STATUS_CHALLENGE
               ENDIF
            ELSE
               nSafeStatus := RPCS_STATUS_ERROR
            ENDIF

         /* Challeng reply */
         CASE cCode == "XHBR95"
            IF nSafeStatus == RPCS_STATUS_CHALLENGE
               lBreak := .not. ::RecvChallenge( )
               IF .not. lBreak
                  nSafeStatus := RPCS_STATUS_LOGGED
               ENDIF
            ELSE
               nSafeStatus := RPCS_STATUS_ERROR
            ENDIF

         /* Close connection */
         CASE cCode == "XHBR92"
            ::oServer:OnClientLogout( Self )
            lBreak := .T.

         /* Execute function */
         CASE cCode == "XHBR20"
            IF nSafeStatus == RPCS_STATUS_LOGGED
               aData := ::RecvFunction( .F., .F. )
               IF aData != NIL
                  lBreak := .not. ::FuncCall( aData[2] )
               ELSE
                  lBreak := .T.
               ENDIF
            ELSEIF nSafeStatus == RPCS_STATUS_RUNNING
               nSafeStatus := RPCS_STATUS_BUSY
            ELSE
               nSafeStatus := RPCS_STATUS_ERROR
            ENDIF


         /* Execute function */
         CASE cCode == "XHBR21"
            IF nSafeStatus == RPCS_STATUS_LOGGED
               aData := ::RecvFunction( .T., .F. )
               IF aData != NIL
                  lBreak := .not. ::FuncCall( aData[2] )
               ELSE
                  lBreak := .T.
               ENDIF
            ELSEIF nSafeStatus == RPCS_STATUS_RUNNING
               nSafeStatus := RPCS_STATUS_BUSY
            ELSE
               nSafeStatus := RPCS_STATUS_ERROR
            ENDIF

         /* Loop function */
         CASE cCode == "XHBR22"
            IF nSafeStatus == RPCS_STATUS_LOGGED
               aData := ::RecvFunction( .F., .T. )
               IF aData != NIL
                  lBreak := .not. ::FuncLoopCall( aData[1], aData[2] )
               ELSE
                  lBreak := .T.
               ENDIF
            ELSEIF nSafeStatus == RPCS_STATUS_RUNNING
               nSafeStatus := RPCS_STATUS_BUSY
            ELSE
               nSafeStatus := RPCS_STATUS_ERROR
            ENDIF

         /* Loop function - compressed */
         CASE cCode == "XHBR23"
            IF nSafeStatus == RPCS_STATUS_LOGGED
               aData := ::RecvFunction( .T., .T. )
               IF aData != NIL
                  lBreak := .not. ::FuncLoopCall( aData[1], aData[2] )
               ELSE
                  lBreak := .T.
               ENDIF
            ELSEIF nSafeStatus == RPCS_STATUS_RUNNING
               nSafeStatus := RPCS_STATUS_BUSY
            ELSE
               nSafeStatus := RPCS_STATUS_ERROR
            ENDIF

         /* Foreach function */
         CASE cCode == "XHBR24"
            IF nSafeStatus == RPCS_STATUS_LOGGED
               aData := ::RecvFunction( .F., .T. )
               IF aData != NIL
                  lBreak := .not. ::FuncForeachCall( aData[1], aData[2] )
               ELSE
                  lBreak := .T.
               ENDIF
            ELSEIF nSafeStatus == RPCS_STATUS_RUNNING
               nSafeStatus := RPCS_STATUS_BUSY
            ELSE
               nSafeStatus := RPCS_STATUS_ERROR
            ENDIF

         /* Foreach function - compressed*/
         CASE cCode == "XHBR25"
            IF nSafeStatus == RPCS_STATUS_LOGGED
               aData := ::RecvFunction( .T., .T. )
               IF aData  != NIL
                  lBreak := .not. ::FuncForeachCall( aData[1], aData[2] )
               ELSE
                  lBreak := .T.
               ENDIF
            ELSEIF nSafeStatus == RPCS_STATUS_RUNNING
               nSafeStatus := RPCS_STATUS_BUSY
            ELSE
               nSafeStatus := RPCS_STATUS_ERROR
            ENDIF

         /* Function execution cancelation request */
         CASE cCode == "XHBR29"
            /* Note: even if the function is already terminated in the
               meanwhile, and the -real- status is not RUNNING anymore,
               there is no problem here. The cancelation request will
               be reset at next function call, and the caller must ignore
               any pending data before the "cancel" call */
            IF nSafeStatus != RPCS_STATUS_RUNNING
               nSafeStatus := RPCS_STATUS_ERROR
            ELSE
               HB_MutexLock( ::mtxBusy )
               ::lCanceled = .T.
               HB_MutexUnlock( ::mtxBusy )
               InetSendAll( ::skRemote, "XHBR34")
            ENDIF

         OTHERWISE
            lBreak := .T.
      ENDCASE

      /* Analisys of the nSafeStatus code */
      DO CASE
         CASE nSafeStatus == RPCS_STATUS_BUSY
            InetSendAll( ::skRemote, "XHBR4011" )

         CASE nSafeStatus == RPCS_STATUS_ERROR
            InetSendAll( ::skRemote, "XHBR4020" )

         /* Update real status only if not in error case */
         OTHERWISE
            /* The running status is set (in this thread) indipendently
               by the function launcher, if everything is fine */
            HB_MutexLock( ::mtxBusy )
            IF ::nStatus != RPCS_STATUS_RUNNING
               ::nStatus := nSafeStatus
            ENDIF
            HB_MutexUnlock( ::mtxBusy )
      ENDCASE

   ENDDO

   // signaling termination of this thread
   ::oServer:Terminating( Self )
   // Destroy resources just before termination
   ::Destroy()
RETURN .T.


METHOD RecvAuth( lEncrypt ) CLASS tRPCServeCon
   LOCAL cLength := Space(8), nLen, nPos
   LOCAL cUserID, cPassword
   LOCAL cReadIn

   IF InetRecvAll( ::skRemote, @cLength, 8 ) != 8
      RETURN .F.
   ENDIF

   nLen := HB_GetLen8( cLength )

   IF (lEncrypt .and. nLen > 128 ) .or. ( .not. lEncrypt .and. nLen > 37 )
      RETURN .F.
   ENDIF

   cReadIn := Space( nLen )
   IF InetRecvAll( ::skRemote, @cReadin, nLen ) != nLen
      RETURN .F.
   ENDIF

   nPos := At( ":", cReadin )
   IF nPos == 0
      RETURN .F.
   ENDIF

   cUserID := Substr(cReadin, 1, nPos-1 )
   cPassword := Substr( cReadin, nPos+1 )

   IF .not. lEncrypt
      ::nAuthLevel := ::oServer:Authorize( cUserid, cPassword )
      IF ::nAuthLevel == 0
         InetSendAll( ::skRemote, "XHBR91NO" )
         RETURN .F.
      ENDIF

      InetSendAll( ::skRemote, "XHBR91OK" )
      IF InetErrorCode( ::skRemote ) != 0
         RETURN .F.
      ENDIF
      ::cUserId := cUserId
      ::oServer:OnClientLogin( Self )
      RETURN .T.
   ENDIF

RETURN ::LaunchChallenge( cUserid, cPassword )


METHOD LaunchChallenge( cUserid, cPassword ) CLASS tRPCServeCon
   LOCAL cChallenge, nCount

   ::cCryptKey := ::oServer:AuthorizeChallenge( cUserid, cPassword )
   IF Empty( ::cCryptKey )
      RETURN .F.
   ENDIF

   ::cChallengeUserid := cUserid

   /* Let's generate the sequence */
   cChallenge := Space( 255 )
   FOR nCount := 1 TO 255
      cChallenge[ nCount ] := Chr( HB_Random(0,255 ) )
   NEXT

   ::nChallengeCRC = HB_Checksum( cChallenge )
   cChallenge := HB_Crypt( cChallenge, ::cCryptKey )

   InetSendAll( ::skRemote, "XHBR94" + HB_CreateLen8( Len( cChallenge ) ) + cChallenge )

   IF InetErrorCode( ::skRemote ) != 0
      RETURN .F.
   ENDIF

RETURN .T.


METHOD RecvChallenge() CLASS tRPCServeCon
   LOCAL cNumber := Space( 8 )

   IF InetRecvAll( ::skRemote, @cNumber ) != 8
      RETURN .F.
   ENDIF

   IF ::nChallengeCRC != HB_GetLen8( cNumber )
      RETURN .F.
   ENDIF

   InetSendAll( ::skRemote, "XHBR91OK" )
   IF InetErrorCode( ::skRemote ) != 0
      RETURN .F.
   ENDIF

   ::nAuthLevel := ::oServer:Authorize( ::cChallengeUserid )
   /* It is always possible that the user has been deleted in the meanwhile */
   IF ::nAuthLevel == 0
      RETURN .F.
   ENDIF

   ::cUserId := ::cChallengeUserid
   ::bEncrypted := .T.
   ::oServer:OnClientLogin( Self )

RETURN .T.


METHOD RecvFunction( bComp, bMode ) CLASS tRPCServeCon
   LOCAL cLength := Space(8), nLen, nComp
   LOCAL cMode := " "
   LOCAL cData

   /* Original lenght of data */
   IF InetRecvAll( ::skRemote, @cLength, 8 ) != 8
      RETURN NIL
   ENDIF

   nLen := HB_GetLen8( cLength )
   IF nLen > 65000
      RETURN NIL
   ENDIF

   /* compressed lenght */
   IF bComp
      IF InetRecvAll( ::skRemote, @cLength, 8 ) != 8
         RETURN NIL
      ENDIF

      nComp := HB_GetLen8( cLength )
   ELSE
      nComp := nLen
   ENDIF

   /* Mode */
   IF bMode
      IF InetRecvAll( ::skRemote, @cMode ) != 1
         RETURN NIL
      ENDIF
   ENDIF

   /* Get data */
   cData := Space( nComp )
   IF InetRecvAll( ::skRemote, @cData ) != nComp
      RETURN NIL
   ENDIF

   /* Eventually decrypt it */
   IF ::bEncrypted
      cData := ::Decrypt( cData )
   ENDIF

   /* Eventually uncompress it */
   IF bComp
      cData := HB_Uncompress( nLen, cData )
   ENDIF

RETURN { cMode, cData }


METHOD FuncCall( cData ) CLASS tRPCServeCon
   LOCAL cSer, cFuncName, aParams

   /* Deserialize all elements */
   cSer := HB_DeserialBegin( cData )
   IF cSer == NIL
      RETURN .F.
   ENDIF
   cFuncName := HB_DeserialNext( @cSer )
   aParams := HB_DeserialNext( @cSer )

   IF aParams == NIL
      RETURN .F.
   ENDIF

   ::oServer:OnClientRequest( Self, 20, { cFuncName, aParams } )
RETURN ::LaunchFunction( cFuncName, aParams, 0 )


METHOD FuncLoopCall( cMode, cData ) CLASS tRPCServeCon
   LOCAL nBegin, nEnd, nStep
   LOCAL cSer
   LOCAL cFuncName, aParams

   /* Deserialize all elements */
   cSer := HB_DeserialBegin( cData )
   IF Empty( cSer )
      RETURN .F.
   ENDIF
   nBegin := HB_DeserialNext( @cSer )
   nEnd := HB_DeserialNext( @cSer )
   nStep := HB_DeserialNext( @cSer )
   cFuncName := HB_DeserialNext( @cSer )
   aParams := HB_DeserialNext( @cSer )

   IF aParams == NIL
      RETURN .F.
   ENDIF

   ::oServer:OnClientRequest( Self, 22, { cFuncName, aParams, cMode, nBegin, nEnd, nStep } )
RETURN ::LaunchFunction( cFuncName, aParams, 1, { cMode, nBegin, nEnd, nStep } )


METHOD FuncForeachCall( cMode, cData ) CLASS tRPCServeCon
   LOCAL cSer
   LOCAL cFuncName, aParams
   LOCAL aItems

   /* Deserialize all elements */
   cSer := HB_DeserialBegin( cData )
   IF Empty( cSer )
      RETURN .F.
   ENDIF

   cFuncName := HB_DeserialNext( @cSer )
   aParams := HB_DeserialNext( @cSer )
   aItems := HB_DeserialNext( @cSer )

   IF aItems  == NIL
      RETURN .F.
   ENDIF

   ::oServer:OnClientRequest( Self, 24, { cFuncName, aParams, aItems } )
RETURN ::LaunchFunction( cFuncName, aParams, 2, { cMode, aItems } )


METHOD LaunchFunction( cFuncName, aParams, nMode, aDesc ) CLASS tRPCServeCon
   LOCAL oFunc

   //Check for function existance
   oFunc := ::oServer:Find( cFuncName )
   IF Empty(oFunc)
      // signal error
      ::oServer:OnFunctionError( Self, cFuncName, 00 )
      InetSendAll( ::skRemote, "XHBR4000" )
      RETURN .T.
   ENDIF

   // check for level
   IF oFunc:nAuthLevel > ::nAuthLevel
      // signal error
      ::oServer:OnFunctionError( Self, cFuncName, 01 )
      InetSendAll( ::skRemote, "XHBR4001" )
      RETURN .T.
   ENDIF

   //check for parameters
   IF aParams == NIL .or. .not. oFunc:CheckTypes( aParams )
      // signal error
      ::oServer:OnFunctionError( Self, cFuncName,02 )
      InetSendAll( ::skRemote, "XHBR4002" )
      RETURN .T.
   ENDIF

   HB_MutexLock( ::mtxBusy )
   // allow progress indicator by default
   ::lAllowProgress := .T.
   // setting the cancel indicator as false
   ::lCanceled := .F.
   // Set the running status
   ::nStatus := RPCS_STATUS_RUNNING
   ::thFunction := StartThread( Self, "FunctionRunner", ;
      cFuncName, oFunc, nMode, aParams, aDesc )
   HB_MutexUnlock( ::mtxBusy )

RETURN .T.


METHOD FunctionRunner( cFuncName, oFunc, nMode, aParams, aDesc ) CLASS tRPCServeCon
   LOCAL nCount
   LOCAL oRet, oElem, aRet
   LOCAL aSubst, nSubstPos

   //? "TH:", ::thFunction
   DO CASE

      CASE nMode == 0  // just run the function
         oRet := oFunc:Run( aParams, Self )
         // Notice: SendResult checks for lCanceled before really sending

      CASE nMode == 1 // run in loop
         aSubst := AClone( aParams )
         nSubstPos := AScan( aParams, {|x| ValType( x ) == "C" .and. x == "$."} )

         SWITCH aDesc[1]
            CASE 'A' // all results
               FOR nCount := aDesc[ 2 ] TO aDesc[ 3 ] STEP aDesc[ 4 ]
                  IF nSubstPos > 0
                     aSubst[ nSubstPos ] := nCount
                  ENDIF
                  oRet := oFunc:Run( aSubst, Self )
                  ::SendResult( oRet, cFuncName )
               NEXT
               oRet := "Done"
            EXIT

            CASE 'C' // Vector of all results
               aRet := {}
               ::lAllowProgress = .F.
               FOR nCount := aDesc[ 2 ] TO aDesc[ 3 ] STEP aDesc[ 4 ]
                  IF nSubstPos > 0
                     aSubst[ nSubstPos ] := nCount
                  ENDIF
                  oRet :=  oFunc:Run( aSubst, Self )
                  IF oRet == NIL
                     ::SendResult( NIL, cFuncName )
                     EXIT
                  ENDIF
                  AAdd( aRet, oRet )
               NEXT
               IF oRet != NIL
                  oRet := aRet
               ENDIF
            EXIT

            CASE 'E' // Just send confirmation at end
               ::lAllowProgress = .F.
               FOR nCount := aDesc[ 2 ] TO aDesc[ 3 ] STEP aDesc[ 4 ]
                  IF nSubstPos > 0
                     aSubst[ nSubstPos ] := nCount
                  ENDIF
                  oRet := oFunc:Run( aSubst, Self )
                  IF oRet == NIL
                     ::SendResult( NIL, cFuncName )
                     EXIT
                  ENDIF
               NEXT
               IF oRet != NIL
                  oRet := "Done"
               ENDIF
            EXIT
         END

      CASE nMode == 2 // Run in a foreach loop
         aSubst := AClone( aParams )
         nSubstPos := AScan( aParams, {|x| ValType( x ) == "C" .and. x == "$."} )

         SWITCH aDesc[1]
            CASE 'A' // all results
               FOR EACH oElem IN  aDesc[ 2 ]
                  IF nSubstPos > 0
                     aSubst[ nSubstPos ] := oElem
                  ENDIF
                  oRet := oFunc:Run( aSubst, Self )
                  ::SendResult( oRet, cFuncName )
               NEXT
               oRet := "Done"
            EXIT

            CASE 'C' // Vector of all results
               aRet := {}
               ::lAllowProgress = .F.
               FOR EACH oElem IN  aDesc[ 2 ]
                  IF nSubstPos > 0
                     aSubst[ nSubstPos ] := oElem
                  ENDIF
                  oRet := oFunc:Run( aSubst, Self )
                  IF oRet == NIL
                     ::SendResult( NIL, cFuncName )
                     EXIT
                  ENDIF
                  AAdd( aRet, oRet )
               NEXT
               IF oRet != NIL
                  oRet := aRet
               ENDIF
            EXIT

            CASE 'E' // Just send confirmation at end
               ::lAllowProgress = .F.
               FOR EACH oElem IN aDesc[ 2 ]
                  IF nSubstPos > 0
                     aSubst[ nSubstPos ] := oElem
                  ENDIF
                  oRet := oFunc:Run( aSubst, Self )
                  IF oRet == NIL
                     EXIT
                  ENDIF
               NEXT
            EXIT
         END
   ENDCASE

   // Now we can signal that execution terminated
   HB_MutexLock( ::mtxBusy )
   ::nStatus := RPCS_STATUS_LOGGED
   HB_MutexUnlock( ::mtxBusy )
   // The execution of the function terminates BEFORE the sending of
   // the last data or the confirmation data, even if the thread
   // has still something to do.
   ::SendResult( oRet, cFuncName )

   //Signal that the thread is no longer alive
   // Should not be needed!
   /*HB_MutexLock( ::mtxBusy )
   ::thFunction := -1
   HB_MutexUnlock( ::mtxBusy )*/
RETURN .T.


METHOD SendResult( oRet, cFuncName )
   LOCAL cData, cOrigLen, cCompLen

   // Ignore requests to send result if function is canceled
   HB_MutexLock( ::mtxBusy )
   IF ::lCanceled
      HB_MutexUnlock( ::mtxBusy )
      ::oServer:OnFunctionCanceled( Self, cFuncName )
      RETURN .T. //as if it were done
   ENDIF
   HB_MutexUnlock( ::mtxBusy )

   IF oRet == NIL
      ::oServer:OnFunctionError( Self, cFuncName, 10 )
      InetSendAll( ::skRemote, "XHBR4010" )
   ELSE
      cData := HB_Serialize( oRet )
      cOrigLen := HB_CreateLen8( Len( cData ) )
      ::oServer:OnFunctionReturn( Self, cData )
      // should we compress it ?

      IF Len( cData ) > 512
         cData := HB_Compress( cData )
         cCompLen := HB_CreateLen8( Len( cData ) )
         InetSendAll( ::skRemote, "XHBR31" + cOrigLen + cCompLen + ::Encrypt( cData ) )
      ELSE
         InetSendAll( ::skRemote, "XHBR30" + cOrigLen + ::Encrypt( cData ) )
      ENDIF
   ENDIF

   IF InetErrorCode( ::skRemote ) != 0
      RETURN .F.
   ENDIF

RETURN .T.


METHOD SendProgress( nProgress, oData ) CLASS tRPCServeCon
   LOCAL cOrigLen, cCompLen, lRet := .T.
   LOCAL cData

   //Ignore if told so
   HB_MutexLock( ::mtxBusy )
   IF .not. ::lAllowProgress .or. ::lCanceled
      HB_MutexUnlock( ::mtxBusy )
      RETURN .T.
   ENDIF
   HB_MutexUnlock( ::mtxBusy )

   ::oServer:OnFunctionProgress( Self, nProgress, oData )
   IF Empty( oData )
      InetSendAll( ::skRemote, "XHBR33" + HB_Serialize( nProgress ) )
   ELSE
      cData := HB_Serialize( oData )
      cOrigLen := HB_CreateLen8( Len( cData ) )
      // do we should compress it ?
      IF Len( cData ) > 512
         cData := HB_Compress( cData )
         cCompLen := HB_CreateLen8( Len( cData ) )
         InetSendAll(::skRemote, "XHBR35" + HB_Serialize( nProgress ) +;
                cOrigLen + cCompLen + ::Encrypt( cData ) )
      ELSE
         InetSendAll( ::skRemote, "XHBR34" + HB_Serialize( nProgress ) +;
               cOrigLen + ::Encrypt( cData ) )
      ENDIF
   ENDIF

   IF InetErrorCode( ::skRemote ) != 0
      lRet := .F.
   ENDIF

RETURN lRet


METHOD Encrypt(cDataIn) CLASS tRPCServeCon
   IF ::bEncrypted
      RETURN HB_Crypt( cDataIn, ::cCryptKey )
   ENDIF
RETURN cDataIn


METHOD Decrypt(cDataIn) CLASS tRPCServeCon
   IF ::bEncrypted
      RETURN HB_Decrypt( cDataIn, ::cCryptKey )
   ENDIF
RETURN cDataIn

/************************************
* RPC SERVICE
*************************************/

CLASS tRPCService
   DATA cServerName INIT "RPCGenericServer"
   DATA aFunctions
   CLASSDATA lInit INIT InetInit()

   DATA nUdpPort INIT 1139
   DATA nTcpPort INIT 1140
   DATA cBindAddress INIT NIL
   DATA thAccept INIT 0
   DATA thUdp INIT 0
   DATA aServing INIT {}
   DATA mtxBusy INIT HB_MutexCreate()

   DATA skUdp
   DATA skServer

   /* Code blocks corresponding to event handlers */
   DATA bAuthorize
   DATA bGetEncryption
   DATA bOnFunctionScan
   DATA bOnServerScan
   DATA bOnClientConnect
   DATA bOnClientLogin
   DATA bOnClientRequest
   DATA bOnFunctionProgress
   DATA bOnFunctionError
   DATA bOnFunctionReturn
   DATA bOnFunctionCanceled
   DATA bOnClientLogout
   DATA bOnClientTerminate

   METHOD New() CONSTRUCTOR

   /* Block run on client connection request */
   DATA bConnection

   /* Function management */
   METHOD Add( xFunction, cVersion, nId, oExec, oMethod )
   METHOD Run( cName, aParams )
   METHOD Describe( cName )
   METHOD Find( cName )
   METHOD Remove( cName )

   /* General services */
   METHOD Start( lStartUdp )
   METHOD Stop()
   METHOD StartService( skIn )
   METHOD Terminating( oConnection )

   /* Tcp services */
   METHOD Accept()

   /* UDP services */
   METHOD UdpListen()
   METHOD UDPParseRequest()
   METHOD UDPInterpretRequest( cData, nPacketLen, cRes )

   /* Utility */
   METHOD AuthorizeChallenge( cUserid, cPassword )

   /* to be overloaded */
   METHOD Authorize( cUserid, cPassword )
   /* Provide encryption key for a user */
   METHOD GetEncryption( cUserId )
   METHOD OnFunctionScan()
   METHOD OnServerScan( )
   METHOD OnClientConnect( oClient )
   METHOD OnClientLogin( oClient )
   METHOD OnClientRequest( oClient, nRequest, cData )
   METHOD OnFunctionProgress( oClient, nProgress, aData )
   METHOD OnFunctionError( oClient, cFuncName, nError )
   METHOD OnFunctionReturn( oClient, aData )
   METHOD OnFunctionCanceled( oClient, cFuncName )
   METHOD OnClientLogout( oClient )
   METHOD OnClientTerminate( oClient )

ENDCLASS


METHOD New() class tRPCService
   ::aFunctions := {}
RETURN Self


METHOD Add( xFunction, cVersion, nLevel, oExec, oMethod )
   LOCAL nElem, lRet := .F.
   LOCAL oFunction

   IF ValType( xFunction ) == "C"
      oFunction := TRpcFunction():New( xFunction, cVersion, nLevel, oExec, oMethod )
   ELSE
      oFunction := xFunction
   ENDIF

   HB_MutexLock( ::mtxBusy )
   nElem := AScan( ::aFunctions, {|x| oFunction:cName == x:cName})
   IF nElem == 0
      Aadd( ::aFunctions  , oFunction )
      lRet := .T.
   ENDIF
   HB_MutexUnlock( ::mtxBusy )
RETURN lRet


METHOD Find( cName ) class tRPCService
   LOCAL nElem
   LOCAL oRet := NIL

   HB_MutexLock( ::mtxBusy )
   nElem := AScan( ::aFunctions, {|x| upper(cName) == upper(x:cName)})
   IF nElem != 0
      oRet := ::aFunctions[ nElem ]
   ENDIF
   HB_MutexUnlock( ::mtxBusy )
RETURN oRet


METHOD Remove( cName ) class tRPCService
   LOCAL nElem
   LOCAL lRet := .F.

   HB_MutexLock( ::mtxBusy )
   nElem := AScan( ::aFunctions, {|x| cName == x:cName})
   IF nElem != 0
      ADel( ::aFunctions, nElem )
      ASize( ::aFunctions, Len( ::aFunctions ) - 1 )
      lRet := .T.
   ENDIF
   HB_MutexUnlock( ::mtxBusy )
RETURN lRet


METHOD Run( cName, aParams ) class tRPCService
   LOCAL oFunc := ::Find( cName )
   LOCAL oRet := NIL

   HB_MutexLock( ::mtxBusy )
   IF ! Empty( oFunc )
      oRet := oFunc:Run( aParams )
   ENDIF
   HB_MutexUnlock( ::mtxBusy )

RETURN oRet


METHOD Describe( cName ) class tRPCService
   LOCAL oFunc := ::Find( cName )
   LOCAL cRet := NIL

   HB_MutexLock( ::mtxBusy )
   IF ! Empty( oFunc )
      cRet := oFunc:Describe()
   ENDIF
   HB_MutexUnlock( ::mtxBusy )

RETURN cRet


METHOD Start( lStartUdp ) CLASS tRPCService

   IF Empty( ::cBindAddress )
      ::skServer := InetServer( ::nTcpPort )
      ::skUdp := InetDGramBind( ::nUdpPort )
   ELSE
      ::skServer := InetServer( ::nTcpPort, ::cBindAddress )
      ::skUdp := InetDGramBind( ::nUdpPort, ::cBindAddress )
   ENDIF

   ::thAccept := StartThread( Self, "Accept" )

   IF lStartUdp != NIL .and. lStartUdp
      ::thUdp := StartThread( Self, "UdpListen" )
   ELSE
      ::thUdp := NIL
   ENDIF

RETURN .T.


METHOD Stop() CLASS tRPCService
   LOCAL oElem

   HB_MutexLock( ::mtxBusy )
   IF .not. IsValidThread( ::thAccept )
      HB_MutexUnlock( ::mtxBusy )
      RETURN .F.
   ENDIF

   InetClose( ::skServer )
   // closing the socket will make their infinite loops to terminate.
   StopThread( ::thAccept)
   JoinThread( ::thAccept )
   IF IsValidThread( ::thUDP )
      InetClose( ::skUdp )
      StopThread( ::thUdp)
      JoinThread( ::thUdp )
   ENDIF

   FOR EACH oElem IN ::aServing
      IF IsValidThread( oElem:thSelf )
         KillThread( oElem:thSelf )
         JoinThread( oElem:thSelf )
      ENDIF
   NEXT
   ASize( ::aServing, 0 )

   // now destroy all the allocated resources
   ::skServer := NIL
   ::skUdp := NIL

   HB_MutexUnlock( ::mtxBusy )

RETURN .T.


METHOD Accept() CLASS tRPCService
   LOCAL skIn

   DO WHILE .T.
      skIn := InetAccept( ::skServer )
      // todo: better sync
      IF InetStatus( ::skServer ) < 0
         EXIT
      ENDIF
      IF skIn != NIL
         ::StartService( skIn )
      ENDIF
   ENDDO
RETURN .T.


METHOD StartService( skIn ) CLASS tRPCService
   LOCAL oService

   HB_MutexLock( ::mtxBusy )
   oService := tRpcServeCon():New( Self, skIn )
   AAdd( ::aServing, oService )
   oService:Start()
   HB_MutexUnlock( ::mtxBusy )
   ::OnClientConnect( oService )
RETURN .T.

METHOD UDPListen( ) CLASS tRPCService
   LOCAL cData := Space( 1000 )
   LOCAL nPacketLen

   DO WHILE .T.
      nPacketLen := InetDGramRecv( ::skUdp, @cData, 1000 )
      IF InetStatus( ::skUdp ) < 0
         EXIT
      ENDIF
      ::UDPParseRequest( cData, nPacketLen )
   ENDDO
RETURN .T.

METHOD UDPParseRequest( cData, nPacketLen ) CLASS tRPCService
   LOCAL cToSend

   IF ::UDPInterpretRequest( cData, nPacketLen, @cToSend )
       InetDGramSend( ::skUdp, ;
         InetAddress( ::skUdp ), InetPort( ::skUdp ), cToSend )
       RETURN .T.
   ENDIF
RETURN .F.

METHOD UDPInterpretRequest( cData, nPacketLen, cRes ) CLASS tRPCService
   LOCAL cCode, cMatch, cNumber, cSerial
   LOCAL oFunc

   IF nPacketLen < 6
      RETURN .F.
   ENDIF

   cCode := Substr( cData, 1, 6 )

   DO CASE

      /* XHRB00 - server scan */
      CASE cCode == "XHBR00"
         IF .not. ::OnServerScan()
            RETURN .F.
         ENDIF
         IF nPacketLen > 6
            cMatch := HB_Deserialize( Substr( cData, 7 ) )
            IF HB_RegexMatch( cMatch, ::cServerName )
              cRes := "XHBR10"+ HB_Serialize( ::cServerName )
            ENDIF
         ELSE
            cRes := "XHBR10"+ HB_Serialize( ::cServerName )
         ENDIF
         RETURN .T.

      /* XRB01 - Function scan */
      CASE cCode == "XHBR01"
         IF .not. ::OnFunctionScan()
            RETURN .F.
         ENDIF
         /* minimal length to be valid */
         IF nPacketLen > 24
            cSerial := HB_DeserialBegin( Substr( cData, 7 ) )
            cMatch := HB_DeserialNext( @cSerial )
            cNumber := NIL
            IF .not. Empty ( cMatch )
               cMatch := HB_RegexComp( cMatch )
               cNumber := HB_DeserialNext( @cSerial )
            ELSE
               cMatch := HB_RegexComp( ".*" )
            ENDIF

            IF Empty( cNumber )
               cNumber := "00000000.0"
            ENDIF

            FOR EACH oFunc IN ::aFunctions
               IF HB_RegexMatch( cMatch, oFunc:cName ) .and. cNumber <= oFunc:cSerial
                  cRes := "XHBR11" + HB_Serialize(::cServerName ) + ;
                     HB_Serialize( ofunc:Describe())
                  RETURN .T.
               ENDIF
            NEXT
         ENDIF

         /* If we don't have the function, we cannot reply */
         RETURN .F.

   ENDCASE

   /* Ignore malfored requests. */
RETURN .F.


METHOD Terminating( oConnection ) CLASS tRPCService
   LOCAL nToken

   ::OnClientTerminate( oConnection )
   HB_MutexLock( ::mtxBusy )
   nToken := AScan( ::aServing, {|x| x == oConnection } )
   IF nToken > 0
      ADel( ::aServing, nToken )
      ASize( ::aServing, Len( ::aServing ) -1 )
   ENDIF
   HB_MutexUnlock( ::mtxBusy )
RETURN .T.


METHOD AuthorizeChallenge( cUserId, cData ) CLASS tRPCService
   LOCAL cKey, nPos, cMarker := "PASSWORD:"

   cKey := ::GetEncryption( cUserId )
   IF Empty( cKey )
      RETURN NIL
   ENDIF

   cData := HB_Decrypt( cData, cKey )
   nPos := At( cMarker, cData )
   IF nPos == 0
      RETURN NIL
   ENDIF

   cData := Substr( cData, nPos + Len( cMarker ) )
   nPos := At( ":", cData )
   IF nPos == 0
      RETURN NIL
   ENDIF

   cData := Substr( cData, 1, nPos - 1 )

   IF ::Authorize( cUserId, cData ) > 0
      RETURN cKey
   ENDIF
RETURN NIL

/* Default authorization will ALWAYS return 1 if a bAuthorize block is not provided */
/* IF cPassword is NIL, must return the level of the given userid */
METHOD Authorize( cUserid, cPassword ) CLASS tRPCService
   IF ::bAuthorize != NIL
      RETURN Eval( ::bAuthorize, cUserid, cPassword )
   ENDIF
RETURN 1

/* By default, do not provide an encryption key for any user */
METHOD GetEncryption( cUserId ) CLASS tRPCService
   IF ::bGetEncryption != NIL
      RETURN Eval( ::bGetEncryption, cUserId )
   ENDIF
RETURN NIL

METHOD OnFunctionScan() CLASS tRPCService
   IF ::bOnFunctionScan != NIL
      RETURN Eval( ::bOnFunctionScan, Self )
   ENDIF
RETURN .T.

METHOD OnServerScan() CLASS tRPCService
   IF ::bOnServerScan != NIL
      RETURN Eval( ::bOnServerScan, Self )
   ENDIF
RETURN .T.

METHOD OnClientConnect( oClient ) CLASS tRPCService
   IF ::bOnClientConnect != NIL
      RETURN Eval( ::bOnClientConnect, oClient )
   ENDIF
RETURN .T.

METHOD OnClientLogin( oClient ) CLASS tRPCService
   IF ::bOnClientLogin != NIL
      Eval( ::bOnClientLogin, oClient )
   ENDIF
RETURN .T.

METHOD OnClientRequest( oClient, nRequest, cData ) CLASS tRPCService
   IF ::bOnClientRequest != NIL
      RETURN Eval( ::bOnClientRequest, oClient, nRequest, cData )
   ENDIF
RETURN .T.

METHOD OnFunctionProgress( oClient, nProgress, aData ) CLASS tRPCService
   IF ::bOnFunctionProgress != NIL
      RETURN Eval( ::bOnFunctionProgress, oClient, nProgress, aData )
   ENDIF
RETURN .T.

METHOD OnFunctionError( oClient, cFunction, nError ) CLASS tRPCService
   IF ::bOnFunctionError != NIL
      RETURN Eval( ::bOnFunctionError, oClient, cFunction, nError )
   ENDIF
RETURN .T.

METHOD OnFunctionReturn( oClient, aData ) CLASS tRPCService
   IF ::bOnFunctionReturn != NIL
      RETURN Eval( ::bOnFunctionReturn, oClient, aData )
   ENDIF
RETURN .T.

METHOD OnFunctionCanceled( oClient, cFuncName ) CLASS tRPCService
   IF ::bOnFunctionCanceled != NIL
      RETURN Eval( ::bOnFunctionCanceled, oClient, cFuncName )
   ENDIF
RETURN .T.

METHOD OnClientLogout( oClient ) CLASS tRPCService
   IF ::bOnClientLogout != NIL
      RETURN Eval( ::bOnClientLogout, oClient )
   ENDIF
RETURN .T.

METHOD OnClientTerminate( oClient ) CLASS tRPCService
   IF ::bOnClientTerminate != NIL
      RETURN Eval( ::bOnClientTerminate, oClient )
   ENDIF
RETURN .T.

/* #endif */
