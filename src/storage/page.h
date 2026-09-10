#include <array>
#include <cstddef>
#include <cstdint>

using PageID = uint32_t;

constexpr std::size_t PAGE_SIZE = 4096;
constexpr PageID INVALID_PAGE_ID = UINT32_MAX;

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

 private:
  PageID page_id_ = INVALID_PAGE_ID;
  std::array<std::byte, PAGE_SIZE> data_{};
};
