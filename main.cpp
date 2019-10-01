#include <iostream>
#include <ostream>
#include <vector>
#include <assert.h>
#include "tablebase.h"
#include "board.h"
#include "hash.h"

const int cNumRows = 4;
const int cNumCols = 4;

// This program creates a database over ALL legal positions on the 4x4 touchdown
// board, and calculates the game theoretic value for each position.

// The algorithm used performs multiple loops over all legal - but still
// unknown - positions.  For each position, it tries all legal moves. If all
// legal moves leads to currently known positions, then this position is
// considered known too, and the tablebase is updated.

int main() {
   assert(cNumRows==4 && cNumCols == 4);

   testHash();
   return 0;

   // The initial values of the tablebase is that all positions are unknown.
   TableBase tb;
   TableBase known;

   bool allKnown = false;
   // Repeat until all positions are known
   while (!allKnown) {
      std::cout << "." << std::flush;  // Output current progress.
      allKnown = true;  // Assume all positions known now.

      // Loop over all positions.
      for (uint32_t hash = 0; hash < 0x01000000; ++hash) {
         // Skip positions that are already known
         if (known.readBit(hash))
            continue;

         // We've reached an unknown position.
         // This forces another round in the outer loop.
         allKnown = false;
         allKnown = true;   // TBD: REMOVE

         bool isKnown = true;
         bool isWin   = true; // This is the value, if the board is dead.

         // For each position, it tries all legal moves. If all
         // legal moves leads to currently known positions, then this position is
         // considered known too, and the tablebase is updated.

         if (isHashLegal(hash)) {
            Board<cNumRows, cNumCols> board(hash);
            if (!board.isWin()) {
               isWin   = false;
               isKnown = false;

               if (board.isLoss()) {
                  isKnown = true;
               }

#if 0
               // Loop over all legal moves.
               for (int sq=cNumCols; sq<cNumRows*cNumCols; ++sq) {
                  int row = sq / cNumCols;
                  int col = sq % cNumCols;

                  if (board.getPiece(row, col) == 1) {
                     if (board.getPiece(row-1, col) == 0) {
                        // try move straight up

                        uint32_t newHash; // TBD: How to calculate new hash ??? We need to swap board around too.

                        if (!known.readBit(newHash))
                        {
                           // If one child is unknown, then we can stop immediately.
                           isKnown = false;
                           break;
                        }
                        if (!tb.readBit(newHash))
                        {
                           // If one child is lost, then we are winning, and can stop immediately.
                           isWin = true;
                           break;
                        }
                     }

                     if (board.getPiece(row-1, col-1) == -1) {
                        // try move up and left
                     }

                     if (board.getPiece(row-1, col+1) == -1) {
                        // try move up and right
                     }
                  } // if
               } // for
#endif

            } // if (!board.isWin()) {
         } // if (isHashLegal(hash)) {

         if (isKnown) {
            known.setBit(hash, true);
            tb.setBit(hash, isWin);
            allKnown = false;   // TBD: REMOVE
         }
      } // for
   } // while

   std::cout << std::endl;

   // Dump statistics
   uint32_t cnt_unknown      = 0;
   uint32_t cnt_illegalHash  = 0;
   uint32_t cnt_illegalBoard = 0;
   uint32_t cnt_lost         = 0;
   uint32_t cnt_legal        = 0;

   for (uint32_t hash = 0; hash < 0x01000000; ++hash) {
      if (!known.readBit(hash)) {   // Discard positions not known.
         cnt_unknown++;
         continue;
      }

      if (!isHashLegal(hash)) {     // Discard illegal hash values.
         cnt_illegalHash++;
         continue;
      }

      Board<cNumRows, cNumCols> board(hash);
      if (board.isWin()) {          // Discard illegal positions.
         cnt_illegalBoard++;
         continue;
      }
      if (board.isLoss()) {         // Discard lost positions.
         cnt_lost++;
         continue;
      }

      cnt_legal++;
   }

   std::cout << "cnt_unknown      " << cnt_unknown      << std::endl;
   std::cout << "cnt_illegalHash  " << cnt_illegalHash  << std::endl;
   std::cout << "cnt_illegalBoard " << cnt_illegalBoard << std::endl;
   std::cout << "cnt_lost         " << cnt_lost         << std::endl;
   std::cout << "cnt_legal        " << cnt_legal        << std::endl;
} // main

