#pragma once

#ifndef _USE_HSCPP_

#define HSCPP_TRACK(type, key)
#define Hscpp_SetSwapHandler(cb) (void)cb
#define Hscpp_IsSwapping() false
#define hscpp_virtual

// These macros are parsed by the hscpp::FileParser, if the hscpp::Preprocessor feature is enabled.
#define hscpp_require_source(...)
#define hscpp_require_include_dir(...)
#define hscpp_require_library(...)
#define hscpp_require_library_dir(...)
#define hscpp_require_preprocessor_def(...)
#define hscpp_module(module)
#define hscpp_message(message)

#define hscpp_if(...)
#define hscpp_elif(...)
#define hscpp_else()
#define hscpp_end()
#define hscpp_return()

#endif
