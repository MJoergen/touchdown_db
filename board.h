#ifndef _TOUCHDOWN_BOARD_H
#define _TOUCHDOWN_BOARD_H

#include <ostream>
#include <assert.h>
#include "hash.h"

template <int tNumRows, int tNumCols>
class Board
{
   friend std::ostream& operator<< (std::ostream& os, const Board& board) {
      for (int row=0; row<4; ++row) {
         for (int col=0; col<4; ++col) {
            if (board.m_player[row][col])
               os << "X";
            else if (board.m_opponent[row][col])
               os << "O";
            else
               os << ".";
         }
         os << std::endl;
      }
      return os;
   }

   public:
   // Construct board from a hash value. The default value corresponds to the
   // initial board position of the game, i.e.:
   //    OOOO
   //    ....
   //    ....
   //    XXXX
   Board(uint32_t hash = 0x00F0F00F) {
      assert(isHashLegal(hash));

      uint16_t b = hash & 0xFFFF;
      uint16_t c = (hash >> 16) & 0xFF;

      for (int row=0; row<tNumRows; ++row) {
         for (int col=0; col<tNumCols; ++col) {
            if (b & 1) {
               if (c & 1) {
                  m_player[row][col]   = 1;
                  m_opponent[row][col] = 0;
               } else {
                  m_player[row][col]   = 0;
                  m_opponent[row][col] = 1;
               }

               c /= 2;
            } else {
               m_player[row][col]   = 0;
               m_opponent[row][col] = 0;
            }
            b /= 2;
         } // for col
      } // for row
   } // Board

   // Return true if the game is already lost for the player to move
   bool isLoss() const {
      for (int col=0; col<tNumCols; ++col) {
         if (m_opponent[tNumRows-1][col])
            return true;
      }
      return false;
   }

   // Return true if the game is already won for the player to move. This is an illegal board position.
   bool isWin() const {
      for (int col=0; col<tNumCols; ++col) {
         if (m_player[0][col])
            return true;
      }
      return false;
   }

   // Return piece value at specific square
   int getPiece(int row, int col) const {
      if (m_player[row][col])
         return 1;
      else if (m_opponent[row][col])
         return -1;
      else
         return 0;
   }

   private:
   int m_player  [tNumRows][tNumCols];
   int m_opponent[tNumRows][tNumCols];
}; // class Board

#endif // _TOUCHDOWN_BOARD_H

