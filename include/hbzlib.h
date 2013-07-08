/*
 * $Id: hbzlib.h 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * xHarbour Zlib Interface functions
 *
 * www - http://www.xharbour.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version, with one exception:
 *
 * The exception is that if you link the Harbour Runtime Library (HRL)
 * and/or the Harbour Virtual Machine (HVM) with other files to produce
 * an executable, this does not by itself cause the resulting executable
 * to be covered by the GNU General Public License. Your use of that
 * executable is in no way restricted on account of linking the HRL
 * and/or HVM code into it.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA (or visit
 * their web site at http://www.gnu.org/).
 *
 */

#ifndef __HBZLIB_INCLUDED
#define __HBZLIB_INCLUDED

#if ( defined(__XCC__) || defined(__LCC__) )
   #include "source\rtl\zlib\zlib.h"
#else
   #include "../source/rtl/zlib/zlib.h"
#endif

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"

#endif /* __HBZLIB_INCLUDED */
