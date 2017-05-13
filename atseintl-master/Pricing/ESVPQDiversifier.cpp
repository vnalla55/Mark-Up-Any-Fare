//-------------------------------------------------------------------
//
//  File:        ESVDiversityUtil.cpp
//  Created:     Jan 27, 2008
//  Authors:
//
//  Description: Class managing ESV Diversity logic
//
//  Updates:
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Pricing/ESVPQDiversifier.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "Pricing/EstimatedSeatValue.h"
#include "Pricing/ESVPQ.h"
#include "Pricing/ESVPQItem.h"

namespace tse
{
static Logger
logger("atseintl.ESVPQDiversifier");

namespace
{
ConfigurableValue<ConfigVector<CarrierCode>>
esvOnlineOnlyCarriers("SHOPPING_OPT", "ESV_ONLINE_ONLY_CARRIERS");
ConfigurableValue<ConfigVector<CarrierCode>>
esvRestrictedCarriers("SHOPPING_OPT", "ESV_RESTRICTED_CARRIERS");
}

void
ESVPQDiversifier::init(ShoppingTrx* trx)
{
  _shoppingTrx = trx;
  _esvOptions = _shoppingTrx->esvOptions();

  if (_esvOptions == nullptr)
  {
    LOG4CXX_ERROR(logger, "ESVPQDiversifier::ESVPQDiversifier: ESVOptions must not be NULL");
    _esvOptions = _shoppingTrx->dataHandle().create<ESVOptions>();
  }
  initAVSCarriers();
  initFlightMustPriceAndSopsPerCxrMaps();
  initCarrierMustPriceMaps();
  initFlightLFSMaps();
  initCarrierLFSMaps();
  initInterlineRestrictedCxr();
}

void
ESVPQDiversifier::initInterlineRestrictedCxr()
{
  // init online only list
  const auto esvOnlineCarriers = esvOnlineOnlyCarriers.getValue();
  _onlineOnlyCxr.insert(_onlineOnlyCxr.end(), esvOnlineCarriers.begin(), esvOnlineCarriers.end());
  // init restricted carriers list
  CarrierCode mainCxr;
  std::vector<CarrierCode> restrictedVec;
  bool nextCxr = false;
  for (const auto carrier : esvRestrictedCarriers.getValue())
  {
    if (carrier == "*")
    {
      nextCxr = true;
      if (!mainCxr.empty())
      {
        std::pair<CarrierCode, std::vector<CarrierCode>> p(mainCxr, restrictedVec);
        _restrictedCxr.push_back(p);
        restrictedVec.clear();
      }
    }
    else
    {
      if (nextCxr)
      {
        mainCxr = carrier;
        nextCxr = false;
      }
      else
      {
        restrictedVec.push_back(carrier);
      }
    }
  }
}

void
ESVPQDiversifier::initAVSCarriers()
{
  boost::char_separator<char> separator(",");
  boost::tokenizer<boost::char_separator<char>> tokenizer(esvOptions()->avsCarriersString(),
                                                          separator);
  boost::tokenizer<boost::char_separator<char>>::iterator token = tokenizer.begin();

  _avsCarriersVec.clear();
  for (token = tokenizer.begin(); token != tokenizer.end(); ++token)
  {
    CarrierCode cxr = static_cast<CarrierCode>(token->data());
    _avsCarriersVec.push_back(cxr);
  }
}

bool
ESVPQDiversifier::checkRestrictedCarriers(ESVPQItem* pqItem, std::string& addInfo) const
{
  AirSeg* airSeg = dynamic_cast<AirSeg*>(pqItem->outSopWrapper()->sop()->itin()->travelSeg().at(0));

  if (airSeg == nullptr)
  {
    addInfo = "DIV6";
    return false;
  }

  CarrierCode& validatingCarrier = airSeg->carrier();

  std::set<CarrierCode> cxrList;

  addCarriers(cxrList, pqItem->outSopWrapper()->paxTypeFareVec());

  if (pqItem->inSopWrapper() != nullptr)
  {
    addCarriers(cxrList, pqItem->inSopWrapper()->paxTypeFareVec());
  }

  if (_shoppingTrx->getOptions()->validateTicketingAgreement())
  {
    if (false ==
        EstimatedSeatValue::checkCarrier(
            *_shoppingTrx, validatingCarrier, pqItem->pq()->interlineTicketCarrier(), cxrList))
    {
      addInfo = "DIV6";
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    bool result = true;
    std::set<CarrierCode>::iterator curCxrIter = cxrList.begin();
    std::set<CarrierCode>::iterator cxrIterEnd = cxrList.end();
    while (result && (curCxrIter != cxrIterEnd))
    {
      std::set<CarrierCode>::iterator cxrIter = cxrList.begin();
      while (result && (cxrIter != cxrIterEnd))
      {
        if (*curCxrIter != *cxrIter)
        {
          if (isOnlineOnlyRestricted(*curCxrIter)) // checking online only restrictions
          {
            result = false;
            addInfo = "DIV5";
          }
          else // checking restricted carriers
          {
            if (!(cxrRestrictionCorrect(*curCxrIter, *cxrIter)))
            {
              result = false;
              addInfo = "DIV6";
            }
          }
        }

        if (result)
        {
          ++cxrIter;
        }
      }

      if (result)
      {
        ++curCxrIter;
      }
    }

    return result;
  }
}

bool
ESVPQDiversifier::cxrRestrictionCorrect(CarrierCode mainCxr, CarrierCode cxr) const
{
  bool result = true;
  std::vector<std::pair<CarrierCode, std::vector<CarrierCode>>>::const_iterator cxrIter =
      _restrictedCxr.begin();
  std::vector<std::pair<CarrierCode, std::vector<CarrierCode>>>::const_iterator cxrIterEnd =
      _restrictedCxr.end();
  // looking for main carrier in the list
  bool found = false;
  while (!found && (cxrIter != cxrIterEnd))
  {
    if ((*cxrIter).first == mainCxr)
    {
      found = true;
    }
    else
    {
      ++cxrIter;
    }
  }

  if (found) // main carrier is restricted
  {
    // loking for carrier in the supported list
    std::vector<CarrierCode>::const_iterator cxrListIter = (*cxrIter).second.begin();
    std::vector<CarrierCode>::const_iterator cxrListIterEnd = (*cxrIter).second.end();
    bool foundInList = false;
    while (!foundInList && (cxrListIter != cxrListIterEnd))
    {
      if (*cxrListIter == cxr)
      {
        foundInList = true;
      }
      else
      {
        ++cxrListIter;
      }
    }

    if (!foundInList)
    {
      result = false;
    }
  }

  return result;
}

void
ESVPQDiversifier::addCarriers(std::set<CarrierCode>& cxrList,
                              const std::vector<PaxTypeFare*>* const paxTypeFareVec) const
{
  std::vector<PaxTypeFare*>::const_iterator fareIter = paxTypeFareVec->begin();
  std::vector<PaxTypeFare*>::const_iterator fareIterEnd = paxTypeFareVec->end();
  for (; fareIter != fareIterEnd; ++fareIter)
  {
    cxrList.insert((*fareIter)->fareMarket()->governingCarrier());
  }
}

bool
ESVPQDiversifier::isOnlineOnlyRestricted(const CarrierCode cxr) const
{
  bool found = false;
  std::vector<CarrierCode>::const_iterator iter = _onlineOnlyCxr.begin();
  std::vector<CarrierCode>::const_iterator iterEnd = _onlineOnlyCxr.end();
  while (!found && (iter != iterEnd))
  {
    if (*iter == cxr)
    {
      found = true;
    }
    else
    {
      ++iter;
    }
  }

  return found;
}

void
ESVPQDiversifier::insertToFlightMap(ShoppingTrx::SchedulingOption* sop,
                                    int count,
                                    FlightMap& flightMap)
{
  FlightMap::iterator it = flightMap.find(sop);

  if (it == flightMap.end())
  {
    flightMap.insert(std::pair<ShoppingTrx::SchedulingOption*, int>(sop, count));
  }
  else
  {
    flightMap[sop] = count;
  }
}

// Check if the itin countdown in flightMap didn't reach zero
bool
ESVPQDiversifier::checkFlightLimit(ShoppingTrx::SchedulingOption* sop, FlightMap& flightMap)
{
  if (sop == nullptr)
  {
    LOG4CXX_ERROR(
        logger,
        "ESVPQDiversifier::checkFlightLimit: There is no Scheduling option to check flight limits");
    return false;
  }

  FlightMap::iterator it = flightMap.find(sop);
  if (it == flightMap.end())
  {
    LOG4CXX_ERROR(
        logger, "ESVPQDiversifier::checkFlightLimit: Scheduling Options not present in Flight Map");
    return false;
  }

  return (flightMap[sop] > 0);
}

void
ESVPQDiversifier::insertToCarrierMap(CarrierCode carrier, int count, CarrierMap& carrierMap)
{
  CarrierMap::iterator it = carrierMap.find(carrier);

  if (it == carrierMap.end())
  {
    carrierMap.insert(std::pair<CarrierCode, int>(carrier, count));
  }
  else
  {
    carrierMap[carrier] = count;
  }
}

// Checks whether carrier countdown in specified carrierMap didn't reach 0
bool
ESVPQDiversifier::checkCarrierLimit(CarrierCode carrier, CarrierMap& carrierMap)
{
  CarrierMap::iterator it = carrierMap.find(carrier);

  if (it == carrierMap.end())
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::checkCarrierLimit: Itinerary not present in Carrier Map");
    return false;
  }

  return (carrierMap[carrier] > 0);
}

// Checks whether carrier count in carrierCountMap didn't reach the value of specifier
// carrierLimitMap
bool
ESVPQDiversifier::checkCarrierLimit(CarrierCode carrier,
                                    CarrierMap& carrierCountMap,
                                    CarrierMap& carrierLimitMap)
{
  CarrierMap::iterator itCount = carrierCountMap.find(carrier);
  CarrierMap::iterator itLimit = carrierLimitMap.find(carrier);

  if ((itCount == carrierCountMap.end()) || (itLimit == carrierLimitMap.end()))
  {
    LOG4CXX_ERROR(logger,
                  "bool ESVPQDiversifier::checkCarrierLimit: Carrier not present in Carrier Map");
    return false;
  }

  return (carrierCountMap[carrier] < carrierLimitMap[carrier]);
}

//-------------------------------------------------------------------------------------
// Flight Option Reuse Limit for Must Price Q69
//-------------------------------------------------------------------------------------

void
ESVPQDiversifier::initFlightMustPriceAndSopsPerCxrMaps()
{
  std::vector<ShoppingTrx::Leg>::iterator legIter = _shoppingTrx->legs().begin();
  std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter;

  _totalOutbound = 0;
  _totalInbound = 0;

  if (legIter == _shoppingTrx->legs().end())
  {
    return;
  }

  //  for each outbound itinerary
  for (sopIter = legIter->sop().begin(); sopIter != legIter->sop().end(); sopIter++)
  {
    CarrierCode outCarrier = sopIter->governingCarrier();
    if (sopIter->getDummy())
    {
      continue;
    }

    if (_outSopsPerCxrMap.find(outCarrier) == _outSopsPerCxrMap.end())
    {
      _outSopsPerCxrMap.insert(std::pair<CarrierCode, int>(outCarrier, 1));
    }
    else
    {
      _outSopsPerCxrMap[outCarrier]++;
    }
    insertToFlightMap(
        &(*sopIter), (int)_esvOptions->flightOptionReuseLimit(), _outFlightCountdownMPMap);
    _totalOutbound++;
  }

  legIter++;

  if (legIter != _shoppingTrx->legs().end())
  {
    //  for each inbound itinerary do
    for (sopIter = legIter->sop().begin(); sopIter != legIter->sop().end(); sopIter++)
    {
      CarrierCode inCarrier = sopIter->governingCarrier();

      if (sopIter->getDummy())
      {
        continue;
      }

      if (_inSopsPerCxrMap.find(inCarrier) == _inSopsPerCxrMap.end())
      {
        _inSopsPerCxrMap.insert(std::pair<CarrierCode, int>(inCarrier, 1));
      }
      else
      {
        _inSopsPerCxrMap[inCarrier]++;
      }

      insertToFlightMap(
          &(*sopIter), (int)_esvOptions->flightOptionReuseLimit(), _inFlightCountdownMPMap);
      _totalInbound++;
    }
  }
}
// sopVec must have size greater then 0
bool
ESVPQDiversifier::checkItinFlightsMustPriceLimits(
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec)
{
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  bool result = checkFlightLimit(*sopIt, _outFlightCountdownMPMap);

  sopIt++;
  if ((sopIt != sopVec.end()) && (result == true))
  {
    result = checkFlightLimit(*sopIt, _inFlightCountdownMPMap);
  }

  return result;
}
// sopVec must have size greater then 0
void
ESVPQDiversifier::decreaseItinFlightsMustPriceCd(
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec)
{
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  _outFlightCountdownMPMap[*sopIt]--;
  sopIt++;

  if (sopIt != sopVec.end())
  {
    _inFlightCountdownMPMap[*sopIt]--;
  }
}

//-------------------------------------------------------------------------------------
// Max Must Price allowed overage per carrier Q68
//-------------------------------------------------------------------------------------
void
ESVPQDiversifier::initCarrierMustPriceMaps()
{
  CarrierMap::iterator it;

  // initializing for inbound carriers in the beginning, for some of them it may be recalculated in
  // the next loop
  for (it = _inSopsPerCxrMap.begin(); it != _inSopsPerCxrMap.end(); it++)
  {
    CarrierCode carrier = it->first;

    int onlineMax = _esvOptions->noOfMustPriceOnlineSolutions();
    int interlineMax = _esvOptions->noOfMustPriceInterlineSolutions();

    // calculate and set maximum number of on/interline and out/inbound allowed itineraries per
    // carrier
    float percent = _esvOptions->percentFactor();

    if (percent < 1)
    {
      onlineMax = static_cast<int>(_esvOptions->noOfMustPriceOnlineSolutions() * percent);

      interlineMax = static_cast<int>(_esvOptions->noOfMustPriceInterlineSolutions() * percent);
    }

    insertToCarrierMap(carrier, onlineMax, _inOnlineCxrCountdownMPMap);
    insertToCarrierMap(carrier, interlineMax, _inInterlineCxrCountdownMPMap);
  }

  for (it = _outSopsPerCxrMap.begin(); it != _outSopsPerCxrMap.end(); it++)
  {
    CarrierCode carrier = it->first;

    int onlineMax = _esvOptions->noOfMustPriceOnlineSolutions();
    int interlineMax = _esvOptions->noOfMustPriceInterlineSolutions();

    // calculate and set maximum number of on/interline and out/inbound allowed itineraries per
    // carrier
    float percent = _esvOptions->percentFactor() + ((float)it->second) / _totalOutbound;

    if (percent < 1)
    {
      onlineMax = static_cast<int>(_esvOptions->noOfMustPriceOnlineSolutions() * percent);

      interlineMax = static_cast<int>(_esvOptions->noOfMustPriceInterlineSolutions() * percent);
    }

    insertToCarrierMap(carrier, onlineMax, _inOnlineCxrCountdownMPMap);
    insertToCarrierMap(carrier, onlineMax, _outOnlineCxrCountdownMPMap);

    insertToCarrierMap(carrier, interlineMax, _inInterlineCxrCountdownMPMap);
    insertToCarrierMap(carrier, interlineMax, _outInterlineCxrCountdownMPMap);
  }
}

// sopVec must have size greater then 0
bool
ESVPQDiversifier::checkItinCarriersMustPriceLimits(
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec, bool onlineFlag)
{
  bool result;
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  if ((*sopIt) == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::checkItinCarriersMustPriceLimits: Scheduling Option is NULL");
    return false;
  }

  if (onlineFlag)
  {
    result = checkCarrierLimit((*sopIt)->governingCarrier(), _outOnlineCxrCountdownMPMap);
    sopIt++;

    if ((result) && (sopIt != sopVec.end()) && ((*sopIt) != nullptr))
    {
      result = checkCarrierLimit((*sopIt)->governingCarrier(), _inOnlineCxrCountdownMPMap);
    }
  }
  else
  {
    result = checkCarrierLimit((*sopIt)->governingCarrier(), _outInterlineCxrCountdownMPMap);
    sopIt++;

    if ((result) && (sopIt != sopVec.end()) && ((*sopIt) != nullptr))
    {
      result = checkCarrierLimit((*sopIt)->governingCarrier(), _inInterlineCxrCountdownMPMap);
    }
  }
  return result;
}

// sopVec must have size greater then 0
void
ESVPQDiversifier::decreaseItinCarriersMustPriceCd(
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec, bool onlineFlag)
{
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  if ((*sopIt) == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::checkItinCarriersMustPriceLimits: Scheduling Option is NULL");
    return;
  }

  if (onlineFlag)
  {
    _outOnlineCxrCountdownMPMap[(*sopIt)->governingCarrier()]--;
    sopIt++;

    if ((sopIt != sopVec.end()) && ((*sopIt) != nullptr))
    {
      _inOnlineCxrCountdownMPMap[(*sopIt)->governingCarrier()]--;
    }
  }
  else
  {
    _outInterlineCxrCountdownMPMap[(*sopIt)->governingCarrier()]--;
    sopIt++;

    if ((sopIt != sopVec.end()) && ((*sopIt) != nullptr))
    {
      _inInterlineCxrCountdownMPMap[(*sopIt)->governingCarrier()]--;
    }
  }
}
//-------------------------------------------------------------------------------------
// Flight Option Reuse Limit for LFS Q5X and Q5W
//-------------------------------------------------------------------------------------

void
ESVPQDiversifier::initFlightLFSMaps()
{
  std::map<ShoppingTrx::SchedulingOption*, int>::iterator iter;
  ShoppingTrx::SchedulingOption* sop;

  // for every itinerary do (inbound and outbound)
  for (iter = _inFlightCountdownMPMap.begin(); iter != _inFlightCountdownMPMap.end(); iter++)
  {
    sop = (*iter).first;

    if (isAVSCarrier(sop->governingCarrier()))
    {
      insertToFlightMap(sop, _esvOptions->loMaximumPerOption(), _flightCountdownLFSMap);
    }
    else
    {
      insertToFlightMap(sop, _esvOptions->hiMaximumPerOption(), _flightCountdownLFSMap);
    }
  }

  for (iter = _outFlightCountdownMPMap.begin(); iter != _outFlightCountdownMPMap.end(); iter++)
  {
    sop = (*iter).first;

    if (isAVSCarrier(sop->governingCarrier()))
    {
      insertToFlightMap(sop, _esvOptions->loMaximumPerOption(), _flightCountdownLFSMap);
    }
    else
    {
      insertToFlightMap(sop, _esvOptions->hiMaximumPerOption(), _flightCountdownLFSMap);
    }
  }
}

// sopVec must have size greater then 0
bool
ESVPQDiversifier::checkItinFlightsLFSLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec)
{
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  bool result = checkFlightLimit(*sopIt, _flightCountdownLFSMap);
  sopIt++;

  if ((sopIt != sopVec.end()) && (result == true))
  {
    result = checkFlightLimit(*sopIt, _flightCountdownLFSMap);
  }

  return result;
}

