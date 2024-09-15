#pragma once

#include "IP_Result.h"

#include <string>
#include <vector>

IP_Result dns_resolve_hostname(int sockfd, const std::string &hostname);
