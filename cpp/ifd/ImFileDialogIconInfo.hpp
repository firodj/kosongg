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

  virtual int GetINode() = 0;
  virtual bool HasIcon() = 0;
  virtual void *GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture) = 0;

  virtual void SetDarkTheme(bool f);
protected:
  bool m_isDarkTheme{};

};

};