#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifdef WIN32
  #ifndef STBI_WINDOWS_UTF8
  #define STBI_WINDOWS_UTF8
  #endif
#endif

#define USE_CONTENTTHREAD  1

#include "ImFileDialog.hpp"
#include "ImFileDialogIconInfo.hpp"

#ifdef __APPLE__
#include "ImFileDialog_osx.hpp"
#endif
#ifdef WIN32
#include "ImFileDialog_win32.hpp"
#endif
#ifdef __linux__
#include "ImFileDialog_linux.hpp"
#endif

#include <chrono>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <lmcons.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

#include "kosongg/UtfConv.h"
#ifdef _WIN32
#include <kosongg/win32dirent.h>
#else
#include <dirent.h>
#endif
#include <errno.h>

#define ICON_SIZE ImGui::GetFont()->FontSize + 3
#define GUI_ELEMENT_SIZE ImMax(GImGui->FontSize + 10.f, 24.f)
#define PI 3.141592f

// ref: https://en.cppreference.com/w/cpp/filesystem/file_size
struct HumanReadable
{
  std::uintmax_t size{};

private:
  friend std::ostream& operator<<(std::ostream& os, HumanReadable hr)
  {
    int o{};
    double mantissa = hr.size;
    for (; mantissa >= 1024.; mantissa /= 1024., ++o);
    os << std::ceil(mantissa * 10.) / 10. << "BKMGTPE"[o];
    return o ? os << "B (" << hr.size << ')' : os;
  }
};

namespace ifd {

  bool IsHidden(const std::filesystem::path &entry_path) {
#if defined(_WIN32)
      const bool& is_hidden =
       ((GetFileAttributesW(entry_path.wstring().c_str()) & FILE_ATTRIBUTE_HIDDEN) ||
        (GetFileAttributesW(entry_path.wstring().c_str()) & FILE_ATTRIBUTE_SYSTEM) ||
        ((!entry_path.filename().empty()) ? (entry_path.filename().wstring()[0] == L'.') : true));
#else
      const std::string& filename = entry_path.filename().string();
      const bool& is_hidden = ((!filename.empty()) ? (filename[0] == '.') : true);
#endif
     return is_hidden;
  }


