#pragma once
#include <exception>
#include <string>

struct ApiError : std::exception {
    int status;
	std::vector<std::string> messages;
    ApiError(int status, std::vector<std::string> messages)
        : status(status),
          messages(std::move(messages))
    {}
    const char* what() const noexcept override {
        return messages.empty() ? "ApiError" : messages.front().c_str();
    }
    static ApiError badRequest(std::string message) {
        return ApiError(400, {std::move(message)});
    }
    static ApiError validationError(std::string message) {
        return ApiError(422, {std::move(message)});
    }
	static ApiError validationErrorMultiple(std::vector<std::string> messages) {
        return ApiError(422, std::move(messages));
    }
    static ApiError internal(std::string message) {
        return ApiError(500, {std::move(message)});
    }
	nlohmann::json to_json() const {
		return {
			{"errors", messages}
		};
	}
};