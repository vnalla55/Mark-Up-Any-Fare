//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DataModel/PricingTrx.h"

#include "Common/Assert.h"
#include "Common/CustomerActivationUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/Monitor.h"
#include "Common/MemoryUsage.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AmVatTaxRatesOnCharges.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Customer.h"
#include "DBAccess/CustomerActivationControl.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DiagTools.h"
#include "Pricing/PriceDeviator.h"
#include "Xform/AltPricingResponseFormatter.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/XMLConvertUtils.h"
#include "Xray/AsyncJsonContainer.h"

#include <boost/algorithm/cxx11/any_of.hpp>

#include <mutex>
#include <numeric>
#include <utility>

namespace tse
{
FALLBACK_DECL(fallbackFootNotePrevalidationForAltDates)
FALLBACK_DECL(fallbackWpncCloning)
FALLBACK_DECL(fallbackAMChargesTaxes)
FALLBACK_DECL(fallbackRTPricingContextFix)
FALLBACK_DECL(callOnceGNRCheck)

static Logger
logger("atseintl.DataModel.PricingTrx");
const DateTime PricingTrx::FAR_FUTURE = DateTime(2025, 12, 12);

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

PricingTrx::PricingTrx()
  : Trx(new Memory::TrxManager(this)),
    _itinsTimeSpan(std::make_pair(&DateTime::openDate(), &FAR_FUTURE)),
    _specFareCache(new SpecifiedFareCache)
{
  if (Memory::changesFallback)
    updateMemUsage();
  _dataHandle.get<BaggagePolicy>(_baggagePolicy);
}

#else

PricingTrx::PricingTrx()
  : Trx(new Memory::TrxManager(this)),
    _itinsTimeSpan(std::make_pair(&DateTime::openDate(), &FAR_FUTURE))
{
  if (Memory::changesFallback)
    updateMemUsage();
  _dataHandle.get<BaggagePolicy>(_baggagePolicy);
}

#endif

void
PricingTrx::setPnrSegmentCollocation(int16_t itinPnrSegment, int16_t similarItinPnrSegment)
{
  _pnrSegmentCollocation[itinPnrSegment].insert(similarItinPnrSegment);
}

bool
PricingTrx::checkPnrSegmentCollocation(int16_t itinPnrSegment, int16_t tpdMatchedViaLoc)
{
  if (_pnrSegmentCollocation.empty() || _pnrSegmentCollocation.count(itinPnrSegment) == 0)
    return false;

  return _pnrSegmentCollocation[itinPnrSegment].count(tpdMatchedViaLoc);
}

void
PricingTrx::setGlobalDirectionToMap(const Loc* loc1, const Loc* loc2, GlobalDirection gd) const
{
  std::pair<const Loc*, const Loc*> key(loc1, loc2);
  _gdMap.insert(std::make_pair(key, gd));
}

bool
PricingTrx::getGlobalDirectionFromMap(const Loc* loc1, const Loc* loc2, GlobalDirection& gd) const
{
  std::pair<const Loc*, const Loc*> key(loc1, loc2);
  GlobalDirectionMap::iterator gdMapIt = _gdMap.find(key);
  if (gdMapIt != _gdMap.end())
  {
    gd = gdMapIt->second;
    return true;
  }

  return false;
}

namespace
{
static size_t maxLegs = 10;

void
computeItinMetrics(const Itin* itin, PricingTrx::ItinMetrics& im)
{
  std::set<CarrierCode> cxrset;
  int tscounts[maxLegs];
  for (size_t j = 0; j < maxLegs; ++j)
    tscounts[j] = 0;
  std::vector<TravelSeg*>::const_iterator t = itin->travelSeg().begin();

  for (; t != itin->travelSeg().end(); ++t)
  {
    if (!(*t)->isAir())
      continue;
    const AirSeg* as = static_cast<const AirSeg*>(*t);
    cxrset.insert(as->carrier());
    if (LIKELY(static_cast<size_t>(as->legId()) < maxLegs))
      ++tscounts[as->legId()];
  }
  if (cxrset.size() == 1)
    ++im.numOnlineItins;
  else
    ++im.numInterlineItins;
  int maxsegcount = tscounts[0];
  for (size_t j = 1; j < maxLegs; ++j)
    if (tscounts[j] > maxsegcount)
      maxsegcount = tscounts[j];
  if (maxsegcount == 1)
    ++im.numNonStopItins;
  else if (maxsegcount == 2)
    ++im.numOneConnectItins;
  else if (maxsegcount == 3)
    ++im.numTwoConnectItins;
  else if (maxsegcount == 4)
    ++im.numThreeConnectItins;
}

} // namespace

PricingTrx::ItinMetrics
PricingTrx::getItinMetrics() const
{
  PricingTrx::ItinMetrics im;

  if (getTrxType() != PricingTrx::MIP_TRX || originDest().size() > 2)
    return im;

  for (const auto itin : this->itin())
  {
    if (itin->farePath().empty() || itin->errResponseCode() != ErrorResponseException::NO_ERROR)
      continue;
    computeItinMetrics(itin, im);
  }
  return im;
}

void
PricingTrx::updateMemUsage()
{
  _referenceRSS = MemoryUsage::getResidentMemorySize() / (1024 * 1024);
  _referenceVM = MemoryUsage::getVirtualMemorySize() / (1024 * 1024);
}

bool
PricingTrx::memGrowthExceeded() const
{
  bool exceeded(false);

  const size_t maxRSSJump = TrxUtil::maxRSSGrowth(*this);
  if (LIKELY(maxRSSJump > 0))
  {
    size_t currentRSS(0);
    if (!Memory::changesFallback)
    {
      currentRSS =
          Memory::MemoryMonitor::instance()->getUpdatedResidentMemorySize() / (1024 * 1024);
    }
    else
    {
      currentRSS = MemoryUsage::getResidentMemorySize() / (1024 * 1024);
    }
    exceeded =
        (currentRSS > _referenceRSS + maxRSSJump) && (currentRSS > TrxUtil::referenceRSS(*this));
  }

  const size_t maxVMJump = TrxUtil::maxVMGrowth(*this);
  if (UNLIKELY(maxVMJump > 0 && !exceeded))
  {
    size_t currentVM(0);
    if (!Memory::changesFallback)
    {
      currentVM = Memory::MemoryMonitor::instance()->getUpdatedVirtualMemorySize() / (1024 * 1024);
    }
    else
    {
      currentVM = MemoryUsage::getVirtualMemorySize() / (1024 * 1024);
    }
    exceeded = (currentVM > _referenceVM + maxVMJump) && (currentVM > TrxUtil::referenceVM(*this));
  }

  return exceeded;
}

//-----------------------------------------------------------------------------
// This method checks the customer control activation table (and sub-tables).
// A customer is considered activated for the given project code if the given
// date is greater than or equal to the table activation date AND the project
// activation indicator is set.
// Note: In addition to this method, processCustomerActivationRecords()
// also updates the CAC map.
//-----------------------------------------------------------------------------
bool
PricingTrx::isCustomerActivatedByDateAndFlag(const std::string& projCode, const DateTime& tktDate)
{
  boost::lock_guard<boost::mutex> guard(_custActvCntrlmutex);

  std::map<std::string, ActivationResult*>::iterator projCACMapDataIter =
      _projCACMapData.find(projCode);

  if (projCACMapDataIter == _projCACMapData.end())
  {
    CustomerActivationUtil caUtil(*this);

    const std::vector<CustomerActivationControl*>& cac =
        caUtil.getCustomerActivationRecords(projCode);

    if (!cac.empty())
    {
      DateTime activationDate;
      // Updates CAC map
      if (caUtil.processCustomerActivationRecords(cac, projCode, activationDate))
      {
        return (tktDate >= activationDate);
      }
    }
    else
      _projCACMapData.insert(std::make_pair(projCode, static_cast<ActivationResult*>(nullptr)));
  }
  else
  {
    ActivationResult* activationRslt = projCACMapDataIter->second;
    if (activationRslt)
    {
      return (tktDate >= activationRslt->finalActvDate());
    }
  }
  return false;
}

bool
PricingTrx::isCustomerActivatedByDate(const std::string& projCode)
{
  boost::lock_guard<boost::mutex> guard(_custActvCntrlmutex);

  std::map<std::string, ActivationResult*>::iterator projCACMapDataIter =
      _projCACMapData.find(projCode);

  if (projCACMapDataIter == _projCACMapData.end())
  {
    CustomerActivationUtil caUtil(*this);

    const std::vector<CustomerActivationControl*>& cac =
        caUtil.getCustomerActivationRecords(projCode);

    if (!cac.empty())
    {
      DateTime activationDate;
      if (caUtil.processCustomerActivationRecords(cac, projCode, activationDate))
      {
        if (_request)
          return (_request->ticketingDT() >= activationDate);
      }
    }
    else
      _projCACMapData.insert(std::make_pair(projCode, static_cast<ActivationResult*>(nullptr)));
  }
  else
  {
    ActivationResult* activationRslt = projCACMapDataIter->second;
    if (activationRslt)
    {
      if (LIKELY(_request))
        return (_request->ticketingDT() >= activationRslt->finalActvDate());
    }
    // For GNR we will check only date, for other projects we will verify
    // the activation flag which is below
  }
  return false;
}

bool
PricingTrx::isCustomerActivatedByFlag(const std::string& projCode, const CarrierCode* cxr)
{
  boost::lock_guard<boost::mutex> guard(_custActvCntrlmutex);

  std::map<std::string, ActivationResult*>::iterator projCACMapDataIter =
      _projCACMapData.find(projCode);

  if (projCACMapDataIter == _projCACMapData.end())
  {
    if (!cxr && getOptions() && getOptions()->isRtw())
      cxr = &itin().front()->fareMarket().back()->governingCarrier();

    CustomerActivationUtil caUtil(*this, cxr);
    const std::vector<CustomerActivationControl*>& cac =
        caUtil.getCustomerActivationRecords(projCode);

    if (!cac.empty())
    {
      DateTime activationDate;
      if (caUtil.processCustomerActivationRecords(cac, projCode, activationDate))
        return true;
    }
    else
      _projCACMapData.insert(std::make_pair(projCode, static_cast<ActivationResult*>(nullptr)));
  }
  else
  {
    ActivationResult* activationRslt = projCACMapDataIter->second;
    if (activationRslt)
      return activationRslt->isActivationFlag();
  }
  return false;
}

namespace
{
inline const DateTime&
getDeparture(const Itin* itin)
{
  return itin->travelSeg().front()->departureDT();
}

struct LessDeparture
{
  bool operator()(const Itin* lh, const Itin* rh) const
  {
    return getDeparture(lh) < getDeparture(rh);
  }
};

inline const DateTime&
getArrival(const Itin* itin)
{
  return itin->travelSeg().back()->arrivalDT();
}

struct LessArrival
{
  bool operator()(const Itin* lh, const Itin* rh) const { return getArrival(lh) < getArrival(rh); }
};

class HasFareBasisPredicate : public std::unary_function<const TravelSeg*, bool>
{
public:
  HasFareBasisPredicate() {}
  bool operator()(const TravelSeg* travelSeg)
  {
    TSE_ASSERT(travelSeg != nullptr);
    return !travelSeg->fareBasisCode().empty();
  }
};

} // namespace

void
PricingTrx::setItinsTimeSpan()
{
  _itinsTimeSpan.first =
      &getDeparture(*std::min_element(itin().begin(), itin().end(), LessDeparture()));
  _itinsTimeSpan.second =
      &getArrival(*std::max_element(itin().begin(), itin().end(), LessArrival()));
}

void
PricingTrx::setTrxGeometryCalculator(skipper::TrxGeometryCalculator* calculator)
{
  TSE_ASSERT(calculator != nullptr);
  TSE_ASSERT(_trxGeometryCalculator == nullptr);
  _trxGeometryCalculator = calculator;
}

bool
PricingTrx::isCommandPricingRq() const
{
  return boost::algorithm::any_of(travelSeg().begin(), travelSeg().end(), HasFareBasisPredicate());
}

void
PricingTrx::computeBrandOrder()
{
  _brandOrder.clear();
  uint16_t brandIndex = 0;
  // Regular brands
  for (uint16_t index = 0; index < _brandProgramVec.size(); ++index)
  {
    const BrandCode& brandCode = _brandProgramVec[index].second->brandCode();
    if (_brandOrder.count(brandCode) == 0)
      _brandOrder[brandCode] = brandIndex++;
  }

  if (fallback::fallbackRTPricingContextFix(this))
    return;

  // Special brand codes:
  _brandOrder[NO_BRAND] = brandIndex++;
  _brandOrder[ANY_BRAND] = brandIndex++;
}

bool
PricingTrx::isWpncCloningEnabled() const
{
  return altTrxType() == PricingTrx::WP &&
         (getOptions()->isRtw() || !fallback::fallbackWpncCloning(this));
}

bool
PricingTrx::isProcess2CC() const
{
  const FopBinNumber& fop = _request->formOfPayment();
  const FopBinNumber& fop2 = _request->secondFormOfPayment();

  if (6 != fop.size() || 6 != fop2.size() || !_request->isFormOfPaymentCard() ||
      _request->paymentAmountFop() == 0)
    return false;

  return std::find_if(fop.begin(), fop.end(), !boost::bind<bool>(&isDigit, _1)) == fop.end() &&
         std::find_if(fop2.begin(), fop2.end(), !boost::bind<bool>(&isDigit, _1)) == fop2.end();
}

bool
PricingTrx::isBaggageRequest()
{
  return (activationFlags().isAB240() &&
          (getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::DisclosureData) ||
           getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::CatalogData)));
}

