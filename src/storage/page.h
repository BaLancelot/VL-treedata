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
  friend class BufferPool;

 public:
  Page() = default;

  explicit Page(PageID page_id) : page_id_(page_id) {} 

  PageID GetPageID() const {
    return page_id_;
  }

  uint16_t GetPinCount() const {
	return pin_count_;
  }

  std::byte* GetData() {  // for mutable access (writing to page)
    return data_.data();
  }

  const std::byte* GetData() const {  // for non mutable (writing from page)
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

  bool IsDirty() {
	  return is_dirty_;
  }

  void ResetData() {
    std::memset(data_.data(), 0, PAGE_SIZE);
  }

 private:
  PageID page_id_ = INVALID_PAGE_ID;
  std::array<std::byte, PAGE_SIZE> data_{};

  // flag for BufferPool to decide whether to write the page before eviction
  bool is_dirty_ = false;

  // safety measure to prevent
  // evicitions of pages that are being in use (even by single thread)
  uint16_t pin_count_ = 0;
};

#endif  // PAGE_H_
