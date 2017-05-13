//-------------------------------------------------------------------
//  Description: Negotiated Fare Security
//      subclass of DB object for Table983 to add simple validation/access
//
//  Copyright Sabre 2004
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

#include "Fares/NegFareSecurity.h"

#include "Common/LocUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

namespace tse
{
using namespace std;

// UTILITY FUNCTIONS
bool
NegFareSecurity::isTvlAgent(const Agent& agent) const
{
  return (!agent.tvlAgencyPCC().empty());
}
bool
NegFareSecurity::isPos() const
{
  return (applInd() != RuleConst::NOT_ALLOWED);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool NegFareSecurity::isMatchGeo
//
// Description:  This function matches Table 983 Geographic Specification 1
//               or Geographic Specification 2 fields.
//               Ignore blanks.  If both blank, match is true.
// </PRE>
// ----------------------------------------------------------------------------
bool
NegFareSecurity::isMatchGeo(const Agent& agent, const DateTime& ticketingDate) const
{
  bool ret = true;
  if (_x->loc1().locType() != LOCTYPE_NONE)
  {
    ret = LocUtil::isInLoc(*agent.agentLocation(),
                           _x->loc1().locType(),
                           _x->loc1().loc(),
                           _x->vendor(),
                           RESERVED,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           ticketingDate);
    if (ret == true)
    {
      return true;
    }
  }
  if (_x->loc2().locType() != LOCTYPE_NONE)
  {
    ret = LocUtil::isInLoc(*agent.agentLocation(),
                           _x->loc2().locType(),
                           _x->loc2().loc(),
                           _x->vendor(),
                           RESERVED,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           ticketingDate);
  }
  return ret;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool NegFareSecurity::isMatchWho
//
// Description:  This function matches Table 983 Who fields
//
// </PRE>
// ----------------------------------------------------------------------------
bool
NegFareSecurity::isMatchWho(const PricingTrx& trx, const Agent* agent) const
{
  if (UNLIKELY(agent == nullptr))
    return false;

  // match crs
  if (_x->carrierCrs()[0] == '1')
  {
    if (agent->abacusUser())
    {
      if (!(_x->carrierCrs() == RuleConst::SABRE1B || _x->carrierCrs() == RuleConst::SABRE1S))
      {
        return false;
      }
    }
    else if (UNLIKELY(agent->axessUser()))
    {
      if (!(_x->carrierCrs() == RuleConst::SABRE1J || _x->carrierCrs() == RuleConst::SABRE1S))
      {
        return false;
      }
    }
    else if (agent->infiniUser())
    {
      if (!(_x->carrierCrs() == RuleConst::SABRE1F || _x->carrierCrs() == RuleConst::SABRE1S))
      {
        return false;
      }
    }

    else if (_x->carrierCrs() != RuleConst::SABRE1S)
    {
      return false;
    }
  }

  // if set, only matches agents (not carrier users)
  if (_x->tvlAgencyInd() != RuleConst::BLANK)
  {
    if (!isTvlAgent(*agent))
    {
      return false;
    }

    if (!_x->carrierCrs().empty() && _x->carrierCrs()[0] != '1' && isTvlAgent(*agent))
      return false;
  }
  // otherwise match carrier (may match TvlAgent w/ CRS access)
  else
  {
    if (!_x->carrierCrs().empty() && _x->carrierCrs()[0] != '1')
    {
      if (isTvlAgent(*agent))
      {
        return false;
      }
      else if (_x->carrierCrs() != trx.billing()->partitionID())
      {
        return false;
      }
    }
  }

  // also must match extra qualifiers (if present)
  if (!_x->dutyFunctionCode().empty())
  {
    if (!RuleUtil::validateDutyFunctionCode(agent, _x->dutyFunctionCode()))
      return false;
  }

  if (localeType() == RuleConst::BLANK)
    return true;
  else if (localeType() == RuleConst::TRAVEL_AGENCY)
    return (agencyPCC() == agent->tvlAgencyPCC());
  else if (localeType() == RuleConst::IATA_TVL_AGENCY_NO)
    return (_x->iataTvlAgencyNo() == agent->tvlAgencyIATA());
  else if (localeType() == RuleConst::HOME_IATA_TVL_AGENCY_NO)
    return (_x->iataTvlAgencyNo() == agent->homeAgencyIATA() ||
            _x->iataTvlAgencyNo() == agent->tvlAgencyIATA());
  else if (localeType() == RuleConst::HOME_TRAVEL_AGENCY)
    return (agencyPCC() == agent->mainTvlAgencyPCC() || agencyPCC() == agent->tvlAgencyPCC());
  else if (localeType() == RuleConst::CRS_CRX_DEPT_CODE)
  {
    // Per Lisa Fishback/CA VanZant	8/8/05
    // Type V should only be validated if the CRS/CXR field is anything other than 1S
    if (_x->carrierCrs() == RuleConst::SABRE1S)
    {
      return false;
    }

    return ((crsCarrierDepartment() == agent->airlineDept()) ||
            (crsCarrierDepartment() == agent->officeDesignator()));
  }
  else
    return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool NegFareSecurity::isMatchWhat
//
// Description:  This function validates the sell and ticket indicators in
//               Table 983 against the pricing or ticketing request.
//               if not for Ticketing, assume for Selling
// </PRE>
// ----------------------------------------------------------------------------
bool
NegFareSecurity::isMatchWhat(bool forTkt) const
{
  if (UNLIKELY(forTkt))
  {
    return (_x->ticketInd() == YES);
  }
  return (_x->sellInd() == YES);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool NegFareSecurity::isMatch
//
// Description:  This function matches Table 983 Who/Where fields
//
// </PRE>
// ----------------------------------------------------------------------------
bool
NegFareSecurity::isMatch(PricingTrx& trx, const Agent* agent) const
{
  if (UNLIKELY(agent == nullptr))
    return false;

  return (isMatchGeo(*agent, trx.getRequest()->ticketingDT()) && isMatchWho(trx, agent));
}
}
