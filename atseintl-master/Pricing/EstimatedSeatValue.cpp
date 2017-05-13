/*----------------------------------------------------------------------------
 *  File:    EstimatedSeatValue.cpp
 *  Created: Jan 10, 2008
 *  Authors:
 *
 *  Description:
 *
 *  Change History:
 *
 *  Copyright Sabre 2003
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/EstimatedSeatValue.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/TSELatencyData.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag956Collector.h"
#include "Diagnostic/Diag957Collector.h"
#include "Diagnostic/Diag958Collector.h"
#include "Pricing/DominatedFlights.h"
#include "Pricing/ESVPQ.h"
#include "Pricing/ESVPQItem.h"
#include "Pricing/VISPQ.h"
#include "Rules/RuleUtil.h"

#include <cmath>
#include <map>

namespace tse
{
namespace
{
ConfigurableValue<ConfigSet<CarrierCode>>
esvOnlineOnlyCarriers("SHOPPING_OPT", "ESV_ONLINE_ONLY_CARRIERS");
ConfigurableValue<ConfigVector<CarrierCode>>
esvRestrictedCarriers("SHOPPING_OPT", "ESV_RESTRICTED_CARRIERS");
ConfigurableValue<bool>
groupItinsCfg("SHOPPING_OPT", "GROUP_ITINS", true);
ConfigurableValue<bool>
groupInterlineItinCfg("SHOPPING_OPT", "GROUP_INTERLINE_ITINS", true);
ConfigurableValue<int32_t>
maxGroupCfg("SHOPPING_OPT", "MAX_GROUP", -1);
ConfigurableValue<bool>
regroupConnectingItins("SHOPPING_OPT", "REGROUP_CONNECTING_ITINS", false);
ConfigurableValue<int32_t>
maxFamiliesInGroupCfg("SHOPPING_OPT", "MAX_FAMILIES_IN_GROUP", 15);
ConfigurableValue<bool>
groupInFamilies("ESV_PERFORMANCE_OPT", "GROUP_IN_FAMILIES", true);
ConfigurableValue<bool>
generateIVOptionsCfg("VIS_OPTIONS", "GENERATE_IV_OPTIONS", true);
}

// ====================== EstimatedSeatValue ==========================

log4cxx::LoggerPtr
EstimatedSeatValue::_logger(log4cxx::Logger::getLogger("atseintl.EstimatedSeatValue"));

namespace
{
bool
isSelected(ESVPQItem* pqItem)
{
  return pqItem->isSelected();
}
}

std::set<CarrierCode> EstimatedSeatValue::_onlineOnlyCxr;
std::map<CarrierCode, std::set<CarrierCode>> EstimatedSeatValue::_restrictedCxr;

void
EstimatedSeatValue::classInit()
{
  // init online only list
  for (const auto esvOnlineOnlyCarrier : esvOnlineOnlyCarriers.getValue())
    _onlineOnlyCxr.insert(esvOnlineOnlyCarrier);
  // init restricted carriers list
  _restrictedCxr.clear();
  CarrierCode mainCxr;
  std::set<CarrierCode> allowedCxr;
  for (const auto esvRestrictedCarrier : esvRestrictedCarriers.getValue())
  {
    if (esvRestrictedCarrier == "*")
    {
      if (!mainCxr.empty())
      {
        _restrictedCxr[mainCxr] = allowedCxr;
        allowedCxr.clear();
        mainCxr.clear();
      }
    }
    else
    {
      if (mainCxr.empty())
        mainCxr = esvRestrictedCarrier;

      allowedCxr.insert(esvRestrictedCarrier);
    }
  }
}

bool
EstimatedSeatValue::isValidInterline(ShoppingTrx& trx, InterlineTicketCarrier* itc, Itin* outItin)
{
  AirSeg* airSeg = dynamic_cast<AirSeg*>(outItin->travelSeg().at(0));

  if (airSeg == nullptr)
  {
    return false;
  }

  CarrierCode& validatingCarrier = airSeg->carrier();

  std::set<CarrierCode> cxrs;

  ShoppingUtil::buildCarriersSet(outItin, cxrs);

  return checkCarrier(trx, validatingCarrier, itc, cxrs);
}

bool
EstimatedSeatValue::isValidInterline(ShoppingTrx& trx,
                                     InterlineTicketCarrier* itc,
                                     Itin* outItin,
                                     Itin* inItin)
{
  AirSeg* airSeg = dynamic_cast<AirSeg*>(outItin->travelSeg().at(0));

  if (airSeg == nullptr)
  {
    return false;
  }

  CarrierCode& validatingCarrier = airSeg->carrier();

  std::set<CarrierCode> cxrs;

  ShoppingUtil::buildCarriersSet(outItin, cxrs);
  ShoppingUtil::buildCarriersSet(inItin, cxrs);

  return checkCarrier(trx, validatingCarrier, itc, cxrs);
}

bool
EstimatedSeatValue::checkCarrier(ShoppingTrx& trx,
                                 CarrierCode& validatingCarrier,
                                 InterlineTicketCarrier* itc,
                                 std::set<CarrierCode>& cxrs)
{
  // If it's online always return true
  if (cxrs.size() == 1)
  {
    return true;
  }

  if (trx.getOptions()->validateTicketingAgreement())
  {
    return ShoppingUtil::checkCarriersSet(trx, validatingCarrier, itc, cxrs);
  }
  else
  {
    std::set<CarrierCode>::iterator i = cxrs.begin();
    std::set<CarrierCode>::iterator e = cxrs.end();
    for (; i != e; ++i)
    {
      if (_onlineOnlyCxr.find(*i) != _onlineOnlyCxr.end())
        return false;

      std::map<CarrierCode, std::set<CarrierCode>>::iterator m = _restrictedCxr.find(*i);
      if (m != _restrictedCxr.end())
      {
        std::set<CarrierCode>& allowed = m->second;
        std::set<CarrierCode>::iterator j = cxrs.begin();
        for (; j != e; ++j)
        {
          if (allowed.find(*j) == allowed.end())
            return false;
        }
      }
    }

    return true;
  }
}

bool
EstimatedSeatValue::checkNumberOfStops(ShoppingTrx::SchedulingOption* sop,
                                       QueueType qType,
                                       int legNo)
{
  if (sop == nullptr)
  {
    return false;
  }

  Itin* itin = sop->itin();
  if (itin == nullptr)
  {
    return false;
  }
  int numberOfSegments = itin->travelSeg().size();

  if (((qType == MPNonstopOnline) || (qType == MPNonstopInterline)) && (numberOfSegments != 1))
  {
    return false;
  }

  if ((qType == MPOutNonstopOnline) || (qType == MPOutNonstopInterline))
  {
    if ((legNo == 0) && (numberOfSegments != 1))
    {
      return false;
    }
    if ((legNo == 1) && (numberOfSegments != 2))
    {
      return false;
    }
  }

  if ((qType == MPSingleStopOnline) && (numberOfSegments != 1) && (numberOfSegments != 2))
  {
    return false;
  }

  return true;
}

bool
EstimatedSeatValue::checkNumberOfStops(ESVPQItem* item,
                                       QueueType qType,
                                       std::string& addInfo,
                                       ShoppingTrx& _trx)
{
  bool result = true;
  int outboundSegs;
  int inboundSegs;

  item->getNumberOfSegs(outboundSegs, inboundSegs);

  if (((qType == MPNonstopOnline) || (qType == MPNonstopInterline)) &&
      ((outboundSegs != 1) || (inboundSegs > 1)))
  {
    result = false;
  }

  // These two queues are only for Roung Trips
  if (((qType == MPOutNonstopOnline) || (qType == MPOutNonstopInterline)) &&
      ((outboundSegs != 1) || (inboundSegs != 2)))
  {
    result = false;
  }

  if (qType == MPSingleStopOnline)
  {
    if ((outboundSegs > 2) || (inboundSegs > 2))
    {
      result = false;
    }
    else if ((outboundSegs == 1) && (inboundSegs == 1))
    {
      result = false;
    }
  }

  if (!result)
  {
    addInfo = "DIV2";
  }
  return result;
}

// the function returns false in the indicator is ONLINE and SOP is not online
bool
EstimatedSeatValue::checkOnlineConfirmity(ShoppingTrx::SchedulingOption* sop, QueueType qType)
{
  if (sop == nullptr)
  {
    return false;
  }

  if ((qType == MPNonstopOnline) || (qType == MPOutNonstopOnline) ||
      (qType == MPSingleStopOnline) || (qType == MPRemainingOnline) || (qType == LFSOnline) ||
      (qType == LFSOnlineByCarrier))
  {
    Itin* itin = sop->itin();
    if (itin == nullptr)
    {
      return false;
    }

    if ("" == itin->onlineCarrier())
    {
      return false;
    }
  }

  return true;
}

bool
EstimatedSeatValue::checkOnlineConfirmity(ESVPQItem* item,
                                          QueueType qType,
                                          std::string& addInfo,
                                          ShoppingTrx& _trx)
{
  bool result = false;

  if (item->isOnline())
  {
    if ((qType == MPNonstopInterline) || (qType == MPOutNonstopInterline))
    {
      result = false;
      addInfo = "DIV3";
    }
    else
    {
      result = true;
    }
  }
  else
  {
    if ((qType == MPNonstopOnline) || (qType == MPOutNonstopOnline) ||
        (qType == MPSingleStopOnline) || (qType == MPRemainingOnline) || (qType == LFSOnline) ||
        (qType == LFSOnlineByCarrier))
    {
      result = false;
      addInfo = "DIV4";
    }
    else
    {
      result = true;
    }
  }

  return result;
}

EstimatedSeatValue::EstimatedSeatValue(ShoppingTrx& trx)
  : _trx(trx), _oneWay(_trx.legs().size() == 1)
{
}

void
EstimatedSeatValue::process()
{
  TSELatencyData metrics(_trx, "ESV PROCESS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::process()");

  if (nullptr == _trx.visOptions())
  {
    _newLogicEnabled = false;
  }
  else
  {
    _newLogicEnabled = _trx.visOptions()->valueBasedItinSelection().enableVIS();
  }

  _logEnabled = true;
  if (_newLogicEnabled)
  {
    newLogic();
  }
  else
  {
    _esvDiv = _trx.dataHandle().create<ESVPQDiversifier>();
    _esvDiv->init(&_trx);

    generateSolutions(_trx.paxType()[0]);
  }
};

void
EstimatedSeatValue::generateSolutions(PaxType* paxType)
{
  TSELatencyData metrics(_trx, "ESV GENERATE SOLUTIONS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::generateSolutions()");

  _pickedItems.clear();

  // ESV proceeds only up to two legs
  if (_trx.legs().size() <= 2)
  {
    std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>> outPerCarrierMap;
    std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>> inbPerCarrierMap;

    generatePerCarrierSopVectorsMaps(outPerCarrierMap, inbPerCarrierMap, paxType);

    diagDisplayAllSopsInLegs(outPerCarrierMap, inbPerCarrierMap);
    if (_trx.esvOptions()->mustPriceFlag())
    {
      generateMustPriceSolutions(outPerCarrierMap, inbPerCarrierMap, paxType);
    }
    generateLFSSolutions(outPerCarrierMap, inbPerCarrierMap, paxType);
  }

  sendSolutions(_pickedItems);
}

void
EstimatedSeatValue::sendSolutions(std::vector<ESVPQItem*>& pickedItems)
{
  PQItemReverseCompare pqComp;
  sort(pickedItems.begin(), pickedItems.end(), pqComp);

  groupSolutionsByGoverningCarrier(pickedItems);

  groupSolutionsInFamilies(pickedItems);

  diagDisplay957(pickedItems);

  regroupSolutions(pickedItems);

  writeSolutionsToOutput(pickedItems);
}

void
EstimatedSeatValue::groupSolutionsInFamilies(std::vector<ESVPQItem*>& esvPQItemVec)
{
  TSELatencyData metrics(_trx, "ESV GROUP SOLUTIONS IN FAMILIES");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::groupSolutionsInFamilies(std::vector<ESVPQItem*>&)");

  // Read server parameters
  std::map<std::string, uint32_t> groupMap;

  // Go thorough all ESV PQ items
  std::vector<ESVPQItem*>::iterator esvPQItemIter;
  uint32_t groupId = 0;
  if (groupInFamilies.getValue())
  {
    for (esvPQItemIter = esvPQItemVec.begin(); esvPQItemIter != esvPQItemVec.end(); esvPQItemIter++)
    {
      ESVPQItem* esvPQItem = (*esvPQItemIter);

      // Check if ESV PQ item is not null
      if (nullptr == esvPQItem)
      {
        LOG4CXX_ERROR(_logger,
                      "EstimatedSeatValue::groupSolutionsInFamilies - ESV PQ Item object is NULL.");
        continue;
      }

      std::string pqItemKey = generatePqItemKey(esvPQItem);

      if ("" == pqItemKey)
      {
        LOG4CXX_ERROR(
            _logger,
            "EstimatedSeatValue::groupSolutionsInFamilies - Error while generating pq item key.");
        continue;
      }

      std::map<std::string, uint32_t>::iterator pqMapIter;
      pqMapIter = groupMap.find(pqItemKey);

      if (groupMap.end() != pqMapIter)
      {
        // If key already exist it means that we should use existing group
        esvPQItem->groupId() = pqMapIter->second;
        esvPQItem->isPrimarySolution() = false;
      }
      else
      {
        // If key doen't exist create new group
        groupMap.insert(std::pair<std::string, uint32_t>(pqItemKey, groupId));
        esvPQItem->groupId() = groupId;
        groupId++;
      }
    }
  }
  else
  {
    for (auto esvPQItem : esvPQItemVec)
    {
      esvPQItem->groupId() = groupId;
      groupId++;
    }
  }
}

std::string
EstimatedSeatValue::generatePqItemKey(ESVPQItem* esvPQItem)
{
  TSELatencyData metrics(_trx, "ESV GENERATE PQ ITEM KEY");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::generatePqItemKey(ESVPQItem*)");

  std::string pqItemKey = "";

  for (uint32_t legId = 0; legId < _trx.legs().size(); legId++)
  {
    ESVSopWrapper* sopWrapper =
        (0 == legId) ? esvPQItem->outSopWrapper() : esvPQItem->inSopWrapper();

    // Add fare basis codes to a key
    for (uint32_t i = 0; i < sopWrapper->paxTypeFareVec()->size(); ++i)
    {
      std::string fareClass((sopWrapper->paxTypeFareVec()->at(i))->fareClass().c_str());
      pqItemKey += fareClass + ":";
    }

    pqItemKey += "^";

    // Add travel segments to a key
    for (uint32_t j = 0; j < sopWrapper->sop()->itin()->travelSeg().size(); ++j)
    {
      TravelSeg* travelSeg = sopWrapper->sop()->itin()->travelSeg()[j];

      pqItemKey += travelSeg->origin()->loc() + "-" + travelSeg->destination()->loc() + ":";
    }

    pqItemKey += "|";
  }

  return pqItemKey;
}

void
EstimatedSeatValue::groupSolutionsByGoverningCarrier(std::vector<ESVPQItem*>& esvPQItemVec)
{
  TSELatencyData metrics(_trx, "ESV GROUP SOLUTIONS BY GOVERNING CARRIER");
  LOG4CXX_DEBUG(_logger,
                "EstimatedSeatValue::groupSolutionsByGoverningCarrier(std::vector<ESVPQItem*>&)");

  // Read server parameters
  int maxGroup = maxGroupCfg.getValue();
  bool groupItin = groupItinsCfg.getValue();
  bool groupInterlineItin = groupInterlineItinCfg.getValue();
  // Assign carrier group id for each ESVPQItem
  std::map<std::string, int> carrierGroupMap;
  std::map<std::string, int>::iterator carrierGroupMapIter;
  int groupId = 0;

  std::vector<ESVPQItem*>::iterator esvPQItemIter;

  for (esvPQItemIter = esvPQItemVec.begin(); esvPQItemIter != esvPQItemVec.end(); esvPQItemIter++)
  {
    ESVPQItem* esvPQItem = (*esvPQItemIter);

    // Check if ESV PQ item is not null
    if (nullptr == esvPQItem)
    {
      LOG4CXX_ERROR(
          _logger,
          "EstimatedSeatValue::groupSolutionsByGoverningCarrier - ESV PQ Item object is NULL.");
      continue;
    }

    ShoppingTrx::SchedulingOption* sop = esvPQItem->outSopWrapper()->sop();

    if (nullptr == sop)
    {
      LOG4CXX_ERROR(_logger,
                    "EstimatedSeatValue::groupSolutionsByGoverningCarrier - Scheduling "
                    "option object is NULL.");
      continue;
    }

    // Datermine carrier key value
    std::string governingCarrier = "";

    if (groupItin)
    {
      governingCarrier = sop->governingCarrier();

      if (groupInterlineItin && (false == esvPQItem->isOnline()))
      {
        // Add "_" character to governing carrier of interline flights
        // to create different group
        governingCarrier = "_" + sop->governingCarrier();
      }
    }
    else
    {
      esvPQItem->carrierGroupId() = 0;
      continue;
    }

    carrierGroupMapIter = carrierGroupMap.find(governingCarrier);

    if (carrierGroupMapIter != carrierGroupMap.end())
    {
      // If it already exist use existing carrier group
      esvPQItem->carrierGroupId() = carrierGroupMapIter->second;
    }
    else
    {
      // If carrier group doesn't exist and we didn't exceed max groups
      // count add new group
      if ((-1 != maxGroup) && ((uint32_t)maxGroup <= carrierGroupMap.size()))
      {
        esvPQItem->carrierGroupId() = groupId;
      }
      else
      {
        groupId++;
        carrierGroupMap.insert(std::make_pair(governingCarrier, groupId));
        esvPQItem->carrierGroupId() = groupId;
      }
    }
  }
}

inline bool
PQItemGroupCompare::
operator()(ESVPQItem* lhs, ESVPQItem* rhs)
{
  bool result = false;

  if (lhs->carrierGroupId() == rhs->carrierGroupId())
  {
    result = lhs->groupId() < rhs->groupId();
  }
  else
  {
    result = lhs->carrierGroupId() < rhs->carrierGroupId();
  }

  return result;
}

void
EstimatedSeatValue::regroupSolutions(std::vector<ESVPQItem*>& esvPQItemVec)
{
  int maxGroup = maxGroupCfg.getValue();
  int maxFamiliesInGroup = maxFamiliesInGroupCfg.getValue();

  // Check if regroup of connecting itins is enabled
  if (!regroupConnectingItins.getValue() || !groupItinsCfg.getValue())
  {
    return;
  }
  // Make a copy of the vector and sort by carrierGroupId(Q4Q) and
  // groupId(Q5Q)
  std::vector<ESVPQItem*> pqItemVec = esvPQItemVec;
  PQItemGroupCompare pqComp;

  sort(pqItemVec.begin(), pqItemVec.end(), pqComp);

  int numLegs = _trx.legs().size();
  int nextCarrierGroupId = pqItemVec.back()->carrierGroupId() + 1;

  std::vector<ESVPQItem*>::iterator iter = pqItemVec.begin();
  std::vector<ESVPQItem*>::iterator iterEnd = pqItemVec.end();

  for (; iter != iterEnd;)
  {
    std::vector<ESVPQItem*>::iterator groupBegin = iter;
    std::vector<ESVPQItem*>::iterator groupEnd = iterEnd;
    std::vector<ESVPQItem*>::iterator prevFamily = groupBegin;
    int numFamilies = 1;
    bool connectingOnly = true;

    while (iter != iterEnd)
    {
      if ((*iter)->carrierGroupId() == (*groupBegin)->carrierGroupId())
      {
        if ((*iter)->getNumberOfSegs() == numLegs)
        {
          connectingOnly = false;
        }

        if ((*iter)->groupId() != (*prevFamily)->groupId())
        {
          ++numFamilies;
          prevFamily = iter;
        }

        ++iter;
        groupEnd = iter;
      }
      else
      {
        // Next group started
        break;
      }
    }

    if (!connectingOnly)
    {
      // This group is either mixed or non-stop only so skip regrouping
      continue;
    }

    if (numFamilies <= maxFamiliesInGroup)
    {
      continue;
    }

    // Regroup the current group
    numFamilies = 1;
    prevFamily = groupBegin;
    int regroupId = (*groupBegin)->carrierGroupId();

    for (std::vector<ESVPQItem*>::iterator groupItr = groupBegin; groupItr != groupEnd; ++groupItr)
    {
      if ((*prevFamily)->groupId() == (*groupItr)->groupId())
      {
        (*groupItr)->carrierGroupId() = regroupId;
        prevFamily = groupItr;
        continue;
      }

      ++numFamilies;

      if (numFamilies <= maxFamiliesInGroup)
      {
        (*groupItr)->carrierGroupId() = regroupId;
        prevFamily = groupItr;
        continue;
      }

      if (maxGroup != -1 && nextCarrierGroupId > maxGroup)
      {
        break;
      }

      regroupId = nextCarrierGroupId;
      ++nextCarrierGroupId;
      numFamilies = 1;
      (*groupItr)->carrierGroupId() = regroupId;
      prevFamily = groupItr;
    }

    if (maxGroup != -1 && nextCarrierGroupId > maxGroup)
    {
      break;
    }
  }
}

void
EstimatedSeatValue::writeSolutionsToOutput(std::vector<ESVPQItem*>& esvPQItemVec)
{
  TSELatencyData metrics(_trx, "ESV WRITE SOLUTIONS TO OUTPUT");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::writeSolutionsToOutput(std::vector<ESVPQItem*>&)");

  std::vector<ESVPQItem*>::iterator esvPQItemIter;

  for (esvPQItemIter = esvPQItemVec.begin(); esvPQItemIter != esvPQItemVec.end(); esvPQItemIter++)
  {
    ESVPQItem* esvPQItem = (*esvPQItemIter);

    // Check if ESV PQ item is not null
    if (nullptr == esvPQItem)
    {
      LOG4CXX_ERROR(_logger,
                    "EstimatedSeatValue::writeSolutionsToOutput - ESV PQ Item object is NULL.");
      continue;
    }

    ESVSolution* esvSolution;
    _trx.dataHandle().get(esvSolution);

    esvSolution->groupId() = esvPQItem->groupId();
    esvSolution->carrierGroupId() = esvPQItem->carrierGroupId();
    esvSolution->totalAmt() = esvPQItem->totalAmt();
    esvSolution->totalPrice() = esvPQItem->totalPrice();
    esvSolution->totalPenalty() = esvPQItem->totalPenalty();
    esvSolution->isJcb() = esvPQItem->isJcb();
    esvSolution->queueType() = esvPQItem->queueType();
    esvSolution->numberOfSegments() = esvPQItem->getNumberOfSegs();

    esvSolution->outboundOption().sopId() = esvPQItem->outSopWrapper()->sop()->sopId();

    for (uint32_t paxTypeFareId = 0;
         paxTypeFareId < esvPQItem->outSopWrapper()->paxTypeFareVec()->size();
         ++paxTypeFareId)
    {
      ESVFareComponent* esvFareComponent;
      _trx.dataHandle().get(esvFareComponent);

      esvFareComponent->paxTypeFare() =
          esvPQItem->outSopWrapper()->paxTypeFareVec()->at(paxTypeFareId);
      esvFareComponent->fareBasisCode() = esvFareComponent->paxTypeFare()->fareClass().c_str();

      esvSolution->outboundOption().esvFareComponentsVec().push_back(esvFareComponent);
    }

    matchBookingInfo(esvSolution->outboundOption(), 0);

    if (esvPQItem->inSopWrapper() != nullptr)
    {
      esvSolution->inboundOption().sopId() = esvPQItem->inSopWrapper()->sop()->sopId();

      for (uint32_t paxTypeFareId = 0;
           paxTypeFareId < esvPQItem->inSopWrapper()->paxTypeFareVec()->size();
           paxTypeFareId++)
      {
        ESVFareComponent* esvFareComponent;
        _trx.dataHandle().get(esvFareComponent);

        esvFareComponent->paxTypeFare() =
            esvPQItem->inSopWrapper()->paxTypeFareVec()->at(paxTypeFareId);
        esvFareComponent->fareBasisCode() = esvFareComponent->paxTypeFare()->fareClass().c_str();

        esvSolution->inboundOption().esvFareComponentsVec().push_back(esvFareComponent);
      }

      matchBookingInfo(esvSolution->inboundOption(), 1);
    }

    if (Diagnostic959 == _trx.diagnostic().diagnosticType())
    {
      ESVSopWrapper::SOPFareListType outType = esvPQItem->getTypes().first;

      if (1 == _trx.legs().size())
      {
        if (ESVSopWrapper::OW == outType)
        {
          esvSolution->pricingUnitType() = PricingUnit::Type::ONEWAY;
        }
      }
      else
      {
        ESVSopWrapper::SOPFareListType inType = esvPQItem->getTypes().second;

        if ((ESVSopWrapper::OW == outType) && (ESVSopWrapper::OW == inType))
        {
          esvSolution->pricingUnitType() = PricingUnit::Type::ONEWAY;
        }
        else if ((ESVSopWrapper::RT == outType) && (ESVSopWrapper::RT == inType))
        {
          esvSolution->pricingUnitType() = PricingUnit::Type::ROUNDTRIP;
        }
        else if ((ESVSopWrapper::CT == outType) && (ESVSopWrapper::CT == inType))
        {
          esvSolution->pricingUnitType() = PricingUnit::Type::CIRCLETRIP;
        }
        else if ((ESVSopWrapper::OJ == outType) && (ESVSopWrapper::OJ == inType))
        {
          esvSolution->pricingUnitType() = PricingUnit::Type::OPENJAW;
        }
      }
    }

    // Write primary solutions to flight matrix
    if (esvPQItem->isPrimarySolution())
    {
      _trx.flightMatrixESV().push_back(esvSolution);
    }
    // and child solutions to estimate matrix
    else
    {
      _trx.estimateMatrixESV().push_back(esvSolution);
    }
  }
}

void
EstimatedSeatValue::matchBookingInfo(ESVOption& esvOption, uint32_t legId)
{
  TSELatencyData metrics(_trx, "ESV MATCH BOOKING INFO");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::matchBookingInfo(ESVOption&, uint32_t)");

  const ShoppingTrx::SchedulingOption* sop = &(_trx.legs()[legId].sop()[esvOption.sopId()]);

  if (nullptr == sop)
  {
    LOG4CXX_ERROR(_logger,
                  "EstimatedSeatValue::matchBookingInfo - SchedulingOption object is NULL.");
    return;
  }

  std::vector<ESVFareComponent*>::iterator fareCompIter;

  for (fareCompIter = esvOption.esvFareComponentsVec().begin();
       fareCompIter != esvOption.esvFareComponentsVec().end();
       fareCompIter++)
  {
    ESVFareComponent* esvFareComponent = (*fareCompIter);

    if (nullptr == esvFareComponent)
    {
      LOG4CXX_ERROR(_logger,
                    "EstimatedSeatValue::matchBookingInfo - ESVFareComponent object is NULL.");
      continue;
    }

    if (nullptr == esvFareComponent->paxTypeFare())
    {
      LOG4CXX_ERROR(_logger, "EstimatedSeatValue::matchBookingInfo - PaxTypeFare object is NULL.");
      continue;
    }

    const PaxTypeFare::FlightBit& bitMap =
        esvFareComponent->paxTypeFare()->flightBitmapESV()[esvOption.sopId()];

    int segStatId = 0;
    bool outputSegment = false;
    std::vector<TravelSeg*>::const_iterator tvlSegIter;

    for (tvlSegIter = sop->itin()->travelSeg().begin();
         tvlSegIter != sop->itin()->travelSeg().end();
         tvlSegIter++)
    {
      TravelSeg* travelSeg = (*tvlSegIter);

      if (travelSeg->origin()->loc() ==
          esvFareComponent->paxTypeFare()->fareMarket()->origin()->loc())
      {
        outputSegment = true;
      }

      if (true == outputSegment)
      {
        ESVSegmentInfo* esvSegmentInfo;
        _trx.dataHandle().get(esvSegmentInfo);

        esvSegmentInfo->travelSeg() = travelSeg;

        if (bitMap._segmentStatus.empty())
        {
          LOG4CXX_ERROR(_logger,
                        "EstimatedSeatValue::matchBookingInfo - Segment status vector in "
                        "flight bit is empty.");
          break;
        }
        else
        {
          const PaxTypeFare::SegmentStatus& segmentStatus = bitMap._segmentStatus.at(segStatId);

          esvSegmentInfo->bookingCode() = segmentStatus._bkgCodeReBook;
          esvSegmentInfo->bookedCabin() = segmentStatus._reBookCabin;
        }

        esvFareComponent->esvSegmentInfoVec().push_back(esvSegmentInfo);
        segStatId++;
      }

      if (travelSeg->destination()->loc() ==
          esvFareComponent->paxTypeFare()->fareMarket()->destination()->loc())
      {
        outputSegment = false;
      }
    }
  }
}

void
EstimatedSeatValue::generateMustPriceSolutions(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap,
    PaxType* paxType)
{
  TSELatencyData metrics(_trx, "ESV GENERATE MUST PRICE SOLUTIONS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::generateMustPriceSolutions()");

  {
    // generate Nonstop Online solution
    TSELatencyData metrics1(_trx, "ESV NONSTOP ONLINE SOLUTIONS");
    generatePerCarrierOnlineSolutions(outPerCarrierMap, inbPerCarrierMap, paxType, MPNonstopOnline);
  }

  {
    // generate Outbound Nonstop - Inbound Single Stop Online solution
    TSELatencyData metrics2(_trx, "ESV OUT-NONSTOP ONLINE SOLUTIONS");
    generatePerCarrierOnlineSolutions(
        outPerCarrierMap, inbPerCarrierMap, paxType, MPOutNonstopOnline);
  }

  {
    // generate Nonstop Interline solutions
    TSELatencyData metrics3(_trx, "ESV NONSTOP INTERLINE SOLUTIONS");
    generatePerCarrierInterlineSolutions(
        outPerCarrierMap, inbPerCarrierMap, paxType, MPNonstopInterline);
  }

  {
    // generate Outbound Nonstop - Inbound Single Stop Interline solution
    TSELatencyData metrics4(_trx, "ESV OUT-NONSTOP INTERLINE SOLUTIONS");
    generatePerCarrierInterlineSolutions(
        outPerCarrierMap, inbPerCarrierMap, paxType, MPOutNonstopInterline);
  }

  {
    // generate Single Stop Online solutions
    TSELatencyData metrics5(_trx, "ESV SINGLE STOP ONLINE SOLUTIONS");
    generatePerCarrierOnlineSolutions(
        outPerCarrierMap, inbPerCarrierMap, paxType, MPSingleStopOnline);
  }

  {
    // generate remaining Online Solutions
    TSELatencyData metrics6(_trx, "ESV REMAINING ONLINE SOLUTIONS");
    generateOverallOnlineSolutions(outPerCarrierMap, inbPerCarrierMap, paxType, MPRemainingOnline);
  }

  {
    // generate remaining Solutions
    TSELatencyData metrics7(_trx, "ESV REMAINING SOLUTIONS");
    generateRemainingSolutions(outPerCarrierMap, inbPerCarrierMap, paxType, MPRemaining);
  }
}
void
EstimatedSeatValue::generateLFSSolutions(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap,
    PaxType* paxType)
{
  TSELatencyData metrics(_trx, "ESV GENERATE LFS SOLUTIONS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::generateLFSSolutions()");

  {
    TSELatencyData metrics1(_trx, "ESV ONLINE SOLUTIONS");
    generatePerCarrierLowFareSolutions(outPerCarrierMap, inbPerCarrierMap, paxType, LFSOnline);
  }

  {
    _esvDiv->recalcCarrierOnlineLimitLFSMap();
    TSELatencyData metrics2(_trx, "ESV ONLINE BY CARRIER SOLUTIONS");
    generateOverallOnlineSolutions(outPerCarrierMap, inbPerCarrierMap, paxType, LFSOnlineByCarrier);
  }

  {
    TSELatencyData metrics3(_trx, "ESV REMAINING SOLUTIONS");
    generateRemainingSolutions(outPerCarrierMap, inbPerCarrierMap, paxType, LFSRemaining);
  }
}

void
EstimatedSeatValue::generatePerCarrierOnlineSolutions(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap,
    PaxType* paxType,
    QueueType pqType)
{
  TSELatencyData metrics(_trx, "ESV PER CARRIER MUST PRICE SOLUTIONS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::generatePerCarrierMustPriceSolutions()");

  std::map<CarrierCode, SopVectorsPair> perCarrierSVPairs =
      generatePerCarrierSopVectorsPairs(outPerCarrierMap, inbPerCarrierMap);
  std::map<CarrierCode, SopVectorsPair>::const_iterator it;

  for (it = perCarrierSVPairs.begin(); it != perCarrierSVPairs.end(); it++)
  {
    CarrierCode carrier = (*it).first;
    std::string pqTitle = generatePQtitle(pqType, carrier);
    std::string pqDivOption = generatePQDivOption(pqType);

    ESVPQ esvpq(&_trx, pqTitle);

    EstimatedSeatValue::SopVectorsPair filteredSVPair =
        filterSopVectorsPairForQueue((*it).second.first, (*it).second.second, pqType);

    esvpq.init(filteredSVPair.first, filteredSVPair.second, pqType, carrier, pqDivOption);

    diagDisplay956(&esvpq);

    int limitForCurrPass = calcLimitForCurrPass(pqType);

    generateSinglePassSolutions(esvpq, limitForCurrPass, pqType);
  }
}

void
EstimatedSeatValue::generatePerCarrierInterlineSolutions(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap,
    PaxType* paxType,
    QueueType pqType)
{
  TSELatencyData metrics(_trx, "ESV PER CARRIER MUST PRICE SOLUTIONS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::generatePerCarrierMustPriceInterlineSolutions()");

  bool firstQueueFlag = true;

  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>::const_iterator outIt;
  std::vector<ESVSopWrapper*> inbSopWracVec;

  for (outIt = outPerCarrierMap.begin(); outIt != outPerCarrierMap.end(); outIt++)
  {
    TSELatencyData metric1(_trx, "For");
    CarrierCode carrier = (*outIt).first;
    std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>::const_iterator inbIt =
        inbPerCarrierMap.find(static_cast<CarrierCode>(""));

    if ((carrier.empty()) || (_esvDiv->isOnlineOnlyRestricted(carrier)) ||
        (inbIt == inbPerCarrierMap.end()))
    {
      continue;
    }

    std::string pqTitle = generatePQtitle(pqType, carrier);
    std::string pqDivOption = generatePQDivOption(pqType);

    ESVPQ esvpq(&_trx, pqTitle);

    SopVectorsPair filteredSVPair =
        filterSopVectorsPairForQueue((*outIt).second, (*inbIt).second, pqType);

    if (firstQueueFlag)
    {
      esvpq.init(filteredSVPair.first, filteredSVPair.second, pqType, carrier, pqDivOption);
      firstQueueFlag = false;
      if (esvpq.legVec().size() == 2)
      {
        inbSopWracVec = *(esvpq.legVec()[1]);
      }
    }
    else
    {
      esvpq.init(filteredSVPair.first, inbSopWracVec, pqType, carrier, pqDivOption);
    }
    diagDisplay956(&esvpq);

    generateSinglePassSolutions(esvpq, calcLimitForCurrPass(pqType), pqType);
  }
}

void
EstimatedSeatValue::generateOverallOnlineSolutions(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap,
    PaxType* paxType,
    QueueType pqType)
{
  TSELatencyData metrics(_trx, "ESV GENERATE OVERALL ONLINE SOLUTIONS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::generateOverallOnlineSolutions()");

  std::map<CarrierCode, SopVectorsPair> perCarrierSVPairs =
      generatePerCarrierSopVectorsPairs(outPerCarrierMap, inbPerCarrierMap);

  std::map<CarrierCode, SopVectorsPair>::iterator it;
  std::vector<ESVPQ> esvpqVec(perCarrierSVPairs.size(), ESVPQ(&_trx));
  int i;

  for (it = perCarrierSVPairs.begin(), i = 0; it != perCarrierSVPairs.end(); it++, i++)
  {
    std::string pqTitle = generatePQtitle(pqType);
    std::string pqDivOption = generatePQDivOption(pqType);

    esvpqVec[i].title() = pqTitle;
    SopVectorsPair filteredSVPair =
        filterSopVectorsPairForQueue((*it).second.first, (*it).second.second, pqType);

    esvpqVec[i].init(filteredSVPair.first, filteredSVPair.second, pqType, "", pqDivOption);
    diagDisplay956(&(esvpqVec[i]));
  }

  generateSinglePassSolutions(esvpqVec, calcLimitForCurrPass(pqType), pqType);
}

void
EstimatedSeatValue::generatePerCarrierLowFareSolutions(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap,
    PaxType* paxType,
    QueueType pqType)
{
  TSELatencyData metrics(_trx, "ESV GENERATE PER CARRIER LOW FARE SOLUTIONS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::generateOverallOnlineSolutions()");

  std::map<CarrierCode, SopVectorsPair> perCarrierSVPairs =
      generatePerCarrierSopVectorsPairs(outPerCarrierMap, inbPerCarrierMap);

  std::map<CarrierCode, SopVectorsPair>::iterator it;
  std::vector<ESVPQ> esvpqVec(perCarrierSVPairs.size(), ESVPQ(&_trx));
  int i;

  for (it = perCarrierSVPairs.begin(), i = 0; it != perCarrierSVPairs.end(); it++, i++)
  {
    std::string pqTitle = generatePQtitle(pqType);
    std::string pqDivOption = generatePQDivOption(pqType);

    ESVPQ esvpq(&_trx);
    esvpq.title() = pqTitle;
    SopVectorsPair filteredSVPair =
        filterSopVectorsPairForQueue((*it).second.first, (*it).second.second, pqType);

    esvpq.init(filteredSVPair.first, filteredSVPair.second, pqType, "", pqDivOption);
    diagDisplay956(&(esvpq));
    generateSinglePassSolutions(esvpq, _trx.esvOptions()->noOfMinOnlinePerCarrier(), pqType);
  }
}
void
EstimatedSeatValue::generateRemainingSolutions(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap,
    PaxType* paxType,
    QueueType pqType)
{
  TSELatencyData metrics(_trx, "ESV GENERATE REMAINING SOLUTIONS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::generateRemainingSolutions()");

  std::vector<SopVectorsPair> sVPairs = generateSopVectorsPairs(outPerCarrierMap, inbPerCarrierMap);
  std::vector<ESVPQ> esvpqVec(sVPairs.size(), ESVPQ(&_trx));

  if (_trx.legs().size() != 2)
  {
    generateRemainingOneWaySolutions(sVPairs, esvpqVec, pqType);
  }
  else
  {
    generateRemainingRoundTripSolutions(sVPairs, esvpqVec, pqType);
  }
}

void
EstimatedSeatValue::generateRemainingOneWaySolutions(std::vector<SopVectorsPair>& sVPairs,
                                                     std::vector<ESVPQ>& esvpqVec,
                                                     QueueType pqType)
{
  std::vector<SopVectorsPair>::iterator it;
  std::map<CarrierCode, std::vector<ESVSopWrapper*>> outSopWrapperVecMap;
  int i;

  for (it = sVPairs.begin(), i = 0; it != sVPairs.end(); it++, i++)
  {
    CarrierCode outCarrier = (*it).first.front()->governingCarrier();

    std::string pqTitle = generatePQtitle(pqType);
    std::string pqDivOption = generatePQDivOption(pqType);

    esvpqVec[i].title() = pqTitle;
    SopVectorsPair filteredSVPair = filterSopVectorsPairForQueue((*it).first, (*it).second, pqType);

    if (outSopWrapperVecMap[outCarrier].size() > 0)
    {
      esvpqVec[i].init(
          outSopWrapperVecMap[outCarrier], filteredSVPair.second, pqType, "", pqDivOption);
    }
    else
    {
      esvpqVec[i].init(filteredSVPair.first, filteredSVPair.second, pqType, "", pqDivOption);
      outSopWrapperVecMap[outCarrier] = *(esvpqVec[i].legVec()[0]);
    }
    diagDisplay956(&(esvpqVec[i]));
  }
  generateSinglePassSolutions(esvpqVec, calcLimitForCurrPass(pqType), pqType);
}

void
EstimatedSeatValue::generateRemainingRoundTripSolutions(std::vector<SopVectorsPair>& sVPairs,
                                                        std::vector<ESVPQ>& esvpqVec,
                                                        QueueType pqType)
{
  std::vector<SopVectorsPair>::iterator it;
  std::map<CarrierCode, std::vector<ESVSopWrapper*>> outSopWrapperVecMap;
  std::map<CarrierCode, std::vector<ESVSopWrapper*>> inbSopWrapperVecMap;
  int i;

  for (it = sVPairs.begin(), i = 0; it != sVPairs.end(); it++, i++)
  {
    CarrierCode outCarrier = (*it).first.front()->governingCarrier();
    CarrierCode inbCarrier = (*it).second.front()->governingCarrier();

    std::string pqTitle = generatePQtitle(pqType);
    std::string pqDivOption = generatePQDivOption(pqType);

    esvpqVec[i].title() = pqTitle;
    SopVectorsPair filteredSVPair = filterSopVectorsPairForQueue((*it).first, (*it).second, pqType);

    if ((outSopWrapperVecMap[outCarrier].size() > 0) &&
        (inbSopWrapperVecMap[inbCarrier].size() > 0))
    {
      esvpqVec[i].init(outSopWrapperVecMap[outCarrier],
                       inbSopWrapperVecMap[inbCarrier],
                       pqType,
                       "",
                       pqDivOption);
    }

    if ((outSopWrapperVecMap[outCarrier].size() > 0) &&
        (inbSopWrapperVecMap[inbCarrier].size() == 0))
    {
      esvpqVec[i].init(
          outSopWrapperVecMap[outCarrier], filteredSVPair.second, pqType, "", pqDivOption);
      inbSopWrapperVecMap[inbCarrier] = *(esvpqVec[i].legVec()[1]);
    }

    if ((outSopWrapperVecMap[outCarrier].size() == 0) &&
        (inbSopWrapperVecMap[inbCarrier].size() != 0))
    {
      esvpqVec[i].init(
          filteredSVPair.first, inbSopWrapperVecMap[inbCarrier], pqType, "", pqDivOption);
      outSopWrapperVecMap[outCarrier] = *(esvpqVec[i].legVec()[0]);
    }

    if ((outSopWrapperVecMap[outCarrier].size() == 0) &&
        (inbSopWrapperVecMap[inbCarrier].size() == 0))
    {
      esvpqVec[i].init(filteredSVPair.first, filteredSVPair.second, pqType, "", pqDivOption);
      outSopWrapperVecMap[outCarrier] = *(esvpqVec[i].legVec()[0]);
      inbSopWrapperVecMap[inbCarrier] = *(esvpqVec[i].legVec()[1]);
    }
    diagDisplay956(&(esvpqVec[i]));
  }
  generateSinglePassSolutions(esvpqVec, calcLimitForCurrPass(pqType), pqType);
}

void
EstimatedSeatValue::generateSinglePassSolutions(ESVPQ& singlePassPQ,
                                                int limitForCurrPass,
                                                QueueType qType)
{
  std::vector<ESVPQ> singlePassPQVec(1, singlePassPQ);

  generateSinglePassSolutions(singlePassPQVec, limitForCurrPass, qType);
}
void
EstimatedSeatValue::generateSinglePassSolutions(std::vector<ESVPQ>& singlePassPQVec,
                                                int limitForCurrPass,
                                                QueueType qType)
{
  TSELatencyData metrics(_trx, "GENERATE SINGLE PASS SOLUTIONS");
  bool firstItem = true;
  bool firstCorrectItemFlag = true;
  std::string addInfo = "";
  MoneyAmount minFareValue = 0.0;
  int lastChosenESVPQNo = -1;
  int numberOfSolutionsPickedInCurrPass = 0;

  if (singlePassPQVec.size() == 0)
  {
    return;
  }
  std::map<int, std::pair<ESVPQ*, ESVPQItem*>> ESVPQMarkupMap =
      buildESVPQMarkupMap(singlePassPQVec);

  ESVPQItem* pqItem = nullptr;
  while (((pqItem != nullptr) || firstItem) &&
         (numberOfSolutionsPickedInCurrPass < limitForCurrPass))
  {
    firstItem = false;

    if (_diag956Collector)
    {
      _diag956Collector->displayESVPQItem(pqItem, "", true, true);
      _diag956Collector->displayAddInfo(addInfo);
      addInfo = "";
    }

    pqItem = getNextItemFromESVPQVec(singlePassPQVec, ESVPQMarkupMap, lastChosenESVPQNo);
    if (nullptr == pqItem)
    {
      break;
    }

    if (!checkIfPQItemCanBePicked(pqItem, addInfo))
    {
      continue;
    }

    if (!checkNumberOfStops(pqItem, qType, addInfo, _trx))
    {
      continue;
    }

    if (!checkOnlineConfirmity(pqItem, qType, addInfo, _trx))
    {
      continue;
    }

    if (!_esvDiv->checkRestrictedCarriers(pqItem, addInfo))
    {
      continue;
    }

    CarrierCode carrier = "";
    if (!pqItem->checkDiversityLimits(_esvDiv, qType, carrier, addInfo))
    {
      if (!carrier.empty())
      {
        rebuildESVPQMarkupMap(ESVPQMarkupMap, lastChosenESVPQNo);
      }
      continue;
    }

    if (firstCorrectItemFlag)
    {
      minFareValue = pqItem->totalAmt();
      firstCorrectItemFlag = false;
    }

    if (!pqItem->checkUpperBoundLimits(_esvDiv, minFareValue, qType, addInfo))
    {
      break;
    }
    addInfo = "DIVC";

    _pickedItems.push_back(pqItem);
    numberOfSolutionsPickedInCurrPass++;
  }

  std::stringstream summary;
  summary << addInfo << "\n"
          << "NUMBER OF SOLUTIONS CHOSEN FOR THIS QUEUE = " << numberOfSolutionsPickedInCurrPass
          << "\n";
  diagDisplay956AddInfo(summary.str());
}

std::string
EstimatedSeatValue::generatePQtitle(QueueType pqType, CarrierCode carrier)
{
  std::stringstream title;

  switch (pqType)
  {
  case MPNonstopOnline:
    title << "MUST PRICE NONSTOP ONLINE SOLUTIONS FOR CARRIER " << carrier;
    break;
  case MPOutNonstopOnline:
    title << "MUST PRICE OUT-NONSTOP INB-SINGLE STOP ONLINE SOLUTIONS FOR CARRIER " << carrier;
    break;
  case MPNonstopInterline:
    title << "MUST PRICE NONSTOP INTERLINE SOLUTIONS FOR CARRIER " << carrier;
    break;
  case MPOutNonstopInterline:
    title << "MUST PRICE OUT-NONSTOP INB-SINGLE STOP INTERLINE SOLUTIONS FOR CARRIER " << carrier;
    break;
  case MPSingleStopOnline:
    title << "MUST PRICE SINGLE STOP ONLINE SOLUTIONS FOR CARRIER " << carrier;
    break;
  case MPRemainingOnline:
    title << "MUST PRICE REMAINING ONLINE SOLUTIONS";
    break;
  case MPRemaining:
    title << "MUST PRICE REMAINING SOLUTIONS";
    break;
  case LFSOnline:
    title << "LOW FARE SEARCH ONLINE SOLUTIONS";
    break;
  case LFSOnlineByCarrier:
    title << "LOW FARE SEARCH ONLINE BY CARRIER SOLUTIONS";
    break;
  case LFSRemaining:
    title << "LOW FARE SEARCH REMAINING SOLUTIONS";
    break;
  default:
    LOG4CXX_ERROR(_logger, "EstimatedSeatValue::generatePQtitle - Unknown queue type.");
    break;
  }
  return title.str();
}
std::string
EstimatedSeatValue::generatePQDivOption(QueueType pqType)
{
  std::string divOption;

  switch (pqType)
  {
  case MPNonstopOnline:
    divOption = "01";
    break;
  case MPOutNonstopOnline:
    divOption = "02";
    break;
  case MPNonstopInterline:
    divOption = "03";
    break;
  case MPOutNonstopInterline:
    divOption = "04";
    break;
  case MPSingleStopOnline:
    divOption = "05";
    break;
  case MPRemainingOnline:
    divOption = "11";
    break;
  case MPRemaining:
    divOption = "12";
    break;
  case LFSOnline:
    divOption = "13";
    break;
  case LFSOnlineByCarrier:
    divOption = "14";
    break;
  case LFSRemaining:
    divOption = "15";
    break;
  default:
    LOG4CXX_ERROR(_logger, "EstimatedSeatValue::generatePQDivOption - Unknown queue type.");
    break;
  }

  return divOption;
}

int
EstimatedSeatValue::calcLimitForCurrPass(QueueType pqType)
{
  int limit;

  switch (pqType)
  {
  case MPNonstopOnline:
    limit = _esvDiv->calcMustPriceLimitForCurrPass(
        _pickedItems.size(), _trx.esvOptions()->noOfMustPriceNonstopOnlineSolutions());
    break;
  case MPOutNonstopOnline:
    limit = _esvDiv->calcMustPriceLimitForCurrPass(
        _pickedItems.size(), _trx.esvOptions()->noOfMustPriceNonAndOneStopOnlineSolutions());
    break;
  case MPNonstopInterline:
    limit = _esvDiv->calcMustPriceLimitForCurrPass(
        _pickedItems.size(), _trx.esvOptions()->noOfMustPriceNonstopInterlineSolutions());
    break;
  case MPOutNonstopInterline:
    limit = _esvDiv->calcMustPriceLimitForCurrPass(
        _pickedItems.size(), _trx.esvOptions()->noOfMustPriceNonAndOneStopInterlineSolutions());
    break;
  case MPSingleStopOnline:
    limit = _esvDiv->calcMustPriceLimitForCurrPass(
        _pickedItems.size(), _trx.esvOptions()->noOfMustPriceSingleStopOnlineSolutions());
    break;
  case MPRemainingOnline:
    limit = _esvDiv->calcMustPriceLimitForCurrPass(
        _pickedItems.size(), _trx.esvOptions()->noOfMustPriceOnlineSolutions());
    break;
  case MPRemaining:
    limit = _esvDiv->calcMustPriceLimitForCurrPass(
        _pickedItems.size(), _trx.esvOptions()->noOfMustPriceInterlineSolutions());
    break;
  case LFSOnline:
    limit = _esvDiv->calcLFSLimitForPassOne(_pickedItems.size());
    break;
  case LFSOnlineByCarrier:
    limit = _esvDiv->calcLFSLimitForPassTwo(_pickedItems.size());
    break;
  case LFSRemaining:
    limit = _esvDiv->calcLFSLimitForPassThree(_pickedItems.size());
    break;
  default:
    limit = 0;
    LOG4CXX_ERROR(_logger, "EstimatedSeatValue::calcLimitForCurrPass - Unknown queue type.");
    break;
  }

  return limit;
}

ESVPQItem*
EstimatedSeatValue::getNextItemFromESVPQVec(
    std::vector<ESVPQ>& singlePassPQVec,
    std::map<int, std::pair<ESVPQ*, ESVPQItem*>>& ESVPQMarkupMap,
    int& lastChosenESVPQNo)
{
  if (lastChosenESVPQNo >= static_cast<int>(ESVPQMarkupMap.size()))
  {
    LOG4CXX_ERROR(_logger,
                  "EstimatedSeatValue::getNextItemFromESVPQVec - LAST CHOSEN NUMBER IS INCORRECT.");
  }

  // lastChosenESVPQNo = -1 means that ESVPQMarkupMap was just built or rebuilt
  // we there should be no getNextItem this time
  if (lastChosenESVPQNo >= 0)
  {
    ESVPQItem* pqItem = ESVPQMarkupMap[lastChosenESVPQNo].first->getNextItem(_diag956Collector);
    ESVPQMarkupMap[lastChosenESVPQNo].second = pqItem;
  }

  std::map<int, std::pair<ESVPQ*, ESVPQItem*>>::iterator markupIt;
  int curChosenESVPQNo = -1;

  for (markupIt = ESVPQMarkupMap.begin(); markupIt != ESVPQMarkupMap.end(); markupIt++)
  {
    if ((*markupIt).second.second == nullptr)
    {
      continue;
    }
    if ((curChosenESVPQNo == -1) ||
        ((curChosenESVPQNo >= 0) && (ESVPQMarkupMap[curChosenESVPQNo].second->totalAmt() >
                                     (*markupIt).second.second->totalAmt())))
    {
      curChosenESVPQNo = (*markupIt).first;
    }
  }

  if (curChosenESVPQNo >= 0)
  {
    lastChosenESVPQNo = curChosenESVPQNo;
    return ESVPQMarkupMap[curChosenESVPQNo].second;
  }
  return nullptr;
}

std::map<int, std::pair<ESVPQ*, ESVPQItem*>>
EstimatedSeatValue::buildESVPQMarkupMap(std::vector<ESVPQ>& singlePassPQVec)
{
  std::map<int, std::pair<ESVPQ*, ESVPQItem*>> ESVPQMarkupMap;

  for (int i = 0; i < (int)singlePassPQVec.size(); i++)
  {
    ESVPQItem* pqItem = singlePassPQVec[i].getNextItem(_diag956Collector);
    ESVPQMarkupMap[i].first = &(singlePassPQVec[i]);
    ESVPQMarkupMap[i].second = pqItem;
  }
  return ESVPQMarkupMap;
}

void
EstimatedSeatValue::rebuildESVPQMarkupMap(
    std::map<int, std::pair<ESVPQ*, ESVPQItem*>>& ESVPQMarkupMap, int& lastChosenESVPQNo)
{
  if ((lastChosenESVPQNo < 0) || (lastChosenESVPQNo >= (int)ESVPQMarkupMap.size()))
  {
    LOG4CXX_ERROR(_logger,
                  "EstimatedSeatValue::rebuildESVPQMarkupMap - LAST CHOSEN NUMBER IS INCORRECT.");
    return;
  }

  for (int i = lastChosenESVPQNo; i < (int)ESVPQMarkupMap.size() - 1; i++)
  {
    ESVPQMarkupMap[i].first = ESVPQMarkupMap[i + 1].first;
    ESVPQMarkupMap[i].second = ESVPQMarkupMap[i + 1].second;
  }

  ESVPQMarkupMap.erase(ESVPQMarkupMap.size() - 1);
  lastChosenESVPQNo = -1;
}

bool
EstimatedSeatValue::checkIfPQItemCanBePicked(ESVPQItem* pqItem,
                                             std::string& addInfo,
                                             bool checkIfNotAlreadyPickedFlag)
{
  if (checkIfNotAlreadyPickedFlag)
  {
    std::vector<ESVPQItem*>::iterator it = _pickedItems.begin();

    for (; it != _pickedItems.end(); it++)
    {
      if (*pqItem == (**it))
      {
        addInfo = "DIV1";
        return false;
      }
    }
  }

  return true;
}

void
EstimatedSeatValue::generatePerCarrierSopVectorsMaps(
    std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap,
    PaxType* paxType)
{
  TSELatencyData metrics(_trx, "ESV GENERATE PER CARRIER SOP VECTORS MAP");
  LOG4CXX_DEBUG(_logger,
                "std::map<CarrierCode, ESVPQ> generateMustPricePerCarrierPQs(PaxType* paxType)");

  std::vector<ShoppingTrx::Leg>& legVec = _trx.legs();
  std::vector<ShoppingTrx::Leg>::iterator legIter = legVec.begin();

  if (legIter == legVec.end())
  {
    return;
  }

  outPerCarrierMap = generatePerCarrierSingleSopVectorsMap(legIter);

  legIter++;

  if (legIter != legVec.end())
  {
    inbPerCarrierMap = generatePerCarrierSingleSopVectorsMap(legIter);
  }
  else
  {
    inbPerCarrierMap = generateDummySingleSopVectorsMap(outPerCarrierMap);
  }
}
std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>
EstimatedSeatValue::generatePerCarrierSingleSopVectorsMap(
    std::vector<ShoppingTrx::Leg>::iterator legIter)
{
  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>> perCarrierMap;

  std::vector<ShoppingTrx::SchedulingOption>& sopVec = legIter->sop();
  std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter;

  for (sopIter = sopVec.begin(); sopIter != sopVec.end(); ++sopIter)
  {
    if (sopIter->getDummy())
    {
      continue;
    }

    perCarrierMap[sopIter->governingCarrier()].push_back(&*sopIter);
    perCarrierMap[""].push_back(&*sopIter);
  }
  return perCarrierMap;
}

std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>
EstimatedSeatValue::generateDummySingleSopVectorsMap(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap)
{
  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>> perCarrierMap;

  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>::const_iterator outPerCxrIt;
  std::vector<ShoppingTrx::SchedulingOption*> dummyInboudVecForOneWays;

  for (outPerCxrIt = outPerCarrierMap.begin(); outPerCxrIt != outPerCarrierMap.end(); ++outPerCxrIt)
  {
    CarrierCode carrier = (*outPerCxrIt).first;
    perCarrierMap[carrier] = dummyInboudVecForOneWays;
  }
  perCarrierMap[""] = dummyInboudVecForOneWays;

  return perCarrierMap;
}

std::map<CarrierCode, EstimatedSeatValue::SopVectorsPair>
EstimatedSeatValue::generatePerCarrierSopVectorsPairs(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap)
{
  TSELatencyData metrics(_trx, "ESV GENERATE PER CARRIER PQS");
  LOG4CXX_DEBUG(_logger,
                "std::map<CarrierCode, ESVPQ> generateMustPricePerCarrierPQs(PaxType* paxType)");

  std::map<CarrierCode, SopVectorsPair> perCarrierSVPairMap;

  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>::const_iterator outIt;
  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>::const_iterator inbIt;

  for (outIt = outPerCarrierMap.begin(); outIt != outPerCarrierMap.end(); ++outIt)
  {
    CarrierCode carrier = (*outIt).first;
    inbIt = inbPerCarrierMap.find(carrier);
    if ((carrier.empty()) || (inbIt == inbPerCarrierMap.end()))
    {
      continue;
    }
    EstimatedSeatValue::SopVectorsPair sVPair((*outIt).second, (*inbIt).second);
    perCarrierSVPairMap[carrier] = sVPair;
  }

  return perCarrierSVPairMap;
}

std::vector<EstimatedSeatValue::SopVectorsPair>
EstimatedSeatValue::generateSopVectorsPairs(
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& outPerCarrierMap,
    const std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>& inbPerCarrierMap)
{
  TSELatencyData metrics(_trx, "ESV GENERATE SOP VECTOR PAIRS");
  LOG4CXX_DEBUG(_logger, "generateSopVectorsPairs()");

  std::vector<SopVectorsPair> sVPairs;

  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>::const_iterator outIt;
  std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>>::const_iterator inbIt;

  for (outIt = outPerCarrierMap.begin(); outIt != outPerCarrierMap.end(); ++outIt)
  {
    CarrierCode outCarrier = (*outIt).first;
    if (outCarrier.empty())
    {
      continue;
    }

    for (inbIt = inbPerCarrierMap.begin(); inbIt != inbPerCarrierMap.end(); inbIt++)
    {
      CarrierCode inbCarrier = (*inbIt).first;
      if (inbCarrier.empty())
      {
        continue;
      }
      EstimatedSeatValue::SopVectorsPair sVPair((*outIt).second, (*inbIt).second);
      sVPairs.push_back(sVPair);
    }
  }

  return sVPairs;
}

EstimatedSeatValue::SopVectorsPair
EstimatedSeatValue::filterSopVectorsPairForQueue(
    const std::vector<ShoppingTrx::SchedulingOption*>& outVec,
    const std::vector<ShoppingTrx::SchedulingOption*>& inVec,
    QueueType qType)
{
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::filterSopVectorsPairForQueue()");

  SopVectorsPair filteredSVPair;
  if ((inVec.size() == 0) && ((qType == MPOutNonstopOnline) || (qType == MPOutNonstopInterline) ||
                              (qType == MPNonstopInterline)))

  {
    return filteredSVPair;
  }

  std::vector<ShoppingTrx::SchedulingOption*> chosenOutSopVec =
      filterSingleSopVectorForQueue(outVec, 0, qType);
  std::vector<ShoppingTrx::SchedulingOption*> chosenInSopVec =
      filterSingleSopVectorForQueue(inVec, 1, qType);

  filteredSVPair.first = chosenOutSopVec;
  filteredSVPair.second = chosenInSopVec;

  return filteredSVPair;
}

std::vector<ShoppingTrx::SchedulingOption*>
EstimatedSeatValue::filterSingleSopVectorForQueue(
    const std::vector<ShoppingTrx::SchedulingOption*>& sopVec, int legNo, QueueType qType)
{
  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator sopIter;
  std::vector<ShoppingTrx::SchedulingOption*> chosenSopVec;

  for (sopIter = sopVec.begin(); sopIter != sopVec.end(); ++sopIter)
  {
    if ((*sopIter)->getDummy())
    {
      continue;
    }

    if (!EstimatedSeatValue::checkNumberOfStops((*sopIter), qType, legNo))
    {
      continue;
    }
    if (!EstimatedSeatValue::checkOnlineConfirmity((*sopIter), qType))
    {
      continue;
    }

    chosenSopVec.push_back(*sopIter);
  }
  return chosenSopVec;
}
void
EstimatedSeatValue::diagDisplay956(ESVPQ* esvpq)
{
  if (esvpq == nullptr)
  {
    return;
  }
  if (_trx.diagnostic().diagnosticType() == Diagnostic956)
  {
    _diag956Collector = dynamic_cast<Diag956Collector*>(DCFactory::instance()->create(_trx));
    TSE_ASSERT(_diag956Collector != nullptr);

    _diag956Collector->activate();

    if (!_diag956Collector->supportedPQType(esvpq->carrier(), esvpq->diversityOption()))
    {
      _diag956Collector->deActivate();
      _diag956Collector = nullptr;
    }
    else
    {
      _diag956Collector->displayESVPQ(esvpq);
    }
  }
}

void
EstimatedSeatValue::diagDisplayAllSopsInLegs(
    std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>> outPerCarrierMap,
    std::map<CarrierCode, std::vector<ShoppingTrx::SchedulingOption*>> inbPerCarrierMap)
{
  std::vector<std::vector<ShoppingTrx::SchedulingOption*>> legVec;

  if (_diag956Collector)
  {
    legVec.push_back(outPerCarrierMap[""]);
    legVec.push_back(inbPerCarrierMap[""]);

    _diag956Collector->displayAddInfo("ALL SOPS IN LEGS:\n");
    _diag956Collector->displayDiagLegsContent(legVec);
  }
}

void
EstimatedSeatValue::diagDisplay956AddInfo(const std::string& addInfo)
{
  if (_diag956Collector)
  {
    _diag956Collector->displayAddInfo(addInfo);
  }
}

void
EstimatedSeatValue::diagDisplay957(std::vector<ESVPQItem*>& solutions)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic957)
  {
    _diag957Collector = dynamic_cast<Diag957Collector*>(DCFactory::instance()->create(_trx));
    TSE_ASSERT(_diag957Collector != nullptr);

    _diag957Collector->activate();
    _diag957Collector->displaySolutions(solutions);
    _diag957Collector->flushMsg();
  }
}

// ================================= ESVSopWrapper ============================

void
ESVSopWrapper::init(PaxType* paxType,
                    ShoppingTrx::SchedulingOption* sop,
                    SOPFareListType sopFareListType,
                    int sopFareListIdx,
                    std::vector<PaxTypeFare*>* paxTypeFareVec,
                    const SOPFarePath* sopFarePath)
{
  _paxType = paxType;
  _sop = sop;
  _sopFareListType = sopFareListType;
  _sopFareListIdx = sopFareListIdx;
  _paxTypeFareVec = paxTypeFareVec;
  _haveCat10Restrictions = sopFarePath->haveCat10Restrictions();
  _combinationType = sopFarePath->combinationType();

  setTotalAmt();
}

void
ESVSopWrapper::setTotalAmt()
{
  SOPFareList& sopFareList = _sop->itin()->paxTypeSOPFareListMap()[_paxType];

  if (_sopFareListType == OW)
  {
    _totalAmt = sopFareList.owSopFarePaths()[_sopFareListIdx]->totalAmount();
  }
  else if (_sopFareListType == RT)
  {
    _totalAmt = sopFareList.rtSopFarePaths()[_sopFareListIdx]->totalAmount();
  }
  else if (_sopFareListType == OJ)
  {
    _totalAmt = sopFareList.ojSopFarePaths()[_sopFareListIdx]->totalAmount();
  }
  else if (_sopFareListType == CT)
  {
    _totalAmt = sopFareList.ctSopFarePaths()[_sopFareListIdx]->totalAmount();
  }
}

/******************************************************************************
  ***                          NEW LOGIC                                    ***
  *****************************************************************************
  1. Select LFS solutions so that each outbound option is used exactly once. Move them to container
  2. Select flights from all outbounds according to requirements (cxr, time bucket, elapset time,
  non-stop connection). Move all of them to container with appriopriate priority.
  3. For the rest of OB options from not selectes in point 2 calculate FlightUtility and IV. Sort
  and fullfill the remaining number of requested flights if needed.
  4. Sort container by priority
  5. Select requested number of outbounds from container
*/
void
EstimatedSeatValue::newLogic()
{
  TSELatencyData metrics(_trx, "newLogic");
  LOG4CXX_DEBUG(_logger, "newLogic");

  std::vector<ESVPQItem*> lfsSolutions;
  std::vector<ESVPQItem*> selectedFlights;
  std::vector<ESVPQItem*> remainingFlights;
  std::vector<ESVPQItem*> finalFlightsOutbound;
  std::vector<ESVPQItem*> finalFlights;
  std::vector<ESVSopWrapper*> inWrapVec;

  if (nullptr == _trx.visOptions())
  {
    LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::newLogic() - VIS Options object are NULL.");
    return;
  }

  if (_trx.diagnostic().diagnosticType() == Diagnostic958)
  {
    _diag958Collector = dynamic_cast<Diag958Collector*>(DCFactory::instance()->create(_trx));

    TSE_ASSERT(_diag958Collector != nullptr);

    _diag958Collector->activate();

    _diag958Collector->initDiag(_trx);

    _diag958Collector->printDiagHeader(_trx);

    _diag958Collector->printVISOptions(*(_trx.visOptions()));
  }

  Itin* firstItin = _trx.legs()[0].sop()[0].itin();
  const TravelSeg* const tsDep = firstItin->firstTravelSeg();
  const TravelSeg* const tsArr = firstItin->lastTravelSeg();
  const Loc& orig = *(tsDep->origin());
  const Loc& dest = *(tsArr->destination());
  short utcoffset = 0;
  const DateTime& depDate = tsDep->departureDT();
  const DateTime& arrDate = tsArr->arrivalDT();
  LocUtil::getUtcOffsetDifference(orig, dest, utcoffset, _trx.dataHandle(), depDate, arrDate);
  utcoffset /= 60; // from minutes to hours
  calculateFlightTimeMinutes(utcoffset);

  uint16_t miles = LocUtil::getTPM(orig, dest, GlobalDirection::ZZ, depDate, _trx.dataHandle());
  uint16_t milesOrig = miles;

  if (miles < 500)
    miles = 500;
  else if (miles < 1000)
    miles = 1000;
  else if (miles < 1500)
    miles = 1500;
  else if (miles < 2000)
    miles = 2000;
  else if (miles < 2500)
    miles = 2500;
  else if (miles < 3000)
    miles = 3000;
  else
    miles = 4000;

  char ap = 'F';
  boost::posix_time::time_duration diff = depDate - _trx.ticketingDate();
  if (diff.hours() / 24 < 14) // 14 days
    ap = 'C';

  if (_diag958Collector)
  {
    _diag958Collector->printInitialInfo(
        orig, dest, utcoffset, depDate, _trx.bookingDate(), milesOrig, miles, ap);
  }

  _paramBetaOut = getParameterBeta(utcoffset, miles, 'O', ap);
  _paramBetaIn = getParameterBeta(utcoffset, miles, 'I', ap);

  if (!validBeta(_paramBetaOut))
  {
    _paramBetaOut = nullptr;
  }

  if (!validBeta(_paramBetaIn))
  {
    _paramBetaIn = nullptr;
  }

  // copy pointers to all SOPs
  std::vector<ShoppingTrx::SchedulingOption*> allOut;
  std::vector<ShoppingTrx::SchedulingOption*> allIn;
  copySOPptr(_trx.legs()[0].sop(), allOut);
  _outSize = allOut.size();

  if (!_oneWay)
  {
    copySOPptr(_trx.legs()[1].sop(), allIn);
    _inSize = allIn.size();
  }

  // Remove outbound and inbound dominated flights
  DominatedFlights dominatedFlights(_trx);
  dominatedFlights.findDominatedFlights(allOut, allIn);

  if (_diag958Collector)
  {
    _diag958Collector->printFlightsList(_trx);
  }

  dominatedFlights.removeDominatedFlights(allOut, allIn);

  // Find LFS solutions and copy them to container
  if (_oneWay)
  {
    selectLowFareAllOneWay(allOut,
                           lfsSolutions,
                           _trx.visOptions()->visLowFareSearch().noOfLFSItineraries(),
                           remainingFlights);
  }
  else
  {
    selectLowFareAllRoundTrip(allOut,
                              allIn,
                              lfsSolutions,
                              _trx.visOptions()->visLowFareSearch().noOfLFSItineraries(),
                              inWrapVec);
    findLFSforAllOut(allOut, inWrapVec, remainingFlights);
  }

  if (_diag958Collector)
  {
    _diag958Collector->printESVPQItems(lfsSolutions, "LFS solutions");
  }

  // Finish processing flights if we weren't able to find any solution.
  if (lfsSolutions.empty())
  {
    return;
  }

  bool generateIVOptions = generateIVOptionsCfg.getValue();

  const MoneyAmount cheapestItinerary = lfsSolutions[0]->totalPrice();

  // Get parameters from request
  const int noOfOutbounds = _oneWay
                                ? _trx.visOptions()->valueBasedItinSelection().noOfOutboundsOW()
                                : _trx.visOptions()->valueBasedItinSelection().noOfOutboundsRT();

  // Find all must have outbound options
  selectInitialOBSet(
      allOut, inWrapVec, selectedFlights, remainingFlights, noOfOutbounds, cheapestItinerary);

  // If we weren't able to find requested number of outbound options add missing
  // options by using incremental value criterion

  if ((!remainingFlights.empty()) && generateIVOptions)
  {
    const int noOfOptions = noOfOutbounds - selectedFlights.size();

    selectByIncrementalValue(remainingFlights, outLegIdx, -1, selectedFlights, noOfOptions);
  }

  // if we don't meet the no. of requested outbounds, then the no. of inbounds is the greater of the
  // following:
  // - the number of requested inbounds
  // - the number of requested solutions divided by the no. of outbounds found
  int noOfRequestedInbounds = _trx.visOptions()->valueBasedItinSelection().noOfInboundsRT();
  int noOfActualOutbounds = selectedFlights.size();

  const int noOfInbounds =
      ((noOfActualOutbounds < noOfOutbounds) &&
       ((noOfActualOutbounds * noOfRequestedInbounds) <
        _trx.esvOptions()->getRequestedNumberOfSolutions()))
          ? ((_trx.esvOptions()->getRequestedNumberOfSolutions() + noOfActualOutbounds - 1) /
             noOfActualOutbounds)
          : noOfRequestedInbounds;

  addLFSOptionsToFlightsSelection(lfsSolutions, outLegIdx, nullptr, selectedFlights);

  if (_diag958Collector)
  {
    _diag958Collector->printSelectedFlights(selectedFlights, true);
  }

  if (_oneWay)
  {
    finalFlights.insert(finalFlights.end(), selectedFlights.begin(), selectedFlights.end());
  }
  else
  {
    finalFlightsOutbound.insert(
        finalFlightsOutbound.end(), selectedFlights.begin(), selectedFlights.end());

    int numSelectedOB = finalFlightsOutbound.size();
    // for each outbound flight find the inbound flight
    for (int outIndex = 0; outIndex < numSelectedOB; ++outIndex)
    {
      _numFound = 0;

      selectedFlights.clear();
      remainingFlights.clear();

      ESVPQItem* obItin = finalFlightsOutbound[outIndex];

      selectInitialIBSet(outIndex,
                         remainingFlights,
                         selectedFlights,
                         allIn,
                         noOfInbounds,
                         cheapestItinerary,
                         obItin);

      if ((!remainingFlights.empty()) && generateIVOptions)
      {
        const short noOfOptions = (const short)(noOfInbounds - selectedFlights.size());

        selectByIncrementalValue(
            remainingFlights, inLegIdx, outIndex, selectedFlights, noOfOptions);
      }

      addLFSOptionsToFlightsSelection(lfsSolutions, inLegIdx, obItin, selectedFlights);

      if (_diag958Collector)
      {
        _diag958Collector->printSelectedFlights(selectedFlights, false);
      }

      finalFlights.insert(finalFlights.end(), selectedFlights.begin(), selectedFlights.end());
    }
  }

  sendSolutions(finalFlights);
}

