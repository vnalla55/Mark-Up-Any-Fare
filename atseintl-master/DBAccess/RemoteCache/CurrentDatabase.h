#pragma once

#include <string>

namespace tse
{

namespace RemoteCache
{

void recordCurrentDatabase(const std::string& database,
                           bool isHistorical,
                           int numberConnections);

std::string getCurrentDatabase(bool isHistorical,
                               bool& inTransition);

}// RemoteCache

}// tse
