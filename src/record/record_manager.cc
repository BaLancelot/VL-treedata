#include "record_manager.h"

#include <stdexcept>

PageID RecordManager::GetFitPageID(size_t data_size) const {
  for (const auto& pair : page_to_space_) {
    if(data_size <= pair.second) {
      return pair.first;
    }
  }

  return INVALID_PAGE_ID;
}

void RecordManager::SetPageFreeSpaceInfo(PageID page,
                                         uint16_t new_free_space) {
  page_to_space_[page] = new_free_space;
}

void RecordManager::DeletePageFreeSpaceInfo(PageID page) {
  if (page_to_space_.find(page) == page_to_space_.end()) {
    throw std::runtime_error("No such PageID in RecordManager.");
  }

  page_to_space_.erase(page);
}
