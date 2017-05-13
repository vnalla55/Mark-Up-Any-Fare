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

#include "FreeBagService/BaggageAncillarySecurityValidatorAB240.h"

#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Rules/RuleConst.h"

#include <stdexcept>

namespace tse
{

BaggageAncillarySecurityValidatorAB240::BaggageAncillarySecurityValidatorAB240(
    AncillaryPricingTrx& trx,
    const std::vector<TravelSeg*>::const_iterator segI,
    const std::vector<TravelSeg*>::const_iterator segIE,
    bool isCollectingT183Info,
    const Indicator typeOfServiceToValidate,
    const Itin *itin)
  : BaggageSecurityValidator(trx, segI, segIE, isCollectingT183Info),
    _typeOfServiceToValidate(typeOfServiceToValidate),
    _itin(itin)
{
  if ((_typeOfServiceToValidate == BAGGAGE_ALLOWANCE ||
       _typeOfServiceToValidate == CARRY_ON_ALLOWANCE) &&
       _itin == nullptr)
    throw std::invalid_argument("Itinerary pointer must not be null when type of service to "
                                "validate is either baggage allowance or carry on allowance.");

  const AncRequest* request = static_cast<const AncRequest*>(_trx.getRequest());
  if (request->isWPBGRequest() && !request->wpbgPostTicket())
    _typeOfServiceToValidate = BAGGAGE_CHARGE;
}

bool
BaggageAncillarySecurityValidatorAB240::checkGds(const SvcFeesSecurityInfo* secInfo) const
{
  if (secInfo->carrierGdsCode().empty())
    return true;

  std::string gdsCodeFromRequest = getGdsCodeFromRequest(secInfo);

  if ((gdsCodeFromRequest == RuleConst::SABRE1B || gdsCodeFromRequest == RuleConst::SABRE1J ||
      gdsCodeFromRequest == RuleConst::SABRE1F) &&
      secInfo->carrierGdsCode() == RuleConst::SABRE1S)
    return true;

  return gdsCodeFromRequest == secInfo->carrierGdsCode();
}

std::string
BaggageAncillarySecurityValidatorAB240::getGdsCodeFromRequest(const SvcFeesSecurityInfo* secInfo) const
{
  switch (_typeOfServiceToValidate)
  {
  case BAGGAGE_ALLOWANCE:
  case CARRY_ON_ALLOWANCE:
    return getGdsCodeFromRequestForAllowanceTypes();
  case BAGGAGE_EMBARGO:
    return getGdsCodeFromRequestForEmbargoes();
  case BAGGAGE_CHARGE:
    return getGdsCodeFromRequestForCharges(secInfo);
  default:
    return getDefaultGdsCodeFromRequest(secInfo);
  }
}

std::string
BaggageAncillarySecurityValidatorAB240::getGdsCodeFromRequestForAllowanceTypes() const
{
  std::string gdsCodeFromRequest;
  AncRequest *ancRequest = static_cast<AncRequest*>(_trx.getRequest());
  bool hasTicketingAgent = _itin->ticketNumber() > 0;

  if (TrxUtil::isRequestFromAS(_trx))
    if (hasTicketingAgent)
      gdsCodeFromRequest = ancRequest->getDefaultTicketingCarrierFromTicketingAgent(_itin->ticketNumber()); // TAG/@AE3
    else
      gdsCodeFromRequest = ancRequest->ticketingAgent()->vendorCrsCode(); // AGI/@AE0
  else
    if (hasTicketingAgent)
      gdsCodeFromRequest = ancRequest->getAirlineCarrierCodeFromTicketingAgent(_itin->ticketNumber()); // TAG/@B00
    else
      gdsCodeFromRequest = ancRequest->ticketingAgent()->cxrCode(); // AGI/@B00

  return gdsCodeFromRequest;
}

std::string
BaggageAncillarySecurityValidatorAB240::getGdsCodeFromRequestForEmbargoes() const
{
  std::string gdsCodeFromRequest;

  if (TrxUtil::isRequestFromAS(_trx))
    gdsCodeFromRequest = _trx.getRequest()->ticketingAgent()->vendorCrsCode(); // AGI/@AE0
  else
    gdsCodeFromRequest = _trx.getRequest()->ticketingAgent()->cxrCode(); // AGI/@B00

  return gdsCodeFromRequest;
}

std::string
BaggageAncillarySecurityValidatorAB240::getGdsCodeFromRequestForCharges(const SvcFeesSecurityInfo* secInfo) const
{
  std::string gdsCodeFromRequest;
  const bool validateGds = secInfo->carrierGdsCode()[0] == '1';

  if (validateGds)
    gdsCodeFromRequest = _trx.getRequest()->ticketingAgent()->cxrCode(); // AGI/@B00
  else
    gdsCodeFromRequest = _trx.billing()->partitionID(); // BIL/@AE0

  return gdsCodeFromRequest;
}

std::string
BaggageAncillarySecurityValidatorAB240::getDefaultGdsCodeFromRequest(const SvcFeesSecurityInfo* secInfo) const
{
  std::string gdsCodeFromRequest;
  const bool validateGds = secInfo->carrierGdsCode()[0] == '1';

  if (validateGds)
    gdsCodeFromRequest = _trx.getRequest()->ticketingAgent()->vendorCrsCode(); // AGI/@AE0
  else
    gdsCodeFromRequest = _trx.getRequest()->ticketingAgent()->cxrCode(); // AGI/@B00

  return gdsCodeFromRequest;
}

} // tse
