//-------------------------------------------------------------------
// File:    PricingUnitFactoryBucket.h
// Created: July 2004
// Authors: Mohammad Hossan
//
//  Copyright Sabre 2004
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

#pragma once

#include "Pricing/PricingUnitFactory.h"

#include <map>
#include <queue>

namespace tse
{

class Combinations;
class PricingUnitFactory;

class PricingUnitFactoryBucket final
{

public:
  PricingUnitFactoryBucket() = default;
  PricingUnitFactoryBucket(const PricingUnitFactoryBucket&) = delete;
  PricingUnitFactoryBucket& operator=(const PricingUnitFactoryBucket&) = delete;

  const PaxType* paxType() const { return _paxType; }
  PaxType*& paxType() { return _paxType; }

  const std::map<PU*, PricingUnitFactory*>& puFactoryBucket() const { return _puFactoryBucket; }
  std::map<PU*, PricingUnitFactory*>& puFactoryBucket() { return _puFactoryBucket; }

  const Combinations* combinations() const { return _combinations; }
  Combinations*& combinations() { return _combinations; }

  const PricingUnitFactory::JourneyPULowerBound* journeyPULowerBound() const
  {
    return _journeyPULowerBound;
  }
  PricingUnitFactory::JourneyPULowerBound*& journeyPULowerBound() { return _journeyPULowerBound; }

  void cloneExceptBucket(PricingUnitFactoryBucket* cloneObj)
  {
    cloneObj->_paxType = _paxType;
    cloneObj->_combinations = _combinations;
    cloneObj->_journeyPULowerBound = _journeyPULowerBound;
  }

private:
  PaxType* _paxType = nullptr;
  Combinations* _combinations = nullptr;
  std::map<PU*, PricingUnitFactory*> _puFactoryBucket;

  PricingUnitFactory::JourneyPULowerBound* _journeyPULowerBound = nullptr;
};
} // tse namespace
