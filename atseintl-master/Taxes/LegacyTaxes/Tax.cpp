// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/Tax.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxExemption.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "Diagnostic/DiagManager.h"
#include "PfcTaxesExemption/AutomaticPfcTaxExemption.h"
#include "Rules/RuleUtil.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/AdjustTax.h"
#include "Taxes/LegacyTaxes/CabinValidator.h"
#include "Taxes/LegacyTaxes/CarrierValidator.h"
#include "Taxes/LegacyTaxes/EquipmentValidator.h"
#include "Taxes/LegacyTaxes/FareClassValidator.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/LegacyTaxes/TaxOnChangeFee.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/TaxRange.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
Tax::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.Tax"));

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(markupAnyFareOptimization);
FALLBACK_DECL(ssdsp1511fix);
FALLBACK_DECL(fallbackChangeSpecialDomesticRoundingLogic);
FALLBACK_DECL(taxProcessExemptionTable);
FALLBACK_DECL(taxShowSomeExemptions);
FALLBACK_DECL(taxFareWithoutBase);
FALLBACK_DECL(roundTaxToZero);

bool
convertMoney(const PricingTrx& trx, const Money& source, Money& target, BSRCollectionResults& bsrResults)
{
  return CurrencyConversionFacade().convert(
      target,
      source,
      trx,
      false,
      CurrencyConversionRequest::TAXES,
      false,
      &bsrResults);
}
}

void
Tax::getTaxSpecConfig(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  if (!taxCodeReg.specConfigName().empty())
    _taxSpecConfig = &trx.dataHandle().getTaxSpecConfig(taxCodeReg.specConfigName());
  else
    _taxSpecConfig = nullptr;
}

void
Tax::caclulateFurthesFareBreak(PricingTrx& trx,
                               TaxResponse& taxResponse)
{
  Diagnostic& diag = trx.diagnostic();
  const Loc* locStart = taxResponse.farePath()->itin()->travelSeg().front()->origin();

  struct
  {
    uint32_t _maxMiles = 0;
    int16_t _noSeg = -1;
  } maxDistance;

  for(const PricingUnit* pricingUnit: taxResponse.farePath()->pricingUnit())
    for(const FareUsage* fareUsage: pricingUnit->fareUsage())
    {
      uint32_t unMiles = taxUtil::calculateMiles(trx, taxResponse,
                    *locStart, *fareUsage->travelSeg().back()->destination(),
                    taxResponse.farePath()->itin()->travelSeg());

      if (diag.diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream << "FARE " << fareUsage->travelSeg().front()->origin()->loc().c_str()
               << fareUsage->travelSeg().back()->destination()->loc().c_str()
               << " DISTANCE " << unMiles << std::endl;
        diag.insertDiagMsg(stream.str());
      }

      if (unMiles > maxDistance._maxMiles)
      {
        maxDistance._maxMiles = unMiles;
        maxDistance._noSeg = taxResponse.farePath()->itin()->segmentOrder(
          fareUsage->travelSeg().back());
      }
    }

  _furthestFareBreakSegment = maxDistance._noSeg;
}


void
Tax::preparePortionOfTravelIndexes(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg)
{
  _furthestFareBreakSegment = -1;

  if (utc::furthestFareBreak1DayRt(trx, taxCodeReg))
  {
    Diagnostic& diag = trx.diagnostic();
    const Loc* locStart = taxResponse.farePath()->itin()->travelSeg().front()->origin();
    if (diag.diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream << "***LOOKING FOR FURTHEST FAREBREAKE - START***" << std::endl
             << "START POINT " << locStart->loc() << " SEQ " << taxCodeReg.seqNo() << std::endl;
      diag.insertDiagMsg(stream.str());
    }

    //not roundtrip
    if ( !LocUtil::isSameCity(locStart->loc(),
        taxResponse.farePath()->itin()->travelSeg().back()->destination()->loc(), trx.dataHandle()))
    {
      if (diag.diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream << "RETURN TO DIFFERENT CITY - "
               << taxResponse.farePath()->itin()->travelSeg().back()->destination()->loc()
               << std::endl << "***LOOKING FOR FURTHEST FAREBREAKE - END***" << std::endl;
        diag.insertDiagMsg(stream.str());
      }
      return;
    }

    //not within the same day
    if (taxResponse.farePath()->itin()->travelSeg().front()->departureDT().day() !=
        taxResponse.farePath()->itin()->travelSeg().back()->departureDT().day())
    {
      if (diag.diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream << "TRAVEL EXCEED ONE DAY"
               << std::endl << "***LOOKING FOR FURTHEST FAREBREAKE - END***" << std::endl;
        diag.insertDiagMsg(stream.str());
      }
      return;
    }

    int16_t fareCount = accumulate(taxResponse.farePath()->pricingUnit().begin(),
             taxResponse.farePath()->pricingUnit().end(), 0,
             [](int total, const PricingUnit* pricingUnit)
               { return total + pricingUnit->fareUsage().size(); }
             );

    if (fareCount < 2)
    {
      if (diag.diagnosticType() == Diagnostic818)
      {
        std::ostringstream stream;
        stream << "ONLY " << fareCount << " FARES"
               << std::endl << "***LOOKING FOR FURTHEST FAREBREAKE - END***" << std::endl;
        diag.insertDiagMsg(stream.str());
      }
      return;
    }

    MirrorImage mirrorImage(trx.getRequest()->ticketingDT(), _taxSpecConfig);
    for (uint16_t segIndx=0; segIndx<taxResponse.farePath()->itin()->travelSeg().size(); segIndx++)
      if (mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, segIndx))
      {
        if (diag.diagnosticType() == Diagnostic818)
        {
          std::ostringstream stream;
          stream << "MIRROR IMAGE RECOGNIZED"
                 << std::endl << "***LOOKING FOR FURTHEST FAREBREAKE - END***" << std::endl;
          diag.insertDiagMsg(stream.str());
        }
        return;
      }

    caclulateFurthesFareBreak(trx, taxResponse);

    if (diag.diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream << "FAREBREAK SEGMENT " << _furthestFareBreakSegment
             << std::endl << "***LOOKING FOR FURTHEST FAREBREAKE - END***" << std::endl;
      diag.insertDiagMsg(stream.str());
    }
  }
}

