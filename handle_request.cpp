#include "handle_request.hpp"
#include "build_input.hpp"

void handleSolveRequest(const httplib::Request& req, httplib::Response& res) {
    try {
		Input input = buildInput(req.body);
		std::vector<std::string> solutions = runSolver(input);
		res.status = 200;
		res.set_content(nlohmann::json(solutions).dump(), "application/json");
	} catch (const ApiError& e) {
		res.status = e.status;
		res.set_content(e.to_json().dump(), "application/json");
	} catch (const std::exception& e) {
		std::cerr << "Internal server error: " << e.what() << std::endl;
		res.status = 500;
		res.set_content(nlohmann::json{{"error", "Internal server error"}}.dump(), "application/json");
	}
}

std::vector<std::string> runSolver(Input const& input) {
	try {
		Solutions solutions = solve(input);
		Output output = Output{solutions};
		if (output.errorMessage.has_value()) {
			throw ApiError::validationError(output.errorMessage.value());
		}
		return output.solutions;
	} catch (const std::exception& e) {
		throw ApiError::internal(std::string("Error during solving: ") + e.what());
	}
}