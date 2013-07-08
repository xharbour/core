/*
 * $Id: ttable.prg 9935 2013-03-21 08:28:29Z zsaulius $
 */

/*
 * Harbour Project source code:
 * Table,Record and Field Class
 *
 * Copyright 2000-2003 Manos Aspradakis maspr@otenet.gr
 * www - http://www.harbour-project.org
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
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 *
 * Copyright 2000 -2002 Luiz Rafael Culik
 * Methods CreateTable(),Gentable(),AddField()
 * Plus optimization for Xharbour
 *
 */


#include "hbclass.ch"
#include "ttable.ch"
#include "set.ch"
#include "ord.ch"
#include "common.ch"
#include "inkey.ch"
#include "dbinfo.ch"
#include "error.ch"
#define COMPILE(c) &("{||" + c + "}")

//request DBFCDX
   STATIC saTables := {}
   /* NetWork Functions */
   STATIC snNetDelay    := 30
   STATIC slNetOk       := .F.
   STATIC scNetMsgColor := "GR+/R"

FUNCTION NetDbUse( cDataBase, cAlias, nSeconds, cDriver, ;
      lNew, lOpenMode, lReadOnly )

   LOCAL nKey
   LOCAL lForever
   LOCAL cOldScreen := SaveScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1 )
   LOCAL lFirstPass := .T.

   DEFAULT cDriver := "DBFCDX"
   DEFAULT lNew := .T.
   DEFAULT lOpenMode := NET_OPEN_MODE
   DEFAULT lReadOnly := .F.
   DEFAULT nSeconds := snNetDelay

   slNetOk  := .F.
   nSeconds *= 1.00
   lforever := ( nSeconds = 0 )

   KEYBOARD Chr( 255 )
   Inkey()

   DO WHILE ( lforever .OR. nSeconds > 0 ) .AND. LastKey() != K_ESC
      IF !lfirstPass
         DispOutAt( MaxRow(), 0, ;
            PadC( "Network retry � " + ;
            LTrim( Str( nSeconds, 4, 1 ) ) + " � ESCape = Exit ", ;
            MaxCol() + 1 ), ;
            scNetMsgColor )
         lFirstPass := .F.
      ENDIF

      dbUseArea( lNew, ;
         ( cDriver ), ( cDatabase ), ( cAlias ), ;
         lOpenMode, ;
         .F. )

      IF !NetErr()  // USE SUCCEEDS
         RestScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1, cOldScreen )
         slNetOk := .T.
      ELSE
         lFirstPass := .F.
      ENDIF

      IF !slNetOK
         nKey     := Inkey( .5 )        // WAIT 1 SECOND
         nSeconds -= .5
      ELSE
         RestScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1, cOldScreen )
         EXIT
      ENDIF

      IF nKey == K_ESC
         RestScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1, cOldScreen )
         EXIT
      ENDIF

   ENDDO

   RestScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1, cOldScreen )

   RETURN ( slNetOk )

FUNCTION NetLock( nType, lReleaseLocks, nSeconds )

   LOCAL cSave       := SaveScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1 )
   LOCAL lContinue   := .T.
   LOCAL lSuccess    := .F.
   LOCAL nWaitTime
   LOCAL bOperation
   LOCAL xIdentifier
   LOCAL nKey
   LOCAL nCh
   LOCAL cWord

   IF .NOT. ( ValType( nType ) == "N" ) .OR. ;
         ( ( .NOT. ( nType == 1 ) ) .AND. ;
         ( .NOT. ( nType == 2 ) ) .AND. ;
         ( .NOT. ( nType == 3 ) ) )
      Alert( "Invalid Argument passed to NETLOCK()" )
      RETURN ( lSuccess )
   ENDIF

   DEFAULT lReleaseLocks := .F.
   DEFAULT nSeconds := snNetDelay

   nWaitTime := nSeconds

   SWITCH nType
   CASE NET_RECLOCK                        // 1 = Record Lock...
      xIdentifier := IF( lReleaseLocks, NIL, RecNo() )
      bOperation  := { | x | dbRLock( x ) }
      EXIT
   CASE NET_FILELOCK                       // 2 = File Lock...
      bOperation := { || FLock() }
      EXIT
   CASE NET_APPEND                         // 3 = Append Blank...
      xIdentifier := lReleaseLocks
      bOperation  := { | x | dbAppend( x ), !NetErr() }
      EXIT
   END

   slNetOk := .F.

   WHILE lContinue

   /*
   IF (nKey := INKEY()) == K_ESC
      RestScreen( maxrow(),0,maxrow(),maxcol()+1, cSave)
      EXIT
   ENDIF
   */

      WHILE nSeconds > 0 .AND. lContinue == .T.
         IF Eval( bOperation, xIdentifier )
            nSeconds  := 0
            lSuccess  := .T.
            lContinue := .F.
            slNetOK   := .T.
            EXIT
         ELSE
            IF nType == 1
               cWord := "( " + dbInfo( 33 ) + " - Record Lock )"
            ELSEIF nType == 2
               cWord := "( " + dbInfo( 33 ) + " - File Lock )"
            ELSEIF nType == 3
               cWord := "( " + dbInfo( 33 ) + " - File Append )"
            ELSE
               cWord := "( " + dbInfo( 33 ) + " -  ??? "
            ENDIF

            DispOutAt( MaxRow(), 0, ;
               PadC( "Network Retry " + cWord + " � " + Str( nSeconds, 3 ) + " � ESC Exit", MaxCol() + 1 ), ;
               scNetMsgColor )

            nKey := Inkey( 1 )          //TONE( 1,1 )
            nSeconds --                 //.5
            IF nKey == K_ESC
               RestScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1, cSave )
               EXIT
            ENDIF
         ENDIF
      ENDDO

      IF LastKey() == K_ESC
         RestScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1, cSave )
         EXIT
      ENDIF

      IF !lSuccess
         nSeconds := nWaitTime
         nCh      := Alert( RETRY_MSG, { "  YES  ", "  NO  " } )

         IF nCh == 1
            lContinue := .T.
         ELSE
            lContinue := .F.
         ENDIF

         IF lContinue == .F.
            //EXIT
            RestScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1, cSave )
            RETURN ( lSuccess )
         ENDIF

      ENDIF
   ENDDO

   RestScreen( MaxRow(), 0, MaxRow(), MaxCol() + 1, cSave )

   RETURN ( lSuccess )

