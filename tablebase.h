#ifndef _TOUCHDOWN_TABLEBASE_H
#define _TOUCHDOWN_TABLEBASE_H

#include <vector>

class TableBase
{
   public:
      // Default constructor clears the table.
      TableBase() {m_table.assign(1<<24, 0);}

      int readBit(uint32_t pos) const {
         return (m_table[pos/8] >> (pos%8)) & 1;
      }

      void setBit(uint32_t pos, int val) {
         if (val)
            m_table[pos/8] |= (1 << (pos%8));
      }

   private:
      std::vector<uint8_t> m_table;
}; // TableBase

#endif // _TOUCHDOWN_TABLEBASE_H

