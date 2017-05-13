//----------------------------------------------------------------------------
//	   © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#ifndef MULTITRANSPORT_H
#define MULTITRANSPORT_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class MultiTransport
{
public:
  MultiTransport() : _domAppl(' '), _intlAppl(' ') {}
  bool operator==(const MultiTransport& rhs) const
  {
    return ((_multitranscity == rhs._multitranscity) && (_carrier == rhs._carrier) &&
            (_multitransLoc == rhs._multitransLoc) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_domAppl == rhs._domAppl) &&
            (_intlAppl == rhs._intlAppl));
  }

  static void dummyData(MultiTransport& obj)
  {
    obj._multitranscity = "aaaaaaaa";
    obj._carrier = "ABC";
    obj._multitransLoc = "bbbbbbbb";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._domAppl = 'D';
    obj._intlAppl = 'E';
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  LocCode _multitranscity;
  CarrierCode _carrier;
  LocCode _multitransLoc;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _domAppl;
  Indicator _intlAppl;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _multitranscity);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _multitransLoc);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _domAppl);
    FLATTENIZE(archive, _intlAppl);
  }

protected:
public:
  LocCode& multitranscity() { return _multitranscity; }
  const LocCode& multitranscity() const { return _multitranscity; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  LocCode& multitransLoc() { return _multitransLoc; }
  const LocCode& multitransLoc() const { return _multitransLoc; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& domAppl() { return _domAppl; }
  const Indicator& domAppl() const { return _domAppl; }

  Indicator& intlAppl() { return _intlAppl; }
  const Indicator& intlAppl() const { return _intlAppl; }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_multitranscity
           & ptr->_carrier
           & ptr->_multitransLoc
           & ptr->_expireDate
           & ptr->_createDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_domAppl
           & ptr->_intlAppl;
  }
};
}

#endif
