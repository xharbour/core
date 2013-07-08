/*
 * $Id: charsort.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 *   Test CT3 function CHARSORT() 
 *
 * Copyright 2001 IntTec GmbH, Neunlindenstr 32, 79106 Freiburg, Germany
 *        Author: Martin Vogel <vogel@inttec.de>
 *
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


#include "ct.ch"


procedure main

 ctinit()

 qout ("Begin test of CHARSORT()")
 qout ("")

 // simple tests
 qout ("Simple tests:")
 qout ([  charsort("qwert")                     == "eqrtw"      ? --> "] + charsort("qwert")                   + ["])
 qout ([  charsort("qwert", 2)                  == "erqwt"      ? --> "] + charsort("qwert", 2)                + ["])
 qout ([  charsort("b1a4a3a2a1", 2, 1)          == "a2a1a3a4b1" ? --> "] + charsort("b1a4a3a2a1", 2, 1)        + ["])
 qout ([  NOTE : The order of equal elements (here the one beginning with the same char) is NOT determined !])
 qout ([  charsort("XXXqwert", 1, 1, 3)         == "XXXeqrtw"   ? --> "] + charsort("XXXqwert", 1, 1, 3)       + ["])
 qout ([  charsort("b1a4a3a2a1", 2, 1, 0, 1)    == "a1b1a2a3a4" ? --> "] + charsort("b1a4a3a2a1", 2, 1, 0, 1)  + ["])
 qout ([  NOTE : The order of equal elements (here the one ending with the same number) is NOT determined !])
 qout ([  charsort("384172852", 1, 1, 0, 0, 4)  == "134872852"  ? --> "] + charsort("384172852", 1, 1, 0, 0, 4)+ ["])
 qout ([  charsort("qwert",,,,,,.T.)            == "wtrqe"      ? --> "] + charsort("qwert",,,,,,.T.)          + ["])
 qout ("")

 qout ("End test of CHARSORT()")
 qout ("")

 ctexit()

return





