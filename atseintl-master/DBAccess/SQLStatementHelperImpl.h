//----------------------------------------------------------------------------
//
//     File:           SQLStatementHelperImpl.h
//     Description:    SQLStatementHelperImpl
//     Created:        04/21/2009
//     Authors:        Andrew Ahmad
//
//     Notes:          This is an implementation class and should not be
//                     used directly. Instead, create an instance of
//                     SQLStatementHelper and use it.
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

#include <string>
#include <vector>

#include <cstdint>

namespace tse
{
class DateTime;
}

namespace DBAccess
{

// Forward declarations
//
class SQLStatementHelperState;

class SQLStatementHelperImpl
{
public:
  SQLStatementHelperImpl() = default;

  SQLStatementHelperImpl(const SQLStatementHelperImpl&) = delete;
  SQLStatementHelperImpl& operator=(const SQLStatementHelperImpl&) = delete;

  virtual ~SQLStatementHelperImpl() = default;

  virtual void generateJoinString(const SQLStatementHelperState& state,
                                  const std::string& table1,
                                  const std::string& alias1,
                                  const std::string& joinType,
                                  const std::string& table2,
                                  const std::string& alias2,
                                  const std::vector<std::string>& joinFields,
                                  std::string& result) const = 0;

  virtual void generateLimitString(const SQLStatementHelperState& state,
                                   uint32_t limit,
                                   std::string& result) const = 0;

  virtual void generateLimitString(const SQLStatementHelperState& state,
                                   const std::string& limit,
                                   std::string& result) const = 0;

  virtual bool ignoreTableDefMissing() const = 0;
  virtual bool ignoreTableDefReplacement() const = 0;

  virtual void formatDateTimeString(const tse::DateTime& dt, std::string& result) const = 0;

  virtual void formatDateString(const tse::DateTime& dt, std::string& result) const = 0;

  virtual void constructStatement(const SQLStatementHelperState& state,
                                  const std::string& command,
                                  const std::string& from,
                                  const std::string& where,
                                  const std::string& groupby,
                                  const std::string& orderby,
                                  const std::string& limit,
                                  std::string& result) const = 0;

  virtual bool
  checkStateValidity(const SQLStatementHelperState& state, std::string& errorMessage) const = 0;
}; // class SQLStatementHelperImpl
} // namespace DBAccess
