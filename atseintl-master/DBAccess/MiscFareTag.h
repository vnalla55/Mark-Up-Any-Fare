//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class MiscFareTag : public RuleItemInfo
{
public:
  MiscFareTag()
    : _unavailtag(' '),
      _constInd(' '),
      _prorateInd(' '),
      _diffcalcInd(' '),
      _refundcalcInd(' '),
      _proportionalInd(' '),
      _curradjustInd(' '),
      _fareClassType1Ind(' '),
      _fareClassType2Ind(' '),
      _geoTblItemNo(0),
      _inhibit(' ')
  {
  }

  virtual ~MiscFareTag() {}

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

  Indicator& constInd() { return _constInd; }
  const Indicator& constInd() const { return _constInd; }

  Indicator& prorateInd() { return _prorateInd; }
  const Indicator& prorateInd() const { return _prorateInd; }

  Indicator& diffcalcInd() { return _diffcalcInd; }
  const Indicator& diffcalcInd() const { return _diffcalcInd; }

  Indicator& refundcalcInd() { return _refundcalcInd; }
  const Indicator& refundcalcInd() const { return _refundcalcInd; }

  Indicator& proportionalInd() { return _proportionalInd; }
  const Indicator& proportionalInd() const { return _proportionalInd; }

  Indicator& curradjustInd() { return _curradjustInd; }
  const Indicator& curradjustInd() const { return _curradjustInd; }

  Indicator& fareClassType1Ind() { return _fareClassType1Ind; }
  const Indicator& fareClassType1Ind() const { return _fareClassType1Ind; }

  FareClassCode& fareClassType1() { return _fareClassType1; }
  const FareClassCode& fareClassType1() const { return _fareClassType1; }

  Indicator& fareClassType2Ind() { return _fareClassType2Ind; }
  const Indicator& fareClassType2Ind() const { return _fareClassType2Ind; }

  FareClassCode& fareClassType2() { return _fareClassType2; }
  const FareClassCode& fareClassType2() const { return _fareClassType2; }

  int& geoTblItemNo() { return _geoTblItemNo; }
  const int& geoTblItemNo() const { return _geoTblItemNo; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const MiscFareTag& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_unavailtag == rhs._unavailtag) &&
            (_constInd == rhs._constInd) && (_prorateInd == rhs._prorateInd) &&
            (_diffcalcInd == rhs._diffcalcInd) && (_refundcalcInd == rhs._refundcalcInd) &&
            (_proportionalInd == rhs._proportionalInd) && (_curradjustInd == rhs._curradjustInd) &&
            (_fareClassType1Ind == rhs._fareClassType1Ind) &&
            (_fareClassType1 == rhs._fareClassType1) &&
            (_fareClassType2Ind == rhs._fareClassType2Ind) &&
            (_fareClassType2 == rhs._fareClassType2) && (_geoTblItemNo == rhs._geoTblItemNo) &&
            (_inhibit == rhs._inhibit));
  }

  static void dummyData(MiscFareTag& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailtag = 'A';
    obj._constInd = 'B';
    obj._prorateInd = 'C';
    obj._diffcalcInd = 'D';
    obj._refundcalcInd = 'E';
    obj._proportionalInd = 'F';
    obj._curradjustInd = 'G';
    obj._fareClassType1Ind = 'H';
    obj._fareClassType1 = "IJKLMNOP";
    obj._fareClassType2Ind = 'Q';
    obj._fareClassType2 = "RSTUVWXY";
    obj._geoTblItemNo = 1;
    obj._inhibit = 'Z';
  }

protected:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _unavailtag;
  Indicator _constInd;
  Indicator _prorateInd;
  Indicator _diffcalcInd;
  Indicator _refundcalcInd;
  Indicator _proportionalInd;
  Indicator _curradjustInd;
  Indicator _fareClassType1Ind;
  FareClassCode _fareClassType1;
  Indicator _fareClassType2Ind;
  FareClassCode _fareClassType2;
  int _geoTblItemNo;
  Indicator _inhibit;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailtag);
    FLATTENIZE(archive, _constInd);
    FLATTENIZE(archive, _prorateInd);
    FLATTENIZE(archive, _diffcalcInd);
    FLATTENIZE(archive, _refundcalcInd);
    FLATTENIZE(archive, _proportionalInd);
    FLATTENIZE(archive, _curradjustInd);
    FLATTENIZE(archive, _fareClassType1Ind);
    FLATTENIZE(archive, _fareClassType1);
    FLATTENIZE(archive, _fareClassType2Ind);
    FLATTENIZE(archive, _fareClassType2);
    FLATTENIZE(archive, _geoTblItemNo);
    FLATTENIZE(archive, _inhibit);
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_unavailtag
           & ptr->_constInd
           & ptr->_prorateInd
           & ptr->_diffcalcInd
           & ptr->_refundcalcInd
           & ptr->_proportionalInd
           & ptr->_curradjustInd
           & ptr->_fareClassType1Ind
           & ptr->_fareClassType1
           & ptr->_fareClassType2Ind
           & ptr->_fareClassType2
           & ptr->_geoTblItemNo
           & ptr->_inhibit;
  }
};
}
