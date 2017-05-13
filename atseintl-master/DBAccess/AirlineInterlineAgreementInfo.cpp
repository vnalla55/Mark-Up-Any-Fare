//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/AirlineInterlineAgreementInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{
const AirlineInterlineAgreementInfo AirlineInterlineAgreementInfo::_emptyAirlineInterlineAgreement;

AirlineInterlineAgreementInfo::AirlineInterlineAgreementInfo()
  : _nation(""),
    _gds(""),
    _validatingCarrier(""),
    _participatingCarrier(""),
    _agreementTypeCode("")
{
}

void
AirlineInterlineAgreementInfo::dummyData(AirlineInterlineAgreementInfo& info)
{
  info.setCountryCode("US");
  info.setGds("1S");
  info.setValidatingCarrier("AA");
  info.setParticipatingCarrier("BB");
  info.setAgreementTypeCode("STD");
  const DateTime dt(time(nullptr));
  info.setEffDate(dt);
  info.setDiscDate(dt);
  info.setCreateDate(dt);
  info.setExpireDate(dt);
}

void
AirlineInterlineAgreementInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _nation);
  FLATTENIZE(archive, _gds);
  FLATTENIZE(archive, _validatingCarrier);
  FLATTENIZE(archive, _participatingCarrier);
  FLATTENIZE(archive, _agreementTypeCode);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _discDate);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _expireDate);
}

bool
AirlineInterlineAgreementInfo::
operator==(const AirlineInterlineAgreementInfo& rhs) const
{
  return ((getCountryCode() == rhs.getCountryCode()) && (getGds() == rhs.getGds()) &&
          (getValidatingCarrier() == rhs.getValidatingCarrier()) &&
          (getParticipatingCarrier() == rhs.getParticipatingCarrier()) &&
          (getAgreementTypeCode() == rhs.getAgreementTypeCode()) &&
          (effDate() == rhs.effDate()) &&
          (discDate() == rhs.discDate()) &&
          (createDate() == rhs.createDate()) &&
          (expireDate() == rhs.expireDate()));
}
}
