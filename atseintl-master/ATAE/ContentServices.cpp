//----------------------------------------------------------------------------
//
//  File:  ContentServices.cpp
//  Description: See ContentServices.h file
//  Created:  May 10, 2004
//  Authors:  Mike Carroll
//
//  Copyright Sabre 2003
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

#include "ATAE/ContentServices.h"

#include "AppConsole/SocketUtils.h"
#include "ATAE/AtaeRequest.h"
#include "ATAE/AtaeResponseHandler.h"
#include "ATAE/PricingDssFlightProcessor.h"
#include "ATAE/PricingDssResponseHandler.h"
#include "ClientSocket/EOSocket.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Config/DynamicConfigurableString.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseSrvStats.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/AvailData.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/CsoPricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FrequentFlyerAccount.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ReservationData.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/Traveler.h"
#include "DBAccess/Cabin.h"
#include "Diagnostic/DiagManager.h"
#include "Util/CompressUtil.h"

#include <boost/regex.hpp>
#include <boost/range/adaptors.hpp>

#include <algorithm>

namespace tse
{
FALLBACK_DECL(fallbackUpdateAvailabilityMap);
FALLBACK_DECL(fallbackDebugOverrideDssv2AndAsv2Responses);
FALLBACK_DECL(fallbackDebugOverrideBrandingServiceResponses);
namespace
{
DynamicConfigurableString
dynamicConfigOverrideDSSv2Response("DEBUG_INFO", "OVERRIDE_DSSV2_RESPONSE", "N");

DynamicConfigurableString
dynamicConfigOverrideASv2Response("DEBUG_INFO", "OVERRIDE_ASV2_RESPONSE", "N");

ConfigurableValue<std::string>
dssV2Host("ITIN_SVC", "DSSV2_HOST");
ConfigurableValue<uint16_t>
dssV2Port("ITIN_SVC", "DSSV2_PORT", 0);
ConfigurableValue<uint32_t>
dssV2TimeOut("ITIN_SVC", "DSSV2_TIMEOUT", 0);
// using uint8_t instead of bool to handle 0/1 values
ConfigurableValue<uint8_t>
dssV2Linger("ITIN_SVC", "DSSV2_LINGER", 0);
ConfigurableValue<uint32_t>
dssV2LingerTime("ITIN_SVC", "DSSV2_LINGER_TIME", 0);
ConfigurableValue<std::string>
asV2Host("ITIN_SVC", "ASV2_HOST");
ConfigurableValue<uint16_t>
asV2Port("ITIN_SVC", "ASV2_PORT", 0);
ConfigurableValue<std::string>
asV2HostCacheProxyMIP("ITIN_SVC", "ASV2_HOST_CACHE_PROXY_MIP");
ConfigurableValue<uint16_t>
asV2PortCacheProxyMIP("ITIN_SVC", "ASV2_PORT_CACHE_PROXY_MIP", 0);
ConfigurableValue<uint32_t>
asV2TimeOut("ITIN_SVC", "ASV2_TIMEOUT", 0);
// using uint8_t instead of bool to handle 0/1 values
ConfigurableValue<uint8_t>
asV2Linger("ITIN_SVC", "ASV2_LINGER", 0);
ConfigurableValue<uint32_t>
asV2LingerTime("ITIN_SVC", "ASV2_LINGER_TIME", 0);
ConfigurableValue<bool>
useAVS("ITIN_SVC", "USE_AVS", false);
}

Logger
ContentServices::_logger("atseintl.ATAE.ContentServices");

namespace
{
std::string _dssV2Host;
uint16_t _dssV2Port;
long _dssV2TimeOut;
bool _dssV2Linger;
long _dssV2LingerTime;

std::string _asV2Host;
uint16_t _asV2Port;
std::string _asV2HostCacheProxyMIP;
uint16_t _asV2PortCacheProxyMIP;
long _asV2TimeOut;
bool _asV2Linger;
long _asV2LingerTime;
bool _useAVS; // for soak test, avoid DCA

const size_t TCP_HEADER_LEN = 12;

struct CopyAvailabilityMapItem : public std::unary_function<std::vector<ClassOfService*>*, bool>
{
  CopyAvailabilityMapItem(std::vector<ClassOfServiceList>* item) : _availabilityMapItem(item) {}

  bool operator()(std::vector<ClassOfService*>* in)
  {
    _availabilityMapItem->push_back(*in);
    return true;
  }

private:
  std::vector<ClassOfServiceList>* _availabilityMapItem;
};
}

//----------------------------------------------------------------------------
// ContentServices::initialize
// This function should be called only once when server start up.
//----------------------------------------------------------------------------
bool
ContentServices::initialize()
{
  _dssV2Host = dssV2Host.getValue();
  _dssV2Port = dssV2Port.getValue();
  _dssV2TimeOut = dssV2TimeOut.getValue();
  _dssV2Linger = dssV2Linger.getValue();
  _dssV2LingerTime = dssV2LingerTime.getValue();
  _asV2Host = asV2Host.getValue();
  _asV2Port = asV2Port.getValue();
  _asV2HostCacheProxyMIP = asV2HostCacheProxyMIP.getValue();
  _asV2PortCacheProxyMIP = asV2PortCacheProxyMIP.getValue();
  _asV2TimeOut = asV2TimeOut.getValue();
  _asV2Linger = asV2Linger.getValue();
  _asV2LingerTime = asV2LingerTime.getValue();
  _useAVS = useAVS.getValue();
  return true;
}

void
ContentServices::getSchedAndAvail(PricingTrx& trx)
{
  if (trx.itin().empty())
  {
    LOG4CXX_FATAL(_logger, "ITIN SIZE < 1 ContentServices::getSchedAndAvail()");
    return;
  }
  Itin& itin = *(trx.itin().front());

  if (itin.travelSeg().empty())
  {
    LOG4CXX_FATAL(_logger, "TVLSEG SIZE < 1 ContentServices::getSchedAndAvail()");
    return;
  }

  bool isWQ = dynamic_cast<NoPNRPricingTrx*>(&trx) != nullptr;
  bool isCSO = trx.excTrxType() == PricingTrx::CSO_EXC_TRX;
  bool isRefund = trx.excTrxType() == PricingTrx::AF_EXC_TRX;

  if (!isWQ && !isCSO)
  {
    if (trx.excTrxType() != PricingTrx::NOT_EXC_TRX &&
        !trx.itin().front()->travelSeg().front()->unflown())
    // empty vector check already done in PricingModelMap
    {
      if (!callDssV2(trx, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN))
      {
        // could fail because unable to get flown schedule,
        // need to recall to get unflown schedule only
        if (!callDssV2(trx, MethodGetFlownSchedule::SKIP_FLOWN))
          throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION,
                                       "SCHEDULE SERVICE RETURNED NO SCHEDULES");
      }
    }
    else
    {
      if (trx.billing()->requestPath() == SWS_PO_ATSE_PATH &&
          trx.excTrxType() == PricingTrx::NOT_EXC_TRX &&
          !trx.itin().front()->travelSeg().front()->unflown())
      {
        if (!callDssV2(trx, MethodGetFlownSchedule::SKIP_FLOWN))
          throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION,
                                       "SCHEDULE SERVICE RETURNED NO SCHEDULES");
      }
      else if (!callDssV2(trx, MethodGetFlownSchedule::NORMAL_GET_FLOWN))
        throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION,
                                     "SCHEDULE SERVICE RETURNED NO SCHEDULES");
    }
  }

  RexBaseTrx* rexBaseTrx = dynamic_cast<RexBaseTrx*>(&trx);
  if (rexBaseTrx != nullptr && rexBaseTrx->isAnalyzingExcItin())
    return;

  if (trx.getOptions()->fareX())
    return;

  if (trx.getRequest()->isLowFareNoAvailability()) // WPNCS
  {
    restFareMarkets(itin, trx);
    return;
  }

  if (!isWQ && !isCSO && !isRefund)
  {
    if (!(trx.billing() &&
          trx.billing()->partitionID() == "WN")) // SWA project Change we skip calls for WN ariline
    {
      if (!callAs2(trx))
        throw ErrorResponseException(ErrorResponseException::ATAE_RETURNED_NO_BOOKING_CODES);
    }
  }

  if (isCSO)
    static_cast<CsoPricingTrx&>(trx).cloneClassOfService();
  else
  {
    // for the OPEN segments make all classes available
    std::vector<FareMarket*>::const_iterator fmIt = itin.fareMarket().begin();
    const std::vector<FareMarket*>::const_iterator fmItEnd = itin.fareMarket().end();
    fmIt = itin.fareMarket().begin();
    for (; fmIt != fmItEnd; ++fmIt)
    {
      FareMarket& fmkt = *(*fmIt);
      if (fmkt.travelSeg().size() != 1)
        continue;

      // check if already processed
      if (!fmkt.classOfServiceVec().empty())
        continue;

      if (fmkt.travelSeg()[0]->segmentType() == Open)
      {
        fillDummyCOS(fmkt, trx.dataHandle(), trx);
        continue;
      }

      getCosFlown(trx, fmkt);
    }

    restFareMarkets(itin, trx);
    if (trx.diagnostic().diagnosticType() != DiagnosticNone)
    {
      soloTest(trx, itin);
      journeyTest(trx, itin);
    }

    if (trx.getRequest()->upSellEntry())
    {
      adjustAvail(trx);
      upSellAvailDiag195(trx, itin.fareMarket());
    }
  }
}

void
ContentServices::getSchedule(PricingTrx& trx)
{
  if (trx.itin().empty())
  {
    LOG4CXX_FATAL(_logger, "ITIN SIZE < 1 ContentServices::getSchedule()");
    return;
  }
  Itin& itin = *(trx.itin().front());

  if (itin.travelSeg().empty())
  {
    LOG4CXX_FATAL(_logger, "TVLSEG SIZE < 1 ContentServices::getSchedule()");
    return;
  }

  bool isWQ = dynamic_cast<NoPNRPricingTrx*>(&trx) != nullptr;
  bool isCSO = trx.excTrxType() == PricingTrx::CSO_EXC_TRX;

  if (!isWQ && !isCSO)
  {
    if (trx.excTrxType() != PricingTrx::NOT_EXC_TRX &&
        !trx.itin().front()->travelSeg().front()->unflown())
    // empty vector check already done in PricingModelMap
    {
      if (!callDssV2(trx, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN))
      {
        // could fail because unable to get flown schedule,
        // need to recall to get unflown schedule only
        if (!callDssV2(trx, MethodGetFlownSchedule::SKIP_FLOWN))
          throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION,
                                       "SCHEDULE SERVICE RETURNED NO SCHEDULES");
      }
    }
    else
    {
      if (trx.billing()->requestPath() == SWS_PO_ATSE_PATH &&
          trx.excTrxType() == PricingTrx::NOT_EXC_TRX &&
          !trx.itin().front()->travelSeg().front()->unflown())
      {
        if (!callDssV2(trx, MethodGetFlownSchedule::SKIP_FLOWN))
          throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION,
                                       "SCHEDULE SERVICE RETURNED NO SCHEDULES");
      }
      else if (!callDssV2(trx, MethodGetFlownSchedule::NORMAL_GET_FLOWN))
        throw ErrorResponseException(ErrorResponseException::SYSTEM_EXCEPTION,
                                     "SCHEDULE SERVICE RETURNED NO SCHEDULES");
    }
  }
}

void
ContentServices::getAvailability(PricingTrx& trx)
{
  if (trx.itin().empty())
  {
    LOG4CXX_FATAL(_logger, "ITIN SIZE < 1 ContentServices::getAvailability()");
    return;
  }
  Itin& itin = *(trx.itin().front());

  if (itin.travelSeg().empty())
  {
    LOG4CXX_FATAL(_logger, "TVLSEG SIZE < 1 ContentServices::getAvailability()");
    return;
  }

  RexBaseTrx* rexBaseTrx = dynamic_cast<RexBaseTrx*>(&trx);
  if (rexBaseTrx != nullptr && rexBaseTrx->isAnalyzingExcItin())
    return;

  if (trx.getOptions()->fareX())
    return;

  if (trx.getRequest()->isLowFareNoAvailability()) // WPNCS
  {
    restFareMarkets(itin, trx);
    return;
  }

  bool isWQ = dynamic_cast<NoPNRPricingTrx*>(&trx) != nullptr;
  bool isCSO = trx.excTrxType() == PricingTrx::CSO_EXC_TRX;
  bool isRefund = trx.excTrxType() == PricingTrx::AF_EXC_TRX;

  if (!isWQ && !isCSO && !isRefund)
  {
    if (!(trx.billing() &&
          trx.billing()->partitionID() == "WN")) // SWA project Change we skip calls for WN ariline
    {
      if (!callAs2(trx))
        throw ErrorResponseException(ErrorResponseException::ATAE_RETURNED_NO_BOOKING_CODES);
    }
  }

  if (isCSO)
    static_cast<CsoPricingTrx&>(trx).cloneClassOfService();
  else
  {
    // for the OPEN segments make all classes available
    std::vector<FareMarket*>::const_iterator fmIt = itin.fareMarket().begin();
    const std::vector<FareMarket*>::const_iterator fmItEnd = itin.fareMarket().end();
    fmIt = itin.fareMarket().begin();
    for (; fmIt != fmItEnd; ++fmIt)
    {
      FareMarket& fmkt = *(*fmIt);
      if (fmkt.travelSeg().size() != 1)
        continue;

      // check if already processed
      if (!fmkt.classOfServiceVec().empty())
        continue;

      if (fmkt.travelSeg()[0]->segmentType() == Open)
      {
        fillDummyCOS(fmkt, trx.dataHandle(), trx);
        continue;
      }

      getCosFlown(trx, fmkt);
    }

    restFareMarkets(itin, trx);
    if (trx.diagnostic().diagnosticType() != DiagnosticNone)
    {
      soloTest(trx, itin);
      journeyTest(trx, itin);
    }

    if (trx.getRequest()->upSellEntry())
    {
      adjustAvail(trx);
      upSellAvailDiag195(trx, itin.fareMarket());
    }
  }
}