FUNCTION NetFunc( bBlock, nSeconds )

   LOCAL lForever      // Retry forever?

   DEFAULT nSeconds := snNetDelay
   lForever := ( nSeconds == 0 )

// Keep trying as long as specified or default
   DO WHILE ( lForever .OR. ( nSeconds > 0 ) )

      IF Eval( bBlock )
         RETURN ( .T. )                 // NOTE
      ENDIF

      Inkey( 1 )    // Wait 0.5 seconds
      nSeconds -= 0.5
   ENDDO

   RETURN ( .F. )      // Not locked

// { DBFName, Alias, { idx Names } }
// Returns:   0   All Ok
//           -1   DBF File not found
//           -2   DBF File open Error
//           -3   Index File open Error

FUNCTION NetOpenFiles( aFiles )

   LOCAL nRet := 0
   LOCAL xFile, cIndex

   FOR EACH xFile IN aFiles

      IF !File( xFile[ 1 ] )
         nRet := - 1
         EXIT
      ENDIF

      IF NetDbUse( xFile[ 1 ], xFile[ 2 ], snNetDelay, "DBFCDX" )
         IF ValType( xFile[ 3 ] ) == "A"
            FOR EACH cIndex IN xFile[ 3 ]
               IF File( cIndex )
                  ordListAdd( cIndex )
               ELSE
                  nRet := - 3
                  EXIT
               ENDIF
            NEXT
         ENDIF
      ELSE
         nRet := - 2
         EXIT
      ENDIF
   NEXT

   RETURN nRet

   /* NETWORK METHODS */

FUNCTION NetDelete()

   slNetOK := .F.

   IF NetLock( NET_RECLOCK ) == .T.
      dbDelete()
      slNetOK := .T.
   ENDIF

   IF !NetErr()
      dbSkip( 0 )
      dbCommit()
   ELSE
      slNetOK := .T.
      Alert( " Failed to DELETE Record -> " + Str( RecNo() ) )
   ENDIF

   RETURN ( slNetOk )

FUNCTION NetReCall()

   slNetOk := .F.

   IF NetLock( NET_RECLOCK ) == .T.
      dbRecall()
      slNetOk := .T.
   ENDIF

   IF !NetErr()
      dbSkip( 0 )
      dbCommit()
   ELSE
      slNetOK := .T.
      Alert( " Failed to RECALL Record -> " + Str( RecNo() ) )
   ENDIF

   RETURN ( slNetOk )

FUNCTION NetRecLock( nSeconds )

   DEFAULT nSeconds := snNetDelay

   slNetOK := .F.

   IF NetLock( NET_RECLOCK, , nSeconds )                     // 1
      slNetOK := .T.
   ENDIF

   RETURN ( slNetOK )

FUNCTION NetFileLock( nSeconds )

   slNetOK := .F.
   DEFAULT nSeconds := snNetDelay

   IF NetLock( NET_FILELOCK, , nSeconds )
      slNetOK := .T.
   ENDIF

   RETURN ( slNetOK )

FUNCTION NetAppend( nSeconds, lReleaseLocks )

   LOCAL nOrd

   DEFAULT lReleaseLocks := .T.
   DEFAULT nSeconds := snNetDelay
   slNetOK := .F.
   nOrd    := ordSetFocus( 0 )          // --> set order to 0 to append ???

   IF NetLock( NET_APPEND, , nSeconds )
      //DbGoBottom()
      slNetOK := .T.
   ENDIF

   ordSetFocus( nOrd )

   RETURN ( slNetOK )

PROCEDURE NetFlush()

   dbCommitAll()
   dbUnlockAll()
   dbSkip( 0 )

   RETURN

FUNCTION NetCommitAll()

   LOCAL n

   FOR n := 1 TO MAX_TABLE_AREAS
      IF !Empty( Alias( n ) )
         ( Alias( n ) )->( dbCommit(), dbUnlock() )
      ENDIF
   NEXT

   RETURN n

FUNCTION IsLocked( nRecId )

   DEFAULT nRecID TO RecNo()

   RETURN ( AScan( dbRLockList(), { | n | n == nRecID } ) > 0 )

FUNCTION NetError()

   RETURN !slNetOK

FUNCTION SetNetDelay( nSecs )

   LOCAL nTemp := snNetDelay

   IF nSecs != NIL
      snNetDelay := nSecs
   ENDIF

   RETURN ( nTemp )

FUNCTION SetNetMsgColor( cColor )

   LOCAL cTemp := scNetMsgColor

   IF cColor != NIL
      scNetmsgColor := cColor
   ENDIF

   RETURN ( cTemp )


/****
*     Utility functions
*
*     TableNew()
*
*     getTable()
*/

FUNCTION TableNew( cDBF, cALIAS, cOrderBag, cDRIVER, ;
      lNET, cPATH, lNEW, lREADONLY )

   LOCAL nPos
   LOCAL lAuto
   LOCAL oDB
   LOCAL o

   DEFAULT lNET TO .T.
   DEFAULT lNEW TO .T.
   DEFAULT lREADONLY TO .F.
   DEFAULT cDRIVER TO "DBFCDX"
   DEFAULT cPATH TO SET( _SET_DEFAULT )
   DEFAULT cAlias TO FixExt( cDbf )
   DEFAULT cOrderBag TO FixExt( cDbf )  //+".CDX"

   lAuto := Set( _SET_AUTOPEN, .F. )

   IF ( nPos := AScan( saTables, { | e | e[ 1 ] == Upper( cALIAS ) } ) ) > 0

      oDB := saTables[ nPos, 2 ]

   ELSE
      o := HBTable():New( cDBF, cALIAS, cOrderBag, cDRIVER, ;
         lNET, cPATH, lNEW, lREADONLY )
      IF o:Open()
         oDB := o:FldInit()
      ENDIF

      AAdd( saTables, { Upper( cAlias ), oDB } )

   ENDIF

   SET( _SET_AUTOPEN, lAuto )

   RETURN oDB

