#include <iostream>
#include <iomanip>
#include "index.h"

// Take a binary value "ab" and return "ba"
static unsigned int reverse2(unsigned int x)
{
   return ((x&1) << 1) | (x >> 1);
}

// Take a binary value "abcd" and return "dcba"
static unsigned int reverse4(unsigned int x)
{
   return (reverse2(x & 3) << 2) | reverse2(x >> 2);
}

// Take a binary value "abcdefgh" and return "hgfedcba"
static unsigned int reverse8(unsigned int x)
{
   return (reverse4(x & 0xF) << 4) | reverse4(x >> 4);
}

uint8_t indexReverse8Cache[0x100];

// Pre-calculate the bit-reverse of all 8-bit numbers
// and store in a 256-byte array.
void indexReverseInit()
{
   for (unsigned int x=0; x<0x100; ++x)
   {
      indexReverse8Cache[x] = reverse8(x);
   }
} // void indexReverseInit()


// This function counts the number of valid and invalid index vaules.
void indexTest()
{
   uint32_t cnt_0 = 0;
   uint32_t cnt_1 = 0;
   uint32_t cnt_2 = 0;
   uint32_t cnt_3 = 0;

   for (uint32_t hash = 0; hash < 0x01000000; ++hash) {

      uint16_t b = hash & 0xFFFF;
      uint16_t c = (hash >> 16) & 0xFF;

      unsigned int b_count = __builtin_popcount(b);
      unsigned int c_count = __builtin_popcount(c);

      cnt_0++;

      // No more than four player pieces.
      if (!(c_count <= 4))
         continue;

      cnt_1++;

      // No more than four enemy pieces.
      if (!(c_count+4 >= b_count))
         continue;

      cnt_2++;

      // No illegally assigned pieces
      if (c >= (1 << b_count))
         continue;

      cnt_3++;
   }

   std::cout << "Total possible index values : " << std::setw(8) << cnt_0 << std::endl;
   std::cout << "No more than 4 player pawns : " << std::setw(8) << cnt_1 << std::endl;
   std::cout << "No more than 4 enemy pawns  : " << std::setw(8) << cnt_2 << std::endl;
   std::cout << "No invalid player bits      : " << std::setw(8) << cnt_3 << std::endl;
} // void indexTest()
