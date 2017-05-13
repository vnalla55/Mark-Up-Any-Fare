//-------------------------------------------------------------------
//  Copyright Sabre 2010
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

#include "Common/CurrencyConversionRequest.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/MaxPenaltyInfo.h"

#include <unordered_set>

namespace tse
{
class DiagManager;
class DiscountInfo;
class PricingTrx;
class FarePathChangeDetermination;
class Diag689Collector;
class PaxTypeFare;
class PaxTypeInfo;
class ProcessTagInfo;
class VoluntaryChangesInfo;
class VoluntaryChangesInfoW;
class ProcessTagPermutation;
class Logger;

class ReissuePenaltyCalculator
{
  friend class ReissuePenaltyCalculatorTest;

public:
  using ConversionType = CurrencyConversionRequest::ApplicationType;
  using FcFee = std::tuple<MoneyAmount, Indicator, const VoluntaryChangesInfoW*>;
  using FcFees = std::vector<FcFee>;

  ReissuePenaltyCalculator()
    : _trx(nullptr),
      _permutation(nullptr),
      _reissueCharges(nullptr),
      _dc(nullptr),
      _calculationCurrency(nullptr),
      _farePathChangeDetermination(nullptr),
      _applScenario('0'),
      _infantWithSeat(false),
      _infantWithoutSeat(false),
      _validatingCarrier(nullptr),
      _paxTypeInfo(nullptr),
      _conversionType(ConversionType::OTHER)
  {
  }

  void initialize(PricingTrx& trx,
                  const CurrencyCode& calculationCurrency,
                  const FarePathChangeDetermination& farePathChangeDetermination,
                  const CarrierCode& validatingCarrier,
                  const PaxTypeInfo& paxTypeInfo,
                  ProcessTagPermutation* permutation,
                  Diag689Collector* dc);

  virtual ~ReissuePenaltyCalculator() {}

  ReissueCharges* process();

  FcFees getPenalties(const std::unordered_set<const VoluntaryChangesInfoW*>& records,
                      const PaxTypeFare& ptf,
                      smp::RecordApplication departureInd,
                      DiagManager& diag);
  static bool
  isMatchingRecord(const VoluntaryChangesInfoW* record3, smp::RecordApplication departureInd);

  static bool isPenalty1HigherThanPenalty2(PenaltyFee& penalty1,
                                           PenaltyFee& penalty2,
                                           const CurrencyCode& _equivalentCurrency,
                                           PricingTrx& _trx);

  static constexpr Indicator HIGHEST_OF_CHANGED_FC = '1';
  static constexpr Indicator HIGHEST_OF_ALL_FC = '2';
  static constexpr Indicator EACH_OF_CHANGED_FC = '3';
  static constexpr Indicator HIGHEST_FROM_CHANGED_PU = '4';
  static constexpr Indicator HIGHEST_FROM_CHANGED_PU_ADDS = '5';

protected:
  static constexpr char HIGH_PENALTY = 'H';
  static constexpr char JOURNEY_APPL = 'J';
  static constexpr char PU_APPL = 'P';

  static constexpr Indicator NO_APPLY = ' ';
  static constexpr Indicator INFANT = '1';
  static constexpr Indicator CHILD = '2';
  static constexpr Indicator YOUTH = '3';
  static constexpr Indicator SENIOR = '4';
  static constexpr Indicator OTHER_19 = '5';
  static constexpr Indicator OTHER_22 = '6';
  static constexpr Indicator INFANT_WITH_SEAT = '7';
  static constexpr Indicator INFANT_WITHOUT_SEAT = '8';
  static constexpr Indicator INFANT_WITHOUT_SEAT_NOFEE = '9';
  static constexpr Indicator INFANT_WITH_SEAT_CAT19 = '0';

  static const PaxTypeCode YOUTH_CODE;
  static const PaxTypeCode SENIOR_CODE;

  static const std::vector<PaxTypeCode> YOUTH_CODES;
  static const std::vector<PaxTypeCode> SENIOR_CODES;

  static constexpr Indicator NO_DEPARTUE_RESTR = ' ';

  void calculatePenaltyFee(const PaxTypeFare& paxTypeFare,
                           const VoluntaryChangesInfoW& volutaryChangesInfo,
                           PenaltyFee& penaltyFee);
  void accumulateCharges();
  bool isPenalty1HigherThanPenalty2(PenaltyFee& penalty1, PenaltyFee& penalty2);
  Percent calculateDiscount(const ProcessTagInfo& tag);
  MoneyAmount getDiscountApplicable(Indicator type, const PaxTypeFare& paxTypeFare);
  void adjustChangeFeeByMinAmount();

  void populateChargesInEquivAndCalcCurr();

  void setUpInfantPassangerTypes();
  void determineApplicationScenario();
  Indicator getApplicationScenarioForComponent(const VoluntaryChangesInfo& rec3);
  Indicator adaptOldApplicationScenario(const VoluntaryChangesInfo& rec3);
  bool validatingCarrierOwnFareComponent(const CarrierCode& validatingCxr);

  void calculatePenaltyFees();
  bool considerFareComponent(const PaxTypeFare& fareComponent) const;

  void insertFee(const ProcessTagInfo& pti, const PenaltyFee& fee);
  bool isInTrxBasedPenaltyFeeCache(const ProcessTagInfo& pti);

  void insertIntoPrintingStructure(const ProcessTagInfo& pti, const PenaltyFee* fee);
  void insertCurrenciesForOldProcess();

  void calculateDiscountedPenaltyFee(const ProcessTagInfo& pti);

  Percent getChildrenDiscount(Indicator type,
                              const PaxTypeInfo& pti,
                              const PaxTypeCode& paxCode,
                              const DiscountInfo& di) const;
  void print();
  void markChargedPenalties();
  void clearReissueCharges();

  struct PrintInfo
  {
    const ProcessTagInfo* pti;
    const PenaltyFee* fee;
    Indicator applicationScenario;
    bool charged;
  };

  PricingTrx* _trx;
  ProcessTagPermutation* _permutation;
  ReissueCharges* _reissueCharges;
  std::map<const PaxTypeFare*, const VoluntaryChangesInfoW*> _fareVoluntaryChangeMap;
  std::set<CurrencyCode> _chargeCurrency;
  CurrencyCode _equivalentCurrency;
  Diag689Collector* _dc;
  const CurrencyCode* _calculationCurrency;

  std::vector<PrintInfo> _printInfos;
  const FarePathChangeDetermination* _farePathChangeDetermination;
  Indicator _applScenario;
  bool _infantWithSeat;
  bool _infantWithoutSeat;
  const CarrierCode* _validatingCarrier;
  const PaxTypeInfo* _paxTypeInfo;
  ConversionType _conversionType;

private:
  static Logger _logger;
};

} // tse
