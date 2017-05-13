#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/CategoryRuleItemSet.h"
#include "Rules/FareMarketDataAccess.h"
#include "Rules/RuleProcessingData.h"
#include "Rules/StopoversInfoWrapper.h"
#include "Rules/TransfersInfoWrapper.h"


#include <vector>

namespace tse
{
class PricingTrx;
class FareByRuleApp;
class FareByRuleItemInfo;
class FareMarket;
class GeneralFareRuleInfo;
class Itin;
class PaxTypeFare;
class Rec2Wrapper;

class FareByRuleOverrideShallowScanner
{
  friend class FareByRuleOverrideShallowScannerTest;

public:
  FareByRuleOverrideShallowScanner(PricingTrx& trx,
                                   const FareMarket& fareMarket,
                                   Itin& itin,
                                   const FareByRuleApp& fbrApp,
                                   const FareByRuleCtrlInfo& fbrCtrlInfo,
                                   const FareByRuleItemInfo& fbrItemInfo,
                                   PaxTypeFare& dummyPtFare,
                                   TariffCategory tarrifCategory);

  virtual ~FareByRuleOverrideShallowScanner() = default;

  bool isValid();

private:
  bool performShallowScan(uint16_t categoryNumber);
  virtual const std::vector<GeneralFareRuleInfo*>&
  getAllPossiblyMatchingRec2s(uint16_t categoryNumber);
  virtual bool validate(const Rec2Wrapper& rec2wrapper);
  bool passSystemAssumption(uint16_t categoryNumber);

  PricingTrx& _trx;
  const FareMarket& _fareMarket;
  Itin& _itin;
  const FareByRuleApp& _fbrApp;
  const FareByRuleCtrlInfo& _fbrCtrlInfo;
  const FareByRuleItemInfo& _fbrItemInfo;
  std::vector<uint16_t> _fbrShallowScanCategories;
  PaxTypeFare& _dummyPtFare;
  TariffCategory _tarrifCategory;
  CategoryRuleItemSet _categoryRuleItemSet;
  RuleProcessingData _rpData;
  FareMarketDataAccess _fareMarketDataAccess;
  bool _skipSecurity;
  StopoversInfoWrapper _soInfoWrapper;
  TransfersInfoWrapper _trInfoWrapper;

  DiagManager _diag;
  bool _isDiagActive;
};
}