FUNCTION GetTable( cAlias )

   LOCAL nPos
   LOCAL oDB

   IF ( nPos := AScan( saTables, { | e | e[ 1 ] == Upper( cALIAS ) } ) ) > 0
      oDB := saTables[ nPos, 2 ]
   ENDIF

   RETURN oDB


/****
*
*     CLASS HBField()
*
*
*
*/

CLASS HBField

   DATA ALIAS INIT Alias()
   DATA Name INIT ""
   DATA TYPE INIT "C"
   DATA Len INIT 0
   DATA Dec INIT 0
   DATA ORDER INIT 0
   DATA Value

   METHOD Get() INLINE ::value := ( ::alias )->( FieldGet( ::order ) )
   METHOD Put( x ) INLINE ::value := x , ;
      ( ::alias )->( FieldPut( ::order, x ) )

ENDCLASS

/****
*
*     CLASS HBRecord()
*
*
*
*/

CLASS HBRecord

   DATA Buffer INIT {}
   DATA ALIAS INIT Alias()
   DATA Number INIT 0
   DATA aFields INIT {}

   METHOD New()
   METHOD Get()
   METHOD Put()

ENDCLASS

METHOD NEW( cAlias ) CLASS HBRecord

   LOCAL i
   LOCAL oFld
   LOCAL aStruc
   LOCAL aItem

   DEFAULT cAlias TO Alias()

   ::Alias   := cAlias
   ::Buffer  := {}
   ::aFields := Array( ( ::alias )->( FCount() ) )

   aStruc := ( ::alias )->( dbStruct() )

   FOR EACH aItem in ::aFields
      i          := HB_EnumIndex()
      oFld       := HBField()
      oFld:order := i
      oFld:Name  := ( ::alias )->( FieldName( i ) )
      oFld:Type  := aStruc[ i, 2 ]
      oFld:LEN   := aStruc[ i, 3 ]
      oFld:Dec   := aStruc[ i, 4 ]
      oFld:Alias := ::alias
      aItem      := oFld
   NEXT

   RETURN Self

PROCEDURE Get() CLASS HBRecord

   LOCAL xField

   FOR EACH xField IN ::aFields
      xField:Get()
      ::buffer[ HB_EnumIndex() ] := xField:value
   NEXT

   RETURN

PROCEDURE Put() CLASS HBRecord

   LOCAL xField

   FOR EACH xField IN ::aFields
      IF xField:Value <> ::buffer[ HB_EnumIndex() ]
         xField:PUT( ::buffer[ HB_EnumIndex() ] )
         ::buffer[ HB_EnumIndex() ] := xField:value
      ENDIF
   NEXT

   RETURN

/****
*
*     CLASS HBTable
*
*
*
*/

//METHOD SetFocus()    INLINE (::Alias)->(Select( ::Area ))
//
//
//encapsulated methods
//
//
//Methods
//
//
//table movement
//
//
//RELATION
//
//
//ORDER Management
//

