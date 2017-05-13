// ----------------------------------------------------------------------------
//  © 2006, Sabre Inc.  All rights reserved.  This software/documentation
//    is the confidential and proprietary product of Sabre Inc. Any
//    unauthorized use, reproduction, or transfer of this
//    software/documentation, in any medium, or incorporation of this
//    software/documentation into any system or publication, is strictly
//    prohibited
//
// ----------------------------------------------------------------------------

#ifndef VOLUNTARY_CHANGES_CONFIG_H
#define VOLUNTARY_CHANGES_CONFIG_H

#include "DBAccess/Flattenizable.h"

namespace tse
{

class VoluntaryChangesConfig
{
public:
  bool operator==(const VoluntaryChangesConfig& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_expireDate == rhs._expireDate) &&
            (_discDate == rhs._discDate) && (_applDate == rhs._applDate));
  }

  static void dummyData(VoluntaryChangesConfig& obj)
  {
    obj._carrier = "ABC";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._applDate = time(nullptr);
  }

private:
  CarrierCode _carrier;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _applDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _applDate);
  }

  CarrierCode& carrier() { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& applDate() { return _applDate; }
  const DateTime& applDate() const { return _applDate; }
};
}

#endif
