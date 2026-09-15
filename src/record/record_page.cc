#include "../record/record_page.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

void RecordPage::Initialize() {
  page_.SetPageType(PageType::RECORD);

  SetSlotCount(0);
  SetLiveRecordCount(0);
  SetFreeSpaceEnd(PAGE_SIZE);
}

uint16_t RecordPage::GetSlotCount() const {
  uint16_t count;

  std::memcpy(&count, page_.GetData() + SLOT_COUNT_OFFSET, sizeof(uint16_t));
  return count;
}

void RecordPage::SetSlotCount(uint16_t slots) {
  std::memcpy(page_.GetData() + SLOT_COUNT_OFFSET, &slots, sizeof(uint16_t));
}

uint16_t RecordPage::GetLiveRecordCount() const {
  uint16_t live;

  std::memcpy(&live, page_.GetData() + LIVE_RECORD_COUNT_OFFSET,
      sizeof(uint16_t));
  return live;
}

void RecordPage::SetLiveRecordCount(uint16_t count) {
  std::memcpy(page_.GetData() + LIVE_RECORD_COUNT_OFFSET, &count,
      sizeof(uint16_t));
}

uint16_t RecordPage::GetFreeSpaceEnd() const {
  uint16_t free_end;

  std::memcpy(&free_end, page_.GetData() + FREE_SPACE_END_OFFSET,
      sizeof(uint16_t));
  return free_end;
}
void RecordPage::SetFreeSpaceEnd(uint16_t offset) {
  std::memcpy(page_.GetData() + FREE_SPACE_END_OFFSET, &offset,
      sizeof(uint16_t));
}

uint16_t RecordPage::GetSlotOffset(uint16_t slot_index) const {
  uint16_t data_offset;
  size_t offset = HEADER_SIZE + slot_index * SLOT_SIZE;

  std::memcpy(&data_offset, page_.GetData() + offset, sizeof(uint16_t));
  return data_offset;
}
void RecordPage::SetSlotOffset(uint16_t slot_index, uint16_t offset) {
  size_t metadata_offset = HEADER_SIZE + slot_index * SLOT_SIZE;

  std::memcpy(page_.GetData() + metadata_offset, &offset, sizeof(uint16_t));
}

uint16_t RecordPage::GetSlotLength(uint16_t slot_index) const {
  uint16_t slot_length;
  size_t slot_length_offset = HEADER_SIZE + slot_index * SLOT_SIZE
                            + sizeof(uint16_t);

  std::memcpy(&slot_length, page_.GetData() + slot_length_offset,
      sizeof(uint16_t));
  return slot_length;
}

void RecordPage::SetSlotLength(uint16_t slot_index, uint16_t length) {
  size_t slot_length_offset = HEADER_SIZE + slot_index * SLOT_SIZE
                            + sizeof(uint16_t);
  
  std::memcpy(page_.GetData() + slot_length_offset, &length, sizeof(uint16_t));
}

RecordSlot RecordPage::GetSlot(uint16_t slot_index) const {
  RecordSlot result;

  result.data_offset = GetSlotOffset(slot_index);
  result.data_size = GetSlotLength(slot_index);

  return result;
}

void RecordPage::SetSlot(uint16_t slot_index, const RecordSlot& slot) {
  SetSlotOffset(slot_index, slot.data_offset);
  SetSlotLength(slot_index, slot.data_size);
}

bool RecordPage::IsSlotFree(uint16_t slot_index) const {
  return GetSlotLength(slot_index) == 0;
}

bool RecordPage::CanFit(size_t record_size) const {
  size_t free_space_end = GetFreeSpaceEnd();
  size_t slot_directory_end = HEADER_SIZE + GetSlotCount() * SLOT_SIZE;

  if (free_space_end < slot_directory_end) {  // defensive against malformation
    return false;
  }

  size_t free_contiguous_space = free_space_end - slot_directory_end;

  // Search for free existing slot. If such is found, no new
  // metadata slot space is needed (can be reused on insertion).
  bool has_free_slot = false;
  
  for(uint16_t i = 0; i < GetSlotCount(); i++) {
    if (IsSlotFree(i)) {
      has_free_slot = true;
      break;
    }
  }

  size_t required_space = record_size;
  if (!has_free_slot) {
    required_space += SLOT_SIZE; // space for new slot required
  }

  return required_space <= free_contiguous_space;
}

std::vector<std::byte> RecordPage::GetRecordData(uint16_t slot_index) const {
  if (slot_index >= GetSlotCount()) {
    throw std::out_of_range("Invalid record slot: " +
        std::to_string(slot_index));
  }

  RecordSlot slot = GetSlot(slot_index);

  if (slot.data_size == 0) {
    throw std::runtime_error("Record slot" + std::to_string(slot_index) +
        " is free.");
  }

  std::vector<std::byte> record_data(slot.data_size);

  std::memcpy(
      record_data.data(),
      page_.GetData() + slot.data_offset,
      slot.data_size);

  return record_data;
}

uint16_t RecordPage::FindFreeSlot() const {
  for (uint16_t i = 0; i < GetSlotCount(); i++) {
    if (IsSlotFree(i)) {
      return i;
    }
  }

  return INVALID_SLOT;
}

void RecordPage::Compact(const RecordSlot& deleted_slot) {
  uint16_t current_free_end = GetFreeSpaceEnd();
  uint16_t deleted_offset = deleted_slot.data_offset;

  size_t size_to_shift = deleted_offset - current_free_end;

  std::memmove(page_.GetData() + current_free_end + deleted_slot.data_size,
               page_.GetData() + current_free_end, size_to_shift);

  for (uint16_t i = 0; i < GetSlotCount(); i++) {
    if (IsSlotFree(i)) {
      continue;
    }

    uint16_t offset = GetSlotOffset(i);

    if (offset < deleted_slot.data_offset) {
      SetSlotOffset(i, offset + deleted_slot.data_size);
    }
  }

  SetFreeSpaceEnd(current_free_end + deleted_slot.data_size);
}

void RecordPage::Delete(uint16_t slot_index) {
  if (slot_index >= GetSlotCount()) {
    throw std::out_of_range("Invalid record slot");
  }

  RecordSlot to_delete = GetSlot(slot_index);

  if (to_delete.data_size == 0) {
    return;
  }

  SetSlot(slot_index, RecordSlot{0, 0});
  SetLiveRecordCount(GetLiveRecordCount() - 1);

  Compact(to_delete);
}

uint16_t RecordPage::Insert(const std::vector<std::byte>& record) {
  // Since we mark free slots as 0s, we must reject empty records.
  if (record.empty()) {
    throw std::invalid_argument("Cannot insert empty record");
  }

  if (!CanFit(record.size())) {
    throw std::runtime_error("Cannot fit a record in this page: "
        + std::to_string(page_.GetPageID()));
  }

  uint16_t slot_index = FindFreeSlot();
  uint16_t slot_count = GetSlotCount();

  if (slot_index == INVALID_SLOT) {
    slot_index = slot_count;
    SetSlotCount(slot_count + 1);
  }

  uint16_t new_free_space_end = GetFreeSpaceEnd() - record.size();
  std::memcpy(page_.GetData() + new_free_space_end, record.data(), record.size());

  SetFreeSpaceEnd(new_free_space_end);
  SetLiveRecordCount(GetLiveRecordCount() + 1);
  
  RecordSlot new_slot = {new_free_space_end, static_cast<uint16_t>(record.size())};
  SetSlot(slot_index, new_slot);

  return slot_index;
}