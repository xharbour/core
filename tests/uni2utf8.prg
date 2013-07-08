/*
   $Id: uni2utf8.prg 9342 2011-03-10 04:39:04Z andijahja $
   Test program for Stand-Alone Unicode String Conversion
   WARNING: Must link HBCC.LIB
*/

#include "simpleio.ch"

#define HB_CSUTF8 4099

PROCEDURE MAIN()

   LOCAL hFile
   LOCAL aCP := { "CP852", "KOI8U", "CP1251", "CP866", "KOI8R", "CP862", "CP1253", "BGMIK" }
   LOCAL aChar := {;
        { "CP852",  "leden, �nor, b�ezen, duben, kv�ten, Yerven, Yervenec, srpen, z���, ��jen, listopad, prosinec" },;
        { "KOI8U",  "�����, �����, ��������, �צ����, �������, �������, ������, �������, ��������, �������, ��������, �������" },;
        { "KOI8R",  "������, �������, ����, ������, ���, ����, ����, ������, ��������, �������, ������, �������" },;
        { "CP1251", "ѳ����, �����, ��������, ������, �������, �������, ������, �������, ��������, �������, ��������" },;
        { "CP1251", "������, ��������, ����, �����, ���, ���, ���, ������, ���������, ��������, �������, ��������" },;
        { "CP1253", "����������, �����������, �������, ��������, �����, �������, �������, ���������, �����������, ���������, ���������, ����������" },;
        { "CP862",  "�����, ������, ���, �����, ���, ����, ����, ������, ������, �������, ������, �����" },;
        { "BGMIK",  "������, ��������, ����, �����, ���, ���, ���, ������, ���������, ��������, �������, ��������" },;
        { "CP866",  "��㤧���, ����, �����i�, ��ᠢi�, ���, ��ࢥ��, �i����, ��i����, ���ᥭ�, �������i�, �i�⠯��, �������" };
        }
   LOCAL aHandle := {}
   LOCAL cCP, i, n
   LOCAL cUTF8 := CHR(239) + CHR(187) + CHR(191)

   FOR EACH cCP IN aCP
      AADD( aHandle, { cCP, HB_CSReg( cCP ) } )
   NEXT

   FOR i := 1 TO LEN( aChar )
       IF ( n := AScan( aHandle, { |e| e[ 1 ] == aChar[ i ][ 1 ] } ) ) > 0
          cUTF8 += PadR( aChar[ i ][ 1 ], 8 ) + ": " + ;
             HB_CSToCS( aChar[ i ][ 2 ], aHandle[ n ][ 2 ], HB_CSUTF8 ) +;
             hb_osnewline()
       ENDIF
   NEXT

   hFile := FCreate( "hbccutf8.txt" )
   FWrite( hFile, cUTF8 )
   FClose( hFile )

RETURN
