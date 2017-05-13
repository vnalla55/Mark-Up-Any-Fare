//----------------------------------------------------------------------------
//	    2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class ServiceGroupInfo
{
public:
  ServiceGroupInfo() {}

  ServiceGroup& svcGroup() { return _svcGroup; }
  const ServiceGroup& svcGroup() const { return _svcGroup; }

  std::string& definition() { return _definition; }
  const std::string& definition() const { return _definition; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  bool operator==(const ServiceGroupInfo& rhs) const
  {
    return ((_svcGroup == rhs._svcGroup) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_expireDate == rhs._expireDate) && (_discDate == rhs._discDate) &&
            (_definition == rhs._definition));
  }

  static void dummyData(ServiceGroupInfo& obj)
  {
    obj._svcGroup = "AB";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._definition = "SOME";
  }

private:
  ServiceGroup _svcGroup;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  std::string _definition;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _svcGroup);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _definition);
  }

};
}
