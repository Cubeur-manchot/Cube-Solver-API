#pragma once
#ifdef _WIN32
#include <winsock2.h>
#endif
#include "cpp-httplib/httplib.h"

void handleSolveRequest(const httplib::Request& req, httplib::Response& res);