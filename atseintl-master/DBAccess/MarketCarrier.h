//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{
class MarketCarrier
{
public:
  MarketCarrier() : _inhibit(' ') {}
  ~MarketCarrier() {};

  LocCode& market1() { return _market1; }
  const LocCode& market1() const { return _market1; }
  LocCode& market2() { return _market2; }
  const LocCode& market2() const { return _market2; }
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }
  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }
  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  const Indicator inhibit() const
  {
    return _inhibit;
  };
  Indicator& inhibit()
  {
    return _inhibit;
  };

  bool operator==(const MarketCarrier& rhs) const
  {
    return ((_market1 == rhs._market1) && (_market2 == rhs._market2) &&
            (_carrier == rhs._carrier) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_expireDate == rhs._expireDate) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(MarketCarrier& obj)
  {
    obj._market1 = "ABCDE";
    obj._market2 = "FGHIJ";
    obj._carrier = "KLM";
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._inhibit = 'N';
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

public:
  LocCode _market1;
  LocCode _market2;
  CarrierCode _carrier;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _expireDate;
  Indicator _inhibit; // Inhibit now checked at App Level

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _market1);
    FLATTENIZE(archive, _market2);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _inhibit);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_market1
           & ptr->_market2
           & ptr->_carrier
           & ptr->_createDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_expireDate
           & ptr->_inhibit;
  }

};

}// tse

