/*
 * $Id: msghe862.c 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Language Support Module (HE862)
 *
 * Copyright 2000 Viktor Szakats <viktor.szakats@syenar.hu> (English, from msgen.c)
 * Copyright 2000 Chen Kedem <niki@actcom.co.il> (Hebrew)
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

/* Language name: Hebrew - Dos     */
/* ISO language code (2 chars): HE */
/* Codepage: 862                   */

#include "hbapilng.h"

static HB_LANG s_lang =
{
   {
      /* Identification */

      "HE862",                     /* ID */
      "Hebrew",                    /* Name (in English) */
      "�����",                     /* Name (in native language) */
      "HE",                        /* RFC ID */
      "862",                       /* Codepage */
      "$Revision: 9279 $ $Date: 2011-02-14 16:06:32 -0200 (seg, 14 fev 2011) $",         /* Version */

      /* Month names */

      "�����",
      "������",
      "���",
      "�����",
      "���",
      "����",
      "����",
      "������",
      "������",
      "�������",
      "������",
      "�����",

      /* Day names */

      "�����",
      "���",
      "�����",
      "�����",
      "�����",
      "����",
      "���",

      /* CA-Cl*pper compatible natmsg items */

      "������ ����       ������ \'��   ����� �����     ����",
      "? ������ ������� ������ ���",
      "\'�� ����",
      "** ������ ����� **",
      "* ����� ����� *",
      "*** �\"�� ***",
      "Ins",
      "   ",
      "���� �����",
      "���� ����: ",
      " - ",
      "Y/N",
      "INVALID EXPRESSION",

      /* Error description names */

      "����� �� �����",
      "Argument error",
      "Bound error",
      "String overflow",
      "Numeric overflow",
      "���� �����",
      "Numeric error",
      "����� �����",
      "Operation too complex",
      "",
      "",
      "����� ����� ���",
      "������ �� �������",
      "No exported method",
      "���� �� �����",
      "Alias does not exist",
      "No exported variable",
      "Illegal characters in alias",
      "Alias already in use",
      "",
      "���� ����� ���� �����",
      "����� ���� �����",
      "����� ���� �����",
      "����� ���� �����",
      "����� ���� �����",
      "����� �����",
      "",
      "",
      "",
      "",
      "����� ���� ��� �����",
      "Limit exceeded",
      "���� �� �� ����� �����",
      "Data type error",
      "Data width error",
      "Workarea not in use",
      "Workarea not indexed",
      "Exclusive required",
      "����� �����",
      "����� ����� �����",
      "Append lock failed",
      "����� ����� �����",
      "",
      "",
      "",
      "",
      "������� �� ���� ����",
      "����� ����",
      "array assign",
      "���� ��",
      "conditional",
      "Invalid self",

      /* Internal error names */

      "Unrecoverable error %lu: ",
      "Error recovery failure",
      "No ERRORBLOCK() for error",
      "Too many recursive error handler calls",
      "RDD invalid or failed to load",
      "Invalid method type from %s",
      "hb_xgrab can't allocate memory",
      "hb_xrealloc called with a NULL pointer",
      "hb_xrealloc called with an invalid pointer",
      "hb_xrealloc can't reallocate memory",
      "hb_xfree called with an invalid pointer",
      "hb_xfree called with a NULL pointer",
      "Can\'t locate the starting procedure: \'%s\'",
      "No starting procedure",
      "Unsupported VM opcode",
      "Symbol item expected from %s",
      "Invalid symbol type for self from %s",
      "Codeblock expected from %s",
      "Incorrect item type on the stack trying to pop from %s",
      "Stack underflow",
      "An item was going to be copied to itself from %s",
      "Invalid symbol item passed as memvar %s",
      "Memory buffer overflow",
      "hb_xgrab requested to allocate zero bytes",
      "hb_xrealloc requested to resize to zero bytes",
      "hb_xalloc requested to allocate zero bytes",

      /* Texts */

      "DD-MM-YYYY",
      "Y",
      "N"
   }
};

#define HB_LANG_ID  HE862
#include "hbmsgreg.h"
