#include "hbdefs.h"

int main()
{
   char buf[16];
   int n, i, l, f, iRet = 0;

   printf("\nStandard C types:\n");
   printf("\t        sizeof(void*)=%d\n", (int) sizeof(void*));
   printf("\t         sizeof(char)=%d\n", (int) sizeof(char) );
   printf("\t    sizeof(short int)=%d\n", (int) sizeof(short int) );
   printf("\t          sizeof(int)=%d\n", (int) sizeof(int) );
   printf("\t     sizeof(long int)=%d\n", (int) sizeof(long int) );
   printf("\tsizeof(long long int)=%d\n", (int) sizeof(long long int) );
   printf("\t        sizeof(float)=%d\n", (int) sizeof(float));
   printf("\t       sizeof(double)=%d\n", (int) sizeof(double));
   printf("\t  sizeof(long double)=%d\n", (int) sizeof(long double));

   printf("\nxHarbour types:\n");
   printf("\t    sizeof(BYTE)=%d %s\n", (int) sizeof(BYTE    ), sizeof(BYTE    )==1 ? "OK" : "BAD" );
   printf("\t   sizeof(SHORT)=%d %s\n", (int) sizeof(SHORT   ), sizeof(SHORT   )==2 ? "OK" : "BAD" );
   printf("\t    sizeof(UINT)=%d %s\n", (int) sizeof(UINT    ), sizeof(UINT)==4 || sizeof(UINT)==8 ? "OK" : "BAD" );
   printf("\t    sizeof(LONG)=%d %s\n", (int) sizeof(LONG    ), sizeof(LONG)==4 || sizeof(LONG)==8 ? "OK" : "BAD" );
   printf("\tsizeof(LONGLONG)=%d %s\n", (int) sizeof(LONGLONG), sizeof(LONGLONG)==8 ? "OK" : "BAD" );
   printf("\t  sizeof(double)=%d %s\n", (int) sizeof(double  ), sizeof(double  )==8 ? "OK" : "BAD" );

   if ( sizeof(BYTE)!=1 || sizeof(SHORT)!=2 ||
        (sizeof(LONG)!=4 && sizeof(LONG)!=8) ||
        (sizeof(LONGLONG)!=4 && sizeof(LONGLONG)!=8) ||
        sizeof(double)!=8 )
   {
      iRet = 1;
   }

   n = 0x31323334;
   memcpy( buf, &n, sizeof( n ) );
   buf[sizeof( n )] = '\0';
   i = atoi( buf );
#if defined(HB_PDP_ENDIAN)
   l = 2143;
#elif defined(HB_BIG_ENDIAN)
   l = 1234;
#else
   l = 4321;
#endif
   printf( "\nn=0x%x -> \"%s\" (%s endian) %s\n", n, buf,
            i == 1234 ? "big" : 
            i == 2143 ? "pdp" : 
            i == 4321 ? "little" : "unknown",
            i == l ? "OK" : "BAD" );
   if( i != l )
   {
      iRet = 1;
   }

   buf[0] = 0x12;
   buf[1] = 0x34;
   buf[2] = 0x56;
   buf[3] = 0x78;
   buf[4] = 0x65;
   i = ( HB_GET_BE_UINT32( buf ) == 0x12345678L &&
         HB_GET_LE_UINT32( buf ) == 0x78563412L );
   if ( ! i )
   {
      iRet = 1;
   }
   printf( "byte order translation: %s\n", i ? "OK" : "BAD" );

   for ( l = 0; l < 4; l++ )
   {
      n = HB_GET_BE_UINT16( &buf[l] );
      f = n == (buf[l] * 256 + buf[l+1]) ? 1 : 0;
      if ( ! f )
      {
         iRet = 1;
      }
      printf( "HB_GET_BE_UINT16(%x,%x) = %x -> %s\n", buf[l], buf[l+1], n,
              f ? "OK" : "BAD" );

      n = HB_GET_LE_UINT16( &buf[l] );
      f = n == (buf[l] + 256 * buf[l+1]) ? 1 : 0;
      if ( ! f )
      {
         iRet = 1;
      }
      printf( "HB_GET_LE_UINT16(%x,%x) = %x -> %s\n", buf[l], buf[l+1], n,
              f ? "OK" : "BAD" );
   }

   n = (char)255;
   printf( "n=%d -> (char) type is %ssinged %s\n", n, n < 0 ? "" : "un",
            n < 0 ? "OK" : "BAD" );
   if ( n >= 0 )
   {
      iRet = 1;
   }

   if ( iRet )
   {
      printf("\nxHarbour cannot be compiled !!!\n");
   }
   else
   {
      printf("\nBasic test is correct, try to compile xHarbour.\n");
   }

   return iRet;
}