CLASS HBTable

   DATA Buffer INIT {}                  // 1
   DATA ALIAS INIT Alias()              // 2
   DATA Area INIT 0 // 3

   DATA oRec
   DATA aStruc INIT {}
   DATA nRecno INIT 0
   DATA cDBF INIT ""
   DATA cOrderBag INIT ""
   DATA cOrderFile INIT ""
   DATA cPATH INIT ""
   DATA Driver INIT "DBFCDX"
   DATA IsNew INIT .T.
   DATA IsReadOnly INIT .F.
   DATA IsNet INIT .T.
   DATA aSaveState INIT {}
   DATA lMonitor INIT .F.
   DATA ReadBuffers INIT {}
   DATA WriteBuffers INIT {}
   DATA DeleteBuffers INIT {}
   DATA nDataOffset INIT 0
   DATA BlankBuffer INIT {}
   DATA aOrders INIT {}
   DATA aChildren INIT {}
   DATA oParent

   METHOD EOF() INLINE ( ::Alias )->( EOF() )
   METHOD BOF() INLINE ( ::Alias )->( BOF() )
   METHOD RecNo() INLINE ( ::Alias )->( RecNo() )
   METHOD LastRec() INLINE ( ::Alias )->( LastRec() )
   METHOD SKIP( n ) INLINE ( ::Alias )->( dbSkip( n ) ), ;
      ::nRecno := ( ::Alias )->( RecNo() )

   METHOD GOTO( n ) INLINE ( ::Alias )->( dbGoto( n ) )
   METHOD goTop() INLINE ( ::Alias )->( dbGoTop() )
   METHOD goBottom() INLINE ( ::Alias )->( dbGoBottom() )
   METHOD SetFocus() INLINE ( ::Alias )->( Select( ::ALias ) )
   METHOD APPEND( l ) INLINE IF( ::isNet, ( ::Alias )->( NetAppend( l ) ), ;
      ( ::alias )->( dbAppend() ) )
   METHOD RECALL(  ) INLINE ( ::Alias )->( NetRecall(  ) )

   METHOD LOCATE( bFor, bWhile, nNext, nRec, lRest ) INLINE ;
      ( ::Alias )->( __dbLocate( bFor, bWhile, ;
      nNext, nRec, lRest ) )
   METHOD CONTINUE() INLINE ( ::Alias )->( __dbContinue() )
   METHOD Found() INLINE ( ::Alias )->( Found() )
   METHOD Kill() INLINE ( ::Alias )->( dbCommit() ), ;
      ( ::Alias )->( dbUnlock() ) , ;
      ( ::Alias )->( dbCloseArea() ), ;
      ::ClearBuffers()
   METHOD ClearBuffers() INLINE ::ReadBuffers := {}, ;
      ::WriteBuffers := {}, ;
      ::DeleteBuffers := {}

   METHOD dbIsShared() INLINE ( ::Alias )->( dbInfo( DBI_SHARED ) )

   METHOD dbIsFLocked(  ) INLINE ( ::Alias )->( dbInfo( DBI_ISFLOCK ) )

   METHOD dbLockCount() INLINE ( ::Alias )->( dbInfo( DBI_LOCKCOUNT ) )

   METHOD dbInfo( n, x ) INLINE ( ::Alias )->( dbInfo( n, x ) )

   METHOD dbGetAlias() INLINE ( ::Alias )
                                                
   METHOD dbFullPath() INLINE ( ::Alias )->( dbInfo( DBI_FULLPATH ) )

   METHOD IsRLocked( n ) INLINE ( ::Alias )->( dbRecordInfo( DBRI_LOCKED, n ) )

   METHOD IsRUpdated( n ) INLINE ( ::Alias )->( dbRecordInfo( DBRI_UPDATED, n ) )

   METHOD dbRecordInfo( n, x ) INLINE ( ::Alias )->( dbRecordInfo( n,, x ) )

   METHOD dbOrderInfo( n, x, u ) INLINE ( ::Alias )->( dbOrderInfo( n, ::cOrderFile, x, u ) )

   METHOD OrderCount() INLINE ;
      ( ::Alias )->( dbOrderInfo( DBOI_ORDERCOUNT, ::cOrderFile ) )

   METHOD AutoOpen( l ) INLINE ;
      ( ::Alias )->( dbOrderInfo( DBOI_AUTOOPEN, ::cOrderFile,, l ) )

   METHOD AutoShare( l ) INLINE ;
      ( ::Alias )->( dbOrderInfo( DBOI_AUTOSHARE, ::cOrderFile,, l ) )

   METHOD Used() INLINE Select( ::Alias ) > 0

   METHOD ordSetFocus( ncTag ) INLINE ( ::Alias )->( ordSetFocus( ncTag ) )
   METHOD ordName( nOrder ) INLINE ;
      ( ::Alias )->( ordName( nOrder, ::cOrderBag ) ) ;

      METHOD ordNumber( cOrder ) INLINE ;
      ( ::Alias )->( ordNumber( cOrder, ::cOrderBag ) ) ;

      METHOD ordScope( n, u ) INLINE ( ::Alias )->( ordScope( n, u ) )

   METHOD ordIsUnique( nc ) INLINE ( ::Alias )->( ordIsUnique( nc, ;
      ::cOrderBag ) ) ;

      METHOD ordSkipUnique( n ) INLINE ( ::Alias )->( ordSkipUnique( n ) )
   METHOD ordSetRelation( n, b, c ) INLINE ( ::Alias )->( ordSetRelation( n, b, c ) )

   METHOD SetTopScope( xScope ) INLINE ;
      ( ::alias )->( ordScope( TOPSCOPE, xScope ) )
   METHOD SetBottomScope( xScope ) INLINE ;
      ( ::alias )->( ordScope( BOTTOMSCOPE, xScope ) )
   METHOD KillScope() INLINE ( ::alias )->( ordScope( TOPSCOPE, NIL ) )  , ;
      ( ::alias )->( ordScope( BOTTOMSCOPE, NIL ) )

   METHOD New( cDBF, cALIAS, cOrderBag, cDRIVER, ;
      lNET, cPATH, lNEW, lREADONLY )

   METHOD OPEN()

   METHOD dbMove( n )
   METHOD FldInit()
   METHOD READ( l )
   METHOD ReadBLANK( l )
   METHOD Write( l )
   METHOD BufWrite( l )
   MESSAGE DELETE() METHOD __oTDelete() // reserved word - *HAS* to be renamed...
   METHOD SetMonitor( l )
   METHOD Undo( a, b, c )

   METHOD dbSkip( n ) INLINE ( ::Alias )->( dbSkip( n ) ), ;
      ::nRecno := ( ::alias )->( RecNo() )

   METHOD dbGoto( n ) INLINE ( ::Alias )->( dbGoto( n ) )

   METHOD dbEval( a, b, c, d, e, f ) INLINE ( ::Alias )->( dbEval( a, b, c, d, e, f ) )
   METHOD dbSeek( a, b, c ) INLINE ( ::Alias )->( dbSeek( a, b, c ) )




   METHOD dbFilter() INLINE ( ::Alias )->( dbFilter() )
   METHOD SetFilter( c ) INLINE ;
      IF( c != NIL, ( ::Alias )->( dbSetFilter( COMPILE( c ), c ) ), ;
      ( ::Alias )->( dbClearFilter() ) )

   METHOD AddChild( oChild, cKey )

   METHOD AddOrder( cTag, cKey, cLabel, ;
      cFor, cWhile, ;
      lUnique, ;
      bEval, nInterval, cOrderFile )
   METHOD GetOrderLabels()
   METHOD SetOrder( xTag )
   METHOD GetOrder( xOrder )
   METHOD FastReindex()
   METHOD REINDEX()
   METHOD CreateTable( cFile )
   METHOD AddField( f, t, l, d )
   METHOD Gentable()

   ERROR HANDLER OnError()

ENDCLASS

//---------------------
//  Constructor...
//---------------------

