//-----------------------------------------------------------------------------------
//	   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//-----------------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class FareByRuleApp
{
public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  PaxTypeCode& primePaxType() { return _primePaxType; }
  const PaxTypeCode& primePaxType() const { return _primePaxType; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& recId() { return _recId; }
  const int& recId() const { return _recId; }

  RuleNumber& ruleNo() { return _ruleNo; }
  const RuleNumber& ruleNo() const { return _ruleNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  int& paxMinAge() { return _paxMinAge; }
  const int& paxMinAge() const { return _paxMinAge; }

  int& paxMaxAge() { return _paxMaxAge; }
  const int& paxMaxAge() const { return _paxMaxAge; }

  int& paxOccFirst() { return _paxOccFirst; }
  const int& paxOccFirst() const { return _paxOccFirst; }

  int& paxOccLast() { return _paxOccLast; }
  const int& paxOccLast() const { return _paxOccLast; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  int& jointCarrierItemNo() { return _jointCarrierItemNo; }
  const int& jointCarrierItemNo() const { return _jointCarrierItemNo; }

  Indicator& negPaxStatusInd() { return _negPaxStatusInd; }
  const Indicator& negPaxStatusInd() const { return _negPaxStatusInd; }

  Indicator& paxInd() { return _paxInd; }
  const Indicator& paxInd() const { return _paxInd; }

  Zone& loc1zoneItemNo() { return _loc1zoneItemNo; }
  const Zone& loc1zoneItemNo() const { return _loc1zoneItemNo; }

  Zone& loc2zoneItemNo() { return _loc2zoneItemNo; }
  const Zone& loc2zoneItemNo() const { return _loc2zoneItemNo; }

  Indicator& directionality() { return _directionality; }
  const Indicator& directionality() const { return _directionality; }

  Indicator& paxId() { return _paxId; }
  const Indicator& paxId() const { return _paxId; }

  LocKey& paxLoc() { return _paxLoc; }
  const LocKey& paxLoc() const { return _paxLoc; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  LocKey& fareLoc1() { return _fareLoc1; }
  const LocKey& fareLoc1() const { return _fareLoc1; }

  LocKey& fareLoc2() { return _fareLoc2; }
  const LocKey& fareLoc2() const { return _fareLoc2; }

  LocKey& whollyWithinLoc() { return _whollyWithinLoc; }
  const LocKey& whollyWithinLoc() const { return _whollyWithinLoc; }

  TSICode& tsi() { return _tsi; }
  const TSICode& tsi() const { return _tsi; }

  LocKey& tvlLoc1() { return _tvlLoc1; }
  const LocKey& tvlLoc1() const { return _tvlLoc1; }

  AccountCode& accountCode() { return _accountCode; }
  const AccountCode& accountCode() const { return _accountCode; }

  TktDesignator& tktDesignator() { return _tktDesignator; }
  const TktDesignator& tktDesignator() const { return _tktDesignator; }

  const Indicator inhibit() const
  {
    return _inhibit;
  };
  Indicator& inhibit()
  {
    return _inhibit;
  };

  Indicator& sameCarrier() { return _sameCarrier; }
  const Indicator& sameCarrier() const { return _sameCarrier; }

  int& carrierFltTblItemNo() { return _carrierFltTblItemNo; }
  const int& carrierFltTblItemNo() const { return _carrierFltTblItemNo; }

  std::vector<PaxTypeCode>& secondaryPaxTypes() { return _secondaryPaxTypes; }
  const std::vector<PaxTypeCode>& secondaryPaxTypes() const { return _secondaryPaxTypes; }

  bool& vendorFWS() { return _vendorFWS; }
  const bool vendorFWS() const { return _vendorFWS; }

  bool operator==(const FareByRuleApp& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_primePaxType == rhs._primePaxType) &&
            (_vendor == rhs._vendor) && (_recId == rhs._recId) && (_ruleNo == rhs._ruleNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_ruleTariff == rhs._ruleTariff) && (_paxMinAge == rhs._paxMinAge) &&
            (_paxMaxAge == rhs._paxMaxAge) && (_paxOccFirst == rhs._paxOccFirst) &&
            (_paxOccLast == rhs._paxOccLast) && (_segCnt == rhs._segCnt) &&
            (_jointCarrierItemNo == rhs._jointCarrierItemNo) &&
            (_negPaxStatusInd == rhs._negPaxStatusInd) && (_paxInd == rhs._paxInd) &&
            (_loc1zoneItemNo == rhs._loc1zoneItemNo) && (_loc2zoneItemNo == rhs._loc2zoneItemNo) &&
            (_directionality == rhs._directionality) && (_paxId == rhs._paxId) &&
            (_paxLoc == rhs._paxLoc) && (_globalDir == rhs._globalDir) &&
            (_fareLoc1 == rhs._fareLoc1) && (_fareLoc2 == rhs._fareLoc2) &&
            (_whollyWithinLoc == rhs._whollyWithinLoc) && (_tsi == rhs._tsi) &&
            (_tvlLoc1 == rhs._tvlLoc1) && (_accountCode == rhs._accountCode) &&
            (_tktDesignator == rhs._tktDesignator) && (_inhibit == rhs._inhibit) &&
            (_sameCarrier == rhs._sameCarrier) &&
            (_carrierFltTblItemNo == rhs._carrierFltTblItemNo) &&
            (_secondaryPaxTypes == rhs._secondaryPaxTypes) && (_vendorFWS == rhs._vendorFWS));
  }

  static void dummyData(FareByRuleApp& obj)
  {
    obj._carrier = "ABC";
    obj._primePaxType = "DEF";
    obj._vendor = "GHIJ";
    obj._recId = 1;
    obj._ruleNo = "KLMN";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._ruleTariff = 2;
    obj._paxMinAge = 3;
    obj._paxMaxAge = 4;
    obj._paxOccFirst = 5;
    obj._paxOccLast = 6;
    obj._segCnt = 7;
    obj._jointCarrierItemNo = 8;
    obj._negPaxStatusInd = 'O';
    obj._paxInd = 'P';
    obj._loc1zoneItemNo = "QRSTUVW";
    obj._loc2zoneItemNo = "XYZabcd";
    obj._directionality = 'e';
    obj._paxId = 'f';

    LocKey::dummyData(obj._paxLoc);

    obj._globalDir = GlobalDirection::US;

    LocKey::dummyData(obj._fareLoc1);
    LocKey::dummyData(obj._fareLoc2);
    LocKey::dummyData(obj._whollyWithinLoc);

    obj._tsi = 9;

    LocKey::dummyData(obj._tvlLoc1);

    obj._accountCode = "aaaaaaaa";
    obj._tktDesignator = "bbbbbbbb";
    obj._inhibit = 'g';
    obj._sameCarrier = 'h';
    obj._carrierFltTblItemNo = 10;

    obj._secondaryPaxTypes.push_back("ijk");
    obj._secondaryPaxTypes.push_back("lmn");
    obj._vendorFWS = true;
  }
  WBuffer& write(WBuffer& os) const { return convert(os, this); }
  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
  CarrierCode _carrier;
  PaxTypeCode _primePaxType;
  VendorCode _vendor;
  int _recId = 0;
  RuleNumber _ruleNo;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  TariffNumber _ruleTariff = 0;
  int _paxMinAge = 0;
  int _paxMaxAge = 0;
  int _paxOccFirst = 0;
  int _paxOccLast = 0;
  int _segCnt = 0;
  int _jointCarrierItemNo = 0;
  Indicator _negPaxStatusInd = ' ';
  Indicator _paxInd = ' ';
  Zone _loc1zoneItemNo;
  Zone _loc2zoneItemNo;
  Indicator _directionality = ' ';
  Indicator _paxId = ' ';
  LocKey _paxLoc;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  LocKey _fareLoc1;
  LocKey _fareLoc2;
  LocKey _whollyWithinLoc;
  TSICode _tsi = 0;
  LocKey _tvlLoc1;
  AccountCode _accountCode;
  TktDesignator _tktDesignator;
  Indicator _inhibit = ' '; // Inhibit now checked at App Level
  Indicator _sameCarrier = ' ';
  int _carrierFltTblItemNo = 0;
  std::vector<PaxTypeCode> _secondaryPaxTypes;
  bool _vendorFWS = false;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _primePaxType);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _recId);
    FLATTENIZE(archive, _ruleNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _paxMinAge);
    FLATTENIZE(archive, _paxMaxAge);
    FLATTENIZE(archive, _paxOccFirst);
    FLATTENIZE(archive, _paxOccLast);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _jointCarrierItemNo);
    FLATTENIZE(archive, _negPaxStatusInd);
    FLATTENIZE(archive, _paxInd);
    FLATTENIZE(archive, _loc1zoneItemNo);
    FLATTENIZE(archive, _loc2zoneItemNo);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _paxId);
    FLATTENIZE(archive, _paxLoc);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _fareLoc1);
    FLATTENIZE(archive, _fareLoc2);
    FLATTENIZE(archive, _whollyWithinLoc);
    FLATTENIZE(archive, _tsi);
    FLATTENIZE(archive, _tvlLoc1);
    FLATTENIZE(archive, _accountCode);
    FLATTENIZE(archive, _tktDesignator);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _sameCarrier);
    FLATTENIZE(archive, _carrierFltTblItemNo);
    FLATTENIZE(archive, _secondaryPaxTypes);
    FLATTENIZE(archive, _vendorFWS);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_carrier & ptr->_primePaxType & ptr->_vendor & ptr->_recId & ptr->_ruleNo &
           ptr->_expireDate & ptr->_createDate & ptr->_effDate & ptr->_discDate & ptr->_ruleTariff &
           ptr->_paxMinAge & ptr->_paxMaxAge & ptr->_paxOccFirst & ptr->_paxOccLast & ptr->_segCnt &
           ptr->_jointCarrierItemNo & ptr->_negPaxStatusInd & ptr->_paxInd & ptr->_loc1zoneItemNo &
           ptr->_loc2zoneItemNo & ptr->_directionality & ptr->_paxId & ptr->_paxLoc &
           ptr->_globalDir & ptr->_fareLoc1 & ptr->_fareLoc2 & ptr->_whollyWithinLoc & ptr->_tsi &
           ptr->_tvlLoc1 & ptr->_accountCode & ptr->_tktDesignator & ptr->_inhibit &
           ptr->_sameCarrier & ptr->_carrierFltTblItemNo & ptr->_secondaryPaxTypes &
           ptr->_vendorFWS;
  }
};
}
