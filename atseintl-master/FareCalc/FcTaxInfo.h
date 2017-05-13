#pragma once

#include "Common/ConfigList.h"
#include "Common/Money.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "FareCalc/Optional.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"

#include <functional>
#include <set>
#include <string>
#include <vector>

namespace tse
{

class PricingTrx;
class CalcTotals;
class TaxResponse;
class TaxRecord;
class TaxItem;
class PfcItem;
class FareCalcConfig;
class FareUsage;

class FcTaxInfoTest;

namespace FareCalc
{

struct TaxInfo
{
  std::vector<std::string> publishedZpTaxInfo;
  std::vector<std::string> zpTaxInfo;
  std::vector<std::string> xfTaxInfo;
  std::vector<std::string> xtTaxInfo;
};

/** Holds information about taxes for given fare usage or leg. */
struct SplitTaxInfo
{
  Money baseFare = INVALID_CURRENCYCODE;
  Money construction = INVALID_CURRENCYCODE;
  Money equivalent = INVALID_CURRENCYCODE;
  Money effectiveDeviation = INVALID_CURRENCYCODE;
  Money totalTax = INVALID_CURRENCYCODE;
  Money total = INVALID_CURRENCYCODE; // equivalent + tax

  int mileage = 0;

  std::vector<const TaxItem*> taxItems;
  std::vector<const TaxItem*> changeFeeTaxItems;
  std::vector<const PfcItem*> pfcItems;
  std::vector<const TaxRecord*> taxRecords;
  std::vector<TaxCode> taxExempts;
};

class FcTaxInfo : public Optional<TaxInfo>
{
  friend class tse::FcTaxInfoTest;
  friend class CalcTotalsTest;

public:
  FcTaxInfo() = default;
  FcTaxInfo(const TaxResponse* taxResponse) : _taxResponse(taxResponse) {}

  void initialize(PricingTrx* trx,
                  CalcTotals* calcTotals,
                  const FareCalcConfig* fcConfig,
                  const TaxResponse* taxResponse);

  const TaxResponse* taxResponse() const { return _taxResponse; }
  const std::vector<TaxRecord*>& taxRecords() const;
  const std::vector<TaxItem*>& taxItems() const;
  const std::vector<TaxItem*>& changeFeeTaxItems() const;
  const std::vector<PfcItem*>& pfcItems() const;

  const std::vector<std::string>& publishedZpTaxInfo() const { return value().publishedZpTaxInfo; }
  const std::vector<std::string>& zpTaxInfo() const { return value().zpTaxInfo; }
  const std::vector<std::string>& xfTaxInfo() const { return value().xfTaxInfo; }
  const std::vector<std::string>& xtTaxInfo() const { return value().xtTaxInfo; }

  MoneyAmount taxAmount() const { return _taxAmount; }
  MoneyAmount& taxAmount() { return _taxAmount; }
  int taxNoDec() const { return _taxNoDec; }
  CurrencyCode taxCurrencyCode() const { return _taxCurrencyCode; }
  MoneyAmount taxOverride() const { return _taxOverride; }
  const std::set<TaxCode>& getTaxExemptCodes() const { return _taxExemptCodes; }
  const bool dispSegmentFeeMsg() const { return _dispSegmentFeeMsg; }
  void subtractSupplementalFeeFromTaxAmount() { _taxAmount -= _supplementalFeeAmount; }

  using TaxesPerLeg = std::map<IntIndex, SplitTaxInfo>;
  using TaxesPerFareUsage = std::map<const FareUsage*, SplitTaxInfo>;

  template <typename Result, typename BucketFunctor>
  void assignTaxItemToResult(const TaxItem* item,
                             Result& result,
                             double totalConstructionAmount,
                             int splitBy,
                             BucketFunctor getBucket) const;
  void getTaxesSplitByLeg(TaxesPerLeg& result) const;
  void getTaxesSplitByFareUsage(TaxesPerFareUsage& result) const;

  const FcTaxInfo::TaxesPerFareUsage&
  getFareUsage2SplitTaxInfo4Pricing() const { return _fareUsage2SplitTaxInfo4Pricing; }

  void computeTaxSummaries(SplitTaxInfo& taxes) const;
  void computeSplitTotals(SplitTaxInfo& taxes) const;
  void computeTaxSummariesAndTotals(SplitTaxInfo& taxes) const;

  class Zerop
  {
  public:
    bool operator()(const TaxRecord* t) const
    {
      return (t->getTaxAmount() < EPSILON && t->isTaxFeeExempt() == false);
    }
  };

protected:
  void compute(TaxInfo& taxInfo) const override;

  MoneyAmount getTaxOverride();

  void collectZpTaxInfo(TaxInfo& taxInfo) const;
  void collectXfTaxInfo(TaxInfo& taxInfo, bool xtOutput) const;
  bool collectTicketingTaxInfo(TaxInfo& taxInfo) const;

private:
  int numberOfLegs() const;

  bool isFlatTax(const TaxItem* item) const;
  bool isPercentageTax(const TaxItem* item) const;

  void calculateRealBaseFaresForLegs(TaxesPerLeg& result) const;
  void calculateEstimatedBaseFaresForLegs(TaxesPerLeg& result) const;
  void calculateEffectivePriceDeviations(TaxesPerLeg& result) const;

  void computeSplitTaxInfo4Pricing();

  PricingTrx* _trx = nullptr;
  CalcTotals* _calcTotals = nullptr;
  const FareCalcConfig* _fcConfig = nullptr;
  const TaxResponse* _taxResponse = nullptr;

  mutable std::vector<TaxRecord*> _taxRecords;
  mutable std::vector<TaxItem*> _taxItems;
  mutable std::vector<TaxItem*> _changeFeeTaxItems;
  mutable std::vector<PfcItem*> _pfcItems;

  MoneyAmount _taxAmount = 0.0;
  MoneyAmount _supplementalFeeAmount = 0.0;
  int _taxNoDec = 0;
  CurrencyCode _taxCurrencyCode;
  MoneyAmount _taxOverride = 0.0;
  std::set<TaxCode> _taxExemptCodes;
  bool _dispSegmentFeeMsg = false;

  FcTaxInfo::TaxesPerFareUsage _fareUsage2SplitTaxInfo4Pricing;
};
}
} // namespace tse::FareCalc
