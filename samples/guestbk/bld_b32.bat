@echo off
rem
rem $Id: bld_b32.bat 9280 2011-02-14 18:16:32Z druzus $
rem

..\..\bin\harbour /n guestbk  /i..\..\include\
..\..\bin\harbour /n inifiles /i..\..\include\
..\..\bin\harbour /n testcgi  /i..\..\include\

echo -O2 -I..\..\include -L..\..\lib > build.tmp
echo -eguestbk.exe >> build.tmp
echo guestbk.c inifiles.c testcgi.c >> build.tmp
echo debug.lib >> build.tmp
echo vm.lib >> build.tmp
echo rtl.lib >> build.tmp
echo gtwin.lib >> build.tmp
echo lang.lib >> build.tmp
echo rdd.lib >> build.tmp
echo macro.lib >> build.tmp
echo pp.lib >> build.tmp
echo dbfntx.lib >> build.tmp
echo dbfcdx.lib >> build.tmp
echo common.lib >> build.tmp
bcc32 @build.tmp
del build.tmp

guestbk