bool
PricingTrx::isObFeeApplied() const
{
  if (!_request->isCollectOBFee())
    return false;

  return (_request->isFormOfPaymentCard() && !_request->formOfPayment().empty());
}

bool
PricingTrx::isAnyNonCreditCardFOP() const
{
  return (!_request->isFormOfPaymentCard() &&
          (_request->isFormOfPaymentCash() || _request->isFormOfPaymentCheck() ||
           _request->isFormOfPaymentGTR()));
}

bool
PricingTrx::isCabinHigherThanEconomyRequested() const
{
  std::map<size_t, CabinType>::const_iterator it = _cabinsForLegs.begin();
  for (; it != _cabinsForLegs.end(); ++it)
  {
    const CabinType& cabin = it->second;
    if (!cabin.isValidCabin())
      continue;

    if (cabin.getCabinIndicator() < CabinType::ECONOMY_CLASS)
      return true;
  }
  return false;
}

size_t
PricingTrx::getWholeTripCabinIndex() const
{
  if (_cabinsForLegs.empty())
  {
    // no valid cabin found, return economy
    return CabinType::generalIndex(CabinType::ECONOMY_CLASS);
  }

  std::map<size_t, CabinType>::const_iterator it = _cabinsForLegs.begin();
  // by default we have economy ...
  size_t cabinIndex = CabinType::generalIndex(CabinType::ECONOMY_CLASS);
  if (it->second.isValidCabin())
  {
    // ...but if the cabin on the first segment is valid we override it
    cabinIndex = it->second.generalIndex();
  }
  ++it;

  for (; it != _cabinsForLegs.end(); ++it)
  {
    // if the current cabin is not valid we consider it as economy
    size_t currentCabin = CabinType::generalIndex(CabinType::ECONOMY_CLASS);
    if (it->second.isValidCabin())
    {
      currentCabin = it->second.generalIndex();
    }
    if (currentCabin != cabinIndex)
    {
      // UNDEFINDE_CLASS index is used to indicate differed cabins requested for
      // different legs.
      return CabinType::generalIndex(CabinType::UNDEFINED_CLASS);
    }
  }
  return cabinIndex;
}

