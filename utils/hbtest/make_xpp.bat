@echo off
rem
rem $Id: make_xpp.bat 9280 2011-02-14 18:16:32Z druzus $
rem

xpp hbtest.prg   /w /n
xpp rt_array.prg /w /n
xpp rt_date.prg  /w /n
xpp rt_file.prg  /w /n
xpp rt_hvm.prg   /w /n
xpp rt_hvma.prg  /w /n
xpp rt_math.prg  /w /n
xpp rt_misc.prg  /w /n
xpp rt_str.prg   /w /n
xpp rt_stra.prg  /w /n
xpp rt_trans.prg /w /n

alink hbtest rt_array rt_date rt_file rt_hvm rt_hvma rt_math rt_misc rt_str rt_stra rt_trans

del *.obj
