#pragma once

#include <chrono>
#include <array>
#include <hscpp/Hotswapper.h>

struct HsCppProgress {
	const char * lastCompilingText;
	std::array<uint8_t, 3> lastCompilingColor;
#ifdef __linux__ 
	std::chrono::system_clock::time_point startCompileTime;
#else
	std::chrono::steady_clock::time_point startCompileTime;
#endif
	std::chrono::duration<float, std::milli> lastElapsedCompileTime;
	hscpp::Hotswapper::UpdateResult lastUpdateResult;

	HsCppProgress() {
		lastCompilingText = "Ready";
		lastCompilingColor = {0, 0, 0};
		lastUpdateResult = hscpp::Hotswapper::UpdateResult::Idle;
	}
};
