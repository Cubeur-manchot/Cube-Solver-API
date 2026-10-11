#include <string>
#include <regex>
#include <ranges>

#include "333_Solver_cmd/json.hpp"
#include "build_input.hpp"
#include "api_error.hpp"

nlohmann::json parseJsonObject(const std::string& jsonString) {
	nlohmann::json jsonObject;
	try {
		jsonObject = nlohmann::json::parse(jsonString);
    } catch (const nlohmann::json::exception& e) {
        throw ApiError::badRequest("Malformed JSON");
    }
	if (!jsonObject.is_object()) {
		throw ApiError::badRequest("JSON root must be an object");
	}
	return jsonObject;
}

bool is_non_empty_array_of_strings(const nlohmann::json& jsonObject) {
    if (!jsonObject.is_array()) {
		return false;
	}
    if (jsonObject.empty()) {
		return false;
	}
    for (const auto& item : jsonObject) {
        if (!item.is_string()) {
            return false;
        }
    }
    return true;
}

void validateFieldPresenceAndType(const nlohmann::json& jsonObject) {
	using Check = bool (nlohmann::json::*)() const;
	struct Field {
		std::string name;
		Check check;
		std::string typeName;
		bool isArrayOfStrings = false;
	};
	static const std::vector<Field> fields = {
		{"scramble", &nlohmann::json::is_string, "string"},
		{"step", &nlohmann::json::is_string, "string"},
		{"allowedMoves", &nlohmann::json::is_array, "non-empty array of strings", true},
		{"minMoves", &nlohmann::json::is_number_unsigned, "positive int"},
		{"maxMoves", &nlohmann::json::is_number_unsigned, "positive int"},
		{"metric", &nlohmann::json::is_string, "string"},
		{"singleSolution", &nlohmann::json::is_boolean, "boolean"}
	};
	std::vector<std::string> errors;
	for (const auto& [name, typeCheck, typeName, isArrayOfStrings] : fields) {
		auto it = jsonObject.find(name);
		if (it == jsonObject.end()) {
			errors.push_back("Missing required field: '" + name + "'.");
			continue;
		}
		if (it->is_null()) {
			errors.push_back("Field '" + name + "' cannot be null.");
			continue;
		}
		if (isArrayOfStrings) {
			if (!is_non_empty_array_of_strings(*it)) {
				errors.push_back("Field '" + name + "' must be a non-empty array of strings.");
			}
		} else { // primitive types
			if (!((*it).*typeCheck)()) {
				errors.push_back("Field '" + name + "' must be of type '" + typeName + "'.");
			}
		}
	}
	if (!errors.empty()) {
		throw ApiError::validationErrorMultiple(errors);
	}
}

	}
}

std::vector<std::string_view> splitString(std::string_view text, char delimiter = ' ') {
	return text | std::views::split(delimiter)
	            | std::views::filter([](auto&& token) { return !token.empty(); })
	            | std::views::transform([](auto&& token) { return std::string_view(token); })
	            | std::ranges::to<std::vector>();
}

void validateFieldValues(const nlohmann::json& jsonObject) {
	std::vector<std::string> errors;
	// step
	std::string step = jsonObject.at("step").get<std::string>();
	bool isStepValid = false;
	std::vector<std::string> validSteps;
	size_t validStepsCount = static_cast<size_t>(Step::ImportTable) - 1; // ImportTable is at last place, and both None and Importtable are invalid
	validSteps.reserve(validStepsCount);
	for (size_t i = 1; i < validStepsCount; ++i) {
		std::string validStep = nlohmann::json(static_cast<Step>(i)).get<std::string>();
		isStepValid = isStepValid || step == validStep;
		validSteps.push_back("'" + validStep + "'");
	}
	if (!isStepValid) {
		errors.push_back("Invalid value '" + step + "' for field 'step'. Valid values are: " + joinStrings(validSteps) + ".");
	}
	// metric
	const std::string& metric = jsonObject.at("metric").get_ref<const std::string&>();
	if (metric != "HTM" && metric != "QTM") {
		errors.push_back("Invalid value '" + metric + "' for field 'metric'. Valid values are: 'HTM', 'QTM'.");
	}
	// allowedMoves
	static const std::regex pattern(R"(^([UFRDBL]w?|[ufrdblEMSxyz])[123]?'?$)");
        if (!std::regex_match(moveString, pattern)) {
	for (const auto& move : jsonObject.at("allowedMoves")) {
        const std::string& moveString = move.get_ref<const std::string&>();
            errors.push_back("Invalid move in 'allowedMoves': '" + moveString + "'.");
        }
    }
	// scramble
	const std::string& scramble = jsonObject.at("scramble").get_ref<const std::string&>();
	auto moves = splitString(scramble);
	if (moves.empty()) {
		errors.push_back("Field 'scramble' must contain at least one move.");
	} else {
		for (const auto& move : moves) {
			if (!std::regex_match(move, pattern)) {
				errors.push_back("Invalid move in 'scramble': '" + move + "'.");
			}
		}
	}
	// minMoves, maxMoves
	unsigned int minMoves = jsonObject.at("minMoves").get<unsigned int>();
	unsigned int maxMoves = jsonObject.at("maxMoves").get<unsigned int>();
	if (minMoves == 0 || maxMoves == 0 || maxMoves >= 25) {
		if (minMoves == 0) {
			errors.push_back("Field 'minMoves' must be a strictly positive integer.");
		}
		if (maxMoves == 0) {
			errors.push_back("Field 'maxMoves' must be a strictly positive integer.");
		}
		if (maxMoves >= 25) {
			errors.push_back("Field 'maxMoves' must be strictly lower than 25.");
		}
	} else {
		if (minMoves > maxMoves) {
			errors.push_back("Field 'maxMoves' must be greater than or equal to 'minMoves'.");
		}
	}
	if (!errors.empty()) {
		throw ApiError::validationErrorMultiple(errors);
	}
}

std::string joinStrings(const std::vector<std::string>& strings, const std::string& separator) {
    if (strings.empty()) {
		return "";
	}
    std::string result = strings.front();
    for (auto it = std::next(strings.begin()); it != strings.end(); ++it)
    {
        result += separator;
        result += *it;
    }
    return result;
}

void validateJsonInput(nlohmann::json& jsonObject) {
	validateFieldPresenceAndType(jsonObject);
	validateFieldValues(jsonObject);
}

Input createInput(const nlohmann::json& jsonObject) {
	try {
		return jsonObject.get<Input>();
	} catch (const nlohmann::json::exception& e) {
		throw ApiError::validationError(std::string("Invalid input format: ") + e.what());
	}
}

Input buildInput(const std::string& body) {
	nlohmann::json jsonInput = parseJsonObject(body);
	validateJsonInput(jsonInput);
	Input input = createInput(jsonInput);
	return input;
}