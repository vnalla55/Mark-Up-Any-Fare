#pragma once

#include "DBAccess/RemoteCache/RemoteCacheHeader.h"
#include "DBAccess/RemoteCache/ASIOServer/Reply.h"

#include <string>

namespace tse
{

namespace RemoteCache
{

class RCRequest;
class RCServerAttributes;

namespace RCStatistics
{

void registerServerCall(const std::string& client,
                        const std::string& dataType,
                        const Reply& reply,
                        bool fromQuery = false);
void registerClientCall(const RCServerAttributes& serverAttributes,
                        const RCRequest& request);

}// RCStatistics

} // RemoteCache

} // tse
