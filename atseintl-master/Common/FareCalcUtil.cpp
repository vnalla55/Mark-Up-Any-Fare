//----------------------------------------------------------------------------
//
//  File:           FareCalcUtil.cpp
//  Created:        10/5/2004
//  Authors:
//
//  Description:    Common functions required for ATSE shopping/pricing.
//
//  Copyright Sabre 2004
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
#include "Common/FareCalcUtil.h"

#include "Common/Assert.h"
#include "Common/ErrorResponseException.h"
#include "Common/FcConfig.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/FareCalcCollector.h"

#include <sstream>

namespace tse
{

static Logger
logger("atseintl.Common.FareCalcUtil");

namespace FareCalcUtil
{
const FareCalcConfig*
getFareCalcConfig(PricingTrx& trx)
{
  FareCalcConfig* fcConfig = trx.fareCalcConfig();
  if (fcConfig == nullptr)
  {
    fcConfig = getFareCalcConfigForAgent(trx,
                                         *trx.getRequest()->ticketingAgent(),
                                         trx.dataHandle(),
                                         trx.getOptions()->isFareCalculationDisplay());
    trx.fareCalcConfig() = fcConfig;
  }

  return fcConfig;
}

const FareCalcConfig*
getFareCalcConfig(PricingDetailTrx& trx)
{
  return getFareCalcConfigForAgent(trx, trx.ticketingAgent(), trx.dataHandle(), false);
}

FareCalcConfig*
getFareCalcConfigForAgent(const Trx& trx,
                                        const Agent& ticketingAgent,
                                        DataHandle& dataHandle,
                                        const bool isFareCalculationDisplay)
{
  Indicator userApplType = CRS_USER_APPL;
  std::string userAppl;

  getUserAppl(trx, ticketingAgent, userApplType, userAppl);

  // Determine user pseudo city code
  std::string userPCC = ticketingAgent.tvlAgencyPCC();
  if (isFareCalculationDisplay)
  {
    userApplType = NO_PARAM;
    userAppl = "";
    userPCC = "";
  }
  else if (userPCC.empty())
  {
    userPCC = ticketingAgent.mainTvlAgencyPCC();
  }

  // Get FareCalcConfig records
  const std::vector<FareCalcConfig*>& fareCalcConfigList =
      dataHandle.getFareCalcConfig(userApplType, userAppl, userPCC);
  if (fareCalcConfigList.empty())
  {
    LOG4CXX_FATAL(logger, "Unable to retrieve Fare Calc Config");
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED);
  }

  std::vector<FareCalcConfig*>::const_iterator iter = fareCalcConfigList.begin();
  std::vector<FareCalcConfig*>::const_iterator iterEnd = fareCalcConfigList.end();
  const Loc* loc = dataHandle.getLoc(ticketingAgent.agentCity(), time(nullptr));

  if (loc == nullptr)
  {
    return nullptr;
    // LOG4CXX_FATAL(logger, "Unable to retrieve Fare Calc Config - Agent city - "
    //              << ticketingAgent.agentCity());
    // throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED);
  }

  // Select the right FareCalcConfig record using the last 2 keys
  for (; iter != iterEnd; iter++)
  {
    if (((*iter)->loc1().loc().empty() && (*iter)->loc1().locType() == ' ') ||
        (LocUtil::isInLoc(
            *loc, (*iter)->loc1().locType(), (*iter)->loc1().loc(), SABRE_USER, MANUAL)))
    {
      return (*iter);
    }
  }

  LOG4CXX_FATAL(logger, "Unable to retrieve Fare Calc Config");
  throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED);
}

void
doubleToStringTruncate(double doubleNum, std::string& strNum, unsigned int noDec)
{
  std::ostringstream stringStream;

  stringStream.setf(std::ios::fixed, std::ios::floatfield);
  stringStream.setf(std::ios::right, std::ios::adjustfield);
  stringStream.precision(noDec + 1);
  stringStream << doubleNum;
  std::string tempStr = stringStream.str();
  const size_t len = tempStr.size() - (noDec == 0 ? 2 : 1);
  strNum = tempStr.substr(0, len);
}

bool
getMsgAppl(FareCalcConfigText::TextAppl appl,
                         std::string& msg,
                         PricingTrx& pricingTrx)
{
  const FareCalcConfig* fcConfig = getFareCalcConfig(pricingTrx);
  if (fcConfig)
    return getMsgAppl(appl, msg, pricingTrx, *fcConfig);
  return false;
}

bool
getMsgAppl(FareCalcConfigText::TextAppl appl,
                         std::string& msg,
                         PricingTrx& pricingTrx,
                         const FareCalcConfig& fcConfig)
{
  const FcConfig* fc = FcConfig::create(&pricingTrx, &fcConfig);
  if (fc)
    return fc->getMsgAppl(appl, msg);
  return false;
}

int
getPtcRefNo(const PricingTrx& trx, const PaxType* paxType)
{
#if 0
    // TODO: this is the code to use when inputOrder problem is fixed.
    TSE_ASSERT(paxType != 0)
    return (paxType->inputOrder() + 1);
#else
  std::vector<PaxType*>::const_iterator i;
  i = std::find(trx.paxType().begin(), trx.paxType().end(), paxType);
  int paxTypeNbr = 1;
  if (i != trx.paxType().end())
  {
    paxTypeNbr += int(std::distance(trx.paxType().begin(), i));
  }
  return paxTypeNbr;
#endif
}

