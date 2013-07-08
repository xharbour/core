/*
 * Harbour Project source code:
 *
 *   Test CT3 Numeric functions - PART 1
 *
 * - CELSIUS
 * - FAHRENHEIT
 * - INFINITY
 *
 * Copyright 2001   Alejandro de Garate  <alex_degarate@hotmail.com>
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



 PROCEDURE MAIN

 CTINIT()

 SET DECIMAL TO 14
 CLS
 ?? "Test of Numeric functions - part 1"

 ? "CELSIUS( 33.8 ) =   1             // CT3"
 ?  SPACE(10),  CELSIUS( 33.8 ), " <-- CT for Harbour "
 ?

 ? "CELSIUS( 338.0 ) = 170             // CT3"
 ?  SPACE(11),  CELSIUS( 338.0 ), " <-- CT for Harbour "
 ?

 ? "CELSIUS( 3380.0) = 1860             // CT3"
 ?  SPACE(12),  CELSIUS( 3380.0), " <-- CT for Harbour "
 ?

 ? "CELSIUS( -33.8) = -36.5555..      // CT3"
 ?  SPACE(10), CELSIUS( -33.8), " <-- CT for Harbour "
 ?

 ? "FAHRENHEIT( 12.5 ) = 54.5           // CT3"
 ?  SPACE(12), FAHRENHEIT( 12.5 ), " <-- CT for Harbour "
 ?

 ? "FAHRENHEIT( 125 ) = 257             // CT3"
 ?  SPACE(12), FAHRENHEIT( 125 ), " <-- CT for Harbour "
 ?

 ? "FAHRENHEIT( 1250 ) = 2282             // CT3"
 ?  SPACE(14), FAHRENHEIT( 1250 ), " <-- CT for Harbour "
 ?
 ? "FAHRENHEIT( -155 ) = -247             // CT3"
 ?  SPACE(14), FAHRENHEIT( -155 ), " <-- CT for Harbour "
 ?
 ? "   PRESS ANY KEY"

 INKEY(0)
 

 ?  "INFINITY()       // CT3"
 ?  SPACE(8), STR( INFINITY(), 30, 15), "  <-- CT for Harbour"
 ?

 CTEXIT()

RETURN
