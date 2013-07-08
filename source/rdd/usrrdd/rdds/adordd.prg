/*
 * $Id: adordd.prg 9941 2013-03-25 17:42:55Z enricomaria $
 */

/*
 * Harbour Project source code:
 * ADORDD - RDD to automatically manage Microsoft ADO
 *
 * Copyright 2007 Fernando Mancera <fmancera@viaopen.com> and
 * Antonio Linares <alinares@fivetechsoft.com>
 * www - http://www.harbour-project.org
 *
 * Copyright 2007-2008 Miguel Angel Marchuet <miguelangel@marchuet.net>
 *  ADO_GOTOID( nWA, nRecord )
 *  ADO_GOTO( nWA, nRecord )
 *  ADO_OPEN( nWA, aOpenInfo ) some modifications
 *     Open: Excel files
 *           Paradox files
 *           Access with password
 *           FireBird
 *  ADO_CLOSE( nWA )
 *  ADO_ZAP( nWA )
 *  ADO_ORDINFO( nWA, nIndex, aOrderInfo ) some modifications
 *  ADO_RECINFO( nWA, nRecord, nInfoType, uInfo )
 *  ADO_FIELDINFO( nWA, nField, nInfoType, uInfo )
 *  ADO_FIELDNAME( nWA, nField )
 *  ADO_FORCEREL( nWA )
 *  ADO_RELEVAL( nWA, aRelInfo )
 *  ADO_EXISTS( nRdd, cTable, cIndex, ulConnect )
 *  ADO_DROP(  nRdd, cTable, cIndex, ulConnect ) 
 *  ADO_LOCATE( nWA, lContinue )
 *
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

#include "rddsys.ch"
#include "fileio.ch"
#include "error.ch"
#include "adordd.ch"
#include "common.ch"
#include "dbstruct.ch"
#include "dbinfo.ch"

#ifndef __XHARBOUR__
   #include "hbusrrdd.ch"
   #xcommand TRY              => bError := errorBlock( {|oErr| break( oErr ) } ) ;;
                                 BEGIN SEQUENCE
   #xcommand CATCH [<!oErr!>] => errorBlock( bError ) ;;
                                 RECOVER [USING <oErr>] <-oErr-> ;;
                                 errorBlock( bError )
   #command FINALLY           => ALWAYS
#else
   #include "usrrdd.ch"
#endif

#define WA_RECORDSET   1
#define WA_BOF         2
#define WA_EOF         3
#define WA_CONNECTION  4
#define WA_CATALOG     5
#define WA_TABLENAME   6
#define WA_ENGINE      7
#define WA_SERVER      8
#define WA_USERNAME    9
#define WA_PASSWORD   10
#define WA_QUERY      11
#define WA_LOCATEFOR  12
#define WA_SCOPEINFO  13
#define WA_SQLSTRUCT  14
#define WA_CONNOPEN   15
#define WA_PENDINGREL 16
#define WA_FOUND      17

#define WA_SIZE       17

#define RDD_CONNECTION 1
#define RDD_CATALOG    2

#define RDD_SIZE       2

ANNOUNCE ADORDD

static s_cTableName, s_cEngine, s_cServer, s_cUserName, s_cPassword, s_cQuery := ""

#ifndef __XHARBOUR__
static bError
#endif

#ifdef __XHARBOUR__

static function HB_TokenGet( cText, nPos, cSep )

   local aTokens := HB_ATokens( cText, cSep )

return If( nPos <= Len( aTokens ), aTokens[ nPos ], "" )

#endif

STATIC FUNCTION ADO_INIT( nRDD )

   LOCAL aRData := Array( RDD_SIZE )

   USRRDD_RDDDATA( nRDD, aRData )

RETURN SUCCESS

STATIC FUNCTION ADO_NEW( nWA )

   LOCAL aWAData := Array( WA_SIZE )

   aWAData[ WA_BOF ] := .F.
   aWAData[ WA_EOF ] := .F.

   USRRDD_AREADATA( nWA, aWAData )

RETURN SUCCESS

STATIC FUNCTION ADO_CREATE( nWA, aOpenInfo )

   LOCAL cDataBase   := HB_TokenGet( aOpenInfo[ UR_OI_NAME ], 1, ";" )
   LOCAL cTableName  := HB_TokenGet( aOpenInfo[ UR_OI_NAME ], 2, ";" )
   LOCAL cDbEngine   := HB_TokenGet( aOpenInfo[ UR_OI_NAME ], 3, ";" )
   LOCAL cServer     := HB_TokenGet( aOpenInfo[ UR_OI_NAME ], 4, ";" )
   LOCAL cUserName   := HB_TokenGet( aOpenInfo[ UR_OI_NAME ], 5, ";" )
   LOCAL cPassword   := HB_TokenGet( aOpenInfo[ UR_OI_NAME ], 6, ";" )
   LOCAL oConnection := TOleAuto():New( "ADODB.Connection" )
   LOCAL oCatalog    := TOleAuto():New( "ADOX.Catalog" )
   LOCAL aWAData     := USRRDD_AREADATA( nWA )
   LOCAL oError, n

   DO CASE
      CASE Lower( Right( cDataBase, 4 ) ) == ".mdb"
           IF ! File( cDataBase )
              oCatalog:Create( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + cDataBase )
           ENDIF
           oConnection:Open( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + cDataBase )

      CASE Lower( Right( cDataBase, 4 ) ) == ".xls"
           IF ! File( cDataBase )
              oCatalog:Create( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + cDataBase + ";Extended Properties='Excel 8.0;HDR=YES';Persist Security Info=False" )
           ENDIF
           oConnection:Open( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + cDataBase + ";Extended Properties='Excel 8.0;HDR=YES';Persist Security Info=False")

      CASE Lower( Right( cDataBase, 3 ) ) == ".db"
           IF ! File( cDataBase )
              oCatalog:Create( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + cDataBase + ";Extended Properties='Paradox 3.x';" )
           ENDIF
           oConnection:Open( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + cDataBase + ";Extended Properties='Paradox 3.x';" )

      CASE Lower( Right( cDataBase, 4 ) ) == ".fdb"
           IF ! File( cDataBase )
              oCatalog:Create( "Driver=Firebird/InterBase(r) driver;Uid=" + cUserName + ";Pwd=" + cPassword + ";DbName=" + cDataBase + ";" )
           ENDIF
#ifdef __XHARBOUR__
           __OutDebug( "Driver=Firebird/InterBase(r) driver;Uid=" + cUserName + ";Pwd=" + cPassword + ";DbName=" + cDataBase + ";" )
#endif
           oConnection:Open( "Driver=Firebird/InterBase(r) driver;Uid=" + cUserName + ";Pwd=" + cPassword + ";DbName=" + cDataBase + ";" )
           oConnection:CursorLocation := adUseClient

#ifdef __XHARBOUR__
           __OutDebug( oConnection:State != adStateClosed,;
                       oConnection:State != adStateOpen )
#endif

      CASE Upper( cDbEngine ) == "MYSQL"
           oConnection:Open( "DRIVER={MySQL ODBC 3.51 Driver};" + ;
                             "server=" + cServer + ;
                             ";database=" + cDataBase + ;
                             ";uid=" + cUserName + ;
                             ";pwd=" + cPassword )

   ENDCASE

   TRY
      oConnection:Execute( "DROP TABLE " + cTableName )
   CATCH
   END TRY

   TRY
      IF Lower( Right( cDataBase, 4 ) ) == ".fdb"
#ifdef __XHARBOUR__
         __OutDebug( "CREATE TABLE " + cTableName + " (" + StrTran( StrTran( aWAData[ WA_SQLSTRUCT ], "[", '"' ), "]", '"' ) + ")" )
#endif
         oConnection:Execute( "CREATE TABLE " + cTableName + " (" + StrTran( StrTran( aWAData[ WA_SQLSTRUCT ], "[", '"' ), "]", '"' ) + ")" )
      ELSE
         oConnection:Execute( "CREATE TABLE [" + cTableName + "] (" + aWAData[ WA_SQLSTRUCT ] + ")" )
      ENDIF
   CATCH
      oError := ErrorNew()
      oError:GenCode     := EG_CREATE
      oError:SubCode     := 1004
      oError:Description := HB_LANGERRMSG( EG_CREATE ) + " (" + ;
                            HB_LANGERRMSG( EG_UNSUPPORTED ) + ")"
      oError:FileName    := aOpenInfo[ UR_OI_NAME ]
      oError:CanDefault  := .T.

      FOR n := 0 To oConnection:Errors:Count - 1
          oError:Description += oConnection:Errors(n):Description
#ifdef __XHARBOUR__
          __OutDebug( oConnection:Errors(n):Description )
#endif
      NEXT

      UR_SUPER_ERROR( nWA, oError )
   END

   oConnection:Close()

RETURN SUCCESS

STATIC FUNCTION ADO_CREATEFIELDS( nWA, aStruct )

   LOCAL aWAData := USRRDD_AREADATA( nWA )
   LOCAL n

   aWAData[ WA_SQLSTRUCT ] := ""

   FOR n := 1 to Len( aStruct )
     IF n > 1
        aWAData[ WA_SQLSTRUCT ] += ", "
     ENDIF
     aWAData[ WA_SQLSTRUCT ] += "[" + aStruct[ n ][ DBS_NAME ] + "]"
     DO CASE
        CASE aStruct[ n ][ DBS_TYPE ] $ "C,Character"
             aWAData[ WA_SQLSTRUCT ] += " CHAR(" + AllTrim( Str( aStruct[ n ][ DBS_LEN ] ) ) + ") NULL"

        CASE aStruct[ n ][ DBS_TYPE ] == "V"
             aWAData[ WA_SQLSTRUCT ] += " VARCHAR(" + AllTrim( Str( aStruct[ n ][ DBS_LEN ] ) ) + ") NULL"

        CASE aStruct[ n ][ DBS_TYPE ] == "B"
             aWAData[ WA_SQLSTRUCT ] += " DOUBLE NULL"

        CASE aStruct[ n ][ DBS_TYPE ] == "Y"
             aWAData[ WA_SQLSTRUCT ] += " SMALLINT NULL"

        CASE aStruct[ n ][ DBS_TYPE ] == "I"
             aWAData[ WA_SQLSTRUCT ] += " MEDIUMINT NULL"

        CASE aStruct[ n ][ DBS_TYPE ] == "D"
             aWAData[ WA_SQLSTRUCT ] += " DATE NULL"

        CASE aStruct[ n ][ DBS_TYPE ] == "T"
             aWAData[ WA_SQLSTRUCT ] += " DATETIME NULL"

        CASE aStruct[ n ][ DBS_TYPE ] == "@"
             aWAData[ WA_SQLSTRUCT ] += " TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP"

        CASE aStruct[ n ][ DBS_TYPE ] == "M"
             aWAData[ WA_SQLSTRUCT ] += " TEXT NULL"

        CASE aStruct[ n ][ DBS_TYPE ] == "N"
             aWAData[ WA_SQLSTRUCT ] += " NUMERIC(" + AllTrim( Str( aStruct[ n ][ DBS_LEN ] ) ) + ")"

        CASE aStruct[ n ][ DBS_TYPE ] == "L"
             aWAData[ WA_SQLSTRUCT ] += " LOGICAL"
     ENDCASE
  NEXT

RETURN SUCCESS

STATIC FUNCTION ADO_OPEN( nWA, aOpenInfo )

   LOCAL aWAData := USRRDD_AREADATA( nWA )
   LOCAL cName, aField, oError, nResult
   LOCAL oRecordSet, nTotalFields, n

   // When there is no ALIAS we will create new one using file name
   IF Empty( aOpenInfo[ UR_OI_ALIAS ] )
      HB_FNAMESPLIT( aOpenInfo[ UR_OI_NAME ], , @cName )
      aOpenInfo[ UR_OI_ALIAS ] := cName
   ENDIF

   IF Empty( aOpenInfo[ UR_OI_CONNECT ] )
      aWAData[ WA_CONNECTION ] := TOleAuto():New( "ADODB.Connection" )
      aWAData[ WA_TABLENAME ] := s_cTableName
      aWAData[ WA_QUERY ]    := s_cQuery
      aWAData[ WA_USERNAME ] := s_cUserName
      aWAData[ WA_PASSWORD ] := s_cPassword
      aWAData[ WA_SERVER ] := s_cServer
      aWAData[ WA_ENGINE ] := s_cEngine
      aWAData[ WA_CONNOPEN ] := .T.

      DO CASE
      CASE Lower( Right( aOpenInfo[ UR_OI_NAME ], 4 ) ) == ".mdb"
           IF Empty( aWAData[ WA_PASSWORD ] )
              aWAData[ WA_CONNECTION ]:Open( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + aOpenInfo[ UR_OI_NAME ] )
           ELSE
              aWAData[ WA_CONNECTION ]:Open( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + aOpenInfo[ UR_OI_NAME ] + ";Jet OLEDB:Database Password=" + AllTrim( aWAData[ WA_PASSWORD ] ) )
           ENDIF

      CASE Lower( Right( aOpenInfo[ UR_OI_NAME ], 4 ) ) == ".xls"
           aWAData[ WA_CONNECTION ]:Open( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + aOpenInfo[ UR_OI_NAME ] + ";Extended Properties='Excel 8.0;HDR=YES';Persist Security Info=False" )

      CASE Lower( Right( aOpenInfo[ UR_OI_NAME ], 4 ) ) == ".dbf"
           aWAData[ WA_CONNECTION ]:Open( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + aOpenInfo[ UR_OI_NAME ] + ";Extended Properties=dBASE IV;User ID=Admin;Password=;" )

      CASE Lower( Right( aOpenInfo[ UR_OI_NAME ], 3 ) ) == ".db"
           aWAData[ WA_CONNECTION ]:Open( "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + aOpenInfo[ UR_OI_NAME ] + ";Extended Properties='Paradox 3.x';" )

      CASE aWAData[ WA_ENGINE ] == "MYSQL"
           aWAData[ WA_CONNECTION ]:Open( "DRIVER={MySQL ODBC 3.51 Driver};" + ;
                                          "server=" + aWAData[ WA_SERVER ] + ;
                                          ";database=" + aOpenInfo[ UR_OI_NAME ] + ;
                                          ";uid=" + aWAData[ WA_USERNAME ] + ;
                                          ";pwd=" + aWAData[ WA_PASSWORD ] )

      case aWAData[ WA_ENGINE ] == "SQL"
           aWAData[ WA_CONNECTION ]:Open( "Provider=SQLOLEDB;" + ;
                                          "server=" + aWAData[ WA_SERVER ] + ;
                                          ";database=" + aOpenInfo[ UR_OI_NAME ] + ;
                                          ";uid=" + aWAData[ WA_USERNAME ] + ;
                                          ";pwd=" + aWAData[ WA_PASSWORD ] )

      CASE aWAData[ WA_ENGINE ] == "ORACLE"
           aWAData[ WA_CONNECTION ]:Open( "Provider=MSDAORA.1;" + ;
                                          "Persist Security Info=False" + ;
                                          If( Empty( aWAData[ WA_SERVER ] ),;
                                          "", ";Data source=" + aWAData[ WA_SERVER ] ) + ;
                                          ";User ID=" + aWAData[ WA_USERNAME ] + ;
                                          ";Password=" + aWAData[ WA_PASSWORD ] )
      CASE aWAData[ WA_ENGINE ] == "FIREBIRD"
           aWAData[ WA_CONNECTION ]:Open( "Driver=Firebird/InterBase(r) driver;" + ;
                                          "Persist Security Info=False" +;
                                          ";Uid=" + aWAData[ WA_USERNAME ] +;
                                          ";Pwd=" + aWAData[ WA_PASSWORD ] +;
                                          ";DbName=" + aOpenInfo[ UR_OI_NAME ] )
      ENDCASE
   ELSE
      aWAData[ WA_CONNECTION ] := TOleAuto():New( aOpenInfo[ UR_OI_CONNECT ], "ADODB.Connection" )
      aWAData[ WA_TABLENAME ] := s_cTableName
      aWAData[ WA_QUERY ]    := s_cQuery
      aWAData[ WA_USERNAME ] := s_cUserName
      aWAData[ WA_PASSWORD ] := s_cPassword
      aWAData[ WA_SERVER ] := s_cServer
      aWAData[ WA_ENGINE ] := s_cEngine
      aWAData[ WA_CONNOPEN ] := .F.
   ENDIF

   // will be initilized
   s_cQuery := ""

   IF Empty( aWAData[ WA_QUERY ] )
      aWAData[ WA_QUERY ] := "SELECT * FROM "
   ENDIF

   oRecordSet := TOleAuto():New( "ADODB.Recordset" )

   IF oRecordSet == NIL
      oError := ErrorNew()
      oError:GenCode     := EG_OPEN
      oError:SubCode     := 1001
      oError:Description := HB_LANGERRMSG( EG_OPEN )
      oError:FileName    := aOpenInfo[ UR_OI_NAME ]
      oError:OsCode      := 0 // TODO
      oError:CanDefault  := .T.

      UR_SUPER_ERROR( nWA, oError )
      RETURN FAILURE
   ENDIF
   oRecordSet:CursorType     := adOpenDynamic
   oRecordSet:CursorLocation := adUseClient
   oRecordSet:LockType       := adLockPessimistic
   IF aWAData[ WA_QUERY ] == "SELECT * FROM "
      oRecordSet:Open( aWAData[ WA_QUERY ] + aWAData[ WA_TABLENAME ], aWAData[ WA_CONNECTION ] )
   ELSE
      oRecordSet:Open( aWAData[ WA_QUERY ], aWAData[ WA_CONNECTION ] )
   ENDIF

   TRY
      aWAData[ WA_CATALOG ] := TOleAuto():New( "ADOX.Catalog" )
      aWAData[ WA_CATALOG ]:ActiveConnection := aWAData[ WA_CONNECTION ]
   CATCH
   END TRY

   IF Empty( aWAData[ WA_CATALOG ] )
      TRY
         aWAData[ WA_CATALOG ] := aWAData[ WA_CONNECTION ]:OpenSchema( adSchemaIndexes )
      CATCH
      END TRY
   ENDIF

   aWAData[ WA_RECORDSET ] := oRecordSet
   aWAData[ WA_BOF ] := aWAData[ WA_EOF ] := .F.

   UR_SUPER_SETFIELDEXTENT( nWA, nTotalFields := oRecordSet:Fields:Count )

   FOR n := 1 TO nTotalFields
      aField := ARRAY( UR_FI_SIZE )
      aField[ UR_FI_NAME ]    := oRecordSet:Fields( n - 1 ):Name
      aField[ UR_FI_TYPE ]    := ADO_GETFIELDTYPE( oRecordSet:Fields( n - 1 ):Type )
      aField[ UR_FI_TYPEEXT ] := 0
      aField[ UR_FI_LEN ]     := ADO_GETFIELDSIZE( aField[ UR_FI_TYPE ], oRecordSet:Fields( n - 1 ):DefinedSize )
      aField[ UR_FI_DEC ]     := 0
#ifdef UR_FI_FLAGS
      aField[ UR_FI_FLAGS ]   := 0
#endif
#ifdef UR_FI_STEP
      aField[ UR_FI_STEP ]    := 0
#endif
      UR_SUPER_ADDFIELD( nWA, aField )
   NEXT

   nResult := UR_SUPER_OPEN( nWA, aOpenInfo )

   IF nResult == SUCCESS
      ADO_GOTOP( nWA )
   ENDIF

RETURN nResult

STATIC FUNCTION ADO_CLOSE( nWA )

   LOCAL aWAData := USRRDD_AREADATA( nWA )
   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   TRY
      oRecordSet:Close()
      IF ! Empty( aWAData[ WA_CONNOPEN ] )
        IF aWAData[ WA_CONNECTION ]:State != adStateClosed
           IF aWAData[ WA_CONNECTION ]:State != adStateOpen
              aWAData[ WA_CONNECTION ]:Cancel()
           ELSE
              aWAData[ WA_CONNECTION ]:Close()
           ENDIF
        ENDIF
      ENDIF
   CATCH
   END

RETURN UR_SUPER_CLOSE( nWA )

STATIC FUNCTION ADO_GETVALUE( nWA, nField, xValue )

   LOCAL aWAData := USRRDD_AREADATA( nWA ), oField, nType

   WITH OBJECT USRRDD_AREADATA( nWA )[ WA_RECORDSET ]
      oField := :Fields( nField - 1 )
      nType := ADO_GETFIELDTYPE( oField:Type )
      IF aWAData[ WA_EOF ] .OR. :EOF .OR. :BOF
         xValue := nil
         IF nType == HB_FT_STRING
            xValue := Space( oField:DefinedSize )
         ENDIF
      ELSE
         xValue := oField:Value

         IF nType == HB_FT_STRING
            IF ValType( xValue ) == "U"
                xValue := Space( oField:DefinedSize )
            ELSE
                xValue := PadR( xValue, oField:DefinedSize )
            ENDIF
         ELSEIF nType == HB_FT_DATE
            // Null values
            IF ValType( xValue ) == "U"
                xValue := cToD( "" )
            ENDIF
#ifdef HB_FT_DATETIME
         ELSEIF nType == HB_FT_DATETIME
            // Null values
            IF ValType( xValue ) == "U"
                xValue := cToD( "" )
            ENDIF
#endif
#ifdef HB_FT_TIMESTAMP
         ELSEIF nType == HB_FT_TIMESTAMP
            // Null values
            IF ValType( xValue ) == "U"
                xValue := cToD( "" )
            ENDIF
#endif
         ENDIF
      ENDIF
   END WITH

RETURN SUCCESS

STATIC FUNCTION ADO_GOTO( nWA, nRecord )

   LOCAL nRecNo

   WITH OBJECT USRRDD_AREADATA( nWA )[ WA_RECORDSET ]
      IF :RecordCount > 0
         :MoveFirst()
         :Move( nRecord - 1, 0 )
      ENDIF
      ADO_RECID( nWA, @nRecNo )
   END WITH

RETURN If( nRecord == nRecNo, SUCCESS, FAILURE )

STATIC FUNCTION ADO_GOTOID( nWA, nRecord )
   LOCAL nRecNo

   WITH OBJECT USRRDD_AREADATA( nWA )[ WA_RECORDSET ]
      IF :RecordCount > 0
         :MoveFirst()
         :Move( nRecord - 1, 0 )
      ENDIF
      ADO_RECID( nWA, @nRecNo )
   END WITH

RETURN If( nRecord == nRecNo, SUCCESS, FAILURE )

STATIC FUNCTION ADO_GOTOP( nWA )

   LOCAL aWAData    := USRRDD_AREADATA( nWA )
   LOCAL oRecordSet := aWAData[ WA_RECORDSET ]

   IF oRecordSet:RecordCount() != 0
      oRecordSet:MoveFirst()
   ENDIF

   aWAData[ WA_BOF ] := .F.
   aWAData[ WA_EOF ] := .F.

RETURN SUCCESS

STATIC FUNCTION ADO_GOBOTTOM( nWA )

   LOCAL aWAData    := USRRDD_AREADATA( nWA )
   LOCAL oRecordSet := aWAData[ WA_RECORDSET ]

   oRecordSet:MoveLast()

   aWAData[ WA_BOF ] := .F.
   aWAData[ WA_EOF ] := .F.

RETURN SUCCESS

STATIC FUNCTION ADO_SKIPRAW( nWA, nToSkip )

   LOCAL aWAData    := USRRDD_AREADATA( nWA )
   LOCAL oRecordSet := aWAData[ WA_RECORDSET ]
   LOCAL nResult    := SUCCESS

   IF ! Empty( aWAData[ WA_PENDINGREL ] )
      IF ADO_FORCEREL( nWA ) != SUCCESS
         RETURN FAILURE
      ENDIF
   ENDIF

   IF nToSkip != 0
      IF aWAData[ WA_EOF ]
         IF nToSkip > 0
            RETURN SUCCESS
         ENDIF
         ADO_GOBOTTOM( nWA )
         ++nToSkip
      ENDIF
      TRY
         IF aWAData[WA_CONNECTION]:State != adStateClosed
            IF nToSkip < 0 .AND. oRecordSet:AbsolutePosition <= -nToSkip
               oRecordSet:MoveFirst()
               aWAData[ WA_BOF ] := .T.
               aWAData[ WA_EOF ] := oRecordSet:EOF
            ELSEIF nToSkip != 0
               oRecordSet:Move( nToSkip )
               aWAData[ WA_BOF ] := .F.
               aWAData[ WA_EOF ] := oRecordSet:EOF
            ENDIF
         ELSE
            nResult := FAILURE
         ENDIF
      CATCH
         nResult := FAILURE
      END
   ENDIF


RETURN nResult

STATIC FUNCTION ADO_BOF( nWA, lBof )

   LOCAL aWAData := USRRDD_AREADATA( nWA )

   lBof := aWAData[ WA_BOF ]

RETURN SUCCESS

STATIC FUNCTION ADO_EOF( nWA, lEof )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]
   LOCAL nResult    := SUCCESS

   TRY
      IF USRRDD_AREADATA( nWA )[WA_CONNECTION]:State != adStateClosed
         lEof := ( oRecordSet:AbsolutePosition == -3 )
      ENDIF
   CATCH
      nResult := FAILURE
   END

RETURN nResult

STATIC FUNCTION ADO_DELETED( nWA, lDeleted )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   TRY
      IF oRecordSet:Status == adRecDeleted
	      lDeleted := .T.
	   ELSE
	      lDeleted := .F.
	   ENDIF
   CATCH
      lDeleted := .F.
   END TRY

RETURN SUCCESS

STATIC FUNCTION ADO_DELETE( nWA )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   oRecordSet:Delete()

   ADO_SKIPRAW( nWA, 1 )

RETURN SUCCESS

STATIC FUNCTION ADO_RECNO( nWA, nRecNo )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]
   LOCAL nResult    := SUCCESS

   TRY
      IF USRRDD_AREADATA( nWA )[WA_CONNECTION]:State != adStateClosed
         nRecno := If( oRecordSet:AbsolutePosition == -3, oRecordSet:RecordCount() + 1, oRecordSet:AbsolutePosition )

      ELSE
         nRecno  := 0
         nResult := FAILURE
      ENDIF
   CATCH
      nRecNo  := 0
      nResult := FAILURE
   END

RETURN nResult

STATIC FUNCTION ADO_RECID( nWA, nRecNo )
RETURN ADO_RECNO( nWA, @nRecNo )

STATIC FUNCTION ADO_RECCOUNT( nWA, nRecords )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   nRecords := oRecordSet:RecordCount()

RETURN SUCCESS

STATIC FUNCTION ADO_PUTVALUE( nWA, nField, xValue )

   LOCAL aWAData := USRRDD_AREADATA( nWA )
   LOCAL oRecordSet := aWAData[ WA_RECORDSET ]

   IF ! aWAData[ WA_EOF ] .AND. !( oRecordSet:Fields( nField - 1 ):Value == xValue )
      oRecordSet:Fields( nField - 1 ):Value := xValue
      TRY
         oRecordSet:Update()
      CATCH
      END
   ENDIF

RETURN SUCCESS

STATIC FUNCTION ADO_APPEND( nWA, lUnLockAll )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]
   
   HB_SYMBOL_UNUSED( lUnLockAll )

   oRecordSet:AddNew()

	TRY
      oRecordSet:Update()
	CATCH
   END

RETURN SUCCESS

STATIC FUNCTION ADO_FLUSH( nWA )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   TRY
      oRecordSet:Update()
   CATCH
   END

RETURN SUCCESS

STATIC FUNCTION ADO_ORDINFO( nWA, nIndex, aOrderInfo )

   LOCAL aWAData    := USRRDD_AREADATA( nWA )
   LOCAL oRecordSet := aWAData[ WA_RECORDSET ]
   LOCAL nResult := SUCCESS

	DO CASE
   CASE nIndex == DBOI_EXPRESSION
      IF ! Empty( aWAData[ WA_CATALOG ] ) .AND. !Empty( aOrderInfo[ UR_ORI_TAG ] ) .AND.;
            aOrderInfo[ UR_ORI_TAG ] < aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes:Count
         aOrderInfo[ UR_ORI_RESULT ] := aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes( aOrderInfo[ UR_ORI_TAG ] ):Name
      ELSE
         aOrderInfo[ UR_ORI_RESULT ] := ""
      ENDIF
   CASE nIndex == DBOI_NAME
      IF ! Empty( aWAData[ WA_CATALOG ] ) .AND. !Empty( aOrderInfo[ UR_ORI_TAG ] ) .AND.;
            aOrderInfo[ UR_ORI_TAG ] < aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes:Count
         aOrderInfo[ UR_ORI_RESULT ] := aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes( aOrderInfo[ UR_ORI_TAG ] ):Name
      ELSE
         aOrderInfo[ UR_ORI_RESULT ] := ""
      ENDIF
   CASE nIndex == DBOI_NUMBER
      aOrderInfo[ UR_ORI_RESULT ] := aOrderInfo[ UR_ORI_TAG ]
   CASE nIndex == DBOI_BAGNAME
      aOrderInfo[ UR_ORI_RESULT ] := ""
   CASE nIndex == DBOI_BAGEXT
      aOrderInfo[ UR_ORI_RESULT ] := ""
   CASE nIndex == DBOI_ORDERCOUNT
      IF ! Empty( aWAData[ WA_CATALOG ] )
         aOrderInfo[ UR_ORI_RESULT ] := aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes:Count
      ELSE
         aOrderInfo[ UR_ORI_RESULT ] := 0
      ENDIF
   CASE nIndex == DBOI_FILEHANDLE
      aOrderInfo[ UR_ORI_RESULT ] := 0
   CASE nIndex == DBOI_ISCOND
      aOrderInfo[ UR_ORI_RESULT ] := .F.
   CASE nIndex == DBOI_ISDESC
      aOrderInfo[ UR_ORI_RESULT ] := .F.
   CASE nIndex == DBOI_UNIQUE
      aOrderInfo[ UR_ORI_RESULT ] := .F.
   CASE nIndex == DBOI_POSITION
      IF aWAData[ WA_CONNECTION ]:State != adStateClosed
         ADO_RECID( nWA, @aOrderInfo[ UR_ORI_RESULT ] )
      ELSE
         aOrderInfo[ UR_ORI_RESULT ] := 0
         nResult := FAILURE
      ENDIF
   CASE nIndex == DBOI_RECNO
      IF aWAData[ WA_CONNECTION ]:State != adStateClosed
         ADO_RECID( nWA, @aOrderInfo[ UR_ORI_RESULT ] )
      ELSE
         aOrderInfo[ UR_ORI_RESULT ] := 0
         nResult := FAILURE
      ENDIF
   CASE nIndex == DBOI_KEYCOUNT
      IF aWAData[ WA_CONNECTION ]:State != adStateClosed
         aOrderInfo[ UR_ORI_RESULT ] := oRecordSet:RecordCount
      ELSE
         aOrderInfo[ UR_ORI_RESULT ] := 0
         nResult := FAILURE
      ENDIF
	ENDCASE

RETURN nResult

STATIC FUNCTION ADO_RECINFO( nWA, nRecord, nInfoType, uInfo )

   LOCAL nResult := SUCCESS
   //LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   DO CASE
   CASE nInfoType == UR_DBRI_DELETED
      uInfo := .F.
   CASE nInfoType == UR_DBRI_LOCKED
      uInfo := .T.
   CASE nInfoType == UR_DBRI_RECSIZE
   CASE nInfoType == UR_DBRI_RECNO
      nResult := ADO_RECID( nWA, @nRecord )
   CASE nInfoType == UR_DBRI_UPDATED
      uInfo := .F.
   CASE nInfoType == UR_DBRI_ENCRYPTED
      uInfo := .F.
   CASE nInfoType == UR_DBRI_RAWRECORD
      uInfo := ""
   CASE nInfoType == UR_DBRI_RAWMEMOS
      uInfo := ""
   CASE nInfoType == UR_DBRI_RAWDATA
      nResult := ADO_GOTO( nWA, nRecord )
      uInfo := ""
   ENDCASE

RETURN nResult

STATIC FUNCTION ADO_FIELDNAME( nWA, nField, cFieldName )
   LOCAL nResult := SUCCESS
   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   TRY
      cFieldName := oRecordSet:Fields( nField - 1 ):Name
   CATCH
      cFieldName := ''
      nResult    := FAILURE
   END TRY

RETURN nResult

STATIC FUNCTION ADO_FIELDINFO( nWA, nField, nInfoType, uInfo )

   LOCAL nType, nLen
   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   DO CASE
   CASE nInfoType == DBS_NAME
       uInfo := oRecordSet:Fields( nField - 1 ):Name

   CASE nInfoType == DBS_TYPE
       nType := ADO_GETFIELDTYPE( oRecordSet:Fields( nField - 1 ):Type )
       DO CASE
       CASE nType == HB_FT_STRING
           uInfo := "C"
       CASE nType == HB_FT_LOGICAL
           uInfo := "L"
       CASE nType == HB_FT_MEMO
           uInfo := "M"
       CASE nType == HB_FT_OLE
           uInfo := "G"
#ifdef HB_FT_PICTURE
       CASE nType == HB_FT_PICTURE
           uInfo := "P"
#endif
       CASE nType == HB_FT_ANY
           uInfo := "V"
       CASE nType == HB_FT_DATE
           uInfo := "D"
#ifdef HB_FT_DATETIME
       CASE nType == HB_FT_DATETIME
           uInfo := "T"
#endif
#ifdef HB_FT_TIMESTAMP
       CASE nType == HB_FT_TIMESTAMP
           uInfo := "@"
#endif
       CASE nType == HB_FT_LONG
           uInfo := "N"
       CASE nType == HB_FT_INTEGER
           uInfo := "I"
       CASE nType == HB_FT_DOUBLE
           uInfo := "B"
       OTHERWISE
           uInfo := "U"
       ENDCASE

   CASE nInfoType == DBS_LEN
        ADO_FIELDINFO( nWA, nField, DBS_TYPE, @nType )
        IF nType == 'N'
            nLen := oRecordSet:Fields( nField - 1 ):Precision
        ELSE
            nLen := oRecordSet:Fields( nField - 1 ):DefinedSize
        ENDIF
        // Un campo mayor de 1024 lo consideramos un campo memo
        uInfo := If( nLen > 1024, 10, nLen )

   CASE nInfoType == DBS_DEC
        ADO_FIELDINFO( nWA, nField, DBS_LEN, @nLen )
        ADO_FIELDINFO( nWA, nField, DBS_TYPE, @nType )
        IF oRecordSet:Fields( nField - 1 ):Type == adInteger
            uInfo := 0
        ELSEIF nType == 'N'
            uInfo := Min( Max( 0, nLen - 1 - oRecordSet:Fields( nField - 1 ):DefinedSize ), 15 )
        ELSE
            uInfo := 0
        ENDIF
#ifdef DBS_FLAG
   CASE nInfoType == DBS_FLAG
        uInfo := 0
#endif
#ifdef DBS_STEP
   CASE nInfoType == DBS_STEP
        uInfo := 0
#endif
   OTHERWISE
       RETURN FAILURE
   ENDCASE

RETURN SUCCESS

STATIC FUNCTION ADO_ORDLSTFOCUS( nWA, aOrderInfo )

   HB_SYMBOL_UNUSED( nWA )
   HB_SYMBOL_UNUSED( aOrderInfo )
/* TODO
   LOCAL nRecNo
   LOCAL aWAData    := USRRDD_AREADATA( nWA )
   LOCAL oRecordSet := aWAData[ WA_RECORDSET ]

   WITH OBJECT oRecordSet
      ADO_RECID( nWA, @nRecNo )

      :Close()
      IF aOrderInfo[ UR_ORI_TAG ] == 0
          :Open( "SELECT * FROM " + s_aTableNames[ nWA ] , HB_QWith(), adOpenDynamic, adLockPessimistic )
      ELSE
          //:Open( "SELECT * FROM " + ::oTabla:cTabla + ' ORDER BY ' + ::OrdKey( uTag ) , QWith(), adOpenDynamic, adLockPessimistic, adCmdUnspecified )
          :Open( "SELECT * FROM " + s_aTableNames[ nWA ], HB_QWith(), adOpenDynamic, adLockPessimistic )
      ENDIF
      aOrderInfo[ UR_ORI_RESULT ] := aOrderInfo[ UR_ORI_TAG ]

      ADO_GOTOP( nWA )
      ADO_GOTO( nWA, nRecNo )
   END WITH
*/
RETURN SUCCESS

