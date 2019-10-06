#include <iostream>
#include <iomanip>
#include <ostream>
#include <vector>
#include <unistd.h>
#include <assert.h>
#include "tablebase.h"
#include "board.h"
#include "index.h"

const int cNumRows = 4;
const int cNumCols = 4;


void dumpAllValidIndices()
{
   for (uint32_t index = 0; index < 0x01000000; ++index) {
      if (!indexIsValid(index)) {
         continue;
      }
      std::cout << std::hex << std::setfill('0') << std::setw(6) << index << std::endl;
   }
} // dumpAllValidIndices

void dumpAllLegalBoards()
{
   for (uint32_t index = 0; index < 0x01000000; ++index) {
      if (!indexIsValid(index)) {
         continue;
      }
      Board<cNumRows, cNumCols> board(index);
      assert (board.getIndex() == index);
      std::cout << std::hex << std::setfill('0') << std::setw(6) << index << " : " << board.toShortString() << " ";
      if (board.isLoss()) 
         std::cout << "LOSS" << std::endl;
      else if (board.isWin())
         std::cout << "INVALID" << std::endl;
      else 
         std::cout << std::endl;
   }
} // dumpAllLegalBoards


// This program creates a database over ALL legal positions on the 4x4 touchdown
// board, and calculates the game theoretic value for each position.

// The algorithm used performs multiple loops over all legal - but still
// unknown - positions.  For each position, it tries all legal moves. If all
// legal moves leads to currently known positions, then this position is
// considered known too, and the database is updated.

