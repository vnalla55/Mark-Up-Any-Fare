#ifndef TESTRESULTSET_H
#define TESTRESULTSET_H

#include <string>
#include "DBAccess/ResultSet.h"

namespace tse
{
class DBAdapter;
class SQLQuery;

class TestResultSet : public ResultSet
{
public:
  TestResultSet(DBAdapter* adapter);

  bool executeQuery(SQLQuery* sqlQuery);
  /* virtual */ bool executeQuery(int16_t queryID, const std::string* sqlStatement);
  void freeResult();
  int numRows() const;
  Row* nextRow();
  void addRow(Row*);
  static std::string lastQuery();
  static bool wasFreed();

  std::vector<Row*> _rows;
  unsigned int _count;

  // these must be static because the ResultSet object gets destructed
  // before the test can look at them. Revisit design!
  static std::string _lastQuery;
  static bool _wasFreed;
};
}

#endif
