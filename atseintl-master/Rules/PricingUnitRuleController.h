#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Rules/RuleController.h"

namespace tse
{
class FareUsage;
class PricingUnit;
class FarePath;
class RuleProcessingData;
class FarePUResultContainer;

class PricingUnitRuleController : public RuleController
{
public:
  PricingUnitRuleController() { _fmPhase = false; }

  PricingUnitRuleController(CategoryPhase phase) : RuleController(phase) { _fmPhase = false; }

  PricingUnitRuleController(CategoryPhase phase, const std::vector<uint16_t>& categories)
    : RuleController(phase, categories)
  {
    _fmPhase = false;
  }

  virtual bool validate(PricingTrx& trx, FarePath& farePath, PricingUnit& pricingUnit);

  virtual bool
  validate(PricingTrx& trx, FarePath& farePath, PricingUnit& pricingUnit, FareUsage& fareUsage);

  virtual bool
  validate(PricingTrx& trx, PricingUnit& pricingUnit, FareUsage& fareUsage, Itin& itin);

  virtual bool validate(PricingTrx& trx, PricingUnit& pricingUnit, Itin& itin);

  virtual bool
  validate(PricingTrx& trx, PricingUnit& pricingUnit, FareUsage*& failedFareUsage, Itin& itin);

  bool processCategorySequence(PricingTrx&,
                               FarePath& farePath,
                               PricingUnit& pricingUnit,
                               FareUsage& fareUsage,
                               std::vector<uint16_t>&);

  bool processCategorySequence(PricingTrx&,
                               PricingUnit& pricingUnit,
                               FareUsage& fareUsage,
                               std::vector<uint16_t>&,
                               Itin& itin);

  bool processCategorySequenceSub(PricingTrx&,
                                  FarePath& farePath,
                                  PricingUnit& pricingUnit,
                                  FareUsage& fareUsage,
                                  std::vector<uint16_t>&);
  Record3ReturnTypes doCategoryPostProcessing(PricingTrx& trx,
                                              RuleControllerDataAccess& da,
                                              const uint16_t category,
                                              RuleProcessingData& rpData,
                                              const Record3ReturnTypes preResult) override;

  void setFareResultContainer(FarePUResultContainer* container)
  {
    _fareResultContainer = container;
  }
  void preparePtfVector(std::vector<PaxTypeFare*>& ptfVec,
                        const PricingTrx& trx,
                        const FareMarket& fm) const;

protected:
  Record3ReturnTypes validateBaseFare(uint16_t category,
                                      const FareByRuleItemInfo* fbrItemInfo,
                                      bool& checkFare,
                                      PaxTypeFare* fbrFare,
                                      RuleControllerDataAccess& da) override;

  Record3ReturnTypes revalidateC15BaseFareForDisc(uint16_t category,
                                                  bool& checkFare,
                                                  PaxTypeFare* ptf,
                                                  RuleControllerDataAccess& da) override;

  Record3ReturnTypes callCategoryRuleItemSet(CategoryRuleItemSet& catRuleIS,
                                             const CategoryRuleInfo&,
                                             const std::vector<CategoryRuleItemInfoSet*>&,
                                             RuleControllerDataAccess& da,
                                             RuleProcessingData& rpData,
                                             bool isLocationSwapped,
                                             bool isFareRule,
                                             bool skipCat15Security) override;

  void applySurchargeGenRuleForFMS(PricingTrx& trx,
                                   RuleControllerDataAccess& da,
                                   uint16_t categoryNumber,
                                   RuleControllerParam& rcParam,
                                   bool skipCat15Security) override;
  Record3ReturnTypes reValidDiscQualifiers(PaxTypeFare& paxTypeFare,
                                           const uint16_t& category,
                                           RuleControllerDataAccess& da) override;

  void adjustStopoversCharges(FarePath& farePath, PricingUnit& pricingUnit);

  void adjustTransfersCharges(FarePath& farePath, PricingUnit& pricingUnit);

  bool chargeFromFirstInbound(FareUsage* fareUsage);

  bool inactiveExcDiag(const RexBaseTrx& rexBaseTrx, const DiagCollector& diag) const;

  Record3ReturnTypes applySystemDefaultAssumption(PricingTrx& trx,
                                                  RuleControllerDataAccess& da,
                                                  const uint16_t category,
                                                  bool& displayDiag) override;

  void print555Fare(const PricingTrx& trx, DiagCollector& diag, const PaxTypeFare& ptf) const;

protected:
  // for unit test only
  explicit PricingUnitRuleController(int) : RuleController(1) {}

  void doReuseDiag(DiagCollector& diag,
                   bool isValid,
                   PricingTrx& trx,
                   const PaxTypeFare* ptf,
                   const FareUsage& fareUsage);

  FarePUResultContainer* _fareResultContainer = nullptr;
};
}
