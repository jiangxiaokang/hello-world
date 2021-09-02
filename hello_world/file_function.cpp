#include "file_function.h"

namespace MyFileUtility 
{

	bool FindMatchedFileNameList(const std::string& dir_path, const std::string& key,
		std::list<std::string>& file_list)
	{
		if (!exists(dir_path)) {
			return false;
		}
		directory_entry dr(dir_path);

		return FindMatchedFileNameListByEntry(dr, key, file_list);
	}
	bool FindMatchedFileNameListByEntry(const directory_entry& de, const std::string& key,
		std::list<std::string>& file_list)
	{
		if (de.status().type() == file_type::regular) {
			//нд╪Ч
			std::string str = de.path().root_path().string();
			if (str.find(key) != str.npos) {
				file_list.push_back(de.path().string());
			}
			return true;
		}
		if (de.status().type() != file_type::directory) {
			return false;
		}
		directory_iterator di(de);
		for (auto& it : di) {
			switch (it.status().type())
			{
			case file_type::regular:
			{
				std::string str = it.path().filename().string();
				if (str.find(key) != str.npos) {
					file_list.push_back(it.path().string());
				}
			}
			break;
			case file_type::directory:
			{
				FindMatchedFileNameListByEntry(it, key, file_list);
			}
			break;
			default:
				break;
			}
		}
		return true;
	}
}


