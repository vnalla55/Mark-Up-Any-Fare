#include "test/testdata/TestRuleCPPUnitHelper.h"

#include "DBAccess/CategoryRuleItemInfo.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleConst.h"

using namespace tse;

bool
TestRuleCPPUnitHelper::buildTestItinAndRuleForIsDirectionPass(
    CategoryRuleItemInfo& catRuleItemInfo,
    const Record3ReturnTypes desiredResult,
    const char dir,
    const PaxTypeFare& ptFare,
    Itin& itin,
    bool& isLocationSwapped)
{
  catRuleItemInfo.setRelationalInd(CategoryRuleItemInfo::THEN);
  catRuleItemInfo.setDirectionality(dir);

  if (dir == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2)
  {
    itin.travelSeg().clear();

    if (desiredResult == SOFTPASS)
    {
      TravelSeg* tvlSeg1 = new AirSeg();

      if ("LOA" != ptFare.fareMarket()->travelSeg().front()->origAirport() &&
          "LOA" != ptFare.fareMarket()->travelSeg().back()->destAirport())
        tvlSeg1->origAirport() = "LOA";
      else if ("LOB" != ptFare.fareMarket()->travelSeg().front()->origAirport() &&
               "LOB" != ptFare.fareMarket()->travelSeg().back()->destAirport())
        tvlSeg1->origAirport() = "LOB";
      else
        tvlSeg1->origAirport() = "LOC";

      tvlSeg1->destAirport() = ptFare.fareMarket()->travelSeg().front()->origAirport();
      tvlSeg1->boardMultiCity() = tvlSeg1->origAirport();
      tvlSeg1->offMultiCity() = tvlSeg1->destAirport();

      TravelSeg* tvlSeg2 = new AirSeg();
      tvlSeg2->origAirport() = ptFare.fareMarket()->travelSeg().back()->destAirport();
      if ("LDA" != ptFare.fareMarket()->travelSeg().front()->origAirport() &&
          "LDA" != ptFare.fareMarket()->travelSeg().back()->destAirport())
        tvlSeg2->destAirport() = "LDA";
      else if ("LDB" != ptFare.fareMarket()->travelSeg().front()->origAirport() &&
               "LDB" != ptFare.fareMarket()->travelSeg().back()->destAirport())
        tvlSeg2->destAirport() = "LDB";
      else
        tvlSeg2->destAirport() = "LDC";

      tvlSeg2->boardMultiCity() = tvlSeg2->origAirport();
      tvlSeg2->offMultiCity() = tvlSeg2->destAirport();

      itin.travelSeg().push_back(tvlSeg1);
      itin.travelSeg().insert(itin.travelSeg().end(),
                              ptFare.fareMarket()->travelSeg().begin(),
                              ptFare.fareMarket()->travelSeg().end());
      itin.travelSeg().push_back(tvlSeg2);
    }
    else
    {
      itin.travelSeg().insert(itin.travelSeg().end(),
                              ptFare.fareMarket()->travelSeg().begin(),
                              ptFare.fareMarket()->travelSeg().end());

      isLocationSwapped = (desiredResult == FAIL);
    }
    return true;
  }

  return false;
}
