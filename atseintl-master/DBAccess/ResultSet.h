//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "DBAccess/Row.h"

namespace tse
{
class DBAdapter;
class SQLQuery;

class ResultSet
{
public:
  ResultSet(DBAdapter* dbAdapt) {}
  virtual ~ResultSet() {}

  virtual bool executeQuery(SQLQuery* sqlQuery) = 0;

  virtual void freeResult() = 0;
  virtual int numRows() const = 0;
  virtual Row* nextRow() = 0;
};

} // namespace tse

