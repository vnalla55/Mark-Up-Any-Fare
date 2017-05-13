// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#pragma once
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/PaxFarePathFactoryBase.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/PUPQItem.h"
#include "Rules/FarePUResultContainer.h"
#include "Rules/PricingUnitLogic.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

#include <list>
#include <string>
#include <vector>

namespace tse
{
class Combinations;
class DiagCollector;
class FactoriesConfig;
class FarePath;
class FarePathFactoryFailedPricingUnits;
class FarePathValidatorTest;
class FPPQItem;
class PaxFPFBaseData;
class PricingTrx;
class PricingUnitFactory;
class FarePathValidator
{
  friend class ::tse::FarePathValidatorTest;

public:
  explicit FarePathValidator(PaxFPFBaseData& paxFPFBaseData);
  bool validate(FPPQItem& fppqitem, DiagCollector& diag, bool pricingAxess);
  void setSettings(const FarePathSettings& settings)
  {
    _allPUF = settings._allPUF;
    _failedPricingUnits = settings._failedPricingUnits;
    _eoeFailedFare = settings._eoeFailedFare;
    _externalLowerBoundAmount = settings._externalLowerBoundAmount;
  }

  void setEoeCombinabilityEnabled(bool isEoeCombinabilityEnabled)
  {
    _isEoeCombinabilityEnabled = isEoeCombinabilityEnabled;
  }

  const std::string& getResults() const { return _results; }
  void updateResults(std::string result) { _results+=result; }
  bool shouldPassCmdPricing() { return _shouldPassCmdPricing; }

  bool penaltyValidation(FarePath& farePath);
  void setPuPath(PUPath* puPath) { _puPath = puPath; }
  void setItin(Itin* itin) { _itin = itin; }
  bool checkFinalPUBusinessRule(FarePath& fpath, DiagCollector& diag);
  bool checkCombinability(FPPQItem& fppqItem, DiagCollector& diag);
  bool failFareRetailerCodeMismatch(const FarePath& farePath, DiagCollector& diag);

private:
  FarePathFactoryFailedPricingUnits* _failedPricingUnits = nullptr;
  std::vector<PricingUnitFactory*> const* _allPUF = nullptr;
  std::string _results = "";
  PricingTrx& _trx;
  const FactoriesConfig& _factoriesConfig;
  Combinations* _combinations = nullptr;
  PaxFPFBaseData& _paxFPFBaseData;

  PUPath* _puPath = nullptr;
  Itin* _itin = nullptr;
  std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>>* _eoeFailedFare = nullptr;
  bool _isEoeCombinabilityEnabled = true;
  bool _shouldPassCmdPricing = false;
  MoneyAmount _externalLowerBoundAmount = -1;

  void exchangeReissueForQrex(FarePath& fp);
  bool validateRexPaxTypeFare(DiagCollector& diag, FarePath& fpath);
  bool isValidFPathForValidatingCxr(FarePath& farePath, DiagCollector& diag);
  bool checkFailedPUIdx(FPPQItem& fppqItem,
                        std::vector<CarrierCode>& failedValCxr,
                        std::string& failedCxrMsg);

  bool checkPULevelCombinability(FPPQItem& fppqItem, DiagCollector& diag);

  bool checkEOECombinability(FPPQItem& fppqItem, DiagCollector& diag)
  {
    FareUsage* failedSourceFareUsage = nullptr;
    FareUsage* failedTargetFareUsage = nullptr;
    return checkEOECombinability(fppqItem, failedSourceFareUsage, failedTargetFareUsage, diag);
  }

  bool checkEOECombinability(FPPQItem& fppqItem,
                             FareUsage*& failedSourceFareUsage,
                             FareUsage*& failedTargetFareUsage,
                             DiagCollector& diag);
  bool failMixedDateForRefund(const FarePath& farePath) const;

  void saveFailedFare(const uint16_t puFactIdx,
                      FareUsage* failedFareUsage1,
                      FareUsage* failedFareUsage2);

