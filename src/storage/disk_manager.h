#ifndef DISK_MANAGER_H_
#define DISK_MANAGER_H_

#include "page.h"

#include <fstream>
#include <filesystem>
#include <vector>

 /**
 * DiskManager owns the database file stream and provides fixed-page disk I/O
 * together with persistent PageID allocation/deallocation.
 * DiskManager does not interpret page contents. It does not know what type
 * the page is.
 * 
 * It relies on the saved in database's file metadata to utilize 
 * reusable PageIDs.
 * 
 * [!] Until Database loads DiskManager related state to DiskManager,
 * its functionality is restricted to only reading PageID 0 (METADATA).
 * 
 * Initial workflow: 
 *  1. Class owner (database) constructs DiskManager using database file path.
 *  2. At this point created DiskManager can be used ONLY for reading PageID 0
 *     which is a METADATA Page. Database class reads PageID 0, parses fields
 *     and metadata structures.
 *  3. Using saved metadata state, load it to DiskManager via
 *     SetMetadataState(). After doing so, DiskManager is now up to date and
 *     is allowed to use full functionality.
 * 
 * Thread safety: Currently NOT thread-safe.
 */

class DiskManager {
 public:
  /**
   * Opens an existing database file for binary input and output.
   * The stream remains open the entire lifetime of DiskManager.
   * 
   * @param file path to the database file.
   * @throws std::runtime_error if the file cannot be opened.
   */
  explicit DiskManager(std::filesystem::path file);

  /**
   * Restores PageID allocator metadata state previously loaded from 
   * database metadata. After this operation, normal DiskManager
   * functionality becomes available.
   * 
   * @param reusable_page_ids previously deallocated PageIDs available
   *                          for reuse.
   * @param next_page_id next PageID that has never yet been allocated.
   * 
   * Expected invarients:
   *  - next_page_id != INVALID_PAGE_ID
   *  - next_page_id >= 1
   *  - reusable IDs are nonzero
   *  - reusable IDs are < next_page_id
   *  - reusable IDs contain no duplicates (consider swapping to unordered_set)
   */
  void RestoreMetadataState(const std::vector<PageID>& reusable_page_ids,
                            PageID next_page_id);
	
  /**
   * Returns first PageID that has never been allocated yet.
   * Used by the database metadata layer when persisting allocator state.
   */                          
  PageID GetNextPageID() const;

  /**
   * Returns the collection of currently reusable PageIDs.
   * Used by the database metadata layer when persisting allocator state.
   */
  const std::vector<PageID>& GetReusablePageIDs() const;
  
  /**
   * Writes exactly PAGE_SIZE bytes from page to its PageID-derived file offset.
   * DiskManager writes the raw serialized bytes already contained in Page
   * and doesn't perform page-format interpretation.
   * 
   * Not allowed to be called until persisted allocator state is restored.
   * 
   * @param page_id reference to a page data of which to write to database file.
   * @throws std::runtime_error if metadata has not been restored yet,
   *         seeking within file or the write fails.
   */
  void WritePage(const Page& page_id);

  /**
   * Reads exactly PAGE_SIZE bytes from database file at PageID-derived file
   * offset and returns them in a Page.
   * Before metadata restoration, only PageID 0 may be read so that
   * database may recover persisted allocator state.
   * 
   * @param page_id PageID to read.
   * @return Page containing the bytes stored at the PageID's db file offset.
   * @throws std::runtime_error if the PageID may not currently be read,
   *         seeking fails, or page cannot be read in full.
   */
  Page ReadPage(PageID page_id);

  /**
   * Deallocates given PageID and makes its db file offset available for 
   * future reuse. This operation doesn't truncate the database file and does
   * not clear old bytes currently stored at page's physical file location.
   * 
   * @param page_id PageID of page to delete
   * @throws std::runtime_error for reserved, invalid, never-allocated, or
   *         already deallocated PageIDs.
   */
  void DeletePage(PageID page_id);

  /**
   * Allocates a PageID for a new page. Reuses one of previously deallocated
   * PageIDs if such exist, otherwise returns next_page_id_ and increments it.
   * 
   * This operation allocates only PageID. It does not initialize or write
   * bytes.
   * 
   * @return PageID for a new page to be created.
   */
  PageID AllocatePageID();

 private:
  PageID next_page_id_ = INVALID_PAGE_ID;

  // In case deletions happened, reuse PageIDs (and therefore database offsets).
  // [Consider switching to unordered_set for a O(1) deletion (no duplicates)]
  std::vector<PageID> reusable_page_ids_;

  std::filesystem::path db_file_path_;

  std::fstream db_file_;

  // Safeguard DiskReader functionality to prevent misuse.
  // (Specifically, using non-reading functionality before loading metadata
  // state).
  bool metadata_initialized_ = false;
};

#endif  // DISK_MANAGER_H_
