//----------------------------------------------------------------------------
//	   © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{

class TariffMileageAddon
{
public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  LocCode& unpublishedAddonLoc() { return _unpublishedAddonLoc; }
  const LocCode& unpublishedAddonLoc() const { return _unpublishedAddonLoc; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  int& milesAdded() { return _milesAdded; }
  const int& milesAdded() const { return _milesAdded; }

  LocCode& publishedLoc() { return _publishedLoc; }
  const LocCode& publishedLoc() const { return _publishedLoc; }

  bool operator==(const TariffMileageAddon& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_unpublishedAddonLoc == rhs._unpublishedAddonLoc) &&
            (_globalDir == rhs._globalDir) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_milesAdded == rhs._milesAdded) &&
            (_publishedLoc == rhs._publishedLoc));
  }

  static void dummyData(TariffMileageAddon& obj)
  {
    obj._carrier = "ABC";
    obj._unpublishedAddonLoc = "DEFGHIJK";
    obj._globalDir = GlobalDirection::US;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._milesAdded = 1;
    obj._publishedLoc = "LMNOPQRS";
  }

private:
  CarrierCode _carrier;
  LocCode _unpublishedAddonLoc;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  int _milesAdded = 0;
  LocCode _publishedLoc;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _unpublishedAddonLoc);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _milesAdded);
    FLATTENIZE(archive, _publishedLoc);
  }

};
}
