// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Taxes/LegacyTaxes/TaxOnChangeFee.h"

#include "Common/BSRCollectionResults.h"
#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Logger.h"
#include "DataModel/Agent.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/Itin.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/ReissueCharges.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/GetTicketingDate.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/LegacyTaxes/TaxOnChangeFee.h"
#include "Taxes/LegacyTaxes/ChangeFeeAmount.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Taxes/LegacyFacades/ItinSelector.h"

using namespace std;

namespace tse
{
FALLBACK_DECL(SSDSP_1988);
FALLBACK_DECL(getCat33ChangeOnFeeFromExcItin);

log4cxx::LoggerPtr
TaxOnChangeFee::_loggerTaxOnChangeFee(log4cxx::Logger::getLogger("atseintl.Taxes.TaxOnChangeFee"));

void
TaxOnChangeFee::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  return;
}

bool
TaxOnChangeFee::validateSequence(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 uint16_t& travelSegStartIndex,
                                 uint16_t& travelSegEndIndex,
                                 bool checkSpn)
{
  std::vector<TaxItem*>::const_iterator taxItemI = taxResponse.changeFeeTaxItemVector().begin();

  for (; taxItemI != taxResponse.changeFeeTaxItemVector().end(); taxItemI++)
  {
    if ((*taxItemI)->taxCode() == taxCodeReg.taxCode())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TAX_ONCE_PER_SEGMENT, Diagnostic809);
      return false;
    }
  }

  return true;
}

void
TaxOnChangeFee::preparePortionOfTravelIndexes(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg)
{
  _oldItin = nullptr;
  if( trx.isExchangeTrx() )
  {
    const BaseExchangeTrx* exchangeTrx = dynamic_cast<const BaseExchangeTrx*>(&trx);

    if (exchangeTrx && (utc::isMatchOrigTicket(trx, taxCodeReg, trx.getRequest()->ticketingDT()) ||
         (exchangeTrx->currentTicketingDT().isValid() &&
          utc::isMatchOrigTicket(trx, taxCodeReg, exchangeTrx->currentTicketingDT()) )) )
    {
      if (!exchangeTrx->exchangeItin().empty())
        _oldItin = exchangeTrx->exchangeItin().front();

      _furthestFareBreakSegment = -1;
    }
  }

  if (!_oldItin)
    Tax::preparePortionOfTravelIndexes(trx, taxResponse, taxCodeReg);
}

