#include "DBAccess/test/TestResultSet.h"
#include "DBAccess/SQLQuery.h"

using namespace std;
using namespace tse;

bool TestResultSet::_wasFreed;
string TestResultSet::_lastQuery;

TestResultSet::TestResultSet(DBAdapter* adapter) : ResultSet(adapter), _count(0)
{
  TestResultSet::_wasFreed = false;
  TestResultSet::_lastQuery = "";
}

bool
TestResultSet::executeQuery(SQLQuery* sqlQuery)
{
  TestResultSet::_lastQuery = *sqlQuery;
  return true;
}

bool
TestResultSet::executeQuery(int16_t queryID, const std::string* sqlStatement)
{
  TestResultSet::_lastQuery = *sqlStatement;
  return true;
}

void
TestResultSet::freeResult()
{
  TestResultSet::_wasFreed = true;
}

int
TestResultSet::numRows() const
{
  return _rows.size();
}

Row*
TestResultSet::nextRow()
{
  if (_count >= _rows.size())
    return 0;
  return _rows[_count++];
}

void
TestResultSet::addRow(Row* row)
{
  _rows.push_back(row);
}

/*static*/ std::string
TestResultSet::lastQuery()
{
  return TestResultSet::_lastQuery;
}

/*static*/ bool
TestResultSet::wasFreed()
{
  return TestResultSet::_wasFreed;
}
