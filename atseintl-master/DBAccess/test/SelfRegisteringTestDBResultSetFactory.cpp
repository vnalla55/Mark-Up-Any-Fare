#include "DBAccess/test/SelfRegisteringTestDBResultSetFactory.h"

#include "DBAccess/DBAdapter.h"
#include "DBAccess/ResultSet.h"

using namespace tse;

SelfRegisteringTestDBResultSetFactory::SelfRegisteringTestDBResultSetFactory() : resultSet(0)
{
  DBResultSet::_factory = this;
}

/*virtual*/ SelfRegisteringTestDBResultSetFactory::~SelfRegisteringTestDBResultSetFactory()
{
  // Memory will be freed by DBResultSet destructor
  resultSet = 0;
}

/*virtual*/ ResultSet*
SelfRegisteringTestDBResultSetFactory::getResultSet(DBAdapter* dbAdapter)
{
  return resultSet;
}

void
SelfRegisteringTestDBResultSetFactory::setResultSet(ResultSet* resultSet)
{
  this->resultSet = resultSet;
}
