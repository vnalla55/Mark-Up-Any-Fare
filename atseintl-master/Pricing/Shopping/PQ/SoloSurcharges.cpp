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
#include "Pricing/Shopping/PQ/SoloSurcharges.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Fares/FareController.h"
#include "Fares/FareValidatorOrchestratorShopping.h"
#include "Pricing/FarePathWrapper.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/Shopping/PQ/ScopedFMGlobalDirSetter.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"
#include "Taxes/LegacyTaxes/ServiceFeeYQ.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Util/BranchPrediction.h"

#include <limits>

using namespace std;
using namespace boost;

namespace tse
{
namespace
{
Logger
logger("atseintl.pricing.SoloSurchargers");

ConfigurableValue<bool> soloSurchargesEnabled(
    "SHOPPING_DIVERSITY", "SOLO_SURCHARGES");
ConfigurableValue<size_t> numFaresForCAT12Calculation(
    "SHOPPING_DIVERSITY", "NUM_FARES_FOR_CAT12_CALCULATION");
ConfigurableValue<size_t> numCategoriesForYQYRCalculation(
    "SHOPPING_DIVERSITY", "NUM_CAT_FOR_YQYR_CALCULATION_PER_BKC");
}

SoloSurcharges::SoloSurcharges(ShoppingTrx& trx)
  : _trx(trx),
    _yqyrCalculator(trx)
{
  if (!soloSurchargesEnabled.getValue())
    return;

  _numFaresForCAT12Calculation = numFaresForCAT12Calculation.getValue();
  if (_numFaresForCAT12Calculation == 0)
    return;

  _enabled = true;
  initTaxDiagnostics();
  _surchargesDetailsEnabled = (_trx.diagnostic().diagnosticType() == Diagnostic923);

  _yqyrCalculator.init();

  // it doesn't make sense to gather more fares than available
  const size_t fvoMaxFares = FareValidatorOrchestratorShopping::numberOfFaresToProcess.getValue();
  _numApplicableFares = std::min(_numFaresForCAT12Calculation, fvoMaxFares);
  _numFaresForCAT12Calculation = _numApplicableFares;

  _numCategoriesForYQYRCalculation = numCategoriesForYQYRCalculation.getValue();
}

void
SoloSurcharges::initTaxDiagnostics()
{
  bool diagEnabled = false;
  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  switch (diagType)
  {
  case AllPassTaxDiagnostic281: // 801
  case LegacyTaxDiagnostic24: // 804
    diagEnabled = true;
    break;
  default:
    diagEnabled = false;
  }

  if (diagEnabled)
  {
    DCFactory* factory = DCFactory::instance();
    _dc = factory->create(_trx);
    _dc->trx() = &_trx;
    _dc->enable(diagType);
  }
}

void
SoloSurcharges::flushTaxDiagnosticsMsg()
{
  if (_dc)
  {
    _dc->flushMsg();
  }
}

void
SoloSurcharges::collectTaxDiagnostics(TaxResponse& taxResponse)
{
  if (UNLIKELY(_dc))
  {
    if (_trx.diagnostic().diagnosticType() == AllPassTaxDiagnostic281 ||
        _trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24)
    {
      *_dc << taxResponse;
    }
  }
}

void
SoloSurcharges::process()
{
  if (!_enabled)
    return;

  for (FareMarket* fareMarket : _trx.fareMarket())
  {
    if (fareMarket->getApplicableSOPs() == nullptr)
    {
      if (fareMarket->origin() && fareMarket->destination())
      {
        LOG4CXX_DEBUG(logger,
                      "ApplicableSOPs for fare market: " << fareMarket->origin()->loc() << "-"
                                                         << fareMarket->destination()->loc()
                                                         << " are not set.");
      }
      else
      {
        LOG4CXX_DEBUG(logger, "ApplicableSOPs for a fare market are not set.");
      }
      continue;
    }

    SurchargesDetail maxDetail;
    SurchargesDetails details;
    _numFareClassCatUsed = 0;

    details.reserve(_numFaresForCAT12Calculation + 1);
    const bool processed = updateAllFares(*fareMarket, details, maxDetail);

    if (UNLIKELY(processed && _surchargesDetailsEnabled))
    {
      details.push_back(maxDetail);
      _surchargesDetailsMap.insert(std::make_pair(fareMarket, details));
    }
  }
}

void
SoloSurcharges::restoreTotalNUCAmount()
{
  if (!_enabled)
    return;

  for (ShoppingTrx::FlightMatrix::value_type& solution : _trx.flightMatrix())
  {
    restoreTotalNUCAmount(solution.second);
  }

  for (ShoppingTrx::EstimateMatrix::value_type& solution : _trx.estimateMatrix())
  {
    restoreTotalNUCAmount(solution.second.second);
  }
}

void
SoloSurcharges::restoreTotalNUCAmount(GroupFarePath* gfp)
{
  if (UNLIKELY(!_enabled))
    return;

  if (UNLIKELY(gfp == nullptr))
    return;

  MoneyAmount totalNUCBaseFareAmount = gfp->getTotalNUCAmount();

  FarePath* farePath = gfp->groupFPPQItem().back()->farePath();
  for (const PricingUnit* pricingUnit : farePath->pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      const PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();
      totalNUCBaseFareAmount -= paxTypeFare->getSoloSurcharges();
    }
  }

