//-------------------------------------------------------------------
//
//  File:        DepartureValidator.h
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

#pragma once

#include "Common/TsePrimitiveTypes.h"

#include <vector>

namespace tse
{

class DiagCollector;
class PricingUnit;
class TravelSeg;
class Itin;
class FareMarket;
class PricingUnit;

class DepartureValidator
{
public:
  DepartureValidator(const Itin* itin,
                     const FareMarket* fc,
                     const PricingUnit* pu,
                     DiagCollector* dc)
    : _itin(itin), _fc(fc), _pu(pu), _dc(dc)
  {
  }

  virtual ~DepartureValidator() {}

  bool validate(uint32_t itemNoR3,
                Indicator puDepartInd,
                Indicator jrDepartInd,
                Indicator fcDepartInd,
                bool& isSoftPass);

  static const Indicator NOT_APPLY, BEFORE, AFTER;

protected:
  const Itin* _itin;
  const FareMarket* _fc;
  const PricingUnit* _pu;
  DiagCollector* _dc;

  bool matchBeforeAfterDeparture(const TravelSeg& travelSeg, Indicator departInd);
  bool match(const std::vector<TravelSeg*>& travelSeg, Indicator departInd);

  bool validateJR(uint32_t itemNoR3, Indicator departInd);
  bool validateFC(uint32_t itemNoR3, Indicator departInd);
  bool validatePU(uint32_t itemNoR3, Indicator departInd, bool& isSoftPass);

private:
  DepartureValidator(const DepartureValidator&);
  DepartureValidator& operator=(const DepartureValidator&);

  friend class DepartureValidatorTest;
};
}
