//----------------------------------------------------------------------------
//
//  File:    ORACLEDBHistoryServer.C
//  Created: Jan 14, 2009
//  Authors: Roger Kelly
//
//  Description:
//     ORACLEDBHistoryServer is a DBServer implementation for doing data access and
//     object relational mapping from the ORACLE database for ATSE International.
//
//  Change History:
//    date - initials - description
//
//  Copyright Sabre 2009
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------
#include "DBAccess/ORACLEDBHistoryServer.h"

#include "Common/Config/ConfigMan.h"
#include "DBAccess/ORACLEAdapter.h"
#include "DBAccess/SQLQuery.h"

#include <boost/thread/mutex.hpp>

#include <string>
#include <vector>

namespace tse
{
bool ORACLEDBHistoryServer::_initialized;
boost::mutex ORACLEDBHistoryServer::_mutex;

//---------------------------------------------------------------------------
// init  function
// initialize DBAdapter; load SQL Defs and table names
// file that DBAs place on servers for physical table mapping
//---------------------------------------------------------------------------
bool
ORACLEDBHistoryServer::init(std::string& dbConnGroup,
                            std::string& dbSection,
                            std::string& user,
                            std::string& pass,
                            std::string& database,
                            std::string& host,
                            int port,
                            bool compress)
{
  if (dbAdapt.init(dbConnGroup, user, pass, database, host, port, compress, true))
  {
    boost::lock_guard<boost::mutex> g(_mutex);
    if (!_initialized)
    {
      loadTableDefs(dbSection);
      loadSQLCalls();
      _initialized = true;
    }
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
// isValid - test if we have a connection to the database
//---------------------------------------------------------------------------
bool
ORACLEDBHistoryServer::isValid()
{
  return dbAdapt.isValid();
}

//---------------------------------------------------------------------------
// loadTableDefs function
// loads table substitutions based on a
// file that DBAs place on servers for physical table mapping
//---------------------------------------------------------------------------
void
ORACLEDBHistoryServer::loadTableDefs(std::string& dbSection)
{
  std::vector<ConfigMan::NameValue> tables;
  std::string iniFileName;

  // Make sure the table defs are loaded into the SQLQuery class
  std::string key = "historyTableDefs";
  SQLQuery::classInit("Historical", _config, dbSection, key);
}

//---------------------------------------------------------------------------
// loadSQLCalls  function
// load SQL Defs into map kept in this class
// SQL paramameters are specified as %(num)([quote][numeric]:
// example :  %2q  "second parm, quote string
// Note: when ORACLE version has PreparedStatement we will change these to ?
//---------------------------------------------------------------------------
void
ORACLEDBHistoryServer::loadSQLCalls()
{
  // Each query must declare a SQLQueryInitializerHelper
  // Historical queries construct with true for isHistorical
  //
  SQLQueryInitializer::initAllQueryClasses(true);
}
}