STATIC FUNCTION ADO_PACK( nWA )

   //LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   HB_SYMBOL_UNUSED( nWA )

RETURN SUCCESS

STATIC FUNCTION ADO_RAWLOCK( nWA, nAction, nRecNo )

/* TODO
   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]
*/
   HB_SYMBOL_UNUSED( nRecNo )
   HB_SYMBOL_UNUSED( nWA )
   HB_SYMBOL_UNUSED( nAction )

RETURN SUCCESS

STATIC FUNCTION ADO_LOCK( nWA, aLockInfo  )

   //LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   HB_SYMBOL_UNUSED( nWA )

   aLockInfo[ UR_LI_METHOD ] := DBLM_MULTIPLE
   aLockInfo[ UR_LI_RECORD ] := RECNO()
   aLockInfo[ UR_LI_RESULT ] := .T.

RETURN SUCCESS

STATIC FUNCTION ADO_UNLOCK( nWA, xRecID )

/* TODO
   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]
*/
   HB_SYMBOL_UNUSED( xRecId )
   HB_SYMBOL_UNUSED( nWA )

RETURN SUCCESS

STATIC FUNCTION ADO_SETFILTER( nWA, aFilterInfo )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   oRecordSet:Filter := SQLTranslate( aFilterInfo[ UR_FRI_CEXPR ] )

