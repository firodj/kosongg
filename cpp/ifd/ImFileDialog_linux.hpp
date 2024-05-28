#pragma once

#include <functional>
#include <filesystem>

namespace ifd {
  const char* GetDefaultFolderIcon();
  const char* GetDefaultFileIcon();

  class FileInfoLinux {
  private:
    struct details;
    details * m_details{};
  public:
    FileInfoLinux();
    FileInfoLinux(const std::filesystem::path& path);
    ~FileInfoLinux();
    int GetINode();
    bool HasIcon();
    void *GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture);
    void SetDarkTheme(bool f);
  };
};