// sopVec must have size greater then 0
void
ESVPQDiversifier::decreaseItinFlightsLFSCd(std::vector<ShoppingTrx::SchedulingOption*>& sopVec)
{
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  _flightCountdownLFSMap[*sopIt]--;
  sopIt++;

  if (sopIt != sopVec.end())
  {
    _flightCountdownLFSMap[*sopIt]--;
  }
}

//------------------------------------------------------------------------------------------------------
// Max allowed overage per Carrier for LFS :Q5T
// Target min number of online solutions per carrier Q5U
// Target min percent of total online solutions Q5V
//------------------------------------------------------------------------------------------------------
void
ESVPQDiversifier::initCarrierLFSMaps()
{
  int maxItin = _esvOptions->getRequestedNumberOfSolutions();

  std::map<CarrierCode, int>::const_iterator it;

  // for each carrier do
  for (it = _outSopsPerCxrMap.begin(); it != _outSopsPerCxrMap.end(); it++)
  {
    CarrierCode outCarrier = (*it).first;

    // OnlineCarrier LFS Maps (Q5U, Q5V)
    insertToCarrierMap(outCarrier, _esvOptions->noOfMinOnlinePerCarrier(), _onlineCxrLimitLFSMap);
    insertToCarrierMap(outCarrier, 0, _onlineCxrCountMapLFS);

    // calculate and set maximum number of  outbound allowed itineraries per carrier - Carrier LFS
    // Map (Q5T)
    float outboundPercent = ((float)_esvOptions->esvPercent()) / 100 +
                            ((float)_outSopsPerCxrMap[outCarrier]) / _totalOutbound;
    int outboundMax = static_cast<int>(outboundPercent * maxItin);

    insertToCarrierMap(outCarrier, outboundMax, _outCxrCountdownLFSMap);
  }

  for (it = _inSopsPerCxrMap.begin(); it != _inSopsPerCxrMap.end(); it++)
  {
    CarrierCode inCarrier = (*it).first;

    // calculate and set maximum number of  inbound allowed itineraries per carrier - Carrier LFS
    // Map (Q5T)
    float inboundPercent = ((float)_esvOptions->esvPercent()) / 100 +
                           ((float)_inSopsPerCxrMap[inCarrier]) / _totalInbound;
    int inboundMax = static_cast<int>(inboundPercent * maxItin);

    insertToCarrierMap(inCarrier, inboundMax, _inCxrCountdownLFSMap);
  }
}

