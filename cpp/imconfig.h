#pragma once

#if defined(_WIN32)
#  if defined(imgui_EXPORTS)
#    define IMGUI_API __declspec(dllexport)					// MSVC Windows: DLL export
#  elif defined(imgui_IMPORTS)
#    define IMGUI_API __declspec(dllimport)                 // MSVC Windows: DLL import
#  endif
#elif defined(imgui_EXPORTS)
#  define IMGUI_API __attribute__((visibility("default")))	// GCC/Clang: override visibility when set is hidden
#endif
#ifndef IMGUI_API
#  define IMGUI_API
#endif
