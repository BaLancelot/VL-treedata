#ifndef RECORD_PAGE_H_
#define RECORD_PAGE_H_

#include "../storage/page.h"

#include <cstddef>
#include <cstdint>
#include <vector>

struct RecordSlot {
  uint16_t data_offset;
  uint16_t data_size;
};

// Page that stores ACTUAL data of the database. Its responsibility is to
// find / create slot, calculate offset for variable-size record data and
// insert it. For deletion, changes the metadata of slot record to invalid
// state, compacts the data to eliminate data holes, updates the offset metadata
// of remaining slots to new offset values.
//
// Consists of metadata header on top, followed by slot records consisting of
// slot -> page offset, and record data itself growing from the bottom of page
// according to offsets described in slot records.
// 
//
// @TODO: Create a RecordManager class responsible for finding page with
// enough free space for a record. This way, before deciding to allocate new
// page, there is an opportunity to use existing free space.
class RecordPage {
 public:
  explicit RecordPage(Page& page) : page_(page) {};

  // Potentially for future record manager to set metadata of newly created
  // record page to initial values
  void Initialize();

  uint16_t GetSlotCount() const;  // represents total number of ever used slots
  void SetSlotCount(uint16_t slots);

  uint16_t GetLiveRecordCount() const; // represents ACTUAL count (i.e. deletes)
  void SetLiveRecordCount(uint16_t count);

  // I.e. offset for next record's *data*

  uint16_t GetFreeSpaceEnd() const;
  void SetFreeSpaceEnd(uint16_t offset);

  // Slot access

  uint16_t GetSlotOffset(uint16_t slot_index) const;         // from metadata
  void SetSlotOffset(uint16_t slot_index, uint16_t offset);

  uint16_t GetSlotLength(uint16_t slot_index) const;         // from metadata
  void SetSlotLength(uint16_t slot_index, uint16_t length);

  RecordSlot GetSlot(uint16_t slot_index) const;
  void SetSlot(uint16_t slot_index, const RecordSlot& slot);

  // Checks if existing slot has valid values (length != 0 -> free)
  bool IsSlotFree(uint16_t slot_index) const;

  // Record operations

  // Checks if given record data can fit (including the metadata overhead part)
  bool CanFit(size_t record_size) const;

  std::vector<std::byte> GetRecordData(uint16_t slot_index) const;

  uint16_t Insert(const std::vector<std::byte>& record);

  // Sets the given slot's metadata to invalid state, therefore marking it free
  void Delete(uint16_t slot_index);

  // @TODO :)
  void RecordPage::Update(uint16_t slot_index,
                          const std::vector<std::byte>& record);

 private:
  // Returns slot index of found free slot metadata, otherwise UINT16_MAX
  uint16_t FindFreeSlot() const;

  // Called during deletion to compact data, preventing data holes
  void Compact(const RecordSlot& deleted_slot);

  // METADATA offsets
  
  static constexpr size_t TYPE_OFFSET = 0;
  static constexpr size_t SLOT_COUNT_OFFSET = 0 + sizeof(PageType);
  static constexpr size_t LIVE_RECORD_COUNT_OFFSET = SLOT_COUNT_OFFSET
                                                   + sizeof(uint16_t);
  static constexpr size_t FREE_SPACE_END_OFFSET = LIVE_RECORD_COUNT_OFFSET
                                                + sizeof(uint16_t);

  static constexpr size_t HEADER_SIZE = FREE_SPACE_END_OFFSET
                                      + sizeof(uint16_t);
  static constexpr size_t SLOT_SIZE = sizeof(uint16_t) + sizeof(uint16_t);

  static constexpr uint16_t INVALID_SLOT = UINT16_MAX;

  Page& page_;
};

#endif  // RECORD_PAGE_H_
