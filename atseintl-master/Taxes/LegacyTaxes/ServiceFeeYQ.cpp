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

#include "Taxes/LegacyTaxes/ServiceFeeYQ.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "Common/YQYR/YQYRUtils.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxText.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/ApplicationFee.h"
#include "Taxes/LegacyTaxes/ServiceFeeRec1ValidatorYQ.h"
#include "Taxes/LegacyTaxes/TaxApplyYQ.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

#include <algorithm>
#include <functional>


using namespace std;

log4cxx::LoggerPtr
tse::YQYR::ServiceFee::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.YQYR.ServiceFee"));

namespace tse
{

namespace YQYR
{
const string
ServiceFee::SERVICE_FEE_DEFAULT_TEXT("SERVICE FEE");
const string
ServiceFee::FUEL(" - FUEL");
const string
ServiceFee::INSURANCE(" - INSURANCE");
const string
ServiceFee::CARRIER_IMPOSED_MISC(" - CARRIER-IMPOSED MISC");

void
ServiceFee::collectFees(PricingTrx& trx, TaxResponse& taxResponse)
{
  std::vector<YQYRCalculator::YQYRApplication> fees;
  FarePath* fp = taxResponse.farePath();

  if(trx.isValidatingCxrGsaApplicable() && trx.diagnostic().diagnosticType() == Diagnostic827)
  {
     const std::string& diagCarrier = trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
     if(diagCarrier.empty() || diagCarrier == taxResponse.validatingCarrier())
     {
        std::ostringstream stream;
        stream << "\n**************************************************************\n"
            << "VALIDATING CARRIER: " << taxResponse.validatingCarrier() << "\n"
            << "\n**************************************************************\n";
        trx.diagnostic().insertDiagMsg(stream.str());
     }
     else return;
  }

  YQYRCalculator* yqyrCalculator = fp->yqyrCalculator();
  if (yqyrCalculator && !YQYRUtils::printDiag(trx) && !dynamic_cast<NetRemitFarePath*>(fp))
    yqyrCalculator->findMatchingPaths(fp, fp->itin()->validatingCarrier(), fees);
  else
  {
    //    std::cout <<"No precalculated S1 list found\n";
    YQYRCalculator yqyrCalculator(trx, *fp->itin(), fp, YQYRUtils::printDiag(trx));
    yqyrCalculator.process();
    yqyrCalculator.findMatchingPaths(fp, fp->itin()->validatingCarrier(), fees);
  }

  applyCharges(trx, taxResponse, fees);
}

void
ServiceFee::applyCharges(PricingTrx &trx,
                         TaxResponse &taxResponse,
                         YQYRCalculator::YQYRFeesApplicationVec &fees)
{
  tse::YQYR::ServiceFeeRec1Validator dataAdapter;

  for (const auto& yqyrApplication : fees)
  {
    // init data adapter
    const YQYRFees* s1 = yqyrApplication.yqyr;
    dataAdapter.LoadData(*const_cast<YQYRFees*>(s1));
    dataAdapter.taxType() = s1->percent() > 0 ? 'P' : 'F'; // TODO - why PERCENT/FIXED don't work?
    dataAdapter.segmentOrderBegin() =
        yqyrApplication.first + 1; // segmentOrderBegin/End are 1-based indices
    dataAdapter.segmentOrderEnd() = yqyrApplication.last + 1;
    applyCharge(trx, taxResponse, dataAdapter);
  }
}


void
ServiceFee::applyCharge(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator)
{
  tse::YQYR::ServiceFeeRec1Validator* serviceFee;

  trx.dataHandle().get(serviceFee);

  if (UNLIKELY(serviceFee == nullptr))
  {
    LOG4CXX_WARN(_logger, "trx.datahandle().get(serviceFee) failed");
    return;
  }

  serviceFee->taxCode() = serviceFeeRec1Validator.taxCode();
  serviceFee->taxCurNodec() = serviceFeeRec1Validator.taxCurNodec();
  serviceFee->seqNo() = serviceFeeRec1Validator.seqNo();
  serviceFee->taxAmt() = serviceFeeRec1Validator.taxAmt();
  serviceFee->feeApplInd() = serviceFeeRec1Validator.feeApplInd();
  serviceFee->bookingCode1() = serviceFeeRec1Validator.bookingCode1();
  serviceFee->bookingCode2() = serviceFeeRec1Validator.bookingCode2();
  serviceFee->bookingCode3() = serviceFeeRec1Validator.bookingCode3();
  serviceFee->taxCur() = serviceFeeRec1Validator.taxCur();
  serviceFee->taxNodec() = serviceFeeRec1Validator.taxNodec();
  serviceFee->itineraryType() = serviceFeeRec1Validator.itineraryType();
  serviceFee->taxType() = serviceFeeRec1Validator.taxType();
  serviceFee->taxIncludedInd() = serviceFeeRec1Validator.taxIncludedInd();

  _travelSegStartIndex = 0;
  _travelSegEndIndex = 0;

  const AirSeg* airSeg;

  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  for (uint16_t index = 0; travelSegI != taxResponse.farePath()->itin()->travelSeg().end();
       travelSegI++, index++)
  {
    airSeg = (*travelSegI)->toAirSeg();

    if (!airSeg)
      continue;

    if (taxResponse.farePath()->itin()->segmentOrder(*travelSegI) ==
        serviceFeeRec1Validator.segmentOrderBegin())
      _travelSegStartIndex = index;

    if (taxResponse.farePath()->itin()->segmentOrder(*travelSegI) ==
        serviceFeeRec1Validator.segmentOrderEnd())
      _travelSegEndIndex = index;
  }

  taxCreate(trx, taxResponse, *serviceFee, _travelSegStartIndex, _travelSegEndIndex);

  // Probably will not use Adjust Tax
  //  adjustTax (trx, taxResponse, *serviceFee);

  if (_paymentCurrency != serviceFee->taxCur())
    doTaxRound(trx, *serviceFee);

  if (UNLIKELY(serviceFeeRec1Validator.taxType() == PERCENTAGE
      && serviceFee->taxCur().empty()))
  {
    serviceFee->taxCur() = taxResponse.farePath()->baseFareCurrency();
  }

  YQYR::TaxApply taxApply;
  taxApply.initializeTaxItem(trx, *this, taxResponse, *serviceFee);

  const CarrierPreference* carrierPreference = TrxUtil::isAutomatedRefundCat33Enabled(trx)
      ? trx.dataHandle().getCarrierPreference(taxResponse.validatingCarrier(),
                                              taxResponse.farePath()->itin()->travelDate())
      : nullptr;

  for (const auto taxItem : taxResponse.taxItemVector())
  {
    if (taxItem->taxCode() != serviceFee->taxCode())
      continue;

    if (taxItem->seqNo() != serviceFee->seqNo())
      continue;

    taxItem->serviceFee() = true;
    taxItem->setCarrierCode(serviceFeeRec1Validator.carrierCode());

    //  Gary Nash CR to Default to Service Fee Always
    //
    //  (*taxItemI)->taxDescription() = serviceFeeRec1Validator.txtMsgs();

    taxItem->taxDescription() = SERVICE_FEE_DEFAULT_TEXT;

    if (UNLIKELY(serviceFee->taxCode().size() < 3))
      continue;

    if (serviceFee->taxCode()[2] == 'I')
    {
      if (serviceFee->taxCode()[1] == 'R')
        taxItem->taxDescription() += CARRIER_IMPOSED_MISC;
      else
        taxItem->taxDescription() += INSURANCE;
    }

    if (serviceFee->taxCode()[2] == 'F')
      taxItem->taxDescription() += FUEL;

    if (carrierPreference)
    {
      const bool nonRefundable = isNonRefundable(taxItem->taxCode(), *carrierPreference);
      taxItem->setRefundableTaxTag(nonRefundable ? 'N' : 'Y');
    }
  }
} // applyCharge()

void
ServiceFee::taxCreate(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator,
                      uint16_t travelSegStartIndex,
                      uint16_t travelSegEndIndex)
{
  CurrencyConversionFacade ccFacade;

  _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (LIKELY(!trx.getOptions()->currencyOverride().empty()))
  {
    _paymentCurrency = trx.getOptions()->currencyOverride();
  }

  Money targetMoney(_paymentCurrency);
  targetMoney.value() = 0;

  _paymentCurrencyNoDec = targetMoney.noDec(trx.ticketingDate());

  _taxAmount = serviceFeeRec1Validator.taxAmt();
  _taxableFare = 0.0;
  _nucTaxableFare = 0.0;
  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegEndIndex;

  if (serviceFeeRec1Validator.taxType() == PERCENTAGE && _taxAmount != 0.0)
  {
    MoneyAmount moneyAmount = 0.0;

    // if(!utc::isChangeFeeSeq(trx, serviceFeeRec1Validator))
    if (!utc::isTaxOnChangeFee(trx, taxSpecConfig()))
    {
      moneyAmount =
          calculateFareDependendTaxableAmount(trx, taxResponse, serviceFeeRec1Validator, ccFacade);
    }
    else
    {
      moneyAmount =
          calculateChangeFeeTaxableAmount(trx, taxResponse, serviceFeeRec1Validator, ccFacade);
    }

    _taxableFare = moneyAmount;

    _taxAmount = _taxableFare * serviceFeeRec1Validator.taxAmt();

    return;
  }

  if (LIKELY(serviceFeeRec1Validator.taxType() == FIXED))
  {
    if (serviceFeeRec1Validator.taxCur() == _paymentCurrency)
      return;

    Money sourceMoney(serviceFeeRec1Validator.taxAmt(), serviceFeeRec1Validator.taxCur());
    BSRCollectionResults bsrResults;

    if (UNLIKELY(!ccFacade.convert(targetMoney,
                          sourceMoney,
                          trx,
                          false,
                          CurrencyConversionRequest::TAXES,
                          false,
                          &bsrResults)))
    {
      LOG4CXX_WARN(_logger, "Currency Convertion Collection *** Tax::taxCreate ***");

      TaxDiagnostic::collectErrors(trx,
                                   serviceFeeRec1Validator,
                                   taxResponse,
                                   TaxDiagnostic::CURRENCY_CONVERTER_BSR,
                                   Diagnostic810);
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
ServiceFee::doAtpcoDefaultTaxRounding(tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator)
{
  serviceFeeRec1Validator.taxcdRoundUnit() = 0.01;
  serviceFeeRec1Validator.taxcdRoundUnitNodec() = 2;
  serviceFeeRec1Validator.taxcdRoundRule() = NEAREST;

  TaxRound taxRound;
  MoneyAmount taxAmount = taxRound.applyTaxRound(_taxAmount, _paymentCurrency, 0.01, NEAREST);

  if (taxAmount)
    _taxAmount = taxAmount;
}

void
ServiceFee::doTaxRound(PricingTrx& trx, tse::YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator)
{
  if (UNLIKELY(serviceFeeRec1Validator.multioccconvrndInd() == YES))
    return;

  if (UNLIKELY(TrxUtil::isAtpcoTaxesDefaultRoundingEnabled(trx)))
  {
    doAtpcoDefaultTaxRounding(serviceFeeRec1Validator);
    return;
  }

  RoundingFactor roundingUnit = serviceFeeRec1Validator.taxcdRoundUnit();
  CurrencyNoDec roundingNoDec = serviceFeeRec1Validator.taxcdRoundUnitNodec();
  RoundingRule roundingRule = serviceFeeRec1Validator.taxcdRoundRule();

  TaxRound taxRound;

  if (UNLIKELY((serviceFeeRec1Validator.spclTaxRounding() == YES) &&
      (serviceFeeRec1Validator.nation() ==
       trx.getRequest()->ticketingAgent()->agentLocation()->nation())))
  {
    MoneyAmount fareAmount = _taxableFare;

    if (_taxablePartialFare)
      fareAmount = _taxablePartialFare;

    MoneyAmount taxSpecialAmount = taxRound.doSpecialTaxRound(trx, fareAmount, _taxAmount);

    if (taxSpecialAmount)
    {
      _taxAmount = taxSpecialAmount;
      return;
    }

    roundingUnit = 0.01;
    roundingRule = NEAREST;
  }

  if (UNLIKELY((serviceFeeRec1Validator.taxCur() == _paymentCurrency) &&
      (serviceFeeRec1Validator.taxType() == FIXED) && (roundingRule == EMPTY)))
    return;

  //
  // Must Round Same Currency To Handle Percentage Taxes
  // Per Gary Nash The Nation Round Rules Apply If Round Rule
  // Is Blank For Specific Tax
  //

  if (LIKELY((serviceFeeRec1Validator.taxCur() != _paymentCurrency) || (roundingRule == EMPTY)))
  {
    taxRound.retrieveNationRoundingSpecifications(trx, roundingUnit, roundingNoDec, roundingRule);
  }

  MoneyAmount taxAmount =
      taxRound.applyTaxRound(_taxAmount, _paymentCurrency, roundingUnit, roundingRule);
  if (taxAmount)
  {
    _taxAmount = taxAmount;
  }
}

bool
ServiceFee::isNonRefundable(const TaxCode& taxCode, const CarrierPreference& carrierPref)
{
  const TaxCode& nonRefYQ = carrierPref.getNonRefundableYQCode(); // values: YQ, YQF, YQI
  const TaxCode& nonRefYR = carrierPref.getNonRefundableYRCode(); // values: YR, YRF, YRI
  return (!nonRefYQ.empty() && taxCode.find(nonRefYQ) != TaxCode::npos) ||
         (!nonRefYR.empty() && taxCode.find(nonRefYR) != TaxCode::npos);
}
} // YQYR namespace
} // tse namespace
