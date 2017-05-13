/*
 * YQYCalculator.cpp
 *
 *  Created on: Feb 25, 2013
 *      Author: SG0892420
 */

#include "Common/YQYR/YQYRCalculator.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/GlobalManager.h"
#include "Common/MemoryUsage.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/YQYR/YQYRCalculatorForREX.h"
#include "Common/YQYR/YQYRUtils.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MileageSubstitution.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "DBAccess/TaxCarrierAppl.h"
#include "DBAccess/TaxCarrierFlightInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/MergedFareMarket.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <mutex>
#include <set>
#include <string>

namespace tse
{

FALLBACK_DECL(yqyrRexFix);
FALLBACK_DECL(unifyMemoryAborter)
FALLBACK_DECL(yqyrGetYQYRPreCalcLessLock)

struct FeePercentageCalculator
{
  FeePercentageCalculator(const PricingTrx& trx, bool useInternationalRounding)
    : _trx(trx), _useInternationalRounding(useInternationalRounding)
  {
  }

  virtual ~FeePercentageCalculator() = default;
  virtual MoneyAmount calculatePercentageFee(Percent pct) const = 0;

protected:
  template <class CurrencySource>
  MoneyAmount calculatePercentageFee(Percent pct, const CurrencySource& currencySource) const
  {
    MoneyAmount moneyAmount = currencySource.getTotalNUCAmount();
    CurrencyConversionFacade ccFacade;

    if (currencySource.calculationCurrency() != currencySource.baseFareCurrency())
    {
      Money targetMoney(currencySource.baseFareCurrency());
      Money sourceMoney(moneyAmount, currencySource.calculationCurrency());
      ccFacade.convert(targetMoney, sourceMoney, _trx, _useInternationalRounding);
      moneyAmount = targetMoney.value();
    }

    CurrencyCode paymentCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
    if (!_trx.getOptions()->currencyOverride().empty())
    {
      paymentCurrency = _trx.getOptions()->currencyOverride();
    }
    Money targetMoney(paymentCurrency);

    if (currencySource.baseFareCurrency() != paymentCurrency)
    {
      Money sourceMoney(moneyAmount, currencySource.baseFareCurrency());
      ccFacade.convert(targetMoney, sourceMoney, _trx, false);
      moneyAmount = targetMoney.value();
    }

    return moneyAmount * pct;
  }

protected:
  const PricingTrx& _trx;
  bool _useInternationalRounding;
};

struct FeePercentageCalculatorFP : public FeePercentageCalculator
{
  FeePercentageCalculatorFP(PricingTrx& trx,
                            bool useInternationalRounding,
                            const FarePath* farePath)
    : FeePercentageCalculator(trx, useInternationalRounding), _farePath(farePath)
  {
  }

  MoneyAmount calculatePercentageFee(Percent pct) const override
  {
    return FeePercentageCalculator::calculatePercentageFee(pct, *_farePath);
  }

private:
  const FarePath* _farePath;
};

struct FeePercentageCalculatorShopping : public FeePercentageCalculator
{
  FeePercentageCalculatorShopping(ShoppingTrx& trx,
                                  bool useInternationalRounding,
                                  const PaxTypeFare& fare,
                                  const Itin& itin)
    : FeePercentageCalculator(trx, useInternationalRounding),
      _moneyAmount(fare.nucFareAmount()),
      _calculationCurrency(itin.calculationCurrency()),
      _baseFareCurrency(itin.originationCurrency())
  {
    if (UNLIKELY(_calculationCurrency.empty()))
      _calculationCurrency = trx.journeyItin()->calculationCurrency();
    if (UNLIKELY(_baseFareCurrency.empty()))
      _baseFareCurrency = trx.journeyItin()->originationCurrency();
  }

  MoneyAmount calculatePercentageFee(Percent pct) const override
  {
    return FeePercentageCalculator::calculatePercentageFee(pct, *this);
  }

  MoneyAmount getTotalNUCAmount() const { return _moneyAmount; }

  CurrencyCode calculationCurrency() const { return _calculationCurrency; }

