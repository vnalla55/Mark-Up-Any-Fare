//----------------------------------------------------------------------------
//
//  Copyright (c) Sabre 2013
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class GenSalesAgentInfo
{
public:
  GenSalesAgentInfo() : _nation(""), _gdsCode(""), _settlementPlan(""),
                       _validatingCxr(""), _nonParticipatingCxr("") {}

  virtual ~GenSalesAgentInfo() {}

  const NationCode& getCountryCode() const { return _nation; }
  void setCountryCode(const NationCode& c) { _nation = c; }

  const CrsCode& getGDSCode() const { return _gdsCode; }
  void setGDSCode(const CrsCode& gds) { _gdsCode = gds; }

  const SettlementPlanType& getSettlementPlanCode() const { return _settlementPlan; }
  void setSettlementPlanCode(const SettlementPlanType& sc) { _settlementPlan = sc; }

  const DateTime& effDate() const { return _effDate; }
  void setEffDate(const DateTime& dt) { _effDate = dt; }

  const DateTime& discDate() const { return _discDate; }
  void setDiscDate(const DateTime& dt) { _discDate = dt; }

  const DateTime& createDate() const { return _createDate; }
  void setCreateDate(const DateTime& dt) { _createDate = dt; }

  const DateTime& expireDate() const { return _expireDate; }
  void setExpireDate(const DateTime& dt) { _expireDate = dt; }

  const CarrierCode& getCxrCode() const { return _validatingCxr; }
  void setCxrCode(const CarrierCode& c) { _validatingCxr = c; }

  const CarrierCode& getNonParticipatingCxr() const { return _nonParticipatingCxr; }
  void setNonParticipatingCxr(const CarrierCode& c) { _nonParticipatingCxr = c; }

  static void dummyData(GenSalesAgentInfo& obj)
  {
    obj._nation = "US";
    obj._gdsCode = "1S";
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._settlementPlan = "STU";
    obj._validatingCxr = "V1";
    obj._nonParticipatingCxr = "M1";
  }

  bool operator==(const GenSalesAgentInfo& rhs) const
  {
    return (_nation == rhs._nation &&
            _gdsCode == rhs._gdsCode &&
            _effDate == rhs._effDate &&
            _discDate == rhs._discDate &&
            _createDate == rhs._createDate &&
            _expireDate == rhs._expireDate &&
            _settlementPlan == rhs._settlementPlan &&
            _validatingCxr == rhs._validatingCxr &&
            _nonParticipatingCxr == rhs._nonParticipatingCxr);
  }

  virtual void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _gdsCode);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _settlementPlan);
    FLATTENIZE(archive, _validatingCxr);
    FLATTENIZE(archive, _nonParticipatingCxr);
  }

private:
  NationCode _nation;
  CrsCode _gdsCode;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _createDate;
  DateTime _expireDate;
  SettlementPlanType _settlementPlan;
  CarrierCode _validatingCxr;
  CarrierCode _nonParticipatingCxr;

};

} // namespace tse