// sopVec must have size greater then 0
bool
ESVPQDiversifier::checkItinCarriersLFSLimits(std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                             CarrierCode& carrier)
{
  bool result;
  carrier = "";
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  if (*sopIt == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::checkItinCarriersLFSLimits: Scheduling Option is NULL");
    return false;
  }

  result = checkCarrierLimit((*sopIt)->governingCarrier(), _outCxrCountdownLFSMap);

  if (!result)
  {
    carrier = (*sopIt)->governingCarrier();
  }
  sopIt++;

  if ((result) && (sopIt != sopVec.end()) && (*sopIt != nullptr))
  {
    result = checkCarrierLimit((*sopIt)->governingCarrier(), _inCxrCountdownLFSMap);
    if (!result)
    {
      carrier = (*sopIt)->governingCarrier();
    }
  }

  return result;
}

// sopVec must have size greater then 0
void
ESVPQDiversifier::decreaseItinCarriersLFSCd(std::vector<ShoppingTrx::SchedulingOption*>& sopVec)
{
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  if (*sopIt == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::checkItinCarriersLFSLimits: Scheduling Option is NULL");
    return;
  }

  _outCxrCountdownLFSMap[(*sopIt)->governingCarrier()]--;
  sopIt++;

  if ((sopIt != sopVec.end()) && (*sopIt != nullptr))
  {
    _inCxrCountdownLFSMap[(*sopIt)->governingCarrier()]--;
  }
}

