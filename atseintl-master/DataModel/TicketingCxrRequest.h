//-------------------------------------------------------------------
//  Copyright Sabre 2014
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"
#include "DataModel/PricingRequest.h"

namespace tse
{
class TicketingCxrRequest : public PricingRequest
{
public:
  TicketingCxrRequest();

  const bool& isTicketDateOverride() const { return _isTicketDateOverride; }
  bool& isTicketDateOverride() { return _isTicketDateOverride; }

  const SettlementPlanType& getSettlementPlan() const { return _settlementPlan; }
  void setSettlementPlan(const SettlementPlanType& sp) { _settlementPlan = sp; }

  const NationCode& getPosCountry() const { return _posCountry; }
  void setPosCountry(const NationCode& c) { _posCountry = c; }

  const NationCode& getSpecifiedCountry() const { return _specifiedCountry; }
  void setSpecifiedCountry(const NationCode& c) { _specifiedCountry = c; }

  const PseudoCityCode& getPcc() const { return _pcc; }
  void setPcc(const PseudoCityCode& p) { _pcc = p; }

  const CarrierCode& getValidatingCxr() const { return _validatingCxr; }
  void setValidatingCxr(const CarrierCode& vc) { _validatingCxr = vc; }

  const std::vector<vcx::ParticipatingCxr>& participatingCxrs() const { return _participatingCxrs; }
  std::vector<vcx::ParticipatingCxr>& participatingCxrs() { return _participatingCxrs; }

  vcx::TicketType getTicketType() const { return _tktType; }
  void setTicketType(vcx::TicketType tt) { _tktType = tt; }

  const CrsCode& getMultiHost() const { return _multiHost; }
  void setMultiHost(const CrsCode& mh) { _multiHost = mh; }

  void setTicketDate(const DateTime& dt) { _ticketDate = dt; }
  const DateTime& getTicketDate() const { return _ticketDate; }

  vcx::TCSRequestType getRequestType()const { return _reqType; }
  void setRequestType(vcx::TCSRequestType reqType) { _reqType = reqType; }

  bool isArcUser() const { return _isArcUser; }
  void setArcUser(bool isArcUser) { _isArcUser = isArcUser; }

protected:
  TicketingCxrRequest(const TicketingCxrRequest&);
  TicketingCxrRequest& operator=(const TicketingCxrRequest&);

private:
  uint16_t _majorSchemaVersion;
  bool _isTicketDateOverride;
  SettlementPlanType _settlementPlan;
  NationCode _posCountry;
  NationCode _specifiedCountry;
  PseudoCityCode _pcc;
  CarrierCode _validatingCxr;
  std::vector<vcx::ParticipatingCxr> _participatingCxrs;
  vcx::TicketType _tktType;
  CrsCode _multiHost;
  DateTime _ticketDate;
  vcx::TCSRequestType _reqType;
  bool _isArcUser;
};

inline TicketingCxrRequest::TicketingCxrRequest()
  : _majorSchemaVersion(0),
    _isTicketDateOverride(false),
    _tktType(vcx::ETKT_PREF),
    _reqType(vcx::NO_REQ),
    _isArcUser(false)
{
}
} // tse namespace

