//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseEnums.h"
#include "DataModel/PricingUnit.h"


namespace tse
{
class PricingTrx;
class FarePath;
class FareUsage;
class DiagCollector;

class CombinabilityScoreboard final
{
  friend class Combinations;
  friend class CombinabilityScoreboardTest;

public:
  CombinabilityScoreboard() = default;
  CombinabilityScoreboard(const CombinabilityScoreboard&) = delete;
  CombinabilityScoreboard& operator=(const CombinabilityScoreboard&) = delete;

  PricingTrx*& trx() { return _trx; }
  const PricingTrx* trx() const { return _trx; }

  static void enableFailedFareUsageOptimization(bool enableIt)
  {
    _enableFailedFareUsageOptimization = enableIt;
  }

private:
  CombinabilityValidationResult
  validate(PricingUnit& pu, FareUsage*& failedFareUsage, DiagCollector& diag);

  bool isMirrorImage(PricingUnit& pu);

  CombinabilityValidationResult
  invalidate(PricingUnit& pu, FareUsage*& failedFareUsage, DiagCollector& diag);

  CombinabilityValidationResult invalidateRoundTrip(PricingUnit& pu, DiagCollector& diag);
  CombinabilityValidationResult checkCt2IndicatorToInvalidate(PricingUnit& pu,
                                                              bool mirrorImage,
                                                              const CombinabilityRuleInfo& pCat10,
                                                              DiagCollector& diag);
  void checkCt2FareRuleData(const CombinabilityRuleInfo& pCat10,
                            bool& chk102Carrier,
                            bool& chk102Rule,
                            bool& chk102Tariff,
                            bool& chk102Class,
                            bool& chk102Type);

  CombinabilityValidationResult
  invalidateCircleTrip(PricingUnit& pu, FareUsage*& failedFareUsage, DiagCollector& diag);
  CombinabilityValidationResult
  checkCt2PlusIndicatorToInvalidate(PricingUnit& pu,
                                    const CombinabilityRuleInfo& pCat10,
                                    DiagCollector& diag);
  void checkCt2PlusFareRuleData(const CombinabilityRuleInfo& pCat10,
                                bool& chk103Carrier,
                                bool& chk103Rule,
                                bool& chk103Tariff,
                                bool& chk103Class,
                                bool& chk103Type);

  CombinabilityValidationResult
  invalidateOpenJaw(PricingUnit& pu, FareUsage*& failedFareUsage, DiagCollector& diag);
  CombinabilityValidationResult checkOJIndicatorToInvalidate(PricingUnit& pu,
                                                             const CombinabilityRuleInfo& pCat10,
                                                             DiagCollector& diag);
  void checkOJFareRuleData(const CombinabilityRuleInfo& pCat10,
                           bool& chk101Carrier,
                           bool& chk101Rule,
                           bool& chk101Tariff,
                           bool& chk101Class,
                           bool& chk101Type);

  CombinabilityValidationResult invalidateOneWay(PricingUnit& pu, DiagCollector& diag);
  CombinabilityValidationResult checkOWIndicatorToInvalidate(PricingUnit& pu,
                                                             const CombinabilityRuleInfo& pCat10,
                                                             DiagCollector& diag);
  void checkOWFareRuleData(const CombinabilityRuleInfo& pCat10,
                           bool& chk104Carrier,
                           bool& chk104Rule,
                           bool& chk104Tariff,
                           bool& chk104Class,
                           bool& chk104Type);

  void comparePaxTypeFares(const PaxTypeFare& firstPtf,
                           const PaxTypeFare& secondPtf,
                           size_t& sameCarrierCount,
                           size_t& sameRuleCount,
                           size_t& sameTariffCount,
                           size_t& sameClassCount,
                           size_t& sameTypeCount) const;
  CombinabilityValidationResult invalidateRuleAndFareData(PricingUnit& pu,
                                                          DiagCollector& diag,
                                                          bool checkCarrier,
                                                          size_t sameCarrierCount,
                                                          bool checkRule,
                                                          size_t sameRuleCount,
                                                          bool checkTariff,
                                                          size_t sameTariffCount,
                                                          bool checkClass,
                                                          size_t sameClassCount,
                                                          bool checkType,
                                                          size_t sameTypeCount) const;