void
EstimatedSeatValue::selectInitialIBSet(const int outIndex,
                                       std::vector<ESVPQItem*>& remainingFlights,
                                       std::vector<ESVPQItem*>& selectedFlights,
                                       const std::vector<ShoppingTrx::SchedulingOption*>& allIn,
                                       const short noOfOptions,
                                       const MoneyAmount cheapestItinerary,
                                       const ESVPQItem* obItin)
{
  TSELatencyData metrics(_trx, "ESV SELECT INITIAL IB SET");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::selectInitialIBSet");

  short noOfItemsFound = 0;

  initRemainingFlights(obItin, allIn, remainingFlights);

  std::vector<std::pair<VisBucket, short>> prioritiesVector;
  preparePriorityVector(prioritiesVector, inLegIdx);

  std::vector<std::pair<VisBucket, short>>::const_iterator priorityIter;

  for (priorityIter = prioritiesVector.begin(); priorityIter != prioritiesVector.end();
       ++priorityIter)
  {
    const std::pair<VisBucket, short>& bucket = (*priorityIter);

    if (noOfItemsFound >= noOfOptions)
    {
      break;
    }

    switch (bucket.first)
    {
    case LFS:
      noOfItemsFound += selectLFSSolutions(remainingFlights, outIndex);
      break;

    case TB:
      noOfItemsFound += selectByTimeBucket(remainingFlights, inLegIdx, outIndex);
      break;

    case ET:
      noOfItemsFound += selectByFlightTime(remainingFlights, inLegIdx, outIndex);
      break;

    case UV:
      noOfItemsFound += selectByUtilityValue(remainingFlights, inLegIdx, outIndex);
      break;

    case SI:
      if (0 == obItin->getNumStops(outLegIdx))
      {
        noOfItemsFound += selectSimpleInterlines(remainingFlights, outIndex, obItin);
      }
      break;

    case NS:
      noOfItemsFound += selectByNumStops(remainingFlights, inLegIdx, outIndex, cheapestItinerary);
      break;

    default:
      LOG4CXX_ERROR(_logger, "EstimatedSeatValue::selectInitialIBSet - Unknown bucket.");
      break;
    }
  }

  finalizeBucketsSelection(remainingFlights, inLegIdx, selectedFlights, noOfOptions);
}