void
PricingTrx::loadAmVatTaxRatesOnCharges()
{
  if (!fallback::fallbackAMChargesTaxes(this) && !_amVatTaxRatesOnCharges &&
      activationFlags().isAB240())
  {
    std::string acmsData(TrxUtil::amChargesTaxesListData(*this));
    _amVatTaxRatesOnCharges = &dataHandle().safe_create<AmVatTaxRatesOnCharges>(acmsData);
  }
}

void
PricingTrx::setFootNotePrevalidationEnabled(bool value)
{
  _footNotePrevalidationEnabled = value;
  setupFootNotePrevalidation();
}

void
PricingTrx::setupFootNotePrevalidation()
{
  if (fallback::fallbackFootNotePrevalidationForAltDates(this))
    _footNotePrevalidationAllowed = _footNotePrevalidationEnabled && !isAltDates();
  else
    _footNotePrevalidationAllowed = _footNotePrevalidationEnabled;
}

bool
PricingTrx::isRequestedFareBasisInfoUseful() const
{
  return std::any_of(_fareMarket.begin(),
                     _fareMarket.end(),
                     [](const FareMarket* fm)
                     { return fm->isRequestedFareBasisInfoUseful(); });
}

bool
PricingTrx::areFareMarketsFailingOnRules() const
{
  size_t tsIdx = 0;
  std::vector<bool> tsHasValidRul(_travelSeg.size());

  if (_fareMarket.empty())
  {
    return false;
  }

  for (TravelSeg* ts : _travelSeg)
  {
    if (!ts->isAir())
    {
      tsHasValidRul[tsIdx] = true;
      ++tsIdx;
      continue;
    }

    for (FareMarket* fm : _fareMarket)
    {
      if (fm->isTravelSegPresent(ts) && !fm->allPaxTypeFare().empty() &&
          fm->existRequestedFareBasisValidFares() && !fm->areAllFaresFailingOnRules())
      {
        tsHasValidRul[tsIdx] = true;
      }
    }
    ++tsIdx;
  }
  return std::any_of(tsHasValidRul.begin(),
                     tsHasValidRul.end(),
                     [](bool item)
                     { return !item; });
}