void
TaxOnChangeFee::taxCreate(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t travelSegStartIndex,
                          uint16_t travelSegEndIndex)
{
  CurrencyConversionFacade ccFacade;

  _paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (!trx.getOptions()->currencyOverride().empty())
  {
    _paymentCurrency = trx.getOptions()->currencyOverride();
  }

  Money targetMoney(_paymentCurrency);
  targetMoney.value() = 0;

  _paymentCurrencyNoDec = targetMoney.noDec(trx.ticketingDate());

  _taxAmount = taxCodeReg.taxAmt();
  _taxableFare = 0.0;
  _travelSegStartIndex = travelSegStartIndex;
  _travelSegEndIndex = travelSegEndIndex;
  _calculationDetails = CalculationDetails();

  if (_taxAmount == 0.0)
    return;

  if (taxCodeReg.taxType() == PERCENTAGE)
  {
    MoneyAmount moneyAmount = AbstractChangeFeeAmount::create(trx, taxResponse,
        taxCodeReg, _paymentCurrency)->getAmountInPaymentCurrency();

    _taxableFare = moneyAmount;
    _taxAmount = _taxableFare * taxCodeReg.taxAmt();
  }
  else if (taxCodeReg.taxType() == FIXED)
  {
    _taxSplitDetails.setIsSet(true);

    if (taxCodeReg.taxCur() == _paymentCurrency)
      return;

    Money sourceMoney(taxCodeReg.taxAmt(), taxCodeReg.taxCur());
    BSRCollectionResults bsrResults;

    if (!ccFacade.convert(targetMoney,
                          sourceMoney,
                          trx,
                          false,
                          CurrencyConversionRequest::TAXES,
                          false,
                          &bsrResults))
    {
      LOG4CXX_WARN(_loggerTaxOnChangeFee, "Currency Convertion Collection *** Tax::taxCreate ***");

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

// ----------------------------------------------------------------------------
// Description:  TaxOnChangeFee::validateTaxOnChangeFees
//
// ----------------------------------------------------------------------------
bool
TaxOnChangeFee::validateTaxOnChangeFees(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg)
{
  return AbstractChangeFeeAmount::create(trx, taxResponse, taxCodeReg,
      _paymentCurrency)->validate();
}

// ----------------------------------------------------------------------------
// Description:  TaxOnChangeFee validate base tax.
//               returns true if base tax is calculated.
// ----------------------------------------------------------------------------
bool
TaxOnChangeFee::validateBaseTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
  {
    return true;
  }

  const std::vector<tse::TaxSpecConfigReg*>* taxSpecConf = taxSpecConfig();
  if (taxSpecConf == nullptr)
    return false;

  const std::string baseTaxCode = utc::baseTaxCode(trx, taxSpecConf);

  if (fallback::getCat33ChangeOnFeeFromExcItin(&trx))
  {
    if (baseTaxCode.empty())
      return true;

    std::vector<TaxItem*>::const_iterator taxItemI = taxResponse.taxItemVector().begin();

    for (; taxItemI != taxResponse.taxItemVector().end(); ++taxItemI)
    {
      if ((*taxItemI)->taxCode() == baseTaxCode)
      {
        return true;
      }
    }

    return false;
  }
  else
  {
    const TaxResponse* taxRes =
        (TrxUtil::isAutomatedRefundCat33Enabled(trx) && ItinSelector(trx).isRefundTrx())
            ? (!trx.getExcItinTaxResponse().empty() ? trx.getExcItinTaxResponse().front() : nullptr)
            : &taxResponse;

    return taxRes &&
           (baseTaxCode.empty() || std::any_of(taxRes->taxItemVector().begin(),
                                               taxRes->taxItemVector().end(),
                                               [&baseTaxCode](const TaxItem* item)
                                               { return item->taxCode() == baseTaxCode; }));
  }
}

bool
TaxOnChangeFee::validateLocRestrictions(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t& startIndex,
                                        uint16_t& endIndex)
{
  if (handleHiddenPoints())
    return true;

  if (_oldItin)
  {
    LocRestrictionValidator locRestrictionValidator;
    isSkipExempt() = true;
    locRestrictionValidator.setItin(_oldItin);

    return locRestrictionValidator.validateLocation(
      trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  return Tax::validateLocRestrictions(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxOnChangeFee::validateGeoSpecLoc1(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    uint16_t& startIndex,
                                    uint16_t& endIndex)
{
  if (_oldItin)
    return true;

  return Tax::validateGeoSpecLoc1(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxOnChangeFee::validateTransferTypeLoc1(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t startIndex,
                                        uint16_t endIndex)
{
  if (_oldItin)
    return true;

  return Tax::validateTransferTypeLoc1(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxOnChangeFee::validateTripTypes(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t& startIndex,
                                  uint16_t& endIndex)
{
  if (handleHiddenPoints() != HIDDEN_POINT_NOT_HANDLED)
  {
    if (_oldItin)
      return true;
    else
      return validateFromToWithHiddenPoints(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  TripTypesValidator tripTypesValidator;

  if( _oldItin )
    tripTypesValidator.setTravelSeg( &getTravelSeg(taxResponse) );

  return tripTypesValidator.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}


bool
TaxOnChangeFee::validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  class TaxCodeValidatorOnChangeFee : public TaxCodeValidator
  {
  public:
    void setTicketingDate(const DateTime& date) {_ticketingDate=date;}

  protected:
    bool validateEffectiveDate(PricingTrx& trx, TaxCodeReg& taxCodeReg) override
    {
      if (_ticketingDate.isEmptyDate())
      {
        if (!fallback::SSDSP_1988(&trx))
        {
          if (ItinSelector(trx).isRefundTrx())
            return checkDate(GetTicketingDate(trx).get(), taxCodeReg);
          else
            return TaxCodeValidator::validateEffectiveDate(trx, taxCodeReg);
        }
        else
        {
          return TaxCodeValidator::validateEffectiveDate(trx, taxCodeReg);
        }
      }

      return checkDate(_ticketingDate, taxCodeReg);
    }

    bool checkDate(const DateTime& ticketingDate, TaxCodeReg& taxCodeReg)
    {
      if (UNLIKELY(ticketingDate > taxCodeReg.expireDate()))
        return false;

      if (UNLIKELY((taxCodeReg.effDate() > ticketingDate) ||
          (taxCodeReg.discDate().date() < ticketingDate.date())))
        return false;

      return true;
    }

    bool isTaxOnChangeFee(PricingTrx& trx, TaxCodeReg& taxCodeReg) const override
    {
      if (_ticketingDate.isEmptyDate())
        return TaxCodeValidator::isTaxOnChangeFee(trx, taxCodeReg);

      return utc::isTaxOnChangeFee(trx, taxCodeReg, _ticketingDate);
    }

    DateTime _ticketingDate = DateTime::emptyDate();
  };

  TaxCodeValidatorOnChangeFee taxCodeValidator;

  if( _oldItin )
  {
    taxCodeValidator.setTravelSeg( &getTravelSeg(taxResponse) );

    if( trx.isExchangeTrx() )
    {
      const BaseExchangeTrx* exchangeTrx = dynamic_cast<const BaseExchangeTrx*>(&trx);

      if (exchangeTrx && exchangeTrx->currentTicketingDT().isValid())
        taxCodeValidator.setTicketingDate(exchangeTrx->currentTicketingDT());
    }
  }

  return taxCodeValidator.validateTaxCode(trx, taxResponse, taxCodeReg);
}

void
TaxOnChangeFee::doTaxRange(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           uint16_t& startIndex,
                           uint16_t& endIndex,
                           TaxCodeReg& taxCodeReg)
{
  if( !_oldItin )
    Tax::doTaxRange(trx, taxResponse, startIndex, endIndex, taxCodeReg);
}

bool
TaxOnChangeFee::validateRange(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex)
{
  if( _oldItin )
    return true;

  return Tax::validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

MoneyAmount
TaxOnChangeFee::fareAmountInNUCForCurrentSegment(PricingTrx& trx,
    const FarePath* farePath,
    const TaxCodeReg& taxCodeReg) const
{
  if( _oldItin )
    return 0;

  return fareAmountInNUCForCurrentSegment(trx, farePath, taxCodeReg);
}

bool
TaxOnChangeFee::validateTransit(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t travelSegIndex)
{
  if( _oldItin )
    return true;

  return Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
}

bool
TaxOnChangeFee::validateCarrierExemption(PricingTrx& trx,
                                         TaxResponse& taxResponse,
                                         TaxCodeReg& taxCodeReg,
                                         uint16_t travelSegIndex)
{
  if( _oldItin )
    return true;

  return Tax::validateCarrierExemption(trx, taxResponse, taxCodeReg, travelSegIndex);
}

bool
TaxOnChangeFee::validateFareClass(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t travelSegIndex)
{
  if( _oldItin )
    return true;

  return Tax::validateFareClass(trx, taxResponse, taxCodeReg, travelSegIndex);
}

bool
TaxOnChangeFee::validateEquipmentExemption(PricingTrx& trx,
                                           TaxResponse& taxResponse,
                                           TaxCodeReg& taxCodeReg,
                                           uint16_t travelSegIndex)
{
  if( _oldItin )
    return true;

  return Tax::validateEquipmentExemption(trx, taxResponse, taxCodeReg, travelSegIndex);
}

bool
TaxOnChangeFee::validateCabin(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t travelSegIndex)
{
  if( _oldItin )
    return true;

  return Tax::validateCabin(trx, taxResponse, taxCodeReg, travelSegIndex);
}

const std::vector<TravelSeg*>&
TaxOnChangeFee::getTravelSeg(const TaxResponse& taxResponse) const
{
  return _oldItin ? _oldItin->travelSeg() : taxResponse.farePath()->itin()->travelSeg();
}

bool
TaxOnChangeFee::shouldCheckTravelDate() const
{
  return static_cast<bool>(!_oldItin);
}

} // namespace tse
