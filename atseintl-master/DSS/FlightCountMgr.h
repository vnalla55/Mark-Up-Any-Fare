//-------------------------------------------------------------------
//  Updates:
//
//	Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "AppConsole/SocketUtils.h"
#include "Common/TseCodeTypes.h"

#include <set>

namespace eo
{
class Socket;
}

namespace tse
{
class FareDisplayTrx;
class FlightCount;

class FlightCountMgr
{
  friend class FlightCountMgrTest;

public:
  virtual ~FlightCountMgr() = default;
  bool getFlightCount(FareDisplayTrx& trx, std::vector<FlightCount*>&) const;

private:
  static const std::string DSS_SERVICE_NAME;

  virtual bool initializeDSS(eo::Socket& socket) const;

  void getCarrierList(FareDisplayTrx& trx, std::set<CarrierCode>& carrierList) const;
  virtual bool writeSocket(eo::Socket& socket,
                           ac::SocketUtils::Message& req,
                           bool standardHeaderNotSupported) const;
  virtual bool readSocket(eo::Socket& socket,
                          ac::SocketUtils::Message& res,
                          bool standardHeaderNotSupported) const;
};
}
