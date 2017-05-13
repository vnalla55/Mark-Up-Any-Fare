//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/NeutralValidatingAirlineInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{

NeutralValidatingAirlineInfo::NeutralValidatingAirlineInfo()
  : _countryCode(""), _gds(""), _airline(""), _settlementPlanType("")
{
}

void
NeutralValidatingAirlineInfo::dummyData(NeutralValidatingAirlineInfo& nva)
{
  nva.setCountryCode("US");
  nva.setGds("1S");
  nva.setAirline("AB");
  nva.setSettlementPlanType("STU");
  const DateTime dt(time(nullptr));
  nva.setEffDate(dt);
  nva.setDiscDate(dt);
  nva.setCreateDate(dt);
  nva.setExpireDate(dt);
}

void
NeutralValidatingAirlineInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _countryCode);
  FLATTENIZE(archive, _gds);
  FLATTENIZE(archive, _airline);
  FLATTENIZE(archive, _settlementPlanType);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _discDate);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _expireDate);
}

bool
NeutralValidatingAirlineInfo::
operator==(const NeutralValidatingAirlineInfo& rhs) const
{
  return ((getCountryCode() == rhs.getCountryCode()) &&
          (getGds() == rhs.getGds()) &&
          (getAirline() == rhs.getAirline()) &&
          (getSettlementPlanType() == rhs.getSettlementPlanType()) &&
          (effDate() == rhs.effDate()) &&
          (discDate() == rhs.discDate()) &&
          (createDate() == rhs.createDate()) &&
          (expireDate() == rhs.expireDate())
         );
}
}
