//-------------------------------------------------------------------
//
//  File:        FarePUResultContainer.cpp
//  Created:     Feb 24, 2013
//  Authors:	 Simon Li
//
//  Description: PaxTypeFare PricingUnit scope Rule result reuse
//
//  Copyright Sabre 2013
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

#include "Rules/FarePUResultContainer.h"

#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "Rules/PricingUnitRuleController.h"
#include "Rules/RuleConst.h"


using namespace std;

namespace tse
{

FarePUResultContainer*
FarePUResultContainer::createFarePUResultContainer(PricingTrx& trx /*, const PricingUnit& pu*/)
{
  if (TrxUtil::reuseFarePUResult())
  {
    //      FarePUResultContainer* container = trx.findSamePUContainer(pu);
    //      if (container)
    //        return container;

    FarePUResultContainer* container = trx.dataHandle().create<FarePUResultContainer>();
    container->_isWPNC =
        trx.getRequest()->isLowFareRequested() || trx.getRequest()->isLowFareNoAvailability();
    // trx.registerContainer(pu, container);
    return container;
  }

  return nullptr;
}

uint16_t
FarePUResultContainer::buildRebookStatBitMap(const PricingUnit& pu)
{
  uint16_t rebookSegInPU = 0;
  uint16_t bitV = 1;
  for (FareUsage* fu : pu.fareUsage())
  {
    for (PaxTypeFare::SegmentStatus& ss : fu->segmentStatus())
    {
      if (ss._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
        rebookSegInPU |= bitV;
      bitV = bitV << 1;
    }
  }
  return rebookSegInPU; // TBI
}

bool
FarePUResultContainer::getPreviousResult(FareUsage& fu, const uint16_t key, bool& isValid) const
{
  if (_isWPNC)
  {
    FareResultWPNCCI ptfResultCI = _resultByPtfWPNC.find(fu.paxTypeFare());
    if (ptfResultCI == _resultByPtfWPNC.end())
      return false;

    ResultByKeyCI iResult = ptfResultCI->second.find(key);
    if (iResult == ptfResultCI->second.end())
      return false;

    isValid = iResult->second;

    return true;
  }
  else
  {
    FareResultCI ptfResultCI = _resultByPtf.find(fu.paxTypeFare());
    if (ptfResultCI == _resultByPtf.end())
      return false;

    isValid = ptfResultCI->second;

    return true;
  }
}

void
FarePUResultContainer::saveResultForReuse(const FareUsage& fu, const uint16_t key, bool isValid)
{
  if (_isWPNC)
    _resultByPtfWPNC[fu.paxTypeFare()].insert(std::make_pair(key, isValid));
  else
    _resultByPtf.insert(std::make_pair(fu.paxTypeFare(), isValid));
}

void
FarePUResultContainer::clear()
{
  _resultByPtfWPNC.clear();
  _resultByPtf.clear();
}
}
