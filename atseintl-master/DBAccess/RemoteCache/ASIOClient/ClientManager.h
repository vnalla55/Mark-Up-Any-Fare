#pragma once

#include <memory>

namespace tse
{

namespace RemoteCache
{

class RCServerAttributes;

typedef std::shared_ptr<struct RCRequest> RCRequestPtr;

namespace ClientManager
{

bool enqueue(RCServerAttributes& serverAttributes,
             RCRequestPtr request);
void start();
void stop();

void healthcheck(const std::string& host,
                 const std::string& port,
                 std::string& msg);

}// ClientManager

}// RemoteCache

}// tse
