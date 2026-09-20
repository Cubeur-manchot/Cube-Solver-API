#pragma once
#include <string>
#include <regex>
#include "333_Solver_cmd/json.hpp"
#include "333_Solver_cmd/Input.hpp"
#include "api_error.hpp"

Input buildInput(const std::string& body);
nlohmann::json parseJsonObject(const std::string& body);
void validateJsonInput(nlohmann::json& jsonInput);
void validateFieldPresenceAndType(const nlohmann::json& jsonObject);
bool is_non_empty_array_of_strings(const nlohmann::json& jsonObject);
void validateFieldValues(nlohmann::json& jsonObject);
std::string joinStrings(const std::vector<std::string>& strings, const std::string& separator = ", ");
std::vector<std::string> splitString(const std::string& string, const std::regex& delimiter = std::regex(R"(\s+)"));
Input createInput(const nlohmann::json& jsonInput);