short
EstimatedSeatValue::selectSimpleInterlines(std::vector<ESVPQItem*>& remainingFlights,
                                           const int outIndex,
                                           const ESVPQItem* outboundItin)
{
  TSELatencyData metrics(_trx, "ESV SELECT SIMPLE INTERLINES");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::selectSimpleInterlines");

  short noOfItemsFound = 0;

  // Simple interline is itinerary where:
  // 1. OB flight is nonstop
  // 2. IB flight is nonstop
  // 3. OB and IB carriers are different

  const int simpleInterlinePriority =
      _trx.visOptions()->visInboundSelectionRT().simpleInterlinePriority();

  const int noOfSimpleInterlineOptions =
      _trx.visOptions()->visInboundSelectionRT().noOfSimpleInterlineOptions();

  if (0 == noOfSimpleInterlineOptions)
  {
    return noOfItemsFound;
  }

  std::sort(remainingFlights.begin(), remainingFlights.end(), NumstopsESVComparator(inLegIdx));

  const CarrierCode& outboundCarrier =
      outboundItin->outSopWrapper()->sop()->itin()->onlineCarrier();

  short noOfSimpleInterlineOptionsFound = 0;

  for (uint32_t n = 0; n < remainingFlights.size(); ++n)
  {
    if (noOfSimpleInterlineOptionsFound >= noOfSimpleInterlineOptions)
    {
      break;
    }

    if (0 != remainingFlights[n]->getNumStops(inLegIdx))
    {
      break;
    }

    const CarrierCode& currentCarrier =
        remainingFlights[n]->inSopWrapper()->sop()->itin()->onlineCarrier();

    if (currentCarrier != outboundCarrier)
    {
      noOfItemsFound += markPQItem(remainingFlights[n], simpleInterlinePriority, "B-SI");
      remainingFlights[n]->screenID() = outIndex;

      ++noOfSimpleInterlineOptionsFound;
    }
  }

  return noOfItemsFound;
}

