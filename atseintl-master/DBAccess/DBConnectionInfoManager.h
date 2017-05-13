//----------------------------------------------------------------------------
//
//     File:           DBConnectionInfoManager.h
//     Description:    Database connection info manager static interface.
//     Created:        03/01/2010
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright 2010, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc. Any unauthorized use, reproduction, or
//         transfer of this software/documentation, in any medium, or
//         incorporation of this software/documentation into any system
//         or publication, is strictly prohibited.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"

#include <stdint.h>

namespace tse
{
class DataManager;
};

namespace DBAccess
{
class DBConnectionInfoManagerImplBase;

class DBConnectionInfoManager
{
public:
  // Public interface methods are all static
  //
  static bool getConnectInfo(const std::string& pool,
                             std::string& host,
                             uint16_t& port,
                             std::string& database,
                             std::string& service,
                             std::string& user,
                             std::string& password,
                             bool isHistorical = false);

  static bool isCurrentConnectInfo(const std::string& pool,
                                   const std::string& host,
                                   uint16_t port,
                                   const std::string& database,
                                   const std::string& service,
                                   const std::string& user,
                                   const std::string& password);

  static void processConnectionFailure(const std::string& host,
                                       uint16_t port,
                                       const std::string& database,
                                       const std::string& service,
                                       const std::string& user,
                                       bool resetPriorityOrder = true);

  static bool clearConnectInfo();

private:
  DBConnectionInfoManager();
  ~DBConnectionInfoManager();

  static void setImpl(DBConnectionInfoManagerImplBase* impl);
  static DBConnectionInfoManagerImplBase* getImpl();

  static DBConnectionInfoManagerImplBase* _impl;

  friend class tse::DataManager;

}; // class DBConnectionInfoManager

}; // namespace DBAccess

