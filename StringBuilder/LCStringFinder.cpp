#include <iostream>
#include <fstream>
#include <filesystem>
#include "LCStringFinder.h"

static constexpr auto cServerStringPath = "old_locale_string.txt";
static constexpr auto cClientStringPath = "new_locale_string.txt";
static constexpr auto cOut = "Result_String.txt";

StringBuilder::StringBuilder()
{
	if (ReadClientString() == false)
		return;

	if (ReadSourceString() == false)
		return;

	MergeLocaleString();

	FindPlaceInSource();
}

StringBuilder::~StringBuilder()
{
#if defined(ELAPSED_TIME)
	const auto end = std::chrono::steady_clock::now();
	std::cout << "Elapsed Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
#endif
}

/*Stage 1*/
bool StringBuilder::ReadClientString()
{
	std::ifstream File(cClientStringPath);

	if (File.is_open() == false) {
		printf("Cannot Open: %s\n", cClientStringPath);
		return false;
	}

	std::string sLine;
	while (std::getline(File, sLine)) {
		if (sLine.empty())
			continue;

		auto pos = sLine.find("\t");
		if (pos == std::string::npos)
			continue;

		if (std::string_view(sLine).substr(pos + 1).empty() == true)
			continue;

		auto key = sLine.substr(pos + 1);
		key.insert(0, "\"");
		key.append("\"");

		mClient.emplace(std::move(key), sLine.substr(0, pos));
	}

	return true;
}

/*Stage 2*/
bool StringBuilder::ReadSourceString()
{
	std::ifstream File(cServerStringPath);

	if (File.is_open() == false) {
		printf("Cannot Open: %s\n", cServerStringPath);
		return false;
	}

	std::string sLine, sTemp;
	bool bFirst = true;
	while (std::getline(File, sLine)) {
		if (sLine.empty())
			continue;

		if (bFirst == false) {
			sTemp.pop_back();
			sLine.pop_back();
			mSource.emplace(std::move(sTemp), sLine);
		}
		else
			sTemp = sLine;

		bFirst = !bFirst;
	}

	return true;
}

/*Stage 3*/
void StringBuilder::MergeLocaleString()
{
	decltype(mSource) mTemp;

	for (const auto& [k, v] : mSource) {
		auto it = mClient.find(v);
		if (it == mClient.end())
			continue;

		const auto& sID = it->second;
		if (sID.empty() == false)
			mTemp.emplace(k, sID);
	}

	mSource = std::move(mTemp);
}

/*Stage 4*/
void StringBuilder::FindPlaceInSource()
{
	for (const auto& entry : std::filesystem::recursive_directory_iterator(std::filesystem::current_path())) {
		const auto FileName = entry.path().string();
		const auto GetExtension = std::string_view(FileName).substr(FileName.find_last_of(".") + 1);

		if (GetExtension.compare("cpp"))
			continue;

		FindPlaceInSource(FileName);
	}

	Out();
}

static std::vector<std::string> vResult;

void StringBuilder::FindPlaceInSource(const std::string& path)
{
	std::ifstream File(path);

	if (File.is_open() == false) {
		printf("Cannot Open: %s\n", path.c_str());
		return;
	}

	std::uint16_t iLine = 0;
	std::string sLine;
	while (std::getline(File, sLine)) {
		iLine++;

		if (sLine.empty())
			continue;

		for (const auto& [k, v] : mSource) {
			if (sLine.find(k) != std::string::npos) {
				auto sRes = path + "(Line:" + std::to_string(iLine) + ") id: " + v;
				vResult.emplace_back(std::move(sRes));
			}
		}
	}
}

/*Stage 5*/
void StringBuilder::Out()
{
	auto ofs = std::ofstream(cOut, std::ios::out | std::ios::trunc);

	for (const auto& s : vResult)
		ofs << s << std::endl;

	ofs.close();

	printf("%zu Result...\n", vResult.size());
}