void
ContentServices::getAvailShopping(PricingTrx& trx)
{
  if (trx.itin().empty())
  {
    LOG4CXX_FATAL(_logger, "ITIN SIZE < 1 ContentServices::getAvailShopping()");
    return;
  }

  AtaeResponseHandler ataeResponseHandler(trx);
  const bool as2RetVal = callAs2Shopping(trx, ataeResponseHandler);
  if (!as2RetVal)
  {
    throw ErrorResponseException(ErrorResponseException::ATAE_RETURNED_NO_BOOKING_CODES);
  }

  std::vector<Itin*>::iterator iI = trx.itin().begin();
  std::vector<Itin*>::iterator iE = trx.itin().end();
  for (; iI != iE; ++iI)
  {
    Itin& itin = *(*iI);
    if ((trx.getOptions()->callToAvailability() == 'T') || (itin.dcaSecondCall()))
    {
      restFareMarkets(itin, trx);
    }
  }
  buildAvailMapMerge(trx, ataeResponseHandler.fareMarketsSentToAtae());

  if (!fallback::fallbackUpdateAvailabilityMap(&trx))
    updateAvailabilityMap(trx, ataeResponseHandler.fareMarketsSentToAtae());
}

void
ContentServices::getSched(AncillaryPricingTrx& trx)
{
  if (trx.itin().front()->travelSeg().front()->unflown())
  {
    callDssV2(trx, MethodGetFlownSchedule::NORMAL_GET_FLOWN);
  }
  else
  {
    if (!callDssV2(trx, MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN))
    {
      callDssV2(trx, MethodGetFlownSchedule::SKIP_FLOWN);
    }
  }
}

void
ContentServices::getSched(BaggageTrx& trx)
{
  callDssV2(trx, MethodGetFlownSchedule::NORMAL_GET_FLOWN);
}

void
ContentServices::fillDummyCOS(FareMarket& fm, DataHandle& dataHandle, PricingTrx& trx)
{
  std::vector<TravelSeg*>::iterator tvlSegIter = fm.travelSeg().begin();
  const std::vector<TravelSeg*>::iterator tvlSegEnd = fm.travelSeg().end();
  AirSeg* pAirSeg = nullptr;
  ClassOfService* cs = nullptr;
  std::vector<ClassOfService*>* cosVec = nullptr;
  const uint16_t numTvlSegs = fm.travelSeg().size();

  for (; tvlSegIter != tvlSegEnd; ++tvlSegIter)
  {
    pAirSeg = dynamic_cast<AirSeg*>(*tvlSegIter);

    dataHandle.get(cosVec);
    fm.classOfServiceVec().push_back(cosVec);

    if (pAirSeg == nullptr)
      continue;

    if (pAirSeg->segmentType() != Open)
      continue;

    // Only for a OPEN segments, make all the booking codes available
    // Refer PL 7742
    if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
    {
      BookingCode bc;
      std::vector<BookingCode> bookingCodes;
      char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      for (uint16_t i = 0; i < 26; ++i)
      {
        bc = alpha[i];
        bookingCodes.push_back(bc);
      }
      if (numTvlSegs == 1)
      {
        pAirSeg->classOfService().clear();
      }
      RBDByCabinUtil rbdCabin(trx, CONTENT_SVC);
      rbdCabin.getCabinsByRbd(*pAirSeg, bookingCodes, cosVec);
      for (const auto elem : *cosVec)
      {
        elem->numSeats() = 9;
      }
      if (numTvlSegs == 1)
      {
        pAirSeg->classOfService() = *cosVec;
      }
    }
    else
    {
      // Only for a OPEN segments, make all the booking codes available
      // Refer PL 7742
      for (uint16_t i = 0; i < 26; ++i)
      {
        cs = dummyCOS(i, pAirSeg, dataHandle);
        if (cs != nullptr && cosVec != nullptr)
          cosVec->push_back(cs);

        // get another pointer to cos to put in for local market
        if (numTvlSegs == 1)
        {
          cs = dummyCOS(i, pAirSeg, dataHandle);
          if (cs != nullptr)
            pAirSeg->classOfService().push_back(cs);
        }
      }
    }
  } // end for (; tvlSegIter != tvlSegEnd; tvlSegIter++)
}

