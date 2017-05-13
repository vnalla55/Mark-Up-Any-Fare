//----------------------------------------------------------------------------
//
//  File:           FareTypeMatrix.h
//  Description:    FareTypeMatrix processing data
//  Created:        2/4/2004
//  Authors:        Roger Kelly
//
//  Updates:
//
//  2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/CabinType.h"
#include "Common/FareTypeDesignator.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareTypeMatrix
{
public:
  FareTypeMatrix()
    : _seqNo(0),
      _fareTypeAppl(' '),
      _restrInd(' '),
      _fareTypeDisplay(' '),
      _versioninheritedInd(' '),
      _versionDisplayInd(' ')
  {
  }
  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  CabinType& cabin() { return _cabin; }
  const CabinType& cabin() const { return _cabin; }

  FareTypeDesignator& fareTypeDesig() { return _fareTypeDesig; }
  const FareTypeDesignator& fareTypeDesig() const { return _fareTypeDesig; }

  Indicator& fareTypeAppl() { return _fareTypeAppl; }
  const Indicator& fareTypeAppl() const { return _fareTypeAppl; }

  Indicator& restrInd() { return _restrInd; }
  const Indicator& restrInd() const { return _restrInd; }

  Indicator& fareTypeDisplay() { return _fareTypeDisplay; }
  const Indicator& fareTypeDisplay() const { return _fareTypeDisplay; }

  Description& description() { return _description; }
  const Description& description() const { return _description; }

  Indicator& versioninheritedInd() { return _versioninheritedInd; }
  const Indicator& versioninheritedInd() const { return _versioninheritedInd; }

  Indicator& versionDisplayInd() { return _versionDisplayInd; }
  const Indicator& versionDisplayInd() const { return _versionDisplayInd; }

  bool operator==(const FareTypeMatrix& rhs) const
  {
    return ((_fareType == rhs._fareType) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) && (_cabin == rhs._cabin) &&
            (_fareTypeDesig == rhs._fareTypeDesig) && (_fareTypeAppl == rhs._fareTypeAppl) &&
            (_restrInd == rhs._restrInd) && (_fareTypeDisplay == rhs._fareTypeDisplay) &&
            (_description == rhs._description) &&
            (_versioninheritedInd == rhs._versioninheritedInd) &&
            (_versionDisplayInd == rhs._versionDisplayInd));
  }

  static void dummyData(FareTypeMatrix& obj)
  {
    obj._fareType = "zzzzzzzz";
    obj._seqNo = 1;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);

    CabinType::dummyData(obj._cabin);
    FareTypeDesignator::dummyData(obj._fareTypeDesig);

    obj._fareTypeAppl = 'B';
    obj._restrInd = 'C';
    obj._fareTypeDisplay = 'D';
    obj._description = "bbbbbbbb";
    obj._versioninheritedInd = 'E';
    obj._versionDisplayInd = 'F';
  }

private:
  FareType _fareType;
  int _seqNo;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  CabinType _cabin;
  FareTypeDesignator _fareTypeDesig;
  Indicator _fareTypeAppl;
  Indicator _restrInd;
  Indicator _fareTypeDisplay;
  Description _description;
  Indicator _versioninheritedInd;
  Indicator _versionDisplayInd;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _cabin);
    FLATTENIZE(archive, _fareTypeDesig);
    FLATTENIZE(archive, _fareTypeAppl);
    FLATTENIZE(archive, _restrInd);
    FLATTENIZE(archive, _fareTypeDisplay);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _versioninheritedInd);
    FLATTENIZE(archive, _versionDisplayInd);
  }

};
}

