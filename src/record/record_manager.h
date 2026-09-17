#ifndef RECORD_MANAGER_H_
#define RECORD_MANAGER_H_

#include "../storage/page.h"

#include <cstdint>
#include <map>

// Record manager is responsible for giving a PageID of RecordPage that has
// enough contiguous free space for a requested size to fit.
// Caller is responsible to update the maximum record fit size of returned by
// RecordManager PageID whenever they edit corresponding page.

// Issue: this is in-memory structure. After database process restarts,
// current mapping will be lost, therefore new database run will forget all
// existing pages' maximum size info.
//
// Potential solution: dedicate space for the RecordManager data in METADATA
// type page, and write to it on program's exit, read from it on start.
// Issue #2: in later stages such as crash recovery efforts, this idea won't
// work, since there won't be a write to metadata page if crash happens.
class RecordManager {
 public:
  // If no fitting Page is found, returns INVALID_PAGE_ID
  PageID GetFitPageID(size_t data_size) const;

  // Does both Insert and Update
  void SetPageFreeSpaceInfo(PageID page, uint16_t new_free_space);

  // TBD if needed.
  void DeletePageFreeSpaceInfo(PageID page);

 private:
 // Maps PageID to maximum record size that can fit
 // (potential metadata overhead considered)
  std::map<PageID, uint16_t> page_to_space_;
};

#endif  // RECORD_MANAGER_H_
