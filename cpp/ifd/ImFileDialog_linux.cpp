#pragma once

#include "ImFileDialog_linux.hpp"

namespace ifd {

struct FileInfoLinux::details {
  int iconID{};
}

FileInfoLinux::FileInfoLinux() {
  m_details = new details();
}

FileInfoLinux::~FileInfoLinux() {
  delete m_details;
}

FileInfoLinux::FileInfoLinux(const std::filesystem::path& path): FileInfoLinux() {
  std::error_code ec;
  m_details->iconID = 1;
  if (std::filesystem::is_directory(path, ec))
    m_details->iconID = 0;
}

int FileInfoLinux::GetINode() {
  return m_details->iconID;
}

bool FileInfoLinux::HasIcon() {
  return true;
}

void * FileInfoLinux::GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture) {
  void * icondata{};

  uint8_t* data = (uint8_t*)ifd::GetDefaultFileIcon(m_isDarkTheme);
  if (m_details->iconID == 0)
    data = (uint8_t*)ifd::GetDefaultFolderIcon(m_isDarkTheme);
  icondata = createTexture(data, DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE, 0);
  return icondata;
}

};