void
PricingTrx::checkFareMarketsCoverTravelSegs()
{
  size_t tsIdx = 0;
  std::vector<bool> tsHasValidRfb(_travelSeg.size());
  std::vector<bool> tsHasValidBkg(_travelSeg.size());
  std::vector<bool> tsHasValidRtg(_travelSeg.size());
  std::vector<bool> tsHasValidRul(_travelSeg.size());

  if (_fareMarket.empty() || _travelSeg.empty())
  {
    return;
  }

  for (TravelSeg* ts : _travelSeg)
  {
    if (!ts->isAir())
    {
      tsHasValidRfb[tsIdx] = true;
      tsHasValidBkg[tsIdx] = true;
      tsHasValidRtg[tsIdx] = true;
      tsHasValidRul[tsIdx] = true;
      ++tsIdx;
      continue;
    }

    for (FareMarket* fm : _fareMarket)
    {
      if (fm->isTravelSegPresent(ts) && !fm->allPaxTypeFare().empty() &&
          fm->existRequestedFareBasisValidFares())
      {
        tsHasValidRfb[tsIdx] = true;
        if (!fm->areAllFaresFailingOnBookingCodes())
        {
          tsHasValidBkg[tsIdx] = true;
        }
        if (!fm->areAllFaresFailingOnRouting())
        {
          tsHasValidRtg[tsIdx] = true;
        }
        if (fm->existValidFares())
        {
          tsHasValidRul[tsIdx] = true;
        }
      }
    }
    ++tsIdx;
  }

  if (std::any_of(tsHasValidRfb.begin(),
                  tsHasValidRfb.end(),
                  [](bool item)
                  { return !item; }))
  {
    throw ErrorResponseException(ErrorResponseException::FARE_BASIS_NOT_AVAIL);
  }
  if (std::any_of(tsHasValidRtg.begin(),
                  tsHasValidRtg.end(),
                  [](bool item)
                  { return !item; }))
  {
    throw ErrorResponseException(ErrorResponseException::NO_FARES_RBD_CARRIER);
  }
  if (std::any_of(tsHasValidBkg.begin(),
                  tsHasValidBkg.end(),
                  [](bool item)
                  { return !item; }) ||
      std::any_of(tsHasValidRul.begin(),
                  tsHasValidRul.end(),
                  [](bool item)
                  { return !item; }))
  {
    throw ErrorResponseException(ErrorResponseException::SFB_RULE_VALIDATION_FAILED);
  }
}

