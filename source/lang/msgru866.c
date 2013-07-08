/*
 * $Id: msgru866.c 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Language Support Module (RU866)
 *
 * Copyright 2000 Alexander S.Kresin <alex@belacy.belgorod.su>
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

/* Language name: Russian */
/* ISO language code (2 chars): RU */
/* Codepage: 866 */

#include "hbapilng.h"

static HB_LANG s_lang =
{
   {
      /* Identification */

      "RU866",                     /* ID */
      "Russian",                   /* Name (in English) */
      "���᪨�",                   /* Name (in native language) */
      "RU",                        /* RFC ID */
      "866",                       /* Codepage */
      "$Revision: 9279 $ $Date: 2011-02-14 16:06:32 -0200 (seg, 14 fev 2011) $",         /* Version */

      /* Month names */

      "������",
      "���ࠫ�",
      "����",
      "��५�",
      "���",
      "���",
      "���",
      "������",
      "�������",
      "������",
      "�����",
      "�������",

      /* Day names */

      "����ᥭ�",
      "�������쭨�",
      "��୨�",
      "�।�",
      "��⢥�",
      "��⭨�",
      "�㡡��",

      /* CA-Cl*pper compatible natmsg items */

      "����� ������      # �����     ��᫥���� ���.  ������",
      "�㦭� �� �ਬ��� ?",
      "���.N",
      "** Subtotal **",
      "* Subsubtotal *",
      "*** Total ***",
      "Ins",
      "   ",
      "����ୠ� ���",
      "��������: ",
      " - ",
      "�/�",
      "�������� ���������",

      /* Error description names */

      "�������⭠� �訡��",
      "������ ��㬥��",
      "��९������� ���ᨢ�",
      "��९������� ��ப�",
      "��९������� �᫠",
      "������� �� ����",
      "��᫥���� �訡��",
      "���⠪��᪠� �訡��",
      "���誮� ᫮���� ������",
      "",
      "",
      "�� 墠⠥� �����",
      "�������⭠� �㭪��",
      "��⮤ �� �ᯮ��஢��",
      "��६����� �� �������",
      "����� �� �������",
      "��६����� �� �ᯮ��஢���",
      "�������⨬� ᨬ���� � ����� �����",
      "����� 㦥 �ᯮ������",
      "",
      "�訡�� ᮧ�����",
      "�訡�� ������",
      "�訡�� �������",
      "�訡�� �⥭��",
      "�訡�� �����",
      "�訡�� ����",
      "",
      "",
      "",
      "",
      "������ �� �����ন������",
      "����� �ॢ�襭",
      "�����㦥�� ���०�����",
      "�訡�� ⨯� ������",
      "�訡�� ࠧ��� ������",
      "���� �� �����",
      "���� �� ������஢��",
      "�ॡ���� �᪫��� �����",
      "�ॡ���� �����஢��",
      "������ �� ࠧ�襭�",
      "���� �����஢�� �� ����������",
      "�����஢�� �� 㤠����",
      "",
      "",
      "",
      "",
      "����୮� ������⢮ ��㬥�⮢",
      "����� � ���ᨢ�",
      "��᢮���� ���ᨢ�",
      "�� ���ᨢ",
      "�ࠢ�����",
      "Invalid self",


      /* Internal error names */

      "����ࠢ���� �訡�� %lu: ",
      "�訡�� �� ����⠭�������",
      "�� ��।���� ERRORBLOCK() ��� �訡��",
      "�ॢ�襭 �।�� ४��ᨢ��� �맮��� ��ࠡ��稪� �訡��",
      "�� 㤠���� ����㧨�� RDD",
      "������ ⨯ ��⮤� %s",
      "hb_xgrab �� ����� ��।����� ������",
      "hb_xrealloc �맢�� � NULL 㪠��⥫��",
      "hb_xrealloc �맢�� � ������ 㪠��⥫��",
      "hb_xrealloc �� ����� �����।����� ������",
      "hb_xfree �맢�� � ������ 㪠��⥫��",
      "hb_xfree �맢�� � NULL 㪠��⥫��",
      "�� ������� ���⮢�� ��楤��: \'%s\'",
      "��������� ���⮢�� ��楤��",
      "VM: ��������� ���",
      "%s: �������� ᨬ���",
      "%s: ������ ⨯ ᨬ���� ��� self",
      "%s: �������� ���� ����",
      "%s: ������ ⨯ ����� �� ���設� �⥪�",
      "��室 �� �।��� �⥪�",
      "%s: ����⪠ ����஢��� ����� �� ᥡ�",
      "%s: ����୮� ��� ��६�����",
      "��९������� ���� �����",
      "hb_xgrab requested to allocate zero bytes",
      "hb_xrealloc requested to resize to zero bytes",
      "hb_xalloc requested to allocate zero bytes",

      /* Texts */

      "����/��/��",
      "�",
      "�"
   }
};

#define HB_LANG_ID  RU866
#include "hbmsgreg.h"
