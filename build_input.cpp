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

struct StepAlias {
	std::string_view english;
	std::string_view french;
};

constexpr auto stepAliases = std::to_array<StepAlias>({
	{"Cross", "Croix"},
	{"Cross+1", "Croix+1"},
	{"Cross+2", "Croix+2"},
	{"Cross+3", "Croix+3"},
	{"Cross+4", "Croix+4"},
});

void normalizeStepName(nlohmann::json& jsonObject) { // if step name is in English, translate it to French, because French is expected by the solver
	std::string& stepName = jsonObject.at("step").get_ref<std::string&>();
	const auto matchingAlias = std::ranges::find(stepAliases, stepName, &StepAlias::english);
	if (matchingAlias != stepAliases.end()) {
		stepName = matchingAlias->french;
	}
}

struct ValidSteps {
	std::vector<std::string> frenchNames;
	std::string englishNamesMessage;
};

std::string_view toEnglishStepName(std::string_view frenchName) {
	const auto matchingAlias = std::ranges::find(stepAliases, frenchName, &StepAlias::french);
	return matchingAlias != stepAliases.end() ? matchingAlias->english : frenchName;
}

const ValidSteps& getValidSteps() {
	static const ValidSteps validSteps = [] {
		constexpr size_t firstValidStep = static_cast<size_t>(Step::None) + 1; // None appears first and is excluded
		constexpr size_t stepsEnd = static_cast<size_t>(Step::ImportTable); // ImportTable appears last and is excluded
		ValidSteps result;
		std::vector<std::string> quotedEnglishNames;
		result.frenchNames.reserve(stepsEnd - firstValidStep);
		quotedEnglishNames.reserve(stepsEnd - firstValidStep);
		for (size_t stepIndex = firstValidStep; stepIndex < stepsEnd; ++stepIndex) {
			std::string frenchName = nlohmann::json(static_cast<Step>(stepIndex)).get<std::string>();
			quotedEnglishNames.push_back("'" + std::string(toEnglishStepName(frenchName)) + "'");
			result.frenchNames.push_back(std::move(frenchName));
		}
		result.englishNamesMessage = quotedEnglishNames | std::views::join_with(std::string_view(", ")) | std::ranges::to<std::string>();
		return result;
	}();
	return validSteps;
}

static const std::regex movePattern(R"(([UFRDBL]w?|[ufrdblEMSxyz])[123]?'?)");

bool isValidMove(std::string_view move) {
	return std::regex_match(move.begin(), move.end(), movePattern);
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
	const std::string& step = jsonObject.at("step").get_ref<const std::string&>();
	const ValidSteps& validSteps = getValidSteps();
	if (!std::ranges::contains(validSteps.frenchNames, step)) {
		errors.push_back("Invalid value '" + step + "' for field 'step'. Valid values are: " + validSteps.englishNamesMessage + ".");
	}
	// metric
	const std::string& metric = jsonObject.at("metric").get_ref<const std::string&>();
	if (metric != "HTM" && metric != "QTM") {
		errors.push_back("Invalid value '" + metric + "' for field 'metric'. Valid values are: 'HTM', 'QTM'.");
	}
	// allowedMoves
	static const std::regex pattern(R"(^([UFRDBL]w?|[ufrdblEMSxyz])[123]?'?$)");
	for (const auto& move : jsonObject.at("allowedMoves")) {
        const std::string& moveString = move.get_ref<const std::string&>();
        if (!isValidMove(moveString)) {
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
			if (!isValidMove(move)) {
				errors.push_back("Invalid move in 'scramble': '" + std::string(move) + "'.");
			}
		}
	}
	// minMoves, maxMoves
	unsigned int minMoves = jsonObject.at("minMoves").get<unsigned int>();
	unsigned int maxMoves = jsonObject.at("maxMoves").get<unsigned int>();
	if (minMoves == 0) {
		errors.push_back("Field 'minMoves' must be a strictly positive integer.");
	}
	if (maxMoves == 0) {
		errors.push_back("Field 'maxMoves' must be a strictly positive integer.");
	}
	if (maxMoves >= 25) {
		errors.push_back("Field 'maxMoves' must be strictly lower than 25.");
	}
	if (minMoves > maxMoves && maxMoves > 0 && maxMoves < 25) {
		errors.push_back("Field 'maxMoves' must be greater than or equal to 'minMoves'.");
	}
	if (!errors.empty()) {
		throw ApiError::validationErrorMultiple(errors);
	}
}

void validateAndNormalizeJsonInput(nlohmann::json& jsonObject) {
	validateFieldPresenceAndType(jsonObject);
	normalizeStepName(jsonObject);
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
	validateAndNormalizeJsonInput(jsonInput);
	Input input = createInput(jsonInput);
	return input;
}