// Recalculate online Carrier Limit value based on target min percentage of total online solutions
// Q5V
void
ESVPQDiversifier::recalcCarrierOnlineLimitLFSMap()
{
  int onlineMaximum = static_cast<int>(_esvOptions->getRequestedNumberOfSolutions() *
                                       _esvOptions->onlinePercent() / 100);

  std::map<CarrierCode, int>::const_iterator it;
  // for each carrier in oubout cnt map do

  for (it = _outSopsPerCxrMap.begin(); it != _outSopsPerCxrMap.end(); it++)
  {
    CarrierCode outCarrier = (*it).first;

    double percentage = (double)((*it).second) / (double)_totalOutbound;

    insertToCarrierMap(
        outCarrier, static_cast<int>(percentage * onlineMaximum), _onlineCxrLimitLFSMap);
  }
}

// sopVec must have size greater then 0
bool
ESVPQDiversifier::checkItinCarriersOnlineLFSLimits(
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec)
{
  bool result;
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  if (*sopIt == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::checkItinCarriersOnlineLFSLimits: Scheduling Option is NULL");
    return false;
  }

  result =
      checkCarrierLimit((*sopIt)->governingCarrier(), _onlineCxrCountMapLFS, _onlineCxrLimitLFSMap);

  return result;
}

