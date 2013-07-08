/******************************************
* TIP test
* HTTP Advanced operations Test
*
* $Id: httpadv.prg 9279 2011-02-14 18:06:32Z druzus $
*****/

PROCEDURE MAIN( cUrl )
   LOCAL oCon, oUrl, i

   oUrl := tURL():New( cUrl )
   IF Empty( oUrl )
      ? "Invalid url " + cUrl
      ?
      QUIT
   ENDIF

   IF oUrl:cProto != "http"
      ? 'This is a header test for http.'
      ? 'Use an http address.'
      ?
      QUIT
   END

   oCon := TipClientHttp():New( oUrl )
   oCon:nConnTimeout := 20000
   ? "Connecting with", oUrl:cServer
   IF oCon:Open( cUrl )
      ? "Connection eshtablished"
      ? "Retreiving", oUrl:cPath, oUrl:cFile, oUrl:cQuery

      IF oCon:Get( oUrl:cPath )
         ? "Get Sucessful"
         FOR i := 1 to Len( oCon:hHeaders )
            ? HGetkeyAt( oCon:hHeaders, i ) +":", HGetValueAt( oCon:hHeaders, i )
         NEXT
      ELSE
         ? "Get failure (server reply:", oCon:cReply , ")"
      ENDIF

      oCon:Close()
   ELSE
      ? "Can't connect with", oUrl:cServer
      IF oCon:SocketCon == NIL
         ? "Connection not initiated"
      ELSEIF InetErrorCode( oCon:SocketCon ) == 0
         ? "Server sayed:", oCon:cReply
      ELSE
         ? "Error in connection:", InetErrorDesc( oCon:SocketCon )
      ENDIF
   END

   ? "Done"
   ?
RETURN
