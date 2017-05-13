//----------------------------------------------------------------------------
//
//        File:           Nation.h
//        Description:    Nation
//        Created:        2/4/2004
//     Authors:        Roger Kelly
//
//        Updates:
//
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef NATION_H
#define NATION_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class Nation
{
public:
  Nation() : _isonumericCode(0) {}

  bool operator==(const Nation& rhs) const
  {
    return ((_nation == rhs._nation) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_expireDate == rhs._expireDate) &&
            (_discDate == rhs._discDate) && (_isonumericCode == rhs._isonumericCode) &&
            (_subArea == rhs._subArea) && (_area == rhs._area) &&
            (_isoalphaCode == rhs._isoalphaCode) && (_primeCur == rhs._primeCur) &&
            (_alternateCur == rhs._alternateCur) && (_conversionCur == rhs._conversionCur) &&
            (_description == rhs._description));
  }

  Nation* newDuplicate()
  {
    // Warning!  Caller is responsible for deleting this when finished!
    Nation* retval = new Nation;

    retval->_nation = _nation;
    retval->_createDate = _createDate;
    retval->_effDate = _effDate;
    retval->_expireDate = _expireDate;
    retval->_discDate = _discDate;
    retval->_isonumericCode = _isonumericCode;
    retval->_subArea = _subArea;
    retval->_area = _area;
    retval->_isoalphaCode = _isoalphaCode;
    retval->_primeCur = _primeCur;
    retval->_alternateCur = _alternateCur;
    retval->_conversionCur = _conversionCur;
    retval->_description = _description;

    return retval;
  }

  static void dummyData(Nation& obj)
  {
    obj._nation = "ABCD";
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._isonumericCode = 1;
    obj._subArea = "EF";
    obj._area = "GHIJ";
    obj._isoalphaCode = "KLM";
    obj._primeCur = "NOP";
    obj._alternateCur = "QRS";
    obj._conversionCur = "TUV";
    obj._description = "aaaaaaaa";
  }

private:
  NationCode _nation;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _expireDate;
  DateTime _discDate;
  ISONumericCode _isonumericCode;
  IATASubAreaCode _subArea;
  IATAAreaCode _area;
  ISOAlphaCode _isoalphaCode;
  CurrencyCode _primeCur;
  CurrencyCode _alternateCur;
  CurrencyCode _conversionCur;
  Description _description;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _isonumericCode);
    FLATTENIZE(archive, _subArea);
    FLATTENIZE(archive, _area);
    FLATTENIZE(archive, _isoalphaCode);
    FLATTENIZE(archive, _primeCur);
    FLATTENIZE(archive, _alternateCur);
    FLATTENIZE(archive, _conversionCur);
    FLATTENIZE(archive, _description);
  }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  ISONumericCode& isonumericCode() { return _isonumericCode; }
  const ISONumericCode& isonumericCode() const { return _isonumericCode; }

  IATASubAreaCode& subArea() { return _subArea; }
  const IATASubAreaCode& subArea() const { return _subArea; }

  IATAAreaCode& area() { return _area; }
  const IATAAreaCode& area() const { return _area; }

  ISOAlphaCode& isoalphaCode() { return _isoalphaCode; }
  const ISOAlphaCode& isoalphaCode() const { return _isoalphaCode; }

  CurrencyCode& primeCur() { return _primeCur; }
  const CurrencyCode& primeCur() const { return _primeCur; }

  CurrencyCode& alternateCur() { return _alternateCur; }
  const CurrencyCode& alternateCur() const { return _alternateCur; }

  CurrencyCode& conversionCur() { return _conversionCur; }
  const CurrencyCode& conversionCur() const { return _conversionCur; }

  Description& description() { return _description; }
  const Description& description() const { return _description; }
};
}

#endif
