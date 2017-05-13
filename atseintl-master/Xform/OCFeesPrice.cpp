// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/CurrencyRoundingUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/SubCodeInfo.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "Xform/OCFeesPrice.h"

namespace tse
{
const size_t TAX_ON_OC_BUFF_SIZE = 15;

OCFeesPrice*
OCFeesPrice::create(const OCFees& ocFees, const PricingTrx& trx, DataHandle& dataHandle)
{
  if (ocFees.isBackingOutTaxes())
    return &dataHandle.safe_create<BackingOutTaxesOCFeesPrice>(trx);
  else
    return &dataHandle.safe_create<RegularOCFeesPrice>(trx);
}

OCFeesPrice*
OCFeesPrice::create(const OCFeesUsage& ocFeesUsage, const PricingTrx& trx, DataHandle& dataHandle)
{
  if (ocFeesUsage.isBackingOutTaxes())
    return &dataHandle.safe_create<BackingOutTaxesOCFeesPrice>(trx);
  else
    return &dataHandle.safe_create<RegularOCFeesPrice>(trx);
}

MoneyAmount
OCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
    const Money& targetMoney)
{
  return getFeeAmountInSellingCurrencyPlusTaxes(ocFees, targetMoney.value());
}

MoneyAmount
OCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
    const MoneyAmount& moneyAmount)
{
  return getFeeAmountInSellingCurrencyPlusTaxes(*ocFeesUsage.oCFees(), moneyAmount);
}

MoneyAmount
OCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
    const Money& targetMoney)
{
  return getFeeAmountInSellingCurrencyPlusTaxes(*ocFeesUsage.oCFees(), targetMoney);
}

MoneyAmount
OCFeesPrice::getBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& feeAmount)
{
  return getBasePrice(*ocFeesUsage.oCFees(), feeAmount);
}

MoneyAmount
OCFeesPrice::getEquivalentBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& amount)
{
  return getEquivalentBasePrice(*ocFeesUsage.oCFees(), amount);
}

bool
OCFeesPrice::isExemptAllTaxes()
{
  return _trx.getRequest()->isExemptAllTaxes();
}

bool
OCFeesPrice::isExemptSpecificTaxes()
{
  return _trx.getRequest()->isExemptSpecificTaxes();
}

const std::vector<std::string>&
OCFeesPrice::getTaxIdExempted()
{
  return _trx.getRequest()->taxIdExempted();
}

MoneyAmount
OCFeesPrice::sumUpTaxes(const OCFeesUsage* ocFeesUsage)
{
  return sumUpTaxes(ocFeesUsage->oCFees());
}

MoneyAmount
OCFeesPrice::sumUpTaxes(const OCFees* ocFees)
{
  return sumUpTaxes(ocFees->getTaxes(), TaxItemFeeRetriever());
}

MoneyAmount
OCFeesPrice::sumUpTaxes(const std::vector<OCFees::TaxItem>& taxItems,
    const boost::function<MoneyAmount(const OCFees::TaxItem&)>& taxAmountCalc)
{
  MoneyAmount result = 0.0;

  if (taxItems.empty())
    return result;

  std::vector<OCFees::TaxItem>::const_iterator iTaxItem = taxItems.begin();
  std::vector<OCFees::TaxItem>::const_iterator iEndTaxItem = setEndTaxOnOcIterator(taxItems);

  for (; iTaxItem != iEndTaxItem; iTaxItem++)
  {
    if (isTaxExempted(iTaxItem->getTaxCode()))
      continue;

    result += taxAmountCalc(*iTaxItem);
  }

  return result;
}

bool
OCFeesPrice::isTaxExempted(const std::string& taxCode)
{
  if (isExemptAllTaxes())
    return true;

  if (isExemptSpecificTaxes())
  {
    if (getTaxIdExempted().empty())
      return true;

    std::vector<std::string>::const_iterator taxIdExemptedI = getTaxIdExempted().begin();
    std::vector<std::string>::const_iterator taxIdExemptedEndI = getTaxIdExempted().end();

    for (; taxIdExemptedI != taxIdExemptedEndI; taxIdExemptedI++)
    {
      if (taxCode.compare(0, taxIdExemptedI->size(), *taxIdExemptedI) == 0)
        return true;
    }
  }

  return false;
}

