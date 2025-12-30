#pragma once
#include <filesystem>

namespace defPath
{
	struct defaultPath
	{
		inline const static std::filesystem::path jsonFolder = "C:/Users/User/source/repos/CloudeService/CloudeServiceServer/bin/json";
		inline const static std::filesystem::path serverFolder = "D:/S/Server";
		inline const static std::filesystem::path fileDataBase = "C:/Users/User/source/repos/CloudeService/CloudeServiceServer/bin/sql/DataBaseFile/FileDataBase.db";
	};
}