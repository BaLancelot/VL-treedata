#ifndef BUFFER_POOL_H_
#define BUFFER_POOL_H_

#include "page.h"

#include <map>

class BufferPool {
 public:
  

 private:
  uint16_t num_pages;
  std::map<PageID, Page> page_pool;
};

#endif  // BUFFER_POOL_H_