METHOD New( cDBF, cALIAS, cOrderBag, cDRIVER, ;
      lNET, cPATH, lNEW, lREADONLY ) CLASS HBTable

   LOCAL cOldRdd

   DEFAULT lNET TO .F.
   DEFAULT lNEW TO .T.
   DEFAULT lREADONLY TO .F.
   DEFAULT cDRIVER TO "DBFCDX"
   DEFAULT cPATH TO SET( _SET_DEFAULT )
   DEFAULT cAlias TO FixExt( cDbf )
   DEFAULT cOrderBag TO FixExt( cDbf )  //+".CDX"
   

   ::IsNew      := lNEW
   ::IsNet      := lNET
   ::IsReadOnly := lREADONLY
   ::cDBF       := cDBF
   ::cPath      := cPATH
   ::cOrderBag  := FixExt( cOrderBag )
   cOldRdd      := rddSetDefault( ::driver )

   ::cOrderFile := ::cOrderBag + ordBagExt()                //".CDX"
   rddSetDefault( cOldRdd )
   ::Driver      := cDRIVER
   ::aOrders     := {}
   ::Area        := 0
   ::Alias       := cALIAS
   ::nDataOffset := Len( self )         //66

   RETURN Self

METHOD OPEN() CLASS HBTable

   LOCAL lSuccess := .T.

   dbUseArea( ::IsNew, ::Driver, ::cDBF, ::Alias, ::IsNET, ::IsREADONLY )

   IF ::IsNET == .T.
      IF NetErr()
         Alert( _NET_USE_FAIL_MSG )
         lSuccess := .F.
         RETURN ( lSuccess )
      ENDIF
   ENDIF

   SELECT( ::Alias )
   ::Area := Select()
   IF ::cOrderBag != NIL .AND. File( ::cPath + ::cOrderFile )

      SET INDEX TO ( ::cPath + ::cOrderBag )
      ( ::Alias )->( ordSetFocus( 1 ) )

   ENDIF

   ::Buffer := Array( ( ::Alias )->( FCount() ) )
   ::aStruc := ( ::Alias )->( dbStruct() )

   ::dbMove( _DB_TOP )

   RETURN ( lSuccess )

PROCEDURE DBMove( nDirection ) CLASS HBTable

   //LOCAL nRec := ( ::Alias )->( RecNo() )

   DEFAULT nDirection TO 0

   DO CASE
   CASE nDirection == 0
      ( ::Alias )->( dbSkip( 0 ) )
   CASE nDirection == _DB_TOP
      ( ::Alias )->( dbGoTop() )
   CASE nDirection == _DB_BOTTOM
      ( ::Alias )->( dbGoBottom() )
   CASE nDirection == _DB_BOF
      ( ::Alias )->( dbGoTop() )
      ( ::Alias )->( dbSkip( - 1 ) )
   CASE nDirection == _DB_EOF
      ( ::Alias )->( dbGoBottom() )
      ( ::Alias )->( dbSkip( 1 ) )
   OTHERWISE
      ( ::Alias )->( dbGoto( nDirection ) )
   ENDCASE

   RETURN

// -->
// -->
// --> Insert field definitions and generate virtual child class...
// -->
// -->

METHOD FldInit() CLASS HBTable

   LOCAL i


   //LOCAL nCount := ( ::Alias )->( FCount() )
   LOCAL aDb


   LOCAL oNew

   LOCAL nScope    := 1

   ::nDataOffset := Len( self ) - 1

   ::Buffer := Array( ( ::Alias )->( FCount() ) )
   IF Empty( ::Buffer )
      ::Read()
   ENDIF

// --> create new oObject class from this one...

   adb := HBClass():new( ::alias, { "hbtable" } )

   FOR i := 1 TO FCount()
      adb:AddData( ( ::Alias )->( FieldName( i ) ), , , nScope )
   NEXT

   aDB:create()

   oNew := adb:Instance()

   oNew:IsNew       := ::IsNew
   oNew:IsNet       := ::IsNet
   oNew:IsReadOnly  := ::IsReadOnly
   oNew:cDBF        := ::cDBF
   oNew:cPath       := ::cPath
   oNew:cOrderBag   := ::cOrderBag
   oNew:cOrderFile  := ::cOrderFile
   oNew:Driver      := ::Driver
   oNew:Area        := ::Area
   oNew:Alias       := ::Alias
   oNew:aStruc      := ::aStruc
   oNew:BlankBuffer := ::BlankBuffer
   oNew:aOrders     := ::aOrders
   oNew:oParent     := ::oParent
   oNew:Buffer      := ::buffer

   SELECT( oNew:Alias )

   oNew:Area := Select()

   oNew:Read()

   IF oNew:cOrderBag != NIL .AND. File( oNew:cPath + oNew:cOrderFile )
      SET INDEX TO ( oNew:cPath + oNew:cOrderBag )
      ( oNew:Alias )->( ordSetFocus( 1 ) )
   ENDIF

   oNew:buffer := Array( ( oNew:alias )->( FCount() ) )
   oNew:aStruc := ( oNew:alias )->( dbStruct() )

   IF oNew:Used()
      oNew:dbMove( _DB_TOP )
      oNew:Read()
   ENDIF

   RETURN oNew

PROCEDURE READ( lKeepBuffer ) CLASS HBTable

   LOCAL i
   LOCAL nSel   := Select( ::Alias )
   LOCAL adata  := Array( 1, 2 )
   LOCAL Buffer

   DEFAULT lKeepBuffer TO .F.

//? len( ::Buffer )

   FOR EACH Buffer in ::Buffer
      
      i      := HB_EnumIndex()
      Buffer := ( ::Alias )->( FieldGet( i ) )

      adata[ 1, 1 ] := ( ::Alias )->( FieldName( i ) )
      adata[ 1, 2 ] := ( ::Alias )->( FieldGet( i ) )
      __objSetValueList( Self, aData )

   NEXT

   IF ( lKeepBuffer == .T. ) .OR. ( ::lMonitor == .T. )
      AAdd( ::ReadBuffers, { ( ::Alias )->( RecNo() ), ::Buffer } )
   ENDIF

   SELECT( nSel )

   RETURN

