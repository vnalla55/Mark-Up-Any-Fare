//----------------------------------------------------------------------------
//
//     File:           ORACLEDBResultSet.h
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

#include "DBAccess/ORACLEDBRow.h"
#include "DBAccess/ORACLEDefineBufferCollection.h"
#include "DBAccess/ResultSet.h"

#include <oci.h>

namespace tse
{
class ORACLEConnectionTimer;
class ORACLEAdapter;
class DBAdapter;
class SQLQuery;

class ORACLEDBResultSet : public ResultSet
{
public:
  ORACLEDBResultSet(DBAdapter* dbAdapt);
  virtual ~ORACLEDBResultSet();

  bool executeQuery(SQLQuery* sqlQuery) override;

  void freeResult() override;
  int numRows() const override;
  Row* nextRow() override;

  int getInt(int columnIndex);
  long int getLong(int columnIndex);
  long long int getLongLong(int columnIndex);
  const char* getString(int columnIndex);
  char getChar(int columnIndex);
  DateTime getDateTime(int columnIndex);
  bool isNull(int columnIndex);

private:
  ORACLEAdapter* _dbAdapt = nullptr;

  OCIStmt* _ociStmt = nullptr;

  const std::string* _sqlStatement = nullptr;
  int16_t _queryID = -1;

  ORACLEDBRow* _currentRow = nullptr;
  uint32_t _currentRowIndex = 0;
  uint32_t _lastFetchSize = 0;
  uint32_t _arrayFetchSize = 1;
  bool _execAndFetch = false;

  const SQLQuery* _sqlQueryObject = nullptr;
  int long long _numFetched = 0;

  ORACLEDefineBufferCollection* _defineBuffers = nullptr;

  bool reuseDefineBuffers();
  bool createDefineBuffers();
  bool releaseDefineBuffers();
  void clearDefineBuffers();
  bool setOracleDefines();

  uint32_t getArrayFetchSize() { return _arrayFetchSize; }

  int32_t getFieldCount();

  ORACLEConnectionTimer* oracleConnectionTimer();

public:
  bool getDefineBuffer(unsigned int colIdx, ORACLEDefineBuffer*& buffer);

  ORACLEConnectionTimer* _oracleConnectionTimer = nullptr;
};

} // namespace tse

