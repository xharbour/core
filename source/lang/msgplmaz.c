/*
 * $Id: msgplmaz.c 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Language Support Module (PL Mazowia)
 *
 * Copyright 1999 {list of individual authors and e-mail addresses}
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

/* Language name: Polish */
/* ISO language code (2 chars): PL */
/* Codepage: Mazowia */

#include "hbapilng.h"

static HB_LANG s_lang =
{
   {
      /* Identification */

      "PLMAZ",                     /* ID */
      "Polish",                    /* Name (in English) */
      "Polski",                    /* Name (in native language) */
      "PL",                        /* RFC ID */
      "Mazowia",                   /* Codepage */
      "$Revision: 9279 $ $Date: 2011-02-14 16:06:32 -0200 (seg, 14 fev 2011) $",         /* Version */

      /* Month names */

       "Stycze�",
       "Luty",
       "Marzec",
       "Kwiecie�",
       "Maj",
       "Czerwiec",
       "Lipiec",
       "Sierpie�",
       "Wrzesie�",
       "Pa�dziernik",
       "Listopad",
       "Grudzie�",

      /* Day names */

       "Niedziela",
       "Poniedzia�ek",
       "Wtorek",
       "�roda",
       "Czwartek",
       "Pi�tek",
       "Sobota",

      /* CA-Cl*pper compatible natmsg items */

      "Baza danych       #Rekord�w    Uaktualniona Rozmiar",
      "Wi�cej przyk�ad�w?",
      "Strona",
      "** Subtotal **",
      "* Subsubtotal *",
      "*** Total ***",
      "Wst",    /* wstaw */
      "Zas",    /* zastap */
      "Nieprawid�owa data",
      "Zakres:",
      " - ",
      "T/N",
      "B��dne wyra�enie",

      /* Error description names */

       "B��d bez opisu",
       "Nieprawid�owy argument",
       "B��d zakresu tablicy",
       "Za du�y string",
       "Przepe�nienie numeryczne",
       "Dzielenie przez zero",
       "B��d numeryczny",
       "Nieprawid�owa sk�adnia",
       "Operacja zbyt z�o�ona",
      "",
      "",
       "Za ma�o pami�ci",
       "Niezdefiniowana funkcja",
       "Metoda jest niedost�pna",
       "Zmienna nie istnieje",
       "Alias bazy nie istnieje",
       "Zmienna jest niedost�pna",
       "Nieprawid�owy alias bazy",
       "Podany alias ju� istnieje",
      "",
       "B��d podczas tworzenia zbioru",
       "B��d podczas otwarcia zbioru",
       "B��d podczas zamkni�cia zbioru",
       "B��d podczas odczytu ze zbioru",
       "B��d podczas zapisu do zbioru",
       "B��d wydruku",
      "",
      "",
      "",
      "",
       "Nieprawid�owa operacja",
       "Przekroczony limit",
       "Uszkodzony indeks bazy",
       "Niezgodny typ danych",
       "Warto�� poza zakresem",
       "Baza jest nie otwarta",
       "Baza nie ma indeksu",
       "Wymagany jest wy��czny dost�p do bazy",
       "Wymagana blokada dost�pu",
       "Zapis niedozwolony",
       "Brak blokady dost�pu podczas dodawania rekordu",
       "Nie uda�o si� zablokowa� dost�pu",
      "",
      "",
      "",
      "",
       "Nieprawid�owa liczba argument�w",
       "pobranie elementu tablicy",
       "zmiana warto�ci elementu tablicy",
       "wymagana jest tablica",
       "wymagany typ: logiczny",
       "Invalid self",

      /* Internal error names */

      "Nienaprawialny b��d nr %lu: ",
      "Nieudana pr�ba naprawy b��du",
      "Brak kodu obs�ugi ERRORBLOCK()",
      "Zbyt wiele zagnie�d�onych b��d�w",
      "Nieza�adowany lub z�y RDD",
      "Z�y typ metody wo�anej z %s",
      "hb_xgrab nie mo�e zarezerwowa� pami�ci",
      "hb_xrealloc wywo�any ze wska�nikiem NULL",
      "hb_xrealloc wywo�any ze z�ym wska�nikiem",
      "hb_xrealloc nie mo�e powi�kszy� bloku pami�ci",
      "hb_xfree wywo�any ze z�ym wska�nikiem",
      "hb_xfree wywo�any ze wska�nikiem NULL",
      "Brak definicji procedury startowej: \'%s\'",
      "Brak procedury startowej",
      "Nieprawid�owa warto�� VM opcode",
      "W %s wymagany jest item typu \'Symbol\'",
      "W %s podano z�y item dla SELF",
      "W %s oczekiwany jest item typu \'Codeblock\'",
      "Funkcja %s wymaga innego typu na stosie",
      "Stos poni�ej dna",
      "Item nie mo�e by� skopiowany w %s",
      "W %s podano z�y item jako memvar",
      "Zapis poza przydzielonym obszarem",
      "hb_xgrab requested to allocate zero bytes",
      "hb_xrealloc requested to resize to zero bytes",
      "hb_xalloc requested to allocate zero bytes",

      /* Texts */

      "YYYY.MM.DD",
      "T",
      "N"
   }
};

#define HB_LANG_ID  PLMAZ
#include "hbmsgreg.h"
