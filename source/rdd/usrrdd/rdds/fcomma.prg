/*
 * $Id: fcomma.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 *    FCOMMA RDD
 *
 * Copyright 2006 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
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
 * A simple RDD which uses HB_F*() functions from MISC library
 * to access CSV files. It allow to open an CSV file and navigate
 * using SKIP()/GOTO()/GOTOP()/GOBOTTOM() functions using
 * BOF()/EOF()/RECNO()/LASTREC() to check current state.
 * HB_F*() functions does not support single field access and allow
 * to read only the whole line. This RDD also. I only added one
 * virtual field which exist in all tables open by this RDD called
 * LINE which contains the current .csv file line.
 */

#include "rddsys.ch"
#include "usrrdd.ch"
#include "fileio.ch"
#include "error.ch"

ANNOUNCE FCOMMA

/*
 * non work area methods receive RDD ID as first parameter
 * Methods INIT and EXIT does not have to execute SUPER methods - these is
 * always done by low level USRRDD code
 */
STATIC FUNCTION FCM_INIT( nRDD )
   LOCAL aRData := ARRAY( 10 )

   /* Set in our private RDD ITEM the array with HB_F*() work are numbers */
   AFILL( aRData, -1 )
   USRRDD_RDDDATA( nRDD, aRData )

RETURN SUCCESS

/*
 * methods: NEW and RELEASE receive pointer to work area structure
 * not work area number. It's necessary because the can be executed
 * before work area is allocated
 * these methods does not have to execute SUPER methods - these is
 * always done by low level USRRDD code
 */
STATIC FUNCTION FCM_NEW( pWA )
   LOCAL aWData := { -1, .F., .F. } 

   /*
    * Set in our private AREA item the array with slot number and
    * BOF/EOF flags. There is no BOF support in HB_F* function so
    * we have to emulate it and there is no phantom record so we
    * cannot return EOF flag directly.
    */
   USRRDD_AREADATA( pWA, aWData )

RETURN SUCCESS

STATIC FUNCTION FCM_CREATE( nWA, aOpenInfo )
   LOCAL oError := ErrorNew()

   oError:GenCode     := EG_CREATE
   oError:SubCode     := 1004
   oError:Description := HB_LANGERRMSG( EG_CREATE ) + " (" + ;
                         HB_LANGERRMSG( EG_UNSUPPORTED ) + ")"
   oError:FileName    := aOpenInfo[ UR_OI_NAME ]
   oError:CanDefault  := .T.
   UR_SUPER_ERROR( nWA, oError )
RETURN FAILURE

STATIC FUNCTION FCM_OPEN( nWA, aOpenInfo )
   LOCAL cName, nMode, nSlot, nHandle, aRData, aWData, aField, oError, nResult

   /* When there is no ALIAS we will create new one using file name */
   IF aOpenInfo[ UR_OI_ALIAS ] == NIL
      HB_FNAMESPLIT( aOpenInfo[ UR_OI_NAME ], , @cName )
      aOpenInfo[ UR_OI_ALIAS ] := cName
   ENDIF

   nMode := IIF( aOpenInfo[ UR_OI_SHARED ], FO_SHARED , FO_EXCLUSIVE ) + ;
            IIF( aOpenInfo[ UR_OI_READONLY ], FO_READ, FO_READWRITE )

   aRData := USRRDD_RDDDATA( USRRDD_ID( nWA ) )
   aWData := USRRDD_AREADATA( nWA )
   nSlot := ASCAN( aRData, -1 )

   IF nSlot == 0
      oError := ErrorNew()
      oError:GenCode     := EG_OPEN
      oError:SubCode     := 1000
      oError:Description := HB_LANGERRMSG( EG_OPEN ) + ", no free slots"
      oError:FileName    := aOpenInfo[ UR_OI_NAME ]
      oError:CanDefault  := .T.
      UR_SUPER_ERROR( nWA, oError )
      RETURN FAILURE
   ENDIF

   HB_FSELECT( nSlot )
   nHandle := HB_FUSE( aOpenInfo[ UR_OI_NAME ], nMode )
   IF nHandle == -1
      oError := ErrorNew()
      oError:GenCode     := EG_OPEN
      oError:SubCode     := 1001
      oError:Description := HB_LANGERRMSG( EG_OPEN )
      oError:FileName    := aOpenInfo[ UR_OI_NAME ]
      oError:OsCode      := fError()
      oError:CanDefault  := .T.

      UR_SUPER_ERROR( nWA, oError )
      RETURN FAILURE
   ENDIF
   aRData[ nSlot ] := nHandle
   aWData[ 1 ] := nSlot
   aWData[ 2 ] := aWData[ 3 ] := .F.

   /* Set one field called LINE to access current record buffer */
   UR_SUPER_SETFIELDEXTENT( nWA, 1 )
   aField := ARRAY( UR_FI_SIZE )
   aField[ UR_FI_NAME ]    := "LINE"
   aField[ UR_FI_TYPE ]    := HB_FT_STRING
   aField[ UR_FI_TYPEEXT ] := 0
   aField[ UR_FI_LEN ]     := 80   // set any arbitrary length - the real size will be differ
   aField[ UR_FI_DEC ]     := 0
   UR_SUPER_ADDFIELD( nWA, aField )

   /* Call SUPER OPEN to finish allocating work area (f.e.: alias settings) */
   nResult := UR_SUPER_OPEN( nWA, aOpenInfo )

   IF nResult == SUCCESS
      FCM_GOTOP( nWA )
   ENDIF

