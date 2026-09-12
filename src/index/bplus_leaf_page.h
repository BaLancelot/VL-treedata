#ifndef BPLUS_LEAF_PAGE_H_
#define BPLUS_LEAF_PAGE_H_

#include "../record/record.h"
#include "../storage/page.h"

using Key = std::uint64_t;

struct LeafEntry {
  Key key;
  RecordID record;
};

// Wrapper class which defines the type of the page used by database.
// Provides a leaf-node view over a Page.
class BPlusLeafPage {
 public:
  explicit BPlusLeafPage(Page& page) : page_(page) {}

  // Interface for accessing / modifying metadata region of leaf page

  PageType GetType() const;
  void SetType(PageType type);

  uint16_t GetEntryCount() const;
  void SetEntryCount(uint16_t count);

  PageID GetParentPageID() const;
  void SetParentPageID(PageID page_id);

  PageID GetNextLeafPageID() const;
  void SetNextLeafPageID(PageID page_id);

  PageID GetPreviousLeafPageID() const;
  void SetPreviousLeafPageID(PageID page_id);

  // Inteface for accessing / modifying entries region of leaf page
  
  // Returns key of the entry at given entry index.
  Key GetEntryKey(uint16_t entry_index) const;

  // Returns RecordID struct populated with PageID and slot number.
  RecordID GetEntryRecordID(uint16_t entry_index) const;

  // Returns fully populated LeafEntry struct containing Key and RecordID
  LeafEntry GetEntry(uint16_t entry_index) const;

  void SetEntryKey(uint16_t entry_index, Key key);
  void SetEntryRecordID(uint16_t entry_index, RecordID record);
  void SetEntry(uint16_t entry_index, const LeafEntry& entry);
  
 private:
  // Leaf page metadata offsets

  static constexpr size_t TYPE_OFFSET = 0;
  static constexpr size_t ENTRY_COUNT_OFFSET = TYPE_OFFSET + sizeof(PageType);
  static constexpr size_t PARENT_PAGE_ID_OFFSET = ENTRY_COUNT_OFFSET
                                                + sizeof(uint16_t);
  static constexpr size_t NEXT_LEAF_PAGE_ID_OFFSET = PARENT_PAGE_ID_OFFSET
                                                   + sizeof(PageID);
  static constexpr size_t PREV_LEAF_PAGE_ID_OFFSET = NEXT_LEAF_PAGE_ID_OFFSET
                                                   + sizeof(PageID);

  // Leaf page entry related data

  static constexpr size_t HEADER_SIZE = PREV_LEAF_PAGE_ID_OFFSET
                                      + sizeof(PageID);
  static constexpr size_t LEAF_ENTRY_SIZE = sizeof(Key) + sizeof(PageID)
                                          + sizeof(uint16_t);

  Page& page_;
};

#endif  // BPLUS_LEAF_PAGE_H_