void
ContentServices::getCosFlown(PricingTrx& trx, FareMarket& fm)
{
  if (!(trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
    return;

  AirSeg* pAirSeg = dynamic_cast<AirSeg*>(fm.travelSeg()[0]);
  if (pAirSeg == nullptr)
    return;

  if (pAirSeg->unflown())
    return;

  std::vector<ClassOfService*>* cosVec = nullptr;
  trx.dataHandle().get(cosVec);
  fm.classOfServiceVec().push_back(cosVec);

  ClassOfService* cs = getACos(trx, pAirSeg);
  if (cs == nullptr)
    return;
  cosVec->push_back(cs);

  cs = getACos(trx, pAirSeg);
  if (cs == nullptr)
    return;
  pAirSeg->classOfService().push_back(cs);
}

ClassOfService*
ContentServices::getACos(PricingTrx& trx, AirSeg* pAirSeg)
{
  ClassOfService* cs = nullptr;
  trx.dataHandle().get(cs);
  uint16_t numSeats = 0;
  numSeats = numSeatsNeeded(trx);
  cs->bookingCode() = pAirSeg->getBookingCode();
  cs->cabin() = pAirSeg->bookedCabin();
  cs->numSeats() = numSeats;
  return cs;
}

ClassOfService*
ContentServices::dummyCOS(uint16_t i, AirSeg* airSeg, DataHandle& dataHandle)
{
  ClassOfService* cs = nullptr;
  dataHandle.get(cs);

  if (cs == nullptr)
    return nullptr;

  cs->numSeats() = 9;
  switch (i)
  {
  case 0:
    cs->bookingCode() = "A";
    break;
  case 1:
    cs->bookingCode() = "B";
    break;
  case 2:
    cs->bookingCode() = "C";
    break;
  case 3:
    cs->bookingCode() = "D";
    break;
  case 4:
    cs->bookingCode() = "E";
    break;
  case 5:
    cs->bookingCode() = "F";
    break;
  case 6:
    cs->bookingCode() = "G";
    break;
  case 7:
    cs->bookingCode() = "H";
    break;
  case 8:
    cs->bookingCode() = "I";
    break;
  case 9:
    cs->bookingCode() = "J";
    break;
  case 10:
    cs->bookingCode() = "K";
    break;
  case 11:
    cs->bookingCode() = "L";
    break;
  case 12:
    cs->bookingCode() = "M";
    break;
  case 13:
    cs->bookingCode() = "N";
    break;
  case 14:
    cs->bookingCode() = "O";
    break;
  case 15:
    cs->bookingCode() = "P";
    break;
  case 16:
    cs->bookingCode() = "Q";
    break;
  case 17:
    cs->bookingCode() = "R";
    break;
  case 18:
    cs->bookingCode() = "S";
    break;
  case 19:
    cs->bookingCode() = "T";
    break;
  case 20:
    cs->bookingCode() = "U";
    break;
  case 21:
    cs->bookingCode() = "V";
    break;
  case 22:
    cs->bookingCode() = "W";
    break;
  case 23:
    cs->bookingCode() = "X";
    break;
  case 24:
    cs->bookingCode() = "Y";
    break;
  case 25:
    cs->bookingCode() = "Z";
    break;
  default:
    return nullptr;
  }

  const Cabin* aCabin = nullptr;
  aCabin = getCabin(dataHandle, airSeg->carrier(), cs->bookingCode(), airSeg->departureDT());
  if (aCabin != nullptr)
  {
    cs->cabin() = aCabin->cabin();
  }
  else
  {
    cs->cabin().setInvalidClass();
    LOG4CXX_ERROR(_logger,
                  "ContentServices::dummyCOS() CABIN TABLE ERROR OPEN FLIGHT:"
                      << airSeg->carrier() << airSeg->flightNumber() << " " << cs->bookingCode()
                      << airSeg->departureDT().dateToString(DDMMMYYYY, ""));
  }
  return cs;
}

bool
ContentServices::stopOversArunkIncluded(FareMarket& fm)
{
  // initAvailBreak(fm);
  if (fm.travelSeg().size() <= 1)
    return false;
  std::vector<TravelSeg*>::const_iterator tvlSegIter = fm.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEnd = fm.travelSeg().end();
  const std::vector<TravelSeg*>::const_iterator tvlSegLast = tvlSegEnd - 1;
  const AirSeg* lastAirSeg = dynamic_cast<const AirSeg*>(*tvlSegLast);
  const AirSeg* airSeg = nullptr;
  const AirSeg* firstAirSeg = nullptr;
  uint16_t nonArunks = 0;
  for (; tvlSegIter != tvlSegEnd; ++tvlSegIter)
  {
    airSeg = dynamic_cast<const AirSeg*>(*tvlSegIter);
    if (airSeg == nullptr)
    {
      if ((*tvlSegIter) != nullptr)
      {
        // only multiAirport ARUNKS are considered connections
        if (!(*tvlSegIter)->arunkMultiAirportForAvailability())
          return true;
        if (tvlSegIter == fm.travelSeg().begin() || tvlSegIter == tvlSegLast)
          return true;
      }
      continue;
    }
    else
      ++nonArunks;
    if (firstAirSeg == nullptr)
      firstAirSeg = airSeg;
    if (airSeg != lastAirSeg && airSeg->stopOver())
      return true;
    if (airSeg->segmentType() == Open)
      return true;
    if (airSeg->carrier() != firstAirSeg->carrier())
      return true;
  }
  // maximum 3 segment connection only
  if (nonArunks > 3)
    return true;

  return false;
}

namespace
{
struct SameFareMarketWithCOS : public std::unary_function<const FareMarket*, bool>
{
  SameFareMarketWithCOS(const FareMarket* fm) : _fm(fm) {}

  bool operator()(const FareMarket* fm) const
  {
    if (fm->travelSeg().size() != _fm->travelSeg().size())
      return false;

    if (fm->classOfServiceVec().empty()) // looking for FM with COS
      return false;

    for (size_t index = 0; index < fm->travelSeg().size(); ++index)
    {
      const AirSeg* airSeg1 = dynamic_cast<const AirSeg*>(_fm->travelSeg()[index]);
      const AirSeg* airSeg2 = dynamic_cast<const AirSeg*>(fm->travelSeg()[index]);

      if (airSeg1 == nullptr || airSeg2 == nullptr)
      {
        if (airSeg1 != airSeg2)
          return false;
        else
          continue;
      }

      if (airSeg1->segmentType() != airSeg2->segmentType())
        return false;
      else if (airSeg1->segmentType() == Open || airSeg1->segmentType() == Arunk)
        continue;

      if (!sameFlight(_fm->travelSeg()[index], fm->travelSeg()[index]))
        return false;
    }

    return true;
  }

  bool sameFlight(const TravelSeg* tvlSeg1, const TravelSeg* tvlSeg2) const
  {
    const AirSeg* airSeg1 = dynamic_cast<const AirSeg*>(tvlSeg1);
    const AirSeg* airSeg2 = dynamic_cast<const AirSeg*>(tvlSeg2);

    if (airSeg1->flightNumber() != airSeg2->flightNumber())
      return false;

    if (airSeg1->carrier() != airSeg2->carrier())
      return false;

    if (airSeg1->origAirport() != airSeg2->origAirport())
      return false;

    if (airSeg1->destAirport() != airSeg2->destAirport())
      return false;

    if (airSeg1->operatingCarrierCode() != airSeg2->operatingCarrierCode())
      return false;

    if (airSeg1->operatingFlightNumber() != airSeg2->operatingFlightNumber())
      return false;

    if (airSeg1->departureDT() != airSeg2->departureDT())
      return false;

    if (airSeg1->arrivalDT() != airSeg2->arrivalDT())
      return false;

    return true;
  }

  const FareMarket* _fm;
};
}

void
ContentServices::restFareMarkets(Itin& itin, PricingTrx& trx)
{
  std::vector<FareMarket*>::iterator fmIt = itin.fareMarket().begin();
  const std::vector<FareMarket*>::iterator fmItEnd = itin.fareMarket().end();

  // first make sure that all the markets
  // with less than 3 flights are built using local availability
  for (; fmIt != fmItEnd; ++fmIt)
  {
    FareMarket& fmkt = *(*fmIt);
    // check if already processed
    if (!fmkt.classOfServiceVec().empty())
      continue;

    if (trx.getTrxType() == PricingTrx::MIP_TRX)
    {
      checkDuplicateFareMarketsShop(itin, *fmIt);
    }
    else
    {
      checkDuplicateFareMarkets(itin, *fmIt, trx);
    }

    if (!fmkt.classOfServiceVec().empty())
      continue;

    if (fmkt.travelSeg().size() > 2 || fmkt.travelSeg().size() < 1)
      continue;

    FareMarketUtil::buildUsingLocal(fmkt, trx, fmkt.travelSeg().size(), fmkt.travelSeg()[0], true);
  }

  // again start the loop to process markets with 3
  // or more flights in them
  fmIt = itin.fareMarket().begin();
  for (; fmIt != fmItEnd; ++fmIt)
  {
    FareMarket& fmkt = *(*fmIt);
    // check if already processed
    if (!fmkt.classOfServiceVec().empty() || fmkt.travelSeg().size() < 3)
      continue;
    const size_t totalFlt = fmkt.travelSeg().size();
    size_t fltsDone = 0;
    size_t startIndex = 0;
    while (startIndex < totalFlt)
    {
      const size_t fltsCovered = FareMarketUtil::numFlts(itin, fmkt, startIndex);
      std::vector<FareMarket*> processedFM;
      FareMarketUtil::group3(itin, fmkt, startIndex, fltsCovered, trx, processedFM, true);

      fltsDone += fltsCovered;
      startIndex = fltsDone;
    }
  }
}

void
ContentServices::soloTest(PricingTrx& trx, Itin& itin)
{
  std::string solo("");
  typedef std::map<std::string, std::string>::const_iterator DiagParamMapVecIC;
  const DiagParamMapVecIC endI = trx.diagnostic().diagParamMap().end();
  const DiagParamMapVecIC beginI = trx.diagnostic().diagParamMap().find(Diagnostic::WPNC_SOLO_TEST);
  if (beginI != endI)
  {
    size_t len = ((*beginI).second).size();
    if (len != 0)
    {
      solo = ((*beginI).second);
    }
  }
  if (solo != "LOTEST")
    return;

  // if /SOLOTEST option was entered, lets zeroise the thru availability
  std::vector<FareMarket*>::iterator fmIt = itin.fareMarket().begin();
  const std::vector<FareMarket*>::iterator fmItEnd = itin.fareMarket().end();

  for (; fmIt != fmItEnd; ++fmIt)
  {
    FareMarket& fmkt = *(*fmIt);
    if (fmkt.classOfServiceVec().empty())
      continue;
    // only zeroise availability for thru markets
    if (fmkt.travelSeg().size() == 1)
      continue;
    uint16_t sizeCosVecVec = fmkt.classOfServiceVec().size();
    for (uint16_t i = 0; i < sizeCosVecVec; ++i)
    {
      std::vector<ClassOfService*>& cosVec = *fmkt.classOfServiceVec()[i];
      for (uint16_t j = 0; j < cosVec.size(); ++j)
      {
        ClassOfService& cs = *cosVec[j];
        if (cs.bookingCode()[0] == 'Y' || j == 0)
          continue;
        cs.numSeats() = 0;
      }
    }
  }
}

void
ContentServices::journeyTest(PricingTrx& trx, Itin& itin)
{
  std::string jny("");
  typedef std::map<std::string, std::string>::const_iterator DiagParamMapVecIC;
  const DiagParamMapVecIC endI = trx.diagnostic().diagParamMap().end();
  const DiagParamMapVecIC beginI = trx.diagnostic().diagParamMap().find(Diagnostic::WPNC_SOLO_TEST);
  if (beginI != endI)
  {
    size_t len = ((*beginI).second).size();
    if (len != 0)
    {
      jny = ((*beginI).second);
    }
  }
  else
  {
    return;
  }
  if (!(jny == "FLOW" || jny == "LOCAL"))
    return;

  // if /FLOW option was entered, lets zeroise the local avail

  std::vector<FareMarket*>::iterator fmIt = itin.fareMarket().begin();
  const std::vector<FareMarket*>::iterator fmItEnd = itin.fareMarket().end();

  uint16_t sizeCosVecVec = 0;
  uint16_t sizeVec = 0;
  uint16_t i = 0;
  uint16_t j = 0;

  std::vector<ClassOfService*>* cosVec = nullptr;
  if (jny == "FLOW")
  {
    for (; fmIt != fmItEnd; ++fmIt)
    {
      FareMarket& fmkt = *(*fmIt);
      if (!(fmkt.flowMarket()))
        continue;
      if (fmkt.classOfServiceVec().empty())
        continue;
      // only zeroise availability for local markets
      sizeCosVecVec = fmkt.classOfServiceVec().size();
      for (i = 0; i < sizeCosVecVec; i++)
      {
        cosVec = fmkt.classOfServiceVec()[i];
        sizeVec = cosVec->size();
        for (j = 0; j < sizeVec; j++)
        {
          ClassOfService& cs = *((*cosVec)[j]);
          if (cs.bookingCode()[0] == 'Y' || j == 0)
            continue;
          cs.numSeats() = 0;
        }
      }
    }
  }
  else // jny == "LOCAL"
  {
    ClassOfService* csp = nullptr;
    for (; fmIt != fmItEnd; ++fmIt)
    {
      FareMarket& fmkt = *(*fmIt);

      // only zeroise availability for local markets
      if (fmkt.travelSeg().size() != 1)
        continue;

      if (fmkt.travelSeg()[0] == nullptr || dynamic_cast<AirSeg*>(fmkt.travelSeg()[0]) == nullptr)
        continue;

      const AirSeg& airSeg = *(dynamic_cast<AirSeg*>(fmkt.travelSeg()[0]));

      if (!(airSeg.flowJourneyCarrier() || airSeg.localJourneyCarrier()))
        continue;

      sizeCosVecVec = fmkt.classOfServiceVec().size();
      for (i = 0; i < sizeCosVecVec; i++)
      {
        cosVec = fmkt.classOfServiceVec()[i];
        sizeVec = cosVec->size();
        for (j = 0; j < sizeVec; j++)
        {
          ClassOfService& cs = *((*cosVec)[j]);
          if (cs.bookingCode()[0] == 'Y' || j == 0)
            continue;
          cs.numSeats() = 0;
        }
      }
      sizeVec = fmkt.travelSeg()[0]->classOfService().size();
      for (j = 0; j < sizeVec; j++)
      {
        csp = fmkt.travelSeg()[0]->classOfService()[j];
        if (csp == nullptr)
          continue;
        if (csp->bookingCode()[0] == 'Y' || j == 0)
          continue;
        csp->numSeats() = 0;
      }
    }
  }
}

void
ContentServices::buildAvailMapMerge(PricingTrx& trx,
                                    std::vector<FareMarket*>& fareMarketsSentToAtae)
{
  std::vector<FareMarket*>::iterator iFm = fareMarketsSentToAtae.begin();
  std::vector<FareMarket*>::iterator eFm = fareMarketsSentToAtae.end();

  std::vector<TravelSeg*>::iterator iTvl;
  std::vector<TravelSeg*>::iterator eTvl;

  uint16_t iCosVec = 0;
  uint16_t tvlSegSize = 0;
  bool tvlSegFound = false;

  uint16_t iCos = 0;
  uint16_t cosVecSize = 0;
  bool cosFound = false;
  uint16_t iList = 0;
  uint16_t listSize = 0;
  PricingTrx::ThruFareAvailabilityMap& thruAvailMap = trx.maxThruFareAvailabilityMap();
  PricingTrx::ThruFareAvailabilityMap::iterator iM = thruAvailMap.begin();
  PricingTrx::ThruFareAvailabilityMap::iterator iE = thruAvailMap.end();
  for (; iM != iE; iM++)
  {
    ClassOfServiceList cosList;
    bool travelSegWasRefreshed = false;

    iFm = fareMarketsSentToAtae.begin();
    for (; iFm != eFm; iFm++)
    {
      FareMarket& fm = *(*iFm);
      tvlSegSize = fm.travelSeg().size();

      if (tvlSegSize < 2)
        continue;
      if (fm.classOfServiceVec().size() != tvlSegSize)
        continue;
      if (stopOversArunkIncluded(fm))
        continue;

      // find the travelSeg in this thru FareMarket
      iTvl = fm.travelSeg().begin();
      eTvl = fm.travelSeg().end();

      tvlSegFound = false;
      for (iCosVec = 0; (iTvl != eTvl && iCosVec < tvlSegSize); iTvl++, iCosVec++)
      {
        if (*iTvl == iM->first)
        {
          tvlSegFound = true;
          break;
        }
      }
      if (!tvlSegFound)
        continue;

      travelSegWasRefreshed = true;

      // get thru class of services and build availMapMerge
      std::vector<ClassOfService*>& cosThru = *(fm.classOfServiceVec()[iCosVec]);
      cosVecSize = cosThru.size();

      for (iCos = 0; iCos < cosVecSize; iCos++)
      {
        cosFound = false;
        listSize = cosList.size();
        for (iList = 0; iList < listSize; iList++)
        {
          if (cosList[iList]->bookingCode() == cosThru[iCos]->bookingCode())
          {
            cosFound = true;
            break;
          }
        }
        if (cosFound)
        {
          if (cosThru[iCos]->numSeats() > cosList[iList]->numSeats())
            cosList[iList] = cosThru[iCos];
        }
        else
        {
          cosList.push_back(cosThru[iCos]);
        }
      }
    }
    if (travelSegWasRefreshed)
      iM->second.swap(cosList);
  }
}

bool
ContentServices::shouldPopulateAllItineraries(const PricingTrx& trx) const
{
  const bool isAncillaryPricingTrx = dynamic_cast<const AncillaryPricingTrx*>(&trx) != nullptr;
  if (isAncillaryPricingTrx)
  {
    const AncRequest* ancRq = static_cast<const AncRequest*>(trx.getRequest());
    if ((ancRq->majorSchemaVersion() >= 2) &&
        (trx.billing()->requestPath() == ACS_PO_ATSE_PATH || ancRq->isWPBGRequest() ||
         trx.activationFlags().isAB240() || trx.billing()->actionCode().substr(0, 5) == "MISC6"))
    {
      return true;
    }
  }
  return false;
}

void
ContentServices::populateFlightMap(PricingTrx& trx,
                                   const MethodGetFlownSchedule getFlownSchedule,
                                   PricingDssFlightMap& flightMap) const
{
  CarrierSwapUtil swapUtil(trx);
  FlightMapBuilder builder(swapUtil, getFlownSchedule);

  if (shouldPopulateAllItineraries(trx))
  {
    for (const Itin* itin : trx.itin())
      builder.populateFlightMap(itin->travelSeg().begin(), itin->travelSeg().end(), flightMap);
  }
  else
  {
    if (!trx.itin().empty())
      builder.populateFlightMap(
          trx.itin()[0]->travelSeg().begin(), trx.itin()[0]->travelSeg().end(), flightMap);
  }
}

bool
ContentServices::isAnySegmentInRequest(const PricingTrx& trx) const
{
  if (shouldPopulateAllItineraries(trx))
    return std::any_of(trx.itin().cbegin(),
                       trx.itin().cend(),
                       [](const Itin* itn)
                       { return !itn->travelSeg().empty(); });
  else
    return !trx.itin().at(0)->travelSeg().empty();
}

void
ContentServices::processDssFlights(PricingTrx& trx,
                                   std::vector<PricingDssFlight>::const_iterator begin,
                                   std::vector<PricingDssFlight>::const_iterator end,
                                   PricingDssFlightMap& flightMap) const
{
  PricingDssFlightProcessor dssFlightProcessor(trx);
  if(begin == end)
    dssFlightProcessor.findCabin(flightMap);

  for (; begin != end; ++begin)
  {
    const PricingDssFlight& dssFlight = *begin;
    PricingDssFlightMap::iterator it = dssFlightProcessor.findAirSegs(dssFlight, flightMap);
    if (it != flightMap.end())
    {
      std::vector<AirSeg*>& airSegs = it->second;
      dssFlightProcessor.populateAirSegs(dssFlight, airSegs.begin(), airSegs.end());
      flightMap.erase(it);
    }
  }
}

bool
ContentServices::callDssV2_old(PricingTrx& trx, const MethodGetFlownSchedule getFlownSchedule)
{
  LOG4CXX_INFO(_logger, "ENTERING ContentServices::callDssV2_old");
  dssReqDiag195(trx, getFlownSchedule);

  PricingDssFlightMap flightMap;

  PricingDssRequest pricingDssRequest(trx);

  std::string request;
  uint16_t numFlightsRequested = 0;

  populateFlightMap(trx, getFlownSchedule, flightMap);
  pricingDssRequest.build(flightMap | boost::adaptors::map_keys, request);
  numFlightsRequested = flightMap.size();

  std::string xmlDiagRequested;
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "DSSREQ")
      xmlDiag(trx, request);
  }

  LOG4CXX_DEBUG(_logger, "DSS REQUEST:" << request);

  if (trx.itin().empty() || !isAnySegmentInRequest(trx))
  {
    LOG4CXX_FATAL(_logger, "REQUEST EMPTY LEAVING ContentServices::callDssV2");
    return false;
  }

  if (numFlightsRequested == 0)
    return true;

  if (_dssV2Host.empty())
  {
    LOG4CXX_FATAL(_logger, "HOST NOT FOUND LEAVING ContentServices::callDssV2");
    return false;
  }

  if (_dssV2Port == 0)
  {
    LOG4CXX_FATAL(_logger, "PORT NOT FOUND LEAVING ContentServices::callDssV2");
    return false;
  }

  eo::Socket socketDs2;
  if (_dssV2TimeOut > 0)
    socketDs2.setTimeOut(_dssV2TimeOut);

  if (_dssV2Linger)
    socketDs2.setLinger(true, _dssV2LingerTime);

  tse::StopWatch sw;
  sw.start();

  if (!socketDs2.connect(_dssV2Host, _dssV2Port))
  {
    LOG4CXX_FATAL(_logger,
                  "PROBLEM CONNECTING HOST: " << _dssV2Host << " PORT: " << _dssV2Port
                                              << " LEAVING ContentServices::callDssV2");
    TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }

  ac::SocketUtils::Message req, res;
  req.command = "RQST";
  req.xmlVersion = "0010";
  req.payload = request;

  bool standardHeaderNotSupported(true);
  if (!ac::SocketUtils::writeMessage(socketDs2, req, standardHeaderNotSupported))
  {
    LOG4CXX_FATAL(_logger, "WRITE MESSAGE FAIL LEAVING ContentServices::callDssV2");
    TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }

  res.clearAll();
  if (!ac::SocketUtils::readMessage(socketDs2, res, standardHeaderNotSupported))
  {
    LOG4CXX_FATAL(_logger, "READ MESSAGE FAIL LEAVING ContentServices::callDssV2");
    TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }

  if (res.command == "REDF")
  {
    if (!CompressUtil::decompress(res.payload))
    {
      LOG4CXX_FATAL(_logger, "DECOMPRESS FAILED ContentServices::callDssV2");
      TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }
    LOG4CXX_DEBUG(_logger, "DSS RESPONSE AFTER DECOMPRESS:" << res.payload);
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "DSS RESPONSE:" << res.payload);
  }

  if (const auto jsonMessage = trx.getXrayJsonMessage())
    jsonMessage->setLlsNetworkTime("DSS", sw.elapsedTime() * xray::MILLISECONDS_IN_SECOND);

  // Parse the response
  const bool isFirstSegmentUnflown =
      !flightMap.empty() ? flightMap.cbegin()->first._unflown : false;
  PricingDssResponseHandler dssRespHandler(trx, isFirstSegmentUnflown);

  dssRespHandler.initialize();

  try
  {
    if (!dssRespHandler.parse(res.payload.c_str()))
    {
      LOG4CXX_FATAL(_logger, "PARSE FAIL LEAVING ContentServices::callDssV2");
      TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }
  }
  catch (std::string& msg)
  {
    msg = "INVALID SEGMENT " + msg;
    const char* error = msg.c_str();
    dssRespDiag195(trx);
    if (xmlDiagRequired(trx, xmlDiagRequested))
    {
      if (xmlDiagRequested == "DSSRES")
        xmlDiag(trx, res.payload);
    }
    throw ErrorResponseException(ErrorResponseException::INVALID_SEGMENT, error);
  }

  processDssFlights(
      trx, dssRespHandler._dssFlights.cbegin(), dssRespHandler._dssFlights.cend(), flightMap);

  dssRespDiag195(trx);
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "DSSRES")
      xmlDiag(trx, res.payload);
  }

  LOG4CXX_INFO(_logger, "SUCESSFUL RETURN ContentServices::callDssV2");
  TseSrvStats::recordDSSv2Call(true,
                               req.payload.length() + TCP_HEADER_LEN,
                               res.payload.length() + TCP_HEADER_LEN,
                               sw.elapsedTime(),
                               trx);

  int numFlightsReturnedFromDss = dssRespHandler.numFltRtnFromDSS();

  if ((getFlownSchedule == MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN) &&
      numFlightsReturnedFromDss == 0)
    return false;
  else
    return true;
}

