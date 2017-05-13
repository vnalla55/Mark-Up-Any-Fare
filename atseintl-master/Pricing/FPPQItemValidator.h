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
#include "Pricing/PUPQItem.h"
#include "Pricing/SavedBaggageCharge.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

#include <string>

namespace tse
{
class BaggageCalculator;
class Combinations;
class DiagCollector;
class FactoriesConfig;
class FarePath;
class FarePathFactoryFailedPricingUnits;
class FPPQItem;
class FPPQItemValidatorTest;
class MixedClassController;
class PaxFPFBaseData;
class PricingTrx;
class PricingUnitFactory;
class PUPath;

enum class ValidationLevel : uint8_t
{ NORMAL_VALIDATION,
  SIMILAR_ITIN_FP_LEVEL,
  SIMILAR_ITIN_PU_LEVEL };

class FPPQItemValidator
{
  friend class ::tse::FPPQItemValidatorTest;

public:
  FPPQItemValidator(PaxFPFBaseData& pfpfBase,
                    const std::vector<PricingUnitFactory*>& allPUF,
                    FarePathFactoryFailedPricingUnits& failedPricingUnits,
                    DiagCollector& diag);
  void validate(FPPQItem* fpPQI,
                std::string& localRes,
                bool& localValid,
                bool& shouldPassCmdPricing,
                bool pricingAxess,
                const FPPQItem* topFppqItem,
                const ValidationLevel = ValidationLevel::NORMAL_VALIDATION);

  uint16_t getProcessedCategory() const { return _ruleController.getProcessedCategory(); }
  bool failedCategoryRevalidatedForSimilarItins() const
  {
    return _ruleController.doesPhaseContainCategory(FPRuleFamilyLogicChildrenValidation,
                                                    getProcessedCategory());
  }

  void validateRules(bool& localValid,
                     bool& shouldPassCmdPricing,
                     FPPQItem* fpPQI,
                     std::string& localRes,
                     ValidationLevel);
  void setYqyrCalc(YQYRCalculator* yqyrCalc) { _yqyrCalc = yqyrCalc; }
  void setBagCalculator(BaggageCalculator* bagCalc) { _bagCalculator = bagCalc; }

private:
  PricingTrx& _trx;
  const std::vector<PricingUnitFactory*>& _allPUF;
  const FactoriesConfig& _factoriesConfig;
  FarePathFactoryFailedPricingUnits& _failedPricingUnits;
  DiagCollector& _diag;
  RuleControllerWithChancelor<PricingUnitRuleController> _ruleController;
  Combinations* _combinations = nullptr;
  PUPath* _puPath = nullptr;
  YQYRCalculator* _yqyrCalc = nullptr;
  BaggageCalculator* _bagCalculator = nullptr;
  SavedBaggageCharge _savedBaggageCharge;

  void processMipAltDateSurcharges(FarePath& farePath);

  void validateJourneys(bool& shouldPassCmdPricing,
                        FPPQItem* fpPQI,
                        const bool isFareX,
                        std::string& localRes,
                        bool& localValid,
                        bool pricingAxess,
                        FarePath& fpI);

  void saveFPScopeFailedPUIdx(FPPQItem& fppqItem,
                              const uint16_t puFactIdx,
                              const MixedClassController& mcc,
                              std::string& res);
  bool checkJourney();
  bool flowJourney(FarePath& fpath, FareMarket& fm);

  BookingCode bookingCode(const FareUsage* fu,
                          const TravelSeg* tvlSeg,
                          const PaxTypeFare::SegmentStatus& fuSegStat);
  bool validateBookingCodes(FarePath& fpath);
  void applyDiffOverride(FarePath& fp);
  bool validForWQ(FarePath& fpath);
  bool wpaHigherCabin(FarePath& fpath);
  void wpaNoMatchHigherCabinValidation(bool& localValid, FarePath& fpI, std::string& localRes);
  CabinType
  cabin(const FareUsage* fu, const TravelSeg* tvlSeg, const PaxTypeFare::SegmentStatus& fuSegStat);
  void
  circleTripsValidation(bool& localValid, FarePath& fpI, bool plusUpsNeeded, std::string& localRes);
  bool ensureCircleTripCorrectlyTyped(FarePath& fpath,
                                      PricingUnit& prU,
                                      bool& fCorrected);
  void displayFarePathDiagnostics(std::string& localRes,
                                  FPPQItem* fpPQI,
                                  FarePath& fpI,
                                  bool& localValid);
  void checkIntlSurfaceTvlLimitation(FarePath& fpI);
  MoneyAmount getYQYRs(FarePath& farePath, ValidationLevel level);
  bool checkMinFare(FarePath& fpath, std::string& _results) const;
  bool
  isItemLessThanTopItem(const FPPQItem* topFppqItem, const FPPQItem& fppqItem, bool needValidate);
  bool validateBookingCodesCat31(FarePath& farePath) const;
  void resetToRebook(FarePath& fp, FPPQItem& fppqItem) const;
  bool isSameFarePathValidForRebook(FarePath& fp) const;
  bool callRefundValidation(std::string& results, FarePath& fpath);
  bool calcBaggageCharge(FarePath& fp);
};
} // tse namespace
