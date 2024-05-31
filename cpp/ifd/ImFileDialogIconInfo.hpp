#pragma once

#include <functional>
#include <filesystem>

#define DEFAULT_ICON_SIZE 32

namespace ifd {

const char* GetDefaultFolderIcon(bool dark = false);
const char* GetDefaultFileIcon(bool dark = false);

class FileIconInfoBase {
public:
  FileIconInfoBase();
  FileIconInfoBase(const std::filesystem::path& path);

  virtual int GetINode();
  virtual bool HasIcon();
  virtual void *GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture);
  virtual void SetDarkTheme(bool f);
protected:
  bool m_isDarkTheme{};
  int m_iconID{};

  bool CheckDefaultIcon(const std::filesystem::path& path);
};

};