  farePath->setTotalNUCAmount(totalNUCBaseFareAmount);
  gfp->setTotalNUCBaseFareAmount(totalNUCBaseFareAmount);
}

PaxTypeFareVec
SoloSurcharges::getApplicableFares(FareMarket& fareMarket)
{
  if (!_numApplicableFares)
    return PaxTypeFareVec();

  PaxTypeFareVec result;
  result.reserve(_numApplicableFares);

  for (PaxTypeFare* ptf : fareMarket.allPaxTypeFare())
  {
    if (!ptf->isValid())
      continue;

    const auto& cxrSopsMap = *fareMarket.getApplicableSOPs();
    if (std::none_of(cxrSopsMap.begin(),
                     cxrSopsMap.end(),
                     [this, ptf](const auto& cxrSops)
                     { return this->preparePTFForValidation(cxrSops.first, *ptf); }))
      continue;

    result.push_back(ptf);
    if (result.size() >= _numApplicableFares)
      break;
  }

  return result;
}

inline static void
applySurcharge(PaxTypeFare& ptf, MoneyAmount totalSurcharge)
{
  ptf.nucFareAmount() += totalSurcharge;
  ptf.setSoloSurcharges(totalSurcharge);
}

bool
SoloSurcharges::updateAllFares(FareMarket& fareMarket,
                               SurchargesDetails& details,
                               SurchargesDetail& maxDetail)
{
  const PaxTypeFareVec& allFares = fareMarket.allPaxTypeFare();
  const PaxTypeFareVec applicableFares = getApplicableFares(fareMarket);

  if (applicableFares.empty())
    return false;

  // 1. Assign INVALID_AMOUNT in order to mark fares not processed yet
  for (PaxTypeFare* ptf : allFares)
    ptf->setSoloSurcharges(INVALID_AMOUNT);

  // 2. Calculate YQYR + surcharges for applicable fares
  size_t numFaresProcessed = 0;
  MoneyAmount yqyrEstimate = 0;

  for (PaxTypeFare* ptf : applicableFares)
  {
    if (UNLIKELY(numFaresProcessed >= _numFaresForCAT12Calculation))
      break;

    const bool calculateYqyr = numFaresProcessed == 0;
    SurchargesDetail currentDetail;

    if (UNLIKELY(!process(&fareMarket, ptf, currentDetail, applicableFares, calculateYqyr)))
      continue;

    if (numFaresProcessed == 0 || currentDetail._surchargesAmount > maxDetail._surchargesAmount)
      maxDetail = currentDetail;
    if (calculateYqyr)
      yqyrEstimate = currentDetail._taxAmount;
    if (UNLIKELY(_surchargesDetailsEnabled))
      details.push_back(currentDetail);

    applySurcharge(*ptf, currentDetail._surchargesAmount + yqyrEstimate);
    ++numFaresProcessed;
  }

  // 3. Update remaining fares with constant, estimated amount.
  for (PaxTypeFare* ptf : allFares)
    if (ptf->getSoloSurcharges() == INVALID_AMOUNT)
      applySurcharge(*ptf, maxDetail._surchargesAmount + yqyrEstimate);

  if (numFaresProcessed > 0)
  {
    FareController fareController(_trx, *_trx.journeyItin(), fareMarket);
    fareController.sortPaxTypeFares();
    return true;
  }

  return false;
}

