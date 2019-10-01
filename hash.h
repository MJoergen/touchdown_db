#ifndef _TOUCHDOWN_HASH_H
#define _TOUCHDOWN_HASH_H

// The hash value of the board consists of a 16-bit string (in bits 15-0) and
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
// and is represented with the hash value 0xF0F00F.
//
//
// The total number of 24-bit hash values is 2^24 = 17 million.
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
// The above reduces the number of valid hash values to 4636513.
//
// The count can be reduced further by observing that the value in the 8-bit
// string may not have set bit numbers corresponding to non-existing pieces on
// the board.  So e.g. if there are only 7 pieces on the board, then the MSB of
// the 8-bit string must be zero. This leads to a count of 2 million.
//

inline bool isHashLegal(uint32_t hash) {
   uint16_t b = hash & 0xFFFF;
   uint16_t c = (hash >> 16) & 0xFF;

   unsigned int b_count = __builtin_popcount(b);   // Total number of pieces
   unsigned int c_count = __builtin_popcount(c);   // Number of player pieces

   // No more than four player pieces.
   // This reduces number of hashes from 16777216 to 65536*163 = 10682368.
   if (c_count > 4)
      return false;

   // No more than four enemy pieces.
   // This reduces number of hashes from 10682368 to 4693459.
   if (c_count+4 < b_count)
      return false;

   // No more player pieces than non-empty squares.
   // This reduces number of hashes from 4693459 to 4636513.
   if (c_count > b_count)
      return false;

   // No illegally assigned pieces
   // This reduces number of hashes from 4636513 to 2267253.
   if (c >= (1 << b_count))
      return false;

   // At this point, the hash corresponds to a board position with at most 4
   // player pieces and at most 4 enemy pieces.  The board position may still
   // be illegal, if the player already has piece at the opponents back row.
   return true;
} // isHashLegal

void testHash();

#endif // _TOUCHDOWN_HASH_H

