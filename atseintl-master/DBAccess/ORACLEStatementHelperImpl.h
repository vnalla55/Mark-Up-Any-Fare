//----------------------------------------------------------------------------
//
//     File:           ORACLEStatementHelperImpl.h
//     Description:    ORACLEStatementHelperImpl
//     Created:        04/20/2009
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

#include "DBAccess/SQLStatementHelperImpl.h"

#include <string>
#include <vector>

namespace DBAccess
{

class ORACLEStatementHelperImpl : public SQLStatementHelperImpl
{
public:
  virtual void generateJoinString(const SQLStatementHelperState& state,
                                  const std::string& table1,
                                  const std::string& alias1,
                                  const std::string& joinType,
                                  const std::string& table2,
                                  const std::string& alias2,
                                  const std::vector<std::string>& joinFields,
                                  std::string& result) const override;

  virtual void generateLimitString(const SQLStatementHelperState& state,
                                   uint32_t limit,
                                   std::string& result) const override;

  virtual void generateLimitString(const SQLStatementHelperState& state,
                                   const std::string& limit,
                                   std::string& result) const override;

  // Since table mapping through ini file is disabled for Oracle
  // IgnoreTableDefMissing and ignoreTableDefReplacement return true
  virtual bool ignoreTableDefMissing() const override { return true; }
  virtual bool ignoreTableDefReplacement() const override { return true; }

  virtual void formatDateTimeString(const tse::DateTime& dt, std::string& result) const override;

  virtual void formatDateString(const tse::DateTime& dt, std::string& result) const override;

  virtual void constructStatement(const SQLStatementHelperState& state,
                                  const std::string& command,
                                  const std::string& from,
                                  const std::string& where,
                                  const std::string& groupby,
                                  const std::string& orderby,
                                  const std::string& limit,
                                  std::string& result) const override;

  virtual bool checkStateValidity(const SQLStatementHelperState& state,
                                  std::string& errorMessage) const override;
}; // class ORACLEStatementHelperImpl
} // namespace DBAccess
