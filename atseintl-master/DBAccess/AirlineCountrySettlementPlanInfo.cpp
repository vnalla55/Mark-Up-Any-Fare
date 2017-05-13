//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/AirlineCountrySettlementPlanInfo.h"

#include "Common/ValidatingCxrConst.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

AirlineCountrySettlementPlanInfo::AirlineCountrySettlementPlanInfo()
  : _nation(""),
    _gds(""),
    _airline(""),
    _settlementPlan(""),
    _preferredTicketingMethod(vcx::TM_EMPTY),
    _requiredTicketingMethod(vcx::TM_EMPTY)
{
}

void
AirlineCountrySettlementPlanInfo::dummyData(AirlineCountrySettlementPlanInfo& acsp)
{
  acsp.setCountryCode("US");
  acsp.setGds("1S");
  acsp.setAirline("AB");
  acsp.setSettlementPlanType("STU");
  acsp.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  acsp.setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
  const DateTime dt(time(nullptr));
  acsp.setEffDate(dt);
  acsp.setDiscDate(dt);
  acsp.setCreateDate(dt);
  acsp.setExpireDate(dt);
}

void
AirlineCountrySettlementPlanInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _nation);
  FLATTENIZE(archive, _gds);
  FLATTENIZE(archive, _airline);
  FLATTENIZE(archive, _settlementPlan);
  FLATTENIZE(archive, _preferredTicketingMethod);
  FLATTENIZE(archive, _requiredTicketingMethod);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _discDate);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _expireDate);
}

bool
AirlineCountrySettlementPlanInfo::
operator==(const AirlineCountrySettlementPlanInfo& rhs) const
{
  return ((getCountryCode() == rhs.getCountryCode()) && (getGds() == rhs.getGds()) &&
          (getAirline() == rhs.getAirline()) &&
          (getSettlementPlanType() == rhs.getSettlementPlanType()) &&
          (getPreferredTicketingMethod() == rhs.getPreferredTicketingMethod()) &&
          (getRequiredTicketingMethod() == rhs.getRequiredTicketingMethod()) &&
          (effDate() == rhs.effDate()) &&
          (discDate() == rhs.discDate()) &&
          (createDate() == rhs.createDate()) &&
          (expireDate() == rhs.expireDate()));
}
}
