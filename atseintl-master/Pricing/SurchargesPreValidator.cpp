//-------------------------------------------------------------------
//
//  File:        SurchargesPreValidator.cpp
//  Created:     Nov 14, 2011
//  Authors:     Grzegorz Fortuna
//
//  Description: Fare-Led MIP Surcharges
//
//  Updates:
//          Mar, 2013 Marek Markiewicz - restored and renamed the file
//                                         made it work for regular MIP
//
//  Copyright Sabre 2011
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

#include "Pricing/SurchargesPreValidator.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/FarePathWrapper.h"
#include "Pricing/PricingUtil.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Taxes/LegacyTaxes/ServiceFeeYQ.h"
#include "Taxes/Pfc/PfcItem.h"

#include <algorithm>
#include <vector>

namespace tse
{

Logger
SurchargesPreValidator::_logger("atseintl.SurchargesPreValidator");

namespace
{
struct TravelSegmentFinder
{
  TravelSegmentFinder(int16_t legId,
                      const LocCode& origAirport,
                      const LocCode& destAirport = LocCode())
    : _legId(legId), _origAirport(origAirport), _destAirport(destAirport)
  {
  }

  bool operator()(const TravelSeg* travelSeg) const
  {
    if (_destAirport.empty())
    {
      return _legId == travelSeg->legId() && _origAirport == travelSeg->origAirport();
    }
    else
    {
      return _legId == travelSeg->legId() && _origAirport == travelSeg->origAirport() &&
             _destAirport == travelSeg->destAirport();
    }
  }

private:
  int16_t _legId;
  LocCode _origAirport;
  LocCode _destAirport;
};

void
flushTaxDiagnostic(PricingTrx& trx, DiagCollector& diag, TaxResponse* taxResponse)
{
  if (taxResponse == nullptr)
    return;

  if (trx.diagnostic().diagnosticType() == AllPassTaxDiagnostic281 ||
      trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24)
  {
    diag << *taxResponse;
  }

  if (taxResponse->diagCollector() != nullptr && taxResponse->diagCollector()->isActive())
    taxResponse->diagCollector()->flushMsg();
}

} // unnamed namespace

SurchargesPreValidator::SurchargesPreValidator(PricingTrx& trx,
                                               const FactoriesConfig& factoriesConfig,
                                               const Itin& itin,
                                               DiagCollector* diag,
                                               size_t numFaresForCAT12Estimation)
  : _trx(trx),
    _factoriesConfig(factoriesConfig),
    _itin(itin),
    _diag(diag),
    _numFaresForCAT12Estimation(numFaresForCAT12Estimation)
{
  if (!_numFaresForCAT12Estimation)
    _numFaresForCAT12Estimation = 1;
}

void
SurchargesPreValidator::process(FareMarket& fareMarket)
{
  for (PaxTypeBucket& cortege : fareMarket.paxTypeCortege())
    processCortege(fareMarket, cortege);
}

void
SurchargesPreValidator::processDefaultAmount(PrecalculatedTaxes& taxes,
                                             PrecalculatedTaxes::Type type)
{
  class SurAndTaxComparator
  {
    PrecalculatedTaxes::Type type;

  public:
    SurAndTaxComparator(PrecalculatedTaxes::Type type) : type(type) {}

    typedef PrecalculatedTaxes::FareToAmountMap::value_type PtfMoney;
    bool operator()(const PtfMoney& left, const PtfMoney& right)
    {
      return (left.second.amount(type) < right.second.amount(type));
    }
  };

  const auto maxElementIt = std::max_element(taxes.begin(), taxes.end(), SurAndTaxComparator(type));
  if (maxElementIt != taxes.end() && maxElementIt->second.has(type))
    taxes.setDefaultAmount(type, maxElementIt->second.amount(type));
}

bool
SurchargesPreValidator::getPtfSurcharges(PaxTypeFare& paxTypeFare,
                                         const FarePathWrapper& farePathWrapper,
                                         PrecalculatedTaxes& taxes)
{
  const auto result = calculatePtfSurcharges(paxTypeFare, farePathWrapper);
  if (UNLIKELY(!result))
    return false;

  taxes.setAmount(PrecalculatedTaxesAmount::CAT12, paxTypeFare, *result);
  return true;
}

bool
SurchargesPreValidator::getPtfYqyr(PaxTypeFare& paxTypeFare,
                                   FarePath& farePath,
                                   PrecalculatedTaxes& taxes)
{
  const auto result = calculatePtfYqyr(paxTypeFare, farePath);
  if (!result)
    return false;

  taxes.setAmount(PrecalculatedTaxesAmount::YQYR, paxTypeFare, *result);
  return true;
}

bool
SurchargesPreValidator::getPtfXFTax(PaxTypeFare& paxTypeFare,
                                    FarePath& farePath,
                                    PrecalculatedTaxes& taxes)
{
  const auto result = calculatePtfXFTax(paxTypeFare, farePath);
  if (!result)
    return false;

  taxes.setDefaultAmount(PrecalculatedTaxesAmount::XF, *result);
  return true;
}

boost::optional<MoneyAmount>
SurchargesPreValidator::calculatePtfSurcharges(PaxTypeFare& paxTypeFare,
                                               const FarePathWrapper& farePathWrapper)
{
  FarePath& farePath = *farePathWrapper.getFarePath();
  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SURCHARGE_RULE); /* category 12 */
  RuleControllerWithChancelor<PricingUnitRuleController> rc(PURuleValidation, categories);