  CurrencyCode baseFareCurrency() const { return _baseFareCurrency; }

private:
  MoneyAmount _moneyAmount;
  CurrencyCode _calculationCurrency;
  CurrencyCode _baseFareCurrency;
};

YQYRCalculator::FBCMatchMapKey::FBCMatchMapKey(const std::string& s1, const std::string& s2)
{
  std::fill(ibuf, ibuf + intBufSize, 0);
  const size_t s1len = s1.length();
  std::memcpy(cbuf, s1.c_str(), s1len);
  cbuf[s1len] = '|';
  std::memcpy(cbuf + s1len + 1, s2.c_str(), s2.length());
}

YQYRCalculator::YQYRCalculator(PricingTrx& trx,
                               Itin& it,
                               const FareMarketPath* fmp,
                               const PaxType* paxType)
  : _hasSomeSlashesInFBCs(it.travelSeg().size(), false),
    _noSlash(false),
    _trx(trx),
    _itin(it),
    _itinCalculationCurrency(it.calculationCurrency()),
    _furthestSeg(it.travelSeg().back()),
    _furthestLoc(_furthestSeg->destination()),
    _oneFPMode(false),
    _returnsToOrigin(false),
    _isOnlineItin(false),
    _direction(Directions::OUTBOUND),
    _paxType(paxType),
    _fareMarketPath(fmp),
    _diagCollection(false),
    _reuseCFCBucket(true),
    _preCalcFailed(false),
    _yqyrApplCountLimit(TrxUtil::yqyrPrecalcLimit(trx)),
    _lastMemGrowthCheck(_yqyrApplCountLimit),
    _travelDate(_trx.adjustedTravelDate(ItinUtil::getTravelDate(_itin.travelSeg())))
{
  initMFMs();
  analyzeItin();
  _feesByCode.reserve(4);
  initFBCMap();
}

YQYRCalculator::YQYRCalculator(PricingTrx& trx, Itin& it, const FarePath* fp, bool enableDiag)
  : _hasSomeSlashesInFBCs(it.travelSeg().size(), false),
    _trx(trx),
    _itin(it),
    _itinCalculationCurrency(it.calculationCurrency()),
    _furthestSeg(it.travelSeg().back()),
    _furthestLoc(_furthestSeg->destination()),
    _oneFPMode(true),
    _returnsToOrigin(false),
    _isOnlineItin(false),
    _direction(Directions::OUTBOUND),
    _paxType(fp->paxType()),
    _diagCollection(enableDiag),
    _reuseCFCBucket(true),
    _preCalcFailed(false),
    _yqyrApplCountLimit(0),
    _lastMemGrowthCheck(0),
    _travelDate(_trx.adjustedTravelDate(ItinUtil::getTravelDate(_itin.travelSeg())))
{
  analyzeItin(fp);
  _feesByCode.reserve(4);
  initFBCMap(fp);

  printDiagnostic("TURNAROUND POINT:", _furthestSeg);
}

YQYRCalculator::YQYRCalculator(PricingTrx& trx, Itin& it)
  : _trx(trx),
    _itin(it),
    _itinCalculationCurrency(it.calculationCurrency()),
    _furthestSeg(it.travelSeg().back()),
    _furthestLoc(_furthestSeg->destination()),
    _oneFPMode(false),
    _returnsToOrigin(false),
    _isOnlineItin(false),
    _direction(Directions::OUTBOUND),
    _diagCollection(false),
    _reuseCFCBucket(true),
    _preCalcFailed(false),
    _yqyrApplCountLimit(TrxUtil::yqyrPrecalcLimit(trx)),
    _lastMemGrowthCheck(_yqyrApplCountLimit),
    _travelDate(_trx.adjustedTravelDate(ItinUtil::getTravelDate(_itin.travelSeg())))
{
}

YQYRCalculator::YQYRCalculator(ShoppingTrx& trx,
                               FarePath* farePath,
                               const FareGeography& geography,
                               const YQYR::Validations validations,
                               const CurrencyCode calculationCurrency,
                               const std::vector<CarrierCode>& concurringCarriers,
                               bool enableDiagnostics)
  : _hasSomeSlashesInFBCs(farePath->itin()->travelSeg().size(), false),
    _trx(trx),
    _itin(*farePath->itin()),
    _itinCalculationCurrency(calculationCurrency),
    _furthestSeg(geography.furthestSeg),
    _furthestLoc(geography.furthestLoc),
    _validations(validations),
    _oneFPMode(true),
    _returnsToOrigin(geography.returnsToOrigin),
    _isOnlineItin(geography.isOnline),
    _direction(geography.direction),
    _paxType(farePath->paxType()),
    _diagCollection(enableDiagnostics),
    _reuseCFCBucket(true),
    _preCalcFailed(false),
    _yqyrApplCountLimit(0),
    _lastMemGrowthCheck(0),
    _travelDate(_trx.adjustedTravelDate(ItinUtil::getTravelDate(farePath->itin()->travelSeg())))
{
  const size_t segmentCount(farePath->itin()->travelSeg().size());
  _fbcBySegNo.rehash(segmentCount);

  std::vector<BookingCode>(segmentCount, BookingCode("-")).swap(_rbdBySegNo);

  const PaxTypeFare* fare = farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare();
  std::string fbc(fare->createFareBasis(_trx, fare->carrier() == CARRIER_WS));

  for (const auto pricingUnit : farePath->pricingUnit())
  {
    for (const auto fareUsage : pricingUnit->fareUsage())
    {
      initFBCMap(fareUsage->travelSeg().begin(),
                 fareUsage->travelSeg().end(),
                 fareUsage->segmentStatus(),
                 fbc);
    }
  }
  _noSlash = std::find(_hasSomeSlashesInFBCs.begin(), _hasSomeSlashesInFBCs.end(), true) ==
             _hasSomeSlashesInFBCs.end();
  _fbcMatchMap.rehash(_fbcBySegNo.size() * 16);

  const CarrierCode &validatingCarrier(farePath->itin()->validatingCarrier());
  _valCxrMap[validatingCarrier] = _valCxrContext =
      &_trx.dataHandle().safe_create<ValCxrContext>(validatingCarrier);
  _valCxrContext->_concurringCarriers = concurringCarriers;

  if (_furthestSeg)
    printDiagnostic("TURNAROUND POINT:", _furthestSeg);
  else
    printDiagnostic("TURNAROUND POINT:", _furthestLoc);
}

YQYRCalculator::YQYRApplication::YQYRApplication(
    const YQYRFees* y, int firstP, int lastP, PricingTrx& trx, const CurrencyCode calcCur)
  : first(firstP), last(lastP), amount(0.0), yqyr(y)
{
  if (y->percent() > 0 || y->feeAmount() == 0.0)
    return;

  CurrencyConversionFacade ccFacade;
  Money sourceMoney(y->feeAmount(), y->cur());
  if (y->connectExemptInd() != 'X' && firstP != lastP && y->feeApplInd() == BLANK)
    sourceMoney.value() = sourceMoney.value() * (lastP - firstP + 1);

  CurrencyCode paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (LIKELY(!trx.getOptions()->currencyOverride().empty()))
  {
    paymentCurrency = trx.getOptions()->currencyOverride();
  }
  Money targetMoney(paymentCurrency);
  ccFacade.convert(targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::TAXES, false);

  targetMoney =
      ServiceFeeUtil(trx).convertMoney(sourceMoney.value(), sourceMoney.code(), paymentCurrency);

  amount = targetMoney.value();
};

namespace
{
template <class SegList, class Map>
struct FBCInserter
{
  FBCInserter(const SegList& segNumbers, Map& m, PricingTrx& trx)
    : _segs(segNumbers), _m(m), _trx(trx), _slashFound(false)
  {
  }
  void operator()(const PaxTypeFare* ptf)
  {
    std::string fbc(ptf->createFareBasis(_trx, ptf->carrier() == CARRIER_WS));
    if (UNLIKELY(_trx.getRequest()->isSpecifiedTktDesignatorEntry()))
    {
      std::string fareBasis = fbc;
      TktDesignator tktDesignatorOverride;
      tktDesignatorOverride = _trx.getRequest()->specifiedTktDesignator(
          _trx.itin().front()->segmentOrder(ptf->fareMarket()->travelSeg().front()));

      if (!tktDesignatorOverride.empty())
      {
        std::string::size_type pos = fareBasis.find("/");
        if (pos != std::string::npos)
        {
          fareBasis.erase(pos);
        }
        fareBasis += ("/" + tktDesignatorOverride).c_str();
      }
      fbc = fareBasis;
    }
    if (!_slashFound)
      _slashFound = fbc.find('/') != std::string::npos;

    for (const auto& s : _segs)
      _m.insert(typename Map::value_type(s - 1, fbc));
  }
  const SegList& _segs;
  Map& _m;
  PricingTrx& _trx;
  bool _slashFound;
};
template <class SegList, class Map>
FBCInserter<SegList, Map>
makeInserter(const SegList& sl, Map& m, PricingTrx& trx)
{
  return FBCInserter<SegList, Map>(sl, m, trx);
}
}

void
YQYRCalculator::initMFMs()
{
  if (_fareMarketPath)
  {
    getMFMs(_fareMarketPath, _allMFMs);
  }
  else
  {
    for (const auto fareMarketPath : _itin.fmpMatrix()->fareMarketPathMatrix())
      getMFMs(fareMarketPath, _allMFMs);
    std::sort(_allMFMs.begin(), _allMFMs.end());
    std::vector<MergedFareMarket*>::iterator end = std::unique(_allMFMs.begin(), _allMFMs.end());
    _allMFMs.erase(end, _allMFMs.end());
  }
}

void
YQYRCalculator::initFBCMap(const FarePath* farePath)
{
  _fbcBySegNo.rehash(_itin.travelSeg().size());

  if (farePath)
  {
    // This piece executes only when YQYRCalculator processes one FP, i.e. when Total Pricing
    // is disabled
    std::vector<BookingCode>(_itin.travelSeg().size(), BookingCode("-")).swap(_rbdBySegNo);
    for (const auto pricingUnit : farePath->pricingUnit())
      for (const auto fareUsage : pricingUnit->fareUsage())
      {
        const TravelSeg* firstTS = fareUsage->travelSeg().front();
        std::string fbc =
            dynamic_cast<TaxTrx*>(&_trx) == nullptr
                ? fareUsage->paxTypeFare()->createFareBasis(_trx,
                                                            fareUsage->paxTypeFare()->carrier() ==
                                                                CARRIER_WS)
                : firstTS->fareBasisCode(); // Tax OTA stores fare basis code only there.

        initFBCMap(fareUsage->travelSeg().begin(),
                   fareUsage->travelSeg().end(),
                   fareUsage->segmentStatus(),
                   fbc);
      }
  }
  else
    for (const auto mergedFarePath : _allMFMs)
      initFBCMapForMFM(mergedFarePath);

  _noSlash = std::find(_hasSomeSlashesInFBCs.begin(), _hasSomeSlashesInFBCs.end(), true) ==
             _hasSomeSlashesInFBCs.end();
  _fbcMatchMap.rehash(_fbcBySegNo.size() * 16);
}

void
YQYRCalculator::initFBCMap(const std::vector<TravelSeg*>::const_iterator travelSegsBegin,
                           const std::vector<TravelSeg*>::const_iterator travelSegsEnd,
                           const PaxTypeFare::SegmentStatusVec& segmentStatus,
                           std::string& fbc)
{
  if (UNLIKELY(_trx.getRequest()->isSpecifiedTktDesignatorEntry()))
  {
    std::string fareBasis = fbc;
    TktDesignator tktDesignatorOverride;
    tktDesignatorOverride =
        _trx.getRequest()->specifiedTktDesignator(_itin.segmentOrder(*travelSegsBegin));

    if (!tktDesignatorOverride.empty())
    {
      std::string::size_type pos = fareBasis.find("/");
      if (pos != std::string::npos)
      {
        fareBasis.erase(pos);
      }
      fareBasis += ("/" + tktDesignatorOverride).c_str();
    }
    fbc = fareBasis;
  }

  size_t idx = 0;
  for (std::vector<TravelSeg*>::const_iterator segIt = travelSegsBegin; segIt != travelSegsEnd;
       ++segIt, ++idx)
  {
    int segNum = _itin.segmentOrder(*segIt) - 1;
    if (UNLIKELY(segNum < 0))
      continue;
    _fbcBySegNo.insert(std::make_pair(segNum, fbc));
    _hasSomeSlashesInFBCs[segNum] = fbc.find('/') != std::string::npos;
    BookingCode bookingCode = (*segIt)->getBookingCode();
    PaxTypeFare::SegmentStatus ss;
    if (idx < segmentStatus.size())
      ss = segmentStatus[idx];
    if (!ss._bkgCodeReBook.empty() && ss._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      bookingCode = ss._bkgCodeReBook;
    _rbdBySegNo[segNum] = bookingCode;
  }
}

void
YQYRCalculator::initFBCMapForMFM(const MergedFareMarket* mfm)
{
  for (const auto fareMarket : mfm->mergedFareMarket())
  {
    std::vector<int> segNums;
    segNums.reserve(fareMarket->travelSeg().size());
    std::transform(fareMarket->travelSeg().cbegin(),
                   fareMarket->travelSeg().cend(),
                   std::back_inserter(segNums),
                   [this](const TravelSeg* ts)
                   { return _itin.segmentOrder(ts); });

    const PaxTypeBucket* ctg = fareMarket->paxTypeCortege(_paxType);
    if (UNLIKELY(!ctg))
      continue;
    if (std::for_each(ctg->paxTypeFare().begin(),
                      ctg->paxTypeFare().end(),
                      makeInserter(segNums, _fbcBySegNo, _trx))._slashFound)
      for (const auto i : segNums)
        _hasSomeSlashesInFBCs[i - 1] = true;
  }
}

void
YQYRCalculator::getMFMs(const FareMarketPath* fmp, std::vector<MergedFareMarket*>& mfm)
{
  mfm.insert(mfm.end(), fmp->fareMarketPath().begin(), fmp->fareMarketPath().end());
  for (const auto& i : fmp->sideTrips())
    for (const auto fmp : i.second)
      mfm.insert(mfm.end(), fmp->fareMarketPath().begin(), fmp->fareMarketPath().end());
}

namespace
{
struct SameTaxcode : public std::binary_function<YQYRFees*, YQYRFees*, bool>
{
  bool operator()(const YQYRFees* s1, const YQYRFees* s2) const
  {
    return s1->taxCode() == s2->taxCode() && s1->subCode() == s2->subCode();
  }
};
}

bool
YQYRCalculator::initializeYQYRList(const CarrierCode cxr)
{
  _feesByCode.clear();
  const std::vector<YQYRFees*>& yqyrs = _trx.dataHandle().getYQYRFees(cxr);

  if (yqyrs.empty())
    return false;

  std::vector<YQYRFees*>::const_iterator first(yqyrs.begin()), last;
  do
  {
    last = std::upper_bound(first, yqyrs.end(), *first, std::not2(SameTaxcode()));
    _feesByCode.push_back(allocateFeeList());
    std::copy(first, last, std::back_inserter(_feesByCode.back()));
    first = last;
  } while (last != yqyrs.end());
  return true;
}

bool
YQYRCalculator::getItinValidatingCarriers(std::vector<CarrierCode>& result) const
{
  if (!_oneFPMode)
    _itin.getValidatingCarriers(_trx, result);
  if (UNLIKELY(_itin.validatingCarrier().empty() && result.empty()))
    return false;

  if (result.empty() && !_itin.validatingCarrier().empty())
    result.push_back(_itin.validatingCarrier());

  return true;
}

void
YQYRCalculator::determineCarriers()
{
  std::vector<CarrierCode> allValCxrs;
  if (UNLIKELY(!getItinValidatingCarriers(allValCxrs)))
    return;

  std::set<CarrierCode> carriersInItin;
  for (const auto travelSeg : _itin.travelSeg())
  {
    const AirSeg* airSeg = travelSeg->toAirSeg();
    if (!airSeg)
      continue;
    carriersInItin.insert(airSeg->marketingCarrierCode());
  }

  initializeCarrierContext(allValCxrs, carriersInItin);

  // Want to preserve the order of carriers in _carriers as it was before the change for Validating
  // Cxr and GSA.
  // Insignificant from the business logic point of view but will make QA life easier.
  // This is the reason _carriers is not a set.
  std::set<CarrierCode> uniqueCxrs;
  for (const auto& vc : _valCxrMap)
    for (const auto& cxr : vc.second->_concurringCarriers)
      if (uniqueCxrs.insert(cxr).second)
        _carriers.push_back(cxr);

  std::vector<CarrierCode>::iterator defValCxrIt =
      std::find(_carriers.begin(), _carriers.end(), _itin.validatingCarrier());
  if (defValCxrIt != _carriers.end() && defValCxrIt != _carriers.begin())
    std::iter_swap(defValCxrIt, _carriers.begin());
}

void
YQYRCalculator::initializeCarrierContext(const std::vector<CarrierCode>& allValCxrs,
                                         const std::set<CarrierCode>& carriersInItin)
{
  for (const auto& validatingCxr : allValCxrs)
  {
    _valCxrMap[validatingCxr] = _valCxrContext =
        &_trx.dataHandle().safe_create<ValCxrContext>(validatingCxr);

    const std::vector<YQYRFeesNonConcur*>& yqyrFeesNC =
        _trx.dataHandle().getYQYRFeesNonConcur(validatingCxr, _trx.getRequest()->ticketingDT());
    if (yqyrFeesNC.empty())
    {
      if (carriersInItin.find(validatingCxr) != carriersInItin.end())
        _valCxrContext->_concurringCarriers.push_back(validatingCxr);
      continue;
    }

    YQYRFeesNonConcur* yqyrFeesNonConcur = yqyrFeesNC.front();

    if (yqyrFeesNonConcur->selfAppl() != 'X' &&
        carriersInItin.find(validatingCxr) != carriersInItin.end())
      _valCxrContext->_concurringCarriers.push_back(validatingCxr);

    const TaxCarrierAppl* taxCarrierAppl =
        (yqyrFeesNonConcur->taxCarrierApplTblItemNo())
            ? _trx.dataHandle().getTaxCarrierAppl(yqyrFeesNonConcur->vendor(),
                                                  yqyrFeesNonConcur->taxCarrierApplTblItemNo())
            : nullptr;

    std::set<CarrierCode> processedCarriers;
    processedCarriers.insert(validatingCxr);

    for (const auto travelSeg : _itin.travelSeg())
    {
      const AirSeg* airSeg = travelSeg->toAirSeg();

      if (!airSeg)
        continue;

      const CarrierCode cxr = airSeg->marketingCarrierCode();

      if (!processedCarriers.insert(cxr).second)
        continue;

      if (!validateT190(taxCarrierAppl, cxr))
        continue;

      _valCxrContext->_concurringCarriers.push_back(airSeg->marketingCarrierCode());
    }
  }
}

bool
YQYRCalculator::validateT190(const TaxCarrierAppl* t190, const CarrierCode cxr)
{
  if (!t190)
    return true;

  for (const auto taxCxrApplSeg : t190->segs())
  {
    if (taxCxrApplSeg->carrier() == DOLLAR_CARRIER)
      return true;

    if (taxCxrApplSeg->carrier() != cxr)
      continue;

    if (taxCxrApplSeg->applInd() == 'X')
      return false;

    return true;
  }

  return false;
}

void
YQYRCalculator::analyzeItin(const FarePath* fp)
{
  const std::vector<TravelSeg*>& travelSegs = _itin.travelSeg();
  const Loc* origin = travelSegs.front()->origin();

  std::set<TravelSeg*> fareBreaks;
  getFareBreaks(fp, fareBreaks);

  std::vector<TravelSeg*> gdSegs;
  std::vector<GlobalDirection> gds;

  bool isInternational = false;
  const NationCode& origNation = origin->nation();

  std::map<int, TravelSeg*> segsByDistance;
  std::set<CarrierCode> cxrs;

  for (SegmentItType curSegIt = travelSegs.begin(); curSegIt < travelSegs.end(); ++curSegIt)
  {
    TravelSeg* curSeg = *curSegIt;
    SegmentItType nextSegIt = curSegIt + 1;

    gdSegs.push_back(curSeg);

    if (curSeg->destination()->nation() != origNation)
      isInternational = true;

    GlobalDirection gd;
    GlobalDirectionFinderV2Adapter::getGlobalDirection(&_trx, _itin.travelDate(), gdSegs, gd);

    gds.push_back(gd);

    if (curSeg->isAir())
      cxrs.insert(static_cast<AirSeg*>(curSeg)->carrier());

    if ((fareBreaks.find(curSeg) != fareBreaks.end()) || // is a fare break
        (curSeg->isAir() && // is not ARUNK and is a stopover
         (curSeg->isForcedStopOver() ||
          (nextSegIt != travelSegs.end() && (*nextSegIt)->isStopOver(curSeg, SECONDS_PER_DAY)))))
    {
      printDiagnostic("DISTANCES:", gdSegs, gds, curSeg);
      const uint32_t mileage = YQYR::YQYRUtils::journeyMileage(
          gdSegs.front()->origin(), gdSegs.back()->destination(), gds, _itin.travelDate(), _trx);

      segsByDistance.insert(std::make_pair(mileage, curSeg));
    }
  }

  std::map<int, TravelSeg*>::reverse_iterator furthestIt = segsByDistance.rbegin();

  if (isInternational)
  {
    if (origNation == travelSegs.back()->destination()->nation())
      _returnsToOrigin = true;
  }
  else
  {
    if (travelSegs.front()->boardMultiCity() == travelSegs.back()->offMultiCity())
      _returnsToOrigin = true;
  }

  if (_returnsToOrigin)
  {
    _direction = Directions::BOTH;

    if (isInternational)
      while (furthestIt != segsByDistance.rend() &&
             furthestIt->second->destination()->nation() == origNation)
        ++furthestIt;

    if (furthestIt != segsByDistance.rend())
      _furthestSeg = furthestIt->second;
  }
  _isOnlineItin = cxrs.size() == 1;
  _furthestLoc = _furthestSeg->destination();
}

// The function checks if the furthest point is a stopover. If this is the case, it is the only
// possible turnaround point.
bool
YQYRCalculator::isSimpleItin(PricingTrx& trx, const Itin& itin)
{
  // Multiple brand solutions have multiple fare market paths for the same
  // pax type. For this reason, YQYRCalculator must be called multiple times for
  // the same itinerary, even in the pricing orchestrator. We cannot afford
  // to make shortcuts in these transactions.
  if (trx.getRequest()->isBrandedFaresRequest() || trx.isBRAll())
  {
    return false;
  }

  bool returnsToOrigin(false);
  bool isSimpleItin(true);

  const std::vector<TravelSeg*>& travelSegs = itin.travelSeg();
  const Loc* origin = travelSegs.front()->origin();
  std::vector<TravelSeg*> gdSegs;
  std::vector<GlobalDirection> gds;

  bool isInternational = false;
  const NationCode& origNation = origin->nation();

  std::map<int, TravelSeg*> segsByDistance;

  for (const auto curSeg : travelSegs)
  {
    gdSegs.push_back(curSeg);

    if (curSeg->destination()->nation() != origNation)
      isInternational = true;

    GlobalDirection gd;
    GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, itin.travelDate(), gdSegs, gd);
    gds.push_back(gd);

    const uint32_t mileage = YQYR::YQYRUtils::journeyMileage(gdSegs.front()->origin(),
                                                             gdSegs.back()->destination(),
                                                             gds, itin.travelDate(), trx);
    segsByDistance.insert(std::make_pair(mileage, curSeg));
  }

  std::map<int, TravelSeg*>::reverse_iterator furthestIt = segsByDistance.rbegin();

  if (isInternational)
  {
    if (origNation == travelSegs.back()->destination()->nation())
      returnsToOrigin = true;
  }
  else
  {
    if (travelSegs.front()->boardMultiCity() == travelSegs.back()->offMultiCity())
      returnsToOrigin = true;
  }

  if (!returnsToOrigin)
    return true;

  if (isInternational)
    while (furthestIt != segsByDistance.rend() &&
           furthestIt->second->destination()->nation() == origNation)
      ++furthestIt;

  if (furthestIt == segsByDistance.rend())
    return false;

  size_t furthestInd = itin.segmentOrder(furthestIt->second);
  if (furthestInd < travelSegs.size())
  {
    if ((!travelSegs[furthestInd]->isStopOver(furthestIt->second, SECONDS_PER_DAY) &&
         !furthestIt->second->isForcedStopOver()) ||
        !furthestIt->second->isAir())
      isSimpleItin = false;
  }

  return isSimpleItin;
}

void
YQYRCalculator::FareGeography::init(const std::vector<TravelSeg*>& segments, bool initFurthestSeg)
{
  if (!furthestLoc)
    initFurthestSeg = false;

  const CarrierCode* prevCarrier = nullptr;

  for (const auto segment : segments)
  {
    if (initFurthestSeg && !furthestSeg &&
        (*segment->destination() == *furthestLoc || *segment->origin() == *furthestLoc))
      furthestSeg = segment;

    const AirSeg* airSeg(segment->toAirSeg());
    if (!prevCarrier && airSeg)
      prevCarrier = &airSeg->carrier();

    if (prevCarrier && *prevCarrier != airSeg->carrier())
      isOnline = false;
  }
}

void
YQYRCalculator::process()
{
  auto processImpl = [this]
  {
    const TSELatencyData m(_trx, "YQYR PRECALCULATION");
    determineCarriers();

    if (_trx.isRexBaseTrx())
    {
      _travelDate = getProcessingDate();
    }

    try
    {
      for (const auto& cxr : _carriers)
        processCarrier(cxr);
    }
    catch (const TPMemoryOverused&)
    {
      _preCalcFailed = true;
    }
    if (!_oneFPMode && !_preCalcFailed)
      findLowerBound();

    _feesByCode.clear();
    _listMemPoolFactory.purge();
    _subSliceResults.clear();
  };

  if (fallback::yqyrGetYQYRPreCalcLessLock(&_trx))
    processImpl();
  else
    std::call_once(processOnce, processImpl);
}

YQYRCalculator::YQYRPath*
YQYRCalculator::YQYRPath::extend(const YQYRApplication& appl) const
{
  YQYRCalculator::YQYRPath* e = new YQYRCalculator::YQYRPath(*this);
  e->add(appl);
  return e;
}

void
YQYRCalculator::YQYRPath::add(const YQYRApplication& appl)
{
  if (appl.feeApplInd() == ' ' && appl.amount > 0)
    amount += appl.amount;
  yqyrs.push_back(appl);
}

void
YQYRCalculator::YQYRPath::display() const
{
  using std::cout;
  if (yqyrs.empty())
  {
    cout << this << ": Empty YQYRpath\n";
    return;
  }
  const YQYRApplication& f = yqyrs.front();
  cout << f.yqyr->carrier() << "/" << f.yqyr->taxCode() << f.yqyr->subCode() << ":";
  for (const auto& yqyrAppl : yqyrs)
    cout << this << ": [" << yqyrAppl.first + 1 << "-" << yqyrAppl.last + 1 << "]"
         << yqyrAppl.yqyr->seqNo();
  cout << "\n";
}

YQYRCalculator::MatchResult
YQYRCalculator::matchFeeRecord(const YQYRFees* yqyr, int startInd, int endInd)
{
  MatchResult r = prevalidateFbc(startInd, *yqyr);
  if (r != MatchResult::UNKNOWN)
  {
    printDiagnostic(yqyr, " FAIL - FARE BASIS PREVALIDATION\n", startInd);
    return r;
  }

  if (!validateJourney(*yqyr))
  {
    printDiagnostic(yqyr, " FAIL - JOURNEY\n");
    return MatchResult::NEVER;
  }

  if (!checkValidatingCxr(*yqyr))
  {
    printDiagnostic(yqyr, " FAIL - VALIDATING CARRIER\n");
    if (_valCxrMap.size() == 1)
      return MatchResult::NEVER;
    return (yqyr->sectorPortionOfTvlInd() == 'P') ? MatchResult::P_FAILED_NO_CHANCE
                                                  : MatchResult::S_FAILED;
  }

  if (_validations.shouldValidate(YQYR::Validations::RETURNS_TO_ORIGIN) &&
      !validateReturnToOrigin(*yqyr))
  {
    printDiagnostic(yqyr, " FAIL - RETURN TO ORIGIN\n");
    return MatchResult::NEVER;
  }

  if (_validations.shouldValidate(YQYR::Validations::PTC) && !validatePTC(*yqyr))
  {
    printDiagnostic(yqyr, " FAIL - PASSANGER TYPE\n");
    return MatchResult::NEVER;
  }

  if (_validations.shouldValidate(YQYR::Validations::POS) && !validatePOS(*yqyr))
  {
    printDiagnostic(yqyr, " FAIL - POINT OF SALE\n");
    return MatchResult::NEVER;
  }

  if (_validations.shouldValidate(YQYR::Validations::POT) && !validatePOT(*yqyr))
  {
    printDiagnostic(yqyr, " FAIL - POINT OF TICKETING\n");
    return MatchResult::NEVER;
  }

  if (_validations.shouldValidate(YQYR::Validations::AGENCY) && !validateAgency(*yqyr))
  {
    printDiagnostic(yqyr, " FAIL - AGENCY\n");
    return MatchResult::NEVER;
  }

  if (!validateTravelDate(*yqyr))
  {
    printDiagnostic(yqyr, " FAIL - TRAVEL DATE\n");
    return MatchResult::NEVER;
  }

  if (_validations.shouldValidate(YQYR::Validations::TICKETING_DATE) &&
      !validateTicketingDate(*yqyr))
  {
    printDiagnostic(yqyr, " FAIL - TICKETING DATE\n");
    return MatchResult::NEVER;
  }

  if (UNLIKELY(!validatePercent(*yqyr)))
  {
    printDiagnostic(yqyr, " FAIL - PERCENT FEE\n");
    return MatchResult::NEVER;
  }

  return (yqyr->sectorPortionOfTvlInd() == 'P') ? validatePortion(*yqyr, startInd, endInd)
                                                : validateSector(*yqyr, startInd);
}

void
YQYRCalculator::processCarrier(const CarrierCode cxr)
{
  initializeYQYRList(cxr);

  for (auto& txCode : _feesByCode)
  {
    _feeList = &txCode;
    processCarrier(cxr, *_feeList);
  } // fee type
}

// validators

bool
YQYRCalculator::validateJourney(const YQYRFees& s1Rec)
{
  // something to check?
  if (s1Rec.journeyLoc1().empty() && s1Rec.viaLoc().empty() && s1Rec.whollyWithinLoc().empty())
  {
    return true;
  }

  /////////////////////////////////
  // check loc1 and loc2
  /////////////////////////////////
  if (_validations.shouldValidate(YQYR::Validations::JOURNEY) && !s1Rec.journeyLoc1().empty())
  {
    const Loc* backLoc = _furthestLoc;
    const Loc* frontLoc = _itin.travelSeg().front()->origin();
    char dir = (s1Rec.journeyLoc1Ind() == 'A') ? '1' : ' ';
    if (!validateLocs(dir,
                      *frontLoc,
                      s1Rec.journeyLoc1(),
                      s1Rec.journeyLocType1(),
                      *backLoc,
                      s1Rec.journeyLoc2(),
                      s1Rec.journeyLocType2(),
                      s1Rec.vendor(),
                      s1Rec.carrier()))
    {
      return false;
    }
  }
  /////////////////////////////////
  // check remaining locs in itin (if needed)
  /////////////////////////////////
  if (!s1Rec.viaLoc().empty() &&
      !validateJourneyVia(_itin.travelSeg().begin(),
                          _itin.travelSeg().end(),
                          s1Rec.viaLoc(),
                          s1Rec.viaLocType(),
                          s1Rec.vendor(),
                          s1Rec.carrier()))
  {
    return false;
  }

  if (!s1Rec.whollyWithinLoc().empty() &&
      !validateWithin(_itin.travelSeg().begin(),
                      _itin.travelSeg().end(),
                      s1Rec.whollyWithinLoc(),
                      s1Rec.whollyWithinLocType(),
                      s1Rec.vendor(),
                      s1Rec.carrier()))
  {
    return false;
  }

  return true;
} // validateJourney()

bool
YQYRCalculator::validateLocs(char dir,
                             const Loc& frontLoc,
                             const LocCode& s1fLoc,
                             LocTypeCode s1fType,
                             const Loc& backLoc,
                             const LocCode& s1bLoc,
                             LocTypeCode s1bType,
                             const VendorCode& vendor,
                             const CarrierCode cxr)
{
  // note that a blank locType will always match [i.e. isInLoc() returns true]
  bool rc = false;

  const ZoneType s1fZone = (s1fType == 'Z') ? RESERVED : TAX_ZONE;
  const ZoneType s1bZone = (s1bType == 'Z') ? RESERVED : TAX_ZONE;

  if (s1fType == 'U')
    s1fType = 'Z';

  if (s1bType == 'U')
    s1bType = 'Z';

  switch (dir)

  {
  case '1':

    if (UNLIKELY(s1fLoc.empty()))
    {
      rc = LocUtil::isInLoc(backLoc,
                            s1bType,
                            s1bLoc,
                            vendor,
                            s1bZone,
                            LocUtil::SERVICE_FEE,
                            GeoTravelType::International,
                            cxr,
                            _trx.getRequest()->ticketingDT());
      break;
    }

    if (s1bLoc.empty())
    {
      rc = LocUtil::isInLoc(frontLoc,
                            s1fType,
                            s1fLoc,
                            vendor,
                            s1fZone,
                            LocUtil::SERVICE_FEE,
                            GeoTravelType::International,
                            cxr,
                            _trx.getRequest()->ticketingDT());
      break;
    }

    rc = (LocUtil::isInLoc(frontLoc,
                           s1fType,
                           s1fLoc,
                           vendor,
                           s1fZone,
                           LocUtil::SERVICE_FEE,
                           GeoTravelType::International,
                           cxr,
                           _trx.getRequest()->ticketingDT()) &&
          LocUtil::isInLoc(backLoc,
                           s1bType,
                           s1bLoc,
                           vendor,
                           s1bZone,
                           LocUtil::SERVICE_FEE,
                           GeoTravelType::International,
                           cxr,
                           _trx.getRequest()->ticketingDT()));
    break;

  case '2':

    if (s1fLoc.empty())
    {
      rc = LocUtil::isInLoc(frontLoc,
                            s1bType,
                            s1bLoc,
                            vendor,
                            s1bZone,
                            LocUtil::SERVICE_FEE,
                            GeoTravelType::International,
                            cxr,
                            _trx.getRequest()->ticketingDT());
      break;
    }

    if (s1bLoc.empty())
    {
      rc = LocUtil::isInLoc(backLoc,
                            s1fType,
                            s1fLoc,
                            vendor,
                            s1fZone,
                            LocUtil::SERVICE_FEE,
                            GeoTravelType::International,
                            cxr,
                            _trx.getRequest()->ticketingDT());
      break;
    }

    rc = (LocUtil::isInLoc(backLoc,
                           s1fType,
                           s1fLoc,
                           vendor,
                           s1fZone,
                           LocUtil::SERVICE_FEE,
                           GeoTravelType::International,
                           cxr,
                           _trx.getRequest()->ticketingDT()) &&
          LocUtil::isInLoc(frontLoc,
                           s1bType,
                           s1bLoc,
                           vendor,
                           s1bZone,
                           LocUtil::SERVICE_FEE,
                           GeoTravelType::International,
                           cxr,
                           _trx.getRequest()->ticketingDT()));
    break;

  default:
    // BETWEEN can match like FROM or TO

    if (UNLIKELY(s1fLoc.empty()))
    {
      rc = (LocUtil::isInLoc(frontLoc,
                             s1bType,
                             s1bLoc,
                             vendor,
                             s1bZone,
                             LocUtil::SERVICE_FEE,
                             GeoTravelType::International,
                             cxr,
                             _trx.getRequest()->ticketingDT()) ||
            LocUtil::isInLoc(backLoc,
                             s1bType,
                             s1bLoc,
                             vendor,
                             s1bZone,
                             LocUtil::SERVICE_FEE,
                             GeoTravelType::International,
                             cxr,
                             _trx.getRequest()->ticketingDT()));

      break;
    }

    if (s1bLoc.empty())
    {
      rc = (LocUtil::isInLoc(frontLoc,
                             s1fType,
                             s1fLoc,
                             vendor,
                             s1fZone,
                             LocUtil::SERVICE_FEE,
                             GeoTravelType::International,
                             cxr,
                             _trx.getRequest()->ticketingDT()) ||
            LocUtil::isInLoc(backLoc,
                             s1fType,
                             s1fLoc,
                             vendor,
                             s1fZone,
                             LocUtil::SERVICE_FEE,
                             GeoTravelType::International,
                             cxr,
                             _trx.getRequest()->ticketingDT()));

      break;
    }

    rc = (LocUtil::isInLoc(frontLoc,
                           s1fType,
                           s1fLoc,
                           vendor,
                           s1fZone,
                           LocUtil::SERVICE_FEE,
                           GeoTravelType::International,
                           cxr,
                           _trx.getRequest()->ticketingDT()) &&
          LocUtil::isInLoc(backLoc,
                           s1bType,
                           s1bLoc,
                           vendor,
                           s1bZone,
                           LocUtil::SERVICE_FEE,
                           GeoTravelType::International,
                           cxr,
                           _trx.getRequest()->ticketingDT()));

    if (rc)
      break;

    rc = (LocUtil::isInLoc(backLoc,
                           s1fType,
                           s1fLoc,
                           vendor,
                           s1fZone,
                           LocUtil::SERVICE_FEE,
                           GeoTravelType::International,
                           cxr,
                           _trx.getRequest()->ticketingDT()) &&
          LocUtil::isInLoc(frontLoc,
                           s1bType,
                           s1bLoc,
                           vendor,
                           s1bZone,
                           LocUtil::SERVICE_FEE,
                           GeoTravelType::International,
                           cxr,
                           _trx.getRequest()->ticketingDT()));

    break;
  } // endswitch-dir

  return rc;
}

bool
YQYRCalculator::validateWithin(SegmentItType front,
                               SegmentItType back,
                               const LocCode& wiLoc,
                               LocTypeCode wiType,
                               const VendorCode& vendor,
                               const CarrierCode cxr)
{
  const ZoneType zoneType = (wiType == 'Z') ? RESERVED : TAX_ZONE;

  if (wiType == 'U')
    wiType = 'Z';

  const AirSeg* airSeg;

  for (SegmentItType iSeg = front; iSeg != back; iSeg++)
  {
    airSeg = (*iSeg)->toAirSeg();

    if (!airSeg)
      continue;

    if (!LocUtil::isInLoc(*airSeg->origin(),
                          wiType,
                          wiLoc,
                          vendor,
                          zoneType,
                          LocUtil::SERVICE_FEE,
                          GeoTravelType::International,
                          cxr,
                          _trx.getRequest()->ticketingDT()))
      return false;

    if (!LocUtil::isInLoc(*airSeg->destination(),
                          wiType,
                          wiLoc,
                          vendor,
                          zoneType,
                          LocUtil::SERVICE_FEE,
                          GeoTravelType::International,
                          cxr,
                          _trx.getRequest()->ticketingDT()))
      return false;
  }
  return true;
}

bool
YQYRCalculator::validateJourneyVia(SegmentItType front,
                                   SegmentItType back,
                                   const LocCode& viaLoc,
                                   LocTypeCode viaType,
                                   const VendorCode& vendor,
                                   const CarrierCode cxr)
{
  // anything to check?
  if (UNLIKELY(viaLoc.empty()))
    return true;

  const ZoneType zoneType = (viaType == 'Z') ? RESERVED : TAX_ZONE;
  if (viaType == 'U')
    viaType = 'Z';

  for (--back; front != back; ++front)
  {
    const TravelSeg* seg = *front;
    if (seg == _furthestSeg)
      continue;

    if (LocUtil::isInLoc(*seg->destination(),
                         viaType,
                         viaLoc,
                         vendor,
                         zoneType,
                         LocUtil::SERVICE_FEE,
                         GeoTravelType::International,
                         cxr,
                         _trx.getRequest()->ticketingDT()))
    {
      return true; // found good via point
    } // endif - check via
  } // end for - all air segs

  return false;
}

bool
YQYRCalculator::checkValidatingCxr(const YQYRFees& yqyr)
{
  if (!yqyr.taxCarrierApplTblItemNo())
    return true;

  const TaxCarrierAppl* taxCarrierAppl =
      _trx.dataHandle().getTaxCarrierAppl(yqyr.vendor(), yqyr.taxCarrierApplTblItemNo());
  if (UNLIKELY(!taxCarrierAppl))
    return false;
  _reuseCFCBucket = false;
  return validateT190(taxCarrierAppl, _valCxrContext->_cxr);
}
bool
YQYRCalculator::validateReturnToOrigin(const YQYRFees& yqyr) const
{
  switch (yqyr.returnToOrigin())
  {
  case 'Y':
    return _returnsToOrigin;
  case 'N':
    return !_returnsToOrigin;
  case ' ':
    return true;
  default:
    break;
  }
  return false;
}

YQYRCalculator::MatchResult
YQYRCalculator::validateSector(const YQYRFees& s1Rec, int segInd)
{
  SegmentItType segIt = _itin.travelSeg().begin() + segInd;
  SegmentItType endIt = segIt + 1;

  const AirSeg* airSeg = static_cast<const AirSeg*>(_itin.travelSeg()[segInd]);

  if (!s1Rec.sectorPortionLoc1().empty() || !s1Rec.sectorPortionLoc2().empty())
  {
    if (!validateLocs(s1Rec.directionality(),
                      *airSeg->origin(),
                      s1Rec.sectorPortionLoc1(),
                      s1Rec.sectorPortionLocType1(),
                      *airSeg->destination(),
                      s1Rec.sectorPortionLoc2(),
                      s1Rec.sectorPortionLocType2(),
                      s1Rec.vendor(),
                      s1Rec.carrier()))
    {
      printDiagnostic(&s1Rec, " FAIL - SECTOR LOCATIONS\n", segInd);
      return MatchResult::S_FAILED;
    }
  }

  if (!validateIntl(segIt, endIt, s1Rec.intlDomInd()))
  {
    printDiagnostic(&s1Rec, " FAIL - INTERNATIONAL\n", segInd);
    return MatchResult::S_FAILED;
  }

  if (!validateEquipment(segIt, endIt, s1Rec.equipCode()))
  {
    printDiagnostic(&s1Rec, " FAIL - EQUIPMENT\n", segInd);
    return MatchResult::S_FAILED;
  }

  if (s1Rec.taxCarrierFltTblItemNo() != 0 &&
      !validateCarrierTable(
          segIt,
          endIt,
          _trx.dataHandle().getTaxCarrierFlight(s1Rec.vendor(), s1Rec.taxCarrierFltTblItemNo())))
  {
    printDiagnostic(&s1Rec, " FAIL - OMCARRIERS\n", segInd);
    return MatchResult::S_FAILED;
  }

  const MatchResult RBDresult = validateBookingCode(segIt, endIt, s1Rec);

  if (RBDresult == MatchResult::S_FAILED)
  {
    printDiagnostic(&s1Rec, " FAIL - BOOKING CODE\n", segInd);
    return MatchResult::S_FAILED;
  }

  const FBCMatch fbcRes = validateFbcTktDsg(segInd, s1Rec);
  switch (fbcRes)
  {
  case FBCMatch::NOWHERE:
    printDiagnostic(&s1Rec, " FAIL - FARE BASIS - NO SLASH IN ANY FBC\n", segInd);
    return MatchResult::NEVER;
  case FBCMatch::NONE:
    printDiagnostic(&s1Rec, " FAIL - FARE BASIS\n", segInd);
    return MatchResult::S_FAILED;
  case FBCMatch::SOME:
    printDiagnostic(&s1Rec, " SOFT PASS\n", segInd);
    return MatchResult::S_CONDITIONALLY;
  case FBCMatch::ALL:
    printDiagnostic(&s1Rec, " PASSED\n", segInd);
    return RBDresult;
  }

  return MatchResult::S_FAILED; // Will never get here but want to avoid a compiler warning.
}

YQYRCalculator::MatchResult
YQYRCalculator::validatePortion(const YQYRFees& s1Rec, int firstInd, int endInd)
{
  if (!s1Rec.sectorPortionViaLoc().empty())
  {
    if (endInd == firstInd)
    {
      printDiagnostic(&s1Rec, " FAILED PORTION VIA LOC\n", firstInd, endInd);
      return MatchResult::P_FAILED_NO_CHANCE;
    }
  }

  if (s1Rec.sectorPortionLoc1().empty() && s1Rec.sectorPortionLoc2().empty())
  {
    if (!s1Rec.sectorPortionViaLoc().empty() || s1Rec.stopConnectInd() != ' ')
    {
      if (endInd - firstInd != 1)
      {
        printDiagnostic(&s1Rec, " FAILED PORTION LOCVIA\n", firstInd, endInd);
        return MatchResult::P_FAILED;
      }
    }
  }

  SegmentItType front = _itin.travelSeg().begin() + firstInd;
  SegmentItType back = _itin.travelSeg().begin() + endInd;
  SegmentItType end = back + 1;

  // check endpoint locs
  if (!s1Rec.sectorPortionLoc1().empty() || !s1Rec.sectorPortionLoc2().empty())
  {
    if (!validateLocs(s1Rec.directionality(),
                      *(*front)->origin(),
                      s1Rec.sectorPortionLoc1(),
                      s1Rec.sectorPortionLocType1(),
                      *(*back)->destination(),
                      s1Rec.sectorPortionLoc2(),
                      s1Rec.sectorPortionLocType2(),
                      s1Rec.vendor(),
                      s1Rec.carrier()))
    {
      printDiagnostic(&s1Rec, " FAILED PORTION LOCS\n", firstInd, endInd);
      return MatchResult::P_FAILED;
    }
  }

  int64_t stopoverTime = SECONDS_PER_DAY; // default is 24 hours
  bool useCalendarDays = false;
  if (s1Rec.stopConnectUnit() != BLANK)
  {
    if (s1Rec.stopConnectTime() == 0)
    {
      useCalendarDays = true;
      stopoverTime = 1; // one calendar day
    }
    else
    {
      stopoverTime *= s1Rec.stopConnectTime();
      switch (s1Rec.stopConnectUnit())
      {
      // use fact that init'ed to DAYS unit
      case 'H':
        stopoverTime /= HOURS_PER_DAY;
        break;
      case 'M':
        stopoverTime *= 30;
        break; // days per month
      case 'N':
        stopoverTime /= 1440;
        break; // minutes per day
      case 'D': // intentional fall-thru; already init'ed
      default:
        break;
      }
    }
  } // endif - prep stopover vars

  // The stopover test applies to the entire portion when no via loc
  // Check both in the validateVia() loop

  if (!validateVia(front,
                   back,
                   s1Rec.sectorPortionViaLoc(),
                   s1Rec.sectorPortionViaLocType(),
                   s1Rec.vendor(),
                   s1Rec.carrier(),
                   s1Rec.stopConnectInd(),
                   stopoverTime,
                   useCalendarDays))
  {
    printDiagnostic(&s1Rec, " FAILED VIA INVALID\n", firstInd, endInd);
    return MatchResult::P_FAILED;
  }

  if (!validateIntl(front, end, s1Rec.intlDomInd()))
  {
    printDiagnostic(&s1Rec, " FAILED P INTL\n", firstInd, endInd);
    return MatchResult::P_FAILED;
  }

  if (UNLIKELY(!validateEquipment(front, end, s1Rec.equipCode())))
  {
    printDiagnostic(&s1Rec, " FAILED EQUIPMENT\n", firstInd, endInd);
    return MatchResult::P_FAILED;
  }

  if (s1Rec.taxCarrierFltTblItemNo() != 0 &&
      !validateCarrierTable(
          front,
          end,
          _trx.dataHandle().getTaxCarrierFlight(s1Rec.vendor(), s1Rec.taxCarrierFltTblItemNo())))
  {
    printDiagnostic(&s1Rec, " FAILED MO CARRIER\n", firstInd, endInd);
    return MatchResult::P_FAILED;
  }

  FBCMatch fbcRes = validateFbcTktDsg(firstInd, s1Rec);
  if (fbcRes == FBCMatch::NONE)
  {
    printDiagnostic(&s1Rec, " P FAILED FARE BASIS\n", firstInd);
    return MatchResult::P_FAILED_NO_CHANCE;
  }
  else if (UNLIKELY(fbcRes == FBCMatch::NOWHERE))
  {
    printDiagnostic(&s1Rec, " P FAILED FARE BASIS - NO SLASH IN ANY FBC\n", firstInd, endInd);
    return MatchResult::NEVER;
  }

  const MatchResult RBDresult = validateBookingCode(front, end, s1Rec);
  if (RBDresult == MatchResult::P_FAILED)
  {
    printDiagnostic(&s1Rec, " FAILED BOOKING CODE\n", firstInd, endInd);
    return MatchResult::P_FAILED;
  }

  for (++firstInd; firstInd <= endInd; ++firstInd)
  {
    switch (validateFbcTktDsg(firstInd, s1Rec))
    {
    case FBCMatch::NOWHERE:
      printDiagnostic(&s1Rec,
                      " P FAILED FARE BASIS - NO SLASH IN ANY FBC\n",
                      firstInd); // Should never get here but all values of an enum must be handled
      // in a switch to avoid warnings
      return MatchResult::NEVER;
    case FBCMatch::NONE:
      printDiagnostic(&s1Rec, " P FAILED FARE BASIS\n", firstInd);
      return MatchResult::P_FAILED;
    case FBCMatch::SOME:
      fbcRes = FBCMatch::SOME;
      break;
    case FBCMatch::ALL:
      break;
    }
  }
  printDiagnostic(&s1Rec, " P PASSED \n", firstInd, endInd);
  return std::min((fbcRes == FBCMatch::ALL) ? MatchResult::UNCONDITIONALLY
                                            : MatchResult::P_CONDITIONALLY,
                  RBDresult);
} // validatePortion()

namespace
{
struct IsInternational : public std::unary_function<const TravelSeg*, bool>
{
  bool operator()(const TravelSeg* seg) const
  {
    const NationCode& origN = seg->origin()->nation();
    const NationCode& destN = seg->destination()->nation();

    if (origN == destN)
      return false;
    if ((origN.equalToConst("XU") && destN.equalToConst("RU")) || (origN.equalToConst("RU") && destN.equalToConst("XU")))
      return false;
    return true;
  }
};
}

bool
YQYRCalculator::validateIntl(SegmentItType first, SegmentItType end, Indicator intlDomInd) const
{
  switch (intlDomInd)
  {
  case ' ':
    return true;
  case 'D':
    return std::find_if(first, end, IsInternational()) == end;
  case 'I':
    return std::find_if(first, end, std::not1(IsInternational())) == end;
  default:
    break;
  }
  return false;
}

bool
YQYRCalculator::validateEquipment(SegmentItType first, SegmentItType end, const EquipmentType& eqp)
    const
{
  if (eqp.empty())
    return true;

  for (; first != end; ++first)
    if ((*first)->equipmentType() != eqp)
      return false;

  return true;
}

bool
YQYRCalculator::validateCarrierTable(SegmentItType first,
                                     SegmentItType end,
                                     const TaxCarrierFlightInfo* t186) const
{
  if (UNLIKELY(!t186))
    return false; // Missing data or a bug in data retrieval for Historical

  const std::vector<CarrierFlightSeg*>& flightSegs = t186->segs();

  static constexpr int ANY_FLIGHT = -1;

  const AirSeg* airSeg;

  for (; first != end; ++first)
  {
    airSeg = static_cast<const AirSeg*>(*first);
    bool matchResult = false;

    for (const auto carrierFltSeg : flightSegs)
    {
      if (UNLIKELY(!(carrierFltSeg->marketingCarrier().empty()) &&
          (carrierFltSeg->marketingCarrier() != airSeg->marketingCarrierCode())))
        continue;

      if (!(carrierFltSeg->operatingCarrier().empty()) &&
          (carrierFltSeg->operatingCarrier() != airSeg->operatingCarrierCode()))
        continue;

      if (carrierFltSeg->flt1() == ANY_FLIGHT)
      {
        matchResult = true;
        break;
      }

      if (carrierFltSeg->flt2() == 0 && carrierFltSeg->flt1() == airSeg->marketingFlightNumber())
      {
        matchResult = true;
        break;
      }

      if (carrierFltSeg->flt1() > airSeg->marketingFlightNumber())
        continue;

      if (carrierFltSeg->flt2() >= airSeg->marketingFlightNumber())
      {
        matchResult = true;
        break;
      }
    }

    if (!matchResult)
      return false;
  }
  return true;
}

YQYRCalculator::MatchResult
YQYRCalculator::validateBookingCode(SegmentItType first, SegmentItType end, const YQYRFees& s1Rec)
    const
{
  if (s1Rec.bookingCode1().empty())
    return MatchResult::UNCONDITIONALLY;

  if (shouldSoftpassRBD())
    return s1Rec.sectorPortionOfTvlInd() == 'P' ? MatchResult::P_CONDITIONALLY
                                                : MatchResult::S_CONDITIONALLY;

  const MatchResult fail(s1Rec.sectorPortionOfTvlInd() == 'P' ? MatchResult::P_FAILED
                                                              : MatchResult::S_FAILED);

  bool ok = true;
  for (; first != end; ++first)
  {
    int segNum = _itin.segmentOrder(*first) - 1;

    if (UNLIKELY(segNum < 0))
      return fail;

    const BookingCode& bkg = _rbdBySegNo.empty() ? (*first)->getBookingCode() : _rbdBySegNo[segNum];

    if (bkg != s1Rec.bookingCode1() && bkg != s1Rec.bookingCode2() && bkg != s1Rec.bookingCode3())
    {
      ok = false;
      break;
    }
  }

  if (ok)
    return MatchResult::UNCONDITIONALLY;

  return fail;
}

bool
YQYRCalculator::checkLocation(const Loc& loc,
                              const LocCode& lcode,
                              LocTypeCode ltype,
                              const VendorCode& vendor,
                              const CarrierCode cxr) const
{
  const ZoneType ztype = (ltype == 'Z') ? RESERVED : TAX_ZONE;

  if (ltype == 'U')
    ltype = 'Z';

  return LocUtil::isInLoc(loc,
                          ltype,
                          lcode,
                          vendor,
                          ztype,
                          LocUtil::SERVICE_FEE,
                          GeoTravelType::International,
                          cxr,
                          _trx.getRequest()->ticketingDT());
}
YQYRCalculator::FBCMatch
YQYRCalculator::validateFbcTktDsg(int segInd, const YQYRFees& s1Rec)
{
  if (s1Rec.fareBasis().empty())
    return FBCMatch::ALL;

  bool match(false), noMatch(false);
  const char patternFirstChar = *s1Rec.fareBasis().data();
  const bool checkFirstChar(patternFirstChar != '-');

  for (std::pair<FBCbySegMapT::iterator, FBCbySegMapT::iterator> fbcs =
           _fbcBySegNo.equal_range(segInd);
       fbcs.first != fbcs.second;
       ++fbcs.first)
  {
    if (checkFirstChar && patternFirstChar != *(fbcs.first->second.data())) // A shortcut to handle
    // an obvious no-match
    {
      noMatch = true;
    }
    else if (matchFareBasis(s1Rec.fareBasis(), fbcs.first->second))
    {
      match = true;
    }
    else
    {
      noMatch = true;
    }
    if (match && noMatch)
      break;
  }

  if (!match)
    return FBCMatch::NONE;

  if (!noMatch)
    return FBCMatch::ALL;

  return FBCMatch::SOME;
}

bool
YQYRCalculator::matchFareBasis(const std::string& tktFareBasis, const std::string& ptfFareBasis)
{
  FBCMatchMapKey key(tktFareBasis, ptfFareBasis);
  const FBCMatchMap::const_iterator i = _fbcMatchMap.find(key);
  if (i != _fbcMatchMap.end())
    return i->second;
  const bool res(YQYR::YQYRUtils::matchFareBasisCode(tktFareBasis, ptfFareBasis));
  _fbcMatchMap.insert(std::make_pair(key, res));
  return res;
}

bool
YQYRCalculator::validateVia(SegmentItType front,
                            SegmentItType back,
                            const LocCode& viaLoc,
                            LocTypeCode viaType,
                            const VendorCode& vendor,
                            const CarrierCode cxr,
                            Indicator connInd,
                            int64_t stopoverTime,
                            bool useCalendarDays)
{
  if (viaLoc.empty() && connInd == BLANK)
    return true;

  bool matchingViaPointFound(false);

  for (; front != back; ++front)
  {
    if (viaLoc.empty())
      return isYQYRStopover(*front, *(front + 1), stopoverTime, useCalendarDays) ==
             (connInd == VIA_STOPOVER);

    if (checkLocation(*(*front)->destination(), viaLoc, viaType, vendor, cxr))
    {
      if (connInd == BLANK)
        return true;
      // If (StopOvr & looking for Conn) or (!StopOvr and !looking for Conn.)
      if (isYQYRStopover(*front, *(front + 1), stopoverTime, useCalendarDays) ==
          (connInd == VIA_CONNECTION))
      {
        return false;
      }
      matchingViaPointFound = true;
    }
  }

  return matchingViaPointFound;
}

void
YQYRCalculator::findMatchingPaths(const FarePath* fp,
                                  const CarrierCode valCxr,
                                  std::vector<YQYRApplication>& results)
{
  if (_preCalcFailed)
  {
    YQYRCalculator yqyrCalculator(_trx, _itin, fp);
    yqyrCalculator.process();
    return yqyrCalculator.findMatchingPaths(fp, fp->itin()->validatingCarrier(), results);
  }
  ValCxrMap::const_iterator ctxIt = _valCxrMap.find(valCxr);
  if (ctxIt == _valCxrMap.end())
    return;

  std::map<int, std::string> fbcBySeg;
  std::map<int, BookingCode> bookingCodesBySeg;
  if (!_oneFPMode)
    prepareMatchMaps(fp, fbcBySeg, bookingCodesBySeg);

  FeePercentageCalculatorFP percentageCalc(_trx, _itin.useInternationalRounding(), fp);
  findMatchingPaths(valCxr, percentageCalc, fbcBySeg, bookingCodesBySeg, results);
}

void
YQYRCalculator::findMatchingPathsShopping(ShoppingTrx& trx,
                                          const CarrierCode valCxr,
                                          const PaxTypeFare& fare,
                                          YQYRFeesApplicationVec& results)
{
  ValCxrMap::const_iterator ctxIt = _valCxrMap.find(valCxr);
  if (UNLIKELY(ctxIt == _valCxrMap.end()))
    return;

  std::map<int, std::string> fbcBySeg;
  std::map<int, BookingCode> bookingCodesBySeg;

  FeePercentageCalculatorShopping percentageCalc(
      trx, _itin.useInternationalRounding(), fare, _itin);
  findMatchingPaths(valCxr, percentageCalc, fbcBySeg, bookingCodesBySeg, results);
}

void
YQYRCalculator::findMatchingPaths(const CarrierCode valCxr,
                                  const FeePercentageCalculator& feePercentageCalc,
                                  std::map<int, std::string>& fbcBySeg,
                                  std::map<int, BookingCode>& bookingCodesBySeg,
                                  YQYRFeesApplicationVec& results)
{
  ValCxrMap::const_iterator ctxIt = _valCxrMap.find(valCxr);
  if (UNLIKELY(ctxIt == _valCxrMap.end()))
    return;

  const ValCxrContext* valCxrContext = ctxIt->second;

  bool skipOutbound = false;
  bool skipInbound = false;
  if (UNLIKELY(_trx.getRequest()->originBasedRTPricing()))
  {
    if (!_trx.outboundDepartureDate().isEmptyDate())
      skipOutbound = true;
    else
      skipInbound = true;
  }

  for (const auto feeBucket : valCxrContext->_feeBuckets)
  {
    std::vector<YQYRApplication> perOutDir;
    std::vector<YQYRApplication> perInDir;
    std::vector<YQYRApplication> perJourney;

    for (const auto feeSlice : feeBucket->feesBySlice)
    {
      const YQYRPath* matchedYP(nullptr);
      if (_oneFPMode)
      {
        if (LIKELY(!(feeSlice->yqyrpaths.empty())))
          matchedYP = feeSlice->yqyrpaths.front();
      }
      else
      {
        for (const auto yqyrPath : feeSlice->yqyrpaths)
        {
          if (matchPath(yqyrPath, fbcBySeg, bookingCodesBySeg))
          {
            matchedYP = yqyrPath;
            break;
          }
        }
      }
      if (LIKELY(matchedYP))
      {
        for (const auto& yqyrApplication : matchedYP->yqyrs)
        {
          YQYRApplication appl(yqyrApplication);
          appl.amount =
              yqyrApplication.yqyr->percent() > 0
                  ? feePercentageCalc.calculatePercentageFee(yqyrApplication.yqyr->percent())
                  : yqyrApplication.amount;
          switch (appl.yqyr->feeApplInd())
          {
          case '1':
            if (feeSlice->dir == Directions::OUTBOUND)
              perOutDir.push_back(appl);
            else
              perInDir.push_back(appl);
            break;
          case '2':
            perJourney.push_back(appl);
            break;
          case ' ':
            printDiagnostic(appl, "APPLY:NOAPPIND ");
            for (int i = 0, count = applCount(appl.yqyr, appl.first, appl.last); i < count; ++i)
              results.push_back(appl);
            break;
          default:
            break;
          }
        }
      }
    }
    if (!perOutDir.empty())
    {
      if (!skipOutbound)
      {
        YQYRApplication appl = *std::max_element(perOutDir.begin(), perOutDir.end());
        printDiagnostic(appl, "APPLY:OUTBOUND ");
        results.push_back(appl);
      }
    }

    if (!perInDir.empty())
    {
      if (!skipInbound)
      {
        YQYRApplication appl = *std::max_element(perInDir.begin(), perInDir.end());
        printDiagnostic(appl, "APPLY:INBOUND ");
        results.push_back(appl);
      }
    }

    if (!perJourney.empty())
    {
      YQYRApplication appl = *std::max_element(perJourney.begin(), perJourney.end());
      printDiagnostic(appl, "APPLY:JOURNEY ");
      results.push_back(appl);
    }
  }
}

MoneyAmount
YQYRCalculator::chargeFarePath(const FarePath& fp, const CarrierCode valCxr) const
{
  if (UNLIKELY(_preCalcFailed))
    return 0.0;

  if(_trx.isRexBaseTrx())
  {
    _travelDate = getProcessingDate();
  }

  ValCxrMap::const_iterator ctxIt = _valCxrMap.find(valCxr);
  if (ctxIt == _valCxrMap.end())
    return 0.0;

  const ValCxrContext* valCxrContext = ctxIt->second;

  if (valCxrContext->_feeBuckets.empty())
    return 0.0;

  MoneyAmount total(0.0);
  std::map<int, std::string> fbcBySeg;
  std::map<int, BookingCode> bookingCodesBySeg;
  prepareMatchMaps(&fp, fbcBySeg, bookingCodesBySeg);

  bool skipOutbound = false;
  bool skipInbound = false;

  if (UNLIKELY(_trx.getRequest()->originBasedRTPricing()))
  {
    if (!_trx.outboundDepartureDate().isEmptyDate())
      skipOutbound = true;
    else
      skipInbound = true;
  }

  FeePercentageCalculatorFP percentageCalc(_trx, _itin.useInternationalRounding(), &fp);
  for (const auto feeBucket : valCxrContext->_feeBuckets)
  {
    std::vector<MoneyAmount> perOutDir;
    std::vector<MoneyAmount> perInDir;
    std::vector<MoneyAmount> perJourney;

    for (const auto feeSlice : feeBucket->feesBySlice)
    {
      const YQYRPath* matchedYP(nullptr);
      for (const auto yqyrPath : feeSlice->yqyrpaths)
      {
        if (matchPath(yqyrPath, fbcBySeg, bookingCodesBySeg))
        {
          matchedYP = yqyrPath;
          break;
        }
      }
      if (LIKELY(matchedYP))
      {
        for (const auto& yqyrApplication : matchedYP->yqyrs)
        {
          MoneyAmount amt =
              yqyrApplication.yqyr->percent() > 0
                  ? percentageCalc.calculatePercentageFee(yqyrApplication.yqyr->percent())
                  : yqyrApplication.amount;
          switch (yqyrApplication.yqyr->feeApplInd())
          {
          case '1':
            if (feeSlice->dir == Directions::OUTBOUND)
              perOutDir.push_back(amt);
            else
              perInDir.push_back(amt);
            break;
          case '2':
            perJourney.push_back(amt);
            break;
          case ' ':
            total += amt;
            break;
          default:
            break;
          }
        }
      }
    }

    if (!perOutDir.empty())
    {
      if (LIKELY(!skipOutbound))
      {
        total += *std::max_element(perOutDir.begin(), perOutDir.end());
      }
    }

    if (!perInDir.empty())
    {
      if (LIKELY(!skipInbound))
      {
        total += *std::max_element(perInDir.begin(), perInDir.end());
      }
    }

    if (!perJourney.empty())
    {
      total += *std::max_element(perJourney.begin(), perJourney.end());
    }
  }

  CurrencyConversionFacade ccFacade;
  CurrencyCode paymentCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (LIKELY(!_trx.getOptions()->currencyOverride().empty()))
  {
    paymentCurrency = _trx.getOptions()->currencyOverride();
  }
  Money targetMoney((_itinCalculationCurrency == NUC) ? USD : _itinCalculationCurrency);
  ccFacade.convert(
      targetMoney, Money(total, paymentCurrency), _trx, false, CurrencyConversionRequest::TAXES);
  return targetMoney.value();
}

bool
YQYRCalculator::isYQYRStopover(TravelSeg* seg,
                               TravelSeg* segNext,
                               int64_t stopoverTime,
                               bool useCalendarDays)
{
  if (seg->isForcedStopOver())
    return true;

  if (seg->isForcedConx())
    return false;

  if (useCalendarDays)
  {
    DateTime fromTime = seg->arrivalDT();
    return (fromTime.addDays(uint32_t(stopoverTime)).date() <= segNext->departureDT().date());
  }

  return segNext->isStopOver(seg, stopoverTime);
}

bool
YQYRCalculator::validatePTC(const YQYRFees& yqyr) const
{
  if (yqyr.psgType().empty())
    return true;

  if (UNLIKELY(yqyr.psgType() == " "))
    return true;

  if (yqyr.psgType() == _paxType->paxType())
    return true;

  if (yqyr.psgType() == CHILD &&
      PaxTypeUtil::isATPCoPaxRollupMatch(_trx, _paxType->paxType(), yqyr.psgType()))
    return true;

  if (_paxType->paxType() != GV1)
    return false;

  const std::vector<const PaxTypeMatrix*>& sabre2ATP =
      _trx.dataHandle().getPaxTypeMatrix(_paxType->paxType());

  for (const auto paxTypeMatrix : sabre2ATP)
    if (paxTypeMatrix->atpPaxType() == yqyr.psgType())
      return true;

  return false;
}

bool
YQYRCalculator::validatePOS(const YQYRFees& yqyr) const
{
  if (yqyr.posLoc().empty())
    return true;
  return checkLocation(
      *(TrxUtil::saleLoc(_trx)), yqyr.posLoc(), yqyr.posLocType(), yqyr.vendor(), yqyr.carrier());
}

bool
YQYRCalculator::validatePOT(const YQYRFees& yqyr) const
{
  if (yqyr.potLoc().empty())
    return true;

  return checkLocation(*(TrxUtil::ticketingLoc(_trx)),
                       yqyr.potLoc(),
                       yqyr.potLocType(),
                       yqyr.vendor(),
                       yqyr.carrier());
}
bool
YQYRCalculator::validateAgency(const YQYRFees& yqyr) const
{
  switch (yqyr.posLocaleType())
  {
  case 'T':
  {
    PseudoCityCode tvlAgencyPCC = _trx.getRequest()->ticketingAgent()->tvlAgencyPCC();
    if (UNLIKELY((yqyr.posAgencyPCC().size() == 5) &&
        (!_trx.getRequest()->ticketingAgent()->officeDesignator().empty())))
    {
      tvlAgencyPCC = _trx.getRequest()->ticketingAgent()->officeDesignator();
    }

    return (tvlAgencyPCC == yqyr.posAgencyPCC());
  }
  case 'I':
  {
    const std::string& iIATA = _trx.getRequest()->ticketingAgent()->tvlAgencyIATA().empty()
                                   ? _trx.getRequest()->ticketingAgent()->airlineIATA()
                                   : _trx.getRequest()->ticketingAgent()->tvlAgencyIATA();
    if (UNLIKELY(iIATA.empty()))
      return false;
    return yqyr.posIataTvlAgencyNo().compare(0, iIATA.size(), iIATA) == 0;
  }
  case ' ':
    return true;
  default:
    return false;
  }

  return false;
}

bool
YQYRCalculator::validateTravelDate(const YQYRFees& yqyr) const
{
  if (_travelDate < yqyr.effDate())
    return false;

  if (_travelDate.date() > yqyr.discDate().date())
    return false;

  if (!_trx.dataHandle().isHistorical() && _travelDate > yqyr.expireDate())
    return false;

  if (UNLIKELY(_trx.dataHandle().isHistorical() && _trx.getRequest()->ticketingDT() > yqyr.modDate()))
  {
    if (_travelDate > yqyr.expireDate())
      return false;
  }

  return true;
}

MoneyAmount
YQYRCalculator::CarrierFeeCodeBucket::lowerBound() const
{
  MoneyAmount lb(0.0);

  bool appl1Outbound(false), appl1Inbound(false), appl2(false);

  for (const auto feeSlice : feesBySlice)
  {
    if (UNLIKELY(feeSlice->yqyrpaths.empty()))
      continue;
    MoneyAmount cheapestInSlice(std::numeric_limits<MoneyAmount>::max());
    bool appl1OutFoundInSlice(false), appl1InFoundInSlice(false), appl2FoundInSlice(false);
    for (const auto yqyrPath : feeSlice->yqyrpaths)
    {
      MoneyAmount amt(yqyrPath->amount); // This is the amount without S1 with fee appl 1/2;
      MoneyAmount maxOut1(0.0), maxIn1(0.0), max2(0.0); // Max per YQYRPath
      for (const auto& yqyrApplication : yqyrPath->yqyrs)
      {
        switch (yqyrApplication.feeApplInd())
        {
        case '1':
          if (feeSlice->dir == Directions::OUTBOUND)
          {
            appl1OutFoundInSlice = true;
            if (!appl1Outbound)
              maxOut1 = std::max(maxOut1, yqyrApplication.amount);
          }
          else //(*sl)->dir == INBOUND)
          {
            appl1InFoundInSlice = true;
            if (!appl1Inbound)
              maxIn1 = std::max(maxIn1, yqyrApplication.amount);
          }
          break;
        case '2':
          if (!appl2)
          {
            appl2FoundInSlice = true;
            max2 = std::max(max2, yqyrApplication.amount);
          }
          break;
        case ' ':
          break;
        default:
          throw std::runtime_error("Wrong value of feeApplInd");
          break;
        }
      }
      amt += maxOut1 + maxIn1 + max2;
      cheapestInSlice = std::min(cheapestInSlice, amt);
    }
    if (appl1OutFoundInSlice)
      appl1Outbound = true;
    if (appl1InFoundInSlice)
      appl1Inbound = true;
    if (appl2FoundInSlice)
      appl2 = true;
    lb += cheapestInSlice;
  }
  return lb;
}

void
YQYRCalculator::findLowerBound()
{
  if (_valCxrMap.empty())
    return;
  _lowerBound = std::numeric_limits<MoneyAmount>::max();
  for (const auto& valCxr : _valCxrMap)
  {
    _valCxrContext = valCxr.second;
    for (const auto carrierFeeCodeBucket : _valCxrContext->_feeBuckets)
      _valCxrContext->_lowerBound += carrierFeeCodeBucket->lowerBound();

    CurrencyConversionFacade ccFacade;
    CurrencyCode paymentCurrency = _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
    if (LIKELY(!_trx.getOptions()->currencyOverride().empty()))
    {
      paymentCurrency = _trx.getOptions()->currencyOverride();
    }

    Money sourceMoney(_valCxrContext->_lowerBound, paymentCurrency);
    Money targetMoney((_itinCalculationCurrency == NUC) ? USD : _itinCalculationCurrency);

    ccFacade.convert(targetMoney, sourceMoney, _trx, false, CurrencyConversionRequest::TAXES);
    _valCxrContext->_lowerBound = targetMoney.value();
    _lowerBound = std::min(_lowerBound, _valCxrContext->_lowerBound);
  }
}

void
YQYRCalculator::merge(const YQYRPath* yp,
                      const YQYRApplication& appl,
                      const std::vector<const YQYRPath*>* ypvIn,
                      std::vector<const YQYRPath*>* ypvOut)
{
  if (ypvIn == nullptr || ypvIn->empty())
  {
    YQYRPath* ypn = yp->extend(appl);
    _trx.dataHandle().deleteList().adopt(ypn);
    ypvOut->push_back(ypn);
    sanityCheck(1);
    return;
  }
  for (const auto yqyrPath : *ypvIn)
  {
    YQYRPath* ypn = yp->extend(appl);
    sanityCheck(yqyrPath->yqyrs.size() + 1);
    _trx.dataHandle().deleteList().adopt(ypn);
    for (const auto& yqyrApplication : yqyrPath->yqyrs)
      ypn->add(yqyrApplication);
    ypvOut->push_back(ypn);
  }
}

bool
YQYRCalculator::matchPath(const YQYRPath* yp,
                          const std::map<int, std::string>& fbcBySeg,
                          const std::map<int, BookingCode>& bookingCodesBySeg) const
{
  //  yp->display();
  for (const auto& yqyrApplication : yp->yqyrs)
  {
    const YQYRFees* yqyr = yqyrApplication.yqyr;
    for (int segId = yqyrApplication.first; segId <= yqyrApplication.last; ++segId)
    {
      if (!yqyr->fareBasis().empty())
      {
        const std::map<int, std::string>::const_iterator fbcIt = fbcBySeg.find(segId);
        if (UNLIKELY(fbcIt == fbcBySeg.end()))
          return false;
        const std::string ptfFareBasis = fbcIt->second;
        FBCMatchMapKey key(yqyr->fareBasis(), ptfFareBasis);
        FBCMatchMap::const_iterator i = _fbcMatchMap.find(key);
        bool res = (i != _fbcMatchMap.end())
                       ? i->second
                       : YQYR::YQYRUtils::matchFareBasisCode(yqyr->fareBasis(), ptfFareBasis);
        if (!res)
          return false;
      }

      if (!yqyr->bookingCode1().empty())
      {
        const std::map<int, BookingCode>::const_iterator rbdIt = bookingCodesBySeg.find(segId);
        if (UNLIKELY(rbdIt == bookingCodesBySeg.end()))
          return false;
        const BookingCode bkgCode = rbdIt->second;
        if (bkgCode != yqyr->bookingCode1() && bkgCode != yqyr->bookingCode2() &&
            bkgCode != yqyr->bookingCode3())
          return false;
      }
    }
  }
  return true;
}

void
YQYRCalculator::prepareMatchMaps(const FarePath* fp,
                                 std::map<int, std::string>& fbcBySeg,
                                 std::map<int, BookingCode>& bookingCodesBySeg) const
{
  for (const auto pricingUnit : fp->pricingUnit())
  {
    for (const auto fareUsage : pricingUnit->fareUsage())
    {
      std::string fbc(fareUsage->paxTypeFare()->createFareBasis(
          _trx, fareUsage->paxTypeFare()->carrier() == CARRIER_WS));
      if (UNLIKELY(_trx.getRequest()->isSpecifiedTktDesignatorEntry()))
      {
        std::string fareBasis = fbc;
        TktDesignator tktDesignatorOverride;
        tktDesignatorOverride = _trx.getRequest()->specifiedTktDesignator(
            _itin.segmentOrder(fareUsage->travelSeg().front()));

        if (!tktDesignatorOverride.empty())
        {
          std::string::size_type pos = fareBasis.find("/");
          if (pos != std::string::npos)
          {
            fareBasis.erase(pos);
          }
          fareBasis += ("/" + tktDesignatorOverride).c_str();
        }
        fbc = fareBasis;
      }

      size_t idx = 0;
      for (std::vector<TravelSeg*>::const_iterator segIt = fareUsage->travelSeg().begin();
           segIt != fareUsage->travelSeg().end();
           ++segIt, ++idx)
      {
        int segNum = _itin.segmentOrder(*segIt) - 1;
        if (UNLIKELY(segNum < 0))
          continue;
        fbcBySeg.insert(std::make_pair(segNum, fbc));
        BookingCode bookingCode = (*segIt)->getBookingCode();
        PaxTypeFare::SegmentStatus ss;
        if (LIKELY(idx < fareUsage->segmentStatus().size()))
          ss = fareUsage->segmentStatus()[idx];
        if (!ss._bkgCodeReBook.empty() && ss._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
          bookingCode = ss._bkgCodeReBook;
        bookingCodesBySeg.insert(std::make_pair(segNum, bookingCode));
      }
    }
  }
}

void
YQYRLBFactory::init(PricingTrx& trx, Itin& itin)
{
  const TSELatencyData m(trx, "YQYR LB FACTORY INIT");
  std::shared_ptr<YQYRLBFactory> f(new YQYRLBFactory(trx, itin));
  itin.yqyrLBFactory() = f;
  f->_isSimpleItin = YQYRCalculator::isSimpleItin(trx, itin);
}

YQYRCalculator*
YQYRLBFactory::getYQYRPreCalc(const FareMarketPath* fmp, const PaxType* paxType)
{
  if (UNLIKELY(areYQYRsExempted()))
    return nullptr;

  if (_isSimpleItin)
    fmp = nullptr;
  std::pair<PaxTypeCode, const FareMarketPath*> key(paxType->paxType(), fmp);

  const bool oldProcessing = fallback::yqyrGetYQYRPreCalcLessLock(&_trx);
  YQYRCalculator* yp = nullptr;
  {
    std::lock_guard<std::mutex> g(_mutex);
    const auto ypIt = _preCalcMap.find(key);
    if (ypIt != _preCalcMap.cend())
    {
      yp = ypIt->second;
    }
    else
    {
      yp = (_trx.excTrxType() != PricingTrx::AR_EXC_TRX)
               ? &_trx.dataHandle().safe_create<YQYRCalculator>(_trx, _itin, fmp, paxType)
               : &_trx.dataHandle().safe_create<YQYRCalculatorForREX>(_trx, _itin, fmp, paxType);

      if (oldProcessing)
        yp->process();

      _preCalcMap.emplace(key, yp);
    }
  }

  if (!oldProcessing)
    yp->process();

  return yp;
}

bool
YQYRLBFactory::areYQYRsExempted() const
{
  if (UNLIKELY(_trx.getRequest()->isExemptAllTaxes()))
    return true;

  if (UNLIKELY(_trx.getRequest()->isExemptSpecificTaxes()))
  {
    if (_trx.getRequest()->taxIdExempted().empty())
      return true;
    std::set<std::string> exempted(_trx.getRequest()->taxIdExempted().begin(),
                                   _trx.getRequest()->taxIdExempted().end());
    static const char* YQYRcodes[] = {"YQ", "YQF", "YQI", "YR", "YRF", "YRI"};
    std::vector<std::string> intersection;
    std::set_intersection(exempted.begin(),
                          exempted.end(),
                          YQYRcodes,
                          YQYRcodes + 6,
                          std::back_inserter(intersection));
    return !intersection.empty();
  }

  return false;
}

bool
YQYRCalculator::shouldSoftpassRBD() const
{
  if (_oneFPMode)
    return false; // Using one YQYRCalculator per fare path (Total Pricing inactive)

  return _trx.getRequest()->isLowFareRequested() || _trx.getRequest()->isLowFareNoAvailability() ||
         _trx.noPNRPricing() || RexPricingTrx::isRexTrxAndNewItin(_trx);
}

void
YQYRCalculator::getFareBreaks(const FarePath* fp, std::set<TravelSeg*>& fareBreaks) const
{
  if (fp)
  {
    for (const auto pricingUnit : fp->pricingUnit())
      for (const auto fareUsage : pricingUnit->fareUsage())
      {
        std::vector<TravelSeg*>::const_iterator firstSegIt = std::find(
            _itin.travelSeg().begin(), _itin.travelSeg().end(), fareUsage->travelSeg().front());
        if (firstSegIt != _itin.travelSeg().begin() && firstSegIt != _itin.travelSeg().end())
          fareBreaks.insert(*(firstSegIt - 1));
        fareBreaks.insert(fareUsage->travelSeg().back());
      }
  }
  else if (_fareMarketPath)
  {
    for (const auto mergedFareMarket : _allMFMs)
    {
      std::vector<TravelSeg*>::const_iterator firstSegIt =
          std::find(_itin.travelSeg().begin(),
                    _itin.travelSeg().end(),
                    mergedFareMarket->travelSeg().front());
      if (firstSegIt != _itin.travelSeg().begin() && firstSegIt != _itin.travelSeg().end())
        fareBreaks.insert(*(firstSegIt - 1));
      fareBreaks.insert(mergedFareMarket->travelSeg().back());
    }
  }
}

int
YQYRCalculator::applCount(const YQYRFees* s1, int first, int last)
{
  if (s1->sectorPortionOfTvlInd() != 'P')
    return 1;

  if (s1->connectExemptInd() != 'X' && s1->feeApplInd() == BLANK)
    return (last - first + 1);

  if (s1->connectExemptInd() == 'X' && first != last && s1->feeApplInd() == BLANK)
  {
    int count = last - first + 1;
    int64_t stopoverTime = SECONDS_PER_DAY; // default is 24 hours
    bool useCalendarDays = false;
    if (s1->stopConnectUnit() != BLANK)
    {
      if (s1->stopConnectTime() == 0)
      {
        useCalendarDays = true;
        stopoverTime = 1; // one calendar day
      }
      else
      {
        stopoverTime *= s1->stopConnectTime();
        switch (s1->stopConnectUnit())
        {
        // use fact that init'ed to DAYS unit
        case 'H':
          stopoverTime /= HOURS_PER_DAY;
          break;
        case 'M':
          stopoverTime *= 30;
          break; // days per month
        case 'N':
          stopoverTime /= 1440;
          break; // minutes per day
        case 'D': // intentional fall-thru; already init'ed
        default:
          break;
        }
      }
    } // endif - prep stopover vars
    SegmentItType front = _itin.travelSeg().begin() + first;
    SegmentItType back = _itin.travelSeg().begin() + last;
    for (; front != back; ++front)
    {
      if (isYQYRStopover(*front, *(front + 1), stopoverTime, useCalendarDays))
        continue;
      if (s1->sectorPortionViaLoc().empty() || checkLocation(*(*front)->destination(),
                                                             s1->sectorPortionViaLoc(),
                                                             s1->sectorPortionViaLocType(),
                                                             s1->vendor(),
                                                             s1->carrier()))
        --count;
    }
    return count;
  }
  return 1;
}

bool
YQYRCalculator::diagParamFilter(const YQYRFees* yqyr)
{
  if (_diagCollection == false)
    return false;

  std::map<std::string, std::string>::const_iterator i =
      _trx.diagnostic().diagParamMap().find("LS"); // Lowest SequenceNumber
  if (i != _trx.diagnostic().diagParamMap().end())
  {
    int seqnr = atoi(i->second.c_str());
    if (yqyr->seqNo() < seqnr)
      return false;
  }

  i = _trx.diagnostic().diagParamMap().find("HS"); // Highiest SequenceNumber
  if (i != _trx.diagnostic().diagParamMap().end())
  {
    int seqnr = atoi(i->second.c_str());
    if (yqyr->seqNo() > seqnr)
      return false;
  }

  i = _trx.diagnostic().diagParamMap().find("TX"); // TaxCode
  if (i != _trx.diagnostic().diagParamMap().end())
  {
    if (i->second != yqyr->taxCode())
      return false;
  }

  i = _trx.diagnostic().diagParamMap().find("TT"); // Tax sub Code
  if (i != _trx.diagnostic().diagParamMap().end())
  {
    if (i->second[0] != yqyr->subCode())
      return false;
  }

  i = _trx.diagnostic().diagParamMap().find("CC"); // Carrier Code
  if (i != _trx.diagnostic().diagParamMap().end())
  {
    if (i->second != yqyr->carrier())
      return false;
  }

  return true;
}

void
YQYRCalculator::printDiagnostic(const YQYRFees* yqyr, const char* diagMsg)
{
  if (LIKELY(_diagCollection == false))
    return;

  if (diagParamFilter(yqyr) == false)
    return;

  std::ostringstream stream;
  stream << yqyr->taxCode() << yqyr->subCode() << " SEQ:" << yqyr->seqNo() << diagMsg;
  _trx.diagnostic().insertDiagMsg(stream.str());
}

void
YQYRCalculator::printDiagnostic(const YQYRFees* yqyr, const char* diagMsg, int startSeg, int endSeg)
{
  if (LIKELY(_diagCollection == false))
    return;

  if (diagParamFilter(yqyr) == false)
    return;

  std::ostringstream stream;
  stream << yqyr->taxCode() << yqyr->subCode() << " SEQ:" << yqyr->seqNo();
  stream << "PSEG:" << startSeg << "-" << endSeg;
  stream << diagMsg;
  _trx.diagnostic().insertDiagMsg(stream.str());
}

void
YQYRCalculator::printDiagnostic(const YQYRFees* yqyr, const char* diagMsg, int Seg)
{
  if (LIKELY(_diagCollection == false))
    return;

  if (diagParamFilter(yqyr) == false)
    return;

  std::ostringstream stream;
  stream << yqyr->taxCode() << yqyr->subCode() << " SEQ:" << yqyr->seqNo();
  stream << "SEG:" << Seg << diagMsg;
  _trx.diagnostic().insertDiagMsg(stream.str());
}

void
YQYRCalculator::printDiagnostic(const YQYRApplication& appl, const char* diagMsg)
{
  if (LIKELY(_diagCollection == false))
    return;

  std::ostringstream stream;
  stream << diagMsg << appl.yqyr->taxCode() << appl.yqyr->subCode() << " SEQ:" << appl.yqyr->seqNo()
         << "\n";
  _trx.diagnostic().insertDiagMsg(stream.str());
}

void
YQYRCalculator::printDiagnostic(const char* diagMsg,
                                const std::vector<TravelSeg*>& gdSegs,
                                const std::vector<GlobalDirection>& gds,
                                TravelSeg* curSeg)
{
  if (LIKELY(_diagCollection == false))
    return;

  std::ostringstream stream;
  const uint32_t mileage = YQYR::YQYRUtils::journeyMileage(gdSegs.front()->origin(),
                                                           gdSegs.back()->destination(),
                                                           gds, _itin.travelDate(), _trx);
  stream << diagMsg << "DIST TO SEG:" << curSeg->segmentOrder() << " IS "
         << mileage << "\n";
  if (curSeg->isAir())
  {
    stream << diagMsg << "DIST TO SEG:" << curSeg->segmentOrder() << " CARRIER "
           << static_cast<AirSeg*>(curSeg)->carrier() << "\n";
  }
  _trx.diagnostic().insertDiagMsg(stream.str());
}

YQYRCalculator::MatchResult
YQYRCalculator::prevalidateFbc(int segInd, const YQYRFees& s1Rec) const
{
  if (s1Rec.hasSlashInFBC())
  {
    if (_noSlash)
      return MatchResult::NEVER;

    if (!_hasSomeSlashesInFBCs[segInd])
      return (s1Rec.sectorPortionOfTvlInd() == 'P') ? MatchResult::P_FAILED_NO_CHANCE
                                                    : MatchResult::S_FAILED;
  }
  return MatchResult::UNKNOWN;
}

bool
YQYRCalculator::validateTicketingDate(const YQYRFees& yqyr) const
{
  if (UNLIKELY(_trx.dataHandle().isHistorical()))
    return true; // YQYRFeesHistoricalDAO checks this in IsNotEffectiveYQHist

  const DateTime& tktDate = _trx.getRequest()->ticketingDT();

  if (tktDate < yqyr.firstTktDate() || tktDate > yqyr.lastTktDate())
    return false;

  return true;
}

namespace
{
struct IsOnCxr : public std::unary_function<TravelSeg*, bool>
{
  IsOnCxr(const CarrierCode cxr) : _cxr(cxr) {}
  bool operator()(TravelSeg* seg) const
  {
    const AirSeg* airSeg(seg->toAirSeg());
    if (!airSeg)
      return false;

    return airSeg->marketingCarrierCode() == _cxr;
  }
  const CarrierCode _cxr;
};
}

void
YQYRCalculator::processDirection(const CarrierCode cxr, Directions dir, FeeListT& feeList)
{
  SegmentItType endOfDirectionIt;
  SegmentItType sliceStartIt;
  const std::vector<TravelSeg*>& travelSegs(_itin.travelSeg());
  _subSliceResults.clear();

  if (_furthestSeg)
  {
    const SegmentItType furthestIt = std::find(travelSegs.begin(), travelSegs.end(), _furthestSeg);

    switch (dir)
    {
    case Directions::OUTBOUND:
    {
      endOfDirectionIt = furthestIt + 1;
      sliceStartIt = std::find_if(travelSegs.begin(), endOfDirectionIt, IsOnCxr(cxr));
    }
    break;
    case Directions::INBOUND:
    {
      endOfDirectionIt = travelSegs.end();
      const int beginOffset = (_direction == Directions::INBOUND ? 0 : 1);
      sliceStartIt = std::find_if(furthestIt + beginOffset, endOfDirectionIt, IsOnCxr(cxr));
    }
    break;

    case Directions::BOTH: // process direction won't ever be called with BOTH
    default:
      break;
    }
  }
  else
  {
    // In case we don't have _furthestSeg, treat the whole itin as a single direction.

    endOfDirectionIt = travelSegs.end();
    sliceStartIt = std::find_if(travelSegs.begin(), endOfDirectionIt, IsOnCxr(cxr));
  }

  if (sliceStartIt == endOfDirectionIt)
    return; // Carrier not present in the direction

  while (sliceStartIt < endOfDirectionIt)
  {
    SegmentItType sliceEndIt =
        std::find_if(sliceStartIt, endOfDirectionIt, std::not1(IsOnCxr(cxr)));
    int startInd = int(sliceStartIt - travelSegs.begin());
    int endInd = int(sliceEndIt - travelSegs.begin() - 1);
    _currentSlice = _trx.dataHandle().create<FeesBySlice>();
    _currentBucket->feesBySlice.push_back(_currentSlice);
    _currentSlice->dir = dir;
    const std::vector<const YQYRPath*>* ypv = processSlice(startInd, endInd, feeList);
    if (ypv)
      _currentSlice->yqyrpaths = *ypv;
    sliceStartIt = std::find_if(sliceEndIt, endOfDirectionIt, IsOnCxr(cxr));
  }
}

const std::vector<const YQYRCalculator::YQYRPath*>*
YQYRCalculator::processSlice(int startInd, int endInd, FeeListT& feeList)
{
  if (startInd > endInd)
    return nullptr;

  std::map<int, std::vector<const YQYRPath*>*>::const_iterator resFromCache =
      _subSliceResults.find(makeSliceKey(startInd, endInd));
  if (resFromCache != _subSliceResults.end())
    return resFromCache->second;

  std::vector<const YQYRPath*>* ypv(nullptr);
  _trx.dataHandle().get(ypv);
  const int origStartInd(startInd);

  YQYRPath* yp = _trx.dataHandle().create<YQYRPath>();

  int curEndInd = endInd;

  while (startInd <= endInd)
  {
    for (auto feeIt = feeList.begin(); feeIt != feeList.end();)
    {
      if (startInd > endInd)
        break;

      const YQYRFees* fee(*feeIt);
      MatchResult result = matchFeeRecord(fee, startInd, curEndInd);
      bool isPortion = fee->sectorPortionOfTvlInd() == 'P';

      switch (result)
      {
      case MatchResult::UNKNOWN:
        printDiagnostic(fee, " ***ERROR IN PROCESSING LOGIC***\n", startInd, curEndInd);
        ++feeIt; // Should never happen but want to avoid compiler warnings that this enum value is
        // not handled
        break;

      case MatchResult::NEVER:
        // Just to avoid BA questions
        printDiagnostic(fee, " ERASED - NEVER MATCH\n", startInd, curEndInd);
        feeIt = feeList.erase(feeIt);
        break;

      case MatchResult::S_FAILED:
        ++feeIt;
        break;

      case MatchResult::P_FAILED_NO_CHANCE:
        ++feeIt;
        curEndInd = endInd;
        break;

      case MatchResult::P_FAILED:
        if (startInd == curEndInd)
        {
          ++feeIt;
          curEndInd = endInd;
        }
        else
          --curEndInd;
        break;

      case MatchResult::S_CONDITIONALLY:
        merge(yp,
              YQYRApplication(fee, startInd, startInd, _trx, _itin.calculationCurrency()),
              processSlice(startInd + 1, endInd, feeList),
              ypv);
        ++feeIt;
        break;

      case MatchResult::P_CONDITIONALLY:
        merge(yp,
              YQYRApplication(fee, startInd, curEndInd, _trx, _itin.calculationCurrency()),
              processSlice(curEndInd + 1, endInd, feeList),
              ypv);
        if (startInd == curEndInd)
        {
          ++feeIt;
          curEndInd = endInd;
        }
        else
          --curEndInd;
        break;

      case MatchResult::UNCONDITIONALLY:
        yp->add(YQYRApplication(
            fee, startInd, isPortion ? curEndInd : startInd, _trx, _itin.calculationCurrency()));
        feeIt = feeList.begin();
        startInd = isPortion ? curEndInd + 1 : startInd + 1;
        curEndInd = endInd;
        break;
      }
    }
    if (startInd <= endInd)
      ++startInd;
  }

  ypv->push_back(yp);
  _subSliceResults.insert(std::make_pair(makeSliceKey(origStartInd, endInd), ypv));
  return ypv;
}

void
YQYRCalculator::processCarrier(const CarrierCode carrier, FeeListT& feeList)
{
  const YQYRFees* y = feeList.front();
  _currentBucket = nullptr;
  _reuseCFCBucket = true;

  for (const auto& valCxr : _valCxrMap)
  {
    _valCxrContext = valCxr.second;
    if (std::find(_valCxrContext->_concurringCarriers.begin(),
                  _valCxrContext->_concurringCarriers.end(),
                  carrier) == _valCxrContext->_concurringCarriers.end())
      continue;

    if (_reuseCFCBucket && _currentBucket)
    {
      _valCxrContext->_feeBuckets.push_back(_currentBucket);
      continue;
    }

    _currentBucket = _trx.dataHandle().create<CarrierFeeCodeBucket>();
    _currentBucket->cxr = y->carrier();
    _valCxrContext->_feeBuckets.push_back(_currentBucket);
    if (uint8_t(_direction) & uint8_t(Directions::OUTBOUND))
      processDirection(carrier, Directions::OUTBOUND, feeList);
    if (uint8_t(_direction) & uint8_t(Directions::INBOUND))
      processDirection(carrier, Directions::INBOUND, feeList);
  } // validating cxr
}

MoneyAmount
YQYRCalculator::calculatePercentageFee(Percent percent, FarePath* farePath) const
{
  FeePercentageCalculatorFP percentageCalc(_trx, _itin.useInternationalRounding(), farePath);
  return percentageCalc.calculatePercentageFee(percent);
}

void
YQYRCalculator::printDiagnostic(const char* diagMsg, const TravelSeg* turnAroundSeg)
{
  if (LIKELY(_diagCollection == false))
    return;

  std::ostringstream stream;
  stream << diagMsg << " SEG:" << turnAroundSeg->segmentOrder() << " LOC " << _furthestLoc->loc()
         << std::endl;
  stream << diagMsg << " FURTHEST SEG: " << turnAroundSeg->origAirport() << " - "
         << turnAroundSeg->destAirport() << "@";

  if (turnAroundSeg->isAir())
    stream << static_cast<const AirSeg*>(turnAroundSeg)->carrier();

  stream << std::endl;

  addItinDiagnostic(diagMsg, stream);

  _trx.diagnostic().insertDiagMsg(stream.str());
}

void
YQYRCalculator::printDiagnostic(const char* diagMsg, const Loc* turnAroundLoc)
{
  if (LIKELY(_diagCollection == false))
    return;

  std::ostringstream stream;
  stream << diagMsg << " LOC: " << turnAroundLoc->loc() << std::endl;
  stream << diagMsg << " FURTHEST SEG IS OUTSIDE OF THE FARE MARKET" << std::endl;

  addItinDiagnostic(diagMsg, stream);

  _trx.diagnostic().insertDiagMsg(stream.str());
}

void
YQYRCalculator::addItinDiagnostic(const char* diagMsg, std::ostringstream& stream)
{
  stream << diagMsg << " IS SIMPLE ITIN:" << isSimpleItin(_trx, _itin) << std::endl;
  stream << diagMsg << " RETURN TO ORIGIN:" << _returnsToOrigin << std::endl;
  stream << diagMsg << " IS ONLINE ITIN:" << _isOnlineItin << std::endl;

  stream << diagMsg << " RBD BY SEG NO: (" << _rbdBySegNo.size() << ") - [";

  for (const BookingCode bk : _rbdBySegNo)
    stream << bk.c_str() << ", ";

  stream << "]" << std::endl;
}

void
YQYRCalculator::sanityCheck(int howMany)
{
  if (UNLIKELY(_oneFPMode))
    return;

  if (fallback::unifyMemoryAborter(&_trx))
  {
    if (!Memory::changesFallback)
      TrxUtil::checkTrxMemoryFlag(_trx);
  }
  else
  {
    _trx.checkTrxAborted();
  }

  _yqyrApplCountLimit -= howMany;
  bool notGood = _yqyrApplCountLimit < 0;
  // Checking per one YQYRCalculator may not be enough.
  // If there are many PTCs in the request, the farthest point is not a stopover
  // and there are many FareMarketPaths a global check is also needed.
  // Doing it every 1 million counts, i.e. at least 24 MB.
  if (UNLIKELY(!notGood && ((_lastMemGrowthCheck - _yqyrApplCountLimit > 1000000))))
  {
    if (Memory::changesFallback)
    {
      notGood = _trx.memGrowthExceeded();
    }
    else
    {
      Memory::GlobalManager::instance()->checkTrxMemoryLimits(_trx);
    }
    _lastMemGrowthCheck = _yqyrApplCountLimit;
  }
  if (UNLIKELY(notGood))
  {
    throw TPMemoryOverused("TOO MANY YQYRPATHS");
  }
}

void
YQYRCalculator::addToValCxrMap(const ValCxrMap& valCxrMap)
{
  _valCxrMap.insert(valCxrMap.begin(), valCxrMap.end());
}

DateTime
YQYRCalculator::getProcessingDate() const
{
  if (fallback::yqyrRexFix(&_trx))
    return _travelDate;

  RexBaseTrx& trx = static_cast<RexBaseTrx&>(_trx);
  if (trx.trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE)
  {
    return std::max(trx.fareApplicationDT(), ItinUtil::getTravelDate(_itin.travelSeg()));
  }
  else
  {
    return trx.previousExchangeDT().isEmptyDate() ?
           trx.originalTktIssueDT() :
           trx.previousExchangeDT();
  }
}

} /* namespace tse */