void
SoloSurcharges::calculateTaxesForShortCutPricing(const SortedFlightsMap& flightMap)
{
  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SURCHARGE_RULE);
  RuleControllerWithChancelor<PricingUnitRuleController> ruleController(FPRuleValidation,
                                                                        categories);

  SortedFlightsMap::const_iterator i = flightMap.begin();
  SortedFlightsMap::const_iterator j = flightMap.end();

  if (_trx.startShortCutPricingItin() == 0)
    return;

  if (std::distance(i, j) <= _trx.startShortCutPricingItin())
    return;

  for (; i != j; ++i)
  {
    if (!std::get<1>((*i).second)->isShortCutPricing())
      continue;

    MoneyAmount totalNUCBaseFareAmount = 0.0;
    for (FPPQItem* fIter : std::get<1>((*i).second)->groupFPPQItem())
    {
      FarePath* farePath = fIter->farePath();
      if (farePath->itin())
      {
        ItinUtil::setItinCurrencies(*farePath->itin(), _trx.ticketingDate());
        farePath->baseFareCurrency() = farePath->itin()->originationCurrency();
        farePath->calculationCurrency() = farePath->itin()->calculationCurrency();
      }
      for (tse::PricingUnit* pIter : farePath->pricingUnit())
      {
        for (FareUsage* fuIter : pIter->fareUsage())
        {
          totalNUCBaseFareAmount += fuIter->paxTypeFare()->fareAmount();
        }
        ruleController.validate(_trx, *farePath, *pIter);
        pIter->taxAmount() = 0;
      }

      farePath->setTotalNUCAmount(totalNUCBaseFareAmount);
      std::get<1>((*i).second)->setTotalNUCBaseFareAmount(totalNUCBaseFareAmount);

      RuleUtil::getSurcharges(_trx, *farePath);

      MoneyAmount taxAmt = getTax(farePath);
      std::get<1>((*i).second)->setTotalNUCAmount(farePath->getTotalNUCAmount() + taxAmt);
      farePath->plusUpAmount() = taxAmt;
    }
  }

  if (_dc)
  {
    i = flightMap.begin();
    std::advance(i, _trx.startShortCutPricingItin());
    uint16_t itinIndex = 0;
    for (; i != j; ++i)
    {
      for (FPPQItem* fIter : std::get<1>((*i).second)->groupFPPQItem())
      {
        *_dc << "ITIN INDEX" << ++itinIndex << std::endl;
        uint16_t taxResIndex = 0;
        for (TaxResponse* tIter : fIter->farePath()->itin()->getTaxResponses())
        {
          *_dc << "TAX RESPONSE INDEX" << ++taxResIndex << std::endl;
          collectTaxDiagnostics(*tIter);
        }
      }
    }
  }
}

MoneyAmount
SoloSurcharges::getTax(FarePath* farePath)
{
  Itin* curItin = farePath->itin();
  MoneyAmount taxAmt = 0;

  if (curItin == nullptr)
    return taxAmt;

  TaxMap::TaxFactoryMap taxFactoryMap;
  TaxMap::buildTaxFactoryMap(_trx.dataHandle(), taxFactoryMap);

  TaxItinerary taxItinerary;
  curItin->farePath().push_back(farePath);
  taxItinerary.initialize(_trx, *curItin, taxFactoryMap);
  taxItinerary.accumulator();
  curItin->farePath().clear();

  const TaxResponse* taxResponse = curItin->getTaxResponses().back();

  if (taxResponse == nullptr)
    return taxAmt;

  TaxResponse::TaxItemVector::const_iterator taxItemIter = taxResponse->taxItemVector().begin();
  TaxResponse::TaxItemVector::const_iterator taxItemEndIter = taxResponse->taxItemVector().end();
  for (; taxItemIter != taxItemEndIter; ++taxItemIter)
  {
    const TaxItem* taxItem = *taxItemIter;
    taxAmt += taxItem->taxAmount();
  }

  return taxAmt;
}

bool
SoloSurcharges::process(FareMarket* fareMarket,
                        PaxTypeFare* paxTypeFare,
                        SurchargesDetail& surchargesDetail,
                        const PaxTypeFareVec& applicableFares,
                        bool withTaxes)
{
  bool validResult = false;

  for (ApplicableSOP::value_type& applicableSOP : *fareMarket->getApplicableSOPs())
  {
    if (!preparePTFForValidation(applicableSOP.first, *paxTypeFare))
      continue;

    const uint32_t bitmapSize = paxTypeFare->getFlightBitmapSize();
    TSE_ASSERT(applicableSOP.second.size() == bitmapSize);

    for (uint32_t bitIndex = 0; !validResult && bitIndex < bitmapSize; ++bitIndex)
    {
      if (!paxTypeFare->isFlightValid(bitIndex))
        continue;

      SOPUsage sopUsage(applicableSOP.second[bitIndex]);

      if (LIKELY(sopUsage.applicable_ && sopUsage.itin_ != nullptr && sopUsage.startSegment_ != -1 &&
          sopUsage.endSegment_ != -1))
      {
        Itin* orgItin = sopUsage.itin_;
        sopUsage.itin_ = _trx.dataHandle().create<Itin>();
        sopUsage.itin_->duplicate(*orgItin, *paxTypeFare, _trx.dataHandle());

        surchargesDetail._paxTypeFare = paxTypeFare;
        surchargesDetail._orgNucFareAmount = paxTypeFare->nucFareAmount();
        validResult =
            process(fareMarket, sopUsage, surchargesDetail, bitIndex, applicableFares, withTaxes);
      }
    }

    if (LIKELY(validResult))
      break;
  }

  return validResult;
}

