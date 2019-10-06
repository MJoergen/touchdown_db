#ifndef _TOUCHDOWN_BOARD_H
#define _TOUCHDOWN_BOARD_H

#include <ostream>
#include <assert.h>
#include "index.h"

template <int tNumRows, int tNumCols>
class Board
{
   // Bits 15-0 indicate whether the square is occupied or not.
   // Bits 31-16 indicate whether the squares is occupied by the player.
   // The initial position is given by 0xF000F00F.

   friend std::ostream& operator<< (std::ostream& os, const Board& board) {
      uint32_t position = board.m_position;
      for (int i=0; i<16; ++i) {
         switch (position & 0x10001) {
            case 0x00000 : os << "."; break;
            case 0x00001 : os << "O"; break;
            case 0x10001 : os << "X"; break;
            default : assert(false);
         }
         if ((i&3) == 3) {
            os << std::endl;
         }
         position /= 2;
      }
      return os;
   } // operator <<

   public:
   // Construct board from a index value.
   // * Bits 15-0 indicate whether the square is occupied or not.
   // * Bits 31-16 indicate whether the squares is occupied by the player.
   // The initial position is given by 0xF000F00F.
   // The default value corresponds to the initial board position of the game:
   //    OOOO
   //    ....
   //    ....
   //    XXXX
   Board(uint32_t index = 0x00F0F00F) {
      assert(indexIsValid(index));

      m_position = index & 0xFFFF;

      uint16_t x = index >> 16;

      uint16_t mask = 0x0001;

      for (int i=0; i<16; ++i) {
         if (index & mask) {
            if (x & 1) {
               m_position |= (mask << 16);
            }
            x /= 2;
         }
         mask *= 2;
      }

   } // Board

   void setPosition(uint32_t position) {
      m_position = position;
   }

   // Recreate the index value from the current board position.
   // * Bits 15-0 indicate whether the square is occupied or not.
   // * Bits 31-16 indicate whether the squares is occupied by the player.
   uint32_t getIndex() const {
      assert (((~m_position) & (m_position >> 16)) == 0);

      uint16_t maskP = 0x0001;
      uint16_t maskX = 0x0001;
      uint8_t x = 0;
      for (int i=0; i<16; ++i) {
         if (m_position & maskP) {
            if (m_position & (maskP << 16)) {
               x |= maskX;
            }
            maskX *= 2;
         }
         maskP *= 2;
      }
      return (x<<16) | (m_position & 0xFFFF);
   } // getIndex

   // Return true if the game is already lost for the player to move
   // This is the case if the opponent has a piece on the first row.
   bool isLoss() const {
      uint16_t opponent = m_position & (~(m_position >> 16));
      if (opponent & 0xF000)
         return true;
      return false;
   }

   // Return true if the game is already won for the player to move. This is an illegal board position.
   // This is the case if the player has a piece on the last row,
   // or if the opponent has no pieces left.
   bool isWin() const {
      uint16_t player = m_position & (m_position >> 16);
      uint16_t opponent = m_position & (~(m_position >> 16));
      if (player & 0x000F)
         return true;
      if (!opponent)
         return true;
      return false;
   }

   // Return piece value at specific square
   int getPiece(int square) const {
      uint16_t opponent = m_position & (~(m_position >> 16));
      uint16_t player = m_position & (m_position >> 16);

      if (player & (1<<square))
         return 1;
      else if (opponent & (1<<square))
         return -1;
      else
         return 0;
   }

   uint32_t getPosition() const {
      return m_position;
   }

   std::string toShortString() const {
      std::string ret;

      uint16_t opponent = m_position & (~(m_position >> 16));
      uint16_t player = m_position & (m_position >> 16);

      for (int i=0; i<16; ++i) {
         if (player & (1<<i))
            ret += "X";
         else if (opponent & (1<<i))
            ret += "O";
         else
            ret += ".";
         if ((i&3)==3)
            ret += " ";
      }

      return ret;
   } // toShortString

   private:
   // Bits 15-0 indicate whether the square is occupied or not.
   // Bits 31-16 indicate whether the squares is occupied by the player.
   // The initial position is given by 0xF000F00F.
   uint32_t m_position;
}; // class Board

#endif // _TOUCHDOWN_BOARD_H

