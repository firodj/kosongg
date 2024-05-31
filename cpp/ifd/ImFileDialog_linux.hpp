#pragma once

#include "ImFileDialogIconInfo.hpp"

namespace ifd {

class FileInfoLinux: public FileIconInfoBase {
private:
  struct details;
  details * m_details{};
public:
  FileInfoLinux();
  FileInfoLinux(const std::filesystem::path& path);
  ~FileInfoLinux();
  int GetINode() override;
  bool HasIcon() override;
  void *GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture) override;
};

};