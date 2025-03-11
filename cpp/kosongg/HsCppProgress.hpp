#pragma once

#include <chrono>
#include <array>

struct HsCppProgress {
	const char * lastCompilingText;
	std::array<uint8_t, 3> lastCompilingColor;
#ifdef __linux__
	std::chrono::system_clock::time_point startCompileTime;
#else
	std::chrono::steady_clock::time_point startCompileTime;
#endif
	std::chrono::duration<float, std::milli> lastElapsedCompileTime;
	int lastUpdateResult;

	HsCppProgress() {
		lastCompilingText = "Ready";
		lastCompilingColor = {0, 0, 0};
		lastUpdateResult = 0;
		startCompileTime = std::chrono::high_resolution_clock::now();
		lastElapsedCompileTime = startCompileTime - startCompileTime;
	}
};
