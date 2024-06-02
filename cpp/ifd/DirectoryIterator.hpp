#pragma once

#include <string>

namespace ifd {

class DirectoryIterator {
public:
  DirectoryIterator(std::string path);
  ~DirectoryIterator();
  bool valid() const;
  void next();
  std::string name() const;
  std::string path() const { return m_path; }

private:
  struct Impl;
  Impl * m_impl;

  std::string  m_path;

  void readNextEntry();
};

};