RETURN SUCCESS

STATIC FUNCTION ADO_CLEARFILTER( nWA )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   TRY
      oRecordSet:Filter := ""
   CATCH
   END

RETURN SUCCESS

STATIC FUNCTION ADO_ZAP( nWA )

   LOCAL aWAData    := USRRDD_AREADATA( nWA )
   LOCAL oRecordSet := aWAData[ WA_RECORDSET ]

   IF aWAData[ WA_CONNECTION ] != NIL .and. aWAData[ WA_TABLENAME ] != nil
      TRY
         aWAData[ WA_CONNECTION ]:Execute( "TRUNCATE TABLE " + aWAData[ WA_TABLENAME ] )
      CATCH
         aWAData[ WA_CONNECTION ]:Execute( "DELETE * FROM " + aWAData[ WA_TABLENAME ] )
      END
      oRecordSet:Requery()
   ENDIF

RETURN SUCCESS

STATIC FUNCTION ADO_SETLOCATE( nWA, aScopeInfo )

   LOCAL aWAData := USRRDD_AREADATA( nWA )

   aScopeInfo[ UR_SI_CFOR ] := SQLTranslate( aWAData[ WA_LOCATEFOR ] )

   aWAData[ WA_SCOPEINFO ] := aScopeInfo

return SUCCESS

