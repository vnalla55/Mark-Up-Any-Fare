#include "DBAccess/test/QueryTester.h"
#include "test/include/CppUnitHelperMacros.h"
#include "DBAccess/test/TestResultSet.h"

using namespace std;
using namespace tse;

QueryTester::QueryTester(DBAdapter& dbAdapter) : _dbAdapter(dbAdapter)
{
  _results = new TestResultSet(&dbAdapter);
  factory.setResultSet(_results);
}

QueryTester::~QueryTester() { CPPUNIT_ASSERT(TestResultSet::wasFreed()); }

void
QueryTester::addRow(Row* row)
{
  _results->addRow(row);
}

string
QueryTester::lastQuery()
{
  return TestResultSet::lastQuery();
}
