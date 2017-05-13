//-------------------------------------------------------------------
//
//  File:        FlightCountMgr
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "DSS/FlightCountMgr.h"

#include "ClientSocket/EOSocket.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/MultiTransportMarkets.h"
#include "Common/TseCodeTypes.h"
#include "Common/TSELatencyData.h"
#include "Common/TseSrvStats.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FareDisplayPref.h"
#include "DSS/DSSRequest.h"
#include "DSS/DSSResponseContentHandler.h"
#include "DSS/FlightCount.h"

#include <vector>

namespace tse
{
static Logger
logger("atseintl.DSS.FlightCountMgr");
namespace
{
ConfigurableValue<std::string>
dssHost("ITIN_SVC", "DSSV2_HOST");
ConfigurableValue<uint16_t>
dssPort("ITIN_SVC", "DSSV2_PORT");
ConfigurableValue<uint32_t>
dssTimeOut("ITIN_SVC", "DSSV2_TIMEOUT");
// dssLinger is not <bool> in order to process '0' and '1' values from ACMS
ConfigurableValue<uint8_t>
dssLinger("ITIN_SVC", "DSSV2_LINGER", 0);
ConfigurableValue<uint32_t>
dssLingerTime("ITIN_SVC", "DSSV2_LINGER_TIME");
}

bool
FlightCountMgr::getFlightCount(FareDisplayTrx& trx, std::vector<FlightCount*>& flightCounts) const
{
  if (!trx.isScheduleCountRequested())
    return false;

  TSELatencyData latency(trx, MetricsUtil::FARE_DISPLAY_DSS);
  DSSRequest dssRequest(trx);
  std::string request;
  std::set<CarrierCode> carrierList;
  getCarrierList(trx, carrierList);
  dssRequest.build(trx, carrierList, request);

  if (request.empty())
  {
    return false;
  }

  if (FareDisplayUtil::isXMLDiagRequested(trx, "DSSREQ"))
    FareDisplayUtil::displayXMLDiag(trx, request, "DSS XML REQUEST");

  eo::Socket socket;
  if (!initializeDSS(socket))
  {
    TseSrvStats::recordDSSv2Call(false, 0, 0, 0, trx);
    LOG4CXX_FATAL(logger, "FAILED  TO CONNECT DSS SERVER FOR FARE QUOTE SHOPPER ENTRY");
    return false;
  }

  ac::SocketUtils::Message req, res;
  req.command = "RQST";
  req.xmlVersion = "0010";
  req.payload = request;
  tse::StopWatch sw;
  sw.start();

  bool standardHeaderNotSupported(
      true); // TODO: invoke the original read/write when DSS starts supporing standard header.
  if (!writeSocket(socket, req, standardHeaderNotSupported))
  {
    sw.stop();
    TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }
  res.clearAll();
  if (!readSocket(socket, res, standardHeaderNotSupported))
  {
    sw.stop();
    TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }

  sw.stop();
  TseSrvStats::recordDSSv2Call(
      true, req.payload.length(), res.payload.length(), sw.elapsedTime(), trx);

  if (const auto jsonMessage = trx.getXrayJsonMessage())
    jsonMessage->setLlsNetworkTime("DSS", sw.elapsedTime() * xray::MILLISECONDS_IN_SECOND);

  if (FareDisplayUtil::isXMLDiagRequested(trx, "DSSRES"))
    FareDisplayUtil::displayXMLDiag(trx, res.payload, "DSS XML RESPONSE");

  // Parse the response
  std::string repository;
  DSSResponseContentHandler docHandler(trx, flightCounts);
  docHandler.initialize();
  if (!docHandler.parse(res.payload.c_str()))
    return false;

  return true;
}

bool
FlightCountMgr::initializeDSS(eo::Socket& socket) const
{
  const std::string dssV2Host = dssHost.getValue();
  if (dssHost.isDefault())
    return false;

  const uint16_t dssV2Port = dssPort.getValue();
  if (dssPort.isDefault())
    return false;

  if (uint32_t dssV2TimeOut = dssTimeOut.getValue())
    socket.setTimeOut(dssV2TimeOut);

  if (dssLinger.getValue())
    socket.setLinger(true, dssLingerTime.getValue());

  if (!socket.connect(dssV2Host, dssV2Port))
  {
    LOG4CXX_DEBUG(logger,
                  "PROBLEM CONNECTING HOST: " << dssV2Host << " PORT: " << dssV2Port
                                              << " LEAVING FlightCountMgr::initializeDSS");
    return false;
  }
  return true;
}

void
FlightCountMgr::getCarrierList(FareDisplayTrx& trx, std::set<CarrierCode>& carrierList) const
{
  if (trx.isShopperRequest())
    carrierList = trx.preferredCarriers();
  else
  {
    std::map<MultiTransportMarkets::Market, std::set<CarrierCode>>* mktCxrMap = nullptr;
    carrierList =
        FareDisplayUtil::getUniqueSetOfCarriers(trx, false, mktCxrMap); // not including addon
    carrierList.insert(trx.requestedCarrier());
  }
}

bool
FlightCountMgr::writeSocket(eo::Socket& socket,
                            ac::SocketUtils::Message& req,
                            bool standardHeaderNotSupported) const
{
  return ac::SocketUtils::writeMessage(socket, req, standardHeaderNotSupported);
}
bool
FlightCountMgr::readSocket(eo::Socket& socket,
                           ac::SocketUtils::Message& res,
                           bool standardHeaderNotSupported) const
{
  return ac::SocketUtils::readMessage(socket, res, standardHeaderNotSupported);
}
}
