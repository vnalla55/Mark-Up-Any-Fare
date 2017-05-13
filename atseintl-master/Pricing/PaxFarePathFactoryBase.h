//-------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/TseDateTimeTypes.h"
#include "Pricing/PaxFPFBaseData.h"
#include "Pricing/SavedFPPQItem.h"

#include <vector>

#include <cstdlib>
#include <cstdint>

namespace tse
{
class DiagCollector;
class FactoriesConfig;
class FareMarketPath;
class FarePathFactory;
class FPPQItem;
class PUPathMatrix;
class PricingUnitFactoryBucket;
class PaxType;
class PricingUnitFactory;

class PaxFarePathFactoryBase : public PaxFPFBaseData
{
public:
  PaxFarePathFactoryBase(const FactoriesConfig& facConfig) : PaxFPFBaseData(facConfig) {}

  virtual ~PaxFarePathFactoryBase() = default;

  // Pure virtual interface
  virtual bool initPaxFarePathFactory(DiagCollector& diag) = 0;

  virtual FPPQItem* getFPPQItem(uint32_t idx, DiagCollector& diag) = 0;

  // Get first 0-amount FP for infant that passes CAT13 validation
  virtual FPPQItem* getFirstValidZeroAmountFPPQItem(uint32_t idx,
                                                    const FPPQItem& primaryFPPQItem,
                                                    DiagCollector& diag,
                                                    uint32_t* newIdx) = 0;

  virtual FPPQItem*
  getSameFareBreakFPPQItem(const FPPQItem* primaryFPPQItem, DiagCollector& diag) = 0;

  virtual FPPQItem* getAlreadyBuiltFPPQItem(uint32_t idx) = 0;

  virtual void removeFPF(DiagCollector* diag,
                         const FareMarketPath* fareMarketPathID,
                         const DatePair* datePair = nullptr,
                         bool nonThruPricing = false) = 0;

  virtual bool checkFailedFPIS(FPPQItem* fppqItem, std::set<DatePair>& datePairSet, bool first) = 0;

  virtual void clear() = 0;

  // Non virtual interface
  void setThroughFarePricing() { _throughFarePricing = true; }

  const std::vector<FarePathFactory*>& farePathFactoryBucket() const
  {
    return _farePathFactoryBucket;
  }
  std::vector<FarePathFactory*>& farePathFactoryBucket() { return _farePathFactoryBucket; }

  const std::vector<PUPathMatrix*>& puPathMatrixVect() const { return _puPathMatrixVect; }
  std::vector<PUPathMatrix*>& puPathMatrixVect() { return _puPathMatrixVect; }

  const std::vector<SavedFPPQItem>& getSavedFPPQItems() const { return _savedFPPQItems; }

  const PricingUnitFactoryBucket* puFactoryBucket() const { return _puFactoryBucket; }
  PricingUnitFactoryBucket*& puFactoryBucket() { return _puFactoryBucket; }

  const bool& primaryPaxType() const { return _primaryPaxType; }
  bool& primaryPaxType() { return _primaryPaxType; }

  const bool pricingAxess() const { return _pricingAxess; }
  bool& pricingAxess() { return _pricingAxess; }

  bool isEoeCombinabilityEnabled() const { return _eoeCombinabilityEnabled; }
  void setEoeCombinabilityEnabled(bool eoeCombinabilityEnabled)
  {
    _eoeCombinabilityEnabled = eoeCombinabilityEnabled;
  }

protected:
  std::vector<FarePathFactory*> _farePathFactoryBucket;
  std::vector<PUPathMatrix*> _puPathMatrixVect;
  std::vector<SavedFPPQItem> _savedFPPQItems;
  PricingUnitFactoryBucket* _puFactoryBucket = nullptr;
  bool _primaryPaxType = false;
  bool _pricingAxess = false;
  bool _throughFarePricing = false; // for estimated pricing of 500-Itin MIP
  bool _eoeCombinabilityEnabled = true;
};
}

