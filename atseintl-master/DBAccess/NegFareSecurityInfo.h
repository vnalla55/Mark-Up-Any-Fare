//----------------------------------------------------------------------------
//       (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class NegFareSecurityInfo : public RuleItemInfo
{
public:
  NegFareSecurityInfo()
    : _seqNo(0),
      _applInd(' '),
      _tvlAgencyInd(' '),
      _localeType(' '),
      _departmentId(0),
      _erspno(0),
      _updateInd(' '),
      _redistributeInd(' '),
      _sellInd(' '),
      _ticketInd(' '),
      _changesOnlyInd(' '),
      _secondarySellerId(0)
  {
  }

  virtual ~NegFareSecurityInfo() {}

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& applInd() { return _applInd; }
  const Indicator& applInd() const { return _applInd; }

  Indicator& tvlAgencyInd() { return _tvlAgencyInd; }
  const Indicator& tvlAgencyInd() const { return _tvlAgencyInd; }

  CarrierCode& carrierCrs() { return _carrierCrs; }
  const CarrierCode& carrierCrs() const { return _carrierCrs; }

  AgencyDutyCode& dutyFunctionCode() { return _dutyFunctionCode; }
  const AgencyDutyCode& dutyFunctionCode() const { return _dutyFunctionCode; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  Indicator& localeType() { return _localeType; }
  const Indicator& localeType() const { return _localeType; }

  AgencyPCC& agencyPCC() { return _agencyPCC; }
  const AgencyPCC& agencyPCC() const { return _agencyPCC; }

  AgencyCode& iataTvlAgencyNo() { return _iataTvlAgencyNo; }
  const AgencyCode& iataTvlAgencyNo() const { return _iataTvlAgencyNo; }

  int& departmentId() { return _departmentId; }
  const int& departmentId() const { return _departmentId; }

  std::string& crsCarrierDepartment() { return _crsCarrierDepartment; }
  const std::string& crsCarrierDepartment() const { return _crsCarrierDepartment; }

  AgencyIATA& lineIATANo() { return _lineIATANo; }
  const AgencyIATA& lineIATANo() const { return _lineIATANo; }

  int& erspno() { return _erspno; }
  const int& erspno() const { return _erspno; }

  Indicator& updateInd() { return _updateInd; }
  const Indicator& updateInd() const { return _updateInd; }

  Indicator& redistributeInd() { return _redistributeInd; }
  const Indicator& redistributeInd() const { return _redistributeInd; }

  Indicator& sellInd() { return _sellInd; }
  const Indicator& sellInd() const { return _sellInd; }

  Indicator& ticketInd() { return _ticketInd; }
  const Indicator& ticketInd() const { return _ticketInd; }

  Indicator& changesOnlyInd() { return _changesOnlyInd; }
  const Indicator& changesOnlyInd() const { return _changesOnlyInd; }

  long& secondarySellerId() { return _secondarySellerId; }
  const long& secondarySellerId() const { return _secondarySellerId; }

  virtual bool operator==(const NegFareSecurityInfo& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_seqNo == rhs._seqNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_applInd == rhs._applInd) && (_tvlAgencyInd == rhs._tvlAgencyInd) &&
            (_carrierCrs == rhs._carrierCrs) && (_dutyFunctionCode == rhs._dutyFunctionCode) &&
            (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) && (_localeType == rhs._localeType) &&
            (_agencyPCC == rhs._agencyPCC) && (_iataTvlAgencyNo == rhs._iataTvlAgencyNo) &&
            (_departmentId == rhs._departmentId) &&
            (_crsCarrierDepartment == rhs._crsCarrierDepartment) &&
            (_lineIATANo == rhs._lineIATANo) && (_erspno == rhs._erspno) &&
            (_updateInd == rhs._updateInd) && (_redistributeInd == rhs._redistributeInd) &&
            (_sellInd == rhs._sellInd) && (_ticketInd == rhs._ticketInd) &&
            (_changesOnlyInd == rhs._changesOnlyInd) &&
            (_secondarySellerId == rhs._secondarySellerId));
  }

  virtual WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

  friend inline std::ostream& dumpObject(std::ostream& os, const NegFareSecurityInfo& obj)
  {
    os << "[";
    dumpObject(os, dynamic_cast<const RuleItemInfo&>(obj));
    os << "|" << obj._seqNo << "|" << obj._createDate << "|" << obj._expireDate << "|"
       << obj._applInd << "|" << obj._tvlAgencyInd << "|" << obj._carrierCrs << "|"
       << obj._dutyFunctionCode << "|" << obj._loc1 << "|" << obj._loc2 << "|" << obj._localeType
       << "|" << obj._agencyPCC << "|" << obj._iataTvlAgencyNo << "|" << obj._departmentId << "|"
       << obj._crsCarrierDepartment << "|" << obj._lineIATANo << "|" << obj._erspno << "|"
       << obj._updateInd << "|" << obj._redistributeInd << "|" << obj._sellInd << "|"
       << obj._ticketInd << "|" << obj._changesOnlyInd << "|" << obj._secondarySellerId << "]";
    return os;
  }

  static void dummyData(NegFareSecurityInfo& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._applInd = 'A';
    obj._tvlAgencyInd = 'B';
    obj._carrierCrs = "CDE";
    obj._dutyFunctionCode = "aaaaaaaa";

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._localeType = 'F';
    obj._agencyPCC = "GHIJK01";
    obj._iataTvlAgencyNo = "LMNO";
    obj._departmentId = 2;
    obj._crsCarrierDepartment = "bbbbbbbb";
    obj._lineIATANo = "cccccccc";
    obj._erspno = 3;
    obj._updateInd = 'P';
    obj._redistributeInd = 'Q';
    obj._sellInd = 'R';
    obj._ticketInd = 'S';
    obj._changesOnlyInd = 'T';
    obj._secondarySellerId = 4;
  }