bool
Tax::validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  TaxCodeValidator taxCodeValidator;

  return taxCodeValidator.validateTaxCode(trx, taxResponse, taxCodeReg);
}

bool
Tax::validateZZTax(PricingTrx& trx, const CountrySettlementPlanInfo* cspi)
{
  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    return ((trx.getRequest()->getSettlementMethod() == "TCH" ||
             (cspi && cspi->getSettlementPlanTypeCode() == "TCH")) &&
            TrxUtil::isTCHAllowed(trx));
  else
    return (trx.getRequest()->getSettlementMethod() == "TCH" &&
        TrxUtil::isTCHAllowed(trx));
}

bool
Tax::validateLocRestrictions(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)

{
  if (handleHiddenPoints())
    return true;

  LocRestrictionValidator locRestrictionValidator;

  return locRestrictionValidator.validateLocation(
      trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
Tax::validateTripTypes(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t& startIndex,
                       uint16_t& endIndex)

{
  if (handleHiddenPoints() != HIDDEN_POINT_NOT_HANDLED)
    return validateFromToWithHiddenPoints(trx, taxResponse, taxCodeReg, startIndex, endIndex);

  TripTypesValidator tripTypesValidator;

  return tripTypesValidator.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
Tax::validateGeoSpecLoc1(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)

{
  return validateTransferTypeLoc1(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
Tax::validateTransferTypeLoc1(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t startIndex,
                              uint16_t endIndex)
{
  if (!_taxSpecConfig)
    return true;

  utc::Loc1TransferType loc1TransferType(trx, taxSpecConfig());

  if (loc1TransferType.isEquipmentToFlt())
  {
    return loc1TransferType.validatePrevFltEquip(trx, taxCodeReg, taxResponse, startIndex);
  }
  else if (loc1TransferType.isFltToFlt())
  {
    return loc1TransferType.validatePrevFltLocs(trx, taxCodeReg, taxResponse, startIndex);
  }

  return true;
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
Tax::validateRange(PricingTrx& trx,
                   TaxResponse& taxResponse,
                   TaxCodeReg& taxCodeReg,
                   uint16_t& startIndex,
                   uint16_t& endIndex)

{
  TaxRange taxRange;

  return taxRange.validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
Tax::validateTransit(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegIndex)
{
  if (handleHiddenPoints() != HIDDEN_POINT_NOT_HANDLED)
  {
    if (taxCodeReg.restrictionTransit().empty())
      return true;

    if (validateTransitOnHiddenPoints(taxCodeReg))
      return taxCodeReg.restrictionTransit().front().transitTaxonly();
  }

  MirrorImage mirrorImage(trx.getRequest()->ticketingDT(), _taxSpecConfig);

  bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);

  if (travelSegIndex == _furthestFareBreakSegment)
    stopOver = true;

  TransitValidator transitValidator;

  return transitValidator.validateTransitRestriction(
      trx, taxResponse, taxCodeReg, travelSegIndex, stopOver, _landToAirStopover);
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
Tax::validateCarrierExemption(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t travelSegIndex)
{
  CarrierValidator carrierValidator;

  return carrierValidator.validateCarrier(trx, taxResponse, taxCodeReg, travelSegIndex);
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
Tax::validateEquipmentExemption(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t travelSegIndex)
{
  EquipmentValidator equipmentValidator;

  return equipmentValidator.validateEquipment(trx, taxResponse, taxCodeReg, travelSegIndex);
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
Tax::validateFareClass(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{
  FareClassValidator fareClassValidator;

  return fareClassValidator.validateFareClassRestriction(
      trx, taxResponse, taxCodeReg, travelSegIndex);
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
Tax::validateCabin(PricingTrx& trx,
                   TaxResponse& taxResponse,
                   TaxCodeReg& taxCodeReg,
                   uint16_t travelSegIndex)
{
  std::vector<TravelSeg*>::const_iterator travelSegI =
      getTravelSeg(taxResponse).begin() + travelSegIndex;

  return CabinValidator().validateCabinRestriction(trx, taxResponse, taxCodeReg, *travelSegI);
}

// ----------------------------------------------------------------------------
// Description:   ticketDesignatorValidation
//                  This will reject a itinerary segment for a
//                  tax code that has a ticket designator exemption
//                  match is located by returning a false
// ----------------------------------------------------------------------------

bool
Tax::validateTicketDesignator(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t travelSegIndex)
{
  if (TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx) &&
      AutomaticPfcTaxExemption::bypassTktDes(*(taxResponse.farePath())))
    return true;

  if (taxCodeReg.taxRestrTktDsgs().empty())
    return true;

  TravelSeg* travelSeg = getTravelSeg(taxResponse)[travelSegIndex];
  const AirSeg* airSeg;

  airSeg = dynamic_cast<const AirSeg*>(travelSeg);

  if (!airSeg)
    return true;

  std::string designator = "";

  if (UNLIKELY(trx.getRequest()->isSpecifiedTktDesignatorEntry()))
  {
    designator = trx.getRequest()->specifiedTktDesignator(travelSeg->segmentOrder()).c_str();
  }

  if (LIKELY(designator.empty()))
  {
    const Itin* itin = taxResponse.farePath()->itin();
    const FarePath* farePath = taxResponse.farePath();

    std::vector<PricingUnit*>::const_iterator puI;
    std::vector<FareUsage*>::const_iterator fuI;
    std::vector<TravelSeg*>::const_iterator tvlSegI;

    FareUsage* fareUsage = nullptr;

    for (puI = farePath->pricingUnit().begin(); puI != farePath->pricingUnit().end(); puI++)
    {
      for (fuI = (*puI)->fareUsage().begin(); fuI != (*puI)->fareUsage().end(); fuI++)
      {
        for (tvlSegI = (*fuI)->travelSeg().begin(); tvlSegI != (*fuI)->travelSeg().end(); tvlSegI++)
        {
          if (itin->segmentOrder(*tvlSegI) == itin->segmentOrder(travelSeg))
            fareUsage = (*fuI);
        } // TvlSeg Loop
      } // FU Loop
    } // PU Loop

    if (UNLIKELY(fareUsage == nullptr))
      return true;

    fareUsage->paxTypeFare()->createTktDesignator(designator);
  }

  if (designator.empty())
    return true;

  std::vector<TaxRestrictionTktDesignator*>::const_iterator taxTktIter =
      taxCodeReg.taxRestrTktDsgs().begin();

  for (; taxTktIter != taxCodeReg.taxRestrTktDsgs().end(); taxTktIter++)
  {
    if (LIKELY(!(*taxTktIter)->carrier().empty()))
    {
      if ((*taxTktIter)->carrier() != airSeg->marketingCarrierCode())
        continue;
    }
    FareClassCode taxRestrTktDsgs = (*taxTktIter)->tktDesignator().c_str();
    FareClassCode tktDesignator = designator.c_str();

    if (UNLIKELY(RuleUtil::matchFareClass(taxRestrTktDsgs.c_str(), tktDesignator.c_str())))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TICKET_DESIGNATOR, Diagnostic809);

      return false;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
// Description:  Tax::sequenceValidation
//                  This will reject a sequence of a particular Fixed
//                  tax code that was previousely applied to the same segment
//                  within the PNR
// ----------------------------------------------------------------------------
bool
Tax::validateSequence(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& travelSegStartIndex,
                      uint16_t& travelSegEndIndex,
                      bool checkSpn)
{
  if (UNLIKELY(utc::isTaxExemptForArunk(trx, taxSpecConfig()) &&
      taxResponse.farePath()->itin()->travelSeg().size() < travelSegStartIndex &&
      !taxResponse.farePath()->itin()->travelSeg()[travelSegStartIndex]->isAir()))
    return false;

  bool validateSeqForPercentageTax;

  validateSeqForPercentageTax = utc::validateSeqForPercentageTax(trx, taxSpecConfig());

  if ((taxCodeReg.taxType() == PERCENTAGE && !validateSeqForPercentageTax) ||
      (taxResponse.taxItemVector().empty()))
    return true;

  std::vector<TaxItem*>::const_iterator taxItemI = taxResponse.taxItemVector().begin();

  for (; taxItemI != taxResponse.taxItemVector().end(); taxItemI++)
  {
    if ((*taxItemI)->taxCode() != taxCodeReg.taxCode())
      continue;

    if (UNLIKELY(checkSpn && (*taxItemI)->specialProcessNo() != taxCodeReg.specialProcessNo()))
      continue;

    if (handleHiddenPoints() != HIDDEN_POINT_NOT_HANDLED)
    {
      if (travelSegStartIndex == (*taxItemI)->travelSegEndIndex() &&
          _hiddenBrdAirport == (*taxItemI)->taxLocalOff())
        continue;

      if (travelSegEndIndex == (*taxItemI)->travelSegStartIndex() &&
          _hiddenOffAirport == (*taxItemI)->taxLocalBoard())
        continue;
    }

    bool validateSeqIssueFix = utc::validateSeqIssueFix(trx, taxSpecConfig());

    if (taxCodeReg.taxCode().equalToConst("ZQ") || validateSeqIssueFix)
    {
      if (((travelSegEndIndex >= (*taxItemI)->travelSegStartIndex()) &&
           (travelSegStartIndex <= (*taxItemI)->travelSegEndIndex())) ||
          (taxCodeReg.occurrence() == APPLIED) || (taxCodeReg.occurrence() == BOOKLET))
      {
        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::TAX_ONCE_PER_SEGMENT, Diagnostic809);
        return false;
      }
    }
    else
    {
      if (((travelSegStartIndex >= (*taxItemI)->travelSegStartIndex()) &&
           (travelSegEndIndex <= (*taxItemI)->travelSegEndIndex())) ||
          (taxCodeReg.occurrence() == APPLIED) || (taxCodeReg.occurrence() == BOOKLET))
      {
        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::TAX_ONCE_PER_SEGMENT, Diagnostic809);
        return false;
      }
    }
  }

  return true;
}

bool
Tax::validateFinalGenericRestrictions(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      uint16_t& startIndex,
                                      uint16_t& endIndex)
{
  return true;
}

void
Tax::taxCreate(PricingTrx& trx,
               TaxResponse& taxResponse,
               TaxCodeReg& taxCodeReg,
               uint16_t travelSegStartIndex,
               uint16_t travelSegEndIndex)
{
  _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (LIKELY(!trx.getOptions()->currencyOverride().empty()))
  {
    _paymentCurrency = trx.getOptions()->currencyOverride();
  }

  Money targetMoney(0, _paymentCurrency);

  _paymentCurrencyNoDec = targetMoney.noDec(trx.ticketingDate());
  _taxAmount = taxCodeReg.taxAmt();
  _taxAmountAdjusted = 0.0;
  _taxableBaseFare = 0.0;
  _taxableFare = 0.0;
  _taxableFareAdjusted = 0.0;
  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegEndIndex;
  _calculationDetails = CalculationDetails();
  _taxOnTaxItems.clear();

  if (!fallback::taxShowSomeExemptions(&trx))
    _isSkipExempt = utc::skipExemption(trx, taxCodeReg);

  if (!fallback::taxProcessExemptionTable(&trx))
  {
    _isExemptedTax = checkAndSetExemptedTax(trx, taxResponse, taxCodeReg);
    if (_isExemptedTax)
      _taxAmount = 0;
  }

  if (_taxAmount == 0.0)
    return;

  if (taxCodeReg.taxType() == PERCENTAGE)
  {
    MoneyAmount moneyAmount = calculateFareDependendTaxableAmount(trx, taxResponse, taxCodeReg);

    _taxSplitDetails.setIsSet(true);

    _taxableBaseFare = moneyAmount;
    _taxableFare = _taxableBaseFare;
    _taxAmount = _taxableFare * taxCodeReg.taxAmt();

    if (!fallback::markupAnyFareOptimization(&trx))
    {
      _taxableFareAdjusted = calculateFareDependendTaxableAmount(trx, taxResponse, taxCodeReg, true);
      _taxAmountAdjusted = _taxableFareAdjusted * taxCodeReg.taxAmt();
    }
  }
  else if (LIKELY(taxCodeReg.taxType() == FIXED))
  {
    _taxSplitDetails.setIsSet(true);

    if (taxCodeReg.taxCur() == _paymentCurrency)
      return;

    Money sourceMoney(taxCodeReg.taxAmt(), taxCodeReg.taxCur());
    BSRCollectionResults bsrResults;
    if (UNLIKELY(!convertMoney(trx, sourceMoney, targetMoney, bsrResults)))
    {
      LOG4CXX_WARN(_logger, "Currency Convertion Collection *** Tax::taxCreate ***");

      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }

    _intermediateCurrency = bsrResults.intermediateCurrency();
    _intermediateNoDec = bsrResults.intermediateNoDec();
    _exchangeRate1 = bsrResults.taxReciprocalRate1();
    _exchangeRate1NoDec = bsrResults.taxReciprocalRate1NoDec();
    _exchangeRate2 = bsrResults.taxReciprocalRate2();
    _exchangeRate2NoDec = bsrResults.taxReciprocalRate2NoDec();
    _intermediateUnroundedAmount = bsrResults.intermediateUnroundedAmount();
    _intermediateAmount = bsrResults.intermediateAmount();

    _taxAmount = targetMoney.value();
  }
}

void
Tax::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  _calculationDetails.isTaxOnTax = true;

  _skipTaxOnTaxIfNoFare = utc::skipTaxOnTaxIfNoFare(trx, taxSpecConfig());
  _requireTaxOnTaxSeqMatch = utc::isTaxOnTaxSeqMatch(trx, taxSpecConfig());
  _requireTaxOnDomTaxMatch = utc::isTaxOnDomTax(trx, taxSpecConfig());

  utc::TaxOnTaxFilterUtc filter(trx.ticketingDate(), taxSpecConfig());

  TaxOnTax taxOnTax(_calculationDetails, _taxSplitDetails);
  taxOnTax.setSkipTaxOnTaxIfNoFare(_skipTaxOnTaxIfNoFare);
  taxOnTax.setRequireTaxOnTaxSeqMatch(_requireTaxOnTaxSeqMatch);
  taxOnTax.setRequireTaxOnDomTaxMatch(_requireTaxOnDomTaxMatch);

  taxOnTax.setIndexRange(std::make_pair(_travelSegStartIndex, _travelSegEndIndex));

  if (filter.enable())
    taxOnTax.filter() = filter;

  if (!fallback::markupAnyFareOptimization(&trx))
  {
    taxOnTax.calculateTaxOnTax(
        trx, taxResponse, _taxAmountAdjusted, _taxableFareAdjusted, taxCodeReg, _taxOnTaxItems, _specialPercentage);
  }

  taxOnTax.calculateTaxOnTax(
      trx, taxResponse, _taxAmount, _taxableFare, taxCodeReg, _taxOnTaxItems, _specialPercentage);
}

void
Tax::adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (!fallback::markupAnyFareOptimization(&trx))
  {
    adjustTax(trx, taxResponse, taxCodeReg, _taxAmountAdjusted);
  }

  adjustTax(trx, taxResponse, taxCodeReg, _taxAmount);
}

void
Tax::adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg, MoneyAmount& amount)
{
  _calculationDetails.taxToAdjustAmount = amount;
  const MoneyAmount taxAmount = AdjustTax::applyAdjust(
      trx, taxResponse, amount, _paymentCurrency, taxCodeReg, _calculationDetails);

  if (taxAmount)
  {
    amount = taxAmount;
  }
}

void
Tax::doTaxRange(PricingTrx& trx,
                TaxResponse& taxResponse,
                uint16_t& startIndex,
                uint16_t& endIndex,
                TaxCodeReg& taxCodeReg)
{
  TaxRange taxRange;
  MoneyAmount amount;

  if (!fallback::markupAnyFareOptimization(&trx))
  {
    amount = taxRange.applyRange(
        trx, taxResponse, _taxAmountAdjusted, _paymentCurrency, startIndex, endIndex, taxCodeReg);

    if (amount)
    {
      _taxAmountAdjusted = amount;
    }
  }

  amount = taxRange.applyRange(
      trx, taxResponse, amount, _paymentCurrency, startIndex, endIndex, taxCodeReg);

  if (UNLIKELY(amount))
  {
    _taxAmount = amount;
  }

  _calculationDetails.taxRangeAmount = _taxAmount;
}

void
Tax::doAtpcoDefaultTaxRounding(PricingTrx& trx)
{
  _calculationDetails.roundingUnit = 0.01;
  _calculationDetails.roundingNoDec = 2;
  _calculationDetails.roundingRule = NEAREST;
  _calculationDetails.isSpecialRounded = false;

  TaxRound taxRound;
  MoneyAmount amount;

  if (!fallback::markupAnyFareOptimization(&trx))
  {
    amount = taxRound.applyTaxRound(_taxAmountAdjusted, _paymentCurrency, 0.01, NEAREST);
    if (amount)
    {
      _taxAmountAdjusted = amount;
    }
  }

  amount = taxRound.applyTaxRound(_taxAmount, _paymentCurrency, 0.01, NEAREST);

  if (amount)
    _taxAmount = amount;
}

void Tax::doTaxRound(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  if (!fallback::markupAnyFareOptimization(&trx))
  {
    doTaxRound(trx, taxCodeReg, _taxAmountAdjusted);
  }
  doTaxRound(trx, taxCodeReg, _taxAmount);
}

void
Tax::doTaxRound(PricingTrx& trx, TaxCodeReg& taxCodeReg, MoneyAmount& amount)
{
  if (UNLIKELY(TrxUtil::isAtpcoTaxesDefaultRoundingEnabled(trx)))
  {
    doAtpcoDefaultTaxRounding(trx);
    return;
  }

  if (UNLIKELY(taxCodeReg.multioccconvrndInd() == YES))
    return;

  RoundingFactor& roundingUnit = _calculationDetails.roundingUnit;
  CurrencyNoDec& roundingNoDec = _calculationDetails.roundingNoDec;
  RoundingRule& roundingRule = _calculationDetails.roundingRule;
  bool& isSpecialRounded = _calculationDetails.isSpecialRounded;

  roundingUnit = taxCodeReg.taxcdRoundUnit();
  roundingNoDec = taxCodeReg.taxcdRoundUnitNodec();
  roundingRule = taxCodeReg.taxcdRoundRule();
  isSpecialRounded = false;

  TaxRound taxRound;

  if (!fallback::fallbackChangeSpecialDomesticRoundingLogic(&trx)) // new functionality
  {
    if (taxCodeReg.spclTaxRounding() == YES)
    {
      if (taxCodeReg.nation() == trx.getRequest()->ticketingAgent()->agentLocation()->nation())
      {
        // Based on IATA reso 024d footnote 3: Other Charges - Australian, Canadian, New Zealand,
        // US Tax Charges when collected in Australia, Canada, New Zealand, US respectively,
        // round to the nearest 0.01
        roundingUnit = 0.01;
        roundingRule = NEAREST;
        roundingNoDec = 2;
      }
      else
      {
        roundingRule = EMPTY; // to enforce fetching info from TaxNation table below
      }
    }
  }
  else // old functionality
  {
    if ((taxCodeReg.spclTaxRounding() == YES) &&
        (taxCodeReg.nation() == trx.getRequest()->ticketingAgent()->agentLocation()->nation()))
    {
      MoneyAmount fareAmount = _taxableFare;

      if (_taxablePartialFare)
        fareAmount = _taxablePartialFare;

      MoneyAmount taxSpecialAmount = taxRound.doSpecialTaxRound(
          trx, fareAmount, amount, utc::specialTaxRoundCentNumber(trx, taxCodeReg) );

      if (taxSpecialAmount)
      {
        amount = taxSpecialAmount;
        isSpecialRounded = true;
        return;
      }

      roundingUnit = 0.01;
      roundingRule = NEAREST;
    }
  }

  if ((taxCodeReg.taxCur() == _paymentCurrency) && (taxCodeReg.taxType() == FIXED) &&
      (roundingRule == EMPTY))
    return;

  //
  // Must Round Same Currency To Handle Percentage Taxes
  // Per Gary Nash The Nation Round Rules Apply If Round Rule
  // Is Blank For Specific Tax
  //

  if ((taxCodeReg.taxCur() != _paymentCurrency) || (roundingRule == EMPTY))
  {
    taxRound.retrieveNationRoundingSpecifications(trx, roundingUnit, roundingNoDec, roundingRule);
  }

  MoneyAmount taxAmount =
      taxRound.applyTaxRound(amount, _paymentCurrency, roundingUnit, roundingRule);

  if (!fallback::roundTaxToZero(&trx))
  {
    amount = taxAmount;
  }
  else
  {
    if (taxAmount)
    {
      amount = taxAmount;
    }
  }
}

MoneyAmount
Tax::fareAmountInNUC(const PricingTrx& trx, const TaxResponse& taxResponse)
{
  return taxResponse.farePath()->getTotalNUCAmount();
}

void
Tax::internationalFareRounding(CurrencyConversionFacade& ccFacade,
                               PricingTrx& trx,
                               Money& targetMoney,
                               MoneyAmount& sourceMoneyAmt)
{
  Money sourceMoney(sourceMoneyAmt, "NUC");

  if (ccFacade.convert(targetMoney, sourceMoney, trx, true))
  {
    sourceMoneyAmt = targetMoney.value();
  }
}

bool
Tax::validateFromToWithHiddenPoints(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    uint16_t& startIndex,
                                    uint16_t& endIndex)
{
  _hiddenBrdAirport = "";
  _hiddenOffAirport = "";

  if (UNLIKELY((taxCodeReg.loc1Type() == LOCTYPE_NONE) && (taxCodeReg.loc2Type() == LOCTYPE_NONE)))
    return true;

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));

  if (UNLIKELY((taxCodeReg.loc1Type() == LOCTYPE_NONE) && (taxCodeReg.loc2Type() != LOCTYPE_NONE)))
  {
    locIt->toSegmentNo(startIndex);

    if (locIt->hasNext())
    {
      locIt->next();
      if (locIt->isInLoc2(taxCodeReg, trx))
      {
        if (locIt->isHidden())
        {
          if (handleHiddenPoints() == HIDDEN_POINT_BOTH_LOCS ||
              handleHiddenPoints() == HIDDEN_POINT_LOC2)
          {
            _hiddenOffAirport = locIt->loc()->loc();
            return true;
          }
        }
        else
          return true;
      }
    }

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);
    return false;
  }

  bool foundStart = false;
  for (locIt->toSegmentNo(startIndex); locIt->hasNext(); locIt->next())
  {
    if (locIt->isInLoc1(taxCodeReg, trx) && locIt->hasNext() && locIt->isNextSegAirSeg())
    {
      if (locIt->isHidden())
      {
        if (handleHiddenPoints() == HIDDEN_POINT_BOTH_LOCS ||
            handleHiddenPoints() == HIDDEN_POINT_LOC1)
          _hiddenBrdAirport = locIt->loc()->loc();
        else
          continue;
      }
      startIndex = locIt->nextSegNo();
      endIndex = startIndex;
      foundStart = true;
      break;
    }
  }

  if (foundStart)
  {
    if (taxCodeReg.loc2Type() == LOCTYPE_NONE)
      return true;

    locIt->next();

    if (taxCodeReg.nextstopoverrestr() == YES)
      for (; locIt->hasNext() && !locIt->isStop(); locIt->next())
        ;

    if (locIt->isInLoc2(taxCodeReg, trx))
    {
      if (locIt->isHidden())
      {
        if (handleHiddenPoints() == HIDDEN_POINT_BOTH_LOCS ||
            handleHiddenPoints() == HIDDEN_POINT_LOC2)
        {
          _hiddenOffAirport = locIt->loc()->loc();
          return true;
        }
      }
      else
      {
        endIndex = locIt->prevSegNo();
        return true;
      }
    }

    if (locIt->isHidden())
    {
      while (locIt->isHidden())
        locIt->next();
      if (locIt->isInLoc2(taxCodeReg, trx))
      {
        endIndex = locIt->prevSegNo();
        return true;
      }
    }
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);
  return false;
}

bool
Tax::validateTransitOnHiddenPoints(const TaxCodeReg& taxCodeReg) const
{
  if ((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
      ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
       (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION)))
  {
    if (!_hiddenOffAirport.empty())
      return true;
  }
  else
  {
    if (!_hiddenBrdAirport.empty())
      return true;
  }

  return false;
}

TaxLocIterator*
Tax::getLocIterator(FarePath& farePath)
{
  if (!_locIteratorInitialized)
  {
    _locIt.initialize(farePath);
    _locIteratorInitialized = true;
  }

  return &_locIt;
}

bool
Tax::shouldSplitPercentageTax(const PricingTrx& trx, const TaxCode& taxCode)
{
  if (trx.getTrxType() == PricingTrx::PRICING_TRX)
  {
    return false;
  }

  if (LIKELY(!trx.getOptions() ||
      (!trx.getOptions()->isSplitTaxesByLeg() && !trx.getOptions()->isSplitTaxesByFareComponent())))
    return false;
  if (taxCode == "UO2")
    return true;

  return false;
}

bool
Tax::isValidFareClassToAmount(const TaxCodeReg& taxCodeReg, const FareUsage& fareUsage) const
{
  const char* fareClassFareCStr = fareUsage.paxTypeFare()->fareClass().c_str();

  auto it = std::find_if(taxCodeReg.restrictionFareClass().begin(),
      taxCodeReg.restrictionFareClass().end(),
      [fareClassFareCStr](const FareClassCode& fareClass) -> bool
        { return RuleUtil::matchFareClass(fareClass.c_str(), fareClassFareCStr); });

  const bool match = it != taxCodeReg.restrictionFareClass().end();

  if ((match && taxCodeReg.fareclassExclInd() == YES) ||
      (!match && taxCodeReg.fareclassExclInd() != YES))
      return false;

  return true;
}

MoneyAmount
Tax::fareAmountInNUCForCurrentSegment(PricingTrx& trx, const FarePath* farePath,
    const TaxCodeReg& taxCodeReg) const
{
  TSE_ASSERT(farePath != nullptr);
  TSE_ASSERT(_travelSegStartIndex <= _travelSegEndIndex);
  TSE_ASSERT(_travelSegEndIndex < farePath->itin()->travelSeg().size());

  MoneyAmount amountForFaresStartingInRange = 0.0;
  const bool isFareClassToAmount = !taxCodeReg.restrictionFareClass().empty() &&
          utc::fareClassToAmount(trx, taxCodeReg);

  for (uint16_t segIdx = _travelSegStartIndex; segIdx <= _travelSegEndIndex; ++segIdx)
  {
    TravelSeg* segOnIndex = farePath->itin()->travelSeg()[segIdx];
    for (PricingUnit* pu : farePath->pricingUnit())
      for (FareUsage* fu : pu->fareUsage())
      {
        if (isFareClassToAmount && !isValidFareClassToAmount(taxCodeReg, *fu))
          continue;

        if (fu->travelSeg().front() == segOnIndex)
        {
          amountForFaresStartingInRange += fu->paxTypeFare()->fare()->nucFareAmount();
        }
      }
  }
  return amountForFaresStartingInRange;
}

MoneyAmount
Tax::calculateFareDependendTaxableAmount(PricingTrx& trx,
                                         TaxResponse& taxResponse,
                                         TaxCodeReg& taxCodeReg,
                                         const bool adjusted)
{
  const CurrencyCode& calculationCurrency = fallback::ssdsp1511fix(&trx) ?
      taxResponse.farePath()->calculationCurrency() :
      taxResponse.farePath()->getCalculationCurrency();

  const CurrencyCode& baseFareCurrency = taxResponse.farePath()->baseFareCurrency();

  MoneyAmount moneyAmount;
  CurrencyConversionFacade ccFacade;

  if (Tax::shouldSplitPercentageTax(trx, taxCodeReg.taxCode()))
  {
    // Here we only take a fare that starts at the currently processed segment
    // because of a tax split we cannot apply it at once to the whole fare path
    // at this moment it is just a quick fix for UO2 tax for VA that may not work with other taxes
    moneyAmount = fareAmountInNUCForCurrentSegment(trx, taxResponse.farePath(), taxCodeReg);

    //TODO MarkupAnyFare check Gst and MarkupAmount, convert
  }
  else
  {
    moneyAmount = fareAmountInNUC(trx, taxResponse);
    if (adjusted)
    {
      moneyAmount += taxResponse.farePath()->getTotalNUCMarkupAmount();
    }

    utc::FareType fareType(trx, taxSpecConfig());
    bool isFareTypeToValidate = fareType.mustBeValidate();
    const bool isFareClassToAmount = !taxCodeReg.restrictionFareClass().empty() &&
        utc::fareClassToAmount(trx, taxCodeReg);

    if (taxCodeReg.taxfullFareInd() == YES || isFareTypeToValidate || isFareClassToAmount)
    {
      moneyAmount = 0.0;

      const FarePath* farePath = taxResponse.farePath();

      std::vector<PricingUnit*>::const_iterator pricingUnitI = farePath->pricingUnit().begin();
      std::vector<FareUsage*>::iterator fareUsageI;

      for (; pricingUnitI != farePath->pricingUnit().end(); pricingUnitI++)
      {
        for (fareUsageI = (*pricingUnitI)->fareUsage().begin();
             fareUsageI != (*pricingUnitI)->fareUsage().end();
             fareUsageI++)
        {
          FareUsage& fareUsage = *(*fareUsageI);

          if (isFareTypeToValidate && !fareType.validateFareTypes(fareUsage, taxCodeReg.nation()))
            continue;

          if (isFareClassToAmount && !isValidFareClassToAmount(taxCodeReg, fareUsage))
            continue;

          const PaxTypeFare* ptCurrFare = nullptr;

          if (fareUsage.adjustedPaxTypeFare() == nullptr)
          { // normal processing
            ptCurrFare = fareUsage.paxTypeFare();
          }
          else // Currency adjustment processing for Nigeria
          {
            ptCurrFare = fareUsage.adjustedPaxTypeFare();
          }

          const PaxTypeFare* ptFare = nullptr;
          if (!fallback::taxFareWithoutBase(&trx))
          {
            if (UNLIKELY(taxCodeReg.taxCode() == "UO2"))
              ptFare = ptCurrFare;
            else
              ptFare = ptCurrFare->fareWithoutBase();
          }
          else
          {
            // real base fare
            ptFare = ptCurrFare->fareWithoutBase();
          }

          Money targetMoneyOrigination(ptFare->fare()->fareAmount(), calculationCurrency);
          if (ptFare->fare()->currency() != targetMoneyOrigination.code() )
          {
            Money sourceMoneyCalculation(ptFare->fare()->fareAmount(), ptFare->fare()->currency());

            if (!ccFacade.convert(targetMoneyOrigination,
                                  sourceMoneyCalculation,
                                  trx,
                                  taxResponse.farePath()->itin()->useInternationalRounding(),
                                  CurrencyConversionRequest::OTHER,
                                  false,
                                  nullptr))
            {
              LOG4CXX_WARN(_logger,
                "Currency Convertion Collection *** Tax::calculateFareDependendTaxableAmount ***");

              TaxDiagnostic::collectErrors(trx, taxCodeReg, taxResponse,
                  TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
            }
          }
          moneyAmount += targetMoneyOrigination.value();
        }
      }
    }

  }
  _calculationDetails.baseFareSumAmount = moneyAmount;

  // Check for Plus Up Pricing
  if (taxResponse.farePath()->itin()->isPlusUpPricing())
  {
    ConsolidatorPlusUp* cPlusUp = taxResponse.farePath()->itin()->consolidatorPlusUp();
    _calculationDetails.earlyPlusUpAmount =
        cPlusUp->calcTaxablePlusUpAmount(trx, taxCodeReg.taxCode(), taxResponse.farePath());
    moneyAmount += _calculationDetails.earlyPlusUpAmount;
  }

  _calculationDetails.calculationCurrency = calculationCurrency;
  _calculationDetails.fareInCalculationCurrencyAmount = moneyAmount;

  if (calculationCurrency != baseFareCurrency)
  {
    Money targetMoneyOrigination(baseFareCurrency);
    targetMoneyOrigination.value() = 0;

    Money sourceMoneyCalculation(moneyAmount, calculationCurrency);

    if (!ccFacade.convert(targetMoneyOrigination,
                          sourceMoneyCalculation,
                          trx,
                          taxResponse.farePath()->itin()->useInternationalRounding(),
                          CurrencyConversionRequest::OTHER,
                          false,
                          &_calculationDetails.calculationToBaseFareResults))
    {
      LOG4CXX_WARN(_logger,
          "Currency Convertion Collection *** Tax::calculateFareDependendTaxableAmount ***");

      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }
    moneyAmount = targetMoneyOrigination.value();
  }

  _calculationDetails.baseFareCurrency = baseFareCurrency;
  _calculationDetails.fareInBaseFareCurrencyAmount = moneyAmount;

  Money targetMoney(_paymentCurrency);
  targetMoney.value() = 0;

  if (baseFareCurrency != _paymentCurrency)
  {
    Money sourceMoney(moneyAmount, baseFareCurrency);

    if (!ccFacade.convert(targetMoney,
                          sourceMoney,
                          trx,
                          false,
                          CurrencyConversionRequest::OTHER,
                          false,
                          &_calculationDetails.baseFareToPaymentResults))
    {
      LOG4CXX_WARN(_logger, "Currency Convertion Collection *** Tax::taxCreate ***");

      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
    }
    moneyAmount = targetMoney.value();

    _calculationDetails.fareInPaymentCurrencyAmount = moneyAmount;
  }
  else
  {
    _calculationDetails.fareInPaymentCurrencyAmount = moneyAmount;
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);
    bool currencyNUCOrUSD = (taxResponse.farePath()->calculationCurrency() == "USD" ||
                             taxResponse.farePath()->calculationCurrency() == "NUC" ||
                             trx.getOptions()->currencyOverride() == "USD" ||
                             trx.getOptions()->currencyOverride() == "NUC");
    if (taxCodeReg.nation() == NATION_ECUADOR && (taxTrx == nullptr || currencyNUCOrUSD))
    {
      _calculationDetails.isInternationalRounded = true;
      internationalFareRounding(ccFacade, trx, targetMoney, moneyAmount);
      _calculationDetails.internationalRoundedAmount = moneyAmount;
    }
  }

  _taxSplitDetails.setIsSet(true);

  return moneyAmount;
}

// ----------------------------------------------------------------------------
// Description:  Tax::validateTaxOnChangeFees
//
// ----------------------------------------------------------------------------
bool
Tax::validateTaxOnChangeFees(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  return true;
}

bool
Tax::validateBaseTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  return true;
}

void
Tax::setupLandToAirStopover(PricingTrx& trx, TaxResponse& taxResponse)
{
  _landToAirStopover = utc::isLandToAirStopover(trx, taxResponse, _taxSpecConfig);
}

void
Tax::updateCalculationDetails(TaxItem* taxItem)
{
  _calculationDetails.isTaxOnTax = true;
  _calculationDetails.taxableTaxes.push_back(
      std::make_pair(taxItem->taxCode(), taxItem->taxAmount()));
  _calculationDetails.taxableTaxSumAmount += taxItem->taxAmount();
  _calculationDetails.taxableTaxItems.push_back(taxItem);
}

const std::vector<TravelSeg*>&
Tax::getTravelSeg(const TaxResponse& taxResponse) const
{
  return taxResponse.farePath()->itin()->travelSeg();
}

bool
Tax::checkAndSetExemptedTax(PricingTrx& trx,
    TaxResponse& taxResponse,
    TaxCodeReg& taxCodeReg) const
{
  if (!utc::validateExemptionTable(trx, taxCodeReg))
      return false;

  const Agent& agent = *trx.getRequest()->ticketingAgent();
  CarrierCode validCarrier = taxResponse.farePath()->itin()->validatingCarrier();

  PseudoCityCode channelId(!agent.tvlAgencyPCC().empty() ?
      (const std::string)agent.tvlAgencyPCC() : agent.officeDesignator());

  DiagManager diag(trx, Diagnostic818, "EX");
  const std::vector<TaxExemption*>& exemptions = trx.dataHandle().getTaxExemption(
      taxCodeReg.taxCode(), channelId, trx.dataHandle().ticketDate());

  if (diag.isActive())
  {
    diag << "***VALIDATE EXEMPTION TABLE SEQ-" << taxCodeReg.taxCode() << taxCodeReg.seqNo() << "\n";
    diag << "DESIGNATOR-" << agent.officeDesignator() << " STATION-" << agent.officeStationCode()
         << " CARIER-" << validCarrier << " PCC*OAC-" << channelId << "\n";
  }

  for (const TaxExemption* exemption : exemptions)
  {
    if (diag.isActive())
    {
      std::string strCarriers;
      for(const CarrierCode& carrier: exemption->validationCxr())
      {
        strCarriers += !strCarriers.empty() ? "," : "";
        strCarriers += carrier;
      }

      diag << "TYPE-" << exemption->channelType()
           << " ID-" << exemption->channelId()
           << " NUMBER-" << exemption->officeStationCode()
           << " CARRIER-" << strCarriers << "\n";
    }

    if (!exemption->channelId().empty())
    {
      if (exemption->channelType() == "TN")
      {
        if(agent.tvlAgencyPCC().empty() ||
           exemption->channelId() != agent.tvlAgencyPCC())
          continue;
      }
      else
      {
        if (agent.officeDesignator().empty() ||
            exemption->channelId() != agent.officeDesignator())
          continue;

        if (agent.officeStationCode().empty() || agent.officeStationCode().length() != 7 ||
            (!exemption->officeStationCode().empty() &&
             exemption->officeStationCode().find(agent.officeStationCode())!=0))
          continue;
      }
    }

    bool match = true;
    if (!exemption->validationCxr().empty() && !validCarrier.empty())
    {
      match = std::any_of(exemption->validationCxr().begin(), exemption->validationCxr().end(),
        [&validCarrier] (const CarrierCode& carrier)->bool { return carrier == validCarrier; });
    }

    if (match)
    {
      diag << " - MATCHING RECORD - EXEMPTION\n";
      return true;
    }
  }

  diag << "***VALIDATE EXEMPTION TABLE - END PROCESSING - NO EXEMPT\n";
  return false;
}