RETURN nResult

STATIC FUNCTION FCM_CLOSE( nWA )
   LOCAL aRData, nSlot := USRRDD_AREADATA( nWA )[ 1 ]

   IF nSlot >= 0
      HB_FSELECT( nSlot )
      HB_FUSE()
      aRData := USRRDD_RDDDATA( USRRDD_ID( nWA ) )
      aRData[ nSlot ] := -1
   ENDIF
RETURN UR_SUPER_CLOSE( nWA )

STATIC FUNCTION FCM_GETVALUE( nWA, nField, xValue )
   LOCAL aWData := USRRDD_AREADATA( nWA )

   IF nField == 1
      IF aWData[ 3 ]
         /* We are at EOF position, return empty value */
         xValue := ""
      ELSE
         HB_FSELECT( aWData[ 1 ] )
         xValue := HB_FREADLN()
      ENDIF
      RETURN SUCCESS
   ENDIF
RETURN FAILURE

STATIC FUNCTION FCM_GOTO( nWA, nRecord )
   LOCAL aWData := USRRDD_AREADATA( nWA )
   HB_FSELECT( aWData[ 1 ] )
   IF nRecord <= 0
      aWData[ 2 ] := aWData[ 3 ] := .T.
   ELSEIF nRecord == 1
      HB_FGOTOP()
      aWData[ 2 ] := aWData[ 3 ] := HB_FEOF()
   ELSE
      HB_FSKIP(0) /* Clear the EOF flag inside HB_F* engin
                     - it's not done automatically in HB_FGOBOTTOM() :-( */
      HB_FGOTO( nRecord )
      aWData[ 2 ] := HB_FRECNO() == 0
      aWData[ 3 ] := HB_FEOF()
   ENDIF
RETURN SUCCESS

STATIC FUNCTION FCM_GOTOID( nWA, nRecord )
RETURN FCM_GOTO( nWA, nRecord )

STATIC FUNCTION FCM_GOTOP( nWA )
   LOCAL aWData := USRRDD_AREADATA( nWA )
   HB_FSELECT( aWData[ 1 ] )
   HB_FGOTOP()
   aWData[ 2 ] := aWData[ 3 ] := HB_FEOF()
RETURN SUCCESS

STATIC FUNCTION FCM_GOBOTTOM( nWA )
   LOCAL aWData := USRRDD_AREADATA( nWA )
   HB_FSELECT( aWData[ 1 ] )
   IF HB_FLASTREC() == 0
      aWData[ 2 ] := aWData[ 3 ] := .T.
   ELSE
      HB_FSKIP(0) /* Clear the EOF flag inside HB_F* engin
                     - it's not done automatically in HB_FGOBOTTOM() :-( */
      HB_FGOBOTTOM()
      aWData[ 2 ] := aWData[ 3 ] := .F.
   ENDIF
RETURN SUCCESS

