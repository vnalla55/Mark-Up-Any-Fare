//-------------------------------------------------------------------
//  Copyright Sabre 2012
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

#include "FreeBagService/BaggageAncillarySecurityValidator.h"

#include "DataModel/Agent.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Rules/RuleConst.h"

namespace tse
{
BaggageAncillarySecurityValidator::BaggageAncillarySecurityValidator(
    AncillaryPricingTrx& trx,
    const std::vector<TravelSeg*>::const_iterator segI,
    const std::vector<TravelSeg*>::const_iterator segIE,
    bool isCollectingT183Info,
    bool validateCharges)
  : BaggageSecurityValidator(trx, segI, segIE, isCollectingT183Info),
    _validateCharges(validateCharges)
{
}

BaggageAncillarySecurityValidator::~BaggageAncillarySecurityValidator() {}

bool
BaggageAncillarySecurityValidator::checkGds(const SvcFeesSecurityInfo* secInfo) const
{
  if (secInfo->carrierGdsCode().empty())
    return true;

  std::string requestGdsCxrCode;

  const bool validateGds = secInfo->carrierGdsCode()[0] == '1';
  const AncRequest* request = static_cast<const AncRequest*>(_trx.getRequest());

  if (_validateCharges ||
      (request->isWPBGRequest() && !request->wpbgPostTicket()))
  {
    if (validateGds)
      requestGdsCxrCode = _trx.getRequest()->ticketingAgent()->cxrCode();
    else
      requestGdsCxrCode = _trx.billing()->partitionID();
  }
  else
  {
    if (validateGds)
      requestGdsCxrCode = _trx.getRequest()->ticketingAgent()->vendorCrsCode();
    else
      requestGdsCxrCode = _trx.getRequest()->ticketingAgent()->cxrCode();
  }

  if ((requestGdsCxrCode == RuleConst::SABRE1B || requestGdsCxrCode == RuleConst::SABRE1J ||
       requestGdsCxrCode == RuleConst::SABRE1F) &&
      secInfo->carrierGdsCode() == RuleConst::SABRE1S)
    return true;

  return requestGdsCxrCode == secInfo->carrierGdsCode();
}

} // tse