PROCEDURE ReadBlank( lKeepBuffer ) CLASS HBTable

   LOCAL i
   LOCAL nSel   := Select( ::Alias )
   LOCAL nRec   := ( ::Alias )->( RecNo() )
   LOCAL adata  := Array( 1, 2 )
   LOCAL Buffer

   DEFAULT lKeepBuffer TO .F.

   ( ::Alias )->( dbGoBottom() )
   ( ::Alias )->( dbSkip( 1 ) )         // go EOF

   FOR EACH Buffer in ::Buffer
      i      := HB_EnumIndex()
      Buffer := ( ::Alias )->( FieldGet( i ) )

      adata[ 1, 1 ] := ( ::Alias )->( FieldName( i ) )
      adata[ 1, 2 ] := ( ::Alias )->( FieldGet( i ) )
      __objSetValueList( Self, aData )

   NEXT

   IF ( lKeepBuffer == .T. ) .OR. ( ::lMonitor == .T. )
      AAdd( ::ReadBuffers, { ( ::Alias )->( RecNo() ), ::Buffer } )
   ENDIF

   ( ::Alias )->( dbGoto( nRec ) )
   SELECT( nSel )

   RETURN

METHOD Write( lKeepBuffer ) CLASS HBTable

   LOCAL i
   LOCAL aOldBuffer := Array( ( ::Alias )->( FCount() ) )
   LOCAL nSel       := Select( ::Alias )
   LOCAL nOrd       := ( ::Alias )->( ordSetFocus() )
   LOCAL aData      := __objGetValueLIST( Self )
   LOCAL xBuffer
   LOCAL n

   DEFAULT lKeepBuffer TO .F.

   IF ( lKeepBuffer == .T. ) .OR. ( ::lMonitor == .T. )

      // --> save old record in temp buffer
      FOR EACH xBuffer IN aOldBuffer
         xBuffer := ( ::Alias )->( FieldGet( HB_EnumIndex() ) )
      NEXT

      AAdd( ::WriteBuffers, { ( ::Alias )->( RecNo() ), aOldBuffer } )

   ENDIF

   IF ::isNet
      IF !( ::Alias )->( NetRecLock() )
         RETURN .F.
      ENDIF
   ENDIF

   ( ::Alias )->( ordSetFocus( 0 ) )

   FOR i := 1 TO ( ::Alias )->( FCount() )
      n := AScan( adata, { | a | a[ 1 ] == ( ::Alias )->( FieldName( i ) ) } )
      ( ::Alias )->( FieldPut( i, adata[ n, 2 ] ) )
   NEXT

   ( ::Alias )->( dbSkip( 0 ) )         // same as commit
   IF ::isNet
      ( ::Alias )->( dbRUnlock() )
   ENDIF
   ( ::Alias )->( ordSetFocus( nOrd ) )
   SELECT( nSel )

   RETURN ( .T. )

METHOD BUFWrite( aBuffer ) CLASS HBTable

   //LOCAL aOldBuffer := Array( ( ::Alias )->( FCount() ) )
   LOCAL nSel       := Select( ::Alias )
   LOCAL nOrd       := ( ::Alias )->( ordSetFocus() )
   LOCAL Buffer

   DEFAULT aBuffer TO ::Buffer

   IF ::isNet
      IF !( ::Alias )->( NetRecLock() )
         RETURN .F.
      ENDIF
   ENDIF

   ( ::Alias )->( ordSetFocus( 0 ) )

   FOR EACH Buffer in aBuffer
      ( ::Alias )->( FieldPut( HB_EnumIndex(), Buffer ) )
   NEXT

   ( ::Alias )->( dbSkip( 0 ) )
   IF ::isNet
      ( ::Alias )->( dbRUnlock() )
   ENDIF
   ( ::Alias )->( ordSetFocus( nOrd ) )
   SELECT( nSel )

   RETURN ( .T. )

METHOD __oTDelete( lKeepBuffer )        // ::Delete()

   LOCAL lRet
   LOCAL lDeleted := Set( _SET_DELETED, .F. )                  // make deleted records visible

// temporarily...
   DEFAULT lKeepBuffer TO .F.

   ::Read()

   IF ::isNet
      lRet := IF( ( ::Alias )->( NetDelete() ), .T. , .F. )
   ELSE
      ( ::alias )->( dbDelete() ) ; lRet := .T.
   ENDIF

   IF ( ( lKeepBuffer == .T. ) .OR. ( ::lMonitor == .T. ) ) .AND. ;
         ( lRet == .T. )
      AAdd( ::DeleteBuffers, { ( ::Alias )->( RecNo() ), ::Buffer } )
   ENDIF

   IF ::isNet
      ( ::Alias )->( dbUnlock() )
   ENDIF

   SET( _SET_DELETED, lDeleted )

   RETURN ( lRet )

METHOD SetMonitor( l ) CLASS HBTable

   LOCAL lTemp := ::lMonitor

   ::lMonitor := !(  l )

   RETURN lTemp

//
//   Transaction control subsystem...
//

