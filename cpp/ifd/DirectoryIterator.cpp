#include "DirectoryIterator.hpp"

#ifdef _WIN32
#include <kosongg/win32dirent.h>
#else
#include <dirent.h>
#endif
#include <errno.h>

namespace ifd {

struct DirectoryIterator::Impl {
  DIR           * m_dir;
  struct dirent * m_entry;
  std::string     m_path;
};

static void checkErrNo() {
  if (errno) {
    if (errno == EILSEQ)
      printf("DEBUG: errno = EILSEQ (illegal byte sequence), please enable utf-8/wchar\n", errno);
    else
      printf("DEBUG: errno = %d\n", errno);
  }
}

DirectoryIterator::DirectoryIterator(std::string path)
  : m_path(path)
  , m_impl(nullptr) {
  m_impl = new Impl();

  if ((m_impl->m_dir = opendir(path.c_str())) == NULL) {
    printf("DEBUG: couldn't open %s\n", path.c_str());
    return;
  }

  readNextEntry();
}

DirectoryIterator::~DirectoryIterator() {
  if (m_impl->m_dir) {
    closedir(m_impl->m_dir);
  }
  delete m_impl;
}

bool DirectoryIterator::valid() const
{
  return (m_impl->m_dir != nullptr && m_impl->m_entry != nullptr);
}

void DirectoryIterator::next()
{
  readNextEntry();
}

std::string DirectoryIterator::name() const {
  // Check directory and entry handle
  if (!valid())
  {
    return "";
  }

  // Return filename of current item
  // m_entry->d_name is an array, not a pointer, so it cannot be nullptr. If m_entry is valid, d_name is also valid.
  return std::string(m_impl->m_entry->d_name);
}

void DirectoryIterator::readNextEntry()
{
  // Check directory handle
  if (!m_impl->m_dir) return;

  // Read next entry
  errno = 0;
  m_impl->m_entry = readdir(m_impl->m_dir);
  checkErrNo();

  if (!m_impl->m_entry) return;

  // Omit '.' and '..'
  std::string name(m_impl->m_entry->d_name);
  while (m_impl->m_entry && (name == ".." || name == "."))
  {
    errno = 0;
    m_impl->m_entry = readdir(m_impl->m_dir);
    checkErrNo();

    name = m_impl->m_entry ? std::string(m_impl->m_entry->d_name) : std::string();
  }
}


};
