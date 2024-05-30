#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ImFileDialog_win32.hpp"
#include <filesystem>
#include <windows.h>
#include <shellapi.h>
#include <Shlobj.h>
#include <lmcons.h>
#pragma comment(lib, "Shell32.lib")

const wchar_t * WinGetEnv(const wchar_t * name)
{
  const unsigned int buffSize = 65535;
  static wchar_t buffer[buffSize];
  if (GetEnvironmentVariableW(name, buffer, buffSize))
  {
    return buffer;
  }
  else
  {
    return 0;
  }
}

void WinGetSpecialFolder(SpecialFolders & result) {
  wchar_t szPath[MAX_PATH];

  if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, szPath))) {
    result.Desktop = szPath;
  }
  if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, 0, szPath))) {
    result.MyDocuments = szPath;
  }
  if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, szPath))) {
    result.MyPictures = szPath;
  }
  if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYMUSIC, NULL, 0, szPath))) {
    result.MyMusic = szPath;
  }
  if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, 0, szPath))) {
    result.MyVideo = szPath;
  }
}

namespace ifd {

struct FileInfoWin32::details {
  SHFILEINFOW fileInfo{ 0 };
};

FileInfoWin32::FileInfoWin32() {
static bool inited = false;
  if (!inited) {
    inited = true;
    CoInitialize(NULL);
  }
  m_details = new details();
}

FileInfoWin32::FileInfoWin32(const std::filesystem::path& path): FileInfoWin32() {
  std::error_code ec;

  DWORD attrs = 0;
  UINT flags = SHGFI_ICON | SHGFI_LARGEICON;
  if (!std::filesystem::exists(path, ec)) {
    flags |= SHGFI_USEFILEATTRIBUTES;
    attrs = FILE_ATTRIBUTE_DIRECTORY;
  }

  std::wstring pathW = path.wstring();
  for (int i = 0; i < pathW.size(); i++)
    if (pathW[i] == '/')
      pathW[i] = '\\';

  SHGetFileInfoW(pathW.c_str(), attrs, &m_details->fileInfo, sizeof(SHFILEINFOW), flags);
  // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shgetfileinfow
  // If SHGetFileInfo returns an icon handle in the hIcon member of the SHFILEINFO structure pointed to by psfi,
  // you are responsible for freeing it with DestroyIcon when you no longer need it.
}

FileInfoWin32::~FileInfoWin32() {
  delete m_details;
}

bool FileInfoWin32::HasIcon() {
  return m_details->fileInfo.hIcon != nullptr;
}

int FileInfoWin32::GetINode() {
  return (int)m_details->fileInfo.iIcon;
}

void * FileInfoWin32::GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture) {
  ICONINFO iconInfo = { 0 };
  if (m_details->fileInfo.hIcon) {
    std::shared_ptr<void> _(nullptr, [&](...) {
      DestroyIcon(m_details->fileInfo.hIcon);
      m_details->fileInfo.hIcon = 0;
    });
    GetIconInfo(m_details->fileInfo.hIcon, &iconInfo);

    if (iconInfo.hbmColor == nullptr)
      return nullptr;

    DIBSECTION ds;
    GetObject(iconInfo.hbmColor, sizeof(ds), &ds);
    int byteSize = ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8);

    if (byteSize == 0)
      return nullptr;

    uint8_t* data = new uint8_t[byteSize];
    GetBitmapBits(iconInfo.hbmColor, byteSize, data);

    void * icondata = createTexture(data, ds.dsBm.bmWidth, ds.dsBm.bmHeight, 0);

    delete []data;

    return icondata;
  }
  return nullptr;
}

};