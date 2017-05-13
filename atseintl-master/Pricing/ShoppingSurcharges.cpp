// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
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
#include "Pricing/ShoppingSurcharges.h"

#include "Common/Assert.h"
#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/Config/DynamicConfigurableNumber.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Fares/FareController.h"
#include "Fares/FareValidatorOrchestratorShopping.h"
#include "Pricing/FarePathWrapper.h"
#include "Pricing/PricingUtil.h"
#include "Rules/RuleUtil.h"
#include "Taxes/LegacyTaxes/ServiceFeeYQ.h"
#include "Util/FlatSet.h"

#include <algorithm>
#include <iterator>

#include <boost/range/adaptor/indexed.hpp>

namespace tse
{

namespace
{
Logger
logger("atseintl.Pricing.ShoppingSurcharges");

DynamicConfigurableFlagOn
enabled("SHOPPING_OPT", "IS_SURCHARGES", false);
DynamicConfigurableFlagOn
asoEnabled("SHOPPING_OPT", "IS_SURCHARGES_ASO", true);
DynamicConfigurableNumber
cat12Fares("SHOPPING_OPT", "IS_SURCHARGES_CAT12_FARES", 10);
DynamicConfigurableFlagOn
cat12QuickGet("SHOPPING_OPT", "IS_SURCHARGES_CAT12_QUICK_GET", false);
DynamicConfigurableNumber
yqyrFares("SHOPPING_OPT", "IS_SURCHARGES_YQYR_FARES", 10);

bool
hasAsoLegs(ShoppingTrx& trx)
{
  return std::any_of(trx.legs().begin(), trx.legs().end(), [](ShoppingTrx::Leg& leg)
  {
    return leg.stopOverLegFlag();
  });
}

bool
isEnabled(ShoppingTrx& trx)
{
  if (!enabled.isValid(&trx))
    return false;

  if (!asoEnabled.isValid(&trx) && hasAsoLegs(trx))
    return false;

  // TODO add alt dates support
  if (trx.isAltDates())
    return false;

  return true;
}

bool
isTaxDiagEnabled(DiagnosticTypes diagType)
{
  switch (diagType)
  {
  case AllPassTaxDiagnostic281: // 801
  case LegacyTaxDiagnostic24: // 804
    return true;

  default:
    return false;
  }
}
}

ShoppingSurcharges::ShoppingSurcharges(ShoppingTrx& trx)
  : _trx(trx), _yqyrCalculator(trx), _enabled(isEnabled(trx))
{
  if (!_enabled)
    return;

  initTaxDiagnostics();

  _yqyrCalculator.init();

  _cat12QuickGet = cat12QuickGet.isValid(&trx);

  const uint32_t fvoMaxFares = FareValidatorOrchestratorShopping::numberOfFaresToProcess.getValue();
  _cat12Fares = std::min(fvoMaxFares, cat12Fares.getValue(&trx));
  _yqyrFares = std::min(_cat12Fares, yqyrFares.getValue(&trx));
}

ShoppingSurchargesCortegeContext::ShoppingSurchargesCortegeContext(FareMarket& fareMarket,
                                                                   PaxTypeBucket& cortege,
                                                                   ShoppingTrx::Leg& leg,
                                                                   uint32_t legIdx)
  : fareMarket(fareMarket),
    cortege(cortege),
    index(leg.carrierIndex()),
    aso(leg.stopOverLegFlag()),
    legIndex(legIdx),
    legsBegin(aso ? &*leg.jumpedLegIndices().begin() : &legIndex),
    legsEnd(aso ? &*leg.jumpedLegIndices().end() : &legIndex + 1)
{
  ShoppingUtil::createKey(fareMarket.governingCarrier(), key);
}

void
ShoppingSurcharges::initTaxDiagnostics()
{
  const DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  if (!isTaxDiagEnabled(diagType))
    return;

  DCFactory* factory = DCFactory::instance();
  _dc = factory->create(_trx);
  _dc->trx() = &_trx;
  _dc->enable(diagType);
}

void
ShoppingSurcharges::flushTaxDiagnosticsMsg()
{
  if (_dc)
  {
    _dc->flushMsg();
  }
}

void
ShoppingSurcharges::collectTaxDiagnostics(TaxResponse& taxResponse)
{
  if (_dc)
  {
    if (_trx.diagnostic().diagnosticType() == AllPassTaxDiagnostic281 ||
        _trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24)
    {
      *_dc << taxResponse;
    }
  }
}

void
ShoppingSurcharges::process()
{
  if (!_enabled)
    return;

  const std::vector<CarrierCode> carriers = getCarrierList();
  for (CarrierCode carrier : carriers)
    precalculateTaxesOnCarrier(carrier);
}

std::vector<CarrierCode>
ShoppingSurcharges::getCarrierList()
{
  StdVectorFlatSet<CarrierCode> list;

  for (ShoppingTrx::Leg& leg : _trx.legs())
  {
    ItinIndex& index = leg.carrierIndex();

    for (const auto& m : index.root())
    {
      const ItinIndex::Key cxrKey = m.first;

      ItinIndex::ItinCell* const cell =
          ShoppingUtil::retrieveDirectItin(index, cxrKey, ItinIndex::CHECK_NOTHING);

      TSE_ASSERT(cell);
      Itin* const directItin = cell->second;

      TSE_ASSERT(directItin);
      if (directItin->fareMarket().empty())
        continue;

      list.insert(directItin->fareMarket().front()->governingCarrier());
    }
  }

  return list.steal_container();
}

void
ShoppingSurcharges::precalculateTaxesOnCarrier(CarrierCode carrier)
{
  for (auto l : boost::adaptors::index(_trx.legs(), 0))
  {
    const uint32_t legIndex = l.index();
    ShoppingTrx::Leg& leg = l.value();

    ItinIndex& index = leg.carrierIndex();

    for (const auto& m : index.root())
    {
      const ItinIndex::Key cxrKey = m.first;

      ItinIndex::ItinCell* const cell =
          ShoppingUtil::retrieveDirectItin(index, cxrKey, ItinIndex::CHECK_NOTHING);

      TSE_ASSERT(cell);
      Itin* const directItin = cell->second;

      TSE_ASSERT(directItin);
      if (directItin->fareMarket().empty())
        continue;

      if (directItin->fareMarket().front()->governingCarrier() != carrier)
        continue;

      for (FareMarket* fareMarket : directItin->fareMarket())
      {
        if (fareMarket->failCode() != ErrorResponseException::NO_ERROR)
          continue;

        bool anyProcessed = false;
        for (PaxTypeBucket& cortege : fareMarket->paxTypeCortege())
        {
          ShoppingSurchargesCortegeContext ctx(*fareMarket, cortege, leg, legIndex);
          anyProcessed |= precalculateTaxesOnCortege(ctx);
        }

        if (anyProcessed)
          sortFares(*fareMarket);
      }
    }
  }
}

bool
ShoppingSurcharges::precalculateTaxesOnCortege(ShoppingSurchargesCortegeContext& ctx)
{
  using Type = PrecalculatedTaxes::Type;

  CxrPrecalculatedTaxes& cxrTaxes = ctx.cortege.mutableCxrPrecalculatedTaxes();
  PrecalculatedTaxes& taxes = cxrTaxes[""];

  const std::vector<PaxTypeFare*> applicableFares = getApplicableFares(ctx);

  uint32_t cat12Remaining = _cat12Fares;
  uint32_t yqyrRemaining = _yqyrFares;

  const MoneyAmount invalidAmount = -std::numeric_limits<MoneyAmount>::infinity();

  MoneyAmount cat12Representative = invalidAmount, yqyrRepresentative = invalidAmount;

  YQYR::V2ISCortegeCalculator cortegeCalculator(_yqyrCalculator, ctx);

  for (PaxTypeFare* fare : applicableFares)
  {
    YQYR::V2ISCortegeCalculator* calculator = nullptr;
    if (yqyrRemaining > 0)
      calculator = &cortegeCalculator;

    PrecalculatedTaxesAmount amounts;
    precalculateTaxes(ctx, calculator, applicableFares, *fare, &amounts);

    if (!amounts.has(Type::CAT12))
      continue;

    --cat12Remaining;
    cat12Representative = std::max(cat12Representative, amounts.amount(Type::CAT12));

    if (amounts.has(Type::YQYR))
    {
      --yqyrRemaining;
      yqyrRepresentative = std::max(yqyrRepresentative, amounts.amount(Type::YQYR));
    }

    taxes.setAmounts(*fare, amounts);

    if (cat12Remaining == 0)
      break;
  }

  if (cat12Representative != invalidAmount)
  {
    taxes.setDefaultAmount(Type::CAT12, cat12Representative);
    taxes.setProcessed(Type::CAT12);
  }

  if (yqyrRepresentative != invalidAmount)
  {
    taxes.setDefaultAmount(Type::YQYR, yqyrRepresentative);
    taxes.setProcessed(Type::YQYR);
  }

  // Won't be used anymore.
  ctx.cortege.clearYqyrCarrierStorage();

  cxrTaxes.processLowerBoundAmounts();
  return taxes.isProcessed(Type::CAT12) || taxes.isProcessed(Type::YQYR);
}

std::vector<PaxTypeFare*>
ShoppingSurcharges::getApplicableFares(ShoppingSurchargesCortegeContext& ctx)
{
  std::vector<PaxTypeFare*> result;
  result.reserve(_cat12Fares);

  for (PaxTypeFare* fare : ctx.cortege.paxTypeFare())
  {
    if (result.size() >= _cat12Fares)
      break;

    if (!fare->isValid() || fare->flightBitmap().empty() || fare->isFlightBitmapInvalid(true))
      continue;

    result.push_back(fare);
  }

  return result;
}

void
ShoppingSurcharges::precalculateTaxes(ShoppingSurchargesCortegeContext& ctx,
                                      YQYR::V2ISCortegeCalculator* calculator,
                                      const std::vector<PaxTypeFare*>& applicableFares,
                                      PaxTypeFare& fare,
                                      PrecalculatedTaxesAmount* amounts)
{
  using Type = PrecalculatedTaxes::Type;

  auto it = ctx.aso ? ctx.index.beginAcrossStopOverRow(_trx, ctx.legIndex, ctx.key)
                    : ctx.index.beginRow(ctx.key);

  const uint32_t bitmapSize = fare.getFlightBitmapSize();
  for (uint32_t bitIndex = 0; bitIndex < bitmapSize; ++bitIndex, ++it)
  {
    if (fare.isFlightInvalid(bitIndex))
      continue;

    ShoppingFarePathWrapperSource source(_trx, fare, ctx.cortege.requestedPaxType(), bitIndex);
    source.initWithCell(*it);

    FarePathWrapper wrapper(source);
    wrapper.buildFarePath();
    FarePath& farePath = *wrapper.getFarePath();

    if (!calculateCAT12(farePath))
      continue;

    if (calculator)
    {
      StdVectorFlatSet<CarrierCode> carriersToProcess = _yqyrCalculator.determineCarriersToProcess(
          farePath.itin(), farePath.itin()->validatingCarrier());

      calculator->initializeCortegeCarrierStorage(carriersToProcess, applicableFares);

      TaxResponse* taxResponse = calculateYQYR(*calculator, it, carriersToProcess, farePath, fare);
      if (taxResponse)
      {
        amounts->setAmount(Type::YQYR, getYQYR(farePath, fare, *taxResponse));
        collectTaxDiagnostics(*taxResponse);
      }
    }

    if (wrapper.isRoundTripEnabled())
      wrapper.removeMirrorFareUsage();
    amounts->setAmount(Type::CAT12, getAmountOfCAT12(farePath));

    return;
  }
}

void
ShoppingSurcharges::sortFares(FareMarket& fareMarket)
{
  FareController fareController(_trx, *_trx.journeyItin(), fareMarket);
  fareController.sortPaxTypeFares();
}

bool
ShoppingSurcharges::calculateCAT12(FarePath& farePath)
{
  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SURCHARGE_RULE); // category 12
  PricingUnitRuleController rc(PURuleValidation, categories);