int main(int argc, char **argv) {
   assert(cNumRows==4 && cNumCols == 4);

   // Initialize table for calculating the bit-reverse of a number.
   indexReverseInit();
   

   // Process command line options
   char c;
   while ((c = getopt(argc, argv, "hild")) != -1) {
      switch (c)
      {
         case 'h' :
            std::cout << "-h : Print this help."          << std::endl;
            std::cout << "-i : Test index calculation."   << std::endl;
            std::cout << "-l : Dump all valid positions." << std::endl;
            std::cout << "-d : Dump all legal boards."    << std::endl;
            return 0;
         case 'i' : indexTest(); return 0;
         case 'l' : dumpAllValidIndices(); return 0;
         case 'd' : dumpAllLegalBoards(); return 0;
         default  : abort ();
      }
   } // while


   // The initial values of the tablebase is that all positions are unknown.
   TableBase tb;
   TableBase known;

   bool updated = true;
   // Repeat as long as the database is updated.
   while (updated) {
      updated = false; // Assume no more updates.

      std::cout << "." << std::flush;  // Output current progress.

      // Loop over all positions.
      for (uint32_t index = 0; index < 0x01000000; ++index) {

         // Skip positions that are already known
         if (known.readBit(index)) {
            continue;
         }

         // First check if index is valid
         if (!indexIsValid(index)) {
            known.setBit(index, true);
            tb.setBit(index, true); // All invalid indices are given the game value WIN.
            updated = true;
            continue;
         }

         // Now construct the board
         Board<cNumRows, cNumCols> board(index);

         // If the position is a WIN, then this is an illegal position.
         if (board.isWin()) {
            known.setBit(index, true);
            tb.setBit(index, true); // All illegal board positions are given the game value WIN.
            updated = true;
            continue;
         }

         // If the position is a LOSS, then we're done with this position.
         if (board.isLoss()) {
            known.setBit(index, true);
            tb.setBit(index, false); // This position is a LOSS.
            updated = true;
            continue;
         }

         //  0  1  2  3
         //  4  5  6  7
         //  8  9 10 11
         // 12 13 14 15
         // Bits 31-16 indicate whether the square is occupied by the player.
         uint32_t mask = 0x00100000;   // Skip last row
         //const uint32_t maskLeft  = 0x11110000;
         //const uint32_t maskRight = 0x88880000;

         uint32_t position = board.getPosition();
         assert ((position & 0x000F0000) == 0); // No pawns on the back row.

         // Now we loop over all legal moves
         // IF any successor leads to an unknown position, then this position is unknown too.
         // If any successor leads to a LOSS (for the opponent), then this position is a WIN.
         // If all successors lead to a WIN (for the opponent), then this position is a LOSS.
         // If no successor available, then this position is a LOSS.
         bool isKnown = true;    // Assume position is known.
         bool isWin   = false;   // Assume position is a LOSS, e.g. if no successors.

         // Loop over all squares
         for (int i=0; i<12; ++i, mask *= 2) {
            // Look for a player pawn.
            if (!(position & mask)) {
               continue;
            }

            // Is square in front empty?
            if (!(position & (mask >> 20))) {
               uint32_t moveMask = mask | (mask >> 4);
               moveMask = moveMask | (moveMask >> 16);
               uint32_t newPosition = position ^ moveMask;   // Move pawn up one row.

               newPosition = (indexReverse16(newPosition >> 16) << 16) | indexReverse16(newPosition & 0xFFFF);
               newPosition ^= (newPosition & 0xFFFF) << 16;

               board.setPosition(newPosition);
               uint32_t newIndex = board.getIndex();

               if (!known.readBit(newIndex))
               {
                  // If one child is unknown, then we can stop immediately.
                  isKnown = false;
                  break;
               }
               if (!tb.readBit(newIndex))
               {
                  // If one child is lost, then we are winning, and can stop immediately.
                  isWin = true;
                  break;
               }
            }
         } // end for

         if (isKnown) {
            known.setBit(index, true);
            tb.setBit(index, isWin);
            updated = true;
         }
      } // for
   } // while

   // Dump to output

   std::cout << std::endl;
   std::cout << "Dump of all output" << std::endl;

   // Loop over all positions.
   uint32_t cnt_invalid_index = 0;
   uint32_t cnt_illegal_board = 0;
   uint32_t cnt_unknown       = 0;
   uint32_t cnt_win           = 0;
   uint32_t cnt_loss          = 0;
   for (uint32_t index = 0; index < 0x01000000; ++index) {
      // First check if index is valid
      if (!indexIsValid(index)) {
         cnt_invalid_index++;
         continue;
      }

      // Now construct the board
      Board<cNumRows, cNumCols> board(index);

      // If the position is a WIN, then this is an illegal position.
      if (board.isWin()) {
         cnt_illegal_board++;
         continue;
      }

      // Skip positions that are unknown
      if (!known.readBit(index)) {
         cnt_unknown++;
         continue;
      }

      std::cout << std::setw(6) << std::hex << index << std::dec << " : ";
      std::cout << board.toShortString() << " : ";
      std::cout << std::setw(8) << std::hex << board.getPosition() << " " << std::dec;

      // Skip positions that are unknown
      if (tb.readBit(index)) {
         cnt_win++;
         std::cout << "WIN" << std::endl;
      } else {
         cnt_loss++;
         std::cout << "LOSS" << std::endl;
      }
   }

   std::cout << std::endl;
   std::cout << "Dump of statistics" << std::endl;
   std::cout << "Invalid Index    : " << cnt_invalid_index << std::endl;
   std::cout << "Illegal position : " << cnt_illegal_board << std::endl;
   std::cout << "Unknown value    : " << cnt_unknown       << std::endl;
   std::cout << "WIN              : " << cnt_win           << std::endl;
   std::cout << "LOSS             : " << cnt_loss          << std::endl;
   std::cout << std::endl;


   std::cout << "Starting position:";
   if (known.readBit(0xF0F00F)) {
      if (tb.readBit(0xF0F00F)) {
         std::cout << "WIN";
      } else {
         std::cout << "LOSS";
      }
   } else {
      std::cout << "Unknown";
   }
   std::cout << std::endl;



} // main

