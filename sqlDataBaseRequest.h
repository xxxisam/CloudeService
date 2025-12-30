#pragma once

#include <string>

// "print", // "/insert", "/get", "/exist", "/delete" with fmi "/sync"
struct sqlDataBaseRequest
{
	static inline const std::string sqlPrint = "/print";
	static inline const std::string sqlInsert = "/insert";
	static inline const std::string sqlGet = "/get";
	static inline const std::string sqlFind = "/find";
	static inline const std::string sqlDelete = "/delete";
	static inline const std::string sqlSync = "/sync";
};