METHOD Undo( nBuffer, nLevel ) CLASS HBTable

   LOCAL nLen
   LOCAL lRet      := .F.
   LOCAL lDelState := Set( _SET_DELETED )
   LOCAL nRec      := ::RecNo()
   LOCAL aBuffers

   DEFAULT nBuffer TO _WRITE_BUFFER

   IF nLevel == NIL
      nLevel := 0
   ENDIF

   SWITCH nBuffer

   CASE _DELETE_BUFFER

      IF !Empty( ::DeleteBuffers )

         SET( _SET_DELETED, .F. )       // make deleted records visible temporarily...

         nLen := Len( ::deleteBuffers )

         DEFAULT nLevel TO nLen

         IF nLevel == 0 .OR. nLevel == nLen     // DO ALL...
            FOR EACH aBuffers IN ::deleteBuffers

               ( ::Alias )->( dbGoto( aBuffers[ 1 ] ) )

               IF ( ::Alias )->( NetRecall() )
                  lRet := .T.
               ELSE
                  lRet := .F.
               ENDIF

            NEXT

            IF lRet
               ::deleteBuffers := {}
            ENDIF

         ELSE       // DO CONTROLLED...

            FOR EACH aBuffers IN ::deleteBuffers
               IF HB_EnumIndex() > ( nLen - nLevel )

                  ( ::Alias )->( dbGoto( aBuffers[ 1 ] ) )

                  IF ( ::Alias )->( NetRecall() )
                     lRet := .T.
                  ELSE
                     lRet := .F.
                  ENDIF
               ENDIF
            NEXT

            IF lRet
               ASize( ::deleteBuffers, ( nLen - nLevel ) )
            ENDIF

         ENDIF

         SET( _SET_DELETED, lDelState )

      ENDIF

   CASE _WRITE_BUFFER
      IF !Empty( ::WriteBuffers )

         nLen := Len( ::WriteBuffers )
         DEFAULT nLevel TO nLen

         IF nLevel == 0 .OR. nLen == nLevel   // Do All...

            FOR EACH aBuffers IN ::writeBuffers

               ( ::Alias )->( dbGoto( aBuffers[ 1 ] ) )

               IF ::BufWrite( aBuffers[ 2 ] )
                  lRet := .T.
               ELSE
                  Alert( "Rollback Failed..." )
                  lRet := .F.
               ENDIF
            NEXT

            IF lRet
               // erase entries
               ::WriteBuffers := {}
            ENDIF

         ELSE       // do controlled...

            FOR EACH aBuffers IN ::writeBuffers
               IF HB_EnumIndex() > ( nLen - nLevel )

                  ( ::Alias )->( dbGoto( aBuffers[ 1 ] ) )

                  IF ::BufWrite( aBuffers[ 2 ] )
                     lRet := .T.
                  ELSE
                     Alert( "Rollback Failed..." )
                     lRet := .F.
                  ENDIF
               ENDIF
            NEXT

            // erase entries
            IF lRet == .T.
               ASize( ::WriteBuffers, ( nLen - nLevel ) )
            ENDIF

         ENDIF

      ENDIF

      DEFAULT

   END

   ( ::Alias )->( dbUnlock() )
   ( ::Alias )->( dbGoto( nRec ) )
   ::Read()

   RETURN ( lRet )

//
//   ORDER MANAGEMENT
//

METHOD AddOrder( cTag, cKey, cLabel, ;
      cFor, cWhile, ;
      lUnique, ;
      bEval, nInterval, cOrderFile ) CLASS HBTable

   LOCAL oOrd

   DEFAULT cOrderFile TO ::cOrderBag

   oOrd := HBOrder():New( cTag, cKey, cLabel, ;
      cFor, cWhile, ;
      lUnique, ;
      bEval, nInterval )

   oOrd:oTable    := Self
   oOrd:cOrderBag := ::cOrderBag

   AAdd( ::aOrders, oOrd )

   RETURN oOrd

METHOD REINDEX() CLASS HBTable

   LOCAL nSel := Select( ::Alias )
   LOCAL nOrd := ( ::Alias )->( ordSetFocus( 0 ) )

   IF Len( ::aOrders ) > 0

      IF ::Used()
         ::Kill()
      ENDIF

      ::Isnet := .F.

      IF File( ::cPath + ::cOrderFile )
         IF FErase( ::cPath + ::cOrderFile ) != 0
            // --> ALERT(".CDX *NOT* Deleted !!!" )
         ENDIF
      ENDIF

      IF !::Open()
         RETURN .F.
      ENDIF

      AEval( ::aOrders, { | o | o:Create() } )

      ::Kill()
      ::IsNet := .T.

      IF !::Open()
         RETURN .F.
      ENDIF

   ENDIF

   ( ::Alias )->( dbSetIndex( ::cOrderBag ) )
   ( ::Alias )->( ordSetFocus( nOrd ) )
   ( ::Alias )->( dbGoTop() )
   ( ::Alias )->( dbUnlock() )
   SELECT( nSel )

   RETURN .T.

METHOD FastReindex() CLASS HBTable

   LOCAL nSel := Select( ::Alias )
   LOCAL nOrd := ( ::Alias )->( ordSetFocus( 0 ) )
   //LOCAL nRec := ( ::Alias )->( RecNo() )

   IF Len( ::aOrders ) > 0

      ::Kill()

      ::Isnet := .F.
      IF File( ::cPath + ::cOrderFile )
         IF FErase( ::cPath + ::cOrderFile ) != 0
            // --> ALERT(".CDX *NOT* Deleted !!!" )
         ENDIF
      ENDIF

      IF !::Open()
         RETURN .F.
      ENDIF

      ( ::Alias )->( ordListRebuild() )

      ::Kill()
      ::IsNet := .T.

      IF !::Open()
         RETURN .F.
      ENDIF

   ENDIF

   ( ::Alias )->( dbSetIndex( ::cOrderBag ) )
   ( ::Alias )->( ordSetFocus( nOrd ) )
   ( ::Alias )->( dbGoTop() )
   ( ::Alias )->( dbUnlock() )
   SELECT( nSel )

   RETURN .T.

METHOD GetOrder( xOrder ) CLASS HBTable

   LOCAL nPos  := 0
   LOCAL xType := ValType( xOrder )

   IF xType == "C"
      nPos := AScan( ::aOrders, { | e | e:Tag == xOrder } )
   ELSEIF xType == "N" .AND. xOrder > 0
      nPos := xOrder
   ENDIF

   IF nPos == 0
      nPos := 1
   ENDIF

   RETURN ::aOrders[ nPos ]                // returns oOrder

METHOD SetOrder( xTag ) CLASS HBTable

   LOCAL xType   := ValType( xTag )
   LOCAL nOldOrd := ( ::Alias )->( ordSetFocus() )

   SWITCH xType
   CASE "C"                    // we have an Order-TAG
      ( ::Alias )->( ordSetFocus( xTag ) )
      EXIT
   CASE "N"                    // we have an Order-Number
      IF xTag <= 0
         ( ::Alias )->( ordSetFocus( 0 ) )
      ELSE
         ::Getorder( xTag ):SetFocus()
      ENDIF
      EXIT
   CASE "O"                    // we have an Order-Object
      xTag:SetFocus()
      EXIT
      DEFAULT
      ( ::Alias )->( ordSetFocus( 0 ) )
   END

   RETURN nOldOrd

