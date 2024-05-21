#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include <codecvt>
#include <string>

// convert UTF-8 string to wstring
inline std::wstring utf8_to_wstring (const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
inline std::string wstring_to_utf8 (const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

struct SpecialFolders {
  std::wstring Desktop;
  std::wstring MyDocuments;
  std::wstring MyMusic;
  std::wstring MyVideo;
  std::wstring MyPictures;
};

void WinGetSpecialFolder(SpecialFolders & result);

const wchar_t * WinGetEnv(const wchar_t * name);

namespace ifd {
  class FileInfoWin32 {
  private:
    struct details;
    details * m_details{};
  public:
    FileInfoWin32();
    FileInfoWin32(const std::filesystem::path& path);
    ~FileInfoWin32();
    int GetINode();
    bool HasIcon();
    void *GetIcon(std::function<void*(uint8_t*, int, int, char)> createTexture);
  };
};
