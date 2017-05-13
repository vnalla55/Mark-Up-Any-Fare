//-------------------------------------------------------------------
//
//  File:        RexPricingRequest.cpp
//  Created:     7 May 2009
//  Authors:     Grzegorz Wanke
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

#include "DataModel/RexBaseRequest.h"

#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"

using namespace tse;

Agent*&
RexBaseRequest::ticketingAgent()
{
  return (((_trx != nullptr) && _trx->isAnalyzingExcItin() && (_prevTicketIssueAgent != nullptr))
              ? _prevTicketIssueAgent
              : currentTicketingAgent());
}

const Agent*
RexBaseRequest::ticketingAgent() const
{
  return (((_trx != nullptr) && _trx->isAnalyzingExcItin() && (_prevTicketIssueAgent != nullptr))
              ? _prevTicketIssueAgent
              : currentTicketingAgent());
}

std::string&
RexBaseRequest::corporateID()
{
  if (_trx != nullptr && _trx->isAnalyzingExcItin())
    return _excCorporateID.empty() ? _corporateID : _excCorporateID;

  return _corporateID;
}

const std::string&
RexBaseRequest::corporateID() const
{
  if (_trx != nullptr && _trx->isAnalyzingExcItin())
    return _excCorporateID.empty() ? _corporateID : _excCorporateID;

  return _corporateID;
}

std::string&
RexBaseRequest::accountCode()
{
  if (_trx != nullptr && _trx->isAnalyzingExcItin())
    return _excAccountCode.empty() ? _accountCode : _excAccountCode;

  return _accountCode;
}

const std::string&
RexBaseRequest::accountCode() const
{
  if (_trx != nullptr && _trx->isAnalyzingExcItin())
    return _excAccountCode.empty() ? _accountCode : _excAccountCode;

  return _accountCode;
}

CarrierCode&
RexBaseRequest::validatingCarrier()
{
  return (_trx && _trx->isAnalyzingExcItin() && !_excValidatingCarrier.empty())
             ? _excValidatingCarrier
             : _validatingCarrier;
}

const CarrierCode&
RexBaseRequest::validatingCarrier() const
{
  return (_trx && _trx->isAnalyzingExcItin() && !_excValidatingCarrier.empty())
             ? _excValidatingCarrier
             : _validatingCarrier;
}

DateTime&
RexBaseRequest::ticketingDT()
{
  return ((_trx != nullptr) ? _trx->ticketingDate() : _ticketingDT);
}

const DateTime&
RexBaseRequest::ticketingDT() const
{
  return ((_trx != nullptr) ? _trx->ticketingDate() : _ticketingDT);
}

std::map<int16_t, TktDesignator>&
RexBaseRequest::tktDesignator()
{
  return (_trx && _trx->isAnalyzingExcItin()) ? _excTktDesignator : _tktDesignator;
}

const std::map<int16_t, TktDesignator>&
RexBaseRequest::tktDesignator() const
{
  return (_trx && _trx->isAnalyzingExcItin()) ? _excTktDesignator : _tktDesignator;
}

const TktDesignator
RexBaseRequest::tktDesignator(const int16_t segmentOrder) const
{
  std::map<int16_t, TktDesignator>::const_iterator i = tktDesignator().find(segmentOrder);

  return i != tktDesignator().end() ? i->second : "";
}

bool
RexBaseRequest::isTktDesignatorEntry()
{
  return tktDesignator().size() > 0;
}

LocCode&
RexBaseRequest::ticketPointOverride()
{
  return (((_trx != nullptr) && _trx->isAnalyzingExcItin()) ? _excTicketPointOverride
                                                      : _ticketPointOverride);
}

const LocCode&
RexBaseRequest::ticketPointOverride() const
{
  return (((_trx != nullptr) && _trx->isAnalyzingExcItin()) ? _excTicketPointOverride
                                                      : _ticketPointOverride);
}

LocCode&
RexBaseRequest::salePointOverride()
{
  return (((_trx != nullptr) && _trx->isAnalyzingExcItin()) ? _excSalePointOverride : _salePointOverride);
}

const LocCode&
RexBaseRequest::salePointOverride() const
{
  return (((_trx != nullptr) && _trx->isAnalyzingExcItin()) ? _excSalePointOverride : _salePointOverride);
}

void
RexBaseRequest::setOriginalTicketAgentLocation(const LocCode& loc)
{
  _originalTicketAgentLocation = _trx->dataHandle().getLoc(loc, _trx->originalTktIssueDT());
}

bool
RexBaseRequest::isAirlineRequest() const
{
  return currentTicketingAgent()->tvlAgencyPCC().empty();
}
