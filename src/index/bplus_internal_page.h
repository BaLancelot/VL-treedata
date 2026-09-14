#ifndef BPLUS_INTERNAL_PAGE_H_
#define BPLUS_INTERNAL_PAGE_H_

#include "../storage/page.h"

using PageID = std::uint32_t;
using Key = std::uint64_t;

struct InternalEntry {
  Key key;
  PageID child;
};

class BPlusInternalPage {
 public:
  explicit BPlusInternalPage(Page& page) : page_(page) {}

  // Interface responsible for accessing / modifying metadata region

  uint16_t GetKeyCount() const;
  void SetKeyCount(uint16_t count);

  PageID GetParentPageID() const;
  void SetParentPageID(PageID page_id);

  PageID GetFirstChildPageID() const;
  void SetFirstChildPageID(PageID page_id);

  // Interface responsible for accessing / modifying entry region

  Key GetEntryKey(uint16_t entry_index) const;
  void SetEntryKey(uint16_t entry_index, Key key);

  PageID GetEntryChildPageID(uint16_t entry_index) const;
  void SetEntryChildPageID(uint16_t entry_index, PageID page_id);

  InternalEntry GetEntry(uint16_t entry_index) const;
  void SetEntry(uint16_t entry_index, const InternalEntry& entry);

 private:
  // Internal page metadata offsets

  static constexpr size_t TYPE_OFFSET = 0;
  static constexpr size_t KEY_COUNT_OFFSET = TYPE_OFFSET + sizeof(PageType);
  static constexpr size_t PARENT_PAGE_ID_OFFSET = KEY_COUNT_OFFSET 
                                                + sizeof(uint16_t);
  static constexpr size_t FIRST_CHILD_PAGE_OFFSET = PARENT_PAGE_ID_OFFSET
                                                  + sizeof(PageID);
  // Internal page entry related data

  static constexpr size_t HEADER_SIZE = FIRST_CHILD_PAGE_OFFSET
                                      + sizeof(PageID);
  static constexpr size_t INTERNAL_ENTRY_SIZE = sizeof(Key) + sizeof(PageID);

  Page& page_;
};

#endif  // BPLUS_INTERNAL_PAGE_H_
