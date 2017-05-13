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
#include "Common/FallbackUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/ChangeFeeAmount.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

namespace tse
{

FALLBACK_DECL(cat33FixTaxesOnChangeFeeAF);

std::unique_ptr<const AbstractChangeFeeAmount>
AbstractChangeFeeAmount::create(PricingTrx& trx, TaxResponse& taxResponse,
    TaxCodeReg& taxCodeReg, CurrencyCode paymentCurrency)
{
  if (trx.excTrxType() == PricingTrx::AF_EXC_TRX)
    return std::unique_ptr<const AbstractChangeFeeAmount>(
        new RefundChangeFeeAmount(trx, taxResponse, taxCodeReg, paymentCurrency));
  else
    return std::unique_ptr<const AbstractChangeFeeAmount>(
        new ReissueChangeFeeAmount(trx, taxResponse, taxCodeReg, paymentCurrency));
}

RefundChangeFeeAmount::RefundChangeFeeAmount(PricingTrx& trx, TaxResponse& taxResponse,
    TaxCodeReg& taxCodeReg, CurrencyCode paymentCurrency)
    : _trx(trx), _taxResponse(taxResponse), _taxCodeReg(taxCodeReg),
      _paymentCurrency(paymentCurrency)
{
  RefundPricingTrx& refundTrx = static_cast<RefundPricingTrx&>(_trx);

  if (!fallback::cat33FixTaxesOnChangeFeeAF(&_trx))
  {
    if (refundTrx.fullRefund())
    {
      _permutation = refundTrx.fullRefundWinningPermutation();
    }
    else
    {
      _permutation = _taxResponse.farePath()->lowestFee33Perm();
    }
  }
  else
  {
    _permutation = refundTrx.fullRefundWinningPermutation();
  }
}

MoneyAmount
RefundChangeFeeAmount::getAmountInPaymentCurrency() const
{
  if (!validate())
    return 0;

  const Money& penalty = _permutation->totalPenalty();
  if (_paymentCurrency == penalty.code())
    return penalty.value();

  Money convertedPenalty(_paymentCurrency);
  if (!CurrencyConversionFacade().convert(convertedPenalty, penalty, _trx,
      _taxResponse.farePath()->itin()->useInternationalRounding(),
      CurrencyConversionRequest::FARES))
  {
    TaxDiagnostic::collectErrors(_trx, _taxCodeReg, _taxResponse,
        TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
  }

  return convertedPenalty.value();
}

bool
RefundChangeFeeAmount::validate() const
{
  return _permutation && !_permutation->totalPenalty().isZeroAmount();
}

ReissueChangeFeeAmount::ReissueChangeFeeAmount(PricingTrx& trx, TaxResponse& taxResponse,
    TaxCodeReg& taxCodeReg, CurrencyCode paymentCurrency)
  : _trx(trx), _taxResponse(taxResponse), _taxCodeReg(taxCodeReg),
    _paymentCurrency(paymentCurrency)
{
  _reissueCharges = _taxResponse.farePath()->reissueCharges();
}

MoneyAmount
ReissueChangeFeeAmount::getAmountInPaymentCurrency() const
{
  if (!validate())
    return 0.0;

  MoneyAmount result = 0;
  CurrencyConversionFacade ccFacade;

  for (auto& fee : _reissueCharges->penaltyFees)
  {
    PenaltyFee* penaltyFee = fee.second;

    if (_paymentCurrency != penaltyFee->penaltyCurrency)
    {
      Money targetMoney(_paymentCurrency);
      targetMoney.value() = 0;

      Money sourceMoney(penaltyFee->penaltyAmount, penaltyFee->penaltyCurrency);

      if (!ccFacade.convert(targetMoney, sourceMoney, _trx,
          _taxResponse.farePath()->itin()->useInternationalRounding(),
          CurrencyConversionRequest::FARES))
      {
        TaxDiagnostic::collectErrors(_trx, _taxCodeReg, _taxResponse,
            TaxDiagnostic::CURRENCY_CONVERTER_BSR, Diagnostic810);
        continue;
      }
      else
      {
        result += targetMoney.value();
      }
    }
    else
      result += penaltyFee->penaltyAmount;
  }

  return result;
}

bool
ReissueChangeFeeAmount::validate() const
{
  return _reissueCharges && _reissueCharges->changeFee > EPSILON
      && !_taxResponse.farePath()->ignoreReissueCharges();
}

} // end of tse namespace
