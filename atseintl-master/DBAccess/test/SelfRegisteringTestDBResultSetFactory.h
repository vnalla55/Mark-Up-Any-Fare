#ifndef SELFREGISTERINGTESTBERESULTSETFACTORY_H
#define SELFREGISTERINGTESTBERESULTSETFACTORY_H

#include "DBAccess/DBResultSet.h"

namespace tse
{
class DBAdapter;
class ResultSet;

class SelfRegisteringTestDBResultSetFactory : public DBResultSetFactory
{
public:
  SelfRegisteringTestDBResultSetFactory();
  virtual ~SelfRegisteringTestDBResultSetFactory();
  virtual ResultSet* getResultSet(DBAdapter*);
  void setResultSet(ResultSet*);

  ResultSet* resultSet;
};
}
#endif
