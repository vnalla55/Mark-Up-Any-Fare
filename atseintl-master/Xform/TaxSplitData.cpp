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

#include "Common/MCPCarrierUtil.h"
#include "Xform/TaxSplitData.h"

#include <string>

namespace tse
{

const CurrencyNoDec
TaxSplitData::getCurrencyNoDec() const
{
  return _taxItem.paymentCurrencyNoDec();
}

const MoneyAmount&
TaxSplitData::getTaxValue() const
{
  return _taxValue;
}

void
TaxSplitData::setTaxValue(const MoneyAmount& taxValue)
{
  _taxValue = taxValue;
}

void
TaxSplitData::computeAndStoreTaxValue()
{
  if (_taxItem.multioccconvrndInd() == YES)
  {
    for (const TaxRecord* taxRecord : _splitTaxInfo.taxRecords)
    {
      if (taxRecord->taxCode() == _taxItem.taxCode())
      {
        _taxValue = taxRecord->getTaxAmount();
        return;
      }
    }
  }

  _taxValue =  _taxItem.taxAmount();
}

std::pair<MoneyAmount, CurrencyNoDec>
TaxSplitData::getAmountPublished() const
{
  MoneyAmount amountPublished = _taxItem.taxAmt();
  Money target(_taxItem.taxCur());
  CurrencyNoDec amountPublishedNoDec = target.noDec(_pricingTrx.ticketingDate());

  if (_taxItem.taxType() == Tax::PERCENTAGE)
  {
    Money source(_taxItem.taxAmount(), _taxItem.paymentCurrency());
    TaxUtil::convertTaxCurrency(_pricingTrx, source, target);
    amountPublished = target.value();
  }

  return std::make_pair(amountPublished, amountPublishedNoDec);
}

CarrierCode
TaxSplitData::getTaxAirlineCode() const
{
  return MCPCarrierUtil::swapToPseudo(&_pricingTrx, _taxItem.carrierCode());
}

}
