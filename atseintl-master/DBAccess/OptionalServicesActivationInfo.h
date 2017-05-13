//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class OptionalServicesActivationInfo
{
public:
  OptionalServicesActivationInfo() : _userApplType(' ') {}

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  ServiceGroup& groupCode() { return _groupCode; }
  const ServiceGroup& groupCode() const { return _groupCode; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  std::string& application() { return _application; }
  const std::string& application() const { return _application; }

  bool operator==(const OptionalServicesActivationInfo& second) const
  {
    return (_userApplType == second._userApplType) && (_userAppl == second._userAppl) &&
           (_groupCode == second._groupCode) && (_createDate == second._createDate) &&
           (_expireDate == second._expireDate) && (_effDate == second._effDate) &&
           (_discDate == second._discDate) && (_application == second._application);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _groupCode);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _application);
  }

  static void dummyData(OptionalServicesActivationInfo& obj)
  {
    obj._userApplType = 'C';
    obj._userAppl = "CCCC";
    obj._groupCode = "EEE";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._application = "PRICING";
  }

private:
  Indicator _userApplType;
  UserApplCode _userAppl;
  ServiceGroup _groupCode;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  std::string _application;

  friend class SerializationTestBase;
};
}

