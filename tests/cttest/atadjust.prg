/*
 * $Id: atadjust.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 *   Test CT3 function ATADJUST() 
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

local aStr := {"Introduction: 1",;
               "Theory: 5",;
               "Material and Methods: 13",;
               "Results: 19",;
               "Discussion: 21"}
local ni

 ctinit()

 // Some simple tests
 qout ("Begin test of ATADJUST()")
 qout ("")

 qout ("  Original strings:")
 for ni := 1 to len (aStr)
   qout (space(4)+aStr[ni])
 next ni
 qout ("")

 qout ('  ATADJUST (":", aStr[ni], 21,,, ".") yields')
 for ni := 1 to len (aStr)
   qout (space(4)+atadjust (":", aStr[ni], 21, 1,,"."))
 next ni
 qout ("")

 qout ('  ATADJUST (":", aStr[ni], 10, 1,, ".") yields')
 for ni := 1 to len (aStr)
   qout (space(4)+atadjust (":", aStr[ni], 10, 1,,"."))
 next ni
 qout ("")

 qout ('  SETATLIKE (CT_SETATLIKE_WILDCARD, ":")')
 qout ('  ATADJUST (":", aStr[ni], 10, 1,, ".") yields now')
 setatlike (CT_SETATLIKE_WILDCARD, ":")
 for ni := 1 to len (aStr)
   qout (space(4)+atadjust (":", aStr[ni], 10, 1,,"."))
 next ni
 qout ("")

 qout ("End test of ATADJUST()")
 qout ("")

 ctexit()

return 