short
EstimatedSeatValue::selectLFSSolutions(std::vector<ESVPQItem*>& remainingFlights,
                                       const int outIndex)
{
  TSELatencyData metrics(_trx, "ESV SELECT LFS SOLUTIONS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::selectLFSSolutions");

  short noOfItemsFound = 0;

  const int lowestFarePriority = _trx.visOptions()->visInboundSelectionRT().lowestFarePriority();

  const int noOfLFSOptions = _trx.visOptions()->visInboundSelectionRT().noOfLFSOptions();

  if (0 == noOfLFSOptions)
  {
    return noOfItemsFound;
  }

  std::sort(remainingFlights.begin(), remainingFlights.end(), ESVComparator(inLegIdx));

  short noOfLFSOptionsFound = 0;

  for (uint32_t n = 0; n < remainingFlights.size(); ++n)
  {
    if (noOfLFSOptionsFound >= noOfLFSOptions)
    {
      break;
    }

    noOfItemsFound += markPQItem(remainingFlights[n], lowestFarePriority, "B-LFS");
    remainingFlights[n]->screenID() = outIndex;

    ++noOfLFSOptionsFound;
  }

  return noOfItemsFound;
}

// (out, inY1), ..., (out, inYN) where N = inbound.size() - all possible.
void
EstimatedSeatValue::initRemainingFlights(const ESVPQItem* obItin,
                                         const std::vector<ShoppingTrx::SchedulingOption*>& allIn,
                                         std::vector<ESVPQItem*>& remainingFlights)
{
  TSELatencyData metrics(_trx, "ESV::initRemainingFlights");
  LOG4CXX_DEBUG(_logger, "ESV::initRemainingFlights");

  ShoppingTrx::SchedulingOption* outSop =
      (ShoppingTrx::SchedulingOption*)(obItin->outSopWrapper()->sop());
  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator it = allIn.begin();
  const std::vector<ShoppingTrx::SchedulingOption*>::const_iterator itEnd = allIn.end();
  std::vector<ShoppingTrx::SchedulingOption*> outSopPtrVec(1, outSop);
  std::vector<ShoppingTrx::SchedulingOption*> inSopPtrVec(1);
  ESVPQItem* pqItem = nullptr;
  const CarrierCode cxr = "";
  const std::string title = "initRemainingFlights";

  VISPQ pq(&_trx, _paramBetaOut, _paramBetaIn, title);
  const std::vector<ESVSopWrapper*>& outSopWrapperVec = outSop->itin()->sopWrapperVec();

  for (; it != itEnd; ++it)
  {
    std::vector<ESVSopWrapper*>& inSopWrapperVec = (*it)->itin()->sopWrapperVec();
    if (inSopWrapperVec.size() == 0)
    {
      inSopPtrVec[0] = *it;
      if (outSopWrapperVec.size() == 0)
        pq.init(outSopPtrVec, inSopPtrVec, LFSRemaining, cxr, title);
      else
        pq.init(outSopWrapperVec, inSopPtrVec, LFSRemaining, cxr, title);
    }
    else
    {
      if (outSopWrapperVec.size() == 0)
        pq.init(outSopPtrVec, inSopWrapperVec, LFSRemaining, cxr, title);
      else
        pq.init(outSopWrapperVec, inSopWrapperVec, LFSRemaining, cxr, title);
    }

    pqItem = pq.getNextItem(_diag956Collector, true);

    if (pqItem)
    {
      pq.computeUtilityValuesForPQItem(pqItem);
      remainingFlights.push_back(pqItem);
    }
  }
}

