#include "lru_replacer.h"

bool LRUReplacer::Evict(FrameID* frame_id) {
  if (lru_list_.empty()) {
    return false;
  }

  *frame_id = lru_list_.back();
  lru_map_.erase(*frame_id);
  lru_list_.pop_back();

  return true;
}

void LRUReplacer::Pin(FrameID frame_id) {
  auto it = lru_map_.find(frame_id);

  if (it != lru_map_.end()) {
    lru_list_.erase(it->second);
    lru_map_.erase(it);
  }
}

void LRUReplacer::Unpin(FrameID frame_id) {
  // Page is already in replacer, no action needed.
  if (lru_map_.find(frame_id) != lru_map_.end()) {
    return;
  }

  if (lru_list_.size() >= capacity_) {
    throw std::logic_error("LRUReplacer capacity exceeded.");
  }

  lru_list_.push_front(frame_id);
  lru_map_[frame_id] = lru_list_.begin();
}

size_t LRUReplacer::Size() const {
  return lru_list_.size();
}