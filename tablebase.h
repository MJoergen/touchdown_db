#ifndef _TOUCHDOWN_TABLEBASE_H
#define _TOUCHDOWN_TABLEBASE_H

#include <string>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

const ssize_t g_numPositions = 1<<24;

class TableBase
{
   public:
      // Default constructor clears the table.
      TableBase(const std::string& fileName)
      {
         m_fd = open(fileName.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // Read from existing file or create new.
         if (m_fd < 0)
         {
            perror("open");
            assert (false);
         }

         if (posix_fallocate(m_fd, 0, g_numPositions/8)) // Make sure new file has the right size
         {
            perror("fallocate");
            assert (false);
         }

         m_table = (uint8_t *) mmap(nullptr, g_numPositions/8, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
         if (m_table == MAP_FAILED) // Map the file to a pointer
         {
            perror("mmap");
            assert (false);
         }
      }

      ~TableBase()
      {
         munmap(m_table, g_numPositions/8);
         close(m_fd);
      }

      int readBit(uint32_t pos) const {
         return (m_table[pos/8] >> (pos%8)) & 1;
      }

      void setBit(uint32_t pos, int val) {
         if (val)
            m_table[pos/8] |= (1 << (pos%8));
      }

   private:
      uint8_t *m_table;
      int      m_fd;

}; // TableBase

#endif // _TOUCHDOWN_TABLEBASE_H

