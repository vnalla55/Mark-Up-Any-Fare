// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Currency.h"
#include "FareCalc/CalcTotals.h"
#include "Xform/ComparablePenaltyFee.h"
#include "Xform/ReissueExchangeModel.h"
#include "Xform/TaxBreakdownModel.h"

namespace tse
{

std::unique_ptr<ReissueExchangeModel>
ReissueExchangeModel::create(const RefundPricingTrx& trx)
{
  return std::unique_ptr<ReissueExchangeModel>(
      new ReissueExchangeModel(*trx.fullRefundWinningPermutation(),
                               trx.exchangeItin().front()->farePath().front()->pricingUnit(),
                               trx));
}

std::unique_ptr<ReissueExchangeModel>
ReissueExchangeModel::create(const RefundPricingTrx& trx, CalcTotals& calcTotals)
{
  return std::unique_ptr<ReissueExchangeModel>(
      new ReissueExchangeModel(*calcTotals.farePath->lowestFee33Perm(),
                               trx.exchangeItin().front()->farePath().front()->pricingUnit(),
                               trx));
}

ReissueExchangeModel::ReissueExchangeModel(const RefundPermutation& perm,
    const PricingUnits& pricingUnits, const PricingTrx& trx) :
    _winnerPerm(perm), _pricingUnits(pricingUnits), _trx(trx)
{

  for(TaxResponse* taxResponse : _trx.taxResponse())
  {
    for(TaxItem* taxItem : taxResponse->changeFeeTaxItemVector())
    {
      _taxesOnChangeFeeBreakdown.emplace_back(new TaxBreakdownModel(trx, *taxItem));
    }
  }
}

Indicator
ReissueExchangeModel::getFormOfRefund() const
{
  return _winnerPerm.formOfRefundInd();
}

bool
ReissueExchangeModel::getReissueTaxRefundable() const
{
  return _winnerPerm.taxRefundable();
}

bool
ReissueExchangeModel::isRefundable() const
{
  return _winnerPerm.refundable(_pricingUnits);
}

const RefundPermutation&
ReissueExchangeModel::getWinnerPermutation() const
{
  return _winnerPerm;
}

const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
ReissueExchangeModel::getTaxesOnChangeFee() const
{
  return _taxesOnChangeFeeBreakdown;
}

DateTime&
AbstractChangeFeeModel::getTicketingDate() const
{
  RexBaseRequest* rexBaseRequest = dynamic_cast<RexBaseRequest*>(_trx.getRequest());
  if (rexBaseRequest == nullptr)
    throw std::runtime_error("RexBaseRequest is NULL");

  return rexBaseRequest->getTicketingDT();
}

CurrencyCode
AbstractChangeFeeModel::getPaymentCurrency() const
{
  if (!_trx.getOptions()->currencyOverride().empty())
    return _trx.getOptions()->currencyOverride();

  return _trx.getRequest()->ticketingAgent()->currencyCodeAgent();
}

MoneyAmount
AbstractChangeFeeModel::getAmountInPaymentCurrency() const
{
  if (getChangeFeeCurrency() == getPaymentCurrency())
    return getAmount();

  return CurrencyUtil::convertMoneyAmount(getAmount(),
                                          getChangeFeeCurrency(),
                                          getPaymentCurrency(),
                                          getTicketingDate(),
                                          _trx,
                                          CurrencyConversionRequest::FARES);
}

CurrencyNoDec
AbstractChangeFeeModel::getPaymentPrecision() const
{
  if (getPaymentCurrency() != NUC)
  {
    const Currency* currency = _trx.dataHandle().getCurrency(getPaymentCurrency());
    if (currency)
      return currency->noDec();
  }
  return Money::NUC_DECIMALS;
}

bool
AbstractChangeFeeModel::isChangeFeeNotApplicable() const
{
  throw std::runtime_error("Not Supported");
}

bool
AbstractChangeFeeModel::isChangeFeeWaived() const
{
  throw std::runtime_error("Not Supported");
}

bool
AbstractChangeFeeModel::isRefundable() const
{
  throw std::runtime_error("Not Supported");
}

CurrencyCode
AbstractChangeFeeModel::getChangeFeeCurrency() const
{
  throw std::runtime_error("Not Supported");
}

ChangeFeeModel::ChangeFeeModel(const ComparablePenaltyFee& fee, RexPricingTrx& trx)
  : AbstractChangeFeeModel(trx), _fee(fee), _trx(trx)
{
}

bool
ChangeFeeModel::isSkipped() const
{
  return false;
}

MoneyAmount
ChangeFeeModel::getAmount() const
{
  return _fee.penaltyAmount;
}

CurrencyNoDec
ChangeFeeModel::getPrecision() const
{
  if (_fee.penaltyCurrency != NUC)
  {
    const Currency* currency = _trx.dataHandle().getCurrency(_fee.penaltyCurrency);
    if (currency)
      return currency->noDec();
  }

  return Money::NUC_DECIMALS;
}

bool
ChangeFeeModel::isZeroFee() const
{
  return false;
}

bool
ChangeFeeModel::isNonZeroFee() const
{
  return false;
}

bool
ChangeFeeModel::isChangeFeeNotApplicable() const
{
  return _fee.notApplicable;
}

bool
ChangeFeeModel::isChangeFeeWaived() const
{
  return _fee.waived;
}

bool
ChangeFeeModel::isHighestChangeFeeIndicator() const
{
  return _fee.highest;
}

CurrencyCode
ChangeFeeModel::getChangeFeeCurrency() const
{
  return _fee.penaltyCurrency;
}

ZeroFeeChangeFeeModel::ZeroFeeChangeFeeModel(const RefundPermutation& winnerPerm, PricingTrx& trx)
  : AbstractChangeFeeModel(trx), _permutation(winnerPerm)
{
}

bool
ZeroFeeChangeFeeModel::isSkipped() const
{
  return false;
}

MoneyAmount
ZeroFeeChangeFeeModel::getAmount() const
{
  return _permutation.totalPenalty().value();
}

CurrencyNoDec
ZeroFeeChangeFeeModel::getPrecision() const
{
  return tse::Money::NUC_DECIMALS;
}

bool
ZeroFeeChangeFeeModel::isZeroFee() const
{
  return true;
}

bool
ZeroFeeChangeFeeModel::isNonZeroFee() const
{
  return false;
}

bool
ZeroFeeChangeFeeModel::isChangeFeeNotApplicable() const
{
  return !_permutation.waivedPenalty();
}

bool
ZeroFeeChangeFeeModel::isChangeFeeWaived() const
{
  return _permutation.waivedPenalty();
}

bool
ZeroFeeChangeFeeModel::isHighestChangeFeeIndicator() const
{
  return true;
}

CurrencyCode
ZeroFeeChangeFeeModel::getChangeFeeCurrency() const
{
  return _permutation.totalPenalty().code();
}

bool
ZeroFeeChangeFeeModel::isRefundable() const
{
  return true;
}

NonZeroFeeChangeFeeModel::NonZeroFeeChangeFeeModel(const RefundPenalty::Fee& fee, PricingTrx& trx)
  : AbstractChangeFeeModel(trx), _amount(fee.amount())
{
  _highestChangeFee = fee.highest();
  _refundable = !fee.nonRefundable();
}

NonZeroFeeChangeFeeModel::NonZeroFeeChangeFeeModel(const RefundPermutation& winnerPerm,
                                                   bool highestChangeFee,
                                                   bool refundable,
                                                   PricingTrx& trx)
  : AbstractChangeFeeModel(trx),
    _highestChangeFee(highestChangeFee),
    _refundable(refundable),
    _amount(winnerPerm.minimumPenalty())
{
}

bool
NonZeroFeeChangeFeeModel::isSkipped() const
{
  return getAmount() <= EPSILON;
}

MoneyAmount
NonZeroFeeChangeFeeModel::getAmount() const
{
  return _amount.value();
}

CurrencyNoDec
NonZeroFeeChangeFeeModel::getPrecision() const
{
  return _amount.noDec();
}

bool
NonZeroFeeChangeFeeModel::isZeroFee() const
{
  return false;
}

bool
NonZeroFeeChangeFeeModel::isNonZeroFee() const
{
  return true;
}

bool
NonZeroFeeChangeFeeModel::isHighestChangeFeeIndicator() const
{
  return _highestChangeFee;
}

CurrencyCode
NonZeroFeeChangeFeeModel::getChangeFeeCurrency() const
{
  return _amount.code();
}

bool
NonZeroFeeChangeFeeModel::isRefundable() const
{
  return _refundable;
}

} // end of tse namespace