void
getUserAppl(const Trx& trx,
                          const Agent& agent,
                          Indicator& userApplType,
                          std::string& userAppl)
{
  userApplType = CRS_USER_APPL;
  std::string csr = agent.vendorCrsCode();

  if (UNLIKELY(agent.tvlAgencyPCC().empty() && agent.hostCarrier() == "AA" &&
      (csr.empty() || !(csr == INFINI_MULTIHOST_ID || csr == ABACUS_MULTIHOST_ID))))
  {
    userApplType = MULTIHOST_USER_APPL;
    userAppl = agent.hostCarrier();
    return;
  }

  if (csr.empty())
  {
    if (LIKELY(agent.hostCarrier().empty() || agent.hostCarrier() == "AA"))
    {
      csr = agent.cxrCode();
    }
    else
    {
      userApplType = MULTIHOST_USER_APPL;
      userAppl = agent.hostCarrier();
      return;
    }
  }

  // Determine user application
  if (UNLIKELY(csr == AXESS_MULTIHOST_ID))
  {
    userAppl = AXESS_USER;
  }
  else if (csr == ABACUS_MULTIHOST_ID)
  {
    userAppl = ABACUS_USER;
  }
  else if (csr == INFINI_MULTIHOST_ID)
  {
    userAppl = INFINI_USER;
  }
  else if (csr == SABRE_MULTIHOST_ID)
  {
    userAppl = SABRE_USER;
  }
}

bool
isSameDisplayLoc(const FareCalcConfig& fcConfig,
                               const LocCode& obAirport,
                               const LocCode& obMultiCity,
                               const LocCode& ibAirport,
                               const LocCode& ibMultiCity)
{
  LocCode obDisplayLoc = getDisplayLoc(fcConfig, obAirport, obMultiCity);
  LocCode ibDisplayLoc = getDisplayLoc(fcConfig, ibAirport, ibMultiCity);
  if (obDisplayLoc == ibDisplayLoc)
  {
    return true;
  }
  return false;
}

LocCode
getDisplayLoc(const FareCalcConfig& fcConfig,
                            const LocCode& airport,
                            const LocCode& multiCity)
{
  LocCode displayLoc;
  if (!getFccDisplayLoc(fcConfig, airport, displayLoc) || displayLoc.empty())
  {
    displayLoc = multiCity;
  }
  return displayLoc;
}

bool
getFccDisplayLoc(const FareCalcConfig& fcConfig,
                               const LocCode& loc,
                               LocCode& displayLoc)
{
  const std::vector<FareCalcConfigSeg*>& fcSeg = fcConfig.segs();

  for (const auto& elem : fcSeg)
  {
    if (elem->marketLoc() == loc)
    {
      displayLoc = elem->displayLoc();
      return true;
    }
  }
  return false;
}

FareCalcCollector*
getFareCalcCollectorForItin(PricingTrx& trx, const Itin* itin)
{
  std::map<const Itin*, FareCalcCollector*>::iterator fareCalcCollectorIter =
      trx.fareCalcCollectorMap().find(itin);

  if (fareCalcCollectorIter == trx.fareCalcCollectorMap().end())
  {
    return nullptr;
  }
  else
  {
    return fareCalcCollectorIter->second;
  }
}

std::string
formatExchangeRate(const ExchRate exchangeRate,
                                 const CurrencyNoDec rateNoDec)
{
  std::string zeroStr("0");
  std::ostringstream exchangeRateStr;
  std::string rateStr;

  exchangeRateStr.precision(rateNoDec);
  exchangeRateStr << std::showpoint << std::fixed << exchangeRate;
  std::string tmpRateStr = exchangeRateStr.str();
  std::string::size_type decimalIdx = tmpRateStr.find(".");
  std::string integralPart = tmpRateStr.substr(0, decimalIdx);

  std::string fractionalPart;

  if (integralPart == "0")
    fractionalPart = tmpRateStr.substr((decimalIdx + 1), tmpRateStr.size());
  else
    fractionalPart = tmpRateStr.substr((decimalIdx + 1), rateNoDec);

  if (integralPart == "0")
  {
    rateStr = "0." + fractionalPart;
    // Remove Trailing Zeroes
    std::string::size_type idx = rateStr.find_last_not_of(zeroStr);
    if (idx != std::string::npos)
      rateStr.erase((idx + 1), rateStr.size());
  }
  else if ((exchangeRate - 1.000000) < EPSILON)
    rateStr = integralPart + "." + "00000";
  else
  {
    if (rateNoDec > 0)
      rateStr = integralPart + "." + fractionalPart;
    else if (fractionalPart.empty())
      rateStr = integralPart;
  }

  // truncate Exchange Rate
  if (rateStr.size() > 15)
  {
    int numCharsToErase = rateStr.size() - 15;
    rateStr.erase((15), numCharsToErase);
  }

  return rateStr;
}

bool
isOneSolutionPerPaxType(const PricingTrx* trx)
{
  if (trx->fareCalcCollector().empty())
    return false;

  FareCalcCollector* fareCalcCollector = trx->fareCalcCollector().front();
  FlatSet<PaxTypeCode> uniquePax;

  for (auto const& p : fareCalcCollector->calcTotalsMap())
  {
    const CalcTotals *ct = p.second;
    if (ct != nullptr && ct->farePath != nullptr && ct->farePath->processed())
    {
      if (!uniquePax.insert(ct->requestedPaxType).second)
        return false;
    }
  }

  return trx->paxType().size() == uniquePax.size();
}

bool isOnlyOneMatchSolution(const PricingTrx* trx)
{
  if (!isOneSolutionPerPaxType(trx) || trx->fareCalcCollector().empty() ||
      trx->fareCalcCollector().front()->calcTotalsMap().empty())
    return false;

  const FarePath* fp = trx->fareCalcCollector().front()->calcTotalsMap().begin()->first;

  return fp && !fp->noMatchOption();
}

} // namespace FareCalcUtil
} // namespace tse