// sopVec must have size greater then 0
void
ESVPQDiversifier::increaseItinOnlineCarriersLFSC(
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec)
{
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIt = sopVec.begin();

  if (*sopIt == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::checkItinCarriersOnlineLFSLimits: There is no "
                  "outbound Scheduling Option or it is NULL");
    return;
  }

  _onlineCxrCountMapLFS[(*sopIt)->governingCarrier()]++;
}

// Processing of Q68 and Q69
bool
ESVPQDiversifier::processItinMustPriceFlightsAndCarriersLimits(
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec, int onlineInterlineInd)
{
  bool result;

  if (_shoppingTrx != nullptr)
  {
    TSELatencyData metrics(*_shoppingTrx, "ESVPQDIVERSIFIER PROCESS MUST PRICE");
  }

  if (sopVec.size() == 0)
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::processItinMustPriceFlightsAndCarriersLimits: No "
                  "Scheduling options in sopVec");
    return false;
  }

  result = checkItinFlightsMustPriceLimits(sopVec);

  if (result)
  {
    result = checkItinCarriersMustPriceLimits(sopVec, onlineInterlineInd);
  }

  if (result)
  {
    decreaseItinFlightsMustPriceCd(sopVec);
    decreaseItinCarriersMustPriceCd(sopVec, onlineInterlineInd);
  }
  return result;
}

