/*
 * $Id: hvmall.c 9727 2012-10-02 20:04:30Z andijahja $
 */

/*
 * Harbour Project source code:
 *    common file with all HVM functions for compilers which can improve
 *    speed automatically inlining functions
 *
 * Copyright 2009 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
 * www - http://harbour-project.org
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

#define _HB_STACK_LOCAL_MACROS_

#  if ! defined( HB_VM_ALL )
      #define HB_VM_ALL
#  endif

#include "hbvmopt.h"
#include "hbstack.h"
#include "thread.h"

#  if defined( HB_THREAD_SUPPORT )
static DWORD      hb_dwCurrentStack;
static HB_STACK   hb_stackMT;
#  else
static HB_STACK   hb_stackST;
#  endif

static BOOL       hb_stack_ready = FALSE;

#include "hvm.c"
#include "itemapi.c"
#include "fastitem.c"
#include "hash.c"
#include "arrays.c"
#include "arrayshb.c"
#include "asort.c"
#include "classes.c"
#include "classesc.c"
#include "codebloc.c"
#include "dynsym.c"
#include "macro.c"
#include "set.c"
#include "memvars.c"
#include "thread.c"
#include "garbage.c"
#include "fm.c"
#include "strapi.c"
#include "extend.c"
#include "debug.c"
#include "do.c"
#include "dynlibhb.c"
#include "errorapi.c"
#include "eval.c"
#include "evalhb.c"
#include "initexit.c"
#include "pcount.c"
#include "proc.c"
#include "pvalue.c"
#include "estack.c"
