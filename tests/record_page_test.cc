#include "record/record_page.h"

#include <cstddef>
#include <iostream>
#include <vector>

int main() {
  Page page;
  RecordPage records(page);
  records.Initialize();

  const std::vector<std::byte> first{std::byte{1}, std::byte{2}};
  const std::vector<std::byte> second{
      std::byte{3}, std::byte{4}, std::byte{5}};

  auto first_slot = records.Insert(first);
  auto second_slot = records.Insert(second);

  if (records.GetRecordData(first_slot) != first ||
      records.GetRecordData(second_slot) != second) {
    std::cerr << "Inserted records did not round-trip\n";
    return 1;
  }

  records.Delete(first_slot);

  if (records.GetLiveRecordCount() != 1 ||
      records.GetRecordData(second_slot) != second) {
    std::cerr << "Deletion corrupted the remaining record\n";
    return 1;
  }

  return 0;
}
