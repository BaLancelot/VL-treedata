#ifndef BUFFER_POOL_H_
#define BUFFER_POOL_H_

#include "disk_manager.h"
#include "lru_replacer.h"
#include "page.h"

#include <cstdint>
#include <list>
#include <map>

constexpr FrameID INVALID_FRAME_ID = UINT16_MAX;

/**
 * The BufferPool class manages a pool of pages in memory, providing
 * functionality to fetch, flush, create, delete, and unpin pages while
 * maintaining a mapping of PageIDs to their corresponding frame indexes.
 * It utilizes a Least Recently Used (LRU) replacement strategy and
 * interacts with a DiskManager for disk operations, ensuring efficient memory
 * management and potential support for multithreading.
 * 
 *   BufferPool is responsible for:
 *   - Fetching existing pages from disk into memory.
 *   - Creating frames for newly allocated pages.
 *   - Tracking which PageID occupies each frame.
 *   - Tracking page pin counts.
 *   - Flushing dirty pages to DiskManager.
 *   - Selecting unpinned frames for replacement through LRUReplacer.
 *   - Returning unused frames to the free-frame list.
 * 
 * [!] Class owner is responsible for explicitly unpinning the pages it is not
 * using anymore. 
 * 
 * Thread safety: Currently NOT thread-safe.
 */

class BufferPool {
 public:

  /**
   * Creates a buffer pool containing pool_size frames. Initially every frame
   * is free and no pages are resident.
   * 
   * @param pool_size number of frames available in memory.
   * @param disk_manager DiskManager class used for persistent page I/O and
   *                     PageID allocation. DiskManager must outlive BufferPool.
   * @throws std::runtime_error if pool_size cannot be represented by FrameID.
   */
  BufferPool(size_t pool_size, DiskManager& disk_manager);

  /**
   * Flushes dirty resident pages and releases BufferPool memory.
   */
  ~BufferPool();

  /**
   * Fetches an existing page into memory and pins it.
   * 
   * If the page is already in the pool (resident):
   *   - increments its pin count
   *   - removes its frame from LRU replacement eligibility
   * Otherwise:
   *   - uses free frame if available, otherwise evicts the LRU available frame
   *   - if eviction happens, flushes the victim page if dirty
   *   - reads the requested page from DiskManager
   *   - loads the page into a selected frame with pin_count == 1
   * 
   * @param page_id PageID to fetch
   * @return pointer to the pinned resident page, or nullptr if no frame can
   *         be obtained (for instance because every frame is pinned).
   *
   * @note Every successfull Fetch call must be eventually paired with UnpinPage()
   */
  Page* FetchPage(PageID page_id);

  /**
   * Writes the resident page to disk immediately.
   * Flushing does not affect page's pin count and does not remove it from
   * the buffer pool.
   * On success, the page is marked clean.
   * 
   * @param page_id PageID to flush
   * @return true if the page was resident and successfully flushed,
   *         false if PageID is invalid or not currently resident.
   */
  bool FlushPage(PageID page_id);

  /**
   * Writes every dirty resident page to disk and marks each one clean.
   * Pin counts and replacement eligibility remains unchanged.
   */
  void FlushAllPages();

  /**
   * Allocates a new persistent PageID and loads a new page into a frame.
   * Obtains either a free frame, or an unpinned LRU victim. A dirty victim
   * is flushed and its data reset afterwards before the reuse.
   *
   * @param page_out output parameter receiving the newly allocated PageID.
   * @return pointer to the newly created pinned page, or nullptr if every frame
   *         is pinned and no frame can be obtained.
   *
   * @note The returned page has one pin and must eventually be UnpinPage()d.
   */
  Page* NewPage(PageID* page_out);

  /**
   * Deallocates a persistent page.
   * 
   * If the page is resident and pinned, deletion fails.
   * If the page is resident and unpinned:
   *   - removes it from LRUReplacer
   *   - removes its page_pool_ entry
   *   - returns its frame to free_frames_
   * If the page is not resident of BufferPool, no buffer cleanup is neccessary.
   * 
   * In either case, DiskManager is called to deallocate PageID marking the
   * page as free to reuse (effectively deleting it).
   * 
   * @param page_id PageID to deallocate.
   * @return true if deletion succeeds, false if page is pinned.
   * @throws std::runtime_error if given PageID is invalid.
   */
  bool DeletePage(PageID page_id);

  /**
   * Releases one logical pin held on a resident page.
   * If dirty flag is true, the page becomes dirty. If passed dirty is false,
   * an already dirty page remains dirty.
   * 
   * When the pin count transitions from 1 to 0, the frame becomes eligible for
   * replacement and is added to LRUReplacer as the most recently used frame.
   * 
   * @param page_id PageID for page whose pin is to be released
   * @param dirty flag whether the caller modified page while holding this pin.
   * @return false if pin_count is already 0 or page is not resident,
   *         true otherwise.
   */
  bool UnpinPage(PageID page_id, bool dirty);

 private:
  size_t pool_size_;

  // Mapping of PageIDs to indexes of pool array
  std::map<PageID, FrameID> page_pool_;

  // List of free pool indexes 
  std::list<FrameID> free_frames_;

  // heap allocated array of pool pages
  Page* pages_;

  // DiskManager reference (owner should create it)
  DiskManager& disk_manager_;

  // LeastRecentlyUsed responsible class
  LRUReplacer lru_;

  // Potential lock for multithreading support?
};

#endif  // BUFFER_POOL_H_