STATIC FUNCTION ADO_LOCATE( nWA, lContinue )

   LOCAL aWAData    := USRRDD_AREADATA( nWA )
   LOCAL oRecordSet := aWAData[ WA_RECORDSET ]

   oRecordSet:Find( aWAData[ WA_SCOPEINFO ][ UR_SI_CFOR ], If( lContinue, 1, 0 ) )
   aWAData[ WA_FOUND ] := ! oRecordSet:EOF
   aWAData[ WA_EOF ] := oRecordSet:EOF

RETURN SUCCESS

STATIC FUNCTION ADO_CLEARREL( nWA )

   LOCAL aWAData := USRRDD_AREADATA( nWA )
   LOCAL nKeys := 0, cKeyName

   TRY
      nKeys := aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Keys:Count
   CATCH
   END

   IF nKeys > 0
      cKeyName := aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Keys( nKeys - 1 ):Name
      IF !( Upper( cKeyName ) == "PRIMARYKEY" )
         aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Keys:Delete( cKeyName )
      ENDIF
   ENDIF

RETURN SUCCESS

STATIC FUNCTION ADO_RELAREA( nWA, nRelNo, nRelArea )

   LOCAL aWAData := USRRDD_AREADATA( nWA )
   TRY
      IF nRelNo <= aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Keys:Count()
         nRelArea := Select( aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Keys( nRelNo - 1 ):RelatedTable )
      ENDIF
   CATCH
      nRelArea := 0
   END TRY