namespace
{
// This function is used to encode XML responses when
// returning them as MSG elements in Diagnostic 195
std::string
xmlAttrEncode(const std::string& str)
{
  std::ostringstream ret;
  size_t prevSplit = 0;
  for (char c : str)
  {
    switch (c)
    {
    case '<':
      ret << "&lt;";
      break;
    case '>':
      ret << "&gt;";
      break;
    case '"':
      ret << "&quot;";
      break;
    case '&':
      ret << "&amp;";
      break;
    case '\n':
      ret << c;
      prevSplit = ret.str().size();
      break;
    default:
      ret << c;
      break;
    }

    if (ret.str().size() - prevSplit >= 64)
    {
      ret << "\n";
      prevSplit = ret.str().size();
    }
  }
  return ret.str();
}
}

bool
ContentServices::callDssV2(PricingTrx& trx, const MethodGetFlownSchedule getFlownSchedule)
{
  if (fallback::fallbackDebugOverrideDssv2AndAsv2Responses(&trx))
  {
    return callDssV2_old(trx, getFlownSchedule);
  }

  LOG4CXX_INFO(_logger, "ENTERING ContentServices::callDssV2");
  dssReqDiag195(trx, getFlownSchedule);

  PricingDssFlightMap flightMap;

  PricingDssRequest pricingDssRequest(trx);

  std::string request;
  uint16_t numFlightsRequested = 0;

  populateFlightMap(trx, getFlownSchedule, flightMap);
  pricingDssRequest.build(flightMap | boost::adaptors::map_keys, request);
  numFlightsRequested = flightMap.size();

  std::string xmlDiagRequested;
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "DSSREQ")
      xmlDiag(trx, request);
  }

  LOG4CXX_DEBUG(_logger, "DSS REQUEST:" << request);

  if (trx.itin().empty() || !isAnySegmentInRequest(trx))
  {
    LOG4CXX_FATAL(_logger, "REQUEST EMPTY LEAVING ContentServices::callDssV2");
    return false;
  }

  std::string dssResponseOverride = dynamicConfigOverrideDSSv2Response.getValue(&trx);
  bool isDssResponseOverridden = (dssResponseOverride != "N");

  tse::StopWatch sw;
  eo::Socket socketDs2;

  if (LIKELY(!isDssResponseOverridden))
  {
    if (numFlightsRequested == 0)
      return true;

    if (_dssV2Host.empty())
    {
      LOG4CXX_FATAL(_logger, "HOST NOT FOUND LEAVING ContentServices::callDssV2");
      return false;
    }

    if (_dssV2Port == 0)
    {
      LOG4CXX_FATAL(_logger, "PORT NOT FOUND LEAVING ContentServices::callDssV2");
      return false;
    }

    if (_dssV2TimeOut > 0)
      socketDs2.setTimeOut(_dssV2TimeOut);

    if (_dssV2Linger)
      socketDs2.setLinger(true, _dssV2LingerTime);

    sw.start();

    if (!socketDs2.connect(_dssV2Host, _dssV2Port))
    {
      LOG4CXX_FATAL(_logger,
                    "PROBLEM CONNECTING HOST: " << _dssV2Host << " PORT: " << _dssV2Port
                                                << " LEAVING ContentServices::callDssV2");
      TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }
  }

  ac::SocketUtils::Message req, res;
  req.command = "RQST";
  req.xmlVersion = "0010";
  req.payload = request;

  if (LIKELY(!isDssResponseOverridden))
  {
    bool standardHeaderNotSupported(true);
    if (!ac::SocketUtils::writeMessage(socketDs2, req, standardHeaderNotSupported))
    {
      LOG4CXX_FATAL(_logger, "WRITE MESSAGE FAIL LEAVING ContentServices::callDssV2");
      TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }

    res.clearAll();
    if (!ac::SocketUtils::readMessage(socketDs2, res, standardHeaderNotSupported))
    {
      LOG4CXX_FATAL(_logger, "READ MESSAGE FAIL LEAVING ContentServices::callDssV2");
      TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }

    if (res.command == "REDF")
    {
      if (!CompressUtil::decompress(res.payload))
      {
        LOG4CXX_FATAL(_logger, "DECOMPRESS FAILED ContentServices::callDssV2");
        TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
        return false;
      }
      LOG4CXX_DEBUG(_logger, "DSS RESPONSE AFTER DECOMPRESS:" << res.payload);
    }
    else
    {
      LOG4CXX_DEBUG(_logger, "DSS RESPONSE:" << res.payload);
    }
  }
  else
  {
    res.payload = dssResponseOverride;
  }

  if (const auto jsonMessage = trx.getXrayJsonMessage())
    jsonMessage->setLlsNetworkTime("DSS", sw.elapsedTime() * xray::MILLISECONDS_IN_SECOND);

  // Parse the response
  const bool isFirstSegmentUnflown =
      !flightMap.empty() ? flightMap.cbegin()->first._unflown : false;
  PricingDssResponseHandler dssRespHandler(trx, isFirstSegmentUnflown);

  dssRespHandler.initialize();

  try
  {
    if (!dssRespHandler.parse(res.payload.c_str()))
    {
      LOG4CXX_FATAL(_logger, "PARSE FAIL LEAVING ContentServices::callDssV2");
      TseSrvStats::recordDSSv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }
  }
  catch (std::string& msg)
  {
    msg = "INVALID SEGMENT " + msg;
    const char* error = msg.c_str();
    dssRespDiag195(trx);
    if (xmlDiagRequired(trx, xmlDiagRequested))
    {
      if (xmlDiagRequested == "DSSRES")
        xmlDiag(trx, res.payload);
    }
    throw ErrorResponseException(ErrorResponseException::INVALID_SEGMENT, error);
  }

  processDssFlights(
      trx, dssRespHandler._dssFlights.cbegin(), dssRespHandler._dssFlights.cend(), flightMap);

  dssRespDiag195(trx);
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "DSSRES")
      xmlDiag(trx, res.payload);
  }

  addDSSResponseToDiag(res.payload, trx);

  LOG4CXX_INFO(_logger, "SUCESSFUL RETURN ContentServices::callDssV2");
  if (LIKELY(!isDssResponseOverridden))
  {
    TseSrvStats::recordDSSv2Call(true,
                                 req.payload.length() + TCP_HEADER_LEN,
                                 res.payload.length() + TCP_HEADER_LEN,
                                 sw.elapsedTime(),
                                 trx);
  }

  int numFlightsReturnedFromDss = dssRespHandler.numFltRtnFromDSS();

  if ((getFlownSchedule == MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN) &&
      numFlightsReturnedFromDss == 0)
    return false;
  else
    return true;
}

void
ContentServices::checkAndLogAs2Errors(const std::string& xmlData) const
{
  boost::match_results<std::string::const_iterator> m;
  boost::regex expression("(?<=!\\[CDATA\\[Transaction failed: )[^]]*(?=]])");

  if (boost::regex_search(xmlData.begin(), xmlData.end(), m, expression))
    LOG4CXX_ERROR(_logger, "ASv2: " << m.str());
}

bool
ContentServices::callAs2(PricingTrx& trx)
{
  LOG4CXX_INFO(_logger, "ENTERING ContentServices::callAs2");

  if (fallback::fallbackDebugOverrideDssv2AndAsv2Responses(&trx))
  {
    return callAs2_old(trx);
  }

  std::string request;
  AtaeRequest ataeRequest(trx, _useAVS); // by default _useAVS is false, use DCA

  // Override ASv2 request action code for Add Back project.
  if (trx.getRequest()->isSpecificAgencyText() && // SAT attribute set for AA agents
      trx.getRequest()->isLowFareRequested())
    ataeRequest.setOverrideActionCode("WPNC");

  ataeRequest.build(request);
  LOG4CXX_DEBUG(_logger, "ASV2 REQUEST:" << request);

  std::string xmlDiagRequested;
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "ATAEREQ")
      xmlDiag(trx, request);
  }

  std::string asv2ResponseOverride = dynamicConfigOverrideASv2Response.getValue(&trx);
  bool isAsv2ResponseOverridden = (asv2ResponseOverride != "N");

  tse::StopWatch sw;
  eo::Socket socketAs2;

  if (request.empty())
  {
    LOG4CXX_FATAL(_logger, "REQUEST EMPTY LEAVING ContentServices::callAs2");
    return false;
  }

  if (LIKELY(!isAsv2ResponseOverridden))
  {
    if (_asV2Host.empty())
    {
      LOG4CXX_FATAL(_logger, "HOST NOT FOUND ContentServices::callAs2");
      return false;
    }

    if (_asV2Port == 0)
    {
      LOG4CXX_FATAL(_logger, "PORT NOT FOUND ContentServices::callAs2");
      return false;
    }

    if (ataeRequest.fareMarketsSentToAtae().empty())
      return true;

    if (_asV2TimeOut > 0)
      socketAs2.setTimeOut(_asV2TimeOut);

    if (_asV2Linger)
      socketAs2.setLinger(true, _asV2LingerTime);

    sw.start();

    if (!socketAs2.connect(_asV2Host, _asV2Port))
    {
      LOG4CXX_FATAL(_logger,
                    "PROBLEM CONNECTING HOST: " << _asV2Host << " PORT: " << _asV2Port
                                                << "ContentServices::callAs2");
      TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }
  }

  ac::SocketUtils::Message req, res;
  req.command = "RQST";
  req.xmlVersion = "00";
  req.payload = request;
  const bool standardHeaderNotSupported(true);

  if (LIKELY(!isAsv2ResponseOverridden))
  {
    if (!ac::SocketUtils::writeMessage(socketAs2, req, standardHeaderNotSupported))
    {
      LOG4CXX_FATAL(_logger, "WRITE MESSAGE FAILED ContentServices::callAs2");
      TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }

    res.clearAll();
    if (!ac::SocketUtils::readMessage(socketAs2, res, standardHeaderNotSupported))
    {
      LOG4CXX_FATAL(_logger, "READ MESSAGE FAILED ContentServices::callAs2");
      TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }

    if (res.command == "REDF")
    {
      if (!CompressUtil::decompress(res.payload))
      {
        LOG4CXX_FATAL(_logger, "DECOMPRESS FAILED ContentServices::callAs2");
        TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
        return false;
      }
      LOG4CXX_DEBUG(_logger, "ASV2 RESPONSE AFTER DECOMPRESS:" << res.payload);
    }
    else
    {
      LOG4CXX_DEBUG(_logger, "ASV2 RESPONSE:" << res.payload);
    }
  }
  else
  {
    res.payload = asv2ResponseOverride;
  }

  if (const auto jsonMessage = trx.getXrayJsonMessage())
    jsonMessage->setLlsNetworkTime("ASV2", sw.elapsedTime() * xray::MILLISECONDS_IN_SECOND);

  addASResponseToDiag(res.payload, trx);

  AtaeResponseHandler ataeResponseHandler(trx);
  ataeResponseHandler.initialize();
  ataeResponseHandler.fareMarketsSentToAtae() = ataeRequest.fareMarketsSentToAtae();
  if (!ataeResponseHandler.parse(res.payload.c_str()))
  {
    LOG4CXX_FATAL(_logger, "PARSE FAIL LEAVING ContentServices::callAsV2");
    TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested.compare(0, 6, "AVFAKE") == 0)
    {
      debugOnlyFakeASV2(trx, ataeResponseHandler.fareMarketsSentToAtae(), xmlDiagRequested); // MSD
      debugOnlyFakeASV2ForThruFMs(
          trx, ataeResponseHandler.fareMarketsSentToAtae(), xmlDiagRequested); // AK
    }
  }

  checkAndLogAs2Errors(res.payload);

  ataeDiag195(trx, ataeResponseHandler.fareMarketsSentToAtae());
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "ATAERES")
      xmlDiag(trx, res.payload);
  }
  processDiag998(trx, res.payload);

  LOG4CXX_INFO(_logger, "SUCESSFUL RETURN ContentServices::callAsV2");
  if (LIKELY(!isAsv2ResponseOverridden))
  {
    TseSrvStats::recordASv2Call(true,
                                req.payload.length() + TCP_HEADER_LEN,
                                res.payload.length() + TCP_HEADER_LEN,
                                sw.elapsedTime(),
                                trx);
  }
  return true;
}