  CombinabilityValidationResult analyzeOneWay(FarePath& farePath, DiagCollector& diag);
  CombinabilityValidationResult checkOWIndicatorOnFarePath(PricingUnit& pu,
                                                           FareUsage& fareUsage,
                                                           const CombinabilityRuleInfo& pCat10,
                                                           DiagCollector& diag);

  CombinabilityValidationResult analyzeOpenJaw(PricingUnit& pu, DiagCollector& diag);
  CombinabilityValidationResult checkOJIndicatorToValidate(PricingUnit& pu,
                                                           FareUsage& fareUsage,
                                                           const CombinabilityRuleInfo& pCat10,
                                                           DiagCollector& diag);

  CombinabilityValidationResult analyzeRoundTrip(PricingUnit& pu, DiagCollector& diag);
  CombinabilityValidationResult checkRTIndicatorToValidate(const PricingUnit& pu,
                                                           FareUsage& fareUsage,
                                                           bool mirrorImage,
                                                           const CombinabilityRuleInfo& pCat10,
                                                           DiagCollector& diag);

  void processSamePointTable993(PricingUnit& pu, DiagCollector& diag);
  bool processSamePointTable993(const FareUsage& curFareUsage,
                                const FareUsage& preFareUsage,
                                DiagCollector& diag);
  bool processSamePointTable993(const FareUsage& fareUsage,
                                const LocCode& loc1,
                                const LocCode& loc2,
                                const DateTime& travelDate,
                                DiagCollector& diag);

  CombinabilityValidationResult
  checkRec2(PricingUnit& pu, FareUsage& fareUsage, DiagCollector& diag);

private:
  enum DiagnosticID
  {
    PASSED_VALIDATION,
    PASSED_SYSTEM_ASSUMPTION,
    INVALID_DIAGNOSTIC_TOO_FEW_FARES,
    FAILED_UNSPECIFIED,
    FAILED_REC_2_CAT_10_NOT_APPLICABLE,
    FAILED_NO_REC_2_CAT_10,
    FAILED_REC2_SCOREBOARD,
    FAILED_ROUND_TRIP_NOT_PERMITTED,
    FAILED_SAME_CARRIER_REQUIRED_FOR_RT,
    FAILED_SAME_RULE_REQUIRED_FOR_RT,
    FAILED_SAME_TARIFF_REQUIRED_FOR_RT,
    FAILED_SAME_FARECLASS_REQUIRED_FOR_RT,
    FAILED_SAME_FARETYPE_REQUIRED_FOR_RT,
    FAILED_CIRCLE_TRIP_NOT_PERMITTED,
    FAILED_SAME_CARRIER_REQUIRED_FOR_CT,
    FAILED_SAME_RULE_REQUIRED_FOR_CT,
    FAILED_SAME_TARIFF_REQUIRED_FOR_CT,
    FAILED_SAME_FARECLASS_REQUIRED_FOR_CT,
    FAILED_SAME_FARETYPE_REQUIRED_FOR_CT,
    FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED,
    FAILED_DOUBLE_OPEN_JAW_NOT_PERMITTED,
    FAILED_SAME_CARRIER_REQUIRED_FOR_OPEN_JAW,
    FAILED_SAME_RULE_REQUIRED_FOR_OPEN_JAW,
    FAILED_SAME_TARIFF_REQUIRED_FOR_OPEN_JAW,
    FAILED_SAME_FARECLASS_REQUIRED_FOR_OPEN_JAW,
    FAILED_SAME_FARETYPE_REQUIRED_FOR_OPEN_JAW,
    FAILED_END_ON_END_NOT_PERMITTED,
    FAILED_END_ON_END_SAME_CARRIER_REQUIRED,
    FAILED_END_ON_END_SAME_RULE_REQUIRED,
    FAILED_END_ON_END_SAME_TARIFF_REQUIRED,
    FAILED_END_ON_END_SAME_FARECLASS_REQUIRED,
    FAILED_END_ON_END_SAME_FARETYPE_REQUIRED,
    FAILED_DESTINATION_OPEN_JAW_REQUIRED,
    FAILED_ORIGIN_OPEN_JAW_REQUIRED,
    FAILED_2_MAX_INTL_FARES_SAME_COUNTRY_REQUIRED,
    FAILED_OJ_DIFF_COUNTRY_REQUIRED,
    FAILED_OJ_SAME_COUNTRY_REQUIRED,
    FAILED_MIRROR_IMAGE_NOT_PERMITTED,
    FAILED_SIDE_TRIP_NOT_PERMITTED,
    FAILED_REQUIRED_EOE_WITH_OTHER_PU
  };

