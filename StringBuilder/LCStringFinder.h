#pragma once

#include <unordered_map>
//#define ELAPSED_TIME

#if defined(ELAPSED_TIME)
#include <chrono>
#endif

class StringBuilder
{
public:
	StringBuilder();
	~StringBuilder();

	bool ReadClientString(); // Stage 1
	bool ReadSourceString(); // Stage 2
	void MergeLocaleString(); // Stage 3
	void FindPlaceInSource(); // Stage 4
	void FindPlaceInSource(const std::string& path); // Stage 4
	void Out(); // Stage 5

private:
	std::unordered_map<std::string, std::string> mClient, mSource;

#ifdef ELAPSED_TIME
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif
};