RETURN SUCCESS

STATIC FUNCTION ADO_RELTEXT( nWA, nRelNo, cExpr )

   LOCAL aWAData := USRRDD_AREADATA( nWA )

   TRY
      IF nRelNo <= aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Keys:Count()
         cExpr := aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Keys( nRelNo - 1 ):Columns( 0 ):RelatedColumn
      ENDIF
   CATCH
      cExpr := ''
   END TRY

RETURN SUCCESS

STATIC FUNCTION ADO_SETREL( nWA, aRelInfo )

   LOCAL aWAData := USRRDD_AREADATA( nWA )
   LOCAL cParent := Alias( aRelInfo[ UR_RI_PARENT ] )
   LOCAL cChild  := Alias( aRelInfo[ UR_RI_CHILD ] )
   LOCAL cKeyName := cParent + "_" + cChild

   TRY
      aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Keys:Append( cKeyName, adKeyForeign,;
                                    aRelInfo[ UR_RI_CEXPR ], cChild, aRelInfo[ UR_RI_CEXPR ] )
   CATCH
      // raise error for can't create relation
   END

RETURN SUCCESS

STATIC FUNCTION ADO_FORCEREL( nWA )

   LOCAL aPendingRel
   LOCAL aWAData := USRRDD_AREADATA( nWA )

   IF ! Empty( aWAData[ WA_PENDINGREL ] )

      aPendingRel := aWAData[ WA_PENDINGREL ]
      aWAData[ WA_PENDINGREL ] := Nil

      RETURN ADO_RELEVAL( nWA, aPendingRel )
   ENDIF

