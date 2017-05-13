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

#include "Common/TseConsts.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexBaseTrx.h"

namespace tse
{
class VoluntaryRefundsInfo;
class RefundPermutation;

class RefundPricingTrx : public RexBaseTrx
{
  friend class RefundPricingTrxTest;

public:
  RefundPricingTrx()
  {
    _excTrxType = PricingTrx::AF_EXC_TRX;
    _reqType = tse::AUTOMATED_REFUND;
  }

  void prepareRequest() override;
  void setUpSkipSecurityForExcItin() override;
  bool repriceWithSameFareDate() override;
  virtual void set(const RexBaseTrx::RequestTypes& reqTypes) override;
  virtual const DateTime& getRuleApplicationDate(const CarrierCode& govCarrier) const override;

  bool process(Service& srv) override { return srv.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  void setAnalyzingExcItin(const bool isAnalyzingExcItin) override;

  std::vector<TravelSeg*>& travelSeg() override;
  const std::vector<TravelSeg*>& travelSeg() const override;

  bool& fullRefund() { return _fullRefund; }
  bool fullRefund() const { return _fullRefund; }

  void setTktValidityDate() override {}

  using Options = std::multimap<const PaxTypeFare*, const VoluntaryRefundsInfo*>;

  void insertOption(const PaxTypeFare* ptf, const VoluntaryRefundsInfo* rec3);

  size_t refundOptions(const PaxTypeFare* ptf);

  const Options& refundOptions() const { return _refundOptions; }

  using Permutations = std::vector<RefundPermutation*>;

  Permutations& permutations() { return _permutations; }
  const Permutations& permutations() const { return _permutations; }

  std::set<const PaxTypeFare*>& reachedR3Validation() { return _reachedR3Validation; }
  const std::set<const PaxTypeFare*>& reachedR3Validation() const { return _reachedR3Validation; }

  void prepareNewFareMarkets();

  virtual Money convertCurrency(const Money& source, const CurrencyCode& targetCurr) const;

  virtual void
  getRec2Cat10WithVariousRetrieveDates(std::vector<MergedFareMarket*>& mergedFareMarketVect,
                                       GetRec2Cat10Function getRec2Cat10) override;

  using WaivedRecord3Set = std::set<const VoluntaryRefundsInfo*>;

  WaivedRecord3Set& waivedRecord3() { return _waivedRecord3; }
  const WaivedRecord3Set& waivedRecord3() const { return _waivedRecord3; }

  void setFullRefundWinningPermutation(const RefundPermutation& perm);
  const RefundPermutation* fullRefundWinningPermutation() const
  {
    return _fullRefundWinningPermutation;
  }

  bool& arePenaltiesAndFCsEqualToSumFromFareCalc()
  {
    return _arePenaltiesAndFCsEqualToSumFromFareCalc;
  }

  bool arePenaltiesAndFCsEqualToSumFromFareCalc() const
  {
    return _arePenaltiesAndFCsEqualToSumFromFareCalc;
  }

  virtual void
  initalizeForRedirect(std::vector<FareMarket*>& fm, std::vector<TravelSeg*>& ts) const override;

  virtual void skipRulesOnExcItin(std::vector<uint16_t>& categorySequence) const override;

  virtual bool isPlusUpCalculationNeeded() const override
  {
    return !arePenaltiesAndFCsEqualToSumFromFareCalc();
  }

private:
  std::vector<FareMarket::RetrievalInfo*> getPermutationsRetrievalInfo() const;

  bool _fullRefund = false;
  Options _refundOptions;
  Permutations _permutations;
  std::set<const PaxTypeFare*> _reachedR3Validation;
  WaivedRecord3Set _waivedRecord3;
  const RefundPermutation* _fullRefundWinningPermutation = nullptr;
  bool _arePenaltiesAndFCsEqualToSumFromFareCalc = false;
};
} // tse namespac3
