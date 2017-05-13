//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Rules/PricingUnitDataAccess.h"

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"

#include <numeric>

namespace tse
{

Itin*
PricingUnitDataAccess::itin()
{
  return (_farePath ? _farePath->itin() : _itin);
}

PaxTypeFare&
PricingUnitDataAccess::paxTypeFare() const
{
  return getBaseOrPaxTypeFare(*(_fareUsage.paxTypeFare()));
}

void
PricingUnitDataAccess::cloneFU(unsigned int ruleCount)
{
  _clonedFU = _fareUsage.clone(_trx.dataHandle());

  // we are only interested in the added surcharges
  _clonedFU->surchargeData().clear();
  _ruleCount = ruleCount;
}

// returns true if no more rule processing is needed, false otherwise
bool
PricingUnitDataAccess::processRuleResult(Record3ReturnTypes& ruleRet,
                                         unsigned int& ruleIndex)
{
  if (ruleRet != FAIL)
  {
    MoneyAmount surcharge = std::accumulate(_clonedFU->surchargeData().begin(),
                                            _clonedFU->surchargeData().end(),
                                            0.0,
                                            [](MoneyAmount init, const auto* sd)
                                            {
                                              return init+sd->amountNuc();
                                            });

    if (surcharge == 0.0)
    {
      // found a rule with no surcharge, no need to continue
      _clonedFU = nullptr;
      _nonFailedRuleExists = false;
      return true;
    }

    if ((!_nonFailedRuleExists) || (surcharge < _ruleSurcharge.surcharge))
    {
      _ruleSurcharge.surchargeData = _clonedFU->surchargeData();
      _ruleSurcharge.ruleRet = ruleRet;
      _ruleSurcharge.ruleIndex = ruleIndex;
      _ruleSurcharge.surcharge = surcharge;
      _nonFailedRuleExists = true;
    }

    _clonedFU->surchargeData().clear();
  }

  if (ruleIndex == _ruleCount - 1)
  {
    // Last rule, select lowest surcharge and apply it to the actual fare usage
    if (!_nonFailedRuleExists)
      ruleIndex = 0;
    else
    {
      ruleIndex = _ruleSurcharge.ruleIndex;
      ruleRet = _ruleSurcharge.ruleRet;
      _fareUsage.surchargeData().insert(_fareUsage.surchargeData().end(),
                                        _ruleSurcharge.surchargeData.begin(),
                                        _ruleSurcharge.surchargeData.end());
      _nonFailedRuleExists = false;
    }
    _clonedFU = nullptr;
    return true;
  }

  return false;
}

}
