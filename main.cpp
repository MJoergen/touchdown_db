#include <iostream>
#include <iomanip>
#include <unistd.h>
#include "tablebase.h"
#include "board.h"
#include "index.h"

const int cNumRows = 4;
const int cNumCols = 4;


static void dumpAllValidIndices()
{
   for (uint32_t index = 0; index < 0x01000000; ++index) {
      if (!indexIsValid(index)) {
         continue;
      }
      std::cout << std::hex << std::setfill('0') << std::setw(6) << index << std::endl;
   }
} // dumpAllValidIndices

static void dumpAllLegalBoards()
{
   for (uint32_t index = 0; index < 0x01000000; ++index) {
      if (!indexIsValid(index)) {
         continue;
      }
      Board<cNumRows, cNumCols> board(index);
      assert (board.getIndex() == index);

      std::cout << std::hex << std::setfill('0') << std::setw(6) << index << " : " << board.toShortString() << " ";
      if (board.isWin())
         std::cout << "INVALID" << std::endl;
      else if (board.isLoss()) 
         std::cout << "LOSS" << std::endl;
      else 
         std::cout << std::endl;
   }
} // dumpAllLegalBoards

// Dump database to output
static void dumpDatabase(const char *filename)
{
   TableBase tb(filename);

   // Loop over all positions.
   uint32_t cnt_invalid_index = 0;
   uint32_t cnt_illegal_board = 0;
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
} // dumpDatabase

static void summarizeDatabase(const char *filename)
{
   TableBase tb(filename);

   // Loop over all positions.
   uint32_t cnt_invalid_index = 0;
   uint32_t cnt_illegal_board = 0;
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
      if (tb.readBit(index)) {
         cnt_win++;
      } else {
         cnt_loss++;
      }
   }

   std::cout << std::endl;
   std::cout << "Dump of statistics"   << std::endl;
   std::cout << "Invalid Index     : " << std::setw(8) << cnt_invalid_index << std::endl;
   std::cout << "Illegal position  : " << std::setw(8) << cnt_illegal_board << std::endl;
   std::cout << "Win               : " << std::setw(8) << cnt_win           << std::endl;
   std::cout << "Loss              : " << std::setw(8) << cnt_loss          << std::endl;
   std::cout << std::endl;


   std::cout << "Starting position : ";
   if (tb.readBit(0xF0F00F)) {
      std::cout << "Win";
   } else {
      std::cout << "Loss";
   }
   std::cout << std::endl;
}

static void generateDatabase(const char *filename)
{
   // The initial values of the tablebase is that all positions are unknown.
   TableBase tb(filename);
   TableBase known("/tmp/touchdown.known");

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

         // Now we loop over all legal moves
         // IF any successor leads to an unknown position, then this position is unknown too.
         // If any successor leads to a LOSS (for the opponent), then this position is a WIN.
         // If all successors lead to a WIN (for the opponent), then this position is a LOSS.
         // If no successor available, then this position is a LOSS.
         bool isKnown = true;    // Assume position is known.
         bool isWin   = false;   // Assume position is a LOSS, e.g. if no successors.

         uint32_t legalMoves[12];
         int moveCount = board.writeLegalMoves(legalMoves);

         // Loop over all squares
         for (int i=0; i<moveCount; ++i) {
            board.setPosition(legalMoves[i]);
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
         } // end for

         if (isKnown) {
            known.setBit(index, true);
            tb.setBit(index, isWin);
            updated = true;
         }
      } // for
   } // while

} //  static void generateDatabase(const char *filename)


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
   while ((c = getopt(argc, argv, "hicld:s:")) != -1) {
      switch (c)
      {
         case 'h' :
            std::cout << "Usage: touchdown_db <options> db_file_name" << std::endl;
            std::cout << "Options:"                                   << std::endl;
            std::cout << "-h : Print this help."                      << std::endl;
            std::cout << "-i : Test index calculation."               << std::endl;
            std::cout << "-c : Dump all valid positions."             << std::endl;
            std::cout << "-l : Dump all legal boards."                << std::endl;
            std::cout << "-d : Dump existing database."               << std::endl;
            std::cout << "-s : Summarize existing database."          << std::endl;
            return 0;
         case 'i' : indexTest(); return 0;
         case 'c' : dumpAllValidIndices(); return 0;
         case 'l' : dumpAllLegalBoards(); return 0;
         case 'd' : dumpDatabase(optarg); return 0;
         case 's' : summarizeDatabase(optarg); return 0;
         default  : abort ();
      }
   } // while

   if (optind >= argc) {
      std::cout << "Missing database filename" << std::endl;
      return 1;
   }

   generateDatabase(argv[optind]);
} // main