RETURN SUCCESS

STATIC FUNCTION ADO_RELEVAL( nWA, aRelInfo )
   LOCAL aInfo, nReturn, nOrder, uResult

   nReturn := ADO_EVALBLOCK( aRelInfo[UR_RI_PARENT], aRelInfo[UR_RI_BEXPR], @uResult )

   IF  nReturn == SUCCESS
      /*
       *  Check the current order
       */
      aInfo := Array( UR_ORI_SIZE )
      nReturn := ADO_ORDINFO( nWA, DBOI_NUMBER, @aInfo )

      IF nReturn == SUCCESS
         nOrder := aInfo[UR_ORI_RESULT]
         IF nOrder != 0
            IF aRelInfo[UR_RI_SCOPED]
               aInfo[UR_ORI_NEWVAL] := uResult
               nReturn := ADO_ORDINFO( nWA, DBOI_SCOPETOP, @aInfo )
               IF nReturn == SUCCESS
                  nReturn := ADO_ORDINFO( nWA, DBOI_SCOPEBOTTOM, @aInfo )
               ENDIF
            ENDIF
            IF nReturn == SUCCESS
               nReturn := ADO_SEEK( nWA, .F., uResult, .F. )
            ENDIF
         ELSE
            nReturn := ADO_GOTO( nWA, uResult )
         ENDIF
      ENDIF
   ENDIF

RETURN nReturn


STATIC FUNCTION ADO_ORDLSTADD( nWA, aOrderInfo )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   TRY
      oRecordSet:Index := aOrderInfo[ UR_ORI_BAG ]
   CATCH
   END

RETURN SUCCESS

STATIC FUNCTION ADO_ORDLSTCLEAR( nWA )

   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]

   TRY
      oRecordSet:Index := ""
   CATCH
   END

RETURN SUCCESS

STATIC FUNCTION ADO_ORDCREATE( nWA, aOrderCreateInfo )

   LOCAL aWAData := USRRDD_AREADATA( nWA )
   LOCAL oIndex, oError, n, lFound := .F.

   if aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes != nil
      for n := 1 to aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes:Count
          oIndex := aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes( n - 1 )
          if oIndex:Name == If( ! Empty( aOrderCreateInfo[ UR_ORCR_TAGNAME ] ), aOrderCreateInfo[ UR_ORCR_TAGNAME ], aOrderCreateInfo[ UR_ORCR_CKEY ] )
             lFound := .T.
             exit
          endif
      next
   endif

   TRY
      IF aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes == nil .OR. ! lFound
         oIndex := TOleAuto():New( "ADOX.Index" )
         oIndex:Name := If( ! Empty( aOrderCreateInfo[ UR_ORCR_TAGNAME ] ), aOrderCreateInfo[ UR_ORCR_TAGNAME ], aOrderCreateInfo[ UR_ORCR_CKEY ] )
         oIndex:PrimaryKey := .F.
         oIndex:Unique := aOrderCreateInfo[ UR_ORCR_UNIQUE ]
         oIndex:Columns:Append( aOrderCreateInfo[ UR_ORCR_CKEY ] )
         aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes:Append( oIndex )
      ENDIF
   CATCH
      oError := ErrorNew()
      oError:GenCode     := EG_CREATE
      oError:SubCode     := 1004
      oError:Description := HB_LANGERRMSG( EG_CREATE ) + " (" + ;
                            HB_LANGERRMSG( EG_UNSUPPORTED ) + ")"
      oError:FileName    := aOrderCreateInfo[ UR_ORCR_BAGNAME ]
      oError:CanDefault  := .T.
      UR_SUPER_ERROR( nWA, oError )
   END

RETURN SUCCESS

STATIC FUNCTION ADO_ORDDESTROY( nWA, aOrderInfo )

   LOCAL aWAData := USRRDD_AREADATA( nWA ), n, oIndex

   IF aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes != nil
      FOR n := 1 to aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes:Count
          oIndex := aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes( n - 1 )
          IF oIndex:Name == aOrderInfo[ UR_ORI_TAG ]
             aWAData[ WA_CATALOG ]:Tables( aWAData[ WA_TABLENAME ] ):Indexes:Delete( oIndex:Name )
          ENDIF
      NEXT
   ENDIF

RETURN SUCCESS

STATIC FUNCTION ADO_EVALBLOCK( nArea, bBlock, uResult )

   LOCAL nCurrArea

   nCurrArea := Select()
   IF nCurrArea != nArea
      DbSelectArea( nArea )
   ELSE
      nCurrArea := 0
   ENDIF

   uResult := Eval( bBlock )

   IF nCurrArea > 0
      DbSelectArea( nCurrArea )
   ENDIF

RETURN SUCCESS

STATIC FUNCTION ADO_EXISTS( nRdd, cTable, cIndex, ulConnect )
   LOCAL lRet := FAILURE
   LOCAL aRData := USRRDD_RDDDATA( nRDD )
   
   HB_SYMBOL_UNUSED( ulConnect )

   IF ! Empty( cTable ) .AND. ! Empty( aRData[ WA_CATALOG ] )
      TRY
         aRData[ WA_CATALOG ]:Tables( cTable )
         lRet := SUCCESS
      CATCH
      END TRY
      IF ! Empty( cIndex )
         TRY
            aRData[ WA_CATALOG ]:Tables( cTable ):Indexes( cIndex )
            lRet := SUCCESS
         CATCH
         END TRY
      ENDIF
   ENDIF

RETURN lRet

STATIC FUNCTION ADO_DROP( nRdd, cTable, cIndex, ulConnect )
   LOCAL lRet := FAILURE
   LOCAL aRData := USRRDD_RDDDATA( nRDD )
   
   HB_SYMBOL_UNUSED( ulConnect )

   IF ! Empty( cTable ) .AND. ! Empty( aRData[ WA_CATALOG ] )
      TRY
         aRData[ WA_CATALOG ]:Tables:Delete( cTable )
         lRet := SUCCESS
      CATCH
      END TRY
      IF ! Empty( cIndex )
         TRY
            aRData[ WA_CATALOG ]:Tables( cTable ):Indexes:Delete( cIndex )
            lRet := SUCCESS
         CATCH
         END TRY
      ENDIF
   ENDIF

RETURN lRet

