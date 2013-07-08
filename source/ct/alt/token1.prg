/*
 * $Id: token1.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * NUMTOKEN() and TOKEN() CA-Tools function
 *
 * Copyright 2000 Phil Barnett <philb@iag.net>
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

#include "common.ch"

// This is used to get a count of how many tokens are in a string based
// on the supplied delimiter. This defaults to the same delimiters as
// used in the like named function in Clipper Tools III, so it should
// work identically.

function NUMTOKEN( cString, cDelimiter )

   local x
   local nStrLen    := len( cString )
   local nHowMany   := 0
   local lFoundWord := .F.
   local retval     := 0

   DEFAULT cDelimiter TO ' ,.;:!?/\<>()^#&%+-*' + chr( 0 ) + chr( 9 ) + ;
           chr( 10 ) + chr( 13 ) + ;
           chr( 26 ) + chr( 138 ) + ;
           chr( 141 )

   for x := 1 to nStrLen
      if substr( cString, x, 1 ) $ cDelimiter
         if lFoundWord
            nHowMany++
         endif
         do while x < nStrLen .and. substr( cString, x + 1, 1 ) $ cDelimiter
            x++
         enddo
      else
         lFoundWord := .T.
      endif
   next

   if nStrLen > 0
      retval := iif( right( cString, 1 ) $ cDelimiter, nHowmany, nHowMany + 1 )
   endif

   return retval

// This is used to extract each token from the string based on the
// delimiter and the numeric pointer which tells us which token to
// return.

// This defaults to the same delimiters as used in the like named
// function in Clipper Tools III, so it should work identically.

function TOKEN( cString, cDelimiter, nPointer )

   local x
   local nStrLen      := len( cString )
   local nHowMany     := 0
   local nLastPointer := 0
   local cPart
   local lFoundWord   := .F.

   DEFAULT cDelimiter TO ' ,.;:!?/\<>()^#&%+-*' + chr( 0 ) + chr( 9 ) + ;
           chr( 10 ) + chr( 13 ) + ;
           chr( 26 ) + chr( 138 ) + ;
           chr( 141 )

   for x := 1 to nStrLen
      if substr( cString, x, 1 ) $ cDelimiter
         if lFoundWord
            nHowMany++
         endif
         if nHowMany == nPointer
            exit
         endif
         do while x < nStrLen .and. substr( cString, x + 1, 1 ) $ cDelimiter
            x++
         enddo
         nLastPointer := x
      else
         lFoundWord := .T.
      endif
   next

   // went all the way without nHowmany == nPointer
   if x == nStrLen + 1
      // take the last word
      cPart := substr( cString, nLastPointer + 1 )
   else
      cPart := substr( cString, nLastPointer + 1, ( x - nLastPointer ) - 1 )
   endif

   return cPart

