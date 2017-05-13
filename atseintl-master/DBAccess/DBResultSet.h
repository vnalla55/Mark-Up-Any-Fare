//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"

namespace tse
{
class DBAdapter;
class SQLQuery;
class ResultSet;
class Row;

class DBResultSetFactory
{
public:
  DBResultSetFactory() {};
  virtual ~DBResultSetFactory() {};
  virtual ResultSet* getResultSet(DBAdapter* dbAdapt) = 0;
};

class ORACLEDBResultSetFactory : public DBResultSetFactory
{
public:
  ORACLEDBResultSetFactory() {};
  virtual ~ORACLEDBResultSetFactory() {};
  virtual ResultSet* getResultSet(DBAdapter* dbAdapt) override;
};

class DBResultSet
{
  friend class SelfRegisteringTestDBResultSetFactory;

public:
  DBResultSet(DBAdapter* dbAdapt);
  virtual ~DBResultSet();

  virtual bool executeQuery(SQLQuery* sqlQuery);

  virtual void freeResult();
  virtual int numRows() const;
  virtual Row* nextRow();
  static void setFactory(DBResultSetFactory* factory);

private:
  ResultSet* _rs;
  static DBResultSetFactory* _factory;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

