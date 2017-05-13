//----------------------------------------------------------------------------
//
//  File:    ORACLEDBHistoryServer.h
//  Created: Jan 14, 2009
//  Authors: Emad Girgis
//
//  Description:
//    ORACLEDBHistoryServer is a DBHistoryServer implementation for doing data access and
//    object relational mapping from the ORACLE database for ATSE International.
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

#pragma once
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/ORACLEAdapter.h"

#include <boost/thread/mutex.hpp>

#include <string>

namespace tse
{
class ConfigMan;

class ORACLEDBHistoryServer final : public DBHistoryServer
{
  static bool _initialized;

  ORACLEAdapter dbAdapt;
  static boost::mutex _mutex; // Thread Safety
  ConfigMan& _config;

public:
  ORACLEDBHistoryServer(ConfigMan& config) : _config(config) {}

  /** init is the function that logs on to the database server and establishes a session
   */
  bool init(std::string& dbConnGroup,
            std::string& dbSection,
            std::string& user,
            std::string& pass,
            std::string& database,
            std::string& host,
            int port) override
  {
    return init(dbConnGroup, dbSection, user, pass, database, host, port, false);
  }

  bool init(std::string& dbConnGroup,
            std::string& dbSection,
            std::string& user,
            std::string& pass,
            std::string& database,
            std::string& host,
            int port,
            bool compress);

  //---------------------------------------------------------------------------
  // getAdapter  function
  // fetch a reference to the adapter  (used primarily by DAO objects)
  //---------------------------------------------------------------------------
  DBAdapter* getAdapter() override { return &dbAdapt; }

  bool isValid();
  void loadSQLCalls() override; // Now called from CacheManager::initCache()
  void loadTableDefs(std::string& dbSection) override; // Now called from CacheManager::initCache()
};
} // namespace tse

