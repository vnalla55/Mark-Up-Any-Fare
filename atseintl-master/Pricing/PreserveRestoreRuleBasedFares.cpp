//----------------------------------------------------------------------------
//  Copyright Sabre 2014
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

#include "Pricing/PreserveRestoreRuleBasedFares.h"

#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "Diagnostic/Diag625Collector.h"
#include "Diagnostic/DiagCollectorGuard.h"
#include "Pricing/PricingUtil.h"

namespace tse
{

void
PreserveRestorePURuleBasedFares::Preserve(PricingUnit* pricingUnit, DiagCollector* diag)
{
  _diag = diag;
  _pricingUnits.push_back(pricingUnit);
  Preserve();
}

void
PreserveRestorePURuleBasedFares::Preserve(const std::vector<PricingUnit*>& pricingUnits,
                                          DiagCollector* diag)
{
  _diag = diag;
  _pricingUnits = pricingUnits;
  Preserve();
}

void
PreserveRestorePURuleBasedFares::Preserve()
{
  for (PricingUnit* pu : _pricingUnits)
  {
    for (FareUsage* fareUsage : pu->fareUsage())
    {
      PaxTypeFare* oldPaxTypeFare = fareUsage->paxTypeFare();
      PaxTypeFare* newPaxTypeFare = determinePaxTypeFare(oldPaxTypeFare);

      if (oldPaxTypeFare != newPaxTypeFare)
      {
        for (PricingUnit* pricingUnit : _pricingUnits)
          pricingUnit->addChangedFareCat10overrideCat25(newPaxTypeFare, oldPaxTypeFare);
        fareUsage->paxTypeFare() = newPaxTypeFare;
        fareUsage->rec2Cat10() = newPaxTypeFare->rec2Cat10();
      }
    }
  }
}

void
PreserveRestorePURuleBasedFares::Restore()
{
  for (PricingUnit* pricingUnit : _pricingUnits)
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      PaxTypeFare* oldfare = pricingUnit->getOldFareCat10overrideCat25(fareUsage->paxTypeFare());
      fareUsage->paxTypeFare() = oldfare;
      fareUsage->rec2Cat10() = oldfare->rec2Cat10();
    }

    pricingUnit->clearFareMapCat10overrideCat25();
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PaxTypeFare* Combinations::determinePaxTypeFare
//
// Description: This method returns the proper PaxTypeFare pointer based upon
//              Category 25 Category Override Tag rules.  If the PaxTypeFare
//              in question is not a FareByRule, the regular
//              PaxTypeFare object will be returned.  If the PaxTypeFare is
//              a Calculated FareByRule and the Category 10 override
//              tag is set to 'B' the base Fare of the FBR PaxTypeFare will
//              be returned.
//
// </PRE>
// ----------------------------------------------------------------------------
PaxTypeFare*
PreserveRestorePURuleBasedFares::determinePaxTypeFare(PaxTypeFare* ptFare) const
{
  PaxTypeFare* ptf = const_cast<PaxTypeFare*>(PricingUtil::determinePaxTypeFare(ptFare));

  if (UNLIKELY((ptf != ptFare) && (_diag->diagnosticType() == Diagnostic625)))
  {
    _diag->enable(Diagnostic625);
    *_diag << *ptFare;
  }
  return ptf;
}

void
PreserveRestoreFPRuleBasedFares::Preserve(FarePath& farePath, DiagCollector* diag)
{
  _oldPUFares.resize(farePath.pricingUnit().size());

  for (PreserveRestorePURuleBasedFares& pu : _oldPUFares)
  {
    pu.Preserve(farePath.pricingUnit(), diag);
  }
}

} // namespace tse
