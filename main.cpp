#include <iostream>
#include <sstream>
#include "handle_request.hpp"

int main() {
	std::cout << "Entering main" << std::endl;
	httplib::Server server;
	server.Post("/solve", handleSolveRequest);
	std::cout << "Server listening on http://localhost:8080" << std::endl;
    server.listen("127.0.0.1", 8080);
    return 0;
}
