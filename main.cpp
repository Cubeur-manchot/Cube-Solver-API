#include <iostream>
#include <cstdlib>
#include <string>

#include "handle_request.hpp"

std::string getEnvironmentVariable(const char* variableName, const std::string& fallbackValue) {
	const char* variableValue = std::getenv(variableName);
	if (variableValue) {
		return variableValue;
	} else {
		std::cout << "Environment variable " << variableName << " is missing. Using default value " << fallbackValue << " instead." << std::endl;
		return fallbackValue;
	}
}

int main() {
	std::cout << "Entering main" << std::endl;
    const std::string host = getEnvironmentVariable("APP_HOST", "127.0.0.1");
    const int port = std::stoi(getEnvironmentVariable("APP_PORT", "8080"));
	httplib::Server server;
	server.Post("/solve", handleSolveRequest);
	std::cout << "Server listening on http://" << host << ":" << port << "..." << std::endl;
    server.listen(host, port);
    return 0;
}
