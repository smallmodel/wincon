#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace JsonHelpers
{
	static std::wstring getUnicodeJsonData(const nlohmann::json& jsonData)
	{
		const std::string jsonString = jsonData.dump();
		return std::wstring(jsonString.cbegin(), jsonString.cend());
	}
}
