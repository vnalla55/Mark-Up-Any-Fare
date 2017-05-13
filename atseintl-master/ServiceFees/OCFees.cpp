//-------------------------------------------------------------------
//
//  File:        OCFees.cpp
//
//  Description:
//
//  Copyright Sabre 2009
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

#include "ServiceFees/OCFees.h"

#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxNation.h"

namespace tse
{

namespace
{
Logger logger("atseintl.ServiceFees.OCFees");
}

OCFees::OCFees(const OCFees& ocfees)
  : _carrier(ocfees._carrier),
    _travelStart(ocfees._travelStart),
    _travelEnd(ocfees._travelEnd),
    _subCodeInfo(ocfees._subCodeInfo),
    _ocFeesSeg(ocfees._ocFeesSeg),
    _failS6(ocfees._failS6),
    _emdChargeIndicator(ocfees._emdChargeIndicator)
{
}

void
OCFees::addSeg(DataHandle& dh)
{
  _curSeg = dh.create<OCFeesSeg>();
  _segs.push_back(_curSeg);
}

void
OCFees::cleanOutCurrentSeg()
{
  _curSeg->_optFee = nullptr;
  _curSeg->_feeAmount = 0;
  _curSeg->_feeCurrency = "";
  _curSeg->_displayAmount = 0;
  _curSeg->_displayCurrency = "";
  _curSeg->_feeNoDec = 0;
  _curSeg->_farePath = nullptr;
  _curSeg->_displayOnly = false;
  _curSeg->_feeGuaranteed = true;
  _curSeg->_purchaseByDate = time(nullptr);
  _curSeg->_softMatchS7Status.setNull();
  _curSeg->_resultingFareClass.clear();
  _curSeg->_carrierFlights.clear();
  _curSeg->_rbdData.clear();
}

void
OCFees::cleanBaggageResults()
{
  cleanOutCurrentSeg();
  _bagSoftPass.setNull();
}

void
OCFees::OCFeesSeg::setBackingOutTaxes(const MoneyAmount& feeAmountInFeeCurrency,
    const MoneyAmount& feeAmountInSellingCurrency,
    const MoneyAmount& feeAmountInSellingCurrencyPlusTaxes)
{
  if (!_backingOutTaxesData)
  {
    _backingOutTaxesData = BackingOutTaxes(feeAmountInFeeCurrency, feeAmountInSellingCurrency,
        feeAmountInSellingCurrencyPlusTaxes);

    _feeAmount = feeAmountInFeeCurrency;
  }
}

bool
OCFees::OcAmountRounder::getFeeRounding(const CurrencyCode& currencyCode,
                                        RoundingFactor& roundingFactor,
                                        CurrencyNoDec& roundingNoDec,
                                        RoundingRule& roundingRule) const
{
  const Currency* currency = this->getCurrency(currencyCode);

  if (UNLIKELY(!currency))
  {
    LOG4CXX_ERROR(logger, "DBAccess getCurrency returned null currency pointer");
    return false;
  }

  if (currency->taxOverrideRoundingUnit() > 0)
  {
    roundingFactor = currency->taxOverrideRoundingUnit();
    roundingNoDec = currency->taxOverrideRoundingUnitNoDec();
    roundingRule = currency->taxOverrideRoundingRule();

    return true;
  }

  const std::string controllingEntityDesc = currency->controllingEntityDesc();
  LOG4CXX_INFO(logger, "Currency country description: " << currency->controllingEntityDesc());

  bool foundNationalCurrency = false;
  bool foundNation = false;
  NationCode nationWithMatchingNationalCurrency;
  NationCode nationCode;

  CurrencyUtil::getControllingNationCode(_trx,
                                         controllingEntityDesc,
                                         nationCode,
                                         foundNation,
                                         foundNationalCurrency,
                                         nationWithMatchingNationalCurrency,
                                         _trx.ticketingDate(),
                                         currencyCode);

  if (LIKELY(foundNation))
  {
    const TaxNation* taxNation = getTaxNation(nationCode);

    if (LIKELY(taxNation))
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      return true;
    }
  }
  else if (foundNationalCurrency)
  {
    const TaxNation* taxNation = getTaxNation(nationWithMatchingNationalCurrency);

    if (taxNation)
    {
      roundingFactor = taxNation->roundingUnit();
      roundingNoDec = taxNation->roundingUnitNodec();
      roundingRule = taxNation->roundingRule();

      return true;
    }
  }
  return false;
}

const Currency*
OCFees::OcAmountRounder::getCurrency(const CurrencyCode& currencyCode) const
{
  return _trx.dataHandle().getCurrency( currencyCode );
}

const TaxNation*
OCFees::OcAmountRounder::getTaxNation(const NationCode& nationCode) const
{
  return _trx.dataHandle().getTaxNation(nationCode, _trx.ticketingDate());
}


MoneyAmount
OCFees::OcAmountRounder::getRoundedFeeAmount(const Money &amount) const
{
  Money roundedAmount = amount;
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;

  if (LIKELY(getFeeRounding(roundedAmount.code(), roundingFactor, roundingNoDec, roundingRule)))
  {
    CurrencyConverter curConverter;
    curConverter.round(roundedAmount, roundingFactor, roundingRule);
  }

  return roundedAmount.value();
}

MoneyAmount
OCFees::BaggageAmountRounder::getRoundedFeeAmount(const Money &amount) const
{
  Money roundedAmount = amount;
  CurrencyRoundingUtil roundingUtil;
  roundingUtil.round(roundedAmount, _trx);

  return roundedAmount.value();
}

}
