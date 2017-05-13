//----------------------------------------------------------------------------
//
//     File:           ORACLEDBRow.h
//     Description:
//     Created:
//     Authors:
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "DBAccess/Row.h"

namespace tse
{
class ORACLEAdapter;
class ORACLEDBResultSet;
class SQLQuery;

class ORACLEDBRow : public Row
{
public:
  int getInt(int columnIndex) const override;
  long int getLong(int columnIndex) const override;
  long long int getLongLong(int columnIndex) const override;
  const char* getString(int columnIndex) const override;
  char getChar(int columnIndex) const override;
  DateTime getDate(int columnIndex) const override;
  bool isNull(int columnIndex) const override;

private:
  friend class ORACLEDBHistoryServer;
  friend class ORACLEDBResultSet;

  DateTime getDateTime(int colIdx) const;

  void init(ORACLEAdapter* dbAdapt, ORACLEDBResultSet* resultSet, const SQLQuery* queryObject);
  void setFieldCount(int numFields);

  bool isFieldNull(int columnIndex) const;

  ORACLEAdapter* _dbAdapt = nullptr;
  ORACLEDBResultSet* _resultSet = nullptr;
  const std::string* _sqlStatement = nullptr;
  const SQLQuery* _queryObject = nullptr;
  int16_t _qryID = -1;
  int _numFields = 0;
};
} // namespace tse