METHOD GetOrderLabels() CLASS HBTable

   LOCAL aRet := {}

   IF !Empty( ::aOrders )
      AEval( ::aOrders, { | e | AAdd( aRet, e:Label ) } )
   ENDIF

   RETURN aRet

//
// Relation Methods
//

PROCEDURE AddChild( oChild, cKey ) CLASS HBTable                 // ::addChild()

   AAdd( ::aChildren, { oChild, cKey } )
   oChild:oParent := Self
   ( ::Alias )->( ordSetRelation( oChild:Alias, COMPILE( cKey ), cKey ) )

   RETURN

/****
*     FixExt( cFileName )
*     extract .CDX filename from .DBF filename
*/

STATIC FUNCTION FixExt( cFileName )

   LOCAL nLeft := At( ".", cFilename )

   RETURN ( Left( cFileName, IF( nLeft == 0, ;
      Len( cFilename ), ;
      nLeft - 1 ) ) )

METHOD CreateTable( cFile ) CLASS HBTable

   ::cDbf := cFile
   IF Len( ::aStruc ) > 0
      ::aStruc  := {}
      ::aOrders := {}
   ENDIF

   RETURN Self

PROCEDURE AddField( f, t, l, d ) CLASS HBTable

   AAdd( ::aStruc, { f, t, l, d } )

   RETURN

PROCEDURE Gentable() CLASS HBTable

   dbCreate( ::cDbf, ::aStruc, ::Driver )

   RETURN

METHOD OnError( uParam ) CLASS HBTable

   LOCAL cMsg := __GetMessage()
   LOCAL nPos
   LOCAL uRet, oErr

   IF uParam <> nil .AND. Left( cMsg, 1 ) == '_'
      cMsg := SubStr( cMsg, 2 )
   ENDIF
   nPos := ( ::Alias )->( FieldPos( cMsg ) )

   IF nPos <> 0
      uRet := ( ::Alias )->( if( uParam == nil, FieldGet(nPos ), FieldPut(nPos, uParam ) ) )
   ELSE

      oErr := ErrorNew()
      oErr:Args          := { Self, cMsg, uParam }
      oErr:CanDefault    := .F.
      oErr:CanRetry      := .F.
      oErr:CanSubstitute := .T.
      oErr:Description   := "Invalid class member"
      oErr:GenCode       := EG_NOVARMETHOD
      oErr:Operation     := "HBTable:" + cMsg
      oErr:Severity      := ES_ERROR
      oErr:SubCode       := - 1
      oErr:SubSystem     := "HBTable"
      uRet := Eval( ErrorBlock(), oErr )

   ENDIF

   RETURN uRet

CLASS HBOrder

   DATA oTable
   DATA cOrderBag
   DATA Label, TAG
   DATA cKey, bKey
   DATA cFor, bFor
   DATA cWhile, bWhile
   DATA UNIQUE INIT .F.
   DATA bEval
   DATA nInterval
   METHOD Alias() INLINE ::oTable:Alias

   METHOD New( cTag, cKey, cLabel, cFor, cWhile, lUnique, bEval, nInterval, cOrderBag )
   METHOD CREATE()

   METHOD SetFocus() INLINE ( ::alias )->( ordSetFocus( ::Tag, ::cOrderBag ) )
   METHOD Destroy() INLINE ( ::alias )->( ordDestroy( ::Tag, ::cOrderBag ) )
   METHOD ordDestroy() INLINE ( ::alias )->( ordDestroy( ::Tag, ::cOrderBag ) )
   METHOD ordBagExt() INLINE ( ::alias )->( ordBagExt() )
   METHOD ordFor() INLINE ( ::alias )->( ordFor( ::Tag, ::cOrderBag ) )
   METHOD ordIsUnique() INLINE ( ::alias )->( ordIsUnique( ::Tag, ::cOrderBag ) )
   METHOD ordKey() INLINE ( ::alias )->( ordKey( ::Tag, ::cOrderBag ) )
   METHOD ordKeyCount() INLINE ( ::alias )->( ordKeyCount( ::Tag, ::cOrderBag ) )
   METHOD ordKeyNo() INLINE ( ::alias )->( ordKeyNo( ::Tag, ::cOrderBag ) )
   METHOD ordKeyVal() INLINE ( ::alias )->( ordKeyVal( ::Tag, ::cOrderBag ) )

ENDCLASS

METHOD New( cTag, cKey, cLabel, cFor, cWhile, lUnique, bEval, nInterval, cOrderBag ) CLASS HBOrder

   DEFAULT cKey TO ".T."
   DEFAULT lUnique TO .F.
   DEFAULT cFor TO ".T."
   DEFAULT cWhile TO ".T."
   DEFAULT bEval TO { || .T. }
   DEFAULT nInterval TO 1
   DEFAULT cLabel TO cTag
   ::cOrderBag := cOrderBag
   ::Tag       := cTag
   ::cKey      := cKey
   ::cFor      := cFor
   ::cWhile    := cWhile
   ::bKey      := COMPILE( cKey )
   ::bFor      := COMPILE( cFor )
   ::bWhile    := COMPILE( cWhile )
   ::bEval     := bEval
   ::nInterval := nInterval
   ::Label     := cLabel

   RETURN Self

PROCEDURE CREATE() CLASS HBOrder

   DEFAULT ::cOrderBag TO ::oTable:cOrderBag
//? "<<<",::alias, ::cOrderBag
   ( ::alias )->( ordCondSet( ::cFor, ::bFor, ;
      .T. , ;
      ::bWhile, ;
      ::bEval, ::nInterval ) )

   ( ::alias )->( ordCreate( ::cOrderBag, ::Tag, ::cKey, ;
      ::bKey, ::Unique ) )

   RETURN