bool
ContentServices::callAs2_old(PricingTrx& trx)
{
  LOG4CXX_INFO(_logger, "ENTERING ContentServices::callAs2_old");

  std::string request;
  AtaeRequest ataeRequest(trx, _useAVS); // by default _useAVS is false, use DCA

  // Override ASv2 request action code for Add Back project.
  if (trx.getRequest()->isSpecificAgencyText() && // SAT attribute set for AA agents
      trx.getRequest()->isLowFareRequested())
    ataeRequest.setOverrideActionCode("WPNC");

  ataeRequest.build(request);
  LOG4CXX_DEBUG(_logger, "ASV2 REQUEST:" << request);

  std::string xmlDiagRequested;
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "ATAEREQ")
      xmlDiag(trx, request);
  }

  if (request.empty())
  {
    LOG4CXX_FATAL(_logger, "REQUEST EMPTY LEAVING ContentServices::callAs2");
    return false;
  }

  if (_asV2Host.empty())
  {
    LOG4CXX_FATAL(_logger, "HOST NOT FOUND ContentServices::callAs2");
    return false;
  }

  if (_asV2Port == 0)
  {
    LOG4CXX_FATAL(_logger, "PORT NOT FOUND ContentServices::callAs2");
    return false;
  }

  if (ataeRequest.fareMarketsSentToAtae().empty())
    return true;

  eo::Socket socketAs2;
  if (_asV2TimeOut > 0)
    socketAs2.setTimeOut(_asV2TimeOut);

  if (_asV2Linger)
    socketAs2.setLinger(true, _asV2LingerTime);

  tse::StopWatch sw;
  sw.start();

  if (!socketAs2.connect(_asV2Host, _asV2Port))
  {
    LOG4CXX_FATAL(_logger,
                  "PROBLEM CONNECTING HOST: " << _asV2Host << " PORT: " << _asV2Port
                                              << "ContentServices::callAs2");
    TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }

  ac::SocketUtils::Message req, res;
  req.command = "RQST";
  req.xmlVersion = "00";
  req.payload = request;
  const bool standardHeaderNotSupported(true);

  if (!ac::SocketUtils::writeMessage(socketAs2, req, standardHeaderNotSupported))
  {
    LOG4CXX_FATAL(_logger, "WRITE MESSAGE FAILED ContentServices::callAs2");
    TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }

  res.clearAll();
  if (!ac::SocketUtils::readMessage(socketAs2, res, standardHeaderNotSupported))
  {
    LOG4CXX_FATAL(_logger, "READ MESSAGE FAILED ContentServices::callAs2");
    TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }

  if (res.command == "REDF")
  {
    if (!CompressUtil::decompress(res.payload))
    {
      LOG4CXX_FATAL(_logger, "DECOMPRESS FAILED ContentServices::callAs2");
      TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }
    LOG4CXX_DEBUG(_logger, "ASV2 RESPONSE AFTER DECOMPRESS:" << res.payload);
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "ASV2 RESPONSE:" << res.payload);
  }

  if (const auto jsonMessage = trx.getXrayJsonMessage())
    jsonMessage->setLlsNetworkTime("ASV2", sw.elapsedTime() * xray::MILLISECONDS_IN_SECOND);

  // Parse the response

  AtaeResponseHandler ataeResponseHandler(trx);
  ataeResponseHandler.initialize();
  ataeResponseHandler.fareMarketsSentToAtae() = ataeRequest.fareMarketsSentToAtae();
  if (!ataeResponseHandler.parse(res.payload.c_str()))
  {
    LOG4CXX_FATAL(_logger, "PARSE FAIL LEAVING ContentServices::callAsV2");
    TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested.compare(0, 6, "AVFAKE") == 0)
    {
      debugOnlyFakeASV2(trx, ataeResponseHandler.fareMarketsSentToAtae(), xmlDiagRequested); // MSD
      debugOnlyFakeASV2ForThruFMs(
          trx, ataeResponseHandler.fareMarketsSentToAtae(), xmlDiagRequested); // AK
    }
  }

  checkAndLogAs2Errors(res.payload);

  ataeDiag195(trx, ataeResponseHandler.fareMarketsSentToAtae());
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "ATAERES")
      xmlDiag(trx, res.payload);
  }
  processDiag998(trx, res.payload);

  LOG4CXX_INFO(_logger, "SUCESSFUL RETURN ContentServices::callAsV2");
  TseSrvStats::recordASv2Call(true,
                              req.payload.length() + TCP_HEADER_LEN,
                              res.payload.length() + TCP_HEADER_LEN,
                              sw.elapsedTime(),
                              trx);
  return true;
}

bool
ContentServices::callAs2Shopping(PricingTrx& trx, AtaeResponseHandler& ataeResponseHandler)
{
  LOG4CXX_INFO(_logger, "ENTERING ContentServices::callAs2Shopping()");
  std::string request;
  AtaeRequest ataeRequest(trx, _useAVS); // by default _useAVS is false, use DCA
  ataeRequest.buildShopping(request);
  LOG4CXX_DEBUG(_logger, "ASV2 SHOPPING REQUEST:" << request);

  std::string xmlDiagRequested;
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "ATAEREQ")
      xmlDiag(trx, request);
  }

  if (request.empty())
  {
    LOG4CXX_FATAL(_logger, "REQUEST EMPTY LEAVING ContentServices::callAs2Shopping()");
    return false;
  }

  if (_asV2Host.empty())
  {
    LOG4CXX_FATAL(_logger, "HOST NOT FOUND ContentServices::callAs2Shopping()");
    return false;
  }

  if (_asV2Port == 0)
  {
    LOG4CXX_FATAL(_logger, "PORT NOT FOUND ContentServices::callAs2Shopping()");
    return false;
  }

  if (ataeRequest.fareMarketsSentToAtae().empty())
    return true;

  eo::Socket socketAs2;
  if (_asV2TimeOut > 0)
    socketAs2.setTimeOut(_asV2TimeOut);

  if (_asV2Linger)
    socketAs2.setLinger(true, _asV2LingerTime);

  tse::StopWatch sw;
  sw.start();

  bool connectValue;
  if (trx.getOptions()->cacheProxyMIP())
  {
    connectValue = socketAs2.connect(_asV2HostCacheProxyMIP, _asV2PortCacheProxyMIP);
  }
  else
  {
    connectValue = socketAs2.connect(_asV2Host, _asV2Port);
  }

  if (!connectValue)
  {
    if (trx.getOptions()->cacheProxyMIP())
    {
      LOG4CXX_FATAL(_logger,
                    "PROBLEM CONNECTING HOST: " << _asV2HostCacheProxyMIP
                                                << " PORT: " << _asV2PortCacheProxyMIP
                                                << "ContentServices::callAs2Shopping()");
    }
    else
    {
      LOG4CXX_FATAL(_logger,
                    "PROBLEM CONNECTING HOST: " << _asV2Host << " PORT: " << _asV2Port
                                                << "ContentServices::callAs2Shopping()");
    }
    TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }

  ac::SocketUtils::Message req, res;
  req.command = "RQST";
  req.xmlVersion = "00";
  req.payload = request;
  const bool standardHeaderNotSupported(true);

  if (!ac::SocketUtils::writeMessage(socketAs2, req, standardHeaderNotSupported))
  {
    LOG4CXX_FATAL(_logger, "WRITE MESSAGE FAILED ContentServices::callAs2Shopping()");
    TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }

  res.clearAll();
  if (!ac::SocketUtils::readMessage(socketAs2, res, standardHeaderNotSupported))
  {
    LOG4CXX_FATAL(_logger, "READ MESSAGE FAILED ContentServices::callAs2Shopping()");
    TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }
  if (res.command == "REDF")
  {
    if (!CompressUtil::decompress(res.payload))
    {
      LOG4CXX_FATAL(_logger, "DECOMPRESS FAILED ContentServices::callAs2Shopping()");
      TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
      return false;
    }
    LOG4CXX_DEBUG(_logger, "ASV2 SHOPPING RESPONSE AFTER DECOMPRESS:" << res.payload);
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "ASV2 SHOPPING RESPONSE:" << res.payload);
  }

  if (const auto jsonMessage = trx.getXrayJsonMessage())
    jsonMessage->setLlsNetworkTime("ASV2", sw.elapsedTime() * xray::MILLISECONDS_IN_SECOND);

  // Parse the response

  ataeResponseHandler.initialize();
  ataeResponseHandler.fareMarketsSentToAtae() = ataeRequest.fareMarketsSentToAtae();
  if (!ataeResponseHandler.parse(res.payload.c_str()))
  {
    LOG4CXX_FATAL(_logger, "PARSE FAIL LEAVING ContentServices::callAsV2Shopping()");
    TseSrvStats::recordASv2Call(false, 0, 0, sw.elapsedTime(), trx);
    return false;
  }
  ataeDiag195(trx, ataeResponseHandler.fareMarketsSentToAtae());
  if (xmlDiagRequired(trx, xmlDiagRequested))
  {
    if (xmlDiagRequested == "ATAERES")
      xmlDiag(trx, res.payload);
  }
  processDiag998(trx, res.payload);

  LOG4CXX_INFO(_logger, "SUCESSFUL RETURN ContentServices::callAsV2Shopping()");
  TseSrvStats::recordASv2Call(true,
                              req.payload.length() + TCP_HEADER_LEN,
                              res.payload.length() + TCP_HEADER_LEN,
                              sw.elapsedTime(),
                              trx);
  return true;
}

void
ContentServices::ataeDiag195(PricingTrx& trx, const std::vector<FareMarket*>& processedFms)
{
  DiagManager diag(trx, Diagnostic195);
  if (!diag.isActive())
    return;

  diag << " \n";
  diag << " \n";
  diag << "******************** AS V2 DIAGNOSTIC ********************* \n";

  std::string asV2Host = _asV2Host;
  uint16_t asV2Port = _asV2Port;
  if (trx.getOptions()->cacheProxyMIP())
  {
    asV2Host = _asV2HostCacheProxyMIP;
    asV2Port = _asV2PortCacheProxyMIP;
  }

  std::string hostSabreView;
  for (char caps : asV2Host)
  {
    if (isalpha(caps) && islower(caps))
    {
      caps = toupper(caps);
    }
    hostSabreView.push_back(caps);
  }
  diag << "HOST: " << hostSabreView << "    PORT: " << asV2Port << "    TIMEOUT: " << _asV2TimeOut
       << " \n";

  diag << "CLIENT TRX ID: " << trx.billing()->clientTransactionID() << " \n";

  if (sendResDataToAtae(trx))
    diag << "-- JOURNEY: RESERVATION DATA SENT TO ATAE -- \n";

  const int nFm = processedFms.size();
  diag << "NUMBER OF FAREMARKETS PROCESSED BY ATAEJ: " << nFm << " \n";

  fareMarketsDiag195(processedFms, diag, trx);

  diag << "\n****************** END DIAG 195 *************************** \n";
  return;
}

