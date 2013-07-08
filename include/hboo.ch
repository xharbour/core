/*
 * $Id: hboo.ch 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Header file for low-level object engine
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
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

/*
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 * Copyright 2000 JF Lefebvre <jfl@mafact.com> and RA Cuylen <rac@mafact.com>
 *    Many enhancements (scopes, class methods)
 *
 * See doc/license.txt for licensing terms.
 *
 */

/* NOTE: This file is also used by C code. */

#ifndef HB_OO_CH_
#define HB_OO_CH_

// Used by ObjFunc.prg (__objGetMsgList) and Classes.c (hb___msgClsSel())
#define HB_MSGLISTALL   0
#define HB_MSGLISTCLASS 1
#define HB_MSGLISTPURE  2

/* Method or Data attribute (nScope)*/
#define HB_OO_CLSTP_EXPORTED    1 /* No comment, default */
#define HB_OO_CLSTP_PUBLISHED   2 /* Filtering scope only */
#define HB_OO_CLSTP_PROTECTED   4 /* Only usable from one of the object's method (even sublclassed object) */
#define HB_OO_CLSTP_HIDDEN      8 /* Only usable from one of the object's method (and not from sublclassed one) */
#define HB_OO_CLSTP_CTOR       16 /* Constructor  (Not yet used) */
#define HB_OO_CLSTP_READONLY   32 /* No comment */
#define HB_OO_CLSTP_SHARED     64 /* Allow a classvar (or classmethod) to be shared by all the subclasses.
                                     Not the default behaviour as each subclass will have its own copy by default. */
#define HB_OO_CLSTP_CLASS     128 /* The related message is a superobject call, uidata is the superclass handle
                                     pInitValue contain one superclass object instance (absolutely needed for Inline msg and class data) */
#define HB_OO_CLSTP_SUPER     256 /* The related message is inherited from a superclass */
#define HB_OO_CLSTP_CLASSCTOR 512

#define HB_OO_CLSTP_SYNC     1024
#define HB_OO_CLSTP_SYMBOL   2048 /* The related pointer function is a PHB_SYMB */
#define HB_OO_CLSTP_PFUNC   32768

#define HB_OO_CLS_INSTANCED         1 /* The class was instanced one time almost */
#define HB_OO_CLS_CLASSCTOR       512
#define HB_OO_CLS_ONERROR_SYMB   2048
#define HB_OO_CLS_DESTRUC_SYMB   4096
#define HB_OO_CLS_REALLOCINIT    8192 /* Realloc pInitValues in AddMsg */
#define HB_OO_CLS_ONERROR_SUPER 16384 /* Onerror handler is inherited */


/* Message types */
#define HB_OO_MSG_METHOD        0
#define HB_OO_MSG_DATA          1
#define HB_OO_MSG_CLASSDATA     2
#define HB_OO_MSG_INLINE        3
#define HB_OO_MSG_VIRTUAL       4
#define HB_OO_MSG_SUPER         5
#define HB_OO_MSG_ONERROR       6
#define HB_OO_MSG_DESTRUCTOR    7

#define HB_OO_PROPERTY          8
#define HB_OO_MSG_PROPERTY      HB_OO_MSG_DATA      + HB_OO_PROPERTY // Auto management of DATA.
#define HB_OO_MSG_CLASSPROPERTY HB_OO_MSG_CLASSDATA + HB_OO_PROPERTY // Auto management of CLASSDATA.
#define HB_OO_MSG_DELEGATE     11

/* Data */
#define HB_OO_DATA_SYMBOL       1
#define HB_OO_DATA_VALUE        2
#define HB_OO_DATA_SYMBOL_PTR   2
#define HB_OO_DATA_TYPE         3
#define HB_OO_DATA_SCOPE        4
#define HB_OO_DATA_PERSISTENT   5

/* ClassData */
#define HB_OO_CLSD_SYMBOL       1
#define HB_OO_CLSD_VALUE        2
#define HB_OO_CLSD_TYPE         3
#define HB_OO_CLSD_SCOPE        4

/* Method */
#define HB_OO_MTHD_SYMBOL       1
#define HB_OO_MTHD_PFUNCTION    2
#define HB_OO_MTHD_DELEGNAME    2
#define HB_OO_MTHD_SCOPE        3
#define HB_OO_MTHD_PERSISTENT   4
#define HB_OO_MTHD_DELEGOBJ     5

/* ClassMethod */ /* for the future */
#define HB_OO_CLSM_SYMBOL       1
#define HB_OO_CLSM_PFUNCTION    2
#define HB_OO_CLSM_SCOPE        3

#define HB_OO_MCLSCTOR_INSTANCE 1
#define HB_OO_MCLSCTOR_CLONE    2

#define FOREACH_BEGIN           1
#define FOREACH_ENUMERATE       2
#define FOREACH_END             3

#endif /* HB_OO_CH_ */
