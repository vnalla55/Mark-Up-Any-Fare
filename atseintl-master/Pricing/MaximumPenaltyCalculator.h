#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "Pricing/Cat16MaxPenaltyCalculator.h"
#include "RexPricing/RefundPermutationGenerator.h"
#include "RexPricing/ReissuePenaltyCalculator.h"

#include <boost/optional.hpp>
#include <algorithm>
#include <vector>
#include <list>

namespace tse
{
class FarePath;
class Money;
class PricingTrx;
class DiagManager;

class MaximumPenaltyCalculator
{
  friend class MaximumPenaltyCalculatorTest;

public:
  typedef std::vector<ReissuePenaltyCalculator::FcFees> FcFeesVec;
  typedef std::map<const PaxTypeFare*, std::size_t> PtfPuMap;

public:
  MaximumPenaltyCalculator(PricingTrx& trx, FarePath& farePath);

  MaxPenaltyResponse::Fees
  changePenalty(smp::RecordApplication departureInd, const CurrencyCode& currencyCode) const;

  MaxPenaltyResponse::Fees
  refundPenalty(smp::RecordApplication departureInd, CurrencyCode currencyCode) const;

  MaxPenaltyResponse::Fees
  changePenaltyCat31(smp::RecordApplication departureInd, const CurrencyCode& currencyCode) const;

  MaxPenaltyResponse::Fees
  changePenaltyCat16(smp::RecordApplication departureInd, CurrencyCode currencyCode) const;

  MaxPenaltyResponse::Fees
  refundPenaltyCat33(smp::RecordApplication departureInd, CurrencyCode currencyCode) const;

  MaxPenaltyResponse::Fees
  refundPenaltyCat16(smp::RecordApplication departureInd, CurrencyCode currencyCode) const;

  boost::optional<MoneyAmount> getMaximum(FcFeesVec fees) const;

protected:
  MaxPenaltyResponse::Fees penaltyCat16(Cat16MaxPenaltyCalculator::PenaltyType penaltyType,
                                        smp::RecordApplication departureInd,
                                        CurrencyCode currencyCode,
                                        DiagManager& diag) const;

  smp::RecordApplication
  refundPermutationAppl(const RefundPermutation& permutation, const PtfPuMap& ptfPuMap) const;

  std::vector<const PaxTypeFare*> findInternationalFCs(std::vector<const PaxTypeFare*> fares,
                                                       uint32_t domesticFCNumber) const;
  const VoluntaryChangesInfoW*
  getOverridingRecord(const std::vector<CarrierCode>& carriers,
                      const std::vector<const PaxTypeFare*>& fareComponents) const;

  bool isOverridePossible() const;

  void overrideDomesticRecords() const;

  const std::vector<CarrierCode> getCarrierList(const VoluntaryChangesInfoW& record,
                                                      const FareMarket& fm) const;
  smp::RecordApplication validateFullyFlown(const RefundPermutation& permutation) const;

  void printDiagnostic555RefundCat33(DiagManager& diag,
                                     const RefundPermutation& perm,
                                     const bool& refundable,
                                     const PtfPuMap& ptfPuMap) const;
  PtfPuMap mapPaxTypeFaresToPricingUnit() const;
  bool isAnyFCPartiallyFlownAfter(const RefundPermutationGenerator::FCtoSequence& seqsByFc,
                                  const PtfPuMap& ptfPuMap) const;

private:
  PricingTrx& _trx;
  FarePath& _farePath;
  bool _diagEnabled;
};

} /* namespace tse */
