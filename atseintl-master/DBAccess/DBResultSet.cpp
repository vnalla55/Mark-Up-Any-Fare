//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "DBAccess/DBResultSet.h"

#include "Common/Logger.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/DBAccessConsts.h"
#include "DBAccess/ORACLEDBResultSet.h"
#include "DBAccess/ResultSet.h"

#include <string>

namespace tse
{
DBResultSetFactory* DBResultSet::_factory = nullptr;

log4cxx::LoggerPtr
DBResultSet::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DBResultSet"));

void
DBResultSet::setFactory(DBResultSetFactory* factory)
{
  _factory = factory;
}

tse::ResultSet*
ORACLEDBResultSetFactory::getResultSet(DBAdapter* dbAdapt)
{
  return new ORACLEDBResultSet(dbAdapt);
}

DBResultSet::DBResultSet(DBAdapter* dbAdapt) { _rs = _factory->getResultSet(dbAdapt); }

DBResultSet::~DBResultSet()
{
  if (LIKELY(_rs != nullptr))
  {
    delete _rs;
  }
}

bool
DBResultSet::executeQuery(SQLQuery* queryObject)
{
  return _rs->executeQuery(queryObject);
}

void
DBResultSet::freeResult()
{
  return _rs->freeResult();
}

int
DBResultSet::numRows() const
{
  return _rs->numRows();
}

Row*
DBResultSet::nextRow()
{
  return _rs->nextRow();
}
}
