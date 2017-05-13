#ifndef QUERY_TESTER_H
#define QUERY_TESTER_H

#include <string>
#include "DBAccess/DBAdapter.h"
#include "DBAccess/test/SelfRegisteringTestDBResultSetFactory.h"

namespace tse
{
class TestResultSet;
class QueryTester
{
public:
  QueryTester(DBAdapter&);
  ~QueryTester();
  void addRow(Row* row);
  std::string lastQuery();

  DBAdapter _dbAdapter;
  TestResultSet* _results;
  SelfRegisteringTestDBResultSetFactory factory;
};
}

#endif
