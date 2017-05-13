//-------------------------------------------------------------------
//
//  File:        RexPricingRequest.cpp
//  Created:     13 June 2008
//  Authors:     Grzegorz Wanke
//
//  Description: Request for reissue transactions
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "DataModel/RexPricingRequest.h"

#include "DataModel/RexPricingTrx.h"

namespace tse
{
void
RexPricingRequest::setNewAndExcAccCodeCorpId()
{
  if ((_trx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE) &&
      ((static_cast<RexPricingTrx*>(_trx))->needRetrieveKeepFare() ||
       (_trx->getTrxType() == PricingTrx::MIP_TRX && _trx->needRetrieveKeepFareAnyItin())) &&
      ((_excAccountCode != _accountCode) || (_excCorporateID != _corporateID)))
  {
    if (!_excAccountCode.empty() && (_excAccountCode != _accountCode))
      _newAndExcAccCode.push_back(_excAccountCode);
    if (!_accountCode.empty())
      _newAndExcAccCode.push_back(_accountCode);

    if (!_excCorporateID.empty() && (_excCorporateID != _corporateID))
      _newAndExcCorpId.push_back(_excCorporateID);
    if (!_corporateID.empty())
      _newAndExcCorpId.push_back(_corporateID);
  }
}
} // tse namespace
