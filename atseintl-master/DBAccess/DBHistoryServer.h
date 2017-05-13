//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once
#include <string>

namespace tse
{
class DBAdapter;

class DBHistoryServer
{
public:
  virtual ~DBHistoryServer() = default;

  virtual void loadSQLCalls() = 0; // Now called from CacheManager::initCache()
  virtual void
  loadTableDefs(std::string& dbSection) = 0; // Now called from CacheManager::initCache()

  /** init is the function that logs on to the database server and establishes a session
  */
  virtual bool init(std::string& DBConnGroup,
                    std::string& dbSection,
                    std::string& user,
                    std::string& pass,
                    std::string& database,
                    std::string& host,
                    int port) = 0;

  virtual DBAdapter* getAdapter() = 0;
};
} // namespace tse
