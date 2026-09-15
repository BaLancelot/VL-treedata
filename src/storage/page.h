#ifndef PAGE_H_
#define PAGE_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

using PageID = uint32_t;

constexpr std::size_t PAGE_SIZE = 4096;
constexpr PageID INVALID_PAGE_ID = UINT32_MAX;

enum class PageType : uint8_t {
  INVALID = 0,
  METADATA,
  BPLUS_INTERNAL,
  BPLUS_LEAF,
  RECORD 
};

// Fixed chunk of memory that will eventually correspond to a fixed chunk of
// database file.
class Page {
 public:
  PageID GetPageID() const {
    return page_id_;
  }

  std::byte* GetData() {
    return data_.data();
  }

  // ALL types of pages share the same metadata field and at same position:
  // Type at offset 0.
  // Generic page should be able to read type to identify which wrapper should
  // be used (i.e when fetching from buffer pool).
  PageType GetPageType() {
    PageType type;
    std::memcpy(&type, GetData(), sizeof(PageType));

    return type;
  }

  void SetPageType(PageType type) {
    std::memcpy(GetData(), &type, sizeof(PageType));
  }

 private:
  PageID page_id_ = INVALID_PAGE_ID;
  std::array<std::byte, PAGE_SIZE> data_{};
};

#endif  // PAGE_H_
