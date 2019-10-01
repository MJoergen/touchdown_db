#include <iostream>
#include "hash.h"

void testHash ()
{
   uint32_t cnt_0 = 0;
   uint32_t cnt_1 = 0;
   uint32_t cnt_2 = 0;
   uint32_t cnt_3 = 0;
   uint32_t cnt_4 = 0;

   for (uint32_t hash = 0; hash < 0x01000000; ++hash) {

      uint16_t b = hash & 0xFFFF;
      uint16_t c = (hash >> 16) & 0xFF;

      unsigned int b_count = __builtin_popcount(b);
      unsigned int c_count = __builtin_popcount(c);

      cnt_0++;

      // No more than four player pieces.
      if (c_count > 4)
         continue;

      cnt_1++;

      // No more than four enemy pieces.
      if (c_count+4 < b_count)
         continue;

      cnt_2++;

      // No more player pieces than non-empty squares.
      if (c_count > b_count)
         continue;

      cnt_3++;

      // No illegally assigned pieces
      if (c >= (1 << b_count))
         continue;

      cnt_4++;
   }

   std::cout << "cnt_0 " << cnt_0 << std::endl;
   std::cout << "cnt_1 " << cnt_1 << std::endl;
   std::cout << "cnt_2 " << cnt_2 << std::endl;
   std::cout << "cnt_3 " << cnt_3 << std::endl;
   std::cout << "cnt_4 " << cnt_4 << std::endl;
} // void testHash ()

