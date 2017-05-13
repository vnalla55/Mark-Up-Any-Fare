// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         23-11-2011
//! \file         PaxFPFBaseData.h
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include "Pricing/FactoriesConfig.h"

#include <time.h>

namespace tse
{
class Combinations;
class PricingTrx;
class PaxType;

class PaxFPFBaseData
{
public:
  PaxFPFBaseData(const FactoriesConfig& factoriesConfig) : _factoriesConfig(factoriesConfig) {}

  ~PaxFPFBaseData() {}

  const FactoriesConfig& getFactoriesConfig() const { return _factoriesConfig; }

  void incrementFPCombTried() { ++_fpCombTried; }
  uint32_t fpCombTried() { return _fpCombTried; }

  void incermentGlobalPushBackCount() { ++_globalPushBackCount; }
  bool reachedGlobaPushBackLimit()
  {
    return _globalPushBackCount >= _factoriesConfig.globalPushBackMax() &&
           _factoriesConfig.globalPushBackMax() > 0;
  }

  const PaxType* paxType() const { return _paxType; }
  PaxType*& paxType() { return _paxType; }

  const PricingTrx* trx() const { return _trx; }
  PricingTrx*& trx() { return _trx; }

  const time_t& shortCktKeepValidFPsTime() const { return _shortCktOnPushBackTimeout; }
  time_t& shortCktKeepValidFPsTime() { return _shortCktOnPushBackTimeout; }

  const bool pricingShortCktHappened() const { return _pricingShortCktHappened; }
  bool& pricingShortCktHappened() { return _pricingShortCktHappened; }

  const bool validFPPushedBack() const { return _validFPPushedBack; }
  bool& validFPPushedBack() { return _validFPPushedBack; }

  Combinations*& combinations() { return _combinations; }

protected:
  const FactoriesConfig& _factoriesConfig;
  Combinations* _combinations = nullptr;
  PricingTrx* _trx = nullptr;
  PaxType* _paxType = nullptr;
  time_t _shortCktOnPushBackTimeout = 0;
  uint32_t _fpCombTried = 0;
  int32_t _globalPushBackCount = 0; // For All FarePathFactory combined Push Back count
  bool _pricingShortCktHappened = false;
  bool _validFPPushedBack = false;
};

} /* namespace tse */
