#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/YQYR/YQYRFilters.h"
#include "Common/YQYR/YQYRCalculator.h"
#include "Common/YQYR/YQYRTypes.h"
#include "Util/FlatSet.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace tse
{
class ShoppingTrx;
class NGSYQYRCalculatorTest;

namespace YQYR
{
struct OldCalcAdapter
{
  OldCalcAdapter(ShoppingTrx& trx,
                 FarePath* farePath,
                 const YQYRCalculator::FareGeography& geography,
                 const Validations validations,
                 YQYRCalculator::YQYRFeesApplicationVec& results,
                 const std::vector<CarrierCode>& concurringCarriers,
                 DiagCollectorShopping* dc);

  void matchFees(const CarrierCode carrier, const std::vector<const YQYRFees*>& fees);

  DiagCollectorShopping* getDc() const { return _dc; }

  void applyFees(const CarrierCode carrier)
  {
    _yqyrCalc.findMatchingPathsShopping(_trx, carrier, *_fare, _results);
  }

private:
  ShoppingTrx& _trx;
  const PaxTypeFare* _fare;
  FarePath* _farePath;
  YQYRCalculator::YQYRFeesApplicationVec& _results;
  DiagCollectorShopping* _dc;

  YQYRCalculator _yqyrCalc;
};

class ShoppingYQYRCalculator
{
  friend class ::tse::NGSYQYRCalculatorTest;

  struct ConcurContext
  {
    const YQYRFeesNonConcur* concurRecord = nullptr;
    const TaxCarrierAppl* taxCarrierAppl = nullptr;
  };

public:
  ShoppingYQYRCalculator(ShoppingTrx& trx);

  bool isFilteredOutByCarrierDiag(const CarrierCode validatingCarrier) const;

  StdVectorFlatSet<CarrierCode>
  determineCarriersToProcess(const Itin* itin, const CarrierCode validatingCarrier);

protected:
  void appendFilter(YQYRFilter* filter);
  void prependFilter(YQYRFilter* filter);
  void setClassifier(YQYRClassifier* classifier);
  void initDefaultFilters();

  const CarrierStorage* getCarrierStorage(const CarrierCode carrier);
  const CarrierStorage&
  getCarrierStorage(FareMarket* fareMarket, PaxTypeBucket& cortege, const CarrierCode carrier);

  bool calculateYQYRsOnBucket(const uint32_t bucketIndex,
                              const CarrierCode validatingCarrier,
                              const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                              FareMarket* fareMarket,
                              PaxTypeBucket& cortege,
                              FarePath* farePath,
                              const YQYRCalculator::FareGeography& geography,
                              YQYRCalculator::YQYRFeesApplicationVec& results);

private:
  void updateValidations(const Validations::Validation validation);
  void readRecords(const CarrierCode carrier);
  ConcurContext getConcur(const CarrierCode carrier);

protected:
  ShoppingTrx& _trx;
  std::unique_ptr<DiagCollectorShopping> _dc;
  const Loc* _journeyOrigin;
  std::vector<YQYRBucket> _originalBuckets;

private:
  YQYRFilters _filters;
  YQYRClassifier* _classifier = nullptr;
  Validations _validations;

  typedef std::unordered_map<CarrierCode, CarrierStorage> FeesPerCarrierMap;
  FeesPerCarrierMap _feesPerCarrier;

  std::unordered_map<CarrierCode, ConcurContext> _concurringCarriers;
};

}
}
