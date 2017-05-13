//-------------------------------------------------------------------
//
//  File:        DepartureValidator.cpp
//  Created:     July 1, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Rules/DepartureValidator.h"

#include "Common/TravelSegUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/DiagCollector.h"
#include "Util/BranchPrediction.h"

namespace tse
{

const Indicator DepartureValidator::NOT_APPLY = ' ';
const Indicator DepartureValidator::BEFORE = 'B';
const Indicator DepartureValidator::AFTER = 'A';

bool
DepartureValidator::validate(uint32_t itemNoR3,
                             Indicator puDepartInd,
                             Indicator joDepartInd,
                             Indicator fcDepartInd,
                             bool& isSoftPass)
{
  return (validatePU(itemNoR3, puDepartInd, isSoftPass) && validateJR(itemNoR3, joDepartInd) &&
          validateFC(itemNoR3, fcDepartInd));
}

bool
DepartureValidator::validatePU(uint32_t itemNoR3, Indicator departInd, bool& isSoftPass)
{
  const bool result = _pu ? match(_pu->travelSeg(), departInd) : isSoftPass = true;

  if (LIKELY(!_dc))
    return result;

  *_dc << "DEPARTURE OF PU: " << departInd << (!_pu ? "\n  NO PU INFO: SOFTPASS\n" : "\n");

  if (!result)
    *_dc << "  FAILED ITEM " << itemNoR3 << " - DEPARTURE OF PU\n";

  return result;
}

bool
DepartureValidator::validateJR(uint32_t itemNoR3, Indicator departInd)
{
  const bool result = match(_itin->travelSeg(), departInd);

  if (LIKELY(!_dc))
    return result;

  *_dc << "DEPARTURE OF JR: " << departInd << "\n";

  if (!result)
    *_dc << "  FAILED ITEM " << itemNoR3 << " - DEPARTURE OF JR\n";

  return result;
}

bool
DepartureValidator::validateFC(uint32_t itemNoR3, Indicator departInd)
{
  const bool result = match(_fc->travelSeg(), departInd);

  if (LIKELY(!_dc))
    return result;

  *_dc << "DEPARTURE OF FC: " << departInd << "\n";

  if (!result)
    *_dc << "  FAILED ITEM " << itemNoR3 << " - DEPARTURE OF FC\n";

  return result;
}

bool
DepartureValidator::match(const std::vector<TravelSeg*>& travelSeg, Indicator departInd)

{
  if (departInd == DepartureValidator::NOT_APPLY)
    return true;

  const TravelSeg* firstSeg = TravelSegUtil::firstNoArunkSeg(travelSeg);

  return firstSeg ? matchBeforeAfterDeparture(*firstSeg, departInd) : false;
}

bool
DepartureValidator::matchBeforeAfterDeparture(const TravelSeg& travelSeg, Indicator departInd)
{
  switch (departInd)
  {
  case DepartureValidator::BEFORE:
    return travelSeg.unflown();
  case DepartureValidator::AFTER:
    return !travelSeg.unflown();
  }
  return true;
}
}
