//----------------------------------------------------------------------------
//  File:    ORACLEDBServer.h
//  Created: Jan 14, 2009
//  Authors: Emad Girgis
//
//  Description:
//    ORACLEDBServer is a DBServer implementation for doing data access and
//    object relational mapping from the ORACLE database for ATSE International.
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
//----------------------------------------------------------------------------

#pragma once
#include "DBAccess/DBServer.h"
#include "DBAccess/ORACLEAdapter.h"

#include <boost/thread/mutex.hpp>

#include <string>

namespace tse
{
class ConfigMan;

class ORACLEDBServer : public DBServer
{
  friend class QueryMockDataManager;
  static void deinitialize() { ORACLEDBServer::_initialized = false; }
  static bool _initialized;

  ORACLEAdapter dbAdapt;
  static boost::mutex _mutex;
  ConfigMan& _config;

public:
  ORACLEDBServer(ConfigMan& config) : _config(config) {}

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

  DBAdapter* getAdapter() override { return &dbAdapt; }

  bool isValid();
  void loadSQLCalls() override;
  void loadTableDefs(std::string& dbSection);
};
} // namespace tse

