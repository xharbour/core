/*
 * $Id: msgua866.c 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Language Support Module (UA866)
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

/* Language name: Ukrainian */
/* ISO language code (2 chars): UA */
/* Codepage: 866 */

#include "hbapilng.h"

static HB_LANG s_lang =
{
   {
      /* Identification */

      "UA866",                     /* ID */
      "Ukrainian",                 /* Name (in English) */
      "������쪠",                /* Name (in native language) */
      "UA",                        /* RFC ID */
      "866",                       /* Codepage */
      "$Revision: 9279 $ $Date: 2011-02-14 16:06:32 -0200 (seg, 14 fev 2011) $",         /* Version */

      /* Month names */

      "�i祭�",
      "��⨩",
      "��१���",
      "��i⥭�",
      "�ࠢ���",
      "��ࢥ��",
      "������",
      "��௥��",
      "���ᥭ�",
      "���⥭�",
      "���⮯��",
      "��㤥��",

      /* Day names */

      "���i��",
      "�����i���",
      "�i��ப",
      "��।�",
      "��⢥�",
      "�'�⭨��",
      "�㡮�",

      /* CA-Cl*pper compatible natmsg items */

      "����� �����       # �����     ��⠭�� ��.     ����i�",
      "����i��i � �ਪ���� ?",
      "���.N",
      "** Subtotal **",
      "* Subsubtotal *",
      "*** Total ***",
      "���",
      "   ",
      "��������� ���",
      "�i������: ",
      " - ",
      "�/�",
      "���������� �����",

      /* Error description names */

      "���i���� �������",
      "���������� ��㬥��",
      "��९������� ��ᨢ�",
      "��९������� �浪�",
      "��९������� �᫠",
      "�i����� �� ���",
      "��᫮�� �������",
      "���⠪�筠 �������",
      "�㦥 ᪫���� �����i�",
      "",
      "",
      "�� ������ ���'��i",
      "���i���� �㭪�i�",
      "��⮤ �� ��ᯮ�⮢����",
      "��i��� �� i���",
      "��i�� �� i���",
      "��i��� �� ��ᯮ�⮢���",
      "�������⨬i ᨬ���� � i���i ��i��",
      "��i�� ��� ������⮢������",
      "",
      "������� �i� �� �⢮७��",
      "������� �i� �� �i������",
      "������� �i� �� �������",
      "������� �i� �� �⠭��",
      "������� �i� �� ������",
      "������� �i� �� ����",
      "",
      "",
      "",
      "",
      "�����i� �� �i��ਬ������",
      "�i�i� ��ॢ�饭�",
      "����� ��誮������",
      "������� � ⨯i �����",
      "������� � ஧�i�i �����",
      "���� �� �i���⨩",
      "���� �� ��i����ᮢ����",
      "����i��� ��᪫���� �����",
      "����i��� ����㢠���",
      "����� �� ����஭���",
      "��i� ����㢠��� �i� �� ��������� ������",
      "������㢠� �� �������",
      "",
      "",
      "",
      "",
      "��������� �i��i��� ��㬥��i�",
      "����� �� ��ᨢ�",
      "��᢮��� ��ᨢ�",
      "�� ��ᨢ",
      "���i��ﭭ�",
      "Invalid self",


      /* Internal error names */

      "�����ࠢ�� ������� %lu: ",
      "������� �i� �� �i���������",
      "�� �����祭� ERRORBLOCK() ��� �������",
      "��ॢ�饭� ���� ४��ᨢ��� ������i� ��஡���� �������",
      "�� �������� �����⠦�� RDD",
      "���������� ⨯ ��⮤� %s",
      "hb_xgrab �� ���� ஧���i��� ���'���",
      "hb_xrealloc ��������� � NULL ����i������",
      "hb_xrealloc ��������� � ���������� ����i������",
      "hb_xrealloc �� ���� ���஧���i��� ���'���",
      "hb_xfree ��������� � ���������� ����i������",
      "hb_xfree ��������� � NULL ����i������",
      "�� �������� ���⮢� ��楤��: \'%s\'",
      "�i����� ���⮢� ��楤��",
      "VM: ���i����� ���",
      "%s: ��i�㢠��� ᨬ���",
      "%s: ���������� ⨯ ᨬ���� ��� self",
      "%s: ��i�㢠��� ���� ����",
      "%s: ���������� ⨯ �������� �� ���設i �⥪�",
      "���i� �� ���i �⥪�",
      "%s: �஡� ���i� ������� �� ᥡ�",
      "%s: ��������� i�'� ��i����",
      "��९������� ����� ���'��i",
      "hb_xgrab requested to allocate zero bytes",
      "hb_xrealloc requested to resize to zero bytes",
      "hb_xalloc requested to allocate zero bytes",

      /* Texts */

      "����/��/��",
      "�",
      "�"
   }
};

#define HB_LANG_ID  UA866
#include "hbmsgreg.h"
