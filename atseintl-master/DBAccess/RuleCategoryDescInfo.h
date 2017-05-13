//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

/**
 * Record 3 descriptions
 */
class RuleCategoryDescInfo
{
public:
  RuleCategoryDescInfo() : _category(0) {}

  CatNumber& category() { return _category; }
  const CatNumber& category() const { return _category; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  std::string& shortDescription() { return _shortDescription; }
  const std::string& shortDescription() const { return _shortDescription; }

  std::string& defaultMsg() { return _defaultMsg; }
  const std::string& defaultMsg() const { return _defaultMsg; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  bool operator==(const RuleCategoryDescInfo& rhs) const
  {
    return ((_category == rhs._category) && (_description == rhs._description) &&
            (_shortDescription == rhs._shortDescription) && (_defaultMsg == rhs._defaultMsg) &&
            (_createDate == rhs._createDate));
  }

  static void dummyData(RuleCategoryDescInfo& obj)
  {
    obj._category = 1;
    obj._description = "aaaaaaaa";
    obj._shortDescription = "bbbbbbbb";
    obj._defaultMsg = "cccccccc";
    obj._createDate = time(nullptr);
  }

private:
  CatNumber _category;
  std::string _description;
  std::string _shortDescription;
  std::string _defaultMsg;
  DateTime _createDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _category);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _shortDescription);
    FLATTENIZE(archive, _defaultMsg);
    FLATTENIZE(archive, _createDate);
  }
};

} // namespace tse

