//----------------------------------------------------------------------------
//
//  File:    ORACLEDBServer.C
//  Created: Jan 14, 2009
//  Authors: Emad Girgis
//
//  Description:
//     ORACLEDBServer is a DBServer implementation for doing data access and
//     object relational mapping from the ORACLE database for ATSE International.
//
//  Change History:
//    date - initials - description
//
//  Copyright Sabre 2004
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
#include "DBAccess/ORACLEDBServer.h"

#include "Common/Config/ConfigMan.h"
#include "DBAccess/ORACLEAdapter.h"
#include "DBAccess/SQLQuery.h"

#include <boost/thread/mutex.hpp>

#include <string>
#include <vector>

namespace tse
{
bool ORACLEDBServer::_initialized = false;
boost::mutex tse::ORACLEDBServer::_mutex;

//---------------------------------------------------------------------------
// init  function
// initialize DBAdapter; load SQL Defs and table names
// file that DBAs place on servers for physical table mapping
//---------------------------------------------------------------------------
bool
ORACLEDBServer::init(std::string& dbConnGroup,
                     std::string& dbSection,
                     std::string& user,
                     std::string& pass,
                     std::string& database,
                     std::string& host,
                     int port,
                     bool compress)
{
  if (dbAdapt.init(dbConnGroup, user, pass, database, host, port, compress, false))
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
ORACLEDBServer::isValid()
{
  return dbAdapt.isValid();
}

//---------------------------------------------------------------------------
// loadTableDefs function
// Now just calls SQLQuery::classInit()
//---------------------------------------------------------------------------
void
ORACLEDBServer::loadTableDefs(std::string& dbSection)
{
  // Load the tabledefs in the SQLQuery class
  std::string key = "tableDefs";
  SQLQuery::classInit(_config, dbSection, key);
} // loadTableDefs()

//---------------------------------------------------------------------------
// loadSQLCalls  function
// load SQL Defs into map kept in this class
// SQL paramameters are specified as %(num)([quote][numeric]:
// example :  %2q  "second parm, quote string
// Note: when ORACLE version has PreparedStatement we will change these to ?
//---------------------------------------------------------------------------
void
ORACLEDBServer::loadSQLCalls()
{
  // Initialize all queries - Each query must declare a SQLQueryInitializerHelper
  //     using the template declared in SQLQuery.h
  //
  SQLQueryInitializer::initAllQueryClasses(false);

} // loadSQLCalls()
}
