#!/usr/bin/sh
#
# $Id: make_hp.sh 9281 2011-02-14 18:28:11Z druzus $
#

# ---------------------------------------------------------------
# Template to initialize the environment before starting
# the GNU make system for Harbour
#
# For further information about the GNU make system please
# check doc/gmake.txt
#
# Copyright 1999-2001 Viktor Szakats (viktor.szakats@syenar.hu)
# See doc/license.txt for licensing terms.
# ---------------------------------------------------------------

if [ -z "$HB_ARCHITECTURE" ]; then export HB_ARCHITECTURE=hpux; fi
if [ -z "$HB_COMPILER" ]; then export HB_COMPILER=gcc; fi
if [ -z "$HB_GPM_MOUSE" ]; then export HB_GPM_MOUSE=no; fi
if [ -z "$HB_GT_LIB" ]; then export HB_GT_LIB=gtsln; fi
if [ -z "$HB_MULTI_GT" ]; then export HB_MULTI_GT=yes; fi
if [ -z "$HB_MT" ]; then export HB_MT=no; fi

# export PRG_USR=
# export C_USR=
# export L_USR=

# Set to constant value to be consistent with the non-GNU make files.

if [ -z "$HB_BIN_INSTALL" ]; then export HB_BIN_INSTALL=$(pwd)/bin/; fi
if [ -z "$HB_LIB_INSTALL" ]; then export HB_LIB_INSTALL=$(pwd)/lib/; fi
if [ -z "$HB_INC_INSTALL" ]; then export HB_INC_INSTALL=$(pwd)/include/; fi

if [ -z "$HB_ARCHITECTURE" ]; then
   echo "Error: HB_ARCHITECTURE is not set."
fi
if [ -z "$HB_COMPILER" ]; then
   echo "Error: HB_COMPILER is not set."
fi

if [ -z "$HB_ARCHITECTURE" ] || [ -z "$HB_COMPILER" ]; then

   echo
   echo "Usage: make_gnu.sh [command]"
   echo
   echo "The following commands are supported:"
   echo "  - all (default)"
   echo "  - clean"
   echo "  - install"
   echo
   echo "Notes:"
   echo
   echo "  - HB_ARCHITECTURE and HB_COMPILER envvars must be set."
   echo "    The following values are currently supported:"
   echo
   echo "    HB_ARCHITECTURE:"
   echo "      - dos   (HB_GT_LIB=gtdos by default)"
   echo "      - w32   (HB_GT_LIB=gtw32 by default)"
   echo "      - linux (HB_GT_LIB=gtstd by default)"
   echo "      - os2   (HB_GT_LIB=gtos2 by default)"
   echo
   read
   echo "    HB_COMPILER:"
   echo "      - When HB_ARCHITECTURE=dos"
   echo "        - bcc16   (Borland C++ 3.x, 4.x, 5.0x, DOS 16-bit)"
   echo "        - djgpp   (Delorie GNU C, DOS 32-bit)"
   echo "        - rxs32   (EMX/RSXNT/DOS GNU C, DOS 32-bit)"
   echo "        - watcom  (Watcom C++ 9.x, 10.x, 11.x, DOS 32-bit)"
   echo "      - When HB_ARCHITECTURE=w32"
   echo "        - bcc32   (Borland C++ 4.x, 5.x, Windows 32-bit)"
   echo "        - gcc     (Cygnus/Cygwin GNU C, Windows 32-bit)"
   echo "        - mingw32 (Cygnus/Mingw32 GNU C, Windows 32-bit)"
   echo "        - rxsnt   (EMX/RSXNT/Win32 GNU C, Windows 32-bit)"
   echo "        - icc     (IBM Visual Age C++, Windows 32-bit)"
   echo "        - msvc    (Microsoft Visual C++, Windows 32-bit)"
   echo "      - When HB_ARCHITECTURE=linux"
   echo "        - gcc     (GNU C, 32-bit)"
   echo "      - When HB_ARCHITECTURE=os2"
   echo "        - gcc     (EMX GNU C, OS/2 32-bit)"
   echo "        - icc     (IBM Visual Age C++ 3.0, OS/2 32-bit)"
   echo
   read
   echo "    HB_GT_LIB:"
   echo "      - gtstd (Standard streaming) (for all architectures)"
   echo "      - gtdos (DOS console)        (for dos architecture)"
   echo "      - gtwin (Win32 console)      (for w32 architecture)"
   echo "      - gtwvt (Win32 win console)  (for w32 architecture)"
   echo "      - gtos2 (OS/2 console)       (for os2 architecture)"
   echo "      - gtpca (PC ANSI console)    (for all architectures)"
   echo "      - gtcrs (Curses console)     (for linux, w32 architectures)"
   echo "      - gtsln (Slang console)      (for linux, w32 architectures)"
   echo "      - gtxvt (XWindow console)    (for linux architecture)"
   echo
   echo "  - Use these optional envvars to configure the make process"
   echo "    when using the 'all' command:"
   echo
   echo "    PRG_USR - Extra Harbour compiler options"
   echo "    C_USR   - Extra C compiler options"
   echo "    L_USR   - Extra linker options"
   exit

else

   # ---------------------------------------------------------------
   # Start the GNU make system
   make $*

   if [ "$*" = "clean" ]; then
      find . -type d -name "$HB_ARCHITECTURE" | xargs rmdir 2> /dev/null
   fi
fi
