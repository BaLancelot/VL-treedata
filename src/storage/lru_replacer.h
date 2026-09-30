#ifndef LRU_REPLACER_H_
#define LRU_REPLACER_H_

#include "page.h"

#include <cstddef>
#include <list>
#include <unordered_map>

using FrameID = uint16_t;

/**
 * LRUReplacer tracks Buffer Pool frames that are currently eligible for
 * eviction and orders them using a Least Recently Used policy.
 * 
 * LRUReplacer does NOT maintain page pin counts. BufferPool owns pin count
 * state and informs LRUReplacer whenever a frame transitions between
 * evictable and non-evictable states.
 * 
 * Invariants:
 *  - Every FrameID appears at most once in lru_list_.
 *  - lru_map_ and lru_list contain exactly same FrameIDs
 *  - Every iterator stored in lru_map_ points to its corresponding lru_list
 *    node
 *  - Every frame contained by LRUReplacer is expected by BufferPool to have
 *    pin_count == 0.
 *  - Size() never exceeds capacity_.
 */
class LRUReplacer {
 public:
  /**
   * Creates an empty replacer of given capacity
   * 
   * @param pool_capacity maximum number of frames that may be tracked.
   */
  explicit LRUReplacer(size_t pool_capacity)
      : capacity_(pool_capacity) {}

  ~LRUReplacer() = default;

  /**
   * Removes the least recently used evictable frame.
   * 
   * @param frame_id output parameter receiving selected FrameID.
   * @return true if a victim was found; false if no evictable frames exist.
   */
  bool Evict(FrameID* frame_id);

  /**
   * Returns the number of frames currently eligible for eviction.
   */
  size_t Size() const;

  /**
   * Marks a frame as non-evictable. If the frame is currently tracked by
   * the replacer, it is removed. If it is already absent, no-op.
   * 
   * @param frame_id FrameID of frame to pin.
   */
  void Pin(FrameID frame_id);

  /**
   * Marks a frame as eligible for eviction. If the frame is not already
   * tracked, it is inserted at the front of LRU list as the most recently
   * used evictable frame. If this frame is already trackable, no-op.
   * 
   * @param frame_id FrameID of frame whose BufferPool-managed pin count
   *                 reached zero.
   * @throws std::logic_error if inserting the frame would exceed capacity_,
   *         meaning that invariant is broken between BufferPool and LRUReplacer
   */
  void Unpin(FrameID frame_id);

 private:
  size_t capacity_;
  // Front of list - MRU, back of list - LRU
  std::list<FrameID> lru_list_;

  // Map for finding a specific FrameID within the map in O(1) avg time.
  // Used mainly by pins/unpins.
  std::unordered_map<FrameID, std::list<FrameID>::iterator> lru_map_;
};

#endif  // LRU_REPLACER_H_
