// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class ServicesSubGroup
{
public:
  ServicesSubGroup() {}

  ServiceGroup& serviceGroup() { return _serviceGroup; }
  const ServiceGroup& serviceGroup() const { return _serviceGroup; }

  ServiceGroup& serviceSubGroup() { return _serviceSubGroup; }
  const ServiceGroup& serviceSubGroup() const { return _serviceSubGroup; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  std::string& definition() { return _definition; }
  const std::string& definition() const { return _definition; }

  bool operator==(const ServicesSubGroup& second) const
  {
    return (_serviceGroup == second._serviceGroup) &&
           (_serviceSubGroup == second._serviceSubGroup) && (_createDate == second._createDate) &&
           (_definition == second._definition);
  }

  static void dummyData(ServicesSubGroup& obj)
  {
    obj._serviceGroup = "NOP";
    obj._serviceSubGroup = "QRS";
    obj._createDate = time(nullptr);
    obj._definition = "TEST";
  }

private:
  ServiceGroup _serviceGroup;
  ServiceGroup _serviceSubGroup;
  DateTime _createDate;
  std::string _definition;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _serviceGroup);
    FLATTENIZE(archive, _serviceSubGroup);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _definition);
  }
};
}

