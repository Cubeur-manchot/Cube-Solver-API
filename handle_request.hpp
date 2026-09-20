#pragma once
#include <winsock2.h>
#include "cpp-httplib/httplib.h"
#include "333_Solver_cmd/Output.hpp"

void handleSolveRequest(const httplib::Request& req, httplib::Response& res);

std::vector<std::string> runSolver(Input const& input);