// Processing of Q6A and Q6S
MoneyAmount
ESVPQDiversifier::calcMustPriceUpperBound(MoneyAmount minFareValue, bool isNonstopFlag)
{
  MoneyAmount upperBoundFactor;

  if (isNonstopFlag)
  {
    upperBoundFactor = _esvOptions->upperBoundFactorForNonstop();
  }
  else
  {
    upperBoundFactor = _esvOptions->upperBoundFactorForNotNonstop();
  }

  return minFareValue * upperBoundFactor;
}

// Processing of Q6U
MoneyAmount
ESVPQDiversifier::calcLFSUpperBound(MoneyAmount minFareValue)
{
  return minFareValue * _esvOptions->upperBoundFactorForLFS();
}

// Processing of Q60 - Q64, Q6Q-Q6R
int
ESVPQDiversifier::calcMustPriceLimitForCurrPass(int numOfAlreadyPickedItins,
                                                int maxMustPricePassSize)
{
  int mustPriceLimitForCurrPass = _esvOptions->getRequestedNumberOfSolutions() -
                                  _esvOptions->noOfESVLowFareSolutionsReq() -
                                  numOfAlreadyPickedItins;

  if (mustPriceLimitForCurrPass > maxMustPricePassSize)
  {
    return maxMustPricePassSize;
  }
  return mustPriceLimitForCurrPass;
}