bool
PricingTrx::hasNoEmptyRfbOrAdditionalAttrOnEachSeg() const
{
  if (_travelSeg.empty())
    return false;

  for (const TravelSeg* tvlSeg : _travelSeg)
  {
    if (tvlSeg->requestedFareBasis().empty())
      return false;

    if (!tvlSeg->requestedFareBasis().front().passengerCode.empty())
      return false;
  }
  return true;
}

bool
PricingTrx::isLowestFareOverride() const
{
  return _options->isMatchAndNoMatchRequested();
}

const Percent*
PricingTrx::getDiscountPercentageNew(const FareMarket& fareMarket) const
{
  return TrxUtil::getDiscountPercentage(fareMarket, getRequest()->discountsNew(), isMip());
}

const DiscountAmount*
PricingTrx::getDiscountAmountNew(const FareMarket& fareMarket) const
{
  return TrxUtil::getDiscountAmount(fareMarket, getRequest()->discountsNew());
}

const Percent*
PricingTrx::getDiscountPercentageNew(const PricingUnit& pricingUnit) const
{
  return TrxUtil::getDiscountPercentage(pricingUnit, getRequest()->discountsNew(), isMip());
}

const DiscountAmount*
PricingTrx::getDiscountAmountNew(const PricingUnit& pricingUnit) const
{
  return TrxUtil::getDiscountAmount(pricingUnit, getRequest()->discountsNew());
}

