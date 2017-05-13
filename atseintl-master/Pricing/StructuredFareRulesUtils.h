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
#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/StructuredRuleData.h"
#include <cstdint>

namespace tse
{
class FarePath;
class FareUsage;
class PricingUnit;

namespace structuredFareRulesUtils
{
void
finalizeDataCollection(FarePath& farePath);

void
updateMostRestrictivePricingUnitData(MostRestrictivePricingUnitSFRData& mostRestrictiveRuleData,
                                     const StructuredRuleData& data);
void
updateMostRestrictiveJourneyData(MostRestrictiveJourneySFRData& mostRestrictiveRuleData,
                                 const StructuredRuleData& data);

void
updateFUFromPaxTypeFare(FareUsage& fu);

void
updateMaxStayTrvCommenceData(MaxStayMap& maxStayMap,
                             uint16_t segmentOrder,
                             DateTime mustCommence,
                             LocCode firstTrvSegLoc);
void
updateMaxStayTrvCompleteData(MaxStayMap& maxStayMap,
                             uint16_t segmentOrder,
                             DateTime mustComplete,
                             LocCode lastTrvSegLoc);

void
updateMaxStayTrvData(MaxStayMap& maxStayMap, const MaxStayMapElem& mapElem);

void
copyAdvanceResAndTktData(const FareUsage& fromFu, FareUsage& toFu);

DateTime
getDateWithEarliestTime(DateTime dateTime, const int minutes = -1);
}
}