void
EstimatedSeatValue::calculateIV(ESVPQItem* tempFlight,
                                const std::vector<ESVPQItem*>& selectedFlights,
                                const std::vector<double>* paramBeta,
                                const int legIdx,
                                const std::vector<double>& DT,
                                const std::vector<bool>& existDT)
{
  TSELatencyData metrics(_trx, "ESV CALCULATE IV");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::calculateIV");

  if (nullptr == paramBeta)
  {
    return;
  }

  double mu = (*paramBeta)[12];
  double marginalProb = 0.0;
  double denominator = 0.0;

  if (selectedFlights.empty())
  {
    tempFlight->IV() = 100.0;
  }
  else
  {
    int nestID = tempFlight->nestID(legIdx);

    if ((nestID < 0) || (nestID > 11))
    {
      LOG4CXX_ERROR(_logger, "EstimatedSeatValue::calculateIV - Nest ID out of range");
      nestID = 0;
    }

    double tempDT = exp(tempFlight->getUtility(legIdx)) + DT[nestID];

    denominator = exp(mu * log(tempDT));

    for (uint32_t i = 0; i < DT.size(); ++i)
    {
      if (existDT[i] && (i != (uint32_t)nestID))
      {
        denominator += exp(mu * log(DT[i]));
      }
    }

    marginalProb = exp(mu * log(tempDT)) / denominator;
    tempFlight->IV() = marginalProb * exp(tempFlight->getUtility(legIdx)) / tempDT * 100;
  }
}