std::vector<OCFees::TaxItem>::const_iterator
OCFeesPrice::setEndTaxOnOcIterator(const std::vector<OCFees::TaxItem>& taxItems)
{
  std::vector<OCFees::TaxItem>::const_iterator iEndTaxItem;

  if (taxItems.size() > TAX_ON_OC_BUFF_SIZE)
  {
    iEndTaxItem = taxItems.begin();
    advance(iEndTaxItem, TAX_ON_OC_BUFF_SIZE);
  }
  else
    iEndTaxItem = taxItems.end();

  return iEndTaxItem;
}

MoneyAmount
RegularOCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
    const Money& targetMoney)
{
  return targetMoney.value() + OCFeesPrice::sumUpTaxes(&ocFees);
}

MoneyAmount
RegularOCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
    const MoneyAmount& moneyAmount)
{
  return moneyAmount + sumUpTaxes(&ocFeesUsage);
}

MoneyAmount
RegularOCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
    const MoneyAmount& moneyAmount)
{
  return moneyAmount + OCFeesPrice::sumUpTaxes(&ocFees);
}

MoneyAmount
RegularOCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
    const Money& targetMoney)
{
  return targetMoney.value() + sumUpTaxes(&ocFeesUsage);
}

MoneyAmount
RegularOCFeesPrice::getBasePrice(const OCFees& /* ocFees */, const MoneyAmount& feeAmount)
{
  return feeAmount;
}

MoneyAmount
RegularOCFeesPrice::getEquivalentBasePrice(const OCFees& /* ocFees */, const MoneyAmount& amount)
{
  return amount;
}

MoneyAmount
RegularOCFeesPrice::getBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& feeAmount)
{
  return getBasePrice(*ocFeesUsage.oCFees(), feeAmount);
}

MoneyAmount
RegularOCFeesPrice::getEquivalentBasePrice(const OCFeesUsage& ocFeesUsage, const MoneyAmount& amount)
{
  return getEquivalentBasePrice(*ocFeesUsage.oCFees(), amount);
}

MoneyAmount
RegularOCFeesPrice::sumUpTaxes(const OCFeesUsage* ocFeesUsage)
{
  return OCFeesPrice::sumUpTaxes(ocFeesUsage->getTaxes(), TaxItemFeeRetriever());
}



MoneyAmount
BackingOutTaxesOCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
    const Money& /* targetMoney */)
{
  return ocFees.getBackingOutTaxes().feeAmountInSellingCurrencyPlusTaxes();
}

MoneyAmount
BackingOutTaxesOCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesUsage,
    const MoneyAmount& /* moneyAmount */)
{
  return ocFeesUsage.getBackingOutTaxes().feeAmountInSellingCurrencyPlusTaxes();
}

MoneyAmount
BackingOutTaxesOCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFees& ocFees,
    const MoneyAmount& /* moneyAmount */)
{
  return ocFees.getBackingOutTaxes().feeAmountInSellingCurrencyPlusTaxes();
}

MoneyAmount
BackingOutTaxesOCFeesPrice::getFeeAmountInSellingCurrencyPlusTaxes(const OCFeesUsage& ocFeesFees,
    const Money& /* targetMoney */)
{
  return ocFeesFees.getBackingOutTaxes().feeAmountInSellingCurrencyPlusTaxes();
}

MoneyAmount
BackingOutTaxesOCFeesPrice::getBasePrice(const OCFees& ocFees,
    const MoneyAmount& /* moneyAmount */)
{
  return ocFees.getBackingOutTaxes().feeAmountInFeeCurrency();
}

MoneyAmount
BackingOutTaxesOCFeesPrice::getEquivalentBasePrice(const OCFees& ocFees,
    const MoneyAmount& /* moneyAmount */)
{
  return ocFees.getBackingOutTaxes().feeAmountInSellingCurrency();
}

MoneyAmount
BackingOutTaxesOCFeesPrice::getBasePrice(const OCFeesUsage& ocFeesUsage,
    const MoneyAmount& /* moneyAmount */)
{
  return ocFeesUsage.getBackingOutTaxes().feeAmountInFeeCurrency();
}

MoneyAmount
BackingOutTaxesOCFeesPrice::getEquivalentBasePrice(const OCFeesUsage& ocFeesUsage,
    const MoneyAmount& /* moneyAmount */)
{
  return ocFeesUsage.getBackingOutTaxes().feeAmountInSellingCurrency();
}

MoneyAmount
BackingOutTaxesOCFeesPrice::sumUpTaxes(const OCFeesUsage* ocFeesUsage)
{
  return OCFeesPrice::sumUpTaxes(ocFeesUsage->getTaxes(), TaxItemFeeRetriever());
}

}
