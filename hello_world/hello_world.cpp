// hello_world.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "file_function.h"
int main()
{
	std::string path(R"(F:\git_repos)");
	std::list<std::string> file_list;
	MyFileUtility::FindMatchedFileNameList(path, ".cpp", file_list);
	for (auto& file : file_list) {
		std::cout << file << std::endl;
	}
	system("pause");
	return 0;
}

