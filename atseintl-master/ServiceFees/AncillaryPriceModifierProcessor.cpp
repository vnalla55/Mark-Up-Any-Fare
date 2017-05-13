//-------------------------------------------------------------------
//  Copyright Sabre 2016
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
#include "ServiceFees/AncillaryPriceModifierProcessor.h"

#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Itin.h"
#include "ServiceFees/ServiceFeesGroup.h"


namespace tse
{

int priceModificationSign(AncillaryPriceModifier::Type type)
{
  return (type == AncillaryPriceModifier::Type::DISCOUNT) ? -1: 1;
}

AncillaryPriceModifierProcessor::AncillaryPriceModifierProcessor(AncillaryPricingTrx& trx, Itin& itin, OCFees::AmountRounder& amountRounder)
  : _trx(trx), _itin(itin), _amountRounder(amountRounder)
{
}

void AncillaryPriceModifierProcessor::processGroups(const std::vector<ServiceFeesGroup*>& serviceFeesGroups)
{
  for (const auto serviceFeesGroup : serviceFeesGroups)
    for (const auto ocFeesGroup : serviceFeesGroup->ocFeesMap())
      for (const auto ocFees : ocFeesGroup.second)
          processOcFees(*ocFees);
}

void AncillaryPriceModifierProcessor::processOcFees(OCFees& ocFees)
{
  auto storedSeg = ocFees.getCurrentSeg();
  for (size_t i = ocFees.segCount(); i > 0; --i)
  {
    ocFees.setSeg(i - 1);
    AncillaryIdentifier aid(ocFees);

    if (!_trx.isSecondCallForMonetaryDiscount())
      ocFees.getCurrentSeg()->_ancPriceModification = std::make_pair(aid, AncillaryPriceModifier());

    auto ancillaryPriceModifierIt = _itin.getAncillaryPriceModifiers().find(aid);
    if (ancillaryPriceModifierIt != _itin.getAncillaryPriceModifiers().end())
      for (auto ancPriceModifier : ancillaryPriceModifierIt->second)
        addModifiedSegment(ocFees, aid, ancPriceModifier);
  }
  ocFees.setCurrentSeg(storedSeg);
}

void AncillaryPriceModifierProcessor::addModifiedSegment(OCFees& ocFees, const AncillaryIdentifier aid, const AncillaryPriceModifier& ancPriceModifier)
{
  OCFees::OCFeesSeg* newSeg = _trx.dataHandle().create<OCFees::OCFeesSeg>();
  *newSeg = *ocFees.getCurrentSeg();
  newSeg->_ancPriceModification = std::make_pair(aid, ancPriceModifier);

  auto priceModificationType = ancPriceModifier._type.get_value_or(AncillaryPriceModifier::Type::DISCOUNT);

  auto priceWithPercentage = applyPercentageModification(Money(newSeg->_feeAmount,
                                                               newSeg->_feeCurrency),
                                                         priceModificationType,
                                                         newSeg->_ancPriceModification.get().second._percentage);

  auto newPrice = priceWithPercentage ? priceWithPercentage : applyMonetaryModification(Money(newSeg->_feeAmount,
                                                                                              newSeg->_feeCurrency),
                                                                                        priceModificationType,
                                                                                        newSeg->_ancPriceModification.get().second._money);
  if (newPrice)
  {
    newSeg->_feeAmount = roundFeeAmount(newPrice.get().value(), newPrice.get().code());
    newSeg->_feeCurrency = newPrice.get().code();
  }

  applyQuantity(newSeg);
  ocFees.segments().push_back(newSeg);
}

void AncillaryPriceModifierProcessor::applyQuantity(OCFees::OCFeesSeg* ocFeesSeg)
{
  ocFeesSeg->_feeAmount *= ocFeesSeg->_ancPriceModification.get().second._quantity;
}

boost::optional<Money>
AncillaryPriceModifierProcessor::applyPercentageModification(const Money& feePrice,
                                                             AncillaryPriceModifier::Type type,
                                                             boost::optional<unsigned int> percentage)
{
  return percentage ? Money(feePrice.value() * (1.0 + (double)priceModificationSign(type) * percentage.get() / 100),
                            feePrice.code())
                    : boost::optional<Money>();
}

double clampToZero(double value)
{
  return value < 0.0 ? 0.0 : value;
}

bool isInvalidOrNotSet(const CurrencyCode& currency)
{
  return currency.empty() || currency == INVALID_CURRENCYCODE;
}

MoneyAmount
AncillaryPriceModifierProcessor::inTargetCurrency(Money money, CurrencyCode targetCurrency)
{
  Money target(money.value(), targetCurrency);

  if (money.code() != targetCurrency && !CurrencyConversionFacade().convert(target, money, _trx, false, CurrencyConversionRequest::TAXES))
  {
    throw ErrorResponseException(ErrorResponseException::CANNOT_CALCULATE_CURRENCY,
                                 "Can't convert currency " + money.code() + " to " + targetCurrency);

  }

  return target.value();
}

boost::optional<Money>
AncillaryPriceModifierProcessor::applyMonetaryModification(const Money& feePrice,
                                                           AncillaryPriceModifier::Type type,
                                                           boost::optional<Money> money)
{
  if (!money)
    return boost::optional<Money>();

  CurrencyCode targetCurrency = isInvalidOrNotSet(feePrice.code()) ? money.get().code() : feePrice.code();

  return Money(clampToZero(feePrice.value() + inTargetCurrency(money.get(), targetCurrency) * priceModificationSign(type)),
               targetCurrency);
}

MoneyAmount AncillaryPriceModifierProcessor::roundFeeAmount(const MoneyAmount& feeAmount, const CurrencyCode& feeCurrency)
{
  return _amountRounder.getRoundedFeeAmount(Money(feeAmount, feeCurrency));
}

}