GlobalDirection*
SoloSurcharges::getGlobalDirection(FareMarket* fareMarket, uint32_t origSopId) const
{
  if (fareMarket->getFmTypeSol() != FareMarket::SOL_FM_LOCAL)
  {
    uint32_t sopIndex = ShoppingUtil::findInternalSopId(_trx, fareMarket->legIndex(), origSopId);

    ShoppingTrx::Leg& leg = _trx.legs().at(fareMarket->legIndex());
    ShoppingTrx::SchedulingOption& sop = leg.sop().at(sopIndex);

    return &sop.globalDirection();
  }

  return nullptr;
}

bool
SoloSurcharges::process(FareMarket* fareMarket,
                        SOPUsage& sopUsage,
                        SurchargesDetail& surchargesDetail,
                        uint32_t bitIndex,
                        const PaxTypeFareVec& applicableFares,
                        bool withTaxes)
{
  if (UNLIKELY(!isValid(surchargesDetail._paxTypeFare)))
    return false;

  std::unique_ptr<ScopedFMGlobalDirSetter> scopedFMGlobalDirSetter(
      createScopedFMGlobalDirSetter(fareMarket, sopUsage));

  bool validResult = true;
  TaxResponse* taxResponse(nullptr);

  if (LIKELY(sopUsage.itin_->validatingCarrier().empty()))
  {
    ValidatingCarrierUpdater vcu(_trx);
    vcu.legacyProcess(*sopUsage.itin_);
  }

  const StdVectorFlatSet<CarrierCode> carriersToProcess =
      _yqyrCalculator.determineCarriersToProcess(sopUsage.itin_,
                                                 sopUsage.itin_->validatingCarrier());
  _yqyrCalculator.initializeFareMarketCarrierStorage(
      fareMarket, carriersToProcess, applicableFares);

  if (LIKELY(sopUsage.itin_->calculationCurrency().empty()))
  {
    sopUsage.itin_->calculationCurrency() = _trx.journeyItin()->calculationCurrency();
  }

  if (LIKELY(sopUsage.itin_->originationCurrency().empty()))
  {
    sopUsage.itin_->originationCurrency() = _trx.journeyItin()->originationCurrency();
  }

  surchargesDetail._validatingCarrier = sopUsage.itin_->validatingCarrier();

  SoloFarePathWrapperSource farePathWrapperSource(
      _trx, *surchargesDetail._paxTypeFare, sopUsage, bitIndex);
  FarePathWrapper farePathWrapper(farePathWrapperSource);

  farePathWrapper.buildFarePath();

  surchargesDetail._farePath = farePathWrapper.getFarePath();

  if (UNLIKELY(!calculateSurcharges(surchargesDetail._farePath,
                           surchargesDetail._farePath->pricingUnit().front())))
    return false;

  if (withTaxes)
  {
    taxResponse = calculateYQYR(carriersToProcess,
                                farePathWrapper,
                                fareMarket,
                                surchargesDetail._paxTypeFare,
                                sopUsage.origSopId_);
    if (LIKELY(validResult &= (taxResponse != nullptr)))
    {
      surchargesDetail._taxAmount =
          getYQYR(*surchargesDetail._farePath, *surchargesDetail._paxTypeFare, *taxResponse);
      collectTaxDiagnostics(*taxResponse);
    }
  }

  if (farePathWrapper.isRoundTripEnabled())
    farePathWrapper.removeMirrorFareUsage();
  surchargesDetail._surchargesAmount = getSurcharges(surchargesDetail._farePath);

  return validResult;
}