void
EstimatedSeatValue::prepareDTVectors(const std::vector<ESVPQItem*>& selectedFlights,
                                     const int legIdx,
                                     std::vector<double>& DT,
                                     std::vector<bool>& existDT)
{
  TSELatencyData metrics(_trx, "ESV PREPARE DT VECTORS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::prepareDTVectors");

  DT.resize(12, 0.0);
  existDT.resize(12, false);

  for (uint32_t i = 0; i < selectedFlights.size(); ++i)
  {
    int nestID = selectedFlights[i]->nestID(legIdx);

    if ((nestID < 0) || (nestID > 11))
    {
      LOG4CXX_ERROR(_logger, "EstimatedSeatValue::prepareDTVectors - Nest ID out of range");
      nestID = 0;
    }

    DT[nestID] = exp(selectedFlights[i]->utility(legIdx)) + DT[nestID];
    existDT[nestID] = true;
  }
}

void
EstimatedSeatValue::selectInitialOBSet(const std::vector<ShoppingTrx::SchedulingOption*>& allOut,
                                       const std::vector<ESVSopWrapper*>& inWrapVec,
                                       std::vector<ESVPQItem*>& selectedFlights,
                                       std::vector<ESVPQItem*>& remainingFlights,
                                       const short noOfOptions,
                                       const MoneyAmount cheapestItinerary)
{
  TSELatencyData metrics(_trx, "ESV SELECT INITIAL OB SET");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::selectInitialOBSet");

  short noOfItemsFound = 0;

  std::vector<std::pair<VisBucket, short>> prioritiesVector;
  preparePriorityVector(prioritiesVector, outLegIdx);

  std::vector<std::pair<VisBucket, short>>::const_iterator priorityIter;

  for (priorityIter = prioritiesVector.begin(); priorityIter != prioritiesVector.end();
       ++priorityIter)
  {
    const std::pair<VisBucket, short>& bucket = (*priorityIter);

    if (noOfItemsFound >= noOfOptions)
    {
      break;
    }

    switch (bucket.first)
    {
    case CR:
      noOfItemsFound += selectByCarrier(remainingFlights);
      break;

    case TB:
      noOfItemsFound += selectByTimeBucket(remainingFlights, outLegIdx, -1);
      break;

    case ET:
      noOfItemsFound += selectByFlightTime(remainingFlights, outLegIdx, -1);
      break;

    case UV:
      noOfItemsFound += selectByUtilityValue(remainingFlights, outLegIdx, -1);
      break;

    case NS:
      noOfItemsFound += selectByNumStops(remainingFlights, outLegIdx, -1, cheapestItinerary);
      break;

    default:
      LOG4CXX_ERROR(_logger, "EstimatedSeatValue::selectInitialOBSet - Unknown bucket.");
      break;
    }
  }

  finalizeBucketsSelection(remainingFlights, outLegIdx, selectedFlights, noOfOptions);
}

void
EstimatedSeatValue::preparePriorityVector(
    std::vector<std::pair<VisBucket, short>>& prioritiesVector, const int legIdx)
{
  short carrierPriority = 0;
  short lowestFarePriority = 0;
  short timeBucketPriority = 0;
  short elapsedTimePriority = 0;
  short utilityValuePriority = 0;
  short simpleInterlinePriority = 0;
  short nonStopPriority = 0;

  // Get carrier priority
  if (outLegIdx == legIdx)
  {
    carrierPriority = _oneWay ? _trx.visOptions()->visOutboundSelectionOW().carrierPriority()
                              : _trx.visOptions()->visOutboundSelectionRT().carrierPriority();

    prioritiesVector.push_back(std::pair<VisBucket, short>(CR, carrierPriority));
  }

  // Get LFS priority
  if (inLegIdx == legIdx)
  {
    lowestFarePriority = _trx.visOptions()->visInboundSelectionRT().lowestFarePriority();

    prioritiesVector.push_back(std::pair<VisBucket, short>(LFS, lowestFarePriority));
  }

  // Get time bucket priority
  timeBucketPriority = _oneWay
                           ? _trx.visOptions()->visOutboundSelectionOW().timeOfDayPriority()
                           : (legIdx == outLegIdx)
                                 ? _trx.visOptions()->visOutboundSelectionRT().timeOfDayPriority()
                                 : _trx.visOptions()->visInboundSelectionRT().timeOfDayPriority();

  prioritiesVector.push_back(std::pair<VisBucket, short>(TB, timeBucketPriority));

  // Get elapsed time priority
  elapsedTimePriority =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().elapsedTimePriority()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visOutboundSelectionRT().elapsedTimePriority()
                    : _trx.visOptions()->visInboundSelectionRT().elapsedTimePriority();

  prioritiesVector.push_back(std::pair<VisBucket, short>(ET, elapsedTimePriority));

  // Get utility value priority
  utilityValuePriority =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().utilityValuePriority()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visOutboundSelectionRT().utilityValuePriority()
                    : _trx.visOptions()->visInboundSelectionRT().utilityValuePriority();

  prioritiesVector.push_back(std::pair<VisBucket, short>(UV, utilityValuePriority));

  // Get simple interline priority
  if (inLegIdx == legIdx)
  {
    simpleInterlinePriority = _trx.visOptions()->visInboundSelectionRT().simpleInterlinePriority();

    prioritiesVector.push_back(std::pair<VisBucket, short>(SI, simpleInterlinePriority));
  }

  // Get non-stop priority
  nonStopPriority = _oneWay ? _trx.visOptions()->visOutboundSelectionOW().nonStopPriority()
                            : (legIdx == outLegIdx)
                                  ? _trx.visOptions()->visOutboundSelectionRT().nonStopPriority()
                                  : _trx.visOptions()->visInboundSelectionRT().nonStopPriority();

  prioritiesVector.push_back(std::pair<VisBucket, short>(NS, nonStopPriority));

  std::sort(prioritiesVector.begin(), prioritiesVector.end(), PriorityComparator());
}