  void recalculatePriority(FPPQItem& fppqItem);
  bool hardPassOnEachLegCheckIBF(FarePath& farePath);
  bool hardPassOnEachLegCheckPBB(FarePath& farePath);
  bool atLeastOneHardPass(TravelSegPtrVec& tSegs, FarePath& fPath, const BrandCode& brandCode);
  bool brandParityOnEachLegCheck(FarePath& farePath);
  bool checkContinuousNLSPFareLoop(FarePath& fpath, DiagCollector& diag);
  bool bypassForGvtPaxType(const FarePath& fpath);
  bool checkNLSPIntlOJToOW(const FarePath& fpath, DiagCollector& diag);
  bool checkSideTripRestriction(std::vector<PricingUnit*>& puVect, DiagCollector& diag);
  bool isTag1Tag1(const PricingUnit& pu1, const PricingUnit& pu2) const
  {
    return (pu1.fareUsage().front()->paxTypeFare()->owrt() == ONE_WAY_MAY_BE_DOUBLED &&
            pu2.fareUsage().front()->paxTypeFare()->owrt() == ONE_WAY_MAY_BE_DOUBLED);
  }
  bool isTag3Tag3(const PricingUnit& pu1, const PricingUnit& pu2) const
  {
    return (pu1.fareUsage().front()->paxTypeFare()->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED &&
            pu2.fareUsage().front()->paxTypeFare()->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED);
  }
  bool isAnyTag2(const PricingUnit& pu1, const PricingUnit& pu2) const;
  bool checkTag1Tag3FareInABAItin(const FarePath& fpath, DiagCollector& diag) const;
  bool checkExtFareInSideTripPU(const PricingUnit& pu, DiagCollector& diag);
  bool checkToOrViaCountryOfCommencement(const Loc& itinOrigin, const FareUsage& mtFareUsage);
  bool checkTag1Tag3CarrierPreference(const PricingUnit& prU1, const PricingUnit& prU2) const;
  bool checkPreCalculatedLoop(const FarePath& fpath, DiagCollector& diag);
  bool checkContinuousNLSPFareLoop(const PricingUnit::PUFareType fareType,
                                   std::vector<PricingUnit*>& puVect,
                                   const uint8_t stNumber,
                                   DiagCollector& diag);
  bool createsLoop(std::map<uint16_t, FareMarket*>& sortedFareMarket);
  bool originBasedSpecialCond(FPPQItem& fppqItem);
  void swapBackToFake(FPPQItem& fppqItem,
                      FareUsage*& fakeFareUsage,
                      FareUsage*& realFareUsage,
                      PaxTypeFare*& realPTF,
                      const uint16_t& fakeFareIndex,
                      const uint16_t& fakePUIndex);
  bool swapRealToFake(FPPQItem& fppqItem,
                      FareUsage*& fakeFareUsage,
                      FareUsage*& realFareUsage,
                      PaxTypeFare*& realPTF,
                      uint16_t& fakeFareIndex,
                      uint16_t& fakePUIndex);

  PUPQItem::PUValidationStatus
  getPUScopeCat10Status(const FPPQItem& fppqItem, const uint16_t puFactIdx);
  void setPUScopeCat10Status(const FPPQItem& fppqItem,
                             const uint16_t puFactIdx,
                             PUPQItem::PUValidationStatus status);
  void saveEOEFailedFare(const PaxTypeFare* paxTypeFare1, const PaxTypeFare* paxTypeFare2);

  bool checkEOEFailedFare(FPPQItem& fppqItem)
  {
    FareUsage* failedSourceFareUsage = nullptr;
    FareUsage* failedEOETargetFareUsage = nullptr;
    return checkEOEFailedFare(fppqItem, failedSourceFareUsage, failedEOETargetFareUsage);
  }
  bool checkEOEFailedFare(FPPQItem& fppqItem,
                          FareUsage*& failedSourceFareUsage,
                          FareUsage*& failedEOETargetFareUsage);
};
} // tse namespace
