/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/StructuredFareRulesUtils.h"

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"

#include <utility>

namespace tse
{
namespace structuredFareRulesUtils
{
namespace
{
void
updateMinStayMostRestrictivePUData(MostRestrictivePricingUnitSFRData& mostRestrictiveRuleData,
                                   const MinStayMapElem& fCMapElem)
{
  auto& minStayMap = mostRestrictiveRuleData._minStayMap;
  auto retVal = minStayMap.insert(fCMapElem);
  const bool insertStatus = retVal.second;

  if (!insertStatus)
  {
    auto it = retVal.first;
    DateTime& earliestTime = it->second.first;
    const DateTime& newMinStayData = fCMapElem.second.first;
    earliestTime.setWithLater(newMinStayData);
  }
}

void
updateMaxStayInFUFromPaxTypeFare(FareUsage& fareUsage, const PaxTypeFare& paxTypeFare)
{
  const MaxStayMap& maxStayMap = paxTypeFare.getStructuredRuleData()._maxStayMap;
  if (maxStayMap.empty())
    return;

  fareUsage.createStructuredRuleDataIfNonexistent();
  MaxStayMap& maxStayMapFromFU = fareUsage.getStructuredRuleData()._maxStayMostRestrictiveFCData;
  for (const MaxStayMapElem& mapElem : maxStayMap)
    updateMaxStayTrvData(maxStayMapFromFU, mapElem);
}

void
updateAdvanceResAndTktInFUFromPaxTypeFare(FareUsage& fareUsage, const PaxTypeFare& paxTypeFare)
{
  const DateTime& lastDateToBook = paxTypeFare.getStructuredRuleData()._advanceReservation;
  const DateTime& latestTktDate = paxTypeFare.getStructuredRuleData()._advanceTicketing;

  if (lastDateToBook.isValid())
  {
    fareUsage.createStructuredRuleDataIfNonexistent();
    fareUsage.getStructuredRuleData().setAdvanceReservationWithEarlier(lastDateToBook);
  }
  if (latestTktDate.isValid())
  {
    fareUsage.createStructuredRuleDataIfNonexistent();
    fareUsage.getStructuredRuleData().setAdvanceTicketingWithEarlier(latestTktDate);
  }
}

constexpr int lastMinuteOfDay = 23 * 60 + 59;
}

DateTime
getDateWithEarliestTime(DateTime dateTime, const int minutes)
{
  if (dateTime.totalMinutes() == 0)
  {
    if (minutes < 0 || minutes == lastMinuteOfDay + 1)
      dateTime.addMinutes(lastMinuteOfDay);
    else
      dateTime.addMinutes(minutes);
  }
  return dateTime;
}

void
updateFUFromPaxTypeFare(FareUsage& fu)
{
  PaxTypeFare* paxTypeFare = fu.paxTypeFare();
  TSE_ASSERT(paxTypeFare);

  auto baseFare = paxTypeFare->getBaseFare();
  if (baseFare && baseFare->hasStructuredRuleData())
    paxTypeFare->setStructuredRuleData(baseFare->getStructuredRuleData());

  if (paxTypeFare->hasStructuredRuleData())
  {
    updateMaxStayInFUFromPaxTypeFare(fu, *paxTypeFare);
    updateAdvanceResAndTktInFUFromPaxTypeFare(fu, *paxTypeFare);
  }
}

void
updateMostRestrictivePricingUnitData(MostRestrictivePricingUnitSFRData& mostRestrictiveRuleData,
                                     const StructuredRuleData& data)
{
  if (data._minStayDate.isValid())
  {
    MinStayMapElem elem = std::make_pair(data._minStaySegmentOrder,
                                         std::make_pair(data._minStayDate, data._minStayLocation));
    updateMinStayMostRestrictivePUData(mostRestrictiveRuleData, elem);
  }

  if (!data._maxStayMostRestrictiveFCData.empty())
  {
    for (const auto& mapElem : data._maxStayMostRestrictiveFCData)
      updateMaxStayTrvData(mostRestrictiveRuleData._maxStayMap, mapElem);
  }
}

void
updateMostRestrictiveJourneyData(MostRestrictiveJourneySFRData& mostRestrictiveRuleData,
                                 const StructuredRuleData& data)
{
  mostRestrictiveRuleData.setAdvanceReservationWithEarlier(data._advanceReservation);
  mostRestrictiveRuleData.setAdvanceTicketingWithEarlier(data._advanceTicketing);
}

void
finalizeDataCollection(FarePath& farePath)
{
  farePath.createMostRestrictiveJourneySFRData();
  auto& mostRestrictiveJourneyData = farePath.getMostRestrictiveJourneySFRData();

  for (PricingUnit* pu : farePath.pricingUnit())
  {
    pu->createMostRestrictivePricingUnitSFRData();
    auto& mostRestrictivePUData = pu->getMostRestrictivePricingUnitSFRData();

    for (FareUsage* fu : pu->fareUsage())
    {
      updateFUFromPaxTypeFare(*fu);
      if (!fu->hasStructuredRuleData())
        continue;

      StructuredRuleData& data = fu->getStructuredRuleData();
      updateMostRestrictivePricingUnitData(mostRestrictivePUData, data);
      updateMostRestrictiveJourneyData(mostRestrictiveJourneyData, data);
    }
  }
}

void
updateMaxStayTrvCommenceData(MaxStayMap& maxStayMap,
                             uint16_t segmentOrder,
                             DateTime mustCommence,
                             LocCode firstTrvSegLoc)
{
  updateMaxStayTrvData(
      maxStayMap,
      std::make_pair(segmentOrder,
                     MaxStayData(firstTrvSegLoc, mustCommence, DateTime::openDate())));
}
void
updateMaxStayTrvCompleteData(MaxStayMap& maxStayMap,
                             uint16_t segmentOrder,
                             DateTime mustComplete,
                             LocCode lastTrvSegLoc)
{
  updateMaxStayTrvData(
      maxStayMap,
      std::make_pair(segmentOrder, MaxStayData(lastTrvSegLoc, DateTime::openDate(), mustComplete)));
}

void
updateMaxStayTrvData(MaxStayMap& maxStayMap, const MaxStayMapElem& mapElem)
{
  auto retVal = maxStayMap.insert(mapElem);

  const bool insertStatus = retVal.second;
  if (!insertStatus)
  {
    auto alreadyExistingElemIt = retVal.first;
    MaxStayData& maxStayData = alreadyExistingElemIt->second;
    const MaxStayData& newMaxStayData = mapElem.second;
    maxStayData.setCommenceWithEarlier(newMaxStayData._mustCommence);
    maxStayData.setCompleteWithEarlier(newMaxStayData._mustComplete);
  }
}

void
copyAdvanceResAndTktData(const FareUsage& fromFu, FareUsage& toFu)
{
  if (!fromFu.hasStructuredRuleData())
    return;

  const StructuredRuleData& fromData = fromFu.getStructuredRuleData();
  if (fromData._advanceReservation.isValid())
  {
    toFu.createStructuredRuleDataIfNonexistent();
    toFu.getStructuredRuleData().setAdvanceReservationWithEarlier(fromData._advanceReservation);
  }
  if (fromData._advanceTicketing.isValid())
  {
    toFu.createStructuredRuleDataIfNonexistent();
    toFu.getStructuredRuleData().setAdvanceTicketingWithEarlier(fromData._advanceTicketing);
  }
}
}
}