  return rc.validate(_trx, farePath, *farePath.pricingUnit().front());
}

MoneyAmount
ShoppingSurcharges::getAmountOfCAT12(FarePath& farePath)
{
  if (_cat12QuickGet)
  {
    return PricingUtil::readSurchargeAndConvertCurrency(
        _trx, *farePath.itin(), *farePath.pricingUnit().front()->fareUsage().front());
  }
  else
  {
    const MoneyAmount surchargesBefore = farePath.plusUpAmount();
    RuleUtil::getSurcharges(_trx, farePath);
    return farePath.plusUpAmount() - surchargesBefore;
  }
}

TaxResponse*
ShoppingSurcharges::buildTaxResponse(FarePath& farePath)
{
  TaxResponse* taxResponse = nullptr;

  if (farePath.itin()->getTaxResponses().empty())
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

TaxResponse*
ShoppingSurcharges::calculateYQYR(YQYR::V2ISCortegeCalculator& calculator,
                                  const ItinIndex::ItinIndexIterator& cellIterator,
                                  const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                                  FarePath& farePath,
                                  const PaxTypeFare& paxTypeFare)
{
  const CarrierCode validatingCarrier = farePath.itin()->validatingCarrier();
  if (_yqyrCalculator.isFilteredOutByCarrierDiag(validatingCarrier))
    return nullptr;

  TaxResponse* taxResponse = buildTaxResponse(farePath);
  if (!taxResponse)
    return nullptr;

  YQYRCalculator::YQYRFeesApplicationVec fees;
  calculator.determineFurthestSeg(cellIterator);
  calculator.calculateYQYRs(validatingCarrier, carriersToProcess, paxTypeFare, farePath, fees);

  YQYR::ServiceFee serviceFee;
  serviceFee.applyCharges(_trx, *taxResponse, fees);

  return taxResponse;
}

MoneyAmount
ShoppingSurcharges::getYQYR(FarePath& farePath, PaxTypeFare& paxTypeFare, TaxResponse& taxResponse)
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
}