  PricingUnit* firstPU = farePath.pricingUnit().front();

  if (UNLIKELY(!rc.validate(_trx, farePath, *firstPU)))
    return boost::none;

  if (farePathWrapper.isRoundTripEnabled())
    farePathWrapper.removeMirrorFareUsage();

  return PricingUtil::readSurchargeAndConvertCurrency(
      _trx, *(farePath.itin()), *(firstPU->fareUsage().front()));
}

boost::optional<MoneyAmount>
SurchargesPreValidator::calculatePtfYqyr(PaxTypeFare& paxTypeFare, FarePath& farePath)
{
  TSELatencyData metrics(_trx, "CALCULATE YQYR");

  TaxResponse* taxResponse = buildTaxResponse(farePath);
  if (!taxResponse)
    return boost::none;

  tse::YQYR::ServiceFee serviceFee;
  serviceFee.collectFees(_trx, *taxResponse);

  bool noBookingCodeTax = false;
  BookingCode bkCode1 = NULL_CODE;
  BookingCode bkCode2 = NULL_CODE;
  BookingCode bkCode3 = NULL_CODE;

  const MoneyAmount amount =
      PricingUtil::readTaxAndConvertCurrency(_trx,
                                             taxResponse,
                                             *(farePath.itin()),
                                             paxTypeFare,
                                             *(farePath.pricingUnit().front()),
                                             noBookingCodeTax,
                                             bkCode1,
                                             bkCode2,
                                             bkCode3,
                                             *(taxResponse->diagCollector()));

  flushTaxDiagnostic(_trx, *_diag, taxResponse);
  return amount;
}

boost::optional<MoneyAmount>
SurchargesPreValidator::calculatePtfXFTax(PaxTypeFare& paxTypeFare, FarePath& farePath)
{
  TSELatencyData metrics(_trx, "CALCULATE XF TAX");

  TaxResponse* taxResponse = buildTaxResponse(farePath);
  if (!taxResponse)
    return boost::none;

  PfcItem pfcItem;
  pfcItem.build(_trx, *taxResponse);

  TSE_ASSERT(!farePath.pricingUnit().empty());
  const PricingUnit& pricingUnit = *farePath.pricingUnit().front();
  const std::vector<TravelSeg*>& travelSegments = pricingUnit.travelSeg();
  const CurrencyCode& farePathCurrency = farePath.itin()->calculationCurrency();

  MoneyAmount xfTaxAmount = 0.0;
  for (const PfcItem* pfcItem : taxResponse->pfcItemVector())
  {
    TravelSegmentFinder finder(pfcItem->legId(), pfcItem->pfcAirportCode());
    std::vector<TravelSeg*>::const_iterator travelSegIt =
        std::find_if(travelSegments.begin(), travelSegments.end(), finder);
    if (travelSegIt != travelSegments.end())
    {
      const MoneyAmount& pfcAmount = PricingUtil::convertCurrency(
          _trx, pfcItem->pfcAmount(), farePathCurrency, pfcItem->pfcCurrencyCode());
      xfTaxAmount += pfcAmount;
    }
  }

  return xfTaxAmount;
}

