//----------------------------------------------------------------------------
//
//     File:           SQLStatementHelperState.h
//     Description:    SQLStatementHelperState
//     Created:        6/3/2009
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

#include <string>

namespace DBAccess
{
class SQLStatementHelperState
{
public:
  const bool hasCommand() const { return _hasCommand; }
  void hasCommand(bool value) { _hasCommand = value; }

  const bool hasFrom() const { return _hasFrom; }
  void hasFrom(bool value) { _hasFrom = value; }

  const bool hasWhere() const { return _hasWhere; }
  void hasWhere(bool value) { _hasWhere = value; }

  const bool hasOrderBy() const { return _hasOrderBy; }
  void hasOrderBy(bool value) { _hasOrderBy = value; }

  const bool hasGroupBy() const { return _hasGroupBy; }
  void hasGroupBy(bool value) { _hasGroupBy = value; }

  const bool hasLimit() const { return _hasLimit; }
  void hasLimit(bool value) { _hasLimit = value; }

  void reset()
  {
    _hasCommand = false;
    _hasFrom = false;
    _hasWhere = false;
    _hasOrderBy = false;
    _hasGroupBy = false;
    _hasLimit = false;
  }

private:
  SQLStatementHelperState()
    : _hasCommand(false),
      _hasFrom(false),
      _hasWhere(false),
      _hasOrderBy(false),
      _hasGroupBy(false),
      _hasLimit(false)
  {
  }
  SQLStatementHelperState(const SQLStatementHelperState& rhs)
    : _hasCommand(rhs._hasCommand),
      _hasFrom(rhs._hasFrom),
      _hasWhere(rhs._hasWhere),
      _hasOrderBy(rhs._hasOrderBy),
      _hasGroupBy(rhs._hasGroupBy),
      _hasLimit(rhs._hasLimit)
  {
  }

  ~SQLStatementHelperState() {}

  SQLStatementHelperState& operator=(const SQLStatementHelperState& rhs)
  {
    if (this == &rhs)
      return *this;

    _hasCommand = rhs._hasCommand;
    _hasFrom = rhs._hasFrom;
    _hasWhere = rhs._hasWhere;
    _hasOrderBy = rhs._hasOrderBy;
    _hasGroupBy = rhs._hasGroupBy;
    _hasLimit = rhs._hasLimit;

    return *this;
  }

  bool _hasCommand;
  bool _hasFrom;
  bool _hasWhere;
  bool _hasOrderBy;
  bool _hasGroupBy;
  bool _hasLimit;

  friend class SQLStatementHelper;
};

} // namespace DBAccess

