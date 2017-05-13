//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class AirlineInterlineAgreementInfo
{
public:
  AirlineInterlineAgreementInfo();
  virtual ~AirlineInterlineAgreementInfo() {}

  const NationCode& getCountryCode() const { return _nation; }
  void setCountryCode(const NationCode& countryCode) { _nation = countryCode; }

  const CrsCode& getGds() const { return _gds; }
  void setGds(const CrsCode& gds) { _gds = gds; }

  const CarrierCode& getValidatingCarrier() const { return _validatingCarrier; }
  void setValidatingCarrier(const CarrierCode& carrierCode) { _validatingCarrier = carrierCode; }

  const CarrierCode& getParticipatingCarrier() const { return _participatingCarrier; }
  void setParticipatingCarrier(const CarrierCode& carrierCode)
  {
    _participatingCarrier = carrierCode;
  }

  const Alpha3Char& getAgreementTypeCode() const { return _agreementTypeCode; }
  void setAgreementTypeCode(const Alpha3Char& agreementCode) { _agreementTypeCode = agreementCode; }

  bool isThirdPartyAgreement() const { return (vcx::AGR_THIRD_PARTY == getAgreementTypeCode()); }
  bool isPaperOnlyAgreement() const { return (vcx::AGR_PAPER == getAgreementTypeCode()); }
  bool isStandardAgreement() const { return (vcx::AGR_STANDARD == getAgreementTypeCode()); }

  const DateTime& effDate() const { return _effDate; }
  void setEffDate(const DateTime& dt) { _effDate = dt; }

  const DateTime& discDate() const { return _discDate; }
  void setDiscDate(const DateTime& dt) { _discDate = dt; }

  const DateTime& createDate() const { return _createDate; }
  void setCreateDate(const DateTime& dt) { _createDate = dt; }

  const DateTime& expireDate() const { return _expireDate; }
  void setExpireDate(const DateTime& dt) { _expireDate = dt; }

  virtual void flattenize(Flattenizable::Archive&);
  static void dummyData(AirlineInterlineAgreementInfo&);

  virtual bool operator==(const AirlineInterlineAgreementInfo&) const;

protected:
  static const AirlineInterlineAgreementInfo _emptyAirlineInterlineAgreement;

  NationCode _nation;
  CrsCode _gds;
  CarrierCode _validatingCarrier;
  CarrierCode _participatingCarrier;
  Alpha3Char _agreementTypeCode;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _createDate;
  DateTime _expireDate;

private:
  AirlineInterlineAgreementInfo(const AirlineInterlineAgreementInfo&);
  AirlineInterlineAgreementInfo& operator=(const AirlineInterlineAgreementInfo&);
};
}