  /* UI CONTROLS */
  bool FolderNode(const char* label, ImTextureID icon, bool& clicked, bool* p_open = NULL)
  {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    clicked = false;

    ImU32 id = window->GetID(label);
    int opened = window->StateStorage.GetInt(id, p_open ? *p_open : 0);
    ImVec2 pos = window->DC.CursorPos;
    const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= pos.x && g.IO.MousePos.x < pos.x + g.FontSize);
    if (ImGui::InvisibleButton(label, ImVec2(-FLT_MIN, g.FontSize + g.Style.FramePadding.y * 2)))
    {
      if (is_mouse_x_over_arrow) {
        int* p_opened = window->StateStorage.GetIntRef(id, p_open ? *p_open : 0);
        opened = *p_opened = !*p_opened;
      } else {
        clicked = true;
      }
    }
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    bool doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
    if (doubleClick && hovered) {
      int* p_opened = window->StateStorage.GetIntRef(id, p_open ? *p_open : 0);
      opened = *p_opened = !*p_opened;
      clicked = false;
    }
    if (hovered || active)
      window->DrawList->AddRectFilled(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

    // Icon, text
    float icon_posX = pos.x + g.FontSize + g.Style.FramePadding.y;
    float text_posX = icon_posX + g.Style.FramePadding.y + ICON_SIZE;
    ImGui::RenderArrow(window->DrawList, ImVec2(pos.x, pos.y+g.Style.FramePadding.y), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[((hovered && is_mouse_x_over_arrow) || opened) ? ImGuiCol_Text : ImGuiCol_TextDisabled]), opened ? ImGuiDir_Down : ImGuiDir_Right);
    window->DrawList->AddImage(icon, ImVec2(icon_posX, pos.y), ImVec2(icon_posX + ICON_SIZE, pos.y + ICON_SIZE));
    ImGui::RenderText(ImVec2(text_posX, pos.y + g.Style.FramePadding.y), label);
    if (opened)
      ImGui::TreePush(label);
    if (p_open)
      *p_open = opened;
    return opened != 0;
  }
  bool FileNode(const char* label, ImTextureID icon) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    //ImU32 id = window->GetID(label);
    ImVec2 pos = window->DC.CursorPos;
    bool ret = ImGui::InvisibleButton(label, ImVec2(-FLT_MIN, g.FontSize + g.Style.FramePadding.y * 2));

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    if (hovered || active)
      window->DrawList->AddRectFilled(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

    // Icon, text
    window->DrawList->AddImage(icon, ImVec2(pos.x, pos.y), ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE));
    ImGui::RenderText(ImVec2(pos.x + g.Style.FramePadding.y + ICON_SIZE, pos.y + g.Style.FramePadding.y), label);

    return ret;
  }
  bool PathBox(const char* label, std::filesystem::path& path, char* pathBuffer, ImVec2 size_arg) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
      return false;

    bool ret = false;
    const ImGuiID id = window->GetID(label);
    int* state = window->StateStorage.GetIntRef(id, 0);

    ImGui::SameLine();

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 uiPos = ImGui::GetCursorPos();
    ImVec2 size = ImGui::CalcItemSize(size_arg, 200, GUI_ELEMENT_SIZE);
    const ImRect bb(pos, pos + size);

    // buttons
    if (!(*state & 0b001)) {
      ImGui::PushClipRect(bb.Min, bb.Max, false);

      // background
      bool hovered = g.IO.MousePos.x >= bb.Min.x && g.IO.MousePos.x <= bb.Max.x &&
        g.IO.MousePos.y >= bb.Min.y && g.IO.MousePos.y <= bb.Max.y;
      bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
      bool anyOtherHC = false; // are any other items hovered or clicked?
      window->DrawList->AddRectFilled(pos, pos + size, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[(*state & 0b10) ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg]));

      // fetch the buttons (so that we can throw some away if needed)
      std::vector<std::string> btnList;
      float totalWidth = 0.0f;
      for (auto comp : path) {
        std::string section = comp.u8string();
        if (section.size() == 1 && (section[0] == '\\' || section[0] == '/'))
          continue;

        totalWidth += ImGui::CalcTextSize(section.c_str()).x + style.FramePadding.x * 2.0f + GUI_ELEMENT_SIZE;
        btnList.push_back(section);
      }
      totalWidth -= GUI_ELEMENT_SIZE;

      // UI buttons
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
      ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
      bool isFirstElement = true;
      for (size_t i = 0; i < btnList.size(); i++) {
        if (totalWidth > size.x - 30 && i != btnList.size() - 1) { // trim some buttons if there's not enough space
          float elSize = ImGui::CalcTextSize(btnList[i].c_str()).x + style.FramePadding.x * 2.0f + GUI_ELEMENT_SIZE;
          totalWidth -= elSize;
          continue;
        }

        ImGui::PushID(static_cast<int>(i));
        if (!isFirstElement) {
          ImGui::ArrowButtonEx("##dir_dropdown", ImGuiDir_Right, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE));
          anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
          ImGui::SameLine();
        }
        if (ImGui::Button(btnList[i].c_str(), ImVec2(0, GUI_ELEMENT_SIZE))) {
#ifdef _WIN32
          std::string newPath = "";
#else
          std::string newPath = "/";
#endif
          for (size_t j = 0; j <= i; j++) {
            newPath += btnList[j];
#ifdef _WIN32
            if (j != i)
              newPath += "\\";
#else
            if (j != i)
              newPath += "/";
#endif
          }
          path = std::filesystem::u8path(newPath);
          ret = true;
        }
        anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
        ImGui::SameLine();
        ImGui::PopID();

        isFirstElement = false;
      }
      ImGui::PopStyleVar(2);


      // click state
      if (!anyOtherHC && clicked) {
        strcpy(pathBuffer, path.u8string().c_str());
        *state |= 0b001;
        *state &= 0b011; // remove SetKeyboardFocus flag
      }
      else
        *state &= 0b110;

      // hover state
      if (!anyOtherHC && hovered && !clicked)
        *state |= 0b010;
      else
        *state &= 0b101;

      ImGui::PopClipRect();

      // allocate space
      ImGui::SetCursorPos(uiPos);
      ImGui::ItemSize(size);
    }
    // input box
    else {
      bool skipActiveCheck = false;
      if (!(*state & 0b100)) {
        skipActiveCheck = true;
        ImGui::SetKeyboardFocusHere();
        if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
          *state |= 0b100;
      }
      if (ImGui::InputTextEx("##pathbox_input", "", pathBuffer, 1024, size_arg, ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string tempStr(pathBuffer);
        if (std::filesystem::exists(tempStr))
          path = std::filesystem::u8path(tempStr);
        ret = true;
      }
      if (!skipActiveCheck && !ImGui::IsItemActive())
        *state &= 0b010;
    }

    return ret;
  }
  bool FavoriteButton(const char* label, bool isFavorite)
  {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    ImVec2 pos = window->DC.CursorPos;
    bool ret = ImGui::InvisibleButton(label, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE));

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    float size = g.LastItemData.Rect.Max.x - g.LastItemData.Rect.Min.x;

    int numPoints = 5;
    float innerRadius = size / 4;
    float outerRadius = size / 2;
    float angle = PI / numPoints;
    ImVec2 center = ImVec2(pos.x + size / 2, pos.y + size / 2);

    // fill
    if (isFavorite || hovered || active) {
      ImU32 fillColor = 0xff00ffff;// ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
      if (hovered || active)
        fillColor = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]);

      // since there is no PathFillConcave, fill first the inner part, then the triangles
      // inner
      window->DrawList->PathClear();
      for (int i = 1; i < numPoints * 2; i += 2)
        window->DrawList->PathLineTo(ImVec2(center.x + innerRadius * sin(i * angle), center.y - innerRadius * cos(i * angle)));
      window->DrawList->PathFillConvex(fillColor);

      // triangles
      for (int i = 0; i < numPoints; i++) {
        window->DrawList->PathClear();

        int pIndex = i * 2;
        window->DrawList->PathLineTo(ImVec2(center.x + outerRadius * sin(pIndex * angle), center.y - outerRadius * cos(pIndex * angle)));
        window->DrawList->PathLineTo(ImVec2(center.x + innerRadius * sin((pIndex + 1) * angle), center.y - innerRadius * cos((pIndex + 1) * angle)));
        window->DrawList->PathLineTo(ImVec2(center.x + innerRadius * sin((pIndex - 1) * angle), center.y - innerRadius * cos((pIndex - 1) * angle)));

        window->DrawList->PathFillConvex(fillColor);
      }
    }

    // outline
    window->DrawList->PathClear();
    for (int i = 0; i < numPoints * 2; i++) {
      float radius = i & 1 ? innerRadius : outerRadius;
      window->DrawList->PathLineTo(ImVec2(center.x + radius * sin(i * angle), center.y - radius * cos(i * angle)));
    }
    window->DrawList->PathStroke(ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), true, 2.0f);

    return ret;
  }
  bool FileIcon(const char* label, bool isSelected, ImTextureID icon, ImVec2 size, bool hasPreview, int previewWidth, int previewHeight)
  {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    float windowSpace = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    ImVec2 pos = window->DC.CursorPos;
    bool ret = false;

    if (ImGui::InvisibleButton(label, size))
      ret = true;

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    bool doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
    if (doubleClick && hovered)
      ret = true;


    float iconSize = size.y - g.FontSize * 2;
    float iconPosX = pos.x + (size.x - iconSize) / 2.0f;
    ImVec2 textSize = ImGui::CalcTextSize(label, 0, true, size.x);


    if (hovered || active || isSelected)
      window->DrawList->AddRectFilled(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : (isSelected ? ImGuiCol_Header : ImGuiCol_HeaderHovered)]));

    if (hasPreview) {
      ImVec2 availSize = ImVec2(size.x, iconSize);

      float scale = std::min<float>(availSize.x / previewWidth, availSize.y / previewHeight);
      availSize.x = previewWidth * scale;
      availSize.y = previewHeight * scale;

      float previewPosX = pos.x + (size.x - availSize.x) / 2.0f;
      float previewPosY = pos.y + (iconSize - availSize.y) / 2.0f;

      window->DrawList->AddImage(icon, ImVec2(previewPosX, previewPosY), ImVec2(previewPosX + availSize.x, previewPosY + availSize.y));
    } else
      window->DrawList->AddImage(icon, ImVec2(iconPosX, pos.y), ImVec2(iconPosX + iconSize, pos.y + iconSize));

    window->DrawList->AddText(g.Font, g.FontSize, ImVec2(pos.x + (size.x-textSize.x) / 2.0f, pos.y + iconSize), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), label, 0, size.x);


    float lastButtomPos = ImGui::GetItemRectMax().x;
    float thisButtonPos = lastButtomPos + style.ItemSpacing.x + size.x; // Expected position if next button was on same line
    if (thisButtonPos < windowSpace)
      ImGui::SameLine();

    return ret;
  }

  FileDialog::FileData::FileData(const std::filesystem::path& path) {
    std::error_code ec;
    Path = path;
    IsDirectory = std::filesystem::is_directory(path, ec);
    Size = IsDirectory ? (size_t)-1 : std::filesystem::file_size(path, ec);

    struct stat attr;
    stat(path.u8string().c_str(), &attr);
    DateModified = attr.st_ctime;

    HasIconPreview = false;
    IconPreview = nullptr;
    IconPreviewData = nullptr;
    IconPreviewHeight = 0;
    IconPreviewWidth = 0;
  }

  FileDialog::FileDialog() {
    m_isOpen = false;
    m_type = 0;
    m_calledOpenPopup = false;
    m_sortColumn = 0;
    m_sortDirection = ImGuiSortDirection_Ascending;
    m_filterSelection = 0;
    m_inputTextbox[0] = 0;
    m_pathBuffer[0] = 0;
    m_searchBuffer[0] = 0;
    m_newEntryBuffer[0] = 0;
    m_selectedFileItem = -1;
    m_zoom = 1.0f;

    m_previewLoader = nullptr;
    m_previewLoaderRunning = false;
    m_contentLoader = nullptr;
    m_contentLoaderRunning = false;

    m_setDirectory(std::filesystem::current_path(), false);

    // favorites are available on every OS
    FileTreeNode* quickAccess = new FileTreeNode("Quick Access");
    quickAccess->Read = true;
    quickAccess->Special = true;
    m_treeCache.push_back(quickAccess);

#ifdef _WIN32
    wchar_t username[UNLEN + 1] = { 0 };
    DWORD username_len = UNLEN + 1;
    GetUserNameW(username, &username_len);

    std::wstring userPath = L"C:\\Users\\" + std::wstring(username) + L"\\";

    // Quick Access / Bookmarks
    quickAccess->Children.push_back(new FileTreeNode(userPath + L"Desktop"));
    quickAccess->Children.push_back(new FileTreeNode(userPath + L"Documents"));
    quickAccess->Children.push_back(new FileTreeNode(userPath + L"Downloads"));
    quickAccess->Children.push_back(new FileTreeNode(userPath + L"Pictures"));

    // OneDrive
    FileTreeNode* oneDrive = new FileTreeNode(userPath + L"OneDrive");
    m_treeCache.push_back(oneDrive);

    // This PC
    FileTreeNode* thisPC = new FileTreeNode("This PC");
    thisPC->Read = true;
    thisPC->Special = true;

#if 0
    auto userProfile = userPath;
    if (const wchar_t * envUserProfile = WinGetEnv(L"USERPROFILE")) {
      userProfile = envUserProfile; userProfile += L"\\";
    }
#endif

    SpecialFolders specialFolders;
    WinGetSpecialFolder(specialFolders);

    if (!specialFolders.Desktop.empty())
      thisPC->Children.push_back(new FileTreeNode(specialFolders.Desktop));
    if (!specialFolders.MyDocuments.empty())
      thisPC->Children.push_back(new FileTreeNode(specialFolders.MyDocuments));
    if (!specialFolders.MyPictures.empty())
      thisPC->Children.push_back(new FileTreeNode(specialFolders.MyPictures));
    if (!specialFolders.MyMusic.empty())
      thisPC->Children.push_back(new FileTreeNode(specialFolders.MyMusic));
    if (!specialFolders.MyVideo.empty())
      thisPC->Children.push_back(new FileTreeNode(specialFolders.MyVideo));

    DWORD d = GetLogicalDrives();
    for (int i = 0; i < 26; i++)
      if (d & (1 << i)) {
        std::string drvPath = std::string(1, 'A' + i) + ":\\";
        FileTreeNode *logicalDrive = new FileTreeNode(drvPath);
#ifdef WIN32
        {
          WCHAR szVolumeName[MAX_PATH];
          auto drvPathW = utf8_to_wstring(drvPath);
          BOOL bSucceeded = GetVolumeInformationW(drvPathW.c_str(),
                                                  szVolumeName,
                                                  MAX_PATH,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  0);
          if (bSucceeded) {
            logicalDrive->DisplayName = wstring_to_utf8(szVolumeName);
            logicalDrive->DisplayName += " (" + drvPath + ")";
          }
        }
#endif
        thisPC->Children.push_back(logicalDrive);
      }
    m_treeCache.push_back(thisPC);
#elif defined(__APPLE__)
    std::string homePath = std::getenv("HOME");
    std::error_code ec;

    // Quick Access
    if (std::filesystem::exists(homePath, ec))
      quickAccess->Children.push_back(new FileTreeNode(homePath));
    if (std::filesystem::exists(homePath + "/Desktop", ec))
      quickAccess->Children.push_back(new FileTreeNode(homePath + "/Desktop"));
    if (std::filesystem::exists(homePath + "/Documents", ec))
      quickAccess->Children.push_back(new FileTreeNode(homePath + "/Documents"));
    if (std::filesystem::exists(homePath + "/Downloads", ec))
      quickAccess->Children.push_back(new FileTreeNode(homePath + "/Downloads"));
    if (std::filesystem::exists(homePath + "/Music", ec))
      quickAccess->Children.push_back(new FileTreeNode(homePath + "/Music"));
    if (std::filesystem::exists(homePath + "/Movies", ec))
      quickAccess->Children.push_back(new FileTreeNode(homePath + "/Movies"));
    if (std::filesystem::exists(homePath + "/Pictures", ec))
      quickAccess->Children.push_back(new FileTreeNode(homePath + "/Pictures"));

    // This PC
    FileTreeNode* thisPC = new FileTreeNode("This PC");
    thisPC->Read = true;
    thisPC->Special = true;

    const char *rootPC = "/";
    if (std::filesystem::exists("/Volumes", ec)) {
      rootPC = "/Volumes";
    }

    for (const auto& entry : std::filesystem::directory_iterator(rootPC, ec)) {
      if (std::filesystem::is_directory(entry, ec)) {
        if (IsHidden(entry.path())) continue;
        thisPC->Children.push_back(new FileTreeNode(entry.path().u8string()));
      }
    }
    m_treeCache.push_back(thisPC);
#else
    std::error_code ec;

    // Quick Access
    struct passwd *pw;
    uid_t uid;
    uid = geteuid();
    pw = getpwuid(uid);
    if (pw) {
      std::string homePath = "/home/" + std::string(pw->pw_name);

      if (std::filesystem::exists(homePath, ec))
        quickAccess->Children.push_back(new FileTreeNode(homePath));
      if (std::filesystem::exists(homePath + "/Desktop", ec))
        quickAccess->Children.push_back(new FileTreeNode(homePath + "/Desktop"));
      if (std::filesystem::exists(homePath + "/Documents", ec))
        quickAccess->Children.push_back(new FileTreeNode(homePath + "/Documents"));
      if (std::filesystem::exists(homePath + "/Downloads", ec))
        quickAccess->Children.push_back(new FileTreeNode(homePath + "/Downloads"));
      if (std::filesystem::exists(homePath + "/Pictures", ec))
        quickAccess->Children.push_back(new FileTreeNode(homePath + "/Pictures"));
    }

    // This PC
    FileTreeNode* thisPC = new FileTreeNode("This PC");
    thisPC->Read = true;
    thisPC->Special = true;

    for (const auto& entry : std::filesystem::directory_iterator("/", ec)) {
      if (std::filesystem::is_directory(entry, ec)) {
        if (IsHidden(entry.path())) continue;
        thisPC->Children.push_back(new FileTreeNode(entry.path().u8string()));
      }
    }
    m_treeCache.push_back(thisPC);
#endif

  }
  FileDialog::~FileDialog() {
    m_clearIconPreview();
    m_clearIcons();
    m_stopContentLoader();
    m_stopPreviewLoader();

    for (auto fn : m_treeCache)
      m_clearTree(fn);
    m_treeCache.clear();
  }
  bool FileDialog::Save(const std::string& key, const std::string& title, const std::string& filter, const std::string& startingDir)
  {
    if (!m_currentKey.empty())
      return false;

    m_currentKey = key;
    m_currentTitle = title + "###" + key;
    m_isOpen = true;
    m_calledOpenPopup = false;
    m_result.clear();
    m_inputTextbox[0] = 0;
    m_selections.clear();
    m_selectedFileItem = -1;
    m_isMultiselect = false;
    m_type = IFD_DIALOG_SAVE;

    m_parseFilter(filter);
    if (!startingDir.empty())
      m_setDirectory(std::filesystem::u8path(startingDir), false);
    else
      m_setDirectory(m_currentDirectory, false); // refresh contents

    return true;
  }
  bool FileDialog::Open(const std::string& key, const std::string& title, const std::string& filter, bool isMultiselect, const std::string& startingDir)
  {
    if (!m_currentKey.empty())
      return false;

    m_currentKey = key;
    m_currentTitle = title + "###" + key;
    m_isOpen = true;
    m_calledOpenPopup = false;
    m_result.clear();
    m_inputTextbox[0] = 0;
    m_selections.clear();
    m_selectedFileItem = -1;
    m_isMultiselect = isMultiselect;
    m_type = filter.empty() ? IFD_DIALOG_DIRECTORY : IFD_DIALOG_FILE;

    m_parseFilter(filter);
    if (!startingDir.empty())
      m_setDirectory(std::filesystem::u8path(startingDir), false);
    else
      m_setDirectory(m_currentDirectory, false); // refresh contents

    return true;
  }
  bool FileDialog::IsDone(const std::string& key)
  {
    bool isMe = m_currentKey == key;

    if (isMe && m_isOpen) {
      if (!m_calledOpenPopup) {
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        ImGui::OpenPopup(m_currentTitle.c_str());
        m_calledOpenPopup = true;
      }

      if (ImGui::BeginPopupModal(m_currentTitle.c_str(), &m_isOpen, ImGuiWindowFlags_NoScrollbar)) {
        m_renderFileDialog();
        ImGui::EndPopup();
      }
      else m_isOpen = false;
    }

    return isMe && !m_isOpen;
  }
  void FileDialog::Close()
  {
    m_currentKey.clear();
    m_backHistory = std::stack<std::filesystem::path>();
    m_forwardHistory = std::stack<std::filesystem::path>();

    // clear the tree
    for (auto fn : m_treeCache) {
      for (auto item : fn->Children) {
        for (auto ch : item->Children)
          m_clearTree(ch);
        item->Children.clear();
        item->Read = false;
      }
    }

    // free icon textures
    m_clearIconPreview();
    m_clearIcons();
    m_stopContentLoader();
  }

  void FileDialog::RemoveFavorite(const std::string& path)
  {
    auto itr = std::find(m_favorites.begin(), m_favorites.end(), m_currentDirectory.u8string());

    if (itr != m_favorites.end())
      m_favorites.erase(itr);

    // remove from sidebar
    for (auto& p : m_treeCache)
      if (p->Path == "Quick Access") {
        for (size_t i = 0; i < p->Children.size(); i++)
          if (p->Children[i]->Path == path) {
            p->Children.erase(p->Children.begin() + i);
            break;
          }
        break;
      }
  }
  void FileDialog::AddFavorite(const std::string& path)
  {
    if (std::count(m_favorites.begin(), m_favorites.end(), path) > 0)
      return;

    if (!std::filesystem::exists(std::filesystem::u8path(path)))
      return;

    m_favorites.push_back(path);

    // add to sidebar
    for (auto& p : m_treeCache)
      if (p->Path == "Quick Access") {
        p->Children.push_back(new FileTreeNode(path));
        break;
      }
  }

  void FileDialog::m_select(const std::filesystem::path& path, bool isCtrlDown)
  {
    bool multiselect = isCtrlDown && m_isMultiselect;

    if (!multiselect) {
      m_selections.clear();
      m_selections.push_back(path);
    } else {
      auto it = std::find(m_selections.begin(), m_selections.end(), path);
      if (it != m_selections.end())
        m_selections.erase(it);
      else
        m_selections.push_back(path);
    }

    if (m_selections.size() == 1) {
      std::string filename = m_selections[0].filename().u8string();
      if (filename.size() == 0)
        filename = m_selections[0].u8string(); // drive

      strcpy(m_inputTextbox, filename.c_str());
    }
    else {
      std::string textboxVal = "";
      for (const auto& sel : m_selections) {
        std::string filename = sel.filename().u8string();
        if (filename.size() == 0)
          filename = sel.u8string();

        textboxVal += "\"" + filename + "\", ";
      }
      strcpy(m_inputTextbox, textboxVal.substr(0, textboxVal.size() - 2).c_str());
    }
  }

  bool FileDialog::m_finalize(const std::string& filename)
  {
    bool hasResult = (!filename.empty() && m_type != IFD_DIALOG_DIRECTORY) || m_type == IFD_DIALOG_DIRECTORY;

    if (hasResult) {
      if (!m_isMultiselect || m_selections.size() <= 1) {
        std::filesystem::path path = std::filesystem::u8path(filename);
        if (path.is_absolute()) m_result.push_back(path);
        else m_result.push_back(m_currentDirectory / path);
        if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE) {
          if (!std::filesystem::exists(m_result.back())) {
            m_result.clear();
            return false;
          }
        }
      }
      else {
        for (const auto& sel : m_selections) {
          if (sel.is_absolute()) m_result.push_back(sel);
          else m_result.push_back(m_currentDirectory / sel);
          if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE) {
            if (!std::filesystem::exists(m_result.back())) {
              m_result.clear();
              return false;
            }
          }
        }
      }

      if (m_type == IFD_DIALOG_SAVE) {
        // add the extension
        if (m_filterSelection < m_filterExtensions.size() && m_filterExtensions[m_filterSelection].size() > 0) {
          if (!m_result.back().has_extension()) {
            std::string extAdd = m_filterExtensions[m_filterSelection][0];
            m_result.back().replace_extension(extAdd);
          }
        }
      }
    }

    m_isOpen = false;

    return true;
  }
  void FileDialog::m_parseFilter(const std::string& filter)
  {
    m_filter = "";
    m_filterExtensions.clear();
    m_filterSelection = 0;

    if (filter.empty())
      return;

    std::vector<std::string> exts;

    size_t lastSplit = 0, lastExt = 0;
    bool inExtList = false;
    for (size_t i = 0; i < filter.size(); i++) {
      if (filter[i] == ',') {
        if (!inExtList)
          lastSplit = i + 1;
        else {
          exts.push_back(filter.substr(lastExt, i - lastExt));
          lastExt = i + 1;
        }
      }
      else if (filter[i] == '{') {
        std::string filterName = filter.substr(lastSplit, i - lastSplit);
        if (filterName == ".*") {
          m_filter += std::string(std::string("All Files (*.*)\0").c_str(), 16);
          m_filterExtensions.push_back(std::vector<std::string>());
        }
        else
          m_filter += std::string((filterName + "\0").c_str(), filterName.size() + 1);
        inExtList = true;
        lastExt = i + 1;
      }
      else if (filter[i] == '}') {
        exts.push_back(filter.substr(lastExt, i - lastExt));
        m_filterExtensions.push_back(exts);
        exts.clear();

        inExtList = false;
      }
    }
    if (lastSplit != 0) {
      std::string filterName = filter.substr(lastSplit);
      if (filterName == ".*") {
        m_filter += std::string(std::string("All Files (*.*)\0").c_str(), 16);
        m_filterExtensions.push_back(std::vector<std::string>());
      }
      else
        m_filter += std::string((filterName + "\0").c_str(), filterName.size() + 1);
    }
  }

  void* FileDialog::m_getIcon(const std::filesystem::path& path)
  {
    std::string pathU8 = path.u8string();
    void *tex = nullptr;

    if (m_icons.count(pathU8) > 0)
      tex = m_icons[pathU8];
    else {

#if defined(_WIN32)
      ifd::FileInfoWin32 iconForFile(path);
#elif defined(__APPLE__)
      ifd::FileInfoOsx iconForFile(pathU8);
#else // LINUX
      ifd::FileIconInfoBase iconForFile(pathU8);
#endif

      // light theme - load default icons
      ImVec4 wndBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
      iconForFile.SetDarkTheme(!((wndBg.x + wndBg.y + wndBg.z) / 3.0f > 0.5f));

      if (iconForFile.HasIcon()) {
        // check if icon is already loaded
        auto itr = std::find(m_iconIndices.begin(), m_iconIndices.end(), iconForFile.GetINode());
        if (itr != m_iconIndices.end()) {
          const std::string& existingIconFilepath = m_iconFilepaths[itr - m_iconIndices.begin()];
          tex = m_icons[existingIconFilepath];
        } else {
          m_iconIndices.push_back(iconForFile.GetINode());
          m_iconFilepaths.push_back(pathU8);
          tex = iconForFile.GetIcon(this->CreateTexture);
        }

        m_icons[pathU8] = tex;
      } else {
        printf("doesnt HasIcon %s\n", pathU8.c_str());

        m_icons[pathU8] = nullptr; // Hmmm....
      }
    }

    return tex;
  }

  void FileDialog::m_clearIcons() {
    m_requestClearIcons = true;
  }
  void FileDialog::m_doClearIcons()
  {
    if (m_requestClearIcons) {
      m_requestClearIcons = false;
    } else {
      return;
    }
    std::vector<unsigned int> deletedIcons;

    // delete textures
    for (auto& icon : m_icons) {
      unsigned int ptr = (unsigned int)((uintptr_t)icon.second);
      if (std::count(deletedIcons.begin(), deletedIcons.end(), ptr)) // skip duplicates
        continue;

      deletedIcons.push_back(ptr);
      this->DeleteTexture(icon.second);
    }
    m_iconFilepaths.clear();
    m_iconIndices.clear();
    m_icons.clear();
  }
  void FileDialog::m_refreshIconPreview() {
    m_requestRefreshIconPreview = true;
  }
  void FileDialog::m_doRefreshIconPreview()
  {
    if (m_requestRefreshIconPreview) {
      m_requestRefreshIconPreview = false;
    } else return;

    if (m_zoom >= 5.0f) {
      if (m_previewLoader == nullptr) {
        m_previewLoaderRunning = true;
        m_previewLoader = new std::thread(&FileDialog::m_loadPreviewRun, this);
      }
    } else {
      m_clearIconPreview();
      m_doClearIconPreview();
    }
  }
  void FileDialog::m_clearIconPreview()
  {
    m_requestClearIconPreview = true;
  }
  void FileDialog::m_doClearIconPreview()
  {
    if (! m_requestClearIconPreview) return;

    m_stopPreviewLoader();

    if (! m_mtxContent.try_lock()) return;
    std::lock_guard<std::mutex> lock(m_mtxContent, std::adopt_lock);

    m_requestClearIconPreview = false;

    for (auto& data : m_content) {
      if (!data.HasIconPreview)
        continue;

      data.HasIconPreview = false;
      this->DeleteTexture(data.IconPreview);

      if (data.IconPreviewData != nullptr) {
        stbi_image_free(data.IconPreviewData);
        data.IconPreviewData = nullptr;
      }
    }
  }
  void FileDialog::m_stopPreviewLoader()
  {
    if (m_previewLoader != nullptr) {
      m_previewLoaderRunning = false;

      if (m_previewLoader && m_previewLoader->joinable())
        m_previewLoader->join();

      delete m_previewLoader;
      m_previewLoader = nullptr;
    }
  }
  std::string toLower(std::string src) {
    return std::string(
      reinterpret_cast<const char*>(
          Utf8StrMakeLwrUtf8Str(
            reinterpret_cast<const unsigned char*>(src.c_str()))));
  }
  void FileDialog::m_loadPreviewRun()
  {
    std::lock_guard<std::mutex> lock(m_mtxContent);
    for (size_t i = 0; m_previewLoaderRunning && i < m_content.size(); i++) {
      auto& data = m_content[i];

      if (data.HasIconPreview)
        continue;

      if (data.Path.has_extension()) {
        std::string ext = toLower(data.Path.extension().u8string());
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
          int width, height, nrChannels;
          unsigned char* image = stbi_load(data.Path.u8string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

          if (image == nullptr || width == 0 || height == 0)
            continue;

          data.HasIconPreview = true;
          data.IconPreviewData = image;
          data.IconPreviewWidth = width;
          data.IconPreviewHeight = height;
        }
      }
    }

    m_previewLoaderRunning = false;
  }
  void FileDialog::m_stopContentLoader() {
    if (m_contentLoader != nullptr) {
      m_contentLoaderRunning = false;

      if (m_contentLoader && m_contentLoader->joinable())
        m_contentLoader->join();

      delete m_contentLoader;
      m_contentLoader = nullptr;

      printf("contentLoader thread delete\n");
    }
  }
  void FileDialog::m_clearTree(FileTreeNode* node)
  {
    if (node == nullptr)
      return;

    for (auto n : node->Children)
      m_clearTree(n);

    delete node;
    node = nullptr;
  }
  void FileDialog::m_setDirectory(const std::filesystem::path& p, bool addHistory)
  {
    m_setDirectoryParam.addHistory = addHistory;
    m_setDirectoryParam.p = p;
    m_setDirectoryParam.requested = true;
  }
  void FileDialog::m_doSetDirectory()
  {
    if (!m_setDirectoryParam.requested) return;
    if (! m_mtxContent.try_lock()) return;
    std::lock_guard<std::mutex> lock(m_mtxContent, std::adopt_lock);
    m_setDirectoryParam.requested = false;

    m_stopContentLoader();

    std::filesystem::path p = m_setDirectoryParam.p;
    bool addHistory = m_setDirectoryParam.addHistory;

    bool isSameDir = m_currentDirectory == p;

    if (addHistory && !isSameDir)
      m_backHistory.push(m_currentDirectory);

    m_currentDirectory = p;
#ifdef _WIN32
    // drives don't work well without the backslash symbol
    if (p.u8string().size() == 2 && p.u8string()[1] == ':')
      m_currentDirectory = std::filesystem::u8path(p.u8string() + "\\");
#endif

    bool shouldListDir = false;
    m_clearIconPreview();
    m_doClearIconPreview();

    m_content.clear(); // p == "" after this line, due to reference
    m_selectedFileItem = -1;

    if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE)
      m_inputTextbox[0] = 0;
    m_selections.clear();

    if (!isSameDir) {
      m_searchBuffer[0] = 0;
      m_clearIcons();
      m_doClearIcons();
    }

    if (p.u8string() == "Quick Access") {
      for (auto& node : m_treeCache) {
        if (node->Path == p)
          for (auto& c : node->Children)
            m_content.push_back(FileData(c->Path));
      }
    }
    else if (p.u8string() == "This PC") {
      for (auto& node : m_treeCache) {
        if (node->Path == p)
          for (auto& c : node->Children)
            m_content.push_back(FileData(c->Path));
      }
    } else
      shouldListDir = true;

    if (shouldListDir) {
      std::error_code ec;
      if (std::filesystem::exists(m_currentDirectory, ec)) {
        m_contentLoaderRunning = true;
#if USE_CONTENTTHREAD == 1
        m_contentLoader = new std::thread([&](...) {
          std::lock_guard<std::mutex> lock(m_mtxContent);
#endif
          std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
          std::shared_ptr<void> _(nullptr, [&](...) {
            m_contentLoaderRunning = false;
          });

#if 1
          DIR *dirp;
          struct dirent dp{};
          struct dirent *result;

          if ((dirp = opendir(m_currentDirectory.u8string().c_str())) == NULL) {
            printf("couldn't open %s\n", m_currentDirectory.u8string().c_str());
            return;
          }

          int rc;
          errno = 0;
          for (errno = 0; (rc = readdir_r(dirp, &dp, &result)) == 0 && result != NULL && errno == 0; errno = 0) {
            if (!m_contentLoaderRunning) {
              break;
            }

            printf("myfile.entryName: -->%s<--  result->d_name: -->%s<--\n",
                  dp.d_name,
                  result->d_name);

            std::string entryPath = m_currentDirectory / dp.d_name;

            if (IsHidden(entryPath)) continue;
            FileData info(entryPath);

            // skip files when IFD_DIALOG_DIRECTORY
            if (!info.IsDirectory && m_type == IFD_DIALOG_DIRECTORY)
              continue;

            // check if filename matches search query
            if (m_searchBuffer[0]) {
              std::string filename = info.Path.u8string();

              std::string filenameSearch = filename;
              std::string query(m_searchBuffer);
              std::transform(filenameSearch.begin(), filenameSearch.end(), filenameSearch.begin(), ::tolower);
              std::transform(query.begin(), query.end(), query.begin(), ::tolower);

              if (filenameSearch.find(query, 0) == std::string::npos)
                continue;
            }

            // check if extension matches
            if (!info.IsDirectory && m_type != IFD_DIALOG_DIRECTORY) {
              if (m_filterSelection < m_filterExtensions.size()) {
                const auto& exts = m_filterExtensions[m_filterSelection];
                if (exts.size() > 0 && info.Path.has_extension()) {
                  std::string extension = toLower(info.Path.extension().u8string());
                  // extension not found? skip
                  if (std::count(exts.begin(), exts.end(), extension) == 0)
                    continue;
                }
              }
            }

            m_content.push_back(info);

          }

          closedir(dirp);
#else
          for (const auto& entry : std::filesystem::directory_iterator(m_currentDirectory, ec)) {
            if (!m_contentLoaderRunning) {
              break;
            }

            if (IsHidden(entry.path())) continue;
            FileData info(entry.path());

            // skip files when IFD_DIALOG_DIRECTORY
            if (!info.IsDirectory && m_type == IFD_DIALOG_DIRECTORY)
              continue;

            // check if filename matches search query
            if (m_searchBuffer[0]) {
              std::string filename = info.Path.u8string();

              std::string filenameSearch = filename;
              std::string query(m_searchBuffer);
              std::transform(filenameSearch.begin(), filenameSearch.end(), filenameSearch.begin(), ::tolower);
              std::transform(query.begin(), query.end(), query.begin(), ::tolower);

              if (filenameSearch.find(query, 0) == std::string::npos)
                continue;
            }

            // check if extension matches
            if (!info.IsDirectory && m_type != IFD_DIALOG_DIRECTORY) {
              if (m_filterSelection < m_filterExtensions.size()) {
                const auto& exts = m_filterExtensions[m_filterSelection];
                if (exts.size() > 0 && info.Path.has_extension()) {
                  std::string extension = toLower(info.Path.extension().u8string());
                  // extension not found? skip
                  if (std::count(exts.begin(), exts.end(), extension) == 0)
                    continue;
                }
              }
            }

            m_content.push_back(info);
          }
#endif

          std::chrono::steady_clock::time_point stop = std::chrono::high_resolution_clock::now();
          printf("total listing time: %.3f ms\n", std::chrono::duration<float, std::milli>(stop - start).count());

          if (!m_contentLoaderRunning) return;

          m_sortContent(m_sortColumn, m_sortDirection);
          m_refreshIconPreview();

#if USE_CONTENTTHREAD == 1
        });
#endif
      }
    }
  }
  void FileDialog::m_sortContent(unsigned int column, unsigned int sortDirection)
  {
    // 0 -> name, 1 -> date, 2 -> size
    m_sortColumn = column;
    m_sortDirection = sortDirection;

    // split into directories and files
    std::partition(m_content.begin(), m_content.end(), [](const FileData& data) {
      return data.IsDirectory;
    });

    if (m_content.size() > 0) {
      // find where the file list starts
      size_t fileIndex = 0;
      for (; fileIndex < m_content.size(); fileIndex++)
        if (!m_content[fileIndex].IsDirectory)
          break;

      // compare function
      auto compareFn = [column, sortDirection](const FileData& left, const FileData& right) -> bool {
        // name
        if (column == 0) {
          std::string lName = left.Path.u8string();
          std::string rName = right.Path.u8string();

          std::transform(lName.begin(), lName.end(), lName.begin(), ::tolower);
          std::transform(rName.begin(), rName.end(), rName.begin(), ::tolower);

          int comp = lName.compare(rName);

          if (sortDirection == ImGuiSortDirection_Ascending)
            return comp < 0;
          return comp > 0;
        }
        // date
        else if (column == 1) {
          if (sortDirection == ImGuiSortDirection_Ascending)
            return left.DateModified < right.DateModified;
          else
            return left.DateModified > right.DateModified;
        }
        // size
        else if (column == 2) {
          if (sortDirection == ImGuiSortDirection_Ascending)
            return left.Size < right.Size;
          else
            return left.Size > right.Size;
        }

        return false;
      };

      // sort the directories
      std::sort(m_content.begin(), m_content.begin() + fileIndex, compareFn);

      // sort the files
      std::sort(m_content.begin() + fileIndex, m_content.end(), compareFn);
    }
  }

  void FileDialog::m_renderTree(FileTreeNode* node)
  {
    // directory
    std::error_code ec;
    ImGui::PushID(node);
    bool isClicked = false;
    std::string displayName = node->DisplayName.empty() ? node->Path.stem().u8string() : node->DisplayName;
    if (displayName.size() == 0)
      displayName = node->Path.u8string();

    bool isOpen = true;

    if (FolderNode(displayName.c_str(), (ImTextureID)m_getIcon(node->Path), isClicked, node->Special ? &isOpen : nullptr)) {
      if (!node->Read) {
        // cache children if it's not already cached
#if 1
        if (std::filesystem::exists(node->Path, ec))
          for (const auto& entry : std::filesystem::directory_iterator(node->Path, ec)) {
            if (std::filesystem::is_directory(entry, ec)) {
              if (IsHidden(entry.path())) continue;
              node->Children.push_back(new FileTreeNode(entry.path().u8string()));
            }
          }
#endif
        node->Read = true;
      }

      // display children
      for (auto c : node->Children)
        m_renderTree(c);

      ImGui::TreePop();
    }
    if (isClicked)
      m_setDirectory(node->Path);
    ImGui::PopID();
  }
  void FileDialog::m_renderContent()
  {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      m_selectedFileItem = -1;

    // table view
    if (m_zoom == 1.0f) {
      if (ImGui::BeginTable("##contentTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY, ImVec2(0, -FLT_MIN))) {
        // header
        ImGui::TableSetupScrollFreeze(0, 1);

        ImGui::TableSetupColumn("Name##filename", ImGuiTableColumnFlags_WidthStretch, 0.0f -1.0f, 0);
        ImGui::TableSetupColumn("Date modified##filedate", ImGuiTableColumnFlags_WidthStretch, 0.0f, 1);
        ImGui::TableSetupColumn("Size##filesize", ImGuiTableColumnFlags_WidthFixed, 0.0f, 2);

        ImGui::TableHeadersRow();

        // sort
        if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
          if (sortSpecs->SpecsDirty) {
            sortSpecs->SpecsDirty = false;
            if (m_mtxContent.try_lock()) {
              std::lock_guard<std::mutex> lock(m_mtxContent, std::adopt_lock);
              m_sortContent(sortSpecs->Specs->ColumnUserID, sortSpecs->Specs->SortDirection);
            }
          }
        }

        // content
        int fileId = 0;

        if (m_mtxContent.try_lock()) {
          std::lock_guard<std::mutex> lock(m_mtxContent, std::adopt_lock);
          for (auto& entry : m_content) {
            std::string filename = entry.Path.filename().u8string();
            if (filename.size() == 0)
              filename = entry.Path.u8string(); // drive

            bool isSelected = std::count(m_selections.begin(), m_selections.end(), entry.Path);

            ImGui::TableNextRow();

            // file name
            ImGui::TableSetColumnIndex(0);
            ImGui::Image((ImTextureID)m_getIcon(entry.Path), ImVec2(ICON_SIZE, ICON_SIZE));
            ImGui::SameLine();
            if (ImGui::Selectable(filename.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
              std::error_code ec;
              bool isDir = std::filesystem::is_directory(entry.Path, ec);

              if (ImGui::IsMouseDoubleClicked(0)) {
                if (isDir) {
                  m_setDirectory(entry.Path);
                  break;
                } else
                  m_finalize(filename);
              } else {
                if ((isDir && m_type == IFD_DIALOG_DIRECTORY) || !isDir)
                  m_select(entry.Path, ImGui::GetIO().KeyCtrl);
              }
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
              m_selectedFileItem = fileId;
            fileId++;

            // date
            ImGui::TableSetColumnIndex(1);
            auto tm = std::localtime(&entry.DateModified);
            if (tm != nullptr)
              ImGui::Text("%d/%d/%d %02d:%02d", tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year, tm->tm_hour, tm->tm_min);
            else ImGui::Text("---");

            // size
            ImGui::TableSetColumnIndex(2);
            if (!entry.IsDirectory) {
              std::stringstream ss;
              ss << HumanReadable{entry.Size};
              ImGui::Text("%s", ss.str().c_str());
            }
            else ImGui::Text("---");
          }
        } else {
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("Listing...");
        }
        ImGui::EndTable();
      }
    }
    // "icon" view
    else {
      // content
      int fileId = 0;

      if (m_mtxContent.try_lock()) {
        std::lock_guard<std::mutex> lock(m_mtxContent, std::adopt_lock);
        for (auto& entry : m_content) {
        if (entry.HasIconPreview && entry.IconPreviewData != nullptr) {
          entry.IconPreview = this->CreateTexture(entry.IconPreviewData, entry.IconPreviewWidth, entry.IconPreviewHeight, 1);
          stbi_image_free(entry.IconPreviewData);
          entry.IconPreviewData = nullptr;
        }

        std::string filename = entry.Path.filename().u8string();
        if (filename.size() == 0)
          filename = entry.Path.u8string(); // drive

        bool isSelected = std::count(m_selections.begin(), m_selections.end(), entry.Path);

        if (FileIcon(filename.c_str(), isSelected, entry.HasIconPreview ? entry.IconPreview : (ImTextureID)m_getIcon(entry.Path), ImVec2(32 + 16 * m_zoom, 32 + 16 * m_zoom), entry.HasIconPreview, entry.IconPreviewWidth, entry.IconPreviewHeight)) {
          std::error_code ec;
          bool isDir = std::filesystem::is_directory(entry.Path, ec);

          if (ImGui::IsMouseDoubleClicked(0)) {
            if (isDir) {
              m_setDirectory(entry.Path);
              break;
            }
            else
              m_finalize(filename);
          }
          else {
            if ((isDir && m_type == IFD_DIALOG_DIRECTORY) || !isDir)
              m_select(entry.Path, ImGui::GetIO().KeyCtrl);
          }
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
          m_selectedFileItem = fileId;
        fileId++;
      }
      }
    }
  }
  void FileDialog::m_renderPopups()
  {
    bool openAreYouSureDlg = false, openNewFileDlg = false, openNewDirectoryDlg = false;
    if (ImGui::BeginPopupContextItem("##dir_context")) {
      if (ImGui::Selectable("New file"))
        openNewFileDlg = true;
      if (ImGui::Selectable("New directory"))
        openNewDirectoryDlg = true;
      if (m_selectedFileItem != -1 && ImGui::Selectable("Delete"))
        openAreYouSureDlg = true;
      ImGui::EndPopup();
    }
    if (openAreYouSureDlg)
      ImGui::OpenPopup("Are you sure?##delete");
    if (openNewFileDlg)
      ImGui::OpenPopup("Enter file name##newfile");
    if (openNewDirectoryDlg)
      ImGui::OpenPopup("Enter directory name##newdir");
    if (ImGui::BeginPopupModal("Are you sure?##delete")) {
      if (m_selectedFileItem >= static_cast<int>(m_content.size()) || m_content.size() == 0)
        ImGui::CloseCurrentPopup();
      else {
        const FileData& data = m_content[m_selectedFileItem];
        ImGui::TextWrapped("Are you sure you want to delete %s?", data.Path.filename().u8string().c_str());
        if (ImGui::Button("Yes")) {
          std::error_code ec;
          std::filesystem::remove_all(data.Path, ec);
          m_setDirectory(m_currentDirectory, false); // refresh
          ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No"))
          ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Enter file name##newfile")) {
      ImGui::PushItemWidth(250.0f);
      ImGui::InputText("##newfilename", m_newEntryBuffer, 1024); // TODO: remove hardcoded literals
      ImGui::PopItemWidth();

      if (ImGui::Button("OK")) {
        std::ofstream out((m_currentDirectory / std::string(m_newEntryBuffer)).string());
        out << "";
        out.close();

        m_setDirectory(m_currentDirectory, false); // refresh
        m_newEntryBuffer[0] = 0;

        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) {
        m_newEntryBuffer[0] = 0;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Enter directory name##newdir")) {
      ImGui::PushItemWidth(250.0f);
      ImGui::InputText("##newfilename", m_newEntryBuffer, 1024); // TODO: remove hardcoded literals
      ImGui::PopItemWidth();

      if (ImGui::Button("OK")) {
        std::error_code ec;
        std::filesystem::create_directory(m_currentDirectory / std::string(m_newEntryBuffer), ec);
        m_setDirectory(m_currentDirectory, false); // refresh
        m_newEntryBuffer[0] = 0;
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
        m_newEntryBuffer[0] = 0;
      }
      ImGui::EndPopup();
    }
  }
  void FileDialog::m_renderFileDialog()
  {
    m_doClearIcons();
    m_doClearIconPreview();
    m_doRefreshIconPreview();
    m_doSetDirectory();

    /***** TOP BAR *****/
    bool noBackHistory = m_backHistory.empty(), noForwardHistory = m_forwardHistory.empty();

    ImGui::PushStyleColor(ImGuiCol_Button, 0);
    if (noBackHistory) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    if (ImGui::ArrowButtonEx("##back", ImGuiDir_Left, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE), m_backHistory.empty() * ImGuiItemFlags_Disabled)) {
      std::filesystem::path newPath = m_backHistory.top();
      m_backHistory.pop();
      m_forwardHistory.push(m_currentDirectory);

      m_setDirectory(newPath, false);
    }
    if (noBackHistory) ImGui::PopStyleVar();
    ImGui::SameLine();

    if (noForwardHistory) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    if (ImGui::ArrowButtonEx("##forward", ImGuiDir_Right, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE), m_forwardHistory.empty() * ImGuiItemFlags_Disabled)) {
      std::filesystem::path newPath = m_forwardHistory.top();
      m_forwardHistory.pop();
      m_backHistory.push(m_currentDirectory);

      m_setDirectory(newPath, false);
    }
    if (noForwardHistory) ImGui::PopStyleVar();
    ImGui::SameLine();

    if (ImGui::ArrowButtonEx("##up", ImGuiDir_Up, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE))) {
      if (m_currentDirectory.has_parent_path())
        m_setDirectory(m_currentDirectory.parent_path());
    }

    std::filesystem::path curDirCopy = m_currentDirectory;
    if (PathBox("##pathbox", curDirCopy, m_pathBuffer, ImVec2(-250, GUI_ELEMENT_SIZE)))
      m_setDirectory(curDirCopy);
    ImGui::SameLine();

    if (FavoriteButton("##dirfav", std::count(m_favorites.begin(), m_favorites.end(), m_currentDirectory.u8string()))) {
      if (std::count(m_favorites.begin(), m_favorites.end(), m_currentDirectory.u8string()))
        RemoveFavorite(m_currentDirectory.u8string());
      else
        AddFavorite(m_currentDirectory.u8string());
    }
    ImGui::SameLine();
    ImGui::PopStyleColor();

    if (ImGui::InputTextEx("##searchTB", "Search", m_searchBuffer, 128, ImVec2(-FLT_MIN, GUI_ELEMENT_SIZE), 0)) // TODO: no hardcoded literals
      m_setDirectory(m_currentDirectory, false); // refresh



    /***** CONTENT *****/
    float bottomBarHeight = (GImGui->FontSize + ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().ItemSpacing.y * 2.0f) * 2;
    if (ImGui::BeginTable("##table", 2, ImGuiTableFlags_Resizable, ImVec2(0, -bottomBarHeight))) {
      ImGui::TableSetupColumn("##tree", ImGuiTableColumnFlags_WidthFixed, 125.0f);
      ImGui::TableSetupColumn("##content", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableNextRow();

      // the tree on the left side
      ImGui::TableSetColumnIndex(0);
      ImGui::BeginChild("##treeContainer", ImVec2(0, -bottomBarHeight));
      for (auto node : m_treeCache)
        m_renderTree(node);
      ImGui::EndChild();

      // content on the right side
      ImGui::TableSetColumnIndex(1);
      ImGui::BeginChild("##contentContainer", ImVec2(0, -bottomBarHeight));
        m_renderContent();
      ImGui::EndChild();
      if (ImGui::IsItemHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f) {
        m_zoom = std::min<float>(25.0f, std::max<float>(1.0f, m_zoom + ImGui::GetIO().MouseWheel));
        m_refreshIconPreview();
      }

      // New file, New directory and Delete popups
      m_renderPopups();

      ImGui::EndTable();
    }



    /***** BOTTOM BAR *****/
    ImGui::Text("File name:");
    ImGui::SameLine();
    if (ImGui::InputTextEx("##file_input", "Filename", m_inputTextbox, 1024, ImVec2((m_type != IFD_DIALOG_DIRECTORY) ? -250.0f : -FLT_MIN, 0), ImGuiInputTextFlags_EnterReturnsTrue)) {
      bool success = m_finalize(std::string(m_inputTextbox));
#ifdef _WIN32
      if (!success)
        MessageBeep(MB_ICONERROR);
#elif defined(__APPLE__)
      if (!success)
        Beep();
#else
      (void)success;
#endif
    }
    if (m_type != IFD_DIALOG_DIRECTORY) {
      ImGui::SameLine();
      ImGui::SetNextItemWidth(-FLT_MIN);
      int sel = static_cast<int>(m_filterSelection);
      if (ImGui::Combo("##ext_combo", &sel, m_filter.c_str())) {
        m_filterSelection = static_cast<size_t>(sel);
        m_setDirectory(m_currentDirectory, false); // refresh
      }
    }

    // buttons
    float ok_cancel_width = GUI_ELEMENT_SIZE * 7;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ok_cancel_width);
    if (ImGui::Button(m_type == IFD_DIALOG_SAVE ? "Save" : "Open", ImVec2(ok_cancel_width / 2 - ImGui::GetStyle().ItemSpacing.x, 0.0f))) {
      std::string filename(m_inputTextbox);
      bool success = false;
      if (!filename.empty() || m_type == IFD_DIALOG_DIRECTORY)
        success = m_finalize(filename);
#ifdef _WIN32
      if (!success)
        MessageBeep(MB_ICONERROR);
#elif defined(__APPLE__)
      if (!success)
        Beep();
#else
      (void)success;
#endif
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(-FLT_MIN, 0.0f))) {
      if (m_type == IFD_DIALOG_DIRECTORY)
        m_isOpen = false;
      else
        m_finalize();
    }

    int escapeKey = ImGui::GetIO().KeyMap[ImGuiKey_Escape];
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
       escapeKey >= 0 && ImGui::IsKeyPressed(ImGuiKey_Escape))
      m_isOpen = false;
  }
}