void
ContentServices::fareMarketsDiag195(const std::vector<FareMarket*>& processedFms,
                                    DiagManager& diag,
                                    PricingTrx& trx)
{
  const AirSeg* airSeg = nullptr;
  std::string conStop("");
  const std::vector<ClassOfService*>* cosVec = nullptr;
  const ClassOfService* pCos = nullptr;
  int iCos = 0;
  int lenVec = 0;
  int adj = 0;
  const int nFm = processedFms.size();
  for (int iFm = 0; iFm < nFm; iFm++)
  {
    const FareMarket& fm = *(processedFms[iFm]);
    diag << "FARE MARKET " << iFm + 1 << " : " << fm.boardMultiCity() << "-" << fm.offMultiCity()
         << " NUMBER OF FLIGHTS: " << fm.travelSeg().size() << " \n";

    OAndDMarket* od = nullptr;
    if ((fm.travelSeg().size() > 1) && (!trx.itin().empty()))
      od = JourneyUtil::getOAndDMarketFromFM(*(trx.itin()[0]), &fm);

    std::vector<TravelSeg*>::const_iterator tvlI = fm.travelSeg().begin();
    const std::vector<TravelSeg*>::const_iterator tvlE = fm.travelSeg().end();
    for (uint32_t iFlt = 0; tvlI != tvlE; tvlI++, iFlt++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*tvlI);
      if (airSeg == nullptr)
      {
        diag << " ARUNK"
             << " \n";
        continue;
      }
      diag << " " << std::setw(2) << airSeg->pnrSegment();
      diag << ((od && od->isJourneyByMarriage() && od->maySegUseODCos(*tvlI)) ? "*" : " ");
      diag << MCPCarrierUtil::swapToPseudo(&trx, airSeg->carrier());
      diag << std::setw(4) << airSeg->flightNumber();
      diag << airSeg->getBookingCode();
      diag << " " << std::setw(5) << airSeg->departureDT().dateToString(DDMMM, "");
      diag << " " << airSeg->origAirport() << airSeg->destAirport();
      diag << " " << std::setw(6) << airSeg->departureDT().timeToString(HHMM_AMPM, "");
      diag << " " << std::setw(6) << airSeg->arrivalDT().timeToString(HHMM_AMPM, "");
      conStop.resize(0);
      if (airSeg->stopOver())
        conStop = "STOPOVER - ";
      else
        conStop = "CONNECTN - ";
      conStop = conStop + airSeg->destAirport();
      diag << " " << conStop << " \n";

      diag << "    AVAIL: ";
      if (!(fm.classOfServiceVec().empty()))
      {
        if (iFlt < fm.classOfServiceVec().size())
        {
          cosVec = fm.classOfServiceVec()[iFlt];
          if (cosVec != nullptr)
          {
            iCos = 0;
            lenVec = (*cosVec).size();
            adj = 0;
            for (; iCos < lenVec; iCos++)
            {
              pCos = (*cosVec)[iCos];
              if (pCos == nullptr)
                continue;

              diag << pCos->bookingCode() << pCos->numSeats() << " ";

              // adj will keep an account if we should keep displaying the booking codes
              // in the same line or should we start a new line
              if (pCos->numSeats() > 99)
              {
                adj = adj + 5;
              }
              else
              {
                if (pCos->numSeats() < 10)
                  adj = adj + 3;
                else
                  adj = adj + 4;
              }
              // only display 40 characters in one line
              if (adj > 46)
              {
                diag << " \n";
                diag << "           ";
                adj = 0;
              }
            }
          }
        }
      }
      diag << " \n";
    }
    diag << " \n";
  }
}

void
ContentServices::printDiag195DssReqForTravelSegment(const TravelSeg* travelSeg,
                                                    const PricingTrx& trx,
                                                    const MethodGetFlownSchedule getFlownSchedule,
                                                    DiagManager& diag) const
{
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
  if (airSeg == nullptr || travelSeg->segmentType() == Open || travelSeg->segmentType() == Arunk)
  {
    return;
  }

  if (!airSeg->unflown() && getFlownSchedule == MethodGetFlownSchedule::SKIP_FLOWN)
    return;

  diag << " " << MCPCarrierUtil::swapToPseudo(&trx, airSeg->carrier()) << " " << std::setw(4)
       << airSeg->flightNumber() << airSeg->getBookingCode() << " " << airSeg->origAirport() << "-"
       << airSeg->destAirport() << " " << airSeg->departureDT().dateToString(YYYYMMDD, "-")
       << " \n";
}

void
ContentServices::dssReqDiag195(PricingTrx& trx, const MethodGetFlownSchedule getFlownSchedule)
{
  DiagManager diag(trx, Diagnostic195);
  if (!diag.isActive())
    return;

  diag << " \n";
  diag << "******************** START  DIAG  195 ********************* \n";

  diagOACData(trx, diag);
  diagReservationData(trx, diag);

  diag << "******************** DSS V2 DIAGNOSTIC ******************** \n";
  std::string hostSabreView;
  for (char caps : _dssV2Host)
  {
    if (isalpha(caps) && islower(caps))
    {
      caps = toupper(caps);
    }
    hostSabreView.push_back(caps);
  }
  diag << "HOST: " << hostSabreView << "    PORT: " << _dssV2Port
       << "    TIMEOUT: " << _dssV2TimeOut << " \n";

  diag << "REQUEST SENT TO DSSV2: \n";

  if (trx.itin().size() < 1)
  {
    diag << "ERROR - ITIN SIZE < 1 \n";
    return;
  }

  if (!isAnySegmentInRequest(trx))
  {
    diag << "ERROR - TRAVEL SEGS SIZE < 1 \n";
    return;
  }

  if (getFlownSchedule == MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN)
    diag << "** GET NEXT SAME DOW FLT SCHEDULE FOR FLOWN SEGMENTS **\n";
  else if (getFlownSchedule == MethodGetFlownSchedule::SKIP_FLOWN)
    diag << "** DO NOT GET SCHEDULE FOR FLOWN SEGMENTS **\n";

  for (auto iter = trx.itin().cbegin(); iter != trx.itin().cend(); ++iter)
  {
    for (const TravelSeg* travelSeg : (*iter)->travelSeg())
    {
      printDiag195DssReqForTravelSegment(travelSeg, trx, getFlownSchedule, diag);
    }

    if (!trx.activationFlags().isAB240())
      break;

    diag << std::string(DiagCollector::DEFAULT_LINEWRAP_LENGTH, '-') << "\n";
  }
}

void
ContentServices::printDiag195DssRespForTravelSegment(const TravelSeg* travelSeg,
                                                     const PricingTrx& trx,
                                                     DiagManager& diag) const
{
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
  if (travelSeg->segmentType() == Open)
  {
    diag << "   -- OPEN SEGMENT ";
    if (airSeg != nullptr)
    {
      diag << airSeg->pnrSegment() << " " << MCPCarrierUtil::swapToPseudo(&trx, airSeg->carrier())
           << std::setw(4) << airSeg->flightNumber() << " " << airSeg->origAirport() << "-"
           << airSeg->destAirport() << " " << airSeg->departureDT().dateToString(YYYYMMDD, "-");
    }
    diag << " --"
         << " \n";
    return;
  }

  if (airSeg == nullptr || travelSeg->segmentType() == Arunk)
  {
    diag << "   -- ARUNK SEGMENT " << travelSeg->pnrSegment() << " " << travelSeg->origAirport()
         << "-" << travelSeg->destAirport() << " --"
         << " \n";
    return;
  }

  if (airSeg == nullptr)
    return;

  diag << std::setw(2) << travelSeg->pnrSegment();

  diag << " " << MCPCarrierUtil::swapToPseudo(&trx, airSeg->carrier()) << " " << std::setw(4)
       << airSeg->flightNumber();
  diag << " " << airSeg->origAirport() << "-" << airSeg->destAirport();

  if (!airSeg->unflown() &&
      (trx.excTrxType() >= PricingTrx::AR_EXC_TRX && trx.excTrxType() <= PricingTrx::ME_DIAG_TRX))
  {
    diag << " EQUIP: " << airSeg->equipmentType();
  }
  else
  {
    diag << " BBR: ";
    if (airSeg->bbrCarrier())
      diag << "T ";
    else
      diag << "F ";
    diag << " EQUIP: " << airSeg->equipmentType();
    diag << "  OPERATING FLT: " << airSeg->operatingCarrierCode() << std::setw(4)
         << airSeg->operatingFlightNumber();

    if ("CHG" == airSeg->equipmentType())
    {
      diag << " \n                                  ";
      const int MaxEqTypeInLine = 7;
      int line = 0;

      for (EquipmentType ep : airSeg->equipmentTypes())
      {
        diag << ep << " ";
        line++;

        if (!(line % MaxEqTypeInLine))
          diag << "\n                                  ";
      }

      diag << " \n";
    }

    diag << " \n";
    diag << "           OFFERD COS: ";
    std::for_each(airSeg->classOfService().begin(),
                  airSeg->classOfService().end(),
                  [&diag](const ClassOfService* classOfService)
                  { diag << classOfService->bookingCode(); });
  }

  diag << "  STOPS: ";
  std::for_each(airSeg->hiddenStops().begin(),
                airSeg->hiddenStops().end(),
                [&diag](const Loc* loc)
                { diag << loc->loc() << " "; });
  diag << " \n";
}

void
ContentServices::dssRespDiag195(PricingTrx& trx)
{
  DiagManager diag(trx, Diagnostic195);
  if (!diag.isActive())
    return;

  diag << " \n";
  diag << "RESPONSE FROM DSSV2: "
       << " \n";

  for (auto iter = trx.itin().cbegin(); iter != trx.itin().cend(); ++iter)
  {
    for (const TravelSeg* travelSeg : (*iter)->travelSeg())
    {
      printDiag195DssRespForTravelSegment(travelSeg, trx, diag);
    }

    if (!trx.activationFlags().isAB240())
      break;

    diag << std::string(DiagCollector::DEFAULT_LINEWRAP_LENGTH, '-') << "\n";
  }
}

void
ContentServices::diagReservationData(PricingTrx& trx, DiagManager& diag)
{
  ReservationData* reservData = trx.getRequest()->reservationData();
  if (reservData == nullptr)
    return;

  std::string xmlDiagRequested;
  if (xmlDiagRequired(trx, xmlDiagRequested))
    return;

  if (xmlDiagRequested != "RESDATA")
    return;

  diag << "---------------- START RESERVATION DATA  ------------------\n";
  diag << "OWNER:" << reservData->agent() << "  OWNER ID:";
  if (reservData->agentInd() > 0)
    diag << reservData->agentInd();
  diag << "  IATA:" << reservData->agentIATA() << "\n";

  diag << "  PCC:" << reservData->agentPCC() << "  CITY:" << reservData->agentCity()
       << "  COUNTRY:" << reservData->agentNation() << "  CRS:" << reservData->agentCRS() << " \n";

  uint16_t numItinSegs = reservData->reservationSegs().size();
  for (uint16_t i = 0; i < numItinSegs; ++i)
  {
    ReservationSeg* resSeg = reservData->reservationSegs()[i];
    diag << "TRAVEL SEGMENT " << (i + 1) << ": \n"
         << "  " << resSeg->carrier() << " " << resSeg->flightNumber() << resSeg->bookingCode()
         << " " << resSeg->pssDepartureDate() << " " << resSeg->origAirport()
         << resSeg->destAirport() << " " << resSeg->actionCode() << resSeg->numInParty() << " "
         << resSeg->pssDepartureTime() << " " << resSeg->pssArrivalTime() << " "
         << resSeg->pssArrivalDate() << "\n";
    diag << "  MARRIED SEG CTRL ID:";
    if (resSeg->marriedSegCtrlId() > 0)
      diag << resSeg->marriedSegCtrlId();

    diag << "  GROUP NUM:";
    if (resSeg->marriedGrpNo() > 0)
      diag << resSeg->marriedGrpNo();

    diag << "  SEQ NUM:";
    if (resSeg->marriedSeqNo() > 0)
      diag << resSeg->marriedSeqNo();

    diag << " \n";
    diag << "  POLLING IND:" << resSeg->pollingInd() << "  ETICKET IND:" << resSeg->eticket()
         << " \n";
  }

  uint16_t numRLS = reservData->recordLocators().size();
  if (numRLS > 0)
    diag << "RECORD LOCATOR DATA : \n";
  for (uint16_t i = 0; i < numRLS; ++i)
  {
    RecordLocatorInfo* rl = reservData->recordLocators()[i];

    diag << "  RECORD LOCATOR CARRIER:" << rl->originatingCxr()
         << "          RECORD LOCATOR:" << rl->recordLocator() << " \n";
  }

  uint16_t numPax = reservData->passengers().size();
  if (numPax > 0)
    diag << "PASSENGER DATA : \n";
  for (uint16_t i = 0; i < numPax; ++i)
  {
    Traveler* pax = reservData->passengers()[i];
    diag << "  PSGR REF:" << pax->referenceNumber() << " \n"
         << "   LAST NAME QUALIFIER:" << pax->lastNameQualifier()
         << "  LAST NAME:" << pax->lastName() << " \n"
         << "   FIRST NAME QUALIFIER:" << pax->firstNameQualifier()
         << "  FIRST NAME:" << pax->firstName() << " \n"
         << "   OTHER NAME:" << pax->otherName()
         << "  TRAVEL WITH INFANT:" << (pax->travelWithInfant() ? "T" : "F") << " \n";

    FrequentFlyerAccount* freqFlyerAcct = pax->freqFlyerAccount();
    if (freqFlyerAcct != nullptr) // FREQ Flyer Info
    {
      diag << "   FREQUENT FLYER VIP:";
      if (freqFlyerAcct->vipType() > 0)
        diag << freqFlyerAcct->vipType();
      diag << "  CARRIER:" << freqFlyerAcct->carrier()
           << "  ACCOUNT:" << freqFlyerAcct->accountNumber() << " \n";
      diag << "   FREQUENT FLYER PARTNER: \n     ";
      const std::vector<CarrierCode>& partners = freqFlyerAcct->partner();
      std::vector<CarrierCode>::const_iterator ffpIter = partners.begin();
      for (; ffpIter != partners.end(); ++ffpIter)
      {
        diag << *ffpIter;
      }
      diag << " \n";
    }

    diag << " \n";
  }

  uint16_t numCorpAcct = reservData->corpFreqFlyerAccounts().size();
  if (numCorpAcct > 0)
    diag << "CORPORATE FREQUENT FLYER INFO : \n";
  for (uint16_t i = 0; i < numCorpAcct; ++i)
  {
    FrequentFlyerAccount* freqFlyerAcct = reservData->corpFreqFlyerAccounts()[i];
    if (freqFlyerAcct != nullptr) // FREQ Flyer Info
    {
      diag << "  CORP FREQUENT FLYER CARRIER: " << freqFlyerAcct->carrier()
           << "  ACCOUNT: " << freqFlyerAcct->accountNumber() << " \n";
    }
  }

  diag << "---------------- END OF RESERVATION DATA ------------------ \n";
  diag << " \n";
}

