//
// $Id: arreval.prg 9279 2011-02-14 18:06:32Z druzus $
//

function Main()

   local a := { 100, 200, 300 }

   aEval(a, {|nValue, nIndex| QOut(nValue, nIndex) })

return nil