STATIC FUNCTION FCM_SKIPRAW( nWA, nRecords )
   LOCAL aWData

   IF nRecords != 0
      aWData := USRRDD_AREADATA( nWA )
      HB_FSELECT( aWData[ 1 ] )
      IF aWData[ 3 ]
         IF nRecords > 0
            RETURN SUCCESS
         ENDIF
         FCM_GOBOTTOM( nWA )
         ++nRecords
      ENDIF
      IF nRecords < 0 .AND. HB_FRECNO() <= -nRecords
         HB_FGOTOP()
         aWData[ 2 ] := .T.
         aWData[ 3 ] := HB_FEOF()
      ELSEIF nRecords != 0
         HB_FSKIP( nRecords )
         aWData[ 2 ] := .F.
         aWData[ 3 ] := HB_FEOF()
      ENDIF
   ENDIF
RETURN SUCCESS

STATIC FUNCTION FCM_BOF( nWA, lBof )
   LOCAL aWData := USRRDD_AREADATA( nWA )
   lBof := aWData[ 2 ]
RETURN SUCCESS

STATIC FUNCTION FCM_EOF( nWA, lEof )
   LOCAL aWData := USRRDD_AREADATA( nWA )
   lEof := aWData[ 3 ]
RETURN SUCCESS

STATIC FUNCTION FCM_DELETED( nWA, lDeleted )
   (nWA)
   lDeleted := .F.
RETURN SUCCESS

STATIC FUNCTION FCM_RECID( nWA, nRecNo )
   LOCAL aWData := USRRDD_AREADATA( nWA )
   HB_FSELECT( aWData[ 1 ] )
   IF aWData[ 3 ]
      nRecNo := HB_FLASTREC() + 1
   ELSE
      nRecNo := HB_FRECNO()
   ENDIF
RETURN SUCCESS

STATIC FUNCTION FCM_RECCOUNT( nWA, nRecords )
   HB_FSELECT( USRRDD_AREADATA( nWA )[ 1 ] )
   nRecords := HB_FLASTREC()
RETURN SUCCESS

/*
 * This function have to exist in all RDD and then name have to be in
 * format: <RDDNAME>_GETFUNCTABLE
 */
FUNCTION FCOMMA_GETFUNCTABLE( pFuncCount, pFuncTable, pSuperTable, nRddID )
   LOCAL cSuperRDD := NIL     /* NO SUPER RDD */
   LOCAL aMyFunc[ UR_METHODCOUNT ]

   aMyFunc[ UR_INIT ]     := ( @FCM_INIT()     )
   aMyFunc[ UR_NEW ]      := ( @FCM_NEW()      )
   aMyFunc[ UR_CREATE ]   := ( @FCM_CREATE()   )
   aMyFunc[ UR_OPEN ]     := ( @FCM_OPEN()     )
   aMyFunc[ UR_CLOSE ]    := ( @FCM_CLOSE()    )
   aMyFunc[ UR_BOF  ]     := ( @FCM_BOF()      )
   aMyFunc[ UR_EOF  ]     := ( @FCM_EOF()      )
   aMyFunc[ UR_DELETED ]  := ( @FCM_DELETED()  )
   aMyFunc[ UR_SKIPRAW ]  := ( @FCM_SKIPRAW()  )
   aMyFunc[ UR_GOTO ]     := ( @FCM_GOTO()     )
   aMyFunc[ UR_GOTOID ]   := ( @FCM_GOTOID()   )
   aMyFunc[ UR_GOTOP ]    := ( @FCM_GOTOP()    )
   aMyFunc[ UR_GOBOTTOM ] := ( @FCM_GOBOTTOM() )
   aMyFunc[ UR_RECID ]    := ( @FCM_RECID()    )
   aMyFunc[ UR_RECCOUNT ] := ( @FCM_RECCOUNT() )
   aMyFunc[ UR_GETVALUE ] := ( @FCM_GETVALUE() )

RETURN USRRDD_GETFUNCTABLE( pFuncCount, pFuncTable, pSuperTable, nRddID, ;
                            cSuperRDD, aMyFunc )

INIT PROCEDURE FCOMMA_INIT()
   rddRegister( "FCOMMA", RDT_FULL )
RETURN