void
ContentServices::diagOACData(PricingTrx& trx, DiagManager& diag)
{
  Agent* agent = trx.getRequest()->ticketingAgent();
  if ((agent == nullptr) || agent->officeDesignator().empty())
    return;

  diag << "AGENT OAC:"
       << "\n";
  if (agent->officeDesignator().size() >= 5)
  {
    diag << "IATA CITY: " << agent->officeDesignator().substr(0, 3) << "\n";
    diag << "ACCOUNT CODE: " << agent->officeDesignator().substr(3, 2) << "\n";
  }
  diag << "OFFICE/STATION CODE: " << agent->officeStationCode() << "\n";
  diag << "DEFAULT TICKETING CARRIER: " << agent->defaultTicketingCarrier() << "\n";
}

bool
ContentServices::sendResDataToAtae(PricingTrx& trx)
{
  if (trx.getRequest()->reservationData() == nullptr)
    return false;

  if (!(trx.getOptions()->journeyActivatedForPricing()))
    return false;

  if (!(trx.getOptions()->applyJourneyLogic()))
    return false;

  // if any carrier in itin is activated for journey
  // then send reservation data to ATAE.
  // if no carrier is activated for Journey then there
  // is no need to send reservatio ndata to ATAE
  const Itin& itin = *(trx.itin()[0]);
  std::vector<TravelSeg*>::const_iterator tvlI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();

  for (; tvlI != tvlE; tvlI++)
  {
    if (*tvlI == nullptr || dynamic_cast<AirSeg*>(*tvlI) == nullptr)
      continue;
    const AirSeg& airSeg = *(dynamic_cast<AirSeg*>(*tvlI));
    if (airSeg.flowJourneyCarrier() || airSeg.localJourneyCarrier())
      return true;
  }
  return false;
}

bool
ContentServices::xmlDiagRequired(PricingTrx& trx, std::string& diagParam)
{
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  if (!trx.diagnostic().isActive())
    return false;

  if (trx.diagnostic().diagParamMap().size() != 0)
  {
    std::map<std::string, std::string>::iterator it = trx.diagnostic().diagParamMap().begin();
    it = trx.diagnostic().diagParamMap().find("AV");

    if (it != trx.diagnostic().diagParamMap().end())
    {
      diagParam = (*it).first + (*it).second;
      return true;
    }
  }
  if (diagType != Diagnostic195)
    return false;

  std::map<std::string, std::string>::iterator endI = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::iterator beginI =
      trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);

  if (beginI == endI)
    return false;

  diagParam = (*beginI).second;
  if (diagParam.empty())
    return false;

  if (diagParam == "ATAEREQ" || diagParam == "ATAERES" || diagParam == "DSSREQ" ||
      diagParam == "DSSRES")
  {
    return true;
  }
  return false;
}

void
ContentServices::xmlDiag(PricingTrx& trx, std::string& xmlData)
{
  DiagManager diag(trx, Diagnostic195);
  if (!diag.isActive())
    return;

  std::string diagParam;
  if (!xmlDiagRequired(trx, diagParam))
    return;

  if (diagParam == "ATAEREQ")
    diagParam = "XML ASV2 REQUEST";
  else if (diagParam == "ATAERES")
    diagParam = "XML ASV2 RESPONSE";
  else if (diagParam == "DSSREQ")
    diagParam = "XML DSSV2 REQUEST";
  else if (diagParam == "DSSRES")
    diagParam = "XML DSSV2 RESPONSE";
  else
    return;
  diag << " \n";
  diag << "************** " << diagParam << " SIZE-" << xmlData.size() << " *************** \n";
  diagParam.resize(0);

  size_t total = 0;
  uint16_t i = 0;
  uint16_t j = 0;
  char dispChar;
  total = xmlData.size();
  for (; i < total; ++i)
  {
    dispChar = xmlData[i];
    if (dispChar == '"')
    {
      if (j > 57)
      {
        diagParam.push_back('\n');
        j = 0;
      }
      continue;
    }
    if (dispChar == '<' && j > 57)
    {
      diagParam.push_back('\n');
      diagParam.push_back(':');
      j = 0;
      continue;
    }
    if (dispChar == '<' || dispChar == '>')
      dispChar = ':';
    else if (dispChar == '=')
      dispChar = '-';

    if (isalpha(dispChar) && islower(dispChar))
      dispChar = toupper(dispChar);

    diagParam.push_back(dispChar);
    if (j >= 60 || dispChar == ':')
    {
      dispChar = '\n';
      diagParam.push_back(dispChar);
      j = 0;
    }
    else
    {
      ++j;
    }
  }

  diag << diagParam;
  diag << " \n";
}

void
ContentServices::checkDuplicateFareMarketsShop(Itin& itin, FareMarket* fm)
{
  std::vector<FareMarket*>::iterator fmI;

  SameFareMarketWithCOS isSameFM(fm);

  fmI = std::find_if(itin.fareMarket().begin(), itin.fareMarket().end(), isSameFM);

  if (fmI != itin.fareMarket().end())
  {
    FareMarket& fmkt = *(*fmI);
    fm->classOfServiceVec() = fmkt.classOfServiceVec();
  }
}

void
ContentServices::checkDuplicateFareMarkets(Itin& itin, FareMarket* fm, PricingTrx& trx)
{
  std::vector<FareMarket*>::iterator fmI = itin.fareMarket().begin();
  std::vector<FareMarket*>::iterator fmE = itin.fareMarket().end();

  for (; fmI != fmE; ++fmI)
  {
    FareMarket& fmkt = *(*fmI);
    if (fmkt.classOfServiceVec().empty())
      continue;
    if (fmkt.travelSeg() == fm->travelSeg())
    {
      fm->classOfServiceVec() = fmkt.classOfServiceVec();
      fm->availBreaks() = fmkt.availBreaks();
      return;
    }
  }
}

void
ContentServices::adjustAvail(PricingTrx& trx)
{
  std::vector<AvailData*>::const_iterator avlDataI = trx.getRequest()->availData().begin();
  std::vector<AvailData*>::const_iterator avlDataE = trx.getRequest()->availData().end();

  for (; avlDataI != avlDataE; avlDataI++)
  {
    AvailData& availData = *(*avlDataI);
    if (availData.flightAvails().empty())
      continue;
    FlightAvail& fltAvail = *(availData.flightAvails().front());
    fltAvail.fillClassOfService(trx);
    updateAvail(trx, fltAvail);
  }
}

void
ContentServices::updateAvail(PricingTrx& trx, const FlightAvail& fltAvail)
{
  Itin& itin = *(trx.itin().front());
  std::vector<FareMarket*>::iterator fmIt = itin.fareMarket().begin();
  const std::vector<FareMarket*>::iterator fmItEnd = itin.fareMarket().end();

  for (; fmIt != fmItEnd; ++fmIt)
  {
    FareMarket& fmkt = *(*fmIt);
    updateFareMarketAvail(fmkt, fltAvail);
  }
}

void
ContentServices::updateFareMarketAvail(FareMarket& fm, const FlightAvail& fltAvail)
{
  std::vector<TravelSeg*>::iterator tvlI = fm.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = fm.travelSeg().end();

  for (uint16_t i = 0; tvlI != tvlE; tvlI++, i++)
  {
    TravelSeg& tvlSeg = *(*tvlI);
    if (tvlSeg.pnrSegment() != fltAvail.upSellPNRsegNum())
      continue;

    if (i < fm.classOfServiceVec().size() && fm.classOfServiceVec()[i])
      updateTravelSegAvail(*(fm.classOfServiceVec()[i]), fltAvail);

    if (fm.travelSeg().size() == 1)
      updateTravelSegAvail(tvlSeg.classOfService(), fltAvail);
  }
}

void
ContentServices::updateTravelSegAvail(std::vector<ClassOfService*>& cos,
                                      const FlightAvail& fltAvail)
{
  std::vector<ClassOfService*>::iterator cosVecI = cos.begin();
  std::vector<ClassOfService*>::iterator cosVecE = cos.end();

  for (; cosVecI != cosVecE; cosVecI++)
  {
    if (*cosVecI == nullptr)
      continue;
    ClassOfService& cs = *(*cosVecI);

    if (!foundCos(cs, fltAvail))
      cs.numSeats() = 0;
  }
}

bool
ContentServices::foundCos(const ClassOfService& cos, const FlightAvail& fltAvail)
{
  std::vector<ClassOfService*>::const_iterator fltAvailCosI = fltAvail.classOfService().begin();
  std::vector<ClassOfService*>::const_iterator fltAvailCosE = fltAvail.classOfService().end();

  for (; fltAvailCosI != fltAvailCosE; fltAvailCosI++)
  {
    if (*fltAvailCosI == nullptr)
      continue;
    const ClassOfService& cs = *(*fltAvailCosI);
    if (cs.bookingCode() == cos.bookingCode())
      return true;
  }

  return false;
}

void
ContentServices::upSellAvailDiag195(PricingTrx& trx, const std::vector<FareMarket*>& processedFms)
{
  DiagManager diag(trx, Diagnostic195);
  if (!diag.isActive())
    return;

  diag << " \n";
  diag << "***************** UPSELL AVAIL DIAGNOSTIC ****************** \n";

  const int nFm = processedFms.size();
  diag << "FAREMARKETS BUILT USING INTELLISELL AVAIL: " << nFm << " \n";

  fareMarketsDiag195(processedFms, diag, trx);

  diag << " \n";
  diag << "AVAIL DATA SENT BY INTELLISELL: " << trx.getRequest()->availData().size() << " \n";
  int marketNumber = 1;
  std::vector<AvailData*>::const_iterator avlDataI = trx.getRequest()->availData().begin();
  std::vector<AvailData*>::const_iterator avlDataE = trx.getRequest()->availData().end();

  std::vector<FlightAvail*>::const_iterator fltAvlI;
  std::vector<FlightAvail*>::const_iterator fltAvlE;

  for (; avlDataI != avlDataE; avlDataI++, marketNumber++)
  {
    const AvailData& availData = *(*avlDataI);
    diag << "MARKET " << marketNumber << " \n";
    fltAvlI = availData.flightAvails().begin();
    fltAvlE = availData.flightAvails().end();
    for (; fltAvlI != fltAvlE; fltAvlI++)
    {
      const FlightAvail& fltAvail = *(*fltAvlI);
      diag << "   " << fltAvail.upSellPNRsegNum();
      diag << " AVL METHOD:" << fltAvail.availMethod() << " ";
      diag << fltAvail.bookingCodeAndSeats() << " \n";
    }
  }
  diag << " \n";
  diag << "ITIN PNR SEGMENT NUMBERS : ";
  Itin& itin = *(trx.itin().front());
  std::vector<TravelSeg*>::const_iterator tvlI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();
  for (; tvlI != tvlE; tvlI++)
  {
    const TravelSeg& tvlSeg = *(*tvlI);
    diag << tvlSeg.pnrSegment() << " ";
  }
  diag << " \n";
  diag << "\n****************** END DIAG 195 *************************** \n";
}

