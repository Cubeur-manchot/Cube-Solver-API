#pragma once
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "cpp-httplib/httplib.h"

#include "333_Solver_cmd/Output.hpp"

void handleSolveRequest(const httplib::Request& req, httplib::Response& res);

std::vector<std::string> runSolver(Input const& input);