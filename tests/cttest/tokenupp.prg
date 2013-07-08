/*
 * $Id: tokenupp.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 *   Test CT3 function TOKENUPPER() 
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

local cStr := ".,.This.,.is.,.a.,.test!"

 ctinit()

 qout ("Begin test of TOKENUPPER()")
 qout ("")

 // Some simple tests
 qout ("  Simple tests:")
 qout ([   tokenupper("Hello, world, here I am!")       == "Hello, World, Here I Am!" ?])
 qout ([                                                -> "] + tokenupper ("Hello, world, here I am!") + ["])
 qout ([   tokenupper("Hello, world, here I am!",,3)    == "Hello, World, Here I am!" ?])                                                    
 qout ([                                                -> "] + tokenupper ("Hello, world, here I am!",,3) + ["])
 qout ([   tokenupper("Hello, world, here I am!",",",3) == "Hello, world, here I am!" ?])                                                    
 qout ([                                                -> "] + tokenupper ("Hello, world, here I am!",",",3) + ["])
 qout ([   tokenupper("Hello, world, here I am!"," w")  == "Hello, wOrld, Here I Am!" ?])                                                    
 qout ([                                                -> "] + tokenupper ("Hello, world, here I am!"," w") + ["])
 qout ("")

 qout ([  Uppercase the tokens in the string "]+cStr+["])
 qout ([            with csetref (.T.) and "@"])
 csetref (.T.)
 qout ("")
 qout ([    --> return value of tokenupper (@cStr): ],tokenupper(@cStr))
 qout ([    --> cStr is now: "]+cStr+["])

 qout ("")
 qout ("End test of TOKENUPPER()")
 qout ()

 ctexit()

return 



