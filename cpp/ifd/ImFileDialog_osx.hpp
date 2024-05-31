#pragma once
#include "ImFileDialogIconInfo.hpp"

#include <string>
#include <functional>

namespace ifd {

class FileInfoOsx: public FileIconInfoBase {
private:
  struct details;
  details * m_details{};
public:
  FileInfoOsx();
  FileInfoOsx(std::string apath);
  ~FileInfoOsx();
  bool HasIcon() override;
  int GetINode() override;
  void *GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture) override;
};

void Beep();

};