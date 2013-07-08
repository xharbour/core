*****************************************************
* Hash Grammar test
*
* Giancarlo Niccolai (C) 2003
*
* This is a test that demonstrates how to use hashes
*
* $Id: hash.prg 9279 2011-02-14 18:06:32Z druzus $
*

PROCEDURE Main()
   LOCAL hHash, hTemp
   LOCAL nSum, nLevel, eError
   LOCAL xKey, xValue, hDest

   SET DATE TO ITALIAN
   CLEAR SCREEN

   @0,0 SAY "*** Hash test ***"
   ? "Giancarlo Niccolai"
   ?

   * Creation by PP command:
   * Equivalent to Hash( K1, V1, ... KN, VN )

   hHash := Hash()
   HSetPartition( hHash, 2, 2 )


   /* Insertion by API */
   hHash["Kval"] = 'StrKey 0'
   hHash[ 8 ] = 'Num Key 0'
   HSet( hHash, 4,  "Numeric key 1" )
   HSet( hHash, 2,  "Numeric key 2" )
   HSet( hHash, "Str8", "String key 1" )
   HSet( hHash, "Str1", "String key 2" )
   HSet( hHash, CToD("1/1/2003"), "Date key 1" )
   HSet( hHash, CToD("30/11/2002") , "Date key 2" )

   ? "Standard API compliance test:"
   ? "Hash type:", ValType( hHash )
   ? "Hash length:", Len( hHash )
   ? "Hash value:", ValToPrg( hHash )
   ? "Empty hash value:", ValToPrg( { => } )
   ? "String representation (should be nothing):", {=>}
   ? "Equality of hashes (Success for .T. , .F.): ", hHash == hHash, hHash == {=>}
   HGetPartition(hHash, @nSum, @nLevel )
   ? "Hash partitioned as: ", nSum, nLevel

   ? "Plus operator: ", ValToPrg( { 1=>1, 'a'=>2} + { 3=>3, 'b'=>4 } )
   hHash += { 5=> "numkey 3" }
   ? "Plusequal operator (success if Len(hHash) == 9: ", Len(hHash), ",(", hHash[5], ")"
   hTemp := {'a'=>1, 1=>2, 'c'=>3}
   ? "Minus hash - hash operator: ", ValToPrg( hTemp - { 1=>2, 'c'=>3} )
   ? "Minus hash - array operator: ", ValToPrg( hTemp - { 1, 'a'} )
   ? "Minus hash - item operator: ", ValToPrg( hTemp - 'a' )
   ? "Hash is now: ", ValToPrg( hHash )
   ? "operator kval IN hash", ('Str1' IN hHash)
   ? "operator {=>} IN hash", ({ "Str8" => "String key 1" } IN hHash)
   ? "Turning autoadd off (currently:", HGetAutoAdd( hHash ), ")"
   HSetAutoAdd( hHash, .F. )
   TRY
      hHash[ 'a very new key' ] := 'a value'
      ? "Failure (new key inserted)"
   CATCH eError
      ? "Success: ", eError:description
   END
   HSetAutoAdd( hHash, .T. )

   ? "Press a Key to continue"
   ?
   Inkey(0)

   ? "VM compliance test:"
   ? "Numeric key value hHash[4]:", hHash[4]
   ? "Date key value hHash[CToD('1/1/2003')]:", hHash[CToD('1/1/2003')]
   ? "String key value hHash['Kval']:", hHash['Kval']
   ? "Assign eval hHash['Kval'] := 100", (hHash['Kval'] := 100)
   ? "Assign eval result hHash['Kval']", hHash['Kval']
   M->iPos := 2
   ? "Memvar test hHash[ m->iPos ]: ", hHash[ m->iPos ]
   m->hMem := hHash
   ? "Memvar assign m->hMem := hHash, ValType( m->hMem ): ", ValType( m->hMem )
   TRY
      ? "Wrong index test: hHash[Array()]: Failed", hHash[Array()]
   CATCH eError
      ? "Wrong index test: hHash[Array()]: Passed (", eError:description, ")"
   END
   HSetCaseMatch( hHash, .F. )
   ? "Hash gramar ':' existing key insensitive:", hHash:kval
   HSetCaseMatch( hHash, .Y. )
   ? "Hash gramar ':' adding key:", ( hHash:ColonKey := 'Colon value' )
   ? "Hash gramar ':' retreiving key:", hHash:ColonKey
   ? "Hash grammar ':' classname:", hHash:ClassName
   ? "Hash grammar ':' keys:", ValToPrg( hHash:Keys )
   ? "Hash grammar ':' values:", ValToPrg( hHash:Values )

   ? "Press a Key to continue"
   ?
   Inkey(0)

   ? "HASH api test:"
   ? "HGetPos existing key:", HGetPos( hHash, 2 )
   ? "HGetPos unexisting key:", HGetPos( hHash, 1000 )
   ? "HDel key Str1: (should be ok) "
      HDel( hHash, "Str1" )
   ? "HGetKeys: ", ValToPrg( hGetKeys( hHash ) )
   ? "HGetValues: ", ValToPrg( hGetValues( hHash ) )
   ? "HGetKeyAt 3d pos: ", HGetKeyAt( hHash, 3 )
   ? "HGetValueAt 3d pos: ", HGetValueAt( hHash, 3 )
   ? "HGetPairAt 4th pos (as array):", ValToPrg( HGetPairAt( hHash, 4 ) )
   HGetPairAt( hHash, 4, @xKey, @xValue )
   ? "HGetPairAt 4th pos (as byref):", xKey, xValue
   ? "HDelAt 4th position:"
      HDelAt( hHash, 4 )
   ? "Setting 4th value to 'A newer value'"
      HSetValueAt( hHash, 4, 'A newer value' )
   ? "Hash is now: ", ValToPrg( hHash )
   ? "Press a Key to continue"
   ?
   Inkey(0)

   ? "HASH Case insensitive test:"
   HSetCaseMatch( hHash, .F. )
   hHash[ 'a' ] := 100
   TRY
      HSetCaseMatch( hHash, .T. )
      ? "Sensitivity test failed: ", hHash[ 'A' ]
   CATCH eError
      ? "Sensitivity test Passed (", eError:description,  ")"
   END

   HSetCaseMatch( hHash, .F. )
   TRY
      ? "Insensitivity test passed: ", hHash[ 'A' ]
   CATCH eError
      ? "Insensitivity test Failed (", eError:description,  ")"
   END

   TRY
      hHash['A'] := 50
      ? "Insensitive assignment (success if 50): ", hHash[ 'a' ]
   CATCH eError
      ? "Insensitivity assignment Failed (", eError:description,  ")"
   END
   ? "Press a Key to continue"
   ?
   Inkey(0)


   ? "HGetPos existing key:", HGetPos( hHash, 2 )
   ? "HGetPos unexisting key:", HGetPos( hHash, 1000 )
   ? "HDel key Str1: (should be ok) "
      HDel( hHash, "Str8" )
   ? "HGetKeys: ", ValToPrg( hGetKeys( hHash ) )
   ? "HGetValues: ", ValToPrg( hGetValues( hHash ) )
   ? "HGetKeyAt 3d pos: ", HGetKeyAt( hHash, 3 )
   ? "HGetValueAt 3d pos: ", HGetValueAt( hHash, 3 )
   ? "HGetPairAt 4th pos (as array):", ValToPrg( HGetPairAt( hHash, 4 ) )
   HGetPairAt( hHash, 4, @xKey, @xValue )
   ? "HGetPairAt 4th pos (as byref):", xKey, xValue
   ? "HDelAt 4th position:"
      HDelAt( hHash, 4 )
   ? "Setting 4th value to 'A newer value'"
      HSetValueAt( hHash, 4, 'A newer value' )
   ? "Hash is now: ", ValToPrg( hHash )
   ? "Press a Key to continue"
   ?
   Inkey(0)


   ? "HASH Secondary API test:"
   ? "Scanning for value 'A newer value': ", HScan( hHash, 'A newer value' )
   ? "Scanning for value 'A newer value' using CB: ",;
       HScan( hHash, {| cKey, cVal| HB_ISSTRING(cVal) .and. cVal == 'A newer value'} )

   nSum := 0
   HEval( hHash, { | cKey, cVal|;
       IIF (HB_ISNUMERIC(cKey), nSum += cKey, )} )

   ? "Eval summing up all the numeric keys :", nSum
   ? "Clone of the hash:", ValToPrg(HClone( hHash ))
   hDest := { 'A'=> 1, 'b'=>2 }
   ? "Merging hash with { 'a'=> 1, 'b'=>2 }:"
   ? "Result: ", ValToPrg(HCopy( hHash, hDest ))

   hDest := { 'B'=> 1, 'A'=>2 }
   ? "Merging limited with a codeblock (Only numeric values): "
   ? "Result: ", ValToPrg( HMerge( hDest, hHash, { |cKey, nVal| HB_IsNumeric( nVal ) } ) )
   * The last "2" means XOR
   ? "Doing a xor merge with the original one (first 4 elements): "
   ? "Result: ",  ValToPrg( HCopy( hHash, hDest, , , 2 ) )

   ? "Doing an AND merge with { 'A'=>0, 'B'=>1 } "
   ? "Result: ",  ValToPrg( HMerge( hDest, {'A'=>0, 'B'=>1 }, 1) )

   ? "Press a Key to continue"
   ?
   Inkey(0)
