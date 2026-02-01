#pragma once
#include <filesystem>

namespace defPath
{
	struct defaultPath
	{
		inline const static std::filesystem::path sertificateFile = R"(C:\Users\User\source\repos\CloudeService\CloudeServiceServer\bin\cert\localhost.pem)";
		inline const static std::filesystem::path sertificateKeyFile = R"(C:\Users\User\source\repos\CloudeService\CloudeServiceServer\bin\cert\localhost-key.pem)";
		inline const static std::filesystem::path serverBin = "C:/Users/User/source/repos/CloudeService/CloudeServiceServer/bin";
		inline const static std::filesystem::path jsonFolder = "C:/Users/User/source/repos/CloudeService/CloudeServiceServer/bin/json";
		inline const static std::filesystem::path serverFolder = "D:/S/Server";
		inline const static std::filesystem::path fileDataBase = "C:/Users/User/source/repos/CloudeService/CloudeServiceServer/bin/sql/DataBaseFile/FileDataBase.db";
		inline const static std::filesystem::path hashDataBase = "C:/Users/User/source/repos/CloudeService/CloudeServiceServer/bin/sql/DataBaseFile/HashDataBase.db";
		inline const static std::filesystem::path userDataBase = "C:/Users/User/source/repos/CloudeService/CloudeServiceServer/bin/sql/DataBaseFile/UserDataBase.db";
		inline const static std::filesystem::path tokenDataBase = "C:/Users/User/source/repos/CloudeService/CloudeServiceServer/bin/sql/DataBaseFile/TokenDataBase.db";
	};
}