private:
  int _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _applInd;
  Indicator _tvlAgencyInd;
  CarrierCode _carrierCrs;
  AgencyDutyCode _dutyFunctionCode;
  LocKey _loc1;
  LocKey _loc2;
  Indicator _localeType;
  AgencyPCC _agencyPCC;
  AgencyCode _iataTvlAgencyNo;
  int _departmentId;
  std::string _crsCarrierDepartment;
  AgencyIATA _lineIATANo;
  int _erspno;
  Indicator _updateInd;
  Indicator _redistributeInd;
  Indicator _sellInd;
  Indicator _ticketInd;
  Indicator _changesOnlyInd;
  long _secondarySellerId;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer & ptr->_seqNo & ptr->_createDate & ptr->_expireDate & ptr->_applInd &
           ptr->_tvlAgencyInd & ptr->_carrierCrs & ptr->_dutyFunctionCode & ptr->_loc1 &
           ptr->_loc2 & ptr->_localeType & ptr->_agencyPCC & ptr->_iataTvlAgencyNo &
           ptr->_departmentId & ptr->_crsCarrierDepartment & ptr->_lineIATANo & ptr->_erspno &
           ptr->_updateInd & ptr->_redistributeInd & ptr->_sellInd & ptr->_ticketInd &
           ptr->_changesOnlyInd & ptr->_secondarySellerId;
  }

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _applInd);
    FLATTENIZE(archive, _tvlAgencyInd);
    FLATTENIZE(archive, _carrierCrs);
    FLATTENIZE(archive, _dutyFunctionCode);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _localeType);
    FLATTENIZE(archive, _agencyPCC);
    FLATTENIZE(archive, _iataTvlAgencyNo);
    FLATTENIZE(archive, _departmentId);
    FLATTENIZE(archive, _crsCarrierDepartment);
    FLATTENIZE(archive, _lineIATANo);
    FLATTENIZE(archive, _erspno);
    FLATTENIZE(archive, _updateInd);
    FLATTENIZE(archive, _redistributeInd);
    FLATTENIZE(archive, _sellInd);
    FLATTENIZE(archive, _ticketInd);
    FLATTENIZE(archive, _changesOnlyInd);
    FLATTENIZE(archive, _secondarySellerId);
  }

};
}
