//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "RexPricing/NewTicketEqualOrHigherValidator.h"

#include "Common/TrxUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DBAccess/Currency.h"
#include "Diagnostic/Diag689Collector.h"
#include "Pricing/PricingUtil.h"
#include "Rules/NegotiatedFareRuleUtil.h"

namespace tse
{

bool
NewTicketEqualOrHigherValidator::match(const ProcessTagPermutation& perm) const
{
  bool result = matchImpl(perm);
  if (!isDiagnostic())
    return result;

  Money newTotal = convert(_farePath.getTotalNUCAmount(), _farePath.itin()->originationCurrency());
  Money newNonref = convert(_farePath.getNonrefundableAmountInNUC(_trx),
                            _farePath.itin()->originationCurrency());

  _diag->printNewTicketEqualOrHigherValidation(_trx.getTotalBaseFareAmount(),
                                               newTotal,
                                               _newBaseAmount,
                                               _excNonrefAmount,
                                               newNonref,
                                               _newNonrefAmount,
                                               (!_trx.getRexRequest().isAirlineRequest() &&
                                                _trx.getRexOptions().isNetSellingIndicator()),
                                               _isNetFarePath,
                                               perm.checkTable988Byte156(),
                                               result);

  return result;
}

bool
NewTicketEqualOrHigherValidator::matchImpl(const ProcessTagPermutation& perm) const
{
  switch (perm.checkTable988Byte156())
  {
  case ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BLANK:
    return true;

  case ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_B:
    return isNewTicketAmountEqualOrHigher();

  case ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_N:
    return isNewNonrefTicketAmountEqualOrHigher(perm.isNMostRestrictiveResidualPenaltyInd());

  case ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BN:
    return isNewTicketAmountEqualOrHigher() &&
           isNewNonrefTicketAmountEqualOrHigher(perm.isNMostRestrictiveResidualPenaltyInd());
  }

  return true;
}

bool
NewTicketEqualOrHigherValidator::isDiagnostic() const
{
  return _diag && _diag->filterPassed();
}

Money
NewTicketEqualOrHigherValidator::getNewTicketAmount() const
{
  return convert(_farePath.getTotalNUCAmount(), _trx.getTotalBaseFareAmount().code());
}

Money
NewTicketEqualOrHigherValidator::getNewNonrefTicketAmount() const
{
  return convert(_farePath.getNonrefundableAmountInNUC(_trx), _trx.getTotalBaseFareAmount().code());
}

Money
NewTicketEqualOrHigherValidator::getExcNonrefTicketAmount() const
{
  return _trx.exchangeItin().front()->getNonRefAmount();
}

bool
NewTicketEqualOrHigherValidator::isNewTicketAmountEqualOrHigher() const
{
  ValidationStateCache::Status& status = _stateCache->baseFareStatus;
  return status.isDetermined
             ? status.value
             : status.determine(_newBaseAmount.value() - _trx.getTotalBaseFareAmount().value() >
                                -_epsilon);
}

bool
NewTicketEqualOrHigherValidator::isNewNonrefTicketAmountEqualOrHigher(bool isSubtractForNonref)
    const
{
  ValidationStateCache::Status& status = _stateCache->nonrefFareStatus;
  return status.isDetermined
             ? status.value
             : status.determine(isSubtractForNonref ||
                                (_newNonrefAmount.value() - _excNonrefAmount.value() > -_epsilon));
}

const FarePath&
NewTicketEqualOrHigherValidator::getFarePath(FarePath& fp) const
{
  if (!(TrxUtil::isCat35TFSFEnabled(_trx) && _trx.getRexOptions().isNetSellingIndicator()))
    return fp;

  if (_trx.getRequest()->isTicketEntry() &&
      (_trx.getRequest()->isFormOfPaymentCard() || _trx.getOptions()->isCat35Sell()))
    return fp;

  CollectedNegFareData* saveCd = fp.collectedNegFareData();
  NegotiatedFareRuleUtil nfru;
  bool isItBt = nfru.processNegFareITBT(_trx, fp) && nfru.isNetTicketingWithItBtData(_trx, fp);
  fp.collectedNegFareData() = saveCd;
  return isItBt ? *PricingUtil::createNetFarePath(_trx, fp) : fp;
}

Money
NewTicketEqualOrHigherValidator::convert(const Money& source,
                                         const CurrencyCode& targetCurr) const
{
  return _trx.convertCurrency(source, targetCurr,
                              _farePath.itin()->useInternationalRounding());
}

MoneyAmount
NewTicketEqualOrHigherValidator::getEpsilon() const
{
  if (_farePath.itin()->useInternationalRounding())
    return EPSILON;

  const Currency* info = _trx.dataHandle().getCurrency(_trx.getTotalBaseFareAmount().code());
  return info ? std::abs(info->domRoundingFactor()) + EPSILON : EPSILON;
}

} // tse
