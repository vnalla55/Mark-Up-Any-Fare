#pragma once
//----------------------------------------------------------------------------
//
// Copyright Sabre 2010
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{

class BaggageSectorException
{
public:
  // Primary Key Fields

  CarrierCode& carrier()
  {
    return _carrier;
  };
  const CarrierCode& carrier() const
  {
    return _carrier;
  };

  DateTime& createDate()
  {
    return _createDate;
  };
  const DateTime& createDate() const
  {
    return _createDate;
  };

  // Non-Key Data
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Directionality& directionality() { return _directionality; }
  const Directionality& directionality() const { return _directionality; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  bool operator==(const BaggageSectorException& rhs) const
  {
    return (_carrier == rhs._carrier) && (_createDate == rhs._createDate) &&
           (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
           (_discDate == rhs._discDate) && (_directionality == rhs._directionality) &&
           (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(BaggageSectorException& obj)
  {
    obj._carrier = "AA";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._directionality = FROM;
    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);
  }

private:
  CarrierCode _carrier;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  Directionality _directionality;
  LocKey _loc1;
  LocKey _loc2;

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_carrier
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_directionality
           & ptr->_loc1
           & ptr->_loc2;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
  }
};

} // namespace tse