  enum DiagnosticIDGroup
  {
    FAILED_SAME_CARRIER_REQUIRED,
    FAILED_SAME_RULE_REQUIRED,
    FAILED_SAME_TARIFF_REQUIRED,
    FAILED_SAME_FARECLASS_REQUIRED,
    FAILED_SAME_FARETYPE_REQUIRED
  };

  void displayDiag(DiagCollector& diag, const PricingUnit& pu, DiagnosticID failureReason) const;
  bool displayRoundTripDiag(DiagCollector& diag,
                            const PricingUnit& pu,
                            DiagnosticID failureReason,
                            const FareUsage* sourceFareUsage,
                            const FareUsage* targetFareUsage) const;
  bool displayCircleTripDiag(DiagCollector& diag,
                             const PricingUnit& pu,
                             DiagnosticID failureReason,
                             const FareUsage* sourceFareUsage,
                             const FareUsage* targetFareUsage) const;
  bool displayOpenJawDiag(PricingUnit::PUSubType puSubType,
                          DiagCollector& diag,
                          const PricingUnit& pu,
                          DiagnosticID failureReason,
                          const FareUsage* sourceFareUsage,
                          const FareUsage* targetFareUsage) const;
  bool displayOneWayDiag(DiagCollector& diag,
                         const PricingUnit& pu,
                         DiagnosticID failureReason,
                         const FareUsage* sourceFareUsage,
                         const FareUsage* targetFareUsage) const;
  void displayFailureReason(DiagCollector& diag,
                            const PricingUnit& pu,
                            DiagnosticID expectedFailureReason,
                            DiagnosticID failureReason,
                            const FareUsage* sourceFareUsage,
                            const FareUsage* targetFareUsage) const;
  bool displayDiagForRuleAndFareData(DiagCollector& diag,
                                     const PricingUnit& pu,
                                     DiagnosticID failureReason,
                                     PricingUnit::Type puType,
                                     const FareUsage* sourceFareUsage,
                                     const FareUsage* targetFareUsage,
                                     bool checkCarrier,
                                     bool checkRule,
                                     bool checkTariff,
                                     bool checkClass,
                                     bool checkType) const;

  void displayDiag(DiagCollector& diag,
                   const PricingUnit& pu,
                   DiagnosticID errNum,
                   const FareUsage* sourceFareUsage,
                   const FareUsage* targetFareUsage) const;

  void outputDiag(DiagCollector& diag,
                  const char* prefix,
                  const char* suffix = nullptr,
                  const FareUsage* sourceFareUsage = nullptr,
                  const FareUsage* targetFareUsage = nullptr) const;

  DiagnosticID getErrorID(DiagnosticIDGroup idGroup, PricingUnit::Type puType) const;

  bool checkSideTripPermitted(Indicator eoeInd, PricingUnit& pu, FareUsage& fu) const;

  PricingTrx* _trx = nullptr;
  static bool _enableFailedFareUsageOptimization;
};
} /* end tse namespace */