void
ContentServices::processDiag998(PricingTrx& trx, const std::string& xmlData)
{
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  if (!trx.diagnostic().isActive())
    return;
  if (!(diagType == Diagnostic998 || diagType == Diagnostic195))
    return;

  std::map<std::string, std::string>::iterator endI = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::iterator beginI =
      trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);

  if (beginI == endI)
    return;

  const std::string& diagParam = (*beginI).second;
  if (diagParam.empty())
    return;

  if (isalpha(diagParam[0]))
    return;

  std::string::size_type index = xmlData.find("<DIA>", 0);
  std::string::size_type dIndex;
  if (index == std::string::npos)
    return;
  dIndex = xmlData.find("<![CDATA[", 0);
  if (dIndex == std::string::npos)
  {
    dIndex = index;
    dIndex += 5;
  }
  else
  {
    dIndex += 9;
  }

  DiagManager diag(trx, diagType);
  if (!diag.isActive())
    return;
  diag << " \n";

  index = xmlData.find("]]></DIA>", 0);
  if (index == std::string::npos)
    index = xmlData.find("</DIA>", 0);
  if (index == std::string::npos)
  {
    diag << "****** BAD DIAG RETURNED FROM AS V2 ****** \n";
  }

  std::string asv2Diag = xmlData.substr(dIndex, index - dIndex + 1);
  std::transform(asv2Diag.begin(), asv2Diag.end(), asv2Diag.begin(), (int (*)(int))toupper);

  diag << asv2Diag << " \n";
}

const Cabin*
ContentServices::getCabin(DataHandle& dataHandle,
                          const CarrierCode& carrier,
                          const BookingCode& classOfService,
                          const DateTime& date)
{
  return dataHandle.getCabin(carrier, classOfService, date);
}

uint16_t
ContentServices::numSeatsNeeded(PricingTrx& trx)
{
  return PaxTypeUtil::totalNumSeats(trx);
}

void
ContentServices::debugOnlyFakeASV2(PricingTrx& trx,
                                   std::vector<FareMarket*>& fareMarkets,
                                   std::string& diagParam)
{
  std::vector<std::string> avails;
  std::string bkg;
  std::string bkgCode;
  std::string bkgResult;
  std::string flag;
  std::string key;
  int segNum;
  size_t pos = std::string::npos;

  std::map<std::string, std::string>::iterator endI = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::iterator beginI = trx.diagnostic().diagParamMap().begin();

  flag.insert(0, diagParam, 6, 1);

  for (const auto fareMarket : fareMarkets)
  {
    for (const auto ts : fareMarket->travelSeg())
    {
      segNum = ts->pnrSegment();
      std::stringstream segNumstr;
      segNumstr << segNum;
      key = "S" + segNumstr.str();
      std::map<std::string, std::string>::const_iterator diagParamIt =
          trx.diagnostic().diagParamMap().find(key);
      for (const auto cos : ts->classOfService())
      {
        bkgCode = cos->bookingCode();

        if (diagParamIt != trx.diagnostic().diagParamMap().end() &&
            (pos = diagParamIt->second.find(bkgCode)) != std::string::npos)
        {
          bkgResult = diagParamIt->second.substr(pos, 2);
        }
        if (bkgCode.compare(0, 1, bkgResult, 0, 1) == 0)
        {
          bkg = bkg + bkgResult + " ";
        }
        else
        {
          bkg = bkg + cos->bookingCode() + flag + " ";
        }
      }
      avails.push_back(bkg);
      bkg.clear();
      bkgResult.clear();
    }
  }

  std::vector<std::vector<BookingCode>> bookingCodes;
  bookingCodes.resize(avails.size());
  std::vector<std::vector<int>> numSeats;
  numSeats.resize(avails.size());
  BookingCode bc;
  std::string seats, bkC;
  for (uint j = 0; j < avails.size(); j++)
  {
    for (uint i = 0; i < avails[j].size();)
    {
      bc = "";
      bc.push_back(avails[j][i]);
      bookingCodes[j].push_back(bc);
      i++;
      seats.resize(0);
      for (uint k = i; k < avails[j].size(); k++)
      {
        i++;
        if (!isdigit(avails[j][k]))
          break;
        seats.push_back(avails[j][k]);
      }
      numSeats[j].push_back(atoi(seats.c_str()));
    }
  }

  std::vector<ClassOfService*>* cosVec = nullptr;
  DataHandle& dataHandle = trx.dataHandle();
  TravelSeg* tvlSeg = nullptr;
  ClassOfService* cs = nullptr;
  bool bcFound = false;
  uint j = 0;
  for (uint i = 0; i < fareMarkets.size() && j < avails.size(); ++i)
  {
    FareMarket& fm = *(fareMarkets[i]);
    fm.classOfServiceVec().clear();
    for (uint k = 0; k < fm.travelSeg().size(); k++)
    {
      dataHandle.get(cosVec);
      if (cosVec == nullptr)
        return;
      fm.classOfServiceVec().push_back(cosVec);
      tvlSeg = fm.travelSeg()[k];
      for (uint iCos = 0; iCos < numSeats[j].size() && iCos < bookingCodes[j].size(); ++iCos)
      {
        cs = getCosFake(trx, bookingCodes[j][iCos], tvlSeg);
        if (cs != nullptr)
        {
          cs->numSeats() = numSeats[j][iCos];
          cosVec->push_back(cs);
        }

        bcFound = false;
        if (fm.travelSeg().size() == 1)
        {
          for (const auto elem : tvlSeg->classOfService())
          {
            ClassOfService& localCos = *elem;
            if (localCos.bookingCode() == bookingCodes[j][iCos])
            {
              localCos.numSeats() = numSeats[j][iCos];
              bcFound = true;
              break;
            }
          }
          if (!bcFound)
          {
            cs = getCosFake(trx, bookingCodes[j][iCos], tvlSeg);
            if (cs != nullptr)
            {
              cs->numSeats() = numSeats[j][iCos];
              tvlSeg->classOfService().push_back(cs);
            }
          }
        }
      }
      j++;
    }
  }
}

void
ContentServices::debugOnlyFakeASV2ForThruFMs(PricingTrx& trx,
                                             std::vector<FareMarket*>& fareMarkets,
                                             std::string& diagParam)
{
  std::map<FareMarket*, std::vector<std::string>> avails;
  std::string bkg;
  std::string bkgDirect;
  std::string bkgCode;
  std::string bkgResult;
  std::string flag;
  std::string key;
  int startSegNum;
  int endSegNum;
  size_t pos = std::string::npos;

  std::map<std::string, std::string>::iterator endI = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::iterator beginI = trx.diagnostic().diagParamMap().begin();

  flag.insert(0, diagParam, 6, 1);

  for (const auto fareMarket : fareMarkets)
  {
    if (fareMarket->travelSeg().front()->pnrSegment() ==
        fareMarket->travelSeg().back()->pnrSegment())
      continue;

    startSegNum = fareMarket->travelSeg().front()->pnrSegment();
    endSegNum = fareMarket->travelSeg().back()->pnrSegment();
    std::stringstream segNumstr;
    segNumstr << startSegNum << endSegNum;
    key = segNumstr.str();

    std::map<std::string, std::string>::const_iterator diagParamIt =
        trx.diagnostic().diagParamMap().find(key);
    if (diagParamIt == trx.diagnostic().diagParamMap().end())
      continue;

    for (const auto ts : fareMarket->travelSeg())
    {
      for (const auto cos : ts->classOfService())
      {
        bkgCode = cos->bookingCode();

        if ((pos = diagParamIt->second.find(bkgCode)) != std::string::npos)
        {
          bkgResult = diagParamIt->second.substr(pos, 2);
          bkg = bkg + bkgResult + " ";
        }
        else
        {
          bkg = bkg + cos->bookingCode() + flag + " ";
        }
      }
      avails[fareMarket].push_back(bkg);
      bkg.clear();
      bkgResult.clear();
    }
  }

  std::map<FareMarket*, std::vector<std::vector<BookingCode>>> bookingCodes;
  std::map<FareMarket*, std::vector<std::vector<int>>> numSeats;
  BookingCode bc;
  std::string seats, bkC;
  std::map<FareMarket*, std::vector<std::string>>::const_iterator availsIt = avails.begin();

  for (; availsIt != avails.end(); ++availsIt)
  {
    bookingCodes[availsIt->first].resize(availsIt->second.size());
    numSeats[availsIt->first].resize(availsIt->second.size());

    for (uint j = 0; j < availsIt->second.size(); j++)
    {
      for (uint i = 0; i < availsIt->second[j].size();)
      {
        bc = "";
        bc.push_back(availsIt->second[j][i]);
        bookingCodes[availsIt->first][j].push_back(bc);
        i++;
        seats.resize(0);
        for (uint k = i; k < availsIt->second[j].size(); k++)
        {
          i++;
          if (!isdigit(availsIt->second[j][k]))
            break;
          seats.push_back(availsIt->second[j][k]);
        }
        numSeats[availsIt->first][j].push_back(atoi(seats.c_str()));
      }
    }
  }

  std::vector<ClassOfService*>* cosVec = nullptr;
  DataHandle& dataHandle = trx.dataHandle();
  TravelSeg* tvlSeg = nullptr;
  ClassOfService* cs = nullptr;
  for (const auto fareMarket : fareMarkets)
  {
    FareMarket& fm = *fareMarket;
    if (numSeats.find(fareMarket) == numSeats.end() ||
        bookingCodes.find(fareMarket) == bookingCodes.end())
      continue;
    fm.classOfServiceVec().clear();
    for (uint k = 0; k < fm.travelSeg().size(); ++k)
    {
      dataHandle.get(cosVec);
      if (cosVec == nullptr)
        return;
      fm.classOfServiceVec().push_back(cosVec);
      tvlSeg = fm.travelSeg()[k];
      for (uint iCos = 0;
           iCos < numSeats[fareMarket][k].size() && iCos < bookingCodes[fareMarket][k].size();
           ++iCos)
      {
        cs = getCosFake(trx, bookingCodes[fareMarket][k][iCos], tvlSeg);
        if (cs != nullptr)
        {
          cs->numSeats() = numSeats[fareMarket][k][iCos];
          cosVec->push_back(cs);
        }
      }
    }
  }
}

ClassOfService*
ContentServices::getCosFake(PricingTrx& trx,
                            const BookingCode& bookingCode,
                            const TravelSeg* travelSeg)
{
  DataHandle& dataHandle = trx.dataHandle();

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
  ClassOfService* cs = nullptr;
  dataHandle.get(cs);
  if (cs == nullptr)
    return nullptr;
  const Cabin* aCabin = nullptr;
  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
  {
    RBDByCabinUtil rbdUtil(trx, CONTENT_SVC_FAKE);
    aCabin = rbdUtil.getCabinByRBD(airSeg->carrier(), bookingCode, *airSeg, false);
  }
  else
  {
    aCabin = dataHandle.getCabin(airSeg->carrier(), bookingCode, airSeg->departureDT());
  }
  if (cs != nullptr)
    cs->bookingCode() = bookingCode;

  if (aCabin == nullptr)
    return nullptr;

  if (cs != nullptr)
    cs->cabin() = aCabin->cabin();
  return cs;
}

void
ContentServices::updateAvailabilityMap(PricingTrx& trx, std::vector<FareMarket*>& fareMarkets) const
{
  for (std::vector<FareMarket*>::const_iterator fmIt = fareMarkets.begin();
       fmIt != fareMarkets.end();
       ++fmIt)
  {
    FareMarket& fm = *(*fmIt);
    std::vector<TravelSeg*> travelSeg;
    for (std::vector<TravelSeg*>::const_iterator tvlSegIt = fm.travelSeg().begin();
         tvlSegIt != fm.travelSeg().end();
         ++tvlSegIt)
    {
      travelSeg.push_back(*tvlSegIt);
      uint64_t avlKey = ShoppingUtil::buildAvlKey(travelSeg);
      AvailabilityMap::iterator foundAvl = trx.availabilityMap().find(avlKey);
      if (foundAvl != trx.availabilityMap().end())
      {
        foundAvl->second->clear();
        CopyAvailabilityMapItem copier(foundAvl->second);
        std::for_each(fm.classOfServiceVec().begin(), fm.classOfServiceVec().end(), copier);
      }
    }
  }
}

void
ContentServices::addASResponseToDiag(const std::string& payload, PricingTrx& trx)
{
  if ((trx.diagnostic().diagnosticType() == Diagnostic195) &&
      (trx.diagnostic().diagParamIsSet("AS", "XML")))
  {
    DiagManager diag(trx, Diagnostic195);
    if (diag.isActive())
    {
      diag << "************************ AS V2 XML ************************\n";
      if (fallback::fallbackDebugOverrideBrandingServiceResponses(&trx))
        diag << xmlAttrEncode(payload) << "\n";
      else
      {
        diag << "AS RESPONSE IN CDATA SECTION\n";
        trx.diagnostic().appendAdditionalDataSection(payload);
      }
      diag << "***********************************************************\n";
    }
  }
}

void
ContentServices::addDSSResponseToDiag(const std::string& payload, PricingTrx& trx)
{
  if ((trx.diagnostic().diagnosticType() == Diagnostic195) &&
      (trx.diagnostic().diagParamIsSet("DS", "SXML")))
  {
    DiagManager diag(trx, Diagnostic195);
    if (diag.isActive())
    {
      diag << "************************ DSS V2 XML ***********************\n";
      if (fallback::fallbackDebugOverrideBrandingServiceResponses(&trx))
        diag << xmlAttrEncode(payload) << "\n";
      else
      {
        trx.diagnostic().appendAdditionalDataSection(payload);
        diag << " DSS RESPONSE IN CDATA SECTION\n";
      }
      diag << "***********************************************************\n";
    }
  }
}

} // tse
