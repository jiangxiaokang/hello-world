#pragma once
#include <filesystem>
#include <list>
#include <string>

namespace MyFileUtility 
{
	using namespace std::filesystem;
	bool FindMatchedFileNameList(const std::string& dir_path,const std::string& key,
		std::list<std::string>& file_list);

	bool FindMatchedFileNameListByEntry(const directory_entry& di, const std::string& key,
		std::list<std::string>& file_list);
}
