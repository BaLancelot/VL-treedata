#include "bplus_leaf_page.h"
#include "../record/record.h"

#include <cstring>
#include <memory>

// [METADATA REGION responsible functionality]

PageType BPlusLeafPage::GetType() const {
  PageType type;
  std::memcpy(&type, page_.GetData() + TYPE_OFFSET, sizeof(PageType));

  return type;
}

void BPlusLeafPage::SetType(PageType type) {
  std::memcpy(page_.GetData() + TYPE_OFFSET, &type, sizeof(PageType));
}

uint16_t BPlusLeafPage::GetEntryCount() const {
  uint16_t size;
  std::memcpy(&size, page_.GetData() + ENTRY_COUNT_OFFSET, sizeof(uint16_t));

  return size;
}

void BPlusLeafPage::SetEntryCount(uint16_t count) {
  std::memcpy(page_.GetData() + ENTRY_COUNT_OFFSET, &count, sizeof(uint16_t));
}

PageID BPlusLeafPage::GetParentPageID() const {
  PageID parent;
  std::memcpy(&parent, page_.GetData() + PARENT_PAGE_ID_OFFSET, sizeof(PageID));

  return parent;
}

void BPlusLeafPage::SetParentPageID(PageID page_id) {
  std::memcpy(page_.GetData() + PARENT_PAGE_ID_OFFSET, &page_id, sizeof(PageID));
}

PageID BPlusLeafPage::GetNextLeafPageID() const {
  PageID next_leaf;
  std::memcpy(&next_leaf, page_.GetData() + NEXT_LEAF_PAGE_ID_OFFSET,
      sizeof(PageID));

  return next_leaf;
}

void BPlusLeafPage::SetNextLeafPageID(PageID next) {
  std::memcpy(page_.GetData() + NEXT_LEAF_PAGE_ID_OFFSET, &next, sizeof(PageID));
}

PageID BPlusLeafPage::GetPreviousLeafPageID() const {
  PageID prev_leaf;
  std::memcpy(&prev_leaf, page_.GetData() + PREV_LEAF_PAGE_ID_OFFSET,
      sizeof(PageID));

  return prev_leaf;
}

void BPlusLeafPage::SetPreviousLeafPageID(PageID prev) {
  std::memcpy(page_.GetData() + PREV_LEAF_PAGE_ID_OFFSET, &prev, sizeof(PageID));
}

// [LEAF ENTRY responsible functionality]

Key BPlusLeafPage::GetEntryKey(uint16_t entry_index) const {
  Key result;
  std::memcpy(&result, page_.GetData() 
         + HEADER_SIZE + entry_index * LEAF_ENTRY_SIZE, sizeof(Key));

  return result;
}

RecordID BPlusLeafPage::GetEntryRecordID(uint16_t entry_index) const {
  RecordID record;
  size_t record_offset = HEADER_SIZE
                       + entry_index * LEAF_ENTRY_SIZE
                       + sizeof(Key);

  std::memcpy(&record.page_id, page_.GetData() + record_offset, sizeof(PageID));
  std::memcpy(&record.slot, page_.GetData() + record_offset + sizeof(PageID),
      sizeof(uint16_t));

  return record;
}

LeafEntry BPlusLeafPage::GetEntry(uint16_t entry_index) const {
  LeafEntry entry;

  entry.key = GetEntryKey(entry_index);
  entry.record = GetEntryRecordID(entry_index);

  return entry;
}

void BPlusLeafPage::SetEntryKey(uint16_t entry_index, Key key) {
  size_t entry_key_offset = HEADER_SIZE + entry_index * LEAF_ENTRY_SIZE;

  std::memcpy(page_.GetData() + entry_key_offset, &key, sizeof(Key));
}

void BPlusLeafPage::SetEntryRecordID(uint16_t entry_index, RecordID record) {
  size_t entry_record_offset = HEADER_SIZE
                             + entry_index * LEAF_ENTRY_SIZE
                             + sizeof(Key);

  std::memcpy(page_.GetData() + entry_record_offset,
      &record.page_id, sizeof(PageID));
  std::memcpy(page_.GetData() + entry_record_offset + sizeof(PageID),
      &record.slot, sizeof(uint16_t));
}

void BPlusLeafPage::SetEntry(uint16_t entry_index, const LeafEntry& entry) {
  SetEntryKey(entry_index, entry.key);
  SetEntryRecordID(entry_index, entry.record);
}
