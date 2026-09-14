#include "../record/record_page.h"

#include <cstddef>
#include <cstdint>
#include <vector>

void RecordPage::Initialize() {
  page_.SetPageType(PageType::RECORD);

  
}