STATIC FUNCTION ADO_SEEK( nWA, lSoftSeek, cKey, lFindLast )

   HB_SYMBOL_UNUSED( nWA )
   HB_SYMBOL_UNUSED( lSoftSeek )
   HB_SYMBOL_UNUSED( cKey )
   HB_SYMBOL_UNUSED( lFindLast )
/* TODO
   LOCAL oRecordSet := USRRDD_AREADATA( nWA )[ WA_RECORDSET ]
   
   LPCDXTAG pTag;

   if ( FAST_GOCOLD( ( AREAP ) pArea ) == FAILURE )
      return FAILURE;

//oRecordSet:Find( aWAData[ WA_SCOPEINFO ][ UR_SI_CFOR ], If( lContinue, 1, 0 ) )
   LPCDXKEY pKey;
   HB_ERRCODE retval = SUCCESS;
   BOOL  fEOF = FALSE, fLast;
   ULONG ulRec;

   IF ! Empty( aWAData[ WA_PENDINGREL ] )
      ADO_FORCEREL( nWA )
   ENDIF

   pArea->fTop = pArea->fBottom = FALSE;
   pArea->fEof = FALSE;

   if ( pTag->UsrUnique )
      fLast = !pTag->UsrAscend;
   else
      fLast = pTag->UsrAscend ? fFindLast : !fFindLast;

   // TODO: runtime error if valtype(pKeyItm) != pTag->Type
   pKey = hb_cdxKeyPutItem( NULL, pKeyItm, fLast ? CDX_MAX_REC_NUM : CDX_IGNORE_REC_NUM, pTag, TRUE, FALSE );

   hb_cdxIndexLockRead( pTag->pIndex );
   hb_cdxTagRefreshScope( pTag );
   ulRec = hb_cdxTagKeyFind( pTag, pKey );
   if ( ( ulRec == 0 && ! fSoftSeek ) || pTag->TagEOF )
      fEOF = TRUE;
   else // if ( fSoftSeek )
   {
      if ( ! hb_cdxBottomScope( pTag ) )
         fEOF = TRUE;
      else if ( ! hb_cdxTopScope( pTag ) )
      {
         hb_cdxTagGoTop( pTag );
         if ( pTag->CurKey->rec == 0 )
            fEOF = TRUE;
      }
   }
   hb_cdxIndexUnLockRead( pTag->pIndex );
   if ( !fEOF )
   {
      retval = SELF_GOTO( ( AREAP ) pArea, pTag->CurKey->rec );
      if ( retval != FAILURE && pArea->fPositioned )
      {
         retval = SELF_SKIPFILTER( ( AREAP ) pArea, fFindLast ? -1 : 1 );
         if ( retval != FAILURE && ulRec && pArea->fPositioned )
         {
            pArea->fFound = ( ulRec == pArea->ulRecNo ||
                     hb_cdxValCompare( pTag, pKey->val, pKey->len,
                        pTag->CurKey->val, pTag->CurKey->len, FALSE ) == 0 );
            if ( ! pArea->fFound && ! fSoftSeek )
               fEOF = TRUE;
         }
      }
   }
   if ( retval != FAILURE &&
        ( fEOF || ! hb_cdxTopScope( pTag ) ||
                  ! hb_cdxBottomScope( pTag ) ) )
   {
      retval = SELF_GOTO( ( AREAP ) pArea, 0 );
   }
   pArea->fBof = FALSE;
   hb_cdxKeyFree( pKey );
   return retval;
*/
RETURN FAILURE

static function ADO_FOUND( nWA, lFound )

   local aWAData := USRRDD_AREADATA( nWA )

   lFound := aWAData[ WA_FOUND ]

return SUCCESS

FUNCTION ADORDD_GETFUNCTABLE( pFuncCount, pFuncTable, pSuperTable, nRddID )

   LOCAL cSuperRDD := NIL   /* NO SUPER RDD */
   LOCAL aADOFunc[ UR_METHODCOUNT ]

   aADOFunc[ UR_INIT ]         := ( @ADO_INIT() )
   aADOFunc[ UR_NEW ]          := ( @ADO_NEW() )
   aADOFunc[ UR_CREATE ]       := ( @ADO_CREATE() )
   aADOFunc[ UR_CREATEFIELDS ] := ( @ADO_CREATEFIELDS() )
   aADOFunc[ UR_OPEN ]         := ( @ADO_OPEN() )
   aADOFunc[ UR_CLOSE ]        := ( @ADO_CLOSE() )
   aADOFunc[ UR_BOF  ]         := ( @ADO_BOF() )
   aADOFunc[ UR_EOF  ]         := ( @ADO_EOF() )
   aADOFunc[ UR_DELETED ]      := ( @ADO_DELETED() )
   aADOFunc[ UR_SKIPRAW ]      := ( @ADO_SKIPRAW() )
   aADOFunc[ UR_GOTO ]         := ( @ADO_GOTO() )
   aADOFunc[ UR_GOTOID ]       := ( @ADO_GOTOID() )
   aADOFunc[ UR_GOTOP ]        := ( @ADO_GOTOP() )
   aADOFunc[ UR_GOBOTTOM ]     := ( @ADO_GOBOTTOM() )
   aADOFunc[ UR_RECNO ]        := ( @ADO_RECNO() )
   aADOFunc[ UR_RECID ]        := ( @ADO_RECID() )
   aADOFunc[ UR_RECCOUNT ]     := ( @ADO_RECCOUNT() )
   aADOFunc[ UR_GETVALUE ]     := ( @ADO_GETVALUE() )
   aADOFunc[ UR_PUTVALUE ]     := ( @ADO_PUTVALUE() )
   aADOFunc[ UR_DELETE ]       := ( @ADO_DELETE() )
   aADOFunc[ UR_APPEND ]       := ( @ADO_APPEND() )
   aADOFunc[ UR_FLUSH ]        := ( @ADO_FLUSH() )
   aADOFunc[ UR_ORDINFO ]      := ( @ADO_ORDINFO() )
   aADOFunc[ UR_RECINFO ]      := ( @ADO_RECINFO() )
   aADOFunc[ UR_FIELDINFO ]    := ( @ADO_FIELDINFO() )
   aADOFunc[ UR_FIELDNAME ]    := ( @ADO_FIELDNAME() )
   aADOFunc[ UR_ORDLSTFOCUS ]  := ( @ADO_ORDLSTFOCUS() )
   aADOFunc[ UR_PACK ]         := ( @ADO_PACK() )
   aADOFunc[ UR_RAWLOCK ]      := ( @ADO_RAWLOCK() )
   aADOFunc[ UR_LOCK ]         := ( @ADO_LOCK() )
   aADOFunc[ UR_UNLOCK ]       := ( @ADO_UNLOCK() )
   aADOFunc[ UR_SETFILTER ]    := ( @ADO_SETFILTER() )
   aADOFunc[ UR_CLEARFILTER ]  := ( @ADO_CLEARFILTER() )
   aADOFunc[ UR_ZAP ]          := ( @ADO_ZAP() )
   aADOFunc[ UR_SETLOCATE ]    := ( @ADO_SETLOCATE() )
   aADOFunc[ UR_LOCATE ]       := ( @ADO_LOCATE() )
   aAdoFunc[ UR_FOUND ]        := ( @ADO_FOUND() )
   aADOFunc[ UR_FORCEREL ]     := ( @ADO_FORCEREL() )
   aADOFunc[ UR_RELEVAL ]      := ( @ADO_RELEVAL() )
   aADOFunc[ UR_CLEARREL ]     := ( @ADO_CLEARREL() )
   aADOFunc[ UR_RELAREA ]      := ( @ADO_RELAREA() )
   aADOFunc[ UR_RELTEXT ]      := ( @ADO_RELTEXT() )
   aADOFunc[ UR_SETREL ]       := ( @ADO_SETREL() )
   aADOFunc[ UR_ORDCREATE ]    := ( @ADO_ORDCREATE() )
   aADOFunc[ UR_ORDDESTROY ]   := ( @ADO_ORDDESTROY() )
   aADOFunc[ UR_ORDLSTADD ]    := ( @ADO_ORDLSTADD() )
   aADOFunc[ UR_ORDLSTCLEAR ]  := ( @ADO_ORDLSTCLEAR() )
   aADOFunc[ UR_EVALBLOCK ]    := ( @ADO_EVALBLOCK() )
   aADOFunc[ UR_SEEK ]         := ( @ADO_SEEK() )
   aADOFunc[ UR_EXISTS ]       := ( @ADO_EXISTS() )
   aADOFunc[ UR_DROP ]         := ( @ADO_DROP() )

