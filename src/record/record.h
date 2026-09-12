#ifndef RECORD_H_
#define RECORD_H_

#include <cstdint>

using PageID = uint32_t;

// Represents a record that is mapped with key and stored in pages.
struct RecordID {
  PageID page_id;
  uint16_t slot;
};

#endif  // RECORD_H_