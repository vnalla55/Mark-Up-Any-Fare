// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

namespace tse
{
class DataHandle;
}

namespace tax
{
class XmlCache;
class DefaultServices;
class Request;

class RequestToV2CacheBuilder
{
public:
  void buildCache(DefaultServices& services,
                  const XmlCache& /*xmlCache*/,
                  const Request& request,
                  tse::DataHandle& dataHandle);
};

} // namespace tax
