/*
 * $Id: hbextern.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * HBEXTERN.CH generator
 *
 * Copyright 1999 Ryszard Glab <rglab@imid.med.pl>
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

/* NOTE: The process is not completely automatical, the generated output should
         be edited by hand after extraction. */

#include "common.ch"
#include "fileio.ch"

#ifndef __HARBOUR__
   #define hb_OSNewLine() ( Chr( 13 ) + Chr( 10 ) )
#endif

#define PATH_SEPARATOR "\"
#define BASE_DIR "..\..\source\"

STATIC aNames := {}

PROCEDURE MAIN()
   LOCAL aDirs:={ BASE_DIR + "vm",;
                  BASE_DIR + "rtl",;
                  BASE_DIR + "rdd",;
                  BASE_DIR + "pp",;
                  BASE_DIR + "tools" }
   LOCAL i
   LOCAL aFiles
   LOCAL nOutput
   LOCAL nTime:=SECONDS()
   
   nOutput := FCREATE( "hbextern.ch_" )
   IF nOutput > 0
      FOR i:=1 TO LEN(aDirs)
         FWRITE( nOutput, "// Files from: " +aDirs[i] )
         FWRITE( nOutput, hb_OSNewLine() )
         FWRITE( nOutput, "//" )
         FWRITE( nOutput, hb_OSNewLine() )
         aFiles := DIRECTORY( aDirs[i] +PATH_SEPARATOR +"*.c" )
         ProcessDir( nOutput, aFiles, aDirs[ i ] )
         aFiles := DIRECTORY( aDirs[i] +PATH_SEPARATOR +"*.prg" )
         ProcessDir( nOutput, aFiles, aDirs[ i ] )
         FWRITE( nOutput, "//" )
         FWRITE( nOutput, REPLICATE( "-", 60) )
         FWRITE( nOutput, hb_OSNewLine() )
      NEXT
      FCLOSE( nOutput )
   ENDIF
   ? SECONDS() - nTime
   
   RETURN

PROCEDURE ProcessDir( nOutput, aFiles, cDir )
   LOCAL i, nLen

   ? "Files from ", cDir
   nLen := LEN( aFiles )
   FOR i := 1 TO nLen
      FWRITE( nOutput, "//" )
      FWRITE( nOutput, hb_OSNewLine() )
      FWRITE( nOutput, "//symbols from file: " +Lower(cDir+ PATH_SEPARATOR +aFiles[i][ 1 ] ))
      FWRITE( nOutput, hb_OSNewLine() )
      FWRITE( nOutput, "//" )
      FWRITE( nOutput, hb_OSNewLine() )
      ProcessFile( nOutput, cDir + PATH_SEPARATOR + aFiles[ i ][ 1 ] )
   NEXT

   RETURN

PROCEDURE ProcessFile( nOut, cFile )
   LOCAL nH

   ? cFile
   IF( AT( "INITSYMB.C", UPPER(cFile) ) == 0 )
      nH := FOPEN( cFile )
      IF( nH > 0 )
         FILEEVAL( nH, 255, hb_OSNewLine(), {|c| Processline(nOut, c)} )
         FCLOSE( nH )
      ENDIF
   ENDIF

   RETURN

