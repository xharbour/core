/******************************************
* TIP test
* Mail - reading and writing multipart mails
*
* Test for reading a multipart message (that must already
* be in its canonical form, that is, line terminator is
* CRLF and it must have no headers other than SMTP/Mime).
*
* This test writes data to standard output, and is
* compiled only under GTCGI;
*
* $Id: tipmail.prg 9279 2011-02-14 18:06:32Z druzus $
*****/

PROCEDURE MAIN( cFileName )
   LOCAL oMail, cData, i

   IF cFileName != NIL
      cData := MemoRead(cFileName)
      IF Ferror() > 0
         ? "Can't open", cFileName
         QUIT
      ENDIF
   ENDIF
   oMail := TipMail():New()
   IF oMail:FromString( cData ) == 0
      ? "Malformed mail. Dumping up to where parsed"
   ENDIF

   WITH OBJECT oMail
      ? "-------------============== HEADERS =================--------------"
      FOR i := 1 TO Len( :hHeaders )
         ? HGetKeyAt( :hHeaders, i ), ":", HGetValueAt( :hHeaders, i )
      NEXT
      ?

      ? "-------------============== RECEIVED =================--------------"
      FOR EACH cData IN :aReceived
         ? cData
      NEXT
      ?

      ? "-------------============== BODY =================--------------"
      ? :GetBody()
      ?

      DO WHILE :GetAttachment() != NIL
         ? "-------------============== ATTACHMENT =================--------------"
         ? :NextAttachment():GetBody()
         ?
      ENDDO

   END

   ? "DONE"
   ?
   /* Writing stream */
   //FWrite( 1, oMail:ToString() )
RETURN
