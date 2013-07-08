/*
 * $Id: _hbconf.h 9827 2012-11-17 13:21:47Z andijahja $
 */

#ifndef _HBCONF_H
#define _HBCONF_H

#include "hbdefs.h"
/* 1234 = LITLE_ENDIAN, 4321 = BIG_ENDIAN */
#if defined( HB_LITTLE_ENDIAN )
#  define BYTEORDER 1234
#elif defined( HB_BIG_ENDIAN )
#  define BYTEORDER 4321
#else
#  error Unsupported machine byte order (endian).
#endif

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to specify how much context to retain around the current parse
   point. */
#define XML_CONTEXT_BYTES 1024

/* Define to make parameter entity parsing functionality available. */
#define XML_DTD 1

/* Define to make XML Namespaces functionality available. */
#define XML_NS 1

/* Build static library. */
#define XML_STATIC 1

#if defined( __BORLANDC__ )
   #pragma warn -csu
#elif defined( __WATCOMC__ )
   #pragma disable_message ( 201 )
#endif

#endif /* _HBCONF_H */
