#include "disk_manager.h"

#include <algorithm>
#include <stdexcept>

DiskManager::DiskManager(std::filesystem::path file)
  : db_file_path_(file), db_file_(file,
                                  std::ios::in |
                                  std::ios::out |
                                  std::ios::binary) {
  // validate file opened
  if (!db_file_) {
    throw std::runtime_error("Unable to open database file: " + 
        db_file_path_.string());
  }
}

void DiskManager::RestoreMetadataState(const std::vector<PageID>& reusable_page_ids,
                                       PageID next_page_id) {
  reusable_page_ids_ = reusable_page_ids;
  next_page_id_ = next_page_id;
  metadata_initialized_ = true;
}

PageID DiskManager::GetNextPageID() const {
  return next_page_id_;
}

const std::vector<PageID>& DiskManager::GetReusablePageIDs() const{
  return reusable_page_ids_;
}

void DiskManager::WritePage(const Page& page) {
  if (!metadata_initialized_) {
    throw std::runtime_error(
        "Write not allowed until DiskManager is properly initialized.");
  }

  std::streamoff offset = static_cast<std::streamoff>(
      page.GetPageID()) * PAGE_SIZE;
  
  // Set position of WRITE pointer
  db_file_.seekp(offset);

  if (!db_file_) {
    throw std::runtime_error("Failed to navigate database file.");
  }

  db_file_.write(reinterpret_cast<const char*>(page.GetData()), PAGE_SIZE);

  if (!db_file_) {
    throw std::runtime_error("Failed to write to database file.");
  }
}

Page DiskManager::ReadPage(PageID page_id) {
  if (!metadata_initialized_ && page_id > 0) {
    throw std::runtime_error(
        "Reading outside of PageID 0 is not allowed" 
        " until DiskManager is properly initialized.");
  }

  if (page_id >= next_page_id_ ||
      std::find(reusable_page_ids_.begin(), reusable_page_ids_.end(), page_id) !=
          reusable_page_ids_.end()) {
    throw std::runtime_error("Cannot read from non-allocated / deleted page.");
  }

  Page read_res(page_id);
  std::streamoff offset = static_cast<std::streamoff>(page_id) * PAGE_SIZE;

  // Set position of READ pointer
  db_file_.seekg(offset);

  if (!db_file_) {
    throw std::runtime_error("Failed to navigate database file.");
  }

  db_file_.read(reinterpret_cast<char*>(read_res.GetData()), PAGE_SIZE);

  if (db_file_.gcount() != static_cast<std::streamsize>(PAGE_SIZE)) {
    throw std::runtime_error("Failed to read complete database page.");
  }

  if (db_file_.bad()) {
    throw std::runtime_error("I/O error while reading database file.");
  }

  return read_res;
}

void DiskManager::DeletePage(PageID page_id) {
  if (page_id == 0 || page_id >= next_page_id_) {
    throw std::runtime_error("Invalid PageID");
  }

  if (!metadata_initialized_) {
    throw std::runtime_error(
        "Delete is not allowed until DiskManager is properly initialized.");
  }

  if (std::find(reusable_page_ids_.begin(),
                reusable_page_ids_.end(),
                page_id) != reusable_page_ids_.end()) {
    throw std::runtime_error("Cannot delete already deleted page.");
  }
  reusable_page_ids_.push_back(page_id);
}


PageID DiskManager::AllocatePageID() {
  if (!metadata_initialized_) {
    throw std::runtime_error(
        "AllocatePageID is not allowed until DiskManager is properly initialized.");
  }
  PageID result;

  if (reusable_page_ids_.empty()) {
    result = next_page_id_;
    next_page_id_++;

    return result;
  }

  result = reusable_page_ids_.back();
  reusable_page_ids_.pop_back();

  return result;
}
