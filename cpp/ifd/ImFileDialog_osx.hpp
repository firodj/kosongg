#ifndef IMFILEDIALOG_OSX_H_
#define IMFILEDIALOG_OSX_H_

#include <string>
#include <functional>

namespace ifd {
  class FileInfoOsx {
  private:
    struct details;
    details * m_details{};
  public:
    FileInfoOsx();
    FileInfoOsx(std::string apath);
    ~FileInfoOsx();
    bool HasIcon();
    int GetINode();
    void *GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture);
  };

  void Beep();
};

#endif