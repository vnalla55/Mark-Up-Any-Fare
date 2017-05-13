//----------------------------------------------------------------------------
//
//     File:           SQLStatementHelper.cpp
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

#include "DBAccess/SQLStatementHelper.h"

#include "Common/TSEException.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/SQLStatementHelperImpl.h"

namespace DBAccess
{
SQLStatementHelper::SQLStatementHelper() : _impl(tse::DataManager::getSQLStatementHelperImpl())
{
}

SQLStatementHelper::SQLStatementHelper(const SQLStatementHelper& rhs)
  : _impl(tse::DataManager::getSQLStatementHelperImpl()), _state(rhs._state)
{
}

SQLStatementHelper::~SQLStatementHelper() {}

SQLStatementHelper&
SQLStatementHelper::
operator=(const SQLStatementHelper& rhs)
{
  if (this == &rhs)
    return *this;

  _state = rhs._state;

  return *this;
}

void
SQLStatementHelper::generateJoinString(const std::string& table1,
                                       const std::string& alias1,
                                       const std::string& joinType,
                                       const std::string& table2,
                                       const std::string& alias2,
                                       const std::vector<std::string>& joinFields,
                                       std::string& result) const
{
  _impl.generateJoinString(_state, table1, alias1, joinType, table2, alias2, joinFields, result);
}

void
SQLStatementHelper::generateLimitString(uint32_t limit, std::string& result) const
{
  return _impl.generateLimitString(_state, limit, result);
}

void
SQLStatementHelper::generateLimitString(const std::string& limit, std::string& result) const
{
  return _impl.generateLimitString(_state, limit, result);
}

bool
SQLStatementHelper::ignoreTableDefMissing() const
{
  return _impl.ignoreTableDefMissing();
}

bool
SQLStatementHelper::ignoreTableDefReplacement() const
{
  return _impl.ignoreTableDefReplacement();
}

void
SQLStatementHelper::formatDateTimeString(const tse::DateTime& dt, std::string& result) const
{
  _impl.formatDateTimeString(dt, result);
}

void
SQLStatementHelper::formatDateString(const tse::DateTime& dt, std::string& result) const
{
  _impl.formatDateString(dt, result);
}

void
SQLStatementHelper::constructStatement(const std::string& command,
                                       const std::string& from,
                                       const std::string& where,
                                       const std::string& groupby,
                                       const std::string& orderby,
                                       const std::string& limit,
                                       std::string& result) const
{
  checkStateValidity();
  _impl.constructStatement(_state, command, from, where, groupby, orderby, limit, result);
}

void
SQLStatementHelper::checkStateValidity() const
{
  std::string errorMessage;
  if (!_impl.checkStateValidity(_state, errorMessage))
  {
    throw tse::TSEException(tse::TSEException::NO_ERROR, errorMessage.c_str());
  }
}
}
