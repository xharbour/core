/*
 * $Id: encode.prg 9892 2012-12-24 20:48:13Z andijahja $
 */

/*
 * Copyright 2009 Viktor Szakats (harbour syenar.net)
 * www - http://harbour-project.org
 */

#message "/ ----------------------------------------------------\"
#message "| This program requires:                              |"
#message "|   hbssl.lib                                         |"
#message "|   libeay32*.lib                                     |"
#message "|   ssleay32*.lib                                     |"
#message "| URL: http://slproweb.com/products/Win32OpenSSL.html |"
#message "\-----------------------------------------------------/ "

#include "hbssl.ch"

PROCEDURE Main()

   LOCAL ctx
   LOCAL result
   LOCAL encrypted
   LOCAL decrypted

   SSL_init()

   OpenSSL_add_all_ciphers()

   ctx := hb_EVP_ENCODE_ctx_create()

   EVP_EncodeInit( ctx )

   encrypted := ""
   result := ""
   EVP_EncodeUpdate( ctx, @result, "sample text" )
   encrypted += result
   EVP_EncodeFinal( ctx, @result )
   encrypted += result
   ? "ENCRYTPTED", ">" + encrypted + "<"

   ctx := hb_EVP_ENCODE_ctx_create()

   EVP_DecodeInit( ctx )

   decrypted := ""
   result := ""
   EVP_DecodeUpdate( ctx, @result, encrypted )
   decrypted += result
   EVP_DecodeFinal( ctx, @result )
   decrypted += result
   ? "DECRYTPTED", ">" + decrypted + "<"

   RETURN
