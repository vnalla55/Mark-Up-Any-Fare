//----------------------------------------------------------------------------
//
//      File:           CurrencySelection.h
//      Description:
//      Created:        2/4/2004
//      Authors:        Roger Kelly
//
//      Updates:
//
//      2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

#include <string>
#include <vector>

namespace tse
{

class CurrencySelection
{
public:
  CurrencySelection()
    : _seqNo(0),
      _journeyRestr(' '),
      _farecomponentRestr(' '),
      _posexcept(' '),
      _poiExcept(' '),
      _govCarrierExcept(' '),
      _restrCurExcept(' '),
      _psgTypeExcept(' '),
      _versioninheritedInd(' '),
      _versionDisplayInd(' ')
  {
  }
  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& journeyRestr() { return _journeyRestr; }
  const Indicator& journeyRestr() const { return _journeyRestr; }

  Indicator& farecomponentRestr() { return _farecomponentRestr; }
  const Indicator& farecomponentRestr() const { return _farecomponentRestr; }

  LocKey& posLoc() { return _posLoc; }
  const LocKey& posLoc() const { return _posLoc; }

  Indicator& posexcept() { return _posexcept; }
  const Indicator& posexcept() const { return _posexcept; }

  LocKey& poiLoc() { return _poiLoc; }
  const LocKey& poiLoc() const { return _poiLoc; }

  Indicator& poiExcept() { return _poiExcept; }
  const Indicator& poiExcept() const { return _poiExcept; }

  CurrencyCode& fareCompPrimeCur() { return _fareCompPrimeCur; }
  const CurrencyCode& fareCompPrimeCur() const { return _fareCompPrimeCur; }

  CurrencyCode& equivOverrideCur() { return _equivOverrideCur; }
  const CurrencyCode& equivOverrideCur() const { return _equivOverrideCur; }

  CurrencyCode& fareQuoteOverrideCur() { return _fareQuoteOverrideCur; }
  const CurrencyCode& fareQuoteOverrideCur() const { return _fareQuoteOverrideCur; }

  Indicator& govCarrierExcept() { return _govCarrierExcept; }
  const Indicator& govCarrierExcept() const { return _govCarrierExcept; }

  Indicator& restrCurExcept() { return _restrCurExcept; }
  const Indicator& restrCurExcept() const { return _restrCurExcept; }

  Indicator& psgTypeExcept() { return _psgTypeExcept; }
  const Indicator& psgTypeExcept() const { return _psgTypeExcept; }

  Indicator& versioninheritedInd() { return _versioninheritedInd; }
  const Indicator& versioninheritedInd() const { return _versioninheritedInd; }

  Indicator& versionDisplayInd() { return _versionDisplayInd; }
  const Indicator& versionDisplayInd() const { return _versionDisplayInd; }

  std::vector<CarrierCode>& govCarriers() { return _govCarriers; }
  const std::vector<CarrierCode>& govCarriers() const { return _govCarriers; }

  std::vector<CurrencyCode>& restrictedCurs() { return _restrictedCurs; }
  const std::vector<CurrencyCode>& restrictedCurs() const { return _restrictedCurs; }

  std::vector<PaxTypeCode>& passengerTypes() { return _passengerTypes; }
  const std::vector<PaxTypeCode>& passengerTypes() const { return _passengerTypes; }

  std::vector<CurrencyCode>& aseanCurs() { return _aseanCurs; }
  const std::vector<CurrencyCode>& aseanCurs() const { return _aseanCurs; }

  std::vector<std::string>& txtSegs() { return _txtSegs; }
  const std::vector<std::string>& txtSegs() const { return _txtSegs; }