bool
PricingTrx::isNormalWpPricing() const
{
  return altTrxType() == PricingTrx::WP && !activationFlags().isSearchForBrandsPricing();
}

void
PricingTrx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  std::string tmpResponse(ere.message());
  if (ere.code() > 0 && ere.message().empty())
  {
    tmpResponse = "UNKNOWN EXCEPTION";
  }

  if ( isNormalWpPricing() )
  {
    PricingResponseFormatter formatter;
    response += formatter.formatResponse(tmpResponse, false, *this, nullptr, ere.code());
  }
  else
  {
    AltPricingResponseFormatter formatter;
    response = formatter.formatResponse(tmpResponse, false, *this, nullptr, ere.code());
  }
}

bool
PricingTrx::convert(std::string& response)
{
  XMLConvertUtils::tracking(*this);
  if (!taxRequestToBeReturnedAsResponse().empty())
  {
    response = taxRequestToBeReturnedAsResponse();
    return true;
  }

  LOG4CXX_DEBUG(logger, "Doing PricingTrx response");

  Diagnostic& diag = diagnostic();
  if (fareCalcCollector().empty())
  {
    LOG4CXX_WARN(logger, "Pricing Response Items are Missing");
  }

  FareCalcCollector* fareCalcCollector = nullptr;
  if ((diag.diagnosticType() == DiagnosticNone || diag.diagnosticType() == Diagnostic855) &&
      !_fareCalcCollector.empty())
  {
    fareCalcCollector = _fareCalcCollector.front();
  }

  std::string tmpResponse = diag.toString();

  if (diag.diagnosticType() == Diagnostic854 &&
      (diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::TOPLINE_METRICS) ||
       diag.diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::FULL_METRICS)))
  {
    tmpResponse += "\n\n" + utils::getMetricsInfo(*this);
  }

  if (diag.diagnosticType() != DiagnosticNone && tmpResponse.length() == 0)
  {
    char tmpBuf[512];
    sprintf(tmpBuf, "DIAGNOSTIC %d RETURNED NO DATA", diag.diagnosticType());
    tmpResponse.insert(0, tmpBuf);
  }

  PricingResponseFormatter formatter;
  response = formatter.formatResponse(tmpResponse, displayOnly(), *this, fareCalcCollector);

  return true;
}

