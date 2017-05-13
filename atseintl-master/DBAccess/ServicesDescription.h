#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class ServicesDescription
{
public:
  ServicesDescription() {}

  ServiceGroupDescription& value() { return _value; }
  const ServiceGroupDescription& value() const { return _value; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  bool operator==(const ServicesDescription& second) const
  {
    return (_value == second._value) && (_description == second._description) &&
           (_createDate == second._createDate) && (_expireDate == second._expireDate) &&
           (_effDate == second._effDate) && (_discDate == second._discDate);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _value);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _description);
  }

  static void dummyData(ServicesDescription& obj)
  {
    obj._value = "AA";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._description = "TEST";
  }

private:
  ServiceGroupDescription _value;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  std::string _description;

  friend class SerializationTestBase;

};
}