int
ESVPQDiversifier::calcLFSLimitForPassOne(int numOfAlreadyPickedItins)
{
  int solutionsLeftToChoose =
      _esvOptions->getRequestedNumberOfSolutions() - numOfAlreadyPickedItins;

  if (solutionsLeftToChoose >
      (int)_outOnlineCxrCountdownMPMap.size() * _esvOptions->noOfMinOnlinePerCarrier())
  {
    solutionsLeftToChoose =
        _outOnlineCxrCountdownMPMap.size() * _esvOptions->noOfMinOnlinePerCarrier();
  }
  return solutionsLeftToChoose;
}

int
ESVPQDiversifier::calcLFSLimitForPassTwo(int numOfAlreadyPickedItins)
{
  return static_cast<int>(_esvOptions->getRequestedNumberOfSolutions() *
                          (float)_esvOptions->onlinePercent() / 100.0);
}
int
ESVPQDiversifier::calcLFSLimitForPassThree(int numOfAlreadyPickedItins)
{
  int passThreeSize = 0;
  if (_esvOptions->getRequestedNumberOfSolutions() > numOfAlreadyPickedItins)
  {
    passThreeSize = _esvOptions->getRequestedNumberOfSolutions() - numOfAlreadyPickedItins;
  }

  return passThreeSize;
}
// Processing of Q5X, Q5W and Q5T - for pass 3
bool
ESVPQDiversifier::processItinLFSFlightsAndCarriersLimits(
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec, CarrierCode& carrier)
{
  bool result;
  if (_shoppingTrx != nullptr)
  {
    TSELatencyData metrics(*_shoppingTrx, "ESVPQDIVERSIFIER PROCESS LFS PASS 3");
  }

  if (sopVec.size() == 0)
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::processItinMustPriceFlightsAndCarriersLimits: No "
                  "Scheduling options in sopVec");
    return false;
  }

  result = checkItinFlightsLFSLimits(sopVec);

  if (result)
  {
    result = checkItinCarriersLFSLimits(sopVec, carrier);
  }

  if (result)
  {
    decreaseItinFlightsLFSCd(sopVec);
    decreaseItinCarriersLFSCd(sopVec);
  }

  return result;
}