  bool operator==(const CurrencySelection& rhs) const
  {
    return ((_versionDate == rhs._versionDate) && (_nation == rhs._nation) &&
            (_seqNo == rhs._seqNo) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_journeyRestr == rhs._journeyRestr) &&
            (_farecomponentRestr == rhs._farecomponentRestr) && (_posLoc == rhs._posLoc) &&
            (_posexcept == rhs._posexcept) && (_poiLoc == rhs._poiLoc) &&
            (_poiExcept == rhs._poiExcept) && (_fareCompPrimeCur == rhs._fareCompPrimeCur) &&
            (_equivOverrideCur == rhs._equivOverrideCur) &&
            (_fareQuoteOverrideCur == rhs._fareQuoteOverrideCur) &&
            (_govCarrierExcept == rhs._govCarrierExcept) &&
            (_restrCurExcept == rhs._restrCurExcept) && (_psgTypeExcept == rhs._psgTypeExcept) &&
            (_versioninheritedInd == rhs._versioninheritedInd) &&
            (_versionDisplayInd == rhs._versionDisplayInd) && (_govCarriers == rhs._govCarriers) &&
            (_restrictedCurs == rhs._restrictedCurs) && (_passengerTypes == rhs._passengerTypes) &&
            (_aseanCurs == rhs._aseanCurs) && (_txtSegs == rhs._txtSegs));
  }

  static void dummyData(CurrencySelection& obj)
  {
    obj._versionDate = time(nullptr);
    obj._nation = "ABCD";
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._journeyRestr = 'E';
    obj._farecomponentRestr = 'F';
    LocKey::dummyData(obj._posLoc);
    obj._posexcept = 'G';
    LocKey::dummyData(obj._poiLoc);
    obj._poiExcept = 'H';
    obj._fareCompPrimeCur = "IJK";
    obj._equivOverrideCur = "LMN";
    obj._fareQuoteOverrideCur = "OPQ";
    obj._govCarrierExcept = 'R';
    obj._restrCurExcept = 'S';
    obj._psgTypeExcept = 'T';
    obj._versioninheritedInd = 'U';
    obj._versionDisplayInd = 'V';

    obj._govCarriers.push_back("WXY");
    obj._govCarriers.push_back("Zab");

    obj._restrictedCurs.push_back("cde");
    obj._restrictedCurs.push_back("fgh");

    obj._passengerTypes.push_back("ijk");
    obj._passengerTypes.push_back("lmn");

    obj._aseanCurs.push_back("opq");
    obj._aseanCurs.push_back("rst");

    obj._txtSegs.push_back("bbbbbbbb");
    obj._txtSegs.push_back("cccccccc");
  }

private:
  DateTime _versionDate;
  NationCode _nation;
  int _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _journeyRestr;
  Indicator _farecomponentRestr;
  LocKey _posLoc;
  Indicator _posexcept;
  LocKey _poiLoc;
  Indicator _poiExcept;
  CurrencyCode _fareCompPrimeCur;
  CurrencyCode _equivOverrideCur;
  CurrencyCode _fareQuoteOverrideCur;
  Indicator _govCarrierExcept;
  Indicator _restrCurExcept;
  Indicator _psgTypeExcept;
  Indicator _versioninheritedInd;
  Indicator _versionDisplayInd;
  std::vector<CarrierCode> _govCarriers;
  std::vector<CurrencyCode> _restrictedCurs;
  std::vector<PaxTypeCode> _passengerTypes;
  std::vector<CurrencyCode> _aseanCurs;
  std::vector<std::string> _txtSegs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _journeyRestr);
    FLATTENIZE(archive, _farecomponentRestr);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _posexcept);
    FLATTENIZE(archive, _poiLoc);
    FLATTENIZE(archive, _poiExcept);
    FLATTENIZE(archive, _fareCompPrimeCur);
    FLATTENIZE(archive, _equivOverrideCur);
    FLATTENIZE(archive, _fareQuoteOverrideCur);
    FLATTENIZE(archive, _govCarrierExcept);
    FLATTENIZE(archive, _restrCurExcept);
    FLATTENIZE(archive, _psgTypeExcept);
    FLATTENIZE(archive, _versioninheritedInd);
    FLATTENIZE(archive, _versionDisplayInd);
    FLATTENIZE(archive, _govCarriers);
    FLATTENIZE(archive, _restrictedCurs);
    FLATTENIZE(archive, _passengerTypes);
    FLATTENIZE(archive, _aseanCurs);
    FLATTENIZE(archive, _txtSegs);
  }

};
}