TaxResponse*
SoloSurcharges::buildTaxResponse(FarePath& farePath)
{
  TaxResponse* taxResponse(nullptr);

  if (LIKELY(farePath.itin()->getTaxResponses().empty()))
  {
    taxResponse = _trx.dataHandle().create<TaxResponse>();
    taxResponse->paxTypeCode() = farePath.paxType()->paxType();
    taxResponse->farePath() = &farePath;
    taxResponse->diagCollector() = DCFactory::instance()->create(_trx);
  }
  else
  {
    taxResponse = farePath.itin()->getTaxResponses().front();
    taxResponse->taxItemVector().clear();
  }

  return taxResponse;
}

bool
SoloSurcharges::isValid(PaxTypeFare* paxTypeFare)
{
  return paxTypeFare->isValid() && !paxTypeFare->isFlightBitmapInvalid();
}

bool
SoloSurcharges::calculateSurcharges(FarePath* farePath, PricingUnit* pricingUnit)
{
  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SURCHARGE_RULE); // category 12
  PricingUnitRuleController rc(PURuleValidation, categories);

  return rc.validate(_trx, *farePath, *pricingUnit);
}

MoneyAmount
SoloSurcharges::getSurcharges(FarePath* farePath)
{
  MoneyAmount surchargesBefore = farePath->plusUpAmount();
  RuleUtil::getSurcharges(_trx, *farePath);
  return farePath->plusUpAmount() - surchargesBefore;
}

TaxResponse*
SoloSurcharges::calculateYQYR(const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                              FarePathWrapper& farePathWrapper,
                              FareMarket* fareMarket,
                              const PaxTypeFare* paxTypeFare,
                              const uint16_t sopId)
{
  FarePath* const farePath = farePathWrapper.getFarePath();

  const CarrierCode validatingCarrier(farePath->itin()->validatingCarrier());
  if (UNLIKELY(_yqyrCalculator.isFilteredOutByCarrierDiag(validatingCarrier)))
    return nullptr;

  TaxResponse* taxResponse = buildTaxResponse(*farePath);
  if (UNLIKELY(!taxResponse))
    return nullptr;

  YQYRCalculator::YQYRFeesApplicationVec fees;

  // The mirror fare usage causes more problems for YQYR calculator than it is worth.
  if (farePathWrapper.isRoundTripEnabled())
    farePathWrapper.removeMirrorFareUsage();

  _yqyrCalculator.calculateYQYRs(
      validatingCarrier, carriersToProcess, paxTypeFare, fareMarket, farePath, sopId, fees);

  YQYR::ServiceFee serviceFee;
  serviceFee.applyCharges(_trx, *taxResponse, fees);

  return taxResponse;
}

MoneyAmount
SoloSurcharges::getYQYR(FarePath& farePath, PaxTypeFare& paxTypeFare, TaxResponse& taxResponse)
{
  bool noBookingCodeTax = false;
  BookingCode bkCode1 = NULL_CODE;
  BookingCode bkCode2 = NULL_CODE;
  BookingCode bkCode3 = NULL_CODE;

  return PricingUtil::readTaxAndConvertCurrency(_trx,
                                                &taxResponse,
                                                *farePath.itin(),
                                                paxTypeFare,
                                                *(farePath.pricingUnit().front()),
                                                noBookingCodeTax,
                                                bkCode1,
                                                bkCode2,
                                                bkCode3,
                                                *taxResponse.diagCollector());
}

ScopedFMGlobalDirSetter*
SoloSurcharges::createScopedFMGlobalDirSetter(FareMarket* fareMarket, const SOPUsage& sopUsage)
{
  return new ScopedFMGlobalDirSetter(
      &_trx, fareMarket, getGlobalDirection(fareMarket, sopUsage.origSopId_));
}

const SoloSurcharges::SurchargesDetailsMap&
SoloSurcharges::surchargesDetailsMap() const
{
  return _surchargesDetailsMap;
}

bool
SoloSurcharges::preparePTFForValidation(const uint32_t carrierKey, PaxTypeFare& ptf)
{
  if (UNLIKELY(!ptf.isValid()))
    return false;

  constexpr bool skippedAsInvalid = true;
  ptf.setComponentValidationForCarrier(carrierKey, _trx.isAltDates(), ptf.getDurationUsedInFVO());

  if (!ptf.isFlightBitmapInvalid(skippedAsInvalid))
    return true;

  if (!_trx.isAltDates())
    return false;

  for (const auto& duration : _trx.durationAltDatePairs())
  {
    if (duration.first == ptf.getDurationUsedInFVO())
      continue;
    ptf.setComponentValidationForCarrier(carrierKey, true, duration.first);
    if (!ptf.isFlightBitmapInvalid(skippedAsInvalid))
      return true;
  }

  return false;
}
}
