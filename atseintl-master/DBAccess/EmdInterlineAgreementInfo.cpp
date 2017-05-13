//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/EmdInterlineAgreementInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{
EmdInterlineAgreementInfo::EmdInterlineAgreementInfo()
  : _nation(""),
    _gds(""),
    _validatingCarrier(""),
    _participatingCarrier("")
{
}

void
EmdInterlineAgreementInfo::dummyData(EmdInterlineAgreementInfo& info)
{
  info.setCountryCode("US");
  info.setValidatingCarrier("AA");
  info.setParticipatingCarrier("BB");
  const DateTime dt(time(nullptr));
  info.setCreateDate(dt);
  info.setEffDate(dt);
  info.setExpireDate(dt);
  info.setDiscDate(dt);
}

void
EmdInterlineAgreementInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _nation);
  FLATTENIZE(archive, _gds);
  FLATTENIZE(archive, _validatingCarrier);
  FLATTENIZE(archive, _participatingCarrier);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _discDate);
}

bool
EmdInterlineAgreementInfo::
operator==(const EmdInterlineAgreementInfo& rhs) const
{
  return ((getCountryCode() == rhs.getCountryCode()) &&
          (getGds() == rhs.getGds()) &&
          (getValidatingCarrier() == rhs.getValidatingCarrier()) &&
          (getParticipatingCarrier() == rhs.getParticipatingCarrier()) &&
          (createDate() == rhs.createDate()) &&
          (effDate() == rhs.effDate()) &&
          (expireDate() == rhs.expireDate()) &&
          (discDate() == rhs.discDate()));
}
} // tse
