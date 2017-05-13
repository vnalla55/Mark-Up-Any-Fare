//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "AncillaryPricingResponseSumDataFiller.h"

#include "Common/CurrencyRoundingUtil.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Xform/PricingResponseXMLTags.h"

namespace tse
{
AncillaryPricingResponseSumDataFiller::
AncillaryPricingResponseSumDataFiller(AncillaryPricingTrx* ancTrx,
                                      XMLConstruct* construct,
                                      const OCFeesUsage &ocFees,
                                      const DateTime* ticketingDate,
                                      AncillaryPricingResponseFormatter* amTaxLogicApplier,
                                      MoneyAmount feeAmount,
                                      bool arePackedDataProvided,
                                      PaxTypeCode paxType)
                                      : _ancTrx(ancTrx),
                                        _construct(construct),
                                        _ocFees(ocFees),
                                        _ticketingDate(ticketingDate),
                                        _amTaxLogicApplier(amTaxLogicApplier),
                                        _feeAmount(feeAmount),
                                        _areProvidedDataPacked(arePackedDataProvided),
                                        _paxType(paxType)
{
  _isBaggageRequested = _ancTrx->getRequest()->majorSchemaVersion() >= 2 &&
                        ServiceFeeUtil::checkServiceGroupForAcs(_ocFees.subCodeInfo()->serviceGroup());
  _ocFeesPrice = OCFeesPrice::create(_ocFees, *_ancTrx, _dataHandle);
  _basePrice = _areProvidedDataPacked ? _feeAmount : _ocFeesPrice->getBasePrice(_ocFees, _feeAmount);
}

AncillaryPricingResponseSumDataFiller::
AncillaryPricingResponseSumDataFiller(AncillaryPricingTrx* ancTrx,
                                      XMLConstruct* construct,
                                      const PaxOCFees* paxOcFees,
                                      OCFeesUsage* ocFees,
                                      AncillaryPricingResponseFormatter* amTaxLogicApplier)
                                      : AncillaryPricingResponseSumDataFiller(ancTrx,
                                                                              construct,
                                                                              *ocFees,
                                                                              &ancTrx->ticketingDate(),
                                                                              amTaxLogicApplier,
                                                                              ocFees->feeAmount(),
                                                                              false,
                                                                              paxOcFees->paxType())
{
}

AncillaryPricingResponseSumDataFiller::
AncillaryPricingResponseSumDataFiller(AncillaryPricingTrx* ancTrx,
                                     XMLConstruct* construct,
                                     const PaxOCFeesUsages* paxOcFeesUsages,
                                     AncillaryPricingResponseFormatter* amTaxLogicApplier)
                                     : AncillaryPricingResponseSumDataFiller(ancTrx,
                                                                             construct,
                                                                             *paxOcFeesUsages->fees(),
                                                                             &ancTrx->ticketingDate(),
                                                                             amTaxLogicApplier,
                                                                             paxOcFeesUsages->fees()->feeAmount(),
                                                                             true,
                                                                             paxOcFeesUsages->paxType())
{
}

void AncillaryPricingResponseSumDataFiller::fillSumElement()
{
  _construct->openElement(xml2::SUMPsgrFareInfo);
  insertDataToSum();
  _construct->closeElement();
}

void AncillaryPricingResponseSumDataFiller::insertDataToSum()
{
  insertPassengerData();
  if (_ancTrx->activationFlags().isMonetaryDiscount())
    insertQuantityData();
  calculateEquivPrice();
  insertBasePriceData();
  insertEquivPriceData();
  insertTaxIndicatorData();
  applyAmTaxLogic();
  insertTotalPriceData();
}

void AncillaryPricingResponseSumDataFiller::insertPassengerData()
{
  _construct->addAttribute(xml2::PsgrTypeCode, _paxType);
}

void AncillaryPricingResponseSumDataFiller::insertQuantityData()
{
  _construct->addAttributeUInteger(xml2::AncillaryQuantity, _ocFees.getAncillaryPriceModifier()._quantity);
}

void AncillaryPricingResponseSumDataFiller::calculateEquivPrice()
{
  ServiceFeeUtil util(*_ancTrx);
  const bool isFlightRelatedService = _ocFees.subCodeInfo()->fltTktMerchInd() == FLIGHT_RELATED_SERVICE;
  const bool isPrepaidBaggage = _ocFees.subCodeInfo()->fltTktMerchInd() == PREPAID_BAGGAGE;

  if (_ancTrx->activationFlags().isAB240() && (isPrepaidBaggage || isFlightRelatedService))
  {
    _equivPrice = util.convertOCFeeCurrency(_ocFees);
  }
  else if (_isBaggageRequested)
  {
    _equivPrice = util.convertBaggageFeeCurrency(_ocFees);
    if (ServiceFeeUtil::isFeeFarePercentage(*_ocFees.optFee()))
    {
      CurrencyRoundingUtil roundingUtil;
      roundingUtil.round(_feeAmount, _ocFees.feeCurrency(), *_ancTrx);
    }
  }
  else
  {
    _equivPrice = util.convertOCFeeCurrency(_ocFees);
  }
}

void AncillaryPricingResponseSumDataFiller::insertBasePriceData()
{
  if (_ocFees.feeCurrency() != "")
  {
    _currency = _ocFees.feeCurrency();
    _basePriceDecimalPlacesCount = _ocFees.feeNoDec();

    // C51 - Base Price
    _construct->addAttributeDouble(xml2::SUMBasePrice, _ocFeesPrice->getBasePrice(_ocFees, _feeAmount),
                                   _basePriceDecimalPlacesCount);

    // C5A - Base Currency
    _construct->addAttribute(xml2::SUMBaseCurrencyCode, _currency);
  }
  else
  {
    _currency = _equivPrice.code();
    _basePriceDecimalPlacesCount = _equivPrice.noDec(*_ticketingDate);

    // C51 - Base Price
    _construct->addAttributeDouble(xml2::SUMBasePrice, _ocFeesPrice->getBasePrice(_ocFees, _feeAmount),
                                   _basePriceDecimalPlacesCount);
  }
}

void AncillaryPricingResponseSumDataFiller::insertEquivPriceData()
{
  if (_ocFees.feeCurrency() != _equivPrice.code() || _basePrice == 0)
  {
    // C52 - SFC  - Equivalent Base Price
    _equivalentPriceDecimalPlacesCount = _equivPrice.noDec(*_ticketingDate);
    _construct->addAttributeDouble(xml2::SUMEquiBasePrice,
                                   _ocFeesPrice->getEquivalentBasePrice(_ocFees, _equivPrice.value()),
                                   _equivalentPriceDecimalPlacesCount);

    // C5B - Equivalent Currency
    _construct->addAttribute(xml2::SUMEquiCurCode, _equivPrice.code());
  }

  _isEquivalentCurrencyAvailable = _ocFees.feeCurrency() != "" && _ocFees.feeCurrency() != _equivPrice.code();
}

void AncillaryPricingResponseSumDataFiller::insertTaxIndicatorData()
{
  // N21 - SFI Tax Indicator
  if (_ocFees.optFee()->taxInclInd() != ' ')
  {
    _construct->addAttributeChar(xml2::TaxInd, _ocFees.optFee()->taxInclInd());
  }
}

void AncillaryPricingResponseSumDataFiller::applyAmTaxLogic()
{
  MoneyAmount equivalentBasePrice = _areProvidedDataPacked ? _equivPrice.value()
                                      : _ocFeesPrice->getEquivalentBasePrice(_ocFees, _equivPrice.value());
  if (_amTaxLogicApplier->isApplyAMTaxLogic(*_ancTrx, _ocFees))
  {
    _amTaxLogicApplier->applyAMTaxLogic(*_ancTrx, _ocFees, _isEquivalentCurrencyAvailable,
                                        _basePrice, equivalentBasePrice, _basePriceDecimalPlacesCount,
                                        _equivalentPriceDecimalPlacesCount, _currency);
  }
}

void AncillaryPricingResponseSumDataFiller::insertTotalPriceData()
{
  // C50 - SFE - Total Price - Place Holder
  if (!_isEquivalentCurrencyAvailable && _isBaggageRequested && !_areProvidedDataPacked)
  {
    _construct->addAttributeDouble(
        xml2::ServiceTotalPrice4OC,
        _ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(_ocFees, _feeAmount),
        _basePriceDecimalPlacesCount);
  }
  else
  {
    _construct->addAttributeDouble(
        xml2::ServiceTotalPrice4OC,
        _ocFeesPrice->getFeeAmountInSellingCurrencyPlusTaxes(_ocFees, _equivPrice),
        _equivPrice.noDec(*_ticketingDate));
  }
}

} // namespace tse