short
EstimatedSeatValue::selectByTimeBucket(std::vector<ESVPQItem*>& remainingFlights,
                                       const int legIdx,
                                       const int outIndex)
{
  TSELatencyData metrics(_trx, "ESV SELECT BY TIME BUCKET");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::selectByTimeBucket");

  short noOfItemsFound = 0;

  const unsigned int noOfOptionsPerBin =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().noOfOptionsPerTimeBin()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visOutboundSelectionRT().noOfOptionsPerTimeBin()
                    : _trx.visOptions()->visInboundSelectionRT().noOfOptionsPerTimeBin();

  const int numOfBins =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().timeOfDayBins().size()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visOutboundSelectionRT().timeOfDayBins().size()
                    : _trx.visOptions()->visInboundSelectionRT().timeOfDayBins().size();

  const int timeBucketPriority =
      _oneWay
          ? _trx.visOptions()->visOutboundSelectionOW().timeOfDayPriority()
          : (legIdx == outLegIdx) ? _trx.visOptions()->visOutboundSelectionRT().timeOfDayPriority()
                                  : _trx.visOptions()->visInboundSelectionRT().timeOfDayPriority();

  if (0 == noOfOptionsPerBin)
  {
    return noOfItemsFound;
  }

  std::map<const VISTimeBin*, std::vector<ESVPQItem*>> timeBuckets;
  std::sort(remainingFlights.begin(), remainingFlights.end(), ESVComparator(legIdx));

  int numOfFullBins = 0;

  std::vector<ESVPQItem*>::const_iterator it = remainingFlights.begin();
  const std::vector<ESVPQItem*>::const_iterator itEnd = remainingFlights.end();

  while ((it != itEnd) && (numOfFullBins < numOfBins))
  {
    ESVPQItem* const pqItem = *it;
    const VISTimeBin* const timeBin = getTimeBin(pqItem, legIdx);

    if (timeBin)
    {
      std::vector<ESVPQItem*>& binVec = timeBuckets[timeBin];

      if (binVec.size() < noOfOptionsPerBin)
      {
        noOfItemsFound += markPQItem(pqItem, timeBucketPriority, "B-TB");

        if (outIndex != -1)
        {
          pqItem->screenID() = outIndex;
        }

        binVec.push_back(pqItem);

        if (binVec.size() == noOfOptionsPerBin)
        {
          ++numOfFullBins;
        }
      }
    }

    ++it;
  }

  return noOfItemsFound;
}

VISTimeBin*
EstimatedSeatValue::getTimeBin(const ESVPQItem* pqItem, const int legIdx) const
{
  const int deptHours = pqItem->getDepartTime(legIdx).hours();
  const int deptMinutes = pqItem->getDepartTime(legIdx).minutes();
  const int deptTime = deptHours * 100 + deptMinutes;
  VISTimeBin* result = nullptr;
  std::vector<VISTimeBin>& timeOfDayBins =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().timeOfDayBins()
              : (legIdx == outLegIdx) ? _trx.visOptions()->visOutboundSelectionRT().timeOfDayBins()
                                      : _trx.visOptions()->visInboundSelectionRT().timeOfDayBins();
  std::vector<VISTimeBin>::iterator it;
  std::vector<VISTimeBin>::iterator itEnd;
  it = timeOfDayBins.begin();
  itEnd = timeOfDayBins.end();

  bool found = false;
  while (!found && (it != itEnd))
  {
    const short& bt = (*it).beginTime();
    const short& et = (*it).endTime();

    if ((deptTime >= bt) && (deptTime <= et))
    {
      found = true;
      result = &(*it);
    }
    else
      ++it;
  }
  return result;
}

short
EstimatedSeatValue::selectByFlightTime(std::vector<ESVPQItem*>& remainingFlights,
                                       const int legIdx,
                                       const int outIndex)
{
  TSELatencyData metrics(_trx, "ESV SELECT BY FLIGHT TIME");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::selectByFlightTime");

  short noOfItemsFound = 0;

  const int elapsedTimePriority =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().elapsedTimePriority()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visOutboundSelectionRT().elapsedTimePriority()
                    : _trx.visOptions()->visInboundSelectionRT().elapsedTimePriority();

  const int numOfElapsedTimeOptions =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().noOfElapsedTimeOptions()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visOutboundSelectionRT().noOfElapsedTimeOptions()
                    : _trx.visOptions()->visInboundSelectionRT().noOfElapsedTimeOptions();

  if (0 == numOfElapsedTimeOptions)
  {
    return noOfItemsFound;
  }

  std::sort(remainingFlights.begin(), remainingFlights.end(), ElapsedComparator(legIdx));

  int numSelected = 0;
  std::vector<ESVPQItem*>::iterator it = remainingFlights.begin();
  const std::vector<ESVPQItem*>::const_iterator itEnd = remainingFlights.end();

  while ((it != itEnd) && (numSelected < numOfElapsedTimeOptions))
  {
    noOfItemsFound += markPQItem(*it, elapsedTimePriority, "B-ET");

    if (outIndex != -1)
    {
      (*it)->screenID() = outIndex;
    }

    ++numSelected;
    ++it;
  }

  return noOfItemsFound;
}

short
EstimatedSeatValue::selectByNumStops(std::vector<ESVPQItem*>& remainingFlights,
                                     const int legIdx,
                                     const int outIndex,
                                     const MoneyAmount cheapestItin)
{
  TSELatencyData metrics(_trx, "ESV SELECT BY NUM STOPS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::selectByNumStops");

  short noOfItemsFound = 0;

  // Get parameters from request
  const short nonStopPriority =
      _oneWay
          ? _trx.visOptions()->visOutboundSelectionOW().nonStopPriority()
          : (legIdx == outLegIdx) ? _trx.visOptions()->visOutboundSelectionRT().nonStopPriority()
                                  : _trx.visOptions()->visInboundSelectionRT().nonStopPriority();

  const short noOfNonStopOptions =
      _oneWay
          ? _trx.visOptions()->visOutboundSelectionOW().noOfNonStopOptions()
          : (legIdx == outLegIdx) ? _trx.visOptions()->visOutboundSelectionRT().noOfNonStopOptions()
                                  : _trx.visOptions()->visInboundSelectionRT().noOfNonStopOptions();

  const double nonStopFareMultiplier =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().nonStopFareMultiplier()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visOutboundSelectionRT().nonStopFareMultiplier()
                    : _trx.visOptions()->visInboundSelectionRT().nonStopFareMultiplier();

  if (0 == noOfNonStopOptions)
  {
    return noOfItemsFound;
  }

  std::sort(remainingFlights.begin(), remainingFlights.end(), NumstopsESVComparator(legIdx));

  const MoneyAmount maxNSPrice = cheapestItin * nonStopFareMultiplier;
  short noOfNonStopOptionsFound = 0;

  for (uint32_t n = 0; n < remainingFlights.size(); ++n)
  {
    if ((remainingFlights[n]->getNumStops(legIdx) != 0) ||
        (remainingFlights[n]->totalPrice() > maxNSPrice) ||
        (noOfNonStopOptionsFound >= noOfNonStopOptions))
    {
      break;
    }

    noOfItemsFound += markPQItem(remainingFlights[n], nonStopPriority, "B-NS");

    if (outIndex != -1)
    {
      remainingFlights[n]->screenID() = outIndex;
    }

    ++noOfNonStopOptionsFound;
  }

  return noOfItemsFound;
}

short
EstimatedSeatValue::selectByUtilityValue(std::vector<ESVPQItem*>& remainingFlights,
                                         const int legIdx,
                                         const int outIndex)
{
  TSELatencyData metrics(_trx, "ESV SELECT BY UTILITY VALUE");
  LOG4CXX_DEBUG(
      _logger,
      "EstimatedSeatValue::selectByUtilityValue(std::vector<ESVPQItem*>&, const int, const int)");

  short noOfItemsFound = 0;

  // Get parameters from request
  const short utilityValuePriority =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().utilityValuePriority()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visOutboundSelectionRT().utilityValuePriority()
                    : _trx.visOptions()->visInboundSelectionRT().utilityValuePriority();

  const short noOfUtilityValueOptions =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().noOfUtilityValueOptions()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visOutboundSelectionRT().noOfUtilityValueOptions()
                    : _trx.visOptions()->visInboundSelectionRT().noOfUtilityValueOptions();

  if (0 == noOfUtilityValueOptions)
  {
    return noOfItemsFound;
  }

  std::sort(remainingFlights.begin(), remainingFlights.end(), UtilityValueESVComparator(legIdx));

  short noOfUtilityValueOptionsFound = 0;

  for (uint32_t n = 0; n < remainingFlights.size(); ++n)
  {
    if (noOfUtilityValueOptionsFound >= noOfUtilityValueOptions)
    {
      break;
    }

    noOfItemsFound += markPQItem(remainingFlights[n], utilityValuePriority, "B-UV");

    if (outIndex != -1)
    {
      remainingFlights[n]->screenID() = outIndex;
    }

    ++noOfUtilityValueOptionsFound;
  }

  return noOfItemsFound;
}

void
EstimatedSeatValue::finalizeBucketsSelection(std::vector<ESVPQItem*>& remainingFlights,
                                             const int legIdx,
                                             std::vector<ESVPQItem*>& selectedFlights,
                                             const short noOfOptions)
{
  TSELatencyData metrics(_trx, "ESV FINALIZE BUCKETS SELECTION");
  LOG4CXX_DEBUG(_logger,
                "EstimatedSeatValue::finalizeBucketsSelection(std::vector<ESVPQItem*>&, "
                "const int, std::vector<ESVPQItem*>&)");

  std::sort(remainingFlights.begin(), remainingFlights.end(), PriorityESVComparator(legIdx));

  short noOfOptionsFound = 0;

  for (uint32_t n = 0; n < remainingFlights.size(); ++n)
  {
    if (noOfOptionsFound < noOfOptions)
    {
      if (remainingFlights[n]->isSelected())
      {
        remainingFlights[n]->selectionOrder() = selectedFlights.size();

        selectedFlights.push_back(remainingFlights[n]);

        ++noOfOptionsFound;
      }
    }
    else
    {
      remainingFlights[n]->isSelected() = false;
    }
  }

  if (noOfOptionsFound == noOfOptions)
  {
    // We had already found requested number of options
    remainingFlights.clear();
  }
  else
  {
    // We need to find additional options in next step
    remainingFlights.erase(
        std::remove_if(remainingFlights.begin(), remainingFlights.end(), isSelected),
        remainingFlights.end());
  }
}

short
EstimatedSeatValue::selectByIncrementalValue(std::vector<ESVPQItem*>& remainingFlights,
                                             const int legIdx,
                                             const int outIndex,
                                             std::vector<ESVPQItem*>& selectedFlights,
                                             const short noOfOptions)
{
  TSELatencyData metrics(_trx, "ESV SELECT BY INCREMENTAL VALUE");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::selectByIncrementalValue");

  short noOfItemsFound = 0;

  const std::vector<double>* paramBeta = (legIdx == outLegIdx) ? _paramBetaOut : _paramBetaIn;

  std::vector<double> DT;
  std::vector<bool> existDT;

  prepareDTVectors(selectedFlights, legIdx, DT, existDT);

  for (uint32_t i = 0; i < remainingFlights.size(); ++i)
  {
    calculateIV(remainingFlights[i], selectedFlights, paramBeta, legIdx, DT, existDT);
  }

  std::sort(
      remainingFlights.begin(), remainingFlights.end(), IncrementalValueESVComparator(legIdx));

  short noOfOptionsFound = 0;

  for (uint32_t n = 0; n < remainingFlights.size(); ++n)
  {
    if (noOfOptionsFound >= noOfOptions)
    {
      break;
    }

    noOfItemsFound += markPQItem(remainingFlights[n], 99, "O-IV");

    if (outIndex != -1)
    {
      remainingFlights[n]->screenID() = outIndex;
    }

    remainingFlights[n]->selectionOrder() = selectedFlights.size();

    selectedFlights.push_back(remainingFlights[n]);

    ++noOfOptionsFound;
  }

  return noOfItemsFound;
}

short
EstimatedSeatValue::markPQItem(ESVPQItem* pqItem,
                               const int priority,
                               const std::string& selectionSource) const
{
  TSELatencyData metrics(_trx, "ESV MARK PQITEM");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::markPQItem");

  // Check if this PQ item was already marked if not we need to count it as a
  // new item
  short newItemsCount = pqItem->isSelected() ? 0 : 1;

  pqItem->isSelected() = true;
  pqItem->selectionSource() += selectionSource + " ";

  if (pqItem->priority() > priority)
  {
    pqItem->priority() = priority;
  }

  return newItemsCount;
}

short
EstimatedSeatValue::selectByCarrier(std::vector<ESVPQItem*>& remainingFlights)
{
  TSELatencyData metrics(_trx, "ESV SELECT BY CARRIER");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::selectByCarrier");

  short noOfItemsFound = 0;

  const short carrierPriority = _oneWay
                                    ? _trx.visOptions()->visOutboundSelectionOW().carrierPriority()
                                    : _trx.visOptions()->visOutboundSelectionRT().carrierPriority();
  const short noOfCarriers = _oneWay ? _trx.visOptions()->visOutboundSelectionOW().noOfCarriers()
                                     : _trx.visOptions()->visOutboundSelectionRT().noOfCarriers();
  const short noOfOptionsPerCarrier =
      _oneWay ? _trx.visOptions()->visOutboundSelectionOW().noOfOptionsPerCarrier()
              : _trx.visOptions()->visOutboundSelectionRT().noOfOptionsPerCarrier();

  if (remainingFlights.empty())
  {
    return noOfItemsFound;
  }

  if ((0 == noOfCarriers) || (0 == noOfOptionsPerCarrier))
  {
    return noOfItemsFound;
  }

  // Sort flights by carrier and esv value
  std::sort(remainingFlights.begin(), remainingFlights.end(), FirstCarrierESVComparator(outLegIdx));

  // Get apropriate number of cheapest options for each carrier and add it to
  // temporary flights vector
  std::vector<ESVPQItem*> tempFlightsVec;
  CarrierCode currentCarrier = "";
  short noOfOptionsFound = 0;

  tempFlightsVec.push_back(remainingFlights[0]);
  currentCarrier = remainingFlights[0]->getFirstCarrier(outLegIdx);
  ++noOfOptionsFound;

  for (uint32_t n = 1; n < remainingFlights.size(); ++n)
  {
    if (remainingFlights[n]->getFirstCarrier(outLegIdx) == currentCarrier)
    {
      if (noOfOptionsFound < noOfOptionsPerCarrier)
      {
        tempFlightsVec.push_back(remainingFlights[n]);
        ++noOfOptionsFound;
      }
    }
    else
    {
      tempFlightsVec.push_back(remainingFlights[n]);
      currentCarrier = remainingFlights[n]->getFirstCarrier(outLegIdx);
      noOfOptionsFound = 1;
    }
  }

  // Sort temporary flights vector by esv value
  std::sort(tempFlightsVec.begin(), tempFlightsVec.end(), ESVComparator(outLegIdx));

  // Mark options for n cheapest carriers
  std::vector<CarrierCode> cheapestCarriersVec;
  short noOfCarriersFound = 0;

  cheapestCarriersVec.push_back(tempFlightsVec[0]->getFirstCarrier(outLegIdx));
  noOfItemsFound += markPQItem(tempFlightsVec[0], carrierPriority, "B-CR");
  ++noOfCarriersFound;

  for (uint32_t n = 1; n < tempFlightsVec.size(); ++n)
  {
    CarrierCode carrierCode = tempFlightsVec[n]->getFirstCarrier(outLegIdx);
    std::vector<CarrierCode>::iterator carierIter;

    carierIter = std::find(cheapestCarriersVec.begin(), cheapestCarriersVec.end(), carrierCode);

    if (carierIter != cheapestCarriersVec.end())
    {
      noOfItemsFound += markPQItem(tempFlightsVec[n], carrierPriority, "B-CR");
    }
    else
    {
      if (noOfCarriersFound < noOfCarriers)
      {
        cheapestCarriersVec.push_back(tempFlightsVec[n]->getFirstCarrier(outLegIdx));
        noOfItemsFound += markPQItem(tempFlightsVec[n], carrierPriority, "B-CR");
        ++noOfCarriersFound;
      }
    }
  }

  return noOfItemsFound;
}

