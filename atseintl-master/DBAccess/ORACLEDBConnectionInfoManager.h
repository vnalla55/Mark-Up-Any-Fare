//----------------------------------------------------------------------------
//
//     File:           ORACLEDBConnectionInfoManager.h
//     Description:    Oracle Database connection info manager implementation.
//     Created:        03/05/2010
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
#include "DBAccess/DBConnectionInfoManagerImplBase.h"

#include <string>

namespace tse
{
class DataManager;
class ConfigMan;
};

namespace DBAccess
{

class ORACLEDBConnectionInfoManager : public DBConnectionInfoManagerImplBase
{
public:
  virtual ~ORACLEDBConnectionInfoManager();

protected:
  ORACLEDBConnectionInfoManager(tse::ConfigMan& config);

  virtual DBPoolInfo* createPoolInfo(const std::string& pool, bool isHistorical) override;

  virtual DBConnectionInfo* createConnInfo(const DBPoolInfo* poolInfo) override;

  friend class tse::DataManager;

private:
  ORACLEDBConnectionInfoManager();

  bool getClusterSpecificServiceName(const std::string& user,
                                     const std::string& password,
                                     const std::string& explorerServiceName,
                                     std::string& clusterSpecificServiceName);

  static log4cxx::LoggerPtr& getLogger();

  tse::ConfigMan& _config;

  std::string _explorerServiceSuffix;

}; // ORACLEDBConnectionInfoManager

}; // namespace DBAccess

