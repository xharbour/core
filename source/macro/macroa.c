/*
 * $Id: macroa.c 9722 2012-10-02 06:13:24Z andijahja $
 */

/* hbexpra.c is also included from ../compiler/expropta.c
 * However it produces a slighty different code if used in
 * macro compiler (there is an additional parameter passed to some functions)
 * 1.34 - ignore this magic number - this is used to force compilation
 */

#define HB_MACRO_SUPPORT

#include "hbmacro.h"
#include "hbcomp.h"

#include "hbexpra.c"
