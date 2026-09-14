#include "bplus_internal_page.h"

#include <cstring>

// [METADATA region functionality]

uint16_t BPlusInternalPage::GetKeyCount() const {
  uint16_t key_count;

  std::memcpy(&key_count, page_.GetData() + KEY_COUNT_OFFSET, sizeof(uint16_t));
  return key_count;
}

void BPlusInternalPage::SetKeyCount(uint16_t key) {
  std::memcpy(page_.GetData() + KEY_COUNT_OFFSET, &key, sizeof(uint16_t));
}

PageID BPlusInternalPage::GetParentPageID() const {
  PageID parent_page;

  std::memcpy(&parent_page, page_.GetData() + PARENT_PAGE_ID_OFFSET,
      sizeof(PageID));
  return parent_page;
}

void BPlusInternalPage::SetParentPageID(PageID parent) {
  std::memcpy(page_.GetData() + PARENT_PAGE_ID_OFFSET, &parent, sizeof(PageID));
}

PageID BPlusInternalPage::GetFirstChildPageID() const {
  PageID first_child;
  
  std::memcpy(&first_child, page_.GetData() + FIRST_CHILD_PAGE_OFFSET,
      sizeof(PageID));
  return first_child;
}

void BPlusInternalPage::SetFirstChildPageID(PageID page_id) {
  std::memcpy(page_.GetData() + FIRST_CHILD_PAGE_OFFSET, &page_id, sizeof(PageID));
}

// [ENTRY region functionality]
Key BPlusInternalPage::GetEntryKey(uint16_t entry_index) const {
  Key key;
  size_t offset = HEADER_SIZE + entry_index * INTERNAL_ENTRY_SIZE;

  std::memcpy(&key, page_.GetData() + offset, sizeof(Key));
  return key;
}

void BPlusInternalPage::SetEntryKey(uint16_t entry_index, Key key) {
  size_t offset = HEADER_SIZE + entry_index * INTERNAL_ENTRY_SIZE;

  std::memcpy(page_.GetData() + offset, &key, sizeof(Key));
}

PageID BPlusInternalPage::GetEntryChildPageID(uint16_t entry_index) const {
  PageID child;
  size_t child_page_id_offset = HEADER_SIZE + entry_index * INTERNAL_ENTRY_SIZE
                              + sizeof(Key);
  
  std::memcpy(&child, page_.GetData() + child_page_id_offset,
      sizeof(PageID));
  return child;
}

void BPlusInternalPage::SetEntryChildPageID(uint16_t entry_index,
                                            PageID page_id) {
  size_t child_page_id_offset = HEADER_SIZE + entry_index * INTERNAL_ENTRY_SIZE
                              + sizeof(Key);
  
  std::memcpy(page_.GetData() + child_page_id_offset, &page_id, sizeof(PageID));
}

InternalEntry BPlusInternalPage::GetEntry(uint16_t entry_index) const {
  InternalEntry entry;

  entry.key = GetEntryKey(entry_index);
  entry.child = GetEntryChildPageID(entry_index);
  return entry;
}

void BPlusInternalPage::SetEntry(uint16_t entry_index,
                                 const InternalEntry& entry) {
  SetEntryKey(entry_index, entry.key);
  SetEntryChildPageID(entry_index, entry.child);
}
