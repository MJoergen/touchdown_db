#ifndef _TOUCHDOWN_BOARD_H
#define _TOUCHDOWN_BOARD_H

#include <ostream>
#include <assert.h>
#include "index.h"

template <int tNumRows, int tNumCols>
class Board
{
   // Construct a 4x4 display of the current board position.
   friend std::ostream& operator<< (std::ostream& os, const Board& board) {
      assert (board.positionIsValid());
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
   // Construct a one-line display of the current board position.
   std::string toShortString() const {
      assert (positionIsValid());
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

   // Construct board from an index value.
   // The default value corresponds to the initial board position of the game:
   //    OOOO
   //    ....
   //    ....
   //    XXXX
   Board(uint32_t index = 0xF0F00F) {
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

      assert (positionIsValid());
   } // Board

   // Recreate the index value from the current board position.
   uint32_t getIndex() const {
      assert (positionIsValid());

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
      uint32_t index = (x<<16) | (m_position & 0xFFFF);
      assert (indexIsValid(index));
      return index;
   } // getIndex

   // Checks the board invariant, that the player pawns must be on non-empty
   // squares.
   bool positionIsValid() const {
      return (((~m_position) & (m_position >> 16)) == 0);
   } // positionIsValid

   uint32_t getPosition() const {
      return m_position;
   }

   void setPosition(uint32_t position) {
      m_position = position;
   }

   // Return true if the game is already lost for the player to move
   // This is the case if the opponent has a piece on the first row,
   // or if the player has no pieces left.
   bool isLoss() const {
      uint16_t player   = m_position & (m_position >> 16);
      uint16_t opponent = m_position & (~(m_position >> 16));
      if (opponent & 0xF000)
         return true;
      if (!player)
         return true;
      return false;
   }

   // Return true if the game is already won for the player to move. This is an illegal board position.
   // This is the case if the player has a piece on the last row,
   // or if the opponent has no pieces left.
   bool isWin() const {
      uint16_t player   = m_position & (m_position >> 16);
      uint16_t opponent = m_position & (~(m_position >> 16));
      if (player & 0x000F)
         return true;
      if (!opponent)
         return true;
      return false;
   }

   // Rotates the board, to view it from the opponets side.
   static uint32_t swapPosition(uint32_t position) {
      uint32_t newPosition = (indexReverse16(position >> 16) << 16) | indexReverse16(position & 0xFFFF);
      newPosition ^= (newPosition & 0xFFFF) << 16;
      return newPosition;
   } // swapPosition

   // Generate a list of all the legal moves in the current position.
   int writeLegalMoves(uint32_t legalMoves[12]) const {
      assert (positionIsValid());   // Check board invariant
      assert (!isWin());            // No pawns on the back row.

      uint16_t player   = m_position & (m_position >> 16);
      uint16_t opponent = m_position & (~(m_position >> 16));

      //  0  1  2  3
      //  4  5  6  7
      //  8  9 10 11
      // 12 13 14 15
      uint16_t mask = 0x0010;   // Skip last row, i.e. bits 0 - 3.
      
      int moveCount = 0;
      // Loop over all squares
      for (int i=0; i<12; ++i, mask *= 2) {
         // Look for a player pawn.
         if (!(player & mask)) {
            continue;
         }

         // Is square in front empty?
         uint16_t newMask = mask >> 4;
         if (!(m_position & newMask)) {
            // Move pawn up one row.
            uint32_t moveMask = mask | newMask;
            moveMask |= moveMask << 16;
            uint32_t newPosition = swapPosition(m_position ^ moveMask);

            legalMoves[moveCount++] = newPosition;
            assert (((~newPosition) & (newPosition >> 16)) == 0);
         }

         // Does the square diagnoally right contain an opponent?
         newMask = mask >> 3;
         const uint16_t maskRight = 0x8888; // All square in the right-most coloumn.
         if (!(mask & maskRight) && (opponent & newMask)) {

            // Capture pawn up right one row.
            uint32_t moveMask = mask | (mask << 16);  // Remove player from original square
            moveMask |= (newMask << 16);              // Capture opponent on new square
            uint32_t newPosition = swapPosition(m_position ^ moveMask);

            legalMoves[moveCount++] = newPosition;
            assert (((~newPosition) & (newPosition >> 16)) == 0);
         }

         // Does the square diagnoally left contain an opponent?
         newMask = mask >> 5;
         const uint16_t maskLeft  = 0x1111; // All squares in the left-most coloumn.
         if (!(mask & maskLeft) && (opponent & newMask)) {

            // Capture pawn up left one row.
            uint32_t moveMask = mask | (mask << 16);  // Remove player from original square
            moveMask |= (newMask << 16);              // Capture opponent on new square
            uint32_t newPosition = swapPosition(m_position ^ moveMask);

            legalMoves[moveCount++] = newPosition;
            assert (((~newPosition) & (newPosition >> 16)) == 0);
         }
      } // end for

      return moveCount;
   } // writeLegalMoves

   private:
   // Bits 15-0 indicate whether the square is occupied or not.
   // Bits 31-16 indicate whether the squares is occupied by the player.
   // The initial position is given by 0xF000F00F.
   uint32_t m_position;
}; // class Board

#endif // _TOUCHDOWN_BOARD_H

