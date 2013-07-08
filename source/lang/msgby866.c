/*
 * $Id: msgby866.c 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Language Support Module (BY866)
 *
 * Copyright 2004 Pavel Tsarenko <tpe2@mail.ru>
 * www - http://www.xharbour.org
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

/* Language name: Belorussian */
/* ISO language code (2 chars): BY */
/* Codepage: 866 */

#include "hbapilng.h"

static HB_LANG s_lang =
{
   {
      /* Identification */

      "BY866",                     /* ID */
      "Belorussian",               /* Name (in English) */
      "������᪠� ",               /* Name (in native language) */
      "BY",                        /* RFC ID */
      "866",                      /* Codepage */
      "$Revision: 9279 $ $Date: 2011-02-14 16:06:32 -0200 (seg, 14 fev 2011) $",         /* Version */

      /* Month names */

      "��㤧���",
      "����",
      "�����i�",
      "��ᠢi�",
      "���",
      "��ࢥ��",
      "�i����",
      "��i����",
      "���ᥭ�",
      "�������i�",
      "�i�⠯��",
      "�������",

      /* Day names */

      "�蠟���",
      "���蠟����",
      "���ࠪ",
      "��ࠤ�",
      "��梥�",
      "���i�",
      "�㡮�",

      /* CA-Cl*pper compatible natmsg items */

      "����� ��������    # ���i��     ����� ��.     �����",
      "������� ���� ��몫��� ?",
      "���.N",
      "** Subtotal **",
      "* Subsubtotal *",
      "*** Total ***",
      "Ins",
      "   ",
      "��ࠢi�쭠� ���",
      "��ﯠ���: ",
      " - ",
      "�/�",
      "������I���� �����",

      /* Error description names */

      "���冷��� ���뫪�",
      "��ࠢi��� ��㬥��",
      "��࠯������� ���i��",
      "��࠯������� ࠤ��",
      "��࠯������� �i��",
      "��﫥��� �� ���",
      "���쪠᭠� ���뫪�",
      "�i�⠪�i筠� ���뫪�",
      "������ ᪫������ �������",
      "",
      "",
      "�ࠪ� �����i",
      "���冷��� �㭪���",
      "��⠤ �� �ᯠ�⠢��",
      "��ࠬ����� �� i��",
      "��i�� �� i��",
      "��ࠬ����� �� �ᯠࠢ���",
      "����묠���� �i����� � i��i ��i��",
      "��i�� 㦮 �몠����������",
      "",
      "���뫪� �⢮࠭��",
      "���뫪� ��������",
      "���뫪� ���뭥���",
      "���뫪� ��⠭��",
      "���뫪� ���i��",
      "���뫪� ��㪠",
      "",
      "",
      "",
      "",
      "������� �� �����묫i�����",
      "�裂 ��ࠢ�蠭�",
      "�������� ��誮������",
      "���뫪� ��� ��������",
      "���뫪� ������ ��������",
      "���� �� ������",
      "���� �� i����iࠢ��",
      "���ࠡ��� �᪫�i��� �����",
      "���ࠡ��� ��������",
      "���i� �� ���������",
      "���� �������i ��� ������",
      "�������� �� ���묠����",
      "",
      "",
      "",
      "",
      "��ࠢi�쭠� ���쪠��� ��㬥���",
      "����� �� ���i��",
      "������� ���i�",
      "�� ���i�",
      "���������",
      "Invalid self",


      /* Internal error names */

      "����������� ���뫪� %lu: ",
      "���뫪� ��� ���������i",
      "�� �맭�砭� ERRORBLOCK() ��� ���뫪i",
      "��ࠢ�蠭�� �裂 �����i���� �몫i��� �������몠 ���뫠�",
      "�� ���묮������ �����i�� RDD",
      "��ࠢi��� �� ��⠤� %s",
      "hb_xgrab �� ���� ࠧ��ઠ���� ������",
      "hb_xrealloc �몫i���� � NULL ��������i���",
      "hb_xrealloc �몫i���� � ��ࠢi��� ��������i���",
      "hb_xrealloc �� ���� ���ࠧ��ઠ���� ������",
      "hb_xfree �몫i���� � ��ࠢi��� ��������i���",
      "hb_xfree �몫i���� � NULL ��������i���",
      "�� ���������� ���⠢�� ������: \'%s\'",
      "�����i砥 ���⠢�� ������",
      "VM: ���冷�� ���",
      "%s: 砪���� �i����",
      "%s: ��ࠢi��� �� �i����� ��� self",
      "%s: 砪���� ���� ����",
      "%s: ��ࠢi��� �� ����� �� �����i ���",
      "���� �� ���� ���",
      "%s: �஡� ���iࠢ��� ����� �� �",
      "%s: ��ࠢi�쭠� i�� ��ࠬ�����",
      "��࠯������� ���� �����i",
      "hb_xgrab requested to allocate zero bytes",
      "hb_xrealloc requested to resize to zero bytes",
      "hb_xalloc requested to allocate zero bytes",

      /* Texts */

      "����/��/��",
      "�",
      "�"
   }
};

#define HB_LANG_ID      BY866
#include "hbmsgreg.h"
