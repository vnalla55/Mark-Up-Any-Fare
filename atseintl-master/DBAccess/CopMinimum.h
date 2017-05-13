//----------------------------------------------------------------------------
//     2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CopCarrier.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class CopMinimum
{
public:
  CopMinimum()
    : _seqNo(0),
      _travelAppl(' '),
      _puNormalSpecialType(' '),
      _puTripType(' '),
      _puAppl(' '),
      _fareTypeAppl(' '),
      _userApplType(' ')
  {
  }

  ~CopMinimum()
  {
    std::vector<CopCarrier*>::iterator CxrIt;
    for (CxrIt = _cxrs.begin(); CxrIt != _cxrs.end(); CxrIt++)
    { // Nuke 'em!
      delete *CxrIt;
    }
  }

  NationCode& copNation() { return _copNation; }
  const NationCode& copNation() const { return _copNation; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

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

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Indicator& travelAppl() { return _travelAppl; }
  const Indicator& travelAppl() const { return _travelAppl; }

  Indicator& puNormalSpecialType() { return _puNormalSpecialType; }
  const Indicator& puNormalSpecialType() const { return _puNormalSpecialType; }

  Indicator& puTripType() { return _puTripType; }
  const Indicator& puTripType() const { return _puTripType; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  Indicator& puAppl() { return _puAppl; }
  const Indicator& puAppl() const { return _puAppl; }

  LocKey& puOrigLoc() { return _puOrigLoc; }
  const LocKey& puOrigLoc() const { return _puOrigLoc; }

  LocKey& puWithinLoc() { return _puWithinLoc; }
  const LocKey& puWithinLoc() const { return _puWithinLoc; }

  LocKey& constPointLoc() { return _constPointLoc; }
  const LocKey& constPointLoc() const { return _constPointLoc; }

  Indicator& fareTypeAppl() { return _fareTypeAppl; }
  const Indicator& fareTypeAppl() const { return _fareTypeAppl; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  std::vector<CopCarrier*>& cxrs() { return _cxrs; }
  const std::vector<CopCarrier*>& cxrs() const { return _cxrs; }

  std::vector<CarrierCode>& tktgCxrExcpts() { return _tktgCxrExcpts; }
  const std::vector<CarrierCode>& tktgCxrExcpts() const { return _tktgCxrExcpts; }

  bool operator==(const CopMinimum& rhs) const
  {
    bool eq =
        ((_copNation == rhs._copNation) && (_versionDate == rhs._versionDate) &&
         (_seqNo == rhs._seqNo) && (_createDate == rhs._createDate) &&
         (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
         (_discDate == rhs._discDate) && (_userAppl == rhs._userAppl) &&
         (_travelAppl == rhs._travelAppl) && (_puNormalSpecialType == rhs._puNormalSpecialType) &&
         (_puTripType == rhs._puTripType) && (_fareType == rhs._fareType) &&
         (_puAppl == rhs._puAppl) && (_puOrigLoc == rhs._puOrigLoc) &&
         (_puWithinLoc == rhs._puWithinLoc) && (_constPointLoc == rhs._constPointLoc) &&
         (_fareTypeAppl == rhs._fareTypeAppl) && (_userApplType == rhs._userApplType) &&
         (_cxrs.size() == rhs._cxrs.size()) && (_tktgCxrExcpts == rhs._tktgCxrExcpts));

    for (size_t i = 0; (eq && (i < _cxrs.size())); ++i)
    {
      eq = (*(_cxrs[i]) == *(rhs._cxrs[i]));
    }

    return eq;
  }

  static void dummyData(CopMinimum& obj)
  {
    obj._copNation = "ABCD";
    obj._versionDate = time(nullptr);
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._userAppl = "EFGH";
    obj._travelAppl = 'I';
    obj._puNormalSpecialType = 'J';
    obj._puTripType = 'K';
    obj._fareType = "LMNOPQRS";
    obj._puAppl = 'T';

    LocKey::dummyData(obj._puOrigLoc);
    LocKey::dummyData(obj._puWithinLoc);
    LocKey::dummyData(obj._constPointLoc);

    obj._fareTypeAppl = 'u';
    obj._userApplType = 'v';

    CopCarrier* cc1 = new CopCarrier;
    CopCarrier* cc2 = new CopCarrier;
    CopCarrier::dummyData(*cc1);
    CopCarrier::dummyData(*cc2);
    obj._cxrs.push_back(cc1);
    obj._cxrs.push_back(cc2);

    obj._tktgCxrExcpts.push_back("wxy");
    obj._tktgCxrExcpts.push_back("z12");
  }

protected:
  NationCode _copNation;
  DateTime _versionDate;
  int _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  UserApplCode _userAppl;
  Indicator _travelAppl;
  Indicator _puNormalSpecialType;
  Indicator _puTripType;
  FareType _fareType;
  Indicator _puAppl;
  LocKey _puOrigLoc;
  LocKey _puWithinLoc;
  LocKey _constPointLoc;
  Indicator _fareTypeAppl;
  Indicator _userApplType;
  std::vector<CopCarrier*> _cxrs;
  std::vector<CarrierCode> _tktgCxrExcpts;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _copNation);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _travelAppl);
    FLATTENIZE(archive, _puNormalSpecialType);
    FLATTENIZE(archive, _puTripType);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _puAppl);
    FLATTENIZE(archive, _puOrigLoc);
    FLATTENIZE(archive, _puWithinLoc);
    FLATTENIZE(archive, _constPointLoc);
    FLATTENIZE(archive, _fareTypeAppl);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _cxrs);
    FLATTENIZE(archive, _tktgCxrExcpts);
  }

protected:
private:
  CopMinimum(const CopMinimum&);
  CopMinimum& operator=(const CopMinimum&);
};
}
