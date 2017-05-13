//----------------------------------------------------------------------------
//
//     File:           SQLStatementHelper.h
//     Description:    SQLStatementHelper
//     Created:        04/20/2009
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright @2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/SQLStatementHelperState.h"

#include <string>
#include <vector>

#include <stdint.h>

namespace tse
{
class DateTime;
}

namespace DBAccess
{

// Forward declarations
//
class SQLStatementHelperImpl;

class SQLStatementHelper
{
public:
  SQLStatementHelper();
  SQLStatementHelper(const SQLStatementHelper& rhs);

  ~SQLStatementHelper();

  SQLStatementHelper& operator=(const SQLStatementHelper& rhs);

  void generateJoinString(const std::string& table1,
                          const std::string& alias1,
                          const std::string& joinType,
                          const std::string& table2,
                          const std::string& alias2,
                          const std::vector<std::string>& joinFields,
                          std::string& result) const;

  void generateLimitString(uint32_t limit, std::string& result) const;
  void generateLimitString(const std::string& limit, std::string& result) const;

  bool ignoreTableDefMissing() const;
  bool ignoreTableDefReplacement() const;

  void formatDateTimeString(const tse::DateTime& dt, std::string& result) const;

  void formatDateString(const tse::DateTime& dt, std::string& result) const;

  void constructStatement(const std::string& command,
                          const std::string& from,
                          const std::string& where,
                          const std::string& groupby,
                          const std::string& orderby,
                          const std::string& limit,
                          std::string& result) const;

  void checkStateValidity() const;
  void resetState() { _state.reset(); }

  const bool hasCommand() const { return _state.hasCommand(); }
  void hasCommand(bool value) { _state.hasCommand(value); }

  const bool hasFrom() const { return _state.hasFrom(); }
  void hasFrom(bool value) { _state.hasFrom(value); }

  const bool hasWhere() const { return _state.hasWhere(); }
  void hasWhere(bool value) { _state.hasWhere(value); }

  const bool hasOrderBy() const { return _state.hasOrderBy(); }
  void hasOrderBy(bool value) { _state.hasOrderBy(value); }

  const bool hasGroupBy() const { return _state.hasGroupBy(); }
  void hasGroupBy(bool value) { _state.hasGroupBy(value); }

  const bool hasLimit() const { return _state.hasLimit(); }
  void hasLimit(bool value) { _state.hasLimit(value); }

private:
  const SQLStatementHelperImpl& _impl;
  SQLStatementHelperState _state;

}; // class SQLStatementHelper

} // namespace DBAccess