// builds the cheapest option for each outbound
void
EstimatedSeatValue::findLFSforAllOut(const std::vector<ShoppingTrx::SchedulingOption*>& allOut,
                                     const std::vector<ESVSopWrapper*>& inWrapVec,
                                     std::vector<ESVPQItem*>& remainingFlights)
{
  TSELatencyData metrics(_trx, "ESV FIND LFS FOR ALL OUT");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::findLFSforAllOut");

  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator outIt = allOut.begin();
  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator outItEnd = allOut.end();
  std::vector<ShoppingTrx::SchedulingOption*> tmpOut;

  // Ensure that there is always 0-idx item.
  tmpOut.push_back(nullptr);

  ESVPQItem* pq = nullptr;

  while (outIt != outItEnd)
  {
    tmpOut[0] = *outIt;
    pq = getCheapestOption(tmpOut, inWrapVec);

    if (pq)
    {
      remainingFlights.push_back(pq);
    }

    ++outIt;
  }
}

ESVPQItem*
EstimatedSeatValue::getCheapestOption(const std::vector<ShoppingTrx::SchedulingOption*>& outVec,
                                      const std::vector<ESVSopWrapper*>& inWrapVec)
{
  TSELatencyData metrics(_trx, "getCheapestOption");
  LOG4CXX_DEBUG(_logger, "getCheapestOption");

  const CarrierCode cxr = "";
  const std::string title = "LFS new logic: getCheapestOption";

  VISPQ pq(&_trx, _paramBetaOut, _paramBetaIn, title);
  pq.init(outVec, inWrapVec, LFSRemaining, cxr, title);

  diagDisplay956(&pq);

  // the first item is the cheapest one
  ESVPQItem* pqItem = pq.getNextItem(_diag956Collector, true);
  if (pqItem)
    pq.computeUtilityValuesForPQItem(pqItem);
  return pqItem;
}

// LFS - just lfsNum the cheapest options
// all options are put into pickedItins
void
EstimatedSeatValue::selectLowFareAllOneWay(
    const std::vector<ShoppingTrx::SchedulingOption*>& allOut,
    std::vector<ESVPQItem*>& pickedItins,
    const int lfsNum,
    std::vector<ESVPQItem*>& remainingFlights)
{
  TSELatencyData metrics(_trx, "selectLowFareAllOneWay");
  LOG4CXX_DEBUG(_logger, "selectLowFareAllOneWay");
  const std::vector<ShoppingTrx::SchedulingOption*> allIn;
  const CarrierCode cxr = "";
  const std::string title = "LFS new logic: selectLowFareOneWay";
  VISPQ pq(&_trx, _paramBetaOut, _paramBetaIn, title);
  pq.init(allOut, allIn, LFSRemaining, cxr, title);
  diagDisplay956(&pq);

  // get requested number of LFS solutions if it is possible
  int solutionNum = 0;
  ESVPQItem* pqItem = pq.getNextItemVIS(_diag956Collector);
  while (pqItem != nullptr)
  {
    remainingFlights.push_back(pqItem);
    if (solutionNum < lfsNum)
      pickedItins.push_back(pqItem);
    ++solutionNum;
    pqItem = pq.getNextItemVIS(_diag956Collector);
  }
}

// LFS - just lfsNum the cheapest options
// all options are put into pickedItins
void
EstimatedSeatValue::selectLowFareAllRoundTrip(
    const std::vector<ShoppingTrx::SchedulingOption*>& allOut,
    const std::vector<ShoppingTrx::SchedulingOption*>& allIn,
    std::vector<ESVPQItem*>& pickedItins,
    const int lfsNum,
    std::vector<ESVSopWrapper*>& inWrapVec)
{
  TSELatencyData metrics(_trx, "selectLowFareAllRoundTrip");
  LOG4CXX_DEBUG(_logger, "selectLowFareAllRoundTrip");
  const CarrierCode cxr = "";
  const std::string title = "LFS new logic: selectLowFareAllRoundTrip";
  VISPQ pq(&_trx, _paramBetaOut, _paramBetaIn, title);
  pq.init(allOut, allIn, LFSRemaining, cxr, title);
  inWrapVec = *(pq.legVec()[inLegIdx]);
  diagDisplay956(&pq);

  // get requested number of LFS solutions if it is possible
  int solutionNum = 0;
  ESVPQItem* pqItem = pq.getNextItemVIS(_diag956Collector);
  bool done = ((pqItem == nullptr) || (solutionNum >= lfsNum));
  while (!done)
  {
    pickedItins.push_back(pqItem);
    ++solutionNum;
    if (solutionNum < lfsNum)
    {
      pqItem = pq.getNextItemVIS(_diag956Collector);
      done = (pqItem == nullptr);
    }
    else
      done = true;
  }
}

void
EstimatedSeatValue::copySOPptr(std::vector<ShoppingTrx::SchedulingOption>& vecSOP,
                               std::vector<ShoppingTrx::SchedulingOption*>& vecSOPptr)
{
  TSELatencyData metrics(_trx, "copySOPptr");
  LOG4CXX_DEBUG(_logger, "copySOPptr");
  std::vector<ShoppingTrx::SchedulingOption>::iterator sopIt = vecSOP.begin();
  std::vector<ShoppingTrx::SchedulingOption>::iterator sopItEnd = vecSOP.end();
  while (sopIt != sopItEnd)
  {
    if (!((*sopIt).getDummy()))
      vecSOPptr.push_back(&(*sopIt));
    ++sopIt;
  }
}

bool
ESVComparator::
operator()(ESVPQItem* lhs, ESVPQItem* rhs) const
{
  // Total price, Is selected, Utility value

  // Compare total price
  if (lhs->totalPrice() < rhs->totalPrice())
  {
    return true;
  }

  if (lhs->totalPrice() > rhs->totalPrice())
  {
    return false;
  }

  // Compare is selected flag
  if (lhs->isSelected() && (!rhs->isSelected()))
  {
    return true;
  }

  if ((!lhs->isSelected()) && rhs->isSelected())
  {
    return false;
  }

  // Compare utility value
  if (lhs->getUtility(_legIdx) > rhs->getUtility(_legIdx))
  {
    return true;
  }

  if (lhs->getUtility(_legIdx) < rhs->getUtility(_legIdx))
  {
    return false;
  }

  return false;
}

bool
FirstCarrierESVComparator::
operator()(ESVPQItem* lhs, ESVPQItem* rhs) const
{
  // Compare first carrier
  CarrierCode lhsFirstCarrier = lhs->getFirstCarrier(_legIdx);
  CarrierCode rhsFirstCarrier = rhs->getFirstCarrier(_legIdx);

  if (lhsFirstCarrier < rhsFirstCarrier)
  {
    return true;
  }

  if (lhsFirstCarrier > rhsFirstCarrier)
  {
    return false;
  }

  return ESVComparator::operator()(lhs, rhs);
}

bool
NumstopsESVComparator::
operator()(ESVPQItem* lhs, ESVPQItem* rhs) const
{
  // Compare number of stops
  int lhsNumStops = lhs->getNumStops(_legIdx);
  int rhsNumStops = rhs->getNumStops(_legIdx);

  if (lhsNumStops < rhsNumStops)
  {
    return true;
  }

  if (lhsNumStops > rhsNumStops)
  {
    return false;
  }

  return ESVComparator::operator()(lhs, rhs);
}

bool
ElapsedComparator::
operator()(ESVPQItem* lhs, ESVPQItem* rhs) const
{
  // Compare elapsed time
  int lhsElapsedTime = lhs->getFlightTimeMinutes(_legIdx);
  int rhsElapsedTime = rhs->getFlightTimeMinutes(_legIdx);

  if (lhsElapsedTime < rhsElapsedTime)
  {
    return true;
  }

  if (lhsElapsedTime > rhsElapsedTime)
  {
    return false;
  }

  return ESVComparator::operator()(lhs, rhs);
}

bool
UtilityValueESVComparator::
operator()(ESVPQItem* lhs, ESVPQItem* rhs) const
{
  // Compare utility value
  if (lhs->getUtility(_legIdx) > rhs->getUtility(_legIdx))
  {
    return true;
  }

  if (lhs->getUtility(_legIdx) < rhs->getUtility(_legIdx))
  {
    return false;
  }

  return ESVComparator::operator()(lhs, rhs);
}

bool
IncrementalValueESVComparator::
operator()(ESVPQItem* lhs, ESVPQItem* rhs) const
{
  // Compare utility value
  if (lhs->IV() > rhs->IV())
  {
    return true;
  }

  if (lhs->IV() < rhs->IV())
  {
    return false;
  }

  return ESVComparator::operator()(lhs, rhs);
}

bool
PriorityESVComparator::
operator()(ESVPQItem* lhs, ESVPQItem* rhs) const
{
  // Compare utility value
  if (lhs->priority() < rhs->priority())
  {
    return true;
  }

  if (lhs->priority() > rhs->priority())
  {
    return false;
  }

  return ESVComparator::operator()(lhs, rhs);
}

bool
EstimatedSeatValue::validBeta(const std::vector<double>* paramBeta, const bool enableMsg)
{
  if (!paramBeta)
  {
    if (enableMsg)
      LOG4CXX_ERROR(_logger, "Beta is null");
    return false;
  }

  if (paramBeta->size() != 13)
  {
    if (enableMsg)
      LOG4CXX_ERROR(_logger, "Beta size is not 13");
    return false;
  }

  return true;
}

const std::vector<double>*
EstimatedSeatValue::getParameterBeta(const int& timeDiff,
                                     const int& mileage,
                                     const char& direction,
                                     const char& APSatInd)
{
  const std::vector<double>* paramBeta =
      _trx.dataHandle().getParameterBeta(timeDiff, mileage, direction, APSatInd);

  if (!paramBeta)
  {
    // Try to use default values
    paramBeta = _trx.dataHandle().getParameterBeta(0, 0, direction, APSatInd);
  }

  if (!validBeta(paramBeta, true))
  {
    std::stringstream msg;
    msg << "for key values: "
        << "timeDiff: " << timeDiff << ", mileage: " << mileage << ", direction: " << direction
        << ", APSatInd: " << APSatInd << "\n";
    LOG4CXX_ERROR(_logger, msg.str());
  }

  if (_diag958Collector)
  {
    _diag958Collector->printParameterBeta(timeDiff, mileage, direction, APSatInd, paramBeta);
  }

  return paramBeta;
}

void
EstimatedSeatValue::calculateFlightTimeMinutes(const int origDestOffset)
{
  calculateFlightTimeMinutes(origDestOffset, outLegIdx);

  if (!_oneWay)
  {
    calculateFlightTimeMinutes(origDestOffset, inLegIdx);
  }
}

void
EstimatedSeatValue::calculateFlightTimeMinutes(const int origDestOffset, const int legIdx)
{
  std::vector<ShoppingTrx::SchedulingOption>::iterator it = _trx.legs()[legIdx].sop().begin();
  const std::vector<ShoppingTrx::SchedulingOption>::iterator itEnd =
      _trx.legs()[legIdx].sop().end();
  while (it != itEnd)
  {
    Itin* itin = (*it).itin();
    itin->calculateFlightTimeMinutes(origDestOffset, legIdx);
    ++it;
  }
}

short
EstimatedSeatValue::addLFSOptionsToFlightsSelection(const std::vector<ESVPQItem*>& lfsSolutions,
                                                    const int legIdx,
                                                    const ESVPQItem* obItin,
                                                    std::vector<ESVPQItem*>& selectedFlights)
{
  TSELatencyData metrics(_trx, "ESV ADD LFS OPTIONS TO FLIGHTS SELECTION");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::addLFSOptionsToFlightsSelection");

  short noOfItemsFound = 0;

  // Get parameters from request
  const short noOfAdditionalOptions =
      _oneWay ? _trx.visOptions()->visLowFareSearch().noOfAdditionalOutboundsOW()
              : (legIdx == outLegIdx)
                    ? _trx.visOptions()->visLowFareSearch().noOfAdditionalOutboundsRT()
                    : _trx.visOptions()->visLowFareSearch().noOfAdditionalInboundsRT();

  short noOfAdditionalOptionsFound = 0;

  for (uint32_t n = 0; n < lfsSolutions.size(); ++n)
  {
    if (noOfAdditionalOptionsFound >= noOfAdditionalOptions)
    {
      break;
    }

    // Check if outbound flight is the same for inbound part
    if (obItin != nullptr)
    {
      if (obItin->outSopWrapper()->sop()->originalSopId() !=
          lfsSolutions[n]->outSopWrapper()->sop()->originalSopId())
      {
        continue;
      }
    }

    if (!findFlightInPQItems(lfsSolutions[n], legIdx, selectedFlights))
    {
      noOfItemsFound += markPQItem(lfsSolutions[n], 99, "O-LFS");
      selectedFlights.push_back(lfsSolutions[n]);
      ++noOfAdditionalOptionsFound;
    }
  }

  return noOfItemsFound;
}

bool
EstimatedSeatValue::findFlightInPQItems(const ESVPQItem* pqItem,
                                        const int legIdx,
                                        std::vector<ESVPQItem*>& selectedFlights)
{
  TSELatencyData metrics(_trx, "ESV FIND FLIGHT IN PQITEMS");
  LOG4CXX_DEBUG(_logger, "EstimatedSeatValue::findFlightInPQItems");

  const ShoppingTrx::SchedulingOption* sop =
      (0 == legIdx) ? pqItem->outSopWrapper()->sop() : pqItem->inSopWrapper()->sop();

  for (uint32_t n = 0; n < selectedFlights.size(); ++n)
  {
    uint32_t originalSopId = (0 == legIdx)
                                 ? selectedFlights[n]->outSopWrapper()->sop()->originalSopId()
                                 : selectedFlights[n]->inSopWrapper()->sop()->originalSopId();

    if (originalSopId == sop->originalSopId())
    {
      return true;
    }
  }

  return false;
}

inline bool
PQItemReverseCompare::
operator()(ESVPQItem* lhs, ESVPQItem* rhs)
{
  return lhs->totalPrice() < rhs->totalPrice();
}

void
ESVPQItemWrapper::init(const int expandColumn, const std::vector<int>& indices)
{
  _expandColumn = expandColumn;
  _indices = indices;
  ++_indices[expandColumn];
}
}