int
PricingTrx::getValidForPOFaresCount() const
{
  return std::accumulate(_fareMarket.cbegin(),
                         _fareMarket.cend(),
                         0,
                         [](const auto val, const auto fareMarket)
                         {
    if (UNLIKELY(!fareMarket))
      return val;
    return val + fareMarket->getValidForPOFaresCount();
  });
}

void
PricingTrx::assignPriceDeviator(PriceDeviatorPtr deviator)
{
  _priceDeviator.assignDeviator(std::move(deviator));
}

void
PricingTrx::configurePriceDeviator() const
{
  _priceDeviator.configureDeviation();
}

bool
PricingTrx::hasPriceDynamicallyDeviated() const
{
  return _request->isDAorPAentry() || _request->isDPorPPentry();
}

void
PricingTrx::addCountrySettlementPlanInfo(CountrySettlementPlanInfo* countrySettlementPlanInfo)
{
  if (isLockingNeededInShoppingPQ())
  {
    std::lock_guard<std::mutex> lock(_valCxrMutex);
    _countrySettlementPlanInfos.push_back(countrySettlementPlanInfo);
  }
  else
    _countrySettlementPlanInfos.push_back(countrySettlementPlanInfo);
}

std::vector<CountrySettlementPlanInfo*>
PricingTrx::getCopyOfCountrySettlementPlanInfos() const
{
  std::lock_guard<std::mutex> lock(_valCxrMutex);
  return _countrySettlementPlanInfos;
}

ValidatingCxrGSAData*
PricingTrx::getValCxrGsaData(const std::string& hashString, bool isLockingNeeded) const
{
  std::unique_lock<std::mutex> lock(validatingCxrMutex(), std::defer_lock);
  if (isLockingNeeded)
    lock.lock();

  auto i = validatingCxrHashMap().find(hashString);

  if (i == validatingCxrHashMap().end())
    return nullptr;
  else
    return i->second;
}

SpValidatingCxrGSADataMap*
PricingTrx::getSpValCxrGsaDataMap(const std::string& hashString, bool isLockingNeeded) const
{
  std::unique_lock<std::mutex> lock(validatingCxrMutex(), std::defer_lock);
  if (isLockingNeeded)
    lock.lock();

  auto i = hashSpValidatingCxrGsaDataMap().find(hashString);

  if (i == hashSpValidatingCxrGsaDataMap().end())
    return nullptr;
  else
    return i->second;
}

bool
PricingTrx::isGNRAllowed()
{
  auto check = [this]
  {
    if (_request && _request->ticketingAgent() && _request->ticketingAgent()->isArcUser())
      return false;
    return isCustomerActivatedByDate("GNR");
  };

  if (isExchangeTrx() || fallback::callOnceGNRCheck(this))
  {
    return check();
  }
  else
  {
    std::call_once(_GNRFlag,
                   [this, check]
                   { _GNRAllowed = check(); });
    return _GNRAllowed;
  }
}

void
PricingTrx::assignXrayJsonMessage(xray::JsonMessagePtr&& jsonMessage)
{
  TSE_ASSERT(!_xrayMessage);
  _xrayMessage = std::move(jsonMessage);
}

void
PricingTrx::pushXrayJsonMessageToContainer()
{
  TSE_ASSERT(_xrayMessage);
  xray::asyncjsoncontainer::push(std::move(_xrayMessage));
}
} // tse namespace