PROCEDURE ProcessLine( nOut, cLine )
   LOCAL nPos

   nPos := AT( "//", cLine )
   IF nPos > 0 .AND. nPos < 7
      RETURN
   ELSE
      nPos := AT( "*", cLine )
      IF nPos > 0 .AND. nPos < 7
         RETURN
      ENDIF
   ENDIF

   nPos := AT( "HB_FUNC(", cLine )
   IF nPos > 0
      cLine := LTRIM( SUBSTR( cLine, nPos + Len("HB_FUNC(") ) )
      nPos := AT( ")", cLine )
      IF nPos > 0
         cLine := ALLTRIM( Left( cLine, nPos - 1 ) )
         ? cLine
         IF (ISALPHA(cLine) .OR. cLine="_") .AND. ASCAN( aNames, {|c|c==cLine} ) == 0
            AADD( aNames, cLine )
            FWRITE( nOut, "EXTERNAL " +cLine + hb_OSNewLine() )
         ENDIF
      ENDIF
   ELSE
      cLine := UPPER( cLine )
      nPos := AT( "FUNCTION", cLine )
      IF nPos > 0
         IF( AT( "STATIC", cLine ) ==  0 )
            cLine := LTRIM( SUBSTR( cLine, nPos+8 ) )
            nPos := AT( "(", cLine )
            IF nPos > 0
               cLine := ALLTRIM( LEFT( cLine, nPos-1 ) )
               ? cLine
               IF (ISALPHA(cLine) .OR. cLine="_") .AND. !(" " $ cLine) .AND. ASCAN( aNames, {|c|c==cLine} ) == 0
                  AADD( aNames, cLine )
                  FWRITE( nOut, "EXTERNAL " +cLine + hb_OSNewLine() )
               ENDIF
            ENDIF
         ENDIF
      ELSE
         nPos := AT( "PROCEDURE", cLine )
         IF nPos > 0
            IF AT( "STATIC", cLine ) ==  0
               cLine := LTRIM( SUBSTR( cLine, nPos+9 ) )
               nPos := AT( "(", cLine )
               IF nPos > 0
                  cLine :=ALLTRIM( LEFT( cLine, nPos-1 ) )
                  ? cLine
                  IF (ISALPHA(cLine) .OR. cLine="_") .AND. !(" " $ cLine) .AND. ASCAN( aNames, {|c|c==cLine} ) == 0
                     AADD( aNames, cLine )
                     FWRITE( nOut, "EXTERNAL " +cLine + hb_OSNewLine() )
                  ENDIF
               ENDIF
            ENDIF
         ENDIF
      ENDIF
   ENDIF

   RETURN

PROCEDURE FileEval( nHandle, nLineLength, cDelim, bBlock )
   LOCAL cLine

   FSEEK( nHandle, 0 )

   cLine := FReadLn( nHandle, 1, nLineLength, cDelim )
   IF FERROR() != 0
      RETURN
   ENDIF

   DO WHILE !( cLine == "" )

      EVAL( bBlock, cLine )

      cLine := FReadLn( nHandle, 1, nLineLength, cDelim )
      IF FERROR() != 0
         RETURN
      ENDIF

   ENDDO

   RETURN

FUNCTION FReadLn( nHandle, nLines, nLineLength, cDelim )
   LOCAL nCurPos
   LOCAL nFileSize
   LOCAL nChrsToRead
   LOCAL nChrsRead
   LOCAL cBuffer
   LOCAL cLines
   LOCAL nCount
   LOCAL nEOLPos

   nCurPos   := FSEEK( nHandle, 0, FS_RELATIVE )
   nFileSize := FSEEK( nHandle, 0, FS_END )
   FSEEK( nHandle, nCurPos )

   nChrsToRead := MIN( nLineLength, nFileSize - nCurPos )

   cLines  := ""
   nCount  := 1
   DO WHILE nCount <= nLines .AND. nChrsToRead != 0

      cBuffer   := SPACE( nChrsToRead )
      nChrsRead := FREAD( nHandle, @cBuffer, nChrsToRead )

      IF nChrsRead != nChrsToRead
         nChrsToRead := 0
      ENDIF

      nEOLPos := AT( cDelim, cBuffer )

      IF nEOLPos == 0
         cLines  += LEFT( cBuffer, nChrsRead )
         nCurPos += nChrsRead
      ELSE
         cLines  += LEFT( cBuffer, nEOLPos + LEN( cDelim ) - 1 )
         nCurPos += nEOLPos + LEN( cDelim ) - 1
         FSEEK( nHandle, nCurPos, FS_SET )
      ENDIF

      IF ( nFileSize - nCurPos ) < nLineLength
         nChrsToRead := nFileSize - nCurPos
      ENDIF

      nCount++

   ENDDO

   RETURN cLines

