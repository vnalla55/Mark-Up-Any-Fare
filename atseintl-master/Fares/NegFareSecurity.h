//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------
//
// subclass to add simple smarts to DB object
//
#pragma once
#include "DBAccess/NegFareSecurityInfo.h"

#include <string>

namespace tse
{
class PricingTrx;
class Agent;

class NegFareSecurity
{
public:
  bool isTvlAgent(const Agent& agent) const;
  bool isPos() const;
  bool isMatchGeo(const Agent& agent, const DateTime& ticketingDate) const;
  bool isMatchWho(const PricingTrx& trx, const Agent* agent) const;
  bool isMatchWhat(bool forTkt) const;
  bool isMatch(PricingTrx& trx, const Agent* agent) const;
  Indicator ticketInd() const { return _x->ticketInd(); }
  Indicator& ticketInd() { return _x->ticketInd(); }
  Indicator localeType() const { return _x->localeType(); }
  Indicator& localeType() { return _x->localeType(); }
  const std::string& crsCarrierDepartment() const { return _x->crsCarrierDepartment(); }
  std::string& crsCarrierDepartment() { return _x->crsCarrierDepartment(); }
  const AgencyPCC& agencyPCC() const { return _x->agencyPCC(); }
  AgencyPCC& agencyPCC() { return _x->agencyPCC(); }
  Indicator applInd() const { return _x->applInd(); }
  Indicator& applInd() { return _x->applInd(); }
  explicit NegFareSecurity(NegFareSecurityInfo* x) : _x(x) {}

private:
  NegFareSecurityInfo* _x;
};
}
