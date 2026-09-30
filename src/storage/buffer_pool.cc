#include "buffer_pool.h"

#include <stdexcept>

BufferPool::BufferPool(size_t pool_size, DiskManager& disk_manager)
  : pool_size_(pool_size), disk_manager_(disk_manager), lru_(pool_size) {

  if (pool_size_ < 1 || pool_size_ > UINT16_MAX) {
    throw std::runtime_error("Invalid pool size.");
  }
  pages_ = new Page[pool_size_];

  // All frames are free initially
  for (size_t i = 0; i < pool_size_; i++) {
    free_frames_.push_back(static_cast<FrameID>(i));
  }
}

BufferPool::~BufferPool() {
  FlushAllPages();
  delete[] pages_;
}

Page* BufferPool::FetchPage(PageID page_id) {
  // Check if it is already in the pool
  if (page_pool_.find(page_id) != page_pool_.end()) {
    FrameID frame_id = page_pool_[page_id];
    Page* page = &pages_[frame_id];
    page->pin_count_++;
    lru_.Pin(frame_id);
    return page;
  }

  // Page is not in the buffer pool, find frame to put it
  FrameID frame_id = INVALID_FRAME_ID;
  if (!free_frames_.empty()) {
    frame_id = free_frames_.front();
    free_frames_.pop_front();
  } else {
    // Find a replacement frame
    if (!lru_.Evict(&frame_id)) {
      return nullptr;
    }

    Page* to_evict = &pages_[frame_id];

    if (to_evict->IsDirty()) {
      disk_manager_.WritePage(*to_evict);
    }

    page_pool_.erase(to_evict->GetPageID());
  }

  // Read the page from disk
  Page *page = &pages_[frame_id];
  *page = disk_manager_.ReadPage(page_id);

  page->pin_count_ = 1;
  page->is_dirty_ = false;

  page_pool_[page_id] = frame_id;
  lru_.Pin(frame_id);

  return page;
}

bool BufferPool::FlushPage(PageID page_id) {
  if (page_id == INVALID_PAGE_ID || page_pool_.find(page_id) == page_pool_.end()) {
    return false;
  }

  FrameID frame_id = page_pool_[page_id];
  Page *page = &pages_[frame_id];

  disk_manager_.WritePage(*page);
  page->is_dirty_ = false;

  return true;
}

Page* BufferPool::NewPage(PageID* page_id) {
  // 1. Check if there are any available frames
  FrameID frame_id = INVALID_FRAME_ID;
  if (!free_frames_.empty()) {
    frame_id = free_frames_.front();
    free_frames_.pop_front();
  } else {
    if (!lru_.Evict(&frame_id)) {
      return nullptr;
    }
        
    Page *to_evict = &pages_[frame_id];
    if (to_evict->IsDirty()) {
      disk_manager_.WritePage(*to_evict);
    }
    page_pool_.erase(to_evict->GetPageID());
  }

  // 2. Allocate new page on disk
  *page_id = disk_manager_.AllocatePageID();
    
  // 3. Initialize the frame
  Page *page = &pages_[frame_id];
  page->page_id_ = *page_id;
  page->pin_count_ = 1;
  page->is_dirty_ = true;  // Very likely will have modified data, so write.
  page->ResetData();

  page_pool_[*page_id] = frame_id;
  lru_.Pin(frame_id);

  return page;
}

bool BufferPool::DeletePage(PageID page_id) {
  if (page_id == 0 || page_id >= disk_manager_.GetNextPageID()) {
    throw std::runtime_error("Invalid PageID.");
  }

  if (page_pool_.find(page_id) == page_pool_.end()) {
    disk_manager_.DeletePage(page_id);  // no buffer cleanup is nessecary
    return true;
  }

  FrameID frame_id = page_pool_[page_id];
  Page* page = &pages_[frame_id];

  // Can't delete page that is in use
  if (page->pin_count_ > 0) {
    return false;
  }

  // Deallocate on disk
  disk_manager_.DeletePage(page_id);

  lru_.Pin(frame_id);  // Same as erasing from LRU
  page_pool_.erase(page_id);

  page->is_dirty_ = false;
  page->page_id_ = INVALID_PAGE_ID;
  page->pin_count_ = 0;

  // Add back to free list
  free_frames_.push_back(frame_id);

  return true;
}

void BufferPool::FlushAllPages() {
  for (const auto &pair : page_pool_) {
    FrameID frame_id = pair.second;
    Page *page = &pages_[frame_id];

    if (page->is_dirty_) {
      disk_manager_.WritePage(*page);
      page->is_dirty_ = false;
    }
  }
}

bool BufferPool::UnpinPage(PageID page_id, bool dirty) {
  if (page_pool_.find(page_id) == page_pool_.end()) {
    return false;
  }

  FrameID frame_id = page_pool_[page_id];
  Page* page = &pages_[frame_id];

  // Can't unpin not pinned page
  if (page->pin_count_ <= 0) {
    return false;
  }

  // Update dirty flag
  if (dirty) {
    page->is_dirty_ = true;
  }

  page->pin_count_--;
  if (page->pin_count_ == 0) {
    lru_.Unpin(frame_id);
  }

  return true;
}
