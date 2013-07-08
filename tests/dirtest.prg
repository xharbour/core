//
// $Id: dirtest.prg 9279 2011-02-14 18:06:32Z druzus $
//

// directory test
function main(filespec,attribs,cshort)

local adir := {}
local x := 0, lShort := .f., cNewLine := HB_OSNewLine()

   IF !cshort == NIL .and. (Upper( cShort ) == "TRUE" .or. Upper( cShort ) == ".T.")
      lShort := .t.
   ENDIF

//adir := asort( directory(filespec,attribs,lShort),,, {|x,y|upper(x[1]) < upper(y[1])} )
 adir := directory(filespec,attribs,lShort)

SET CENTURY ON

for x := 1 to len(adir)
   outstd(cNewLine)
   outstd(padr(adir[x,1], 20), "|", ;
          transform(adir[x,2], "999,999,999,999"), "|", ;
          adir[x,3], "|", ;
          adir[x,4], "|", ;
          adir[x,5])
next x

return nil
