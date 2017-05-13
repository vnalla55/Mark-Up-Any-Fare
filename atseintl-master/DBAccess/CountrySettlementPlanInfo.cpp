//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/CountrySettlementPlanInfo.h"

#include "Common/TseEnums.h"
#include "Common/ValidatingCxrConst.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

CountrySettlementPlanInfo::CountrySettlementPlanInfo()
  : _nation(""),
    _settlementPlan(""),
    _preferredTicketingMethod(vcx::TM_EMPTY),
    _requiredTicketingMethod(vcx::TM_EMPTY)
{
}

CountrySettlementPlanInfo::CountrySettlementPlanInfo(const NationCode& countryCode, const SettlementPlanType& type)
  : _nation(countryCode),
    _settlementPlan(type),
    _preferredTicketingMethod(vcx::TM_EMPTY),
    _requiredTicketingMethod(vcx::TM_EMPTY)
{
}

void
CountrySettlementPlanInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _nation);
  FLATTENIZE(archive, _settlementPlan);
  FLATTENIZE(archive, _preferredTicketingMethod);
  FLATTENIZE(archive, _requiredTicketingMethod);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _discDate);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _expireDate);
}

void
CountrySettlementPlanInfo::dummyData(CountrySettlementPlanInfo& info)
{
  info.setCountryCode("CC");
  info.setSettlementPlanTypeCode("ABC");
  info.setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
  info.setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
  const DateTime dt(time(nullptr));
  info.setEffDate(dt);
  info.setDiscDate(dt);
  info.setCreateDate(dt);
  info.setExpireDate(dt);

}

bool
CountrySettlementPlanInfo::
operator==(const CountrySettlementPlanInfo& rhs) const
{
  return ((getCountryCode() == rhs.getCountryCode()) &&
          (getSettlementPlanTypeCode() == rhs.getSettlementPlanTypeCode()) &&
          (getPreferredTicketingMethod() == rhs.getPreferredTicketingMethod()) &&
          (getRequiredTicketingMethod() == rhs.getRequiredTicketingMethod()) &&
          (effDate() == rhs.effDate()) &&
          (discDate() == rhs.discDate()) &&
          (createDate() == rhs.createDate()) &&
          (expireDate() == rhs.expireDate()));
}
}
