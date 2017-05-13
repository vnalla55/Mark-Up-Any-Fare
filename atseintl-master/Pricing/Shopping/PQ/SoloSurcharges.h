// ----------------------------------------------------------------
//
//   Copyright Sabre 2010
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/YQYR/NGSYQYRCalculator.h"

#include <map>
#include <memory>
#include <tuple>
#include <vector>

namespace tse
{
class DiagCollector;
class FareMarket;
class FarePath;
class FarePathWrapper;
class GroupFarePath;
class PaxTypeFare;
class PricingUnit;
class ScopedFMGlobalDirSetter;
class ShoppingTrx;
class TaxResponse;
struct SOPUsage;

typedef std::vector<int> SOPVec;
typedef std::tuple<SOPVec, GroupFarePath*, SOPVec> SolutionTuple;
typedef std::multimap<MoneyAmount, SolutionTuple> SortedFlightsMap;
typedef std::vector<PaxTypeFare*> PaxTypeFareVec;

class SoloSurcharges
{
  static constexpr MoneyAmount INVALID_AMOUNT = -1;

public:
  struct SurchargesDetail
  {
    MoneyAmount _surchargesAmount = 0;
    MoneyAmount _taxAmount = 0;
    MoneyAmount _orgNucFareAmount = 0;
    CarrierCode _validatingCarrier;
    FarePath* _farePath = nullptr;
    PaxTypeFare* _paxTypeFare = nullptr;
  };

  using SurchargesDetails = std::vector<SurchargesDetail>;
  using SurchargesDetailsMap = std::map<const FareMarket*, SurchargesDetails>;

  explicit SoloSurcharges(ShoppingTrx& trx);
  SoloSurcharges(const SoloSurcharges&) = delete;
  void operator=(const SoloSurcharges&) = delete;

  virtual ~SoloSurcharges() { flushTaxDiagnosticsMsg(); }

  bool isEnabled() const { return _enabled; }
  const SurchargesDetailsMap& surchargesDetailsMap() const;

  void process();
  void restoreTotalNUCAmount();
  void restoreTotalNUCAmount(GroupFarePath* gfp);
  void calculateTaxesForShortCutPricing(const SortedFlightsMap& flightMap);

protected:
  virtual PaxTypeFareVec getApplicableFares(FareMarket& fareMarket);
  virtual bool isValid(PaxTypeFare* paxTypeFare);
  virtual bool preparePTFForValidation(const uint32_t carrierKey, PaxTypeFare& ptf);

  virtual TaxResponse* calculateYQYR(const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                                     FarePathWrapper& farePathWrapper,
                                     FareMarket* fareMarket,
                                     const PaxTypeFare* paxTypeFare,
                                     const uint16_t sopId);
  virtual MoneyAmount
  getYQYR(FarePath& farePath, PaxTypeFare& paxTypeFare, TaxResponse& taxResponse);
  virtual bool calculateSurcharges(FarePath* farePath, PricingUnit* pricingUnit);
  virtual MoneyAmount getSurcharges(FarePath* farePath);
  virtual ScopedFMGlobalDirSetter*
  createScopedFMGlobalDirSetter(FareMarket* fareMarket, const SOPUsage& sopUsage);

private:
  void initTaxDiagnostics();
  void flushTaxDiagnosticsMsg();
  void collectTaxDiagnostics(TaxResponse& taxResponse);

  bool process(FareMarket* fareMarket,
               SOPUsage& sopUsage,
               SurchargesDetail& surchargesDetail,
               uint32_t bitIndex,
               const PaxTypeFareVec& applicableFares,
               bool withTaxes);
  bool process(FareMarket* fareMarket,
               PaxTypeFare* paxTypeFare,
               SurchargesDetail& surchargesDetail,
               const PaxTypeFareVec& applicableFares,
               bool withTaxes);
  bool
  updateAllFares(FareMarket& fareMarket, SurchargesDetails& details, SurchargesDetail& reprDetail);
  GlobalDirection* getGlobalDirection(FareMarket* fareMarket, uint32_t origSopId) const;

  TaxResponse* buildTaxResponse(FarePath& farePath);
  MoneyAmount getTax(FarePath* farePath);

protected:
  ShoppingTrx& _trx;
  bool _enabled = false;
  bool _surchargesDetailsEnabled = false;
  size_t _numFaresForCAT12Calculation = 0;
  size_t _numApplicableFares = 0;
  size_t _numCategoriesForYQYRCalculation = 0;
  size_t _numFareClassCatUsed = 0;
  YQYR::NGSYQYRCalculator _yqyrCalculator;
  SurchargesDetailsMap _surchargesDetailsMap;
  DiagCollector* _dc = nullptr;
};
}
