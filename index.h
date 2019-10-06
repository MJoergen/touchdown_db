#ifndef _TOUCHDOWN_INDEX_H
#define _TOUCHDOWN_INDEX_H

// This 256-byte array stores the bit-reverse of all 8-bit numbers.
extern uint8_t indexReverse8Cache[0x100];

void indexReverseInit();
   
inline uint8_t indexReverse8(uint8_t x) { return indexReverse8Cache[x]; }

inline uint16_t indexReverse16(uint16_t x) { 
   return (indexReverse8(x & 0xFF) << 8) | indexReverse8(x >> 8);
}
   
inline uint32_t indexReverse32(uint32_t x) { 
   return (indexReverse16(x & 0xFFFF) << 16) | indexReverse16(x >> 16);
}
   

// The index value of the board consists of a 16-bit string (in bits 15-0) and
// an 8-bit string (in bits 23-16). 
// The 16-bit string has one bit corresponding to each position on the board,
// and a zero bit indicates an empty square, whereas a one bit indicates a
// non-empty square.
// The 8-bit string has one bit corresponding to each non-empty square on the
// board, and a one bit indicates the current player to move, whereas a zero
// bit indicates the opponent.
//
// The bit ordering is as follows:
//   col 0  1  2  3
// row +-----------
//  0  | 0  1  2  3
//  1  | 4  5  6  7
//  2  | 8  9 10 11
//  3  |12 13 14 15
//
// The player to move starts in positions 12-15 (i.e. row 3) and moves up the
// board towards lower row numbers.
//
// The starting position has the following layout:
//    OOOO
//    ....
//    ....
//    XXXX
// and is represented with the index value 0xF0F00F.
//
//
// The total number of 24-bit index values is 2^24 = 17 million.
// However, not all of these values correspond to a valid board position.
// First of, a maximum of four pieces are allowed for the player to move. So
// the 8-bit string may have at most four non-zero bits. The total number of
// possibilities can be written as B(8,0) + ... + B(8,4), where B(n,m) =
// n!/(m!*(m-m)!). This sum can be re-written as (2^8 + B(8,4))/2 = 163.
//
// Similarly, a maximum of four pieces are allowed for the opponent.
// Furthermore, the number of player pieces must be at least equal to the
// total number of pieces.
//
// These conditions can be expressed diagramatically by showing the legal
// values of c_count (number of player pieces) versus b_count (total number
// of pieces):
//
// (c_count)
// 4|....XXXXX
// 3|...XXXXX.
// 2|..XXXXX..
// 1|.XXXXX...
// 0|XXXX.....
//  +---------
//   012345678 (b_count)
//
// The above reduces the number of valid index values to 4636513.
//
// The count can be reduced further by observing that the value in the 8-bit
// string may not have set bit numbers corresponding to non-existing pieces on
// the board.  So e.g. if there are only 7 pieces on the board, then the MSB of
// the 8-bit string must be zero. This leads to a count of 2 million.
//

inline bool indexIsValid(uint32_t index) {
   uint16_t b = index & 0xFFFF;
   uint16_t c = (index >> 16) & 0xFF;

   unsigned int b_count = __builtin_popcount(b);   // Total number of pieces
   unsigned int c_count = __builtin_popcount(c);   // Number of player pieces

   // No more than four player pieces.
   if (c_count > 4)
      return false;

   // No more than four enemy pieces.
   if (c_count+4 < b_count)
      return false;

   // No illegally assigned pieces
   if (c >= (1 << b_count))
      return false;

   // At this point, the index corresponds to a board position with at most 4
   // player pieces and at most 4 enemy pieces.  The board position may still
   // be illegal, if the player already has a piece at the opponents back row.
   return true;
} // indexIsValid

void indexTest();

#endif // _TOUCHDOWN_INDEX_H