RETURN USRRDD_GETFUNCTABLE( pFuncCount, pFuncTable, pSuperTable, nRddID, cSuperRDD,;
                            aADOFunc )

INIT PROCEDURE ADORDD_INIT()
   rddRegister( "ADORDD", RDT_FULL )
RETURN

STATIC FUNCTION ADO_GETFIELDSIZE( nDBFFieldType, nADOFieldSize )

   LOCAL nDBFFieldSize := 0

   DO CASE

      CASE nDBFFieldType == HB_FT_STRING
           nDBFFieldSize := nADOFieldSize

      CASE nDBFFieldType == HB_FT_INTEGER
           nDBFFieldSize := nADOFieldSize

      CASE nDBFFieldType == HB_FT_DATE
           nDBFFieldSize := 8

      CASE nDBFFieldType == HB_FT_DOUBLE
           nDBFFieldSize := nADOFieldSize

#ifdef HB_FT_DATETIME
      CASE nDBFFieldType == HB_FT_DATETIME
           nDBFFieldSize := 8
#endif

#ifdef HB_FT_TIMESTAMP
      CASE nDBFFieldType == HB_FT_TIMESTAMP
           nDBFFieldSize := 8
#endif

      CASE nDBFFieldType == HB_FT_OLE
           nDBFFieldSize := 10

#ifdef HB_FT_PICTURE
      CASE nDBFFieldType == HB_FT_PICTURE
           nDBFFieldSize := 10
#endif

      CASE nDBFFieldType == HB_FT_LOGICAL
           nDBFFieldSize := 1

      CASE nDBFFieldType == HB_FT_MEMO
           nDBFFieldSize := 10

   ENDCASE

RETURN nDBFFieldSize

STATIC FUNCTION ADO_GETFIELDTYPE( nADOFieldType )

   LOCAL nDBFFieldType := 0

   DO CASE

      CASE nADOFieldType == adEmpty
      CASE nADOFieldType == adTinyInt
         nDBFFieldType := HB_FT_INTEGER

      CASE nADOFieldType == adSmallInt
         nDBFFieldType := HB_FT_INTEGER

      CASE nADOFieldType == adInteger
         nDBFFieldType := HB_FT_INTEGER

      CASE nADOFieldType == adBigInt
         nDBFFieldType := HB_FT_INTEGER

      CASE nADOFieldType == adUnsignedTinyInt
      CASE nADOFieldType == adUnsignedSmallInt
      CASE nADOFieldType == adUnsignedInt
      CASE nADOFieldType == adUnsignedBigInt
      CASE nADOFieldType == adSingle

      CASE nADOFieldType == adDouble
		   nDBFFieldType := HB_FT_DOUBLE

      CASE nADOFieldType == adCurrency
         nDBFFieldType := HB_FT_INTEGER

      CASE nADOFieldType == adDecimal
         nDBFFieldType := HB_FT_LONG

      CASE nADOFieldType == adNumeric
         nDBFFieldType := HB_FT_LONG


      CASE nADOFieldType == adError
      CASE nADOFieldType == adUserDefined
      CASE nADOFieldType == adVariant
		   nDBFFieldType := HB_FT_ANY

      CASE nADOFieldType == adIDispatch

      CASE nADOFieldType == adIUnknown

      CASE nADOFieldType == adGUID
		   nDBFFieldType := HB_FT_STRING

      CASE nADOFieldType == adDate
#ifdef HB_FT_DATETIME
         nDBFFieldType := HB_FT_DATETIME
#else
         nDBFFieldType := HB_FT_DATE
#endif

      CASE nADOFieldType == adDBDate
#ifdef HB_FT_DATETIME
         nDBFFieldType := HB_FT_DATETIME
#else
         nDBFFieldType := HB_FT_DATE
#endif

      CASE nADOFieldType == adDBTime

      CASE nADOFieldType == adDBTimeStamp
#ifdef HB_FT_TIMESTAMP
         nDBFFieldType := HB_FT_TIMESTAMP
#else
         //nDBFFieldType := HB_FT_DATE
#endif

      CASE nADOFieldType == adFileTime
#ifdef HB_FT_DATETIME
         nDBFFieldType := HB_FT_DATETIME
#else
         //nDBFFieldType := HB_FT_DATE
#endif

      CASE nADOFieldType == adBSTR
	 nDBFFieldType := HB_FT_STRING

      CASE nADOFieldType == adChar
         nDBFFieldType := HB_FT_STRING

      CASE nADOFieldType == adVarChar
         nDBFFieldType := HB_FT_STRING

      CASE nADOFieldType == adLongVarChar
         nDBFFieldType := HB_FT_MEMO

      CASE nADOFieldType == adWChar
         nDBFFieldType := HB_FT_STRING

      CASE nADOFieldType == adVarWChar
         nDBFFieldType := HB_FT_STRING

      CASE nADOFieldType == adBinary
         nDBFFieldType := HB_FT_OLE

      CASE nADOFieldType == adVarBinary
         nDBFFieldType := HB_FT_OLE

      CASE nADOFieldType == adLongVarBinary
         nDBFFieldType := HB_FT_OLE

      CASE nADOFieldType == adChapter

      CASE nADOFieldType == adVarNumeric
      // case nADOFieldType == adArray

      CASE nADOFieldType == adBoolean
         nDBFFieldType := HB_FT_LOGICAL

      CASE nADOFieldType == adLongVarWChar
         nDBFFieldType := HB_FT_MEMO

      CASE nADOFieldType == adPropVariant
         nDBFFieldType := HB_FT_MEMO

   ENDCASE

RETURN nDBFFieldType

function HB_AdoSetTable( cTableName )

   s_cTableName := cTableName

return nil

function HB_AdoSetEngine( cEngine )

   s_cEngine := cEngine

return nil

function HB_AdoSetServer( cServer )

   s_cServer := cServer

return nil

function HB_AdoSetUser( cUser )

   s_cUserName := cUser

RETURN NIL

function HB_AdoSetPassword( cPassword )

   s_cPassword := cPassword

RETURN NIL

FUNCTION HB_AdoSetQuery( cQuery )

   DEFAULT cQuery TO "SELECT * FROM "

   s_cQuery := cQuery

RETURN NIL

FUNCTION HB_AdoSetLocateFor( cLocateFor )

   USRRDD_AREADATA( Select() )[ WA_LOCATEFOR ] := cLocateFor

RETURN NIL

STATIC FUNCTION SQLTranslate( cExpr )

  if Left( cExpr, 1 ) == '"' .and. Right( cExpr, 1 ) == '"'
     cExpr := SubStr( cExpr, 2, Len( cExpr ) - 2 )
  endif

  cExpr := StrTran( cExpr, '""', "" )
  cExpr := StrTran( cExpr, '"', "'" )
  cExpr := StrTran( cExpr, "''", "'" )
  cExpr := StrTran( cExpr, "==", "=" )
  cExpr := StrTran( cExpr, ".and.", "AND" )
  cExpr := StrTran( cExpr, ".or.", "OR" )
  cExpr := StrTran( cExpr, ".AND.", "AND" )
  cExpr := StrTran( cExpr, ".OR.", "OR" )

RETURN cExpr

FUNCTION HB_AdoRddGetConnection( nWA )

   DEFAULT nWA TO Select()

RETURN USRRDD_AREADATA( nWA )[ WA_CONNECTION ]

FUNCTION HB_AdoRddGetCatalog( nWA )

   DEFAULT nWA TO Select()

RETURN USRRDD_AREADATA( nWA )[ WA_CATALOG ]

FUNCTION HB_AdoRddGetRecordSet( nWA )

   local aWAData

   DEFAULT nWA TO Select()

   aWAData := USRRDD_AREADATA( nWA )

RETURN If( aWAData != nil, aWAData[ WA_RECORDSET ], nil )