/*
 * $Id: hbstdgen.ch 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * 
 *
 * Copyright 2006 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
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

/* include hbsetup.ch to set default build settings */
#include "hbsetup.ch"
/* include std.ch to set default rules */
#include "std.ch"

/* undefine all rules which are not necessary in default PP tables */
#undef HB_SETUP_CH_

#undef HB_EXTENSION
#undef HB_EXT_INKEY

#undef HB_C52_UNDOC
#undef HB_C52_STRICT

#undef HB_COMPAT_C53
#undef HB_COMPAT_XPP
#undef HB_COMPAT_VO
#undef HB_COMPAT_FLAGSHIP
#undef HB_COMPAT_FOXPRO
#undef HB_COMPAT_DBASE
#undef HB_COMPAT_CLIP
#undef HB_COMPAT_XHB

#undef HB_SHORTNAMES
#undef HB_FILE_VER_STATIC

#undef __HARBOUR__
#undef __DATE__
#undef __TIME__
#undef __HB_MAIN__
#undef __ARCH16BIT__
#undef __ARCH32BIT__
#undef __ARCH64BIT__
#undef __LITTLE_ENDIAN__
#undef __BIG_ENDIAN__
#undef __PDP_ENDIAN__