// Processing of Q5X, Q5W and ( Q5U - pass 1 or Q5V - pass2 )
bool
ESVPQDiversifier::processItinLFSFlightsAndCarriersOnlineLimits(
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec, CarrierCode& carrier)
{
  bool result = false;
  carrier = "";
  if (_shoppingTrx != nullptr)
  {
    TSELatencyData metrics(*_shoppingTrx, "ESVPQDIVERSIFIER PROCESS LFS PASS 1 & 2");
  }

  if (sopVec.size() == 0)
  {
    LOG4CXX_ERROR(logger,
                  "ESVPQDiversifier::processItinMustPriceFlightsAndCarriersLimits: No "
                  "Scheduling options in sopVec");
    return false;
  }

  result = checkItinFlightsLFSLimits(sopVec);

  if (result)
  {
    result = checkItinCarriersOnlineLFSLimits(sopVec);
  }
  if (result)
  {
    decreaseItinFlightsLFSCd(sopVec);
    increaseItinOnlineCarriersLFSC(sopVec);
  }
  else
  {
    carrier = sopVec[0]->governingCarrier();
  }

  return result;
}

bool
ESVPQDiversifier::isAVSCarrier(CarrierCode carrier)
{
  std::vector<CarrierCode>::iterator it;

  for (it = _avsCarriersVec.begin(); it != _avsCarriersVec.end(); ++it)
  {
    if (carrier == static_cast<CarrierCode>(*it))
    {
      return true;
    }
  }
  return false;
}
}
