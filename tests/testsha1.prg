/*
 * $Id: testsha1.prg 9608 2012-07-28 22:13:31Z andijahja $
 */

/*
 * Harbour Project source code:
 *
 * Rewritten from C: Viktor Szakats (harbour syenar.net)
 * www - http://harbour-project.org
 */

PROCEDURE Main()

   ? ">" + hb_sha1( "hello" ) + "<"
   ? ">" + hb_sha1( "hello", .F. ) + "<"
   ? ">" + hb_sha1( "hello", .T. ) + "<"

   ? ">" + hb_hmac_sha1( "hello", "key" ) + "<"
   ? ">" + hb_hmac_sha1( "hello", "key", .F. ) + "<"
   ? ">" + hb_hmac_sha1( "hello", "key", .T. ) + "<"

   RETURN