void
SurchargesPreValidator::processCortege(FareMarket& fareMarket, PaxTypeBucket& cortege)
{
  CxrPrecalculatedTaxes& taxes = cortege.mutableCxrPrecalculatedTaxes();
  if (!taxes.empty() && taxes.begin()->second.isProcessed(PrecalculatedTaxesAmount::CAT12))
    return; // we already calculated surcharges in this FareMarket

  std::vector<CarrierCode> fmValCxrs = fareMarket.validatingCarriers();
  if (fmValCxrs.empty())
    fmValCxrs.push_back(""); // Happens when GSA is disabled

  for (CarrierCode valCxr : fmValCxrs)
  {
    size_t numberOfFaresProcessed = 0;
    for (PaxTypeFare* paxTypeFare : cortege.paxTypeFare())
    {
      if (numberOfFaresProcessed == _numFaresForCAT12Estimation)
        break;

      if (!paxTypeFare->validatingCarriers().empty() &&
          std::find(paxTypeFare->validatingCarriers().begin(),
                    paxTypeFare->validatingCarriers().end(),
                    valCxr) ==  paxTypeFare->validatingCarriers().end())
      {
        continue; // This validating carrier is invalid for the ptf
      }

      if (processPtf(*paxTypeFare, cortege.requestedPaxType(), taxes[valCxr], valCxr))
        ++numberOfFaresProcessed;
    }
  }

  // get max surcharge and tax
  for (CarrierCode valCxr : fmValCxrs)
  {
    PrecalculatedTaxes& precalculatedTaxes = taxes[valCxr];

    processDefaultAmount(precalculatedTaxes, PrecalculatedTaxesAmount::CAT12);
    processDefaultAmount(precalculatedTaxes, PrecalculatedTaxesAmount::YQYR);

    precalculatedTaxes.setProcessed(PrecalculatedTaxesAmount::CAT12);
  }

  taxes.processLowerBoundAmounts();
}

bool
SurchargesPreValidator::processPtf(PaxTypeFare& paxTypeFare,
                                   PaxType* paxType,
                                   PrecalculatedTaxes& taxes,
                                   const CarrierCode& valCxr)
{
  if (!paxTypeFare.isValid())
    return false;

  AltDatesFarePathWrapperSource source(_trx, paxTypeFare, _itin, paxType);

  // Remove tse:: prefix when removing the fallback.
  tse::FarePathWrapper farePathWrapper(source);
  farePathWrapper.buildFarePath();
  FarePath& farePath = *farePathWrapper.getFarePath();
  if (!valCxr.empty())
    farePath.validatingCarriers().assign(1, valCxr);

  if (UNLIKELY(!getPtfSurcharges(paxTypeFare, farePathWrapper, taxes)))
    return false;

  if (LIKELY(!TrxUtil::isTotalPriceEnabled(_trx)))
  {
    if (!taxes.isProcessed(PrecalculatedTaxesAmount::YQYR))
    {
      if (!getPtfYqyr(paxTypeFare, farePath, taxes))
        return false;

      taxes.setProcessed(PrecalculatedTaxesAmount::YQYR);
    }
  }

  if (!taxes.isProcessed(PrecalculatedTaxesAmount::XF))
  {
    if (!getPtfXFTax(paxTypeFare, farePath, taxes))
      return false;

    taxes.setProcessed(PrecalculatedTaxesAmount::XF);
  }

  return true;
}

TaxResponse*
SurchargesPreValidator::buildTaxResponse(FarePath& farePath)
{
  TaxResponse* taxResponse = nullptr;
  if (farePath.itin()->getTaxResponses().empty())
  {
    _trx.dataHandle().get(taxResponse);
    if (taxResponse == nullptr)
      return nullptr;

    taxResponse->paxTypeCode() = farePath.paxType()->paxType();
    taxResponse->farePath() = &farePath;
    taxResponse->diagCollector() = DCFactory::instance()->create(_trx);
    return taxResponse;
  }

  taxResponse = farePath.itin()->getTaxResponses().front();
  taxResponse->taxItemVector().clear();

  return taxResponse;
}

} // tse
