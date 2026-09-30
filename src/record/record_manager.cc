#include "record_manager.h"

#include <stdexcept>

void RecordManager::RestoreMetadataState(std::map<PageID, uint32_t>& metadata) {
  page_to_space_ = metadata;
}

const std::map<PageID, uint32_t>& RecordManager::GetMetadataState() const {
  return page_to_space_;
}

PageID RecordManager::GetFitPageID(size_t data_size) const {
  for (const auto& pair : page_to_space_) {
    if(data_size <= pair.second) {
      return pair.first;
    }
  }

  return INVALID_PAGE_ID;
}

void RecordManager::SetPageFreeSpaceInfo(PageID page,
                                         uint32_t new_free_space) {
  page_to_space_[page] = new_free_space;
}

void RecordManager::DeletePageFreeSpaceInfo(PageID page) {
  if (page_to_space_.find(page) == page_to_space_.end()) {
    throw std::runtime_error("No such PageID in RecordManager.");
  }

  page_to_space_.erase(page);
}
