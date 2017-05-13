//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------
#pragma once

#include "Common/TsePrimitiveTypes.h"

#include <map>
#include <set>
#include <vector>

namespace tse
{
class FPPQItem;
class FarePathFactoryFailedPricingUnits;
class PaxTypeFare;
class PricingUnitFactory;
class YQYRCalculator;

struct FarePathSettings
{
  FarePathSettings(
      std::vector<PricingUnitFactory*>* allPUF = nullptr,
      FarePathFactoryFailedPricingUnits* failedPricingUnits = nullptr,
      std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>>* eoeFailedFare = nullptr,
      YQYRCalculator* yqyrCalc = nullptr,
      MoneyAmount externalLowerBoundAmount = 0)
    : _allPUF(allPUF),
      _failedPricingUnits(failedPricingUnits),
      _eoeFailedFare(eoeFailedFare),
      _yqyrCalc(yqyrCalc),
      _externalLowerBoundAmount(externalLowerBoundAmount)
  {
  }
  std::vector<PricingUnitFactory*>* _allPUF = nullptr;
  FarePathFactoryFailedPricingUnits* _failedPricingUnits = nullptr;
  std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>>* _eoeFailedFare = nullptr;
  YQYRCalculator* _yqyrCalc = nullptr;
  MoneyAmount _externalLowerBoundAmount = 0;
};

struct SavedFPPQItem
{
  enum class Stage : uint8_t { FP_LEVEL, MOTHER_SOLUTION };
  SavedFPPQItem(uint16_t id,
                uint16_t failedCategory,
                FPPQItem* fppqItem,
                FarePathSettings settings,
                Stage stage)
    : _fppqItem(fppqItem),
      _settings(std::move(settings)),
      _failedCategory(failedCategory),
      _id(id),
      _stage(stage)
  {
  }

  FPPQItem* _fppqItem;
  FarePathSettings _settings;
  uint16_t _failedCategory;
  uint16_t _id;
  Stage _stage;
  bool _